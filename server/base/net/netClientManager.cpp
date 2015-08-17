
#include "netClientManager.h"
#include "logManager.h"
#include "msgDefine.h"

using namespace FOXSDK;


fvoid			OnNetClientMsgEnter( netSocket* socket )
{

}
fvoid			OnNetClientMsgExit( netSocket* socket )
{

}




SocketClientMsgMap	FOXSDK::netClientManager::mMsgMap;
netClientManager* FOXSDK::gNetClientManager;
netClientManager::netClientManager()
{
	gNetClientManager = this;
}

netClientManager::~netClientManager()
{

}


netClient*	netClientManager::CreateClient( const fbyte* ip , fuint16 port , fint32 i , fint32 o )
{
	fuint32 count = mClientVector.size();
	for ( fuint32 j = 0 ; j < count ; ++j )
	{
		if ( STRCMP( mClientVector[ j ]->IP.c_str() , ip ) == 0 && mClientVector[ j ]->Port == port )
		{
			return mClientVector[ j ];
		}
	}

	netClient* client = new netClient();
	client->Init( i , o );
	client->SetAddress( ip , port );
	client->SetHandler( OnNetClientMsgEnter , OnNetClientMsgExit , HandlerMainSocket );
	client->Start();

	mClientVector.push_back( client );

	return client;
}


fvoid		netClientManager::SendMsg( netMsgHead* head )
{
	for ( fuint32 i = 0 ; i < mClientVector.size() ; i++ )
	{
		netSocket* socket = mClientVector[ i ]->GetSocket();

		if ( socket && socket->IsVaild() )
		{
			socket->PackMsg( head );
			socket->SendMsg();
		}
	}
}


fvoid       netClientManager::RegeditMsg( fint32 msg , fvoid* handler , const fbyte* des )
{
    if ( mMsgMap.find( msg ) != mMsgMap.end() )
    {
        ASSERT( 0 );
    }
    
    netClientMsgHandler h;
    h.f = handler;
    h.des = des;
    mMsgMap[ msg ] = h;
}


fvoid       netClientManager::HandlerMainSocket( netSocket* socket , netMsgHead* head )
{
    SocketClientMsgMapIter iter = mMsgMap.find( head->type );
    
    if ( iter == mMsgMap.end() )
    {
        //FLOG0( "msg not reg %d ", head->type );
        return;
    }
    
#ifdef DEBUG
    //FLOG0( "recv msg %d %s " , head->type , iter->second.des.c_str() );
#endif
    
    void (*p)( netSocket* , netMsgHead* ) = ( void (*)( netSocket* , netMsgHead* ) )iter->second.f;
    if ( p )
    {
        (*p)( socket , head );
    }
}


fvoid       netClientManager::Update( freal32 delay )
{
    

}




