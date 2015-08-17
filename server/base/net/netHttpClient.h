#ifndef _NETHTTPCLIENT_H_
#define _NETHTTPCLIENT_H_



#include "netSocket.h"
#include "netServer.h"

namespace FOXSDK
{
	struct netHttpClientUrl 
	{
		fint64 id;
		string url;
		string cookie;
		map< string , string > keyValues;
	};
	typedef list< netHttpClientUrl > netHttpClintUrlList;
	typedef list< netHttpClientUrl >::iterator netHttpClintUrlListIter;


	typedef fvoid(*netHttpClientHandler)( netHttpClientUrl& result );

	class netHttpClient : public netService
	{
	public:

		netHttpClient();
		virtual ~netHttpClient();

		virtual fvoid			SetAccept( netSocket* socket ){};

		fvoid					SetHandler( netHttpClientHandler msg );

		fvoid					Start();
		fvoid					Update();

		fvoid					HttpGet( const netHttpClientUrl& url );

	protected:

		fvoid					HandleStart();

		netHttpClintUrlList		list;
		netHttpClintUrlList		resultlist;

		fvoid					Run();
		fvoid					RunHttp();

		netHttpClientHandler	OnMsgRecevied;

		thread					mServiceThread;
		mutex					mClientListMutex;
	};

}





#endif
