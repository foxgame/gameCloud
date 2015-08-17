//
//  server.h
//  test
//
//  Created by fox on 12-11-20.
//  Copyright (c) 2012年 __MyCompanyName__. All rights reserved.
//

#ifndef _NETSERVER_H_
#define _NETSERVER_H_



#include "netSocket.h"

namespace FOXSDK
{
    typedef fvoid(*netServerHandler)(netSocket* s);
    typedef fvoid(*netServerSHandler)(netSocket* s , netMsgHead* h );
    
    class netServer : public netService
    {
       
        
    public:
        
        friend class netSocket;
        
        netServer();
        virtual ~netServer();

		fvoid				Init( fint32 m , fint32 i , fint32 o , fubyte mc = F_FALSE );
        
        virtual fvoid		SetAccept( netSocket* socket );
        fvoid				SetAddress( const fbyte* ip , fuint16 port );
        fvoid				SetHandler( netServerHandler enter , netServerHandler exit , netServerSHandler msg );
        
        fvoid				ServerRun();
        fvoid				ServerUpdate();
        
        fvoid				Start();
        fvoid				Stop();
        
        fvoid				Update();
        
        
    protected:
        
        netServerHandler	OnEnter;
        netServerHandler	OnExit;
        netServerSHandler	OnMsg;
        
        fvoid				HandleStart();
        fvoid				HandleAccept( const boost::system::error_code& error , netSocket* socket );
        
        netSocket*			GetFreeSocket();
        
        
        SocketVector    mFreeSocket;
        SocketVector    mUsedSocket;
        SocketVector    mAcceptSocket;
        
        netSocket**		mSocket;
		fint32			mSocketMax;
        
        tcp::endpoint   mServerAddr;
        tcp::acceptor   mAcceptor;
        
        thread			mServiceThread;
        mutex			mClientListMutex;
    };

}





#endif
