//
//  server.cpp
//  test
//
//  Created by fox on 12-11-20.
//  Copyright (c) 2012年 __MyCompanyName__. All rights reserved.
//

#include "netServer.h"

using namespace FOXSDK;

netServer::netServer()
:   mAcceptor( *this ) , mSocketMax( 0 )
{

}


netServer::~netServer()
{

}

fvoid			netServer::Init( fint32 m , fint32 i , fint32 o , fubyte mc )
{
	mSocketMax = m;
	mSocket = new netSocket*[ m ];
	for ( int j = 0 ; j < m ; j++ )
	{
		mSocket[ j ] = new netSocket( *this );
		mSocket[ j ]->InitBuffer( i , o );
		mSocket[ j ]->UseMsgCount( mc );
		mFreeSocket.push_back( mSocket[ j ] );
	}
}

netSocket*      netServer::GetFreeSocket()
{
	if ( mFreeSocket.empty() )
	{
		return NULL;
	}

	netSocket* socket = *mFreeSocket.begin();
	mFreeSocket.erase( mFreeSocket.begin() );

	return socket;
}


fvoid       netServer::SetAccept( netSocket* socket )
{
	mAcceptor.async_accept( *socket ,
		boost::bind( &netServer::HandleAccept , this , boost::asio::placeholders::error ,
		socket ) );
}


fvoid       netServer::SetAddress( const char* ip , unsigned short port )
{
	boost::system::error_code ec;
	mServerAddr = tcp::endpoint( address::from_string( ip , ec ) , port );
	assert(!ec);
}

fvoid       netServer::SetHandler( netServerHandler enter , netServerHandler exit , netServerSHandler msg )
{
	OnEnter = enter;
	OnExit = exit;
	OnMsg = msg;
}


fvoid       netServer::ServerRun()
{
	while ( true )
	{
		boost::system::error_code ec;

		try
		{
			run( ec );
		}
		catch ( ... )
		{
		}
	}
}


fvoid       netServer::ServerUpdate()
{

}


fvoid       netServer::Start()
{
	thread t( boost::bind( &netServer::HandleStart , this ) );
	this_thread::yield();
	t.swap( mServiceThread );
}


fvoid       netServer::HandleStart()
{
	boost::system::error_code ec;
	mAcceptor.open( mServerAddr.protocol(), ec );
	assert( !ec );
	mAcceptor.set_option(tcp::acceptor::reuse_address(true) , ec );
	assert( !ec );
	mAcceptor.bind( mServerAddr , ec );
	assert( !ec );
	mAcceptor.listen( socket_base::max_connections , ec );
	assert( !ec );

	for ( int i = 0 ; i < mSocketMax ; ++i )
	{
		SetAccept( mSocket[ i ] );
	}

	thread_group tg;
	for ( int i = 0; i < MAX_THREAD ; ++i )
	{
		tg.create_thread(boost::bind( &netServer::ServerRun , this ) );
	}

	this_thread::yield();
	tg.join_all();

	//ServerUpdate();
}


fvoid       netServer::Stop()
{

}


fvoid       netServer::Update()
{
	size_t size = mAcceptSocket.size();

	if ( size )
	{
		mutex::scoped_lock lock( mClientListMutex );

		size = mAcceptSocket.size();

		for ( size_t i = 0 ; i < size ; ++i )
		{
			SocketVectorIter iter = std::find( mUsedSocket.begin() , mUsedSocket.end() , mAcceptSocket[i] );
			if ( iter != mUsedSocket.end() )
			{
				(*OnExit)( *iter );
				mUsedSocket.erase( iter );

				FLOG0( "OnNetMsgExit size=%d" , mUsedSocket.size() );
			}

			(*OnEnter)( mAcceptSocket[i] );

			mUsedSocket.push_back( mAcceptSocket[i] );
			FLOG0( "OnNetMsgEnter size=%d " , mUsedSocket.size() );
		}

		mAcceptSocket.clear();

		lock.unlock();
	}

	size = mUsedSocket.size();

	for ( size_t i = 0 ; i < size ; ++i )
	{
		if ( mUsedSocket[ i ]->mIsReadyClose )
		{
			(*OnExit)( mUsedSocket[ i ] );

			mUsedSocket.erase( mUsedSocket.begin() + i );

			i--;
			size--;

			FLOG0( "OnNetMsgExit size=%d" , mUsedSocket.size() );

			continue;
		}
		
		netMsgHead* head = NULL;

		int b = mUsedSocket[ i ]->ReadMsg( &head );


		switch ( b )
		{
		case MSG_INVALID:
			{
				(*OnExit)( mUsedSocket[ i ] );

				mUsedSocket.erase( mUsedSocket.begin() + i );

				i--;
				size--;

				FLOG0( "OnNetMsgExit size=%d" , mUsedSocket.size() );
			}
			break;
		case MSG_OK:
			{
				(*OnMsg)( mUsedSocket[ i ] , head );

				mUsedSocket[ i ]->RemoveMsg( head->size );
			}
			break;
		default:
			break;
		}

	}

}


fvoid       netServer::HandleAccept( const boost::system::error_code& error , netSocket* socket )
{
	if ( error )
	{
		socket->Clear();
		SetAccept( socket );
	}
	else
	{
		// on enter

		mutex::scoped_lock lock( mClientListMutex );

		//        SocketVectorIter iter = std::find( mUsedSocket.begin() , mUsedSocket.end() , socket );
		//        if ( iter != mUsedSocket.end() )
		//        {
		//            OnExit();
		//            mUsedSocket.erase( iter );
		//        }


		mAcceptSocket.push_back( socket );

		socket->mClose = 0;
		socket->Run();

		lock.unlock();
	}
}

