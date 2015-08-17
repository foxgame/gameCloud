
#pragma warning(disable:4244 4996)

#include "netHttpClient.h"
#include "../server/server/QQSig.h"
#include <avhttp.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/date_time.hpp>

using namespace FOXSDK;

netHttpClient::netHttpClient()
{
}

netHttpClient::~netHttpClient(void)
{
}


fvoid					netHttpClient::Start()
{
	thread t(boost::bind(&netHttpClient::HandleStart,this));
	this_thread::yield();
	t.swap( mServiceThread );
}

fvoid					netHttpClient::HandleStart()
{
	thread_group tg;
	tg.create_thread( boost::bind( &netHttpClient::Run , this ) );

	this_thread::yield();
	tg.join_all();
}

fvoid					netHttpClient::SetHandler( netHttpClientHandler msg )
{
	OnMsgRecevied = msg;
}

fvoid					netHttpClient::HttpGet( const netHttpClientUrl& url )
{
	list.push_back( url );
}

fvoid					netHttpClient::RunHttp()
{
	try
	{
		#ifdef IOS_SERVER

		if ( !list.size() )
		{
		    SLEEP( 300 );
			return;
		}

		netHttpClintUrlListIter iter = list.begin();

		string body = "{";
		for ( map<string,string>::iterator iter1 = iter->keyValues.begin() ; iter1 != iter->keyValues.end() ; ++iter1 )
		{
			body += "\"";
			body += iter1->first;
			body += "\":\"";
			body += iter1->second;
			body += "\"";
		}
		body += "}";

		static map< fint64 , fbyte > passed;
		fint64 hashPassed = BKDRHash( body.c_str() );
		if ( passed.find( hashPassed ) != passed.end() )
		{
			list.erase( iter );
			return;
		}
		passed[ hashPassed ] = 1;

		asio::io_service io;
		avhttp::http_stream stream(io);
		avhttp::post_form( stream , body );
		stream.open( iter->url );



// 		std::istream is(&stream);
// 		is.unsetf(std::ios_base::skipws);
// 		std::string sz;
// 		sz.append(std::istream_iterator<char>(is), std::istream_iterator<char>()



		string stra,strc;
		vector<string> vecStr;
		ptree pt;
		stringstream jsonstream;
		jsonstream << &stream;
		read_json<ptree>( jsonstream , pt );

		netHttpClientUrl clientUrl;

		for ( ptree::iterator it = pt.begin(); it != pt.end() ; ++it )
		{
			string str1 = it->first.data();
			string str2 = it->second.data();
			if ( !str2.size() )
			{
				for ( ptree::iterator it1 = it->second.begin(); it1 != it->second.end() ; ++it1 )
				{
					string str3 = it1->first.data();
					string str4 = it1->second.data();
					clientUrl.keyValues[ str3 ] = str4;
				}
			}
			clientUrl.keyValues[ str1 ] = str2;
		}
		clientUrl.id = iter->id;

		list.erase( iter );

		mutex::scoped_lock lock( mClientListMutex );
		resultlist.push_back( clientUrl );
		lock.unlock();
#else

		if ( !list.size() )
		{
			SLEEP( 300 );
			return;
		}

		netHttpClintUrlListIter iter = list.begin();

		asio::io_service io;
		avhttp::http_stream stream(io);

		avhttp::get_form( stream , iter->cookie );
		stream.open( iter->url );

		string stra,strc;
		vector<string> vecStr;
		ptree pt;
		stringstream jsonstream;
		jsonstream << &stream;
		read_json<ptree>( jsonstream , pt );

		netHttpClientUrl clientUrl;

		for ( ptree::iterator it = pt.begin(); it != pt.end() ; ++it )
		{
			string str1 = it->first.data();
			string str2 = it->second.data();
			if ( !str2.size() )
			{
				for ( ptree::iterator it1 = it->second.begin(); it1 != it->second.end() ; ++it1 )
				{
					string str3 = it1->first.data();
					string str4 = it1->second.data();
					clientUrl.keyValues[ str3 ] = str4;
				}
			}
			clientUrl.keyValues[ str1 ] = str2;
		}
		clientUrl.id = iter->id;

		list.erase( iter );

		mutex::scoped_lock lock( mClientListMutex );
		resultlist.push_back( clientUrl );
		lock.unlock();

#endif

    }
	catch( std::exception& e )
	{
		cout << e.what();
	}

}

fvoid					netHttpClient::Update()
{
	if ( resultlist.size() )
	{
		mutex::scoped_lock lock( mClientListMutex );

		netHttpClintUrlListIter iter = resultlist.begin();
		OnMsgRecevied( *iter );
		resultlist.erase( iter );

		lock.unlock();
	}
}

fvoid					netHttpClient::Run()
{
	while ( true )
	{
		try
		{
			RunHttp();

			SLEEP( 30 );
		}
		catch( ... )
		{

		}
	}
}
