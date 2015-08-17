//
//  netClient.cpp
//  test
//
//  Created by fox on 12-11-20.
//  Copyright (c) 2012年 __MyCompanyName__. All rights reserved.
//

#include "netClient.h"

using namespace FOXSDK;


netClient::netClient()
	:	mIsConnected( F_FALSE )
{
	mSocket = new netSocket( *this );
#ifdef TEST
	for ( fint32 i = 0 ; i < TEST_COUNT ; i++ )
	{
		mSocketTest[ i ] = new netSocket( *this );
		mIsConnectedTest[ i ] = F_FALSE;
	}
#endif
}

netClient::~netClient(void)
{
}


fvoid					netClient::Init( fint32 i , fint32 o )
{
	mSocket->InitBuffer( i , o );
#ifdef TEST
	for ( fint32 j = 0 ; j < TEST_COUNT ; j++ )
	{
		mSocketTest[ j ]->InitBuffer( i , o );
	}
#endif
}

fvoid					netClient::Start()
{
	thread t(boost::bind(&netClient::HandleStart,this));
	this_thread::yield();
	t.swap(mServiceThread);
}

fvoid					netClient::HandleStart()
{
	// Use thread group can add code to connect more servers
	// here can SetConnect more socket connect

	SetAccept( mSocket );

#ifdef TEST
	for ( fint32 i = 0 ; i < TEST_COUNT ; i++ )
	{
		SetAccept( mSocketTest[ i ] );
	}
#endif

	thread_group tg;
	for ( fuint32 i =  0 ; i < 1 ; ++i)
	{
		tg.create_thread( boost::bind( &netClient::Run , this ) );
	}

	this_thread::yield();
	tg.join_all();
}

fvoid					netClient::SetAddress( const fbyte* ip , fuint16 port )
{
	boost::system::error_code ec;
	mServerAddr = tcp::endpoint( address::from_string( ip , ec ) , port );
	assert(!ec);

	IP = ip;
	Port = port;
}

fvoid					netClient::SetAccept( netSocket* socket )
{
	socket->async_connect( mServerAddr ,
		boost::bind( &netClient::HandleConnect , this , boost::asio::placeholders::error , socket ) );
}

fvoid					netClient::HandleConnect( const boost::system::error_code& error , netSocket* socket )
{
    if( error.value() != boost::system::errc::success && 
		error.value() != boost::system::errc::already_connected )
    {
        FLOG0( "netClient error %s %d " , error.message().c_str() , error.value() );
		mIsConnected = F_FALSE;
		SetAccept( socket );
		SLEEP( 300 );
	}
	else
	{
        socket->mClose = 0;
		socket->Run();
	}
}

fvoid					netClient::SetHandler( netClientHandler enter , netClientHandler exit , netClientSHandler msg )
{
	OnMsgConnected		= enter;
	OnMsgRecevied		= msg;
	OnMsgDisconnect		= exit;
}

fvoid					netClient::Run()
{
	while ( true )
	{
		boost::system::error_code ec;
		try
		{
			run( ec );
		}
		catch( ... )
		{

		}
	}
}

netSocket*				netClient::GetSocket()
{
	return mSocket;
}

fvoid					netClient::Update()
{

#ifdef TEST

	for ( fint32 i = 0 ; i < TEST_COUNT ; i++ )
	{
		if ( !mIsConnectedTest[ i ] )
		{
			if ( mSocketTest[ i ]->IsVaild() )
			{
				mIsConnectedTest[ i ] = F_TRUE;
				(*OnMsgConnected)( mSocketTest[ i ] );
			}
		}

		netMsgHead* head = F_NULL;
		int status = mSocketTest[ i ]->ReadMsg( &head );
		switch (status)
		{
		case MSG_INVALID:
			{
				//(*OnMsgDisconnect)( mSocketTest[ i ] );
				//mIsConnectedTest[ i ] = F_FALSE;
			}
			break;
		case MSG_OK:
			{
				(OnMsgRecevied)( mSocketTest[ i ] , head );
				mSocketTest[ i ]->RemoveMsg( head->size );
			}
			break;
		case MSG_WAITTING:
		case MSG_REVCING:
			break;
		}
	}


#endif

	if ( !mIsConnected )
	{
		if ( mSocket->IsVaild() )
		{
			mIsConnected = F_TRUE;
			(*OnMsgConnected)(mSocket);
		}

		return;
	}

	netMsgHead* head = F_NULL;
	int status = mSocket->ReadMsg( &head );
	switch (status)
	{
	case MSG_INVALID:
		{
			(*OnMsgDisconnect)( mSocket );
			mIsConnected = F_FALSE;
		}
		break;
	case MSG_OK:
		{
			(OnMsgRecevied)( mSocket , head );
			mSocket->RemoveMsg( head->size );
		}
		break;
	case MSG_WAITTING:
	case MSG_REVCING:
		break;
	}

}


