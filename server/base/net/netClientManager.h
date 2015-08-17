//
//  GameSocketManager.h
//  
//


#ifndef _NETCLIENTMANAGER_H_
#define _NETCLIENTMANAGER_H_


#include "netClient.h"

namespace FOXSDK
{
	struct netClientMsgHandler
	{
		netClientMsgHandler()
		: f( F_NULL )
		{
		}

		fvoid*  f;
		string  des;
	};

	typedef map< fint32 , netClientMsgHandler > SocketClientMsgMap;
	typedef map< fint32 , netClientMsgHandler >::iterator SocketClientMsgMapIter;

	class netClientManager : public baseSingle< netClientManager >
	{
	public:

		netClientManager();
		~netClientManager();

		fvoid		RegeditMsg( fint32 msg , fvoid* handler , const fbyte* des );

		netClient*	CreateClient( const fbyte* ip , fuint16 port , fint32 i , fint32 o );

		fvoid		SendMsg( netMsgHead* head );

	protected:

		fvoid				Update( freal32 delay );
		static fvoid		HandlerMainSocket( netSocket* socket , netMsgHead* head );
		
		static SocketClientMsgMap	mMsgMap;

		NetClientVector		mClientVector;
	};


	extern netClientManager* gNetClientManager;

}


#endif


