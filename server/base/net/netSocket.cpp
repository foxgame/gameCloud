//
//  socket.cpp
//  test
//
//  Created by fox on 12-11-19.
//  Copyright (c) 2012年 __MyCompanyName__. All rights reserved.
//


#pragma warning(disable:4244 4996)

#include "netSocket.h"
#include "netServer.h"
#include "logManager.h"

using namespace FOXSDK;

//mutex   netSocket::mMutex;

fubyte msgCode[] = {41,35,62,4,97,108,86,46,82,16,73,113,113,59,105,107,51,38,91,60,
	7,12,62,25,36,94,13,28,6,55,71,94,51,18,77,72,67,59,11,38,31,3,90,125,9,56,37,
	31,93,84,75,124,22,117,69,59,19,13,9,10,28,91,46,50,32,26,80,110,64,120,54,125,
	18,73,50,118,30,125,73,92,45,79,20,114,68,64,102,80,107,68,48,55,50,59,33,34,118,
	34,17,29,97,11,31,90,48,74,25,2,57,114,29,73,44,0,126,69,25,85,105,0,50,106,73,76,
	83,63,103,86,63,20,86,126,45,92,14,102,3,111,87,73,97,127,105,15,97,77,81,30,29,28,
	22,114,114,102,29,112,4,79,74,119,2,87,104,57,44,83,75,73,18,30,51,116,30,12,116,85,
	84,31,84,36,89,126,53,79,50,34,116,76,79,83,16,45,72,83,15,117,102,89,29,42,101,64,
	119,43,120,1,7,68,14,95,80,0,84,97,13,62,123,5,21,7,59,51,2,31,24,112,18,90,100,84,
	78,49,5,62,105,21,120,70,106,4,22,115,14,89,22,47,103,104,84,119,74,74,80,87,104,118,
	99
};

netSocket::netSocket( io_service& io_service ):tcp::socket ( io_service ) ,
	mHeadBuffer( buffer( mRecvBuffer , sizeof(netMsgHead) ) ),
	mBodyBuffer( buffer( mRecvBuffer + sizeof(netMsgHead) , MAX_SOCKET_BUFFER - sizeof(netMsgHead) ) ) ,
	mSendBuffer1( buffer( mSendBuffer , MAX_SOCKET_BUFFER ) ),
	mBodyLen( 0 ) , mRecvStage( FSRS_NULL ) ,
	mVaild( 0 ) ,
	mSend( 0 ) ,
	mClose( 0 ) ,
	mTimer( io_service ) ,
	mCloseTimer( io_service ) ,
	mUseMsgCount( 0 ) , mUseMsgCountSend( 0 ) ,
	mMsgCount( 0 ) , mMsgCountSend( 0 ) ,
	mIsReadyClose( 0 )
{
	mService = (netService*)&io_service;

	boost::asio::socket_base::linger option( true , 0 );
	boost::system::error_code ec1;
	set_option( option , ec1 );
}

netSocket::~netSocket()
{

}

fvoid	netSocket::InitBuffer( fint32 i , fint32 o )
{
	mIBuffer.InitBuffer( i );
	mOBuffer.InitBuffer( o );


}


fint32      netSocket::ReadMsg( netMsgHead** head )
{
	if ( !mVaild )
	{
		return MSG_INVALID;
	}

	int len = mIBuffer.GetLen();

	if ( len )
	{
		*head = (netMsgHead*)mIBuffer.GetStart();

		if ( (*head)->size > len )
		{
			return MSG_REVCING;
		}

		return MSG_OK;
	}

	return MSG_WAITTING;
}


fvoid       netSocket::RemoveMsg( fint32 len )
{
	mIBuffer.RemoveBuffer( len );
}


fvoid       netSocket::RecvMsg( const boost::system::error_code& ec, size_t bytes_transferred )
{
	if ( ec )
	{
		ReadyClose();
		return;
	}

	if ( !mVaild || mClose || mIsReadyClose )
	{
		return;
	}

	mTimer.cancel();

#ifdef DEBUG
	static fint32 count = 0; count++;
	static fint32 count1 = 0;
#endif

	switch ( mRecvStage )
	{
	case (fint32)FSRS_HEAD:
		{
			netMsgHead* head = (netMsgHead*)mRecvBuffer;
			mBodyLen = head->size;

			if ( mBodyLen == 0 )
			{
				ReadyClose();
				return;
			}

			if ( mBodyLen == sizeof( netMsgHead ) )
			{
				mIBuffer.Write( mRecvBuffer , sizeof( netMsgHead ) );
				ReadHead();
#ifdef DEBUG
				count1 += mBodyLen;
				FLOG0( "RecvMsg count=%d total=%.2fk" , count , count1 * 0.001f );
#endif
				return;
			}
			else
			{
				mRecvStage = FSRS_BODY;
				ReadBody();
				return;
			}
		}
		break;
	case (fint32)FSRS_BODY:
		{
#ifdef USE_ZIP
			netDataStream stream;
			stream.reserve( mBodyLen );

			if ( mBodyLen <=  sizeof( netMsgHead ) )
			{
				ReadBody();
				return;
			}

			if ( mUseMsgCount )
			{
				mMsgCount++;
				for ( fint32 i = sizeof( netMsgHead ) ; i < mBodyLen ; i++ )
				{
					mRecvBuffer[ i ] -= msgCode[ mMsgCount ];
				}
			}

			try
			{
				boost::iostreams::filtering_ostream sm;
				sm.push( boost::iostreams::gzip_decompressor() );
				sm.push( boost::iostreams::back_inserter( stream ) );

				boost::iostreams::copy( boost::make_iterator_range( mRecvBuffer + sizeof( netMsgHead ) , mRecvBuffer + mBodyLen ),
					sm );
			}
			catch ( ... )
			{
				ReadyClose();
				return;
			}

			fuint16* ppsize = (fuint16*)mRecvBuffer;
			fuint16 ss = stream.size();
			*ppsize = ss + sizeof( netMsgHead ) ;

			if ( *ppsize > mIBuffer.GetSpace() - 32 )
			{
				FLOG3( "iBuffer not enouth space." );
				mRecvStage = FSRS_HEAD;
				ReadHead();
				return;
			}

			mIBuffer.Write( mRecvBuffer , sizeof( netMsgHead ) );
			mIBuffer.Write( (fbyte*)&stream[ 0 ] , ss );
			mRecvStage = FSRS_HEAD;
			ReadHead();

#ifdef DEBUG
			count1 += mBodyLen;
			FLOG0( "RecvMsg count=%d total=%.2fk" , count , count1 * 0.001f );
#endif

#else
#ifdef DEBUG
			count1 += mBodyLen;
			FLOG0( "RecvMsg count=%d total=%.2fk" , count , count1 * 0.001f );
#endif
			if ( mBodyLen > mIBuffer.GetSpace() - 32 )
			{
				FLOG3( "iBuffer not enouth space." );
				mRecvStage = FSRS_HEAD;
				ReadHead();
				return;
			}

			mIBuffer.Write( mRecvBuffer , mBodyLen );
			mRecvStage = FSRS_HEAD;
			ReadHead();
#endif
			return;
		}
		break;
	default:
		{
			//assert( 0 );
		}
		break;
	}

}


fvoid       netSocket::Clear()
{
	if ( !mVaild )
	{
		return;
	}

	mTimer.cancel();
	mCloseTimer.cancel();

	mVaild = 0;
	mIsReadyClose = 0;

	mRecvStage = FSRS_NULL;

	mBodyLen = 0;

	mIBuffer.ClearBuffer();
	mOBuffer.ClearBuffer();

	mSend = 0;
	mMsgCount = 0;
	mMsgCountSend = 0;

    //FLOG0( "Close" );
}


fbool       netSocket::PackMsg( netMsgHead* head )
{
	if ( !mVaild || mClose || mIsReadyClose )
	{
		return F_FALSE;
	}

	if ( head->size > mOBuffer.GetSpace() - 32 )
	{
		FLOG3( "oBuffer not enouth space." );
		return F_FALSE;
	}

#ifdef USE_ZIP

	netDataStream stream;
	stream.reserve( head->size );

#ifdef DEBUG
	static fint32 count = 0; count++;
	static fint32 count1 = 0;
#endif

	if ( head->size == sizeof( netMsgHead ) )
	{
		mOBuffer.Write( (fbyte*)head , head->size );
#ifdef DEBUG
		count1 += head->size;
		FLOG0( "PackMsg count=%d total=%.2fk type=%d" , count , count1 * 0.001f , head->type );
#endif
	}
	else
	{

        try
        {
            filtering_ostream sm;
            sm.push( boost::iostreams::gzip_compressor() );
            sm.push( boost::iostreams::back_inserter( stream ) );


            copy( boost::make_iterator_range( (fbyte*)head + sizeof( netMsgHead ) , (fbyte*)head + head->size ),sm );
        }
        catch ( ... )
        {
            ReadyClose();
            return F_FALSE;
        }

		fuint16 size = (fuint16)stream.size();
		fuint16 ss = size + sizeof( netMsgHead );

		if ( mUseMsgCountSend )
		{
			mMsgCountSend++;

			for ( fuint16 i = 0 ; i < size ; ++i )
			{
				stream[ i ] += msgCode[ mMsgCountSend ];
			}
		}

		mOBuffer.Write( (fbyte*)&ss , 2 );
		mOBuffer.Write( (fbyte*)&head->type , 2 );
		mOBuffer.Write( (fbyte*)&stream[ 0 ], size );

#ifdef DEBUG
		count1 += ss;
		FLOG0( "PackMsg count=%d total=%.2fk type=%d" , count , count1 * 0.001f , head->type );
#endif
	}
#else
	mOBuffer.Write( (fbyte*)head , head->size );
#endif

	return F_TRUE;
}

fvoid       netSocket::SendMsg()
{
	if ( !mVaild || mClose || mIsReadyClose )
	{
		return;
	}

	if ( !mSend )
	{
		int len = mOBuffer.ReadRemove( &mSendBuffer );

		if ( len )
		{
			mSend = 1;

			async_write( *this , mSendBuffer1 , transfer_exactly( len ) , boost::bind( &netSocket::SendMsg , this , boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred )  ) ;
		}
	}
}

fvoid       netSocket::SendMsg( const boost::system::error_code& ec, size_t bytes_transferred )
{
	if ( ec )
	{
		ReadyClose();
		return;
	}

#ifdef DEBUG
	static fint32 count = 0; count++;
	static fint32 count1 = 0; count1 += bytes_transferred;
	FLOG0( "SendMsg count=%d total=%.2fk" , count , count1 * 0.001f );
#endif
	int len = mOBuffer.ReadRemove( &mSendBuffer );

	if ( len )
	{
		async_write( *this , mSendBuffer1 , transfer_exactly( len ) , boost::bind( &netSocket::SendMsg , this , boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred )  ) ;
	}
	else
	{
		mSend = 0;
	}
}

fvoid		netSocket::ReadyClose()
{
	if ( !mVaild || mClose || mIsReadyClose )
	{
		return;
	}

	try
	{
		boost::system::error_code ec;
		shutdown( boost::asio::ip::tcp::socket::shutdown_both , ec );
	}
	catch ( ... )
	{
	}

	mClose = 1;

	mTimer.cancel();
	mCloseTimer.cancel();

	mCloseTimer.expires_from_now( boost::posix_time::seconds( 3 ) );
	mCloseTimer.async_wait( boost::bind( &netSocket::HandleClose , this , boost::asio::placeholders::error ) );

	mIsReadyClose = 1;
}

void		netSocket::HandleClose( const boost::system::error_code& error )
{
	if( error )
	{
		return;
	}

	if ( !mIsReadyClose )
	{
		return;
	}

	try
	{
		boost::system::error_code ec2;
		tcp::socket::close( ec2 );
	}
	catch ( ... )
	{
	}

	Clear();
	mService->SetAccept( this );
}

void        netSocket::HandleWait( const boost::system::error_code& error )
{
    if( error )
    {
        return;
    }

	ReadyClose();
}


fvoid       netSocket::ReadHead()
{
	async_read( *this , mHeadBuffer ,
		transfer_exactly( sizeof(netMsgHead) ) ,
		boost::bind( &netSocket::RecvMsg , this , boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred ) ) ;

	mTimer.cancel();
	mTimer.expires_from_now( boost::posix_time::seconds( 180 ) );
	mTimer.async_wait( boost::bind( &netSocket::HandleWait , this , boost::asio::placeholders::error ) );
}


fvoid       netSocket::ReadBody()
{
	async_read( *this , mBodyBuffer ,
		transfer_exactly( mBodyLen - sizeof(netMsgHead) ) ,
		boost::bind( &netSocket::RecvMsg , this , boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred ) ) ;

	mTimer.cancel();
	mTimer.expires_from_now( boost::posix_time::seconds( 180 ) );
	mTimer.async_wait( boost::bind( &netSocket::HandleWait , this , boost::asio::placeholders::error) );
}


fvoid       netSocket::Run()
{
	mRecvStage = FSRS_HEAD;

	mVaild = 1;

	ReadHead();
}

