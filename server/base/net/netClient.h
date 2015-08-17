//
//  server.h
//  test
//
//  Created by fox on 12-11-20.
//  Copyright (c) 2012年 __MyCompanyName__. All rights reserved.
//

#ifndef _NETCLIENT_H_
#define _NETCLIENT_H_



#include "netSocket.h"
#include "netServer.h"

#ifdef TEST
#define TEST_COUNT 1000
#endif

namespace FOXSDK
{
	typedef fvoid(*netClientHandler)(netSocket* s);
	typedef fvoid(*netClientSHandler)(netSocket* s , netMsgHead* h );

	class netClient : public netService
	{

	public:

		friend class netSocket;

		netClient();
		virtual ~netClient();

		string					IP;
		fuint16					Port;

		fvoid					Init( fint32 i , fint32 o );

		fvoid					SetAddress( const fbyte* ip , fuint16 port );
		fvoid					SetHandler( netClientHandler enter , netClientHandler exit , netClientSHandler msg );

		fvoid					Start();
		fvoid					Update();

		netSocket*				GetSocket();

		virtual fvoid			SetAccept( netSocket* socket);

	protected:

		
		fvoid					HandleStart();
		fvoid					HandleConnect( const boost::system::error_code& error, netSocket* socket);
		fvoid					HandleConnect( const boost::system::error_code& error );
		fvoid					Run();

		netClientHandler		OnMsgConnected;
		netClientSHandler		OnMsgRecevied;
		netClientHandler		OnMsgDisconnect;

		tcp::endpoint			mServerAddr;

		netSocket*				mSocket;
#ifdef TEST
		netSocket*				mSocketTest[ TEST_COUNT ];
		fbool					mIsConnectedTest[ TEST_COUNT ];
#endif
		thread					mServiceThread;
		fbool					mIsConnected;
	};

	typedef vector< netClient* > NetClientVector;
	typedef vector< netClient* >::iterator NetClientVectorIter;

}





#endif
