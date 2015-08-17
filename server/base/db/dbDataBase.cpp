#include "dbDataBase.h"
#include "dbRecordSet.h"
#include <iostream>
#include <string.h>

namespace FOXSDK
{
#ifdef WIN32
#pragma comment(lib, "libmySQL.lib")
#pragma comment(lib, "winmm.lib")
#endif

	dbDatabase::dbDatabase() :
		mDbc( F_NULL ) ,
		mOpen( F_FALSE )
	{
		memset( mDBName, 0 , sizeof(mDBName) );
	}

	dbDatabase::~dbDatabase()
	{
		while ( mAsyncSQL_Request.size() > 0 )
		{
			//logManager::Print( "AsyncSQL_Request:%d\r\n" , mAsyncSQL_Request.size() );

			SLEEP( 100 );
		}

		this->Close();
		//F_SAFE_DELETE( mThread );

		//logManager::Print("~CDatabase Over \n");
	}


    void			dbDatabase::ConvertBinaryToString( char* pBinaryData , int nLen , char* buffer )
    {
        if ( !mDbc )
        {
            return;
        }

        mysql_real_escape_string( mDbc , buffer , pBinaryData , nLen );
    }

	fbool			dbDatabase::Open( const fbyte* szDBServer, const fbyte* szLoginName,
		const fbyte* szPassword, const fbyte* szDBName, fbool bSQLChk)
	{
		strcpy(mHost, szDBServer);
		strcpy(mUser, szLoginName);
		strcpy(mPasswd, szPassword);
		strcpy(mDBName, szDBName);

		if ( !szDBServer ) {
			// You must specify a database server name.
			return F_FALSE;
		}

		if ( !szLoginName ) {
			// You must specify a database user name.
			return F_FALSE;
		}

		if ( !szDBName) {
			// You must specify a database name.
			return F_FALSE;
		}

		mOpen = F_TRUE;

        thread t(boost::bind( &dbDatabase::OnThreadCreate , this ) );

		return true;
	}

	fint32			dbDatabase::OnThreadCreate()
	{
		//cerr << "dbDatabase::OnThreadCreate()\n" << endl;
		mDbc = this->Connect( mHost, mUser, mPasswd, mDBName);

		if ( !mDbc )
		{
			//logManager::Print( "dbtool connect failed." );
			//mThread->Close();
			return 1;
		}

        while( mOpen && mDbc )
		{
            this->OnThreadProcess();
		}

		return 0;
	}

	fint32			dbDatabase::OnThreadDestroy()
	{
		//cerr << "dbDatabase::OnThreadDestroy()\n" << endl;
		mysql_thread_end();
		return 0;
	}

	fint32			dbDatabase::OnThreadEvent()
	{
		//cerr << "dbDatabase::OnThreadEvent()\n" << endl;
		return 0;
	}


	fint32			dbDatabase::OnThreadProcess()
	{
		dbRequest* pRequest = F_NULL;
		dbResult* pResult = F_NULL;

		if ( mAsyncSQL_Request.empty() )
		{
		}
		else
		{
            mQueueMutex.lock();
			pRequest = mAsyncSQL_Request.front();
			mAsyncSQL_Request.pop();
            mQueueMutex.unlock();
		}

		static fint32 sleepn = 0;

		if ( pRequest == F_NULL )
		{
			sleepn = 0;
			SLEEP( 100 );
			return 0;                   //**************** a *return* routine here ********************
		}

		sleepn++;
		if ( sleepn > 100 )
		{
			sleepn = 0;
			SLEEP( 30 );
		}


#ifdef DEBUG
		static fint32 count = 0; count++;
		FLOG0( "db Ressult %d" , count );
#endif
		//DWORD dwStart = ::clock();

		//time_t StartTime = timeGetTime();
		MYSQL_RES* pSqlResult;
		mDBMutex.lock();
		if ( mysql_query( mDbc, pRequest->Desrible) != 0 )
		{
			// Error in database requesting.
			FLOG4( "%s" , mysql_error(mDbc) );
			F_SAFE_DELETE(pRequest);
			mDBMutex.unlock();
			return 0;					//**************** a *return* routine here ********************
		}

		pSqlResult = mysql_store_result(mDbc);
		mysql_next_result(mDbc);

		mDBMutex.unlock();

		dbRecordset *pRecordSet = NULL;
		if (pSqlResult == NULL)
		{
			// No result?
			// TODO: not a good resolve.
			F_SAFE_DELETE(pRequest);
			return 0;                  //**************** a *return* routine here ********************
		}
		else
		{
			pRecordSet = new dbRecordset( *this , DB_MODE_EDIT );
			if (pRecordSet == NULL)
			{
				//logManager::Print("memory (new dbRecordset) allocating failed");
				mysql_free_result(pSqlResult);
				// TODO: not a good resolve.
				F_SAFE_DELETE(pRequest);
				return 0;					//**************** a *return* routine here ********************
			}
			if (!pRecordSet->Create(pSqlResult))
			{
				F_SAFE_DELETE(pRequest);
				F_SAFE_DELETE(pRecordSet);
				return 0;					//**************** a *return* routine here ********************
			}
		}

		if ((pResult = new dbResult) == NULL)
		{
			//logManager::Print("memory (new dbResult) allocating failed");
			F_SAFE_DELETE(pRequest);
			F_SAFE_DELETE(pRecordSet);
			return 0;					//**************** a *return* routine here ********************
		}

		memset( pResult , 0 , sizeof(dbResult) );
		pResult->callback = pRequest->callback;
		pResult->Falg = F_TRUE;
		pResult->pData = pRequest->pData;
		pResult->pDataF = pRequest->pDataF;
		pResult->pResult = pRecordSet;

		F_SAFE_DELETE(pRequest);

		mQueueMutex.lock();
		mAsyncSQL_Result.push( pResult );
		mQueueMutex.unlock();


		return 0;
	}

	fvoid			dbDatabase::Close()
	{
		mOpen = F_FALSE;

        // 这里得处理下，，
//		while (	mThread->GetStatus() != STATUS_CLOSED)
//		{
//			//SLEEP(100);
//			cerr << "waiting for the thread to quit." << endl;
//		}

		SLEEP( 10000 );

		if (mDbc != F_NULL)
		{
			mysql_close(mDbc);
			mDbc = NULL;
		}
	}

	MYSQL*			dbDatabase::Connect( const fbyte* szHost , const fbyte* szUser , const fbyte* szPasswd , const fbyte* szDB ,
			fuint32 uiPort, fbyte* szSocket, fuint32 uiFlag)
	{
		MYSQL* hdbc = mysql_init( F_NULL );

		if ( hdbc == F_NULL )
		{
			//logManager::Print( "ERROR: dbtool init error." );
			return F_NULL;
		}

		mysql_options( hdbc , MYSQL_READ_DEFAULT_GROUP , "" );
		mysql_options( hdbc , MYSQL_OPT_CONNECT_TIMEOUT , "" );

		hdbc->options.read_timeout = 1000;
		hdbc->reconnect = 1;
		hdbc->free_me = 1;

		unsigned long  flag = CLIENT_MULTI_RESULTS | CLIENT_MULTI_STATEMENTS
			| CLIENT_FOUND_ROWS | CLIENT_INTERACTIVE;

		if ( !mysql_real_connect( hdbc , szHost , szUser , szPasswd , szDB ,
			uiPort , szSocket , flag ) )
		{
			printf( "ERROR: dbtool real connect failed: %s \n" , mysql_error(hdbc) );

			mysql_close(hdbc);
			return F_NULL;
		}
		char *str = (char*)malloc(10);
		free(str);
		mysql_set_character_set( hdbc, "utf8" );
		mysql_set_server_option( hdbc, MYSQL_OPTION_MULTI_STATEMENTS_ON );
		return hdbc;
	}

	fint64			dbDatabase::GetLastInsertedID()
	{
		if ( !this->IsOpen() )
			return 0;
		fuint64 id;

		mDBMutex.lock();
		id = mysql_insert_id(mDbc);
		mDBMutex.unlock();
		return id;
	}

	fbool			dbDatabase::Execute( const fbyte* szSQL , DB_MODE eMode )
	{
		if ( !szSQL )
			return F_FALSE;

		if ( !this->IsOpen() )
			return F_FALSE;

		if ( F_NULL == ExecuteSQL( szSQL ) )
		{
			return F_TRUE;
		}

		return F_FALSE;
	}

	MYSQL_RES*		dbDatabase::ExecuteSQL( const fbyte* szSQL )
	{
		if ( !this->IsOpen() )
		{
			return F_NULL;
		}

		if (!mDbc)
		{
			//logManager::Print( "ERROR: Database not init" );
			return F_NULL;
		}

		MYSQL_RES* pRes = F_NULL;

		mDBMutex.lock();
		//DWORD dwStart = ::clock();

		if ( mysql_query( mDbc, szSQL ) != 0 )
		{
			//logManager::Print( "err: Database ExecuteSQL(%s) occur mysql error(%s).", mysql_error(mDbc), szSQL);
            mDBMutex.unlock();

			return F_NULL;
		}
		pRes = mysql_store_result( mDbc );
		this->StatisticSQL( szSQL, mAccess );
		mDBMutex.unlock();
		return pRes;
	}

	fbool			dbDatabase::ExecuteSyncSQL( const fbyte* pszSQL )
	{
		MYSQL_RES* pRes = this->ExecuteSQL( pszSQL );

		if (pRes !=NULL) {
			// There is a result;
			// But we will not return it.
			mysql_free_result(pRes);
			return F_TRUE;
		}
		return F_FALSE;
	}

	fbool			dbDatabase::ExecuteAsyncSQL( const fbyte* pszSQL , fvoid * pUser , fvoid* pUserF , SQL_RESULT_CALLBACK callback )
	{
		if ( !this->IsOpen() )
		{
			return F_FALSE;
		}

		if ( strlen(pszSQL) == 0 )
		{
			cerr << "sql command is null" << endl;
			return F_FALSE;
		}

		// chk sql
		if ( !(pszSQL && strlen(pszSQL) > 0 && strlen(pszSQL) < DB_MAX_SQL) )
		{
			cerr << "strlen(pszSQL = " << strlen(pszSQL) << endl;
			cerr << "no sql command" << endl;
			return F_FALSE;
		}

//		// set event
//		if ( mThread != F_NULL) {
//			// TODO: what is this for ?
//			mThread->SetEvent();
//		}

		dbRequest *request = new dbRequest;
		if (request == F_NULL) {
			//logManager::Print("memory (new dbRequest) allocating failed.");
		}
		memset( request, 0, sizeof(dbRequest) );

		strncpy( request->Desrible , pszSQL , ::strlen(pszSQL) );

		request->callback = callback;
		request->pData = pUser;
		request->pDataF = pUserF;

		mQueueMutex.lock();
		mAsyncSQL_Request.push( request );
		mQueueMutex.unlock();

		return F_TRUE;
	}

	// Out of maintaining.
	fbool			dbDatabase::CheckSQL( const fbyte* szSQL )
	{
		// TODO: Is this still of use ?
		if ( !szSQL )
			return F_FALSE;

		fbyte pszSQL[DB_MAX_SQL] = "";
		strcpy( pszSQL , szSQL );
		//strupr( pszSQL , DB_MAX_SQL);

		// update sql
		if ( 0 == strcmp(pszSQL, "UPDATE") )
		{
			if ( !strstr(pszSQL, "WHERE") || !strstr(pszSQL, "LIMIT") )
			{
				// invalid sql
				//logManager::Print( "ERROR: Invalid update SQL [%s]." , pszSQL );

				return F_FALSE;
			}
		}

		// delete sql
		if ( 0 == strcmp(pszSQL, "DELETE") )
		{
			if ( !strstr(pszSQL, "WHERE") )
			{
				// invalid sql
				//logManager::Print( "ERROR: Invalid SQL delete [%s]." , pszSQL );
				return F_FALSE;
			}
		}

		return F_TRUE;
	}

	// WARNING: Out of maintaining.
	fvoid			dbDatabase::StatisticSQL( const fbyte* pszSQL , fint32 dwRun )
	{
// 		if ( !pszSQL )
// 			return;
//
// 		char szSQL[DB_MAX_SQL] = "";
// 		strncpy( szSQL , pszSQL , strlen(pszSQL) );
// 		//strlwr( szSQL , DB_MAX_SQL );
//
// 		//
// 		fint32 dwTotalSQL = mInfoDB.TotalSQL;

//		if ( mTime.IsExpired() )
//		{
//			//crtManager::crtMemset( &mInfoDB , 0L , sizeof(DB_INFO) );
//			mInfoDB.TotalSQL = dwTotalSQL;
//		}

		//
// 		mInfoDB.TotalSQL++;
// 		mInfoDB.SQLPer5Min++;
// 		mInfoDB.SQLRunPer5Min += dwRun;
//
// 		if ( mInfoDB.SQLMaxPer5Min < dwRun )
// 			mInfoDB.SQLMaxPer5Min = dwRun;
//
// 		if ( 0 != ::strstr(szSQL, "update") )
// 		{
// 			mInfoDB.UpdatePer5Min++;
// 			mInfoDB.UpdateRunPer5Min += dwRun;
// 			if ( mInfoDB.UpdateMaxPer5Min < dwRun )
// 				mInfoDB.UpdateMaxPer5Min = dwRun;
// 		}
// 		else if ( 0 != ::strstr(szSQL, "select") )
// 		{
// 			mInfoDB.SelectPer5Min++;
// 			mInfoDB.SelectRunPer5Min += dwRun;
// 			if ( mInfoDB.SelectMaxPer5Min < dwRun )
// 				mInfoDB.SelectMaxPer5Min = dwRun;
// 		}
// 		else if ( 0 != ::strstr(szSQL, "insert") )
// 		{
// 			mInfoDB.InsertPer5Min++;
// 			mInfoDB.InsertRunPer5Min += dwRun;
// 			if ( mInfoDB.InsertMaxPer5Min < dwRun )
// 				mInfoDB.InsertMaxPer5Min = dwRun;
// 		}
// 		else if ( 0 != ::strstr(szSQL, "delete") )
// 		{
// 			mInfoDB.DeletePer5Min++;
// 			mInfoDB.DeleteRunPer5Min += dwRun;
// 			if ( mInfoDB.DeleteMaxPer5Min < dwRun )
// 				mInfoDB.DeleteMaxPer5Min = dwRun;
// 		}
	}

	// WARNING:  Out of maintaining.
	fvoid			dbDatabase::UpdateStatInfo( DB_INFO& infoDB )
	{
		memset( &infoDB , 0L , sizeof(DB_INFO) );

		fulong dwTotalSQL = mInfoDB.TotalSQL;

//		if ( mTime.IsExpired() )
//		{
//			mInfoDB.SQLAvgPer5Min += (mInfoDB.SQLPer5Min!=0) ? mInfoDB.SQLRunPer5Min*1000 / mInfoDB.SQLPer5Min : 0;
//			mInfoDB.SelectAvgPer5Min += (mInfoDB.SelectRunPer5Min!=0) ? mInfoDB.SelectRunPer5Min*1000 / mInfoDB.SelectPer5Min : 0;
//			mInfoDB.UpdateAvgPer5Min += (mInfoDB.UpdateRunPer5Min!=0) ? mInfoDB.UpdateRunPer5Min*1000 / mInfoDB.UpdatePer5Min : 0;
//			mInfoDB.InsertAvgPer5Min += (mInfoDB.InsertRunPer5Min!=0) ? mInfoDB.InsertRunPer5Min*1000 / mInfoDB.InsertPer5Min : 0;
//			mInfoDB.DeleteAvgPer5Min += (mInfoDB.DeleteRunPer5Min!=0) ? mInfoDB.DeleteRunPer5Min*1000 / mInfoDB.DeletePer5Min : 0;
//
//			infoDB = mInfoDB;
//
//			memset( &mInfoDB , 0L , sizeof(DB_INFO) );
//
//			mInfoDB.TotalSQL = dwTotalSQL;
//
//			return;
//		}

		infoDB = mInfoDB;
	}

	// WARNING: Out of maintaining.
	dbIRecordset*	dbDatabase::CreateRecordset( const fbyte* szSQL , DB_MODE eMode )
	{
		if ( !szSQL )
			return F_NULL;

		if ( !this->IsOpen() )
			return F_NULL;

 		dbRecordset* pRes = new dbRecordset( *this , eMode );

 		if ( !pRes )
 			return F_NULL;

 		if ( !pRes->Create(szSQL) )
 		{
 			F_SAFE_DELETE(pRes);
  			return F_NULL;
 		}
  		return pRes;
	}

	// WARNING: Out of maintaining.
	dbIRecord*		dbDatabase::MakeDefaultRecord( const fbyte* szTable , fulong id )
	{
		if ( !szTable )
			return F_NULL;

		if ( !this->IsOpen() )
			return F_NULL;

 		dbRecordset* pRes = new dbRecordset( *this , DB_MODE_EDIT );

 		if ( !pRes )
 			return F_NULL;

 		dbRecord* pRec = pRes->MakeDefaultRecord(szTable);

 		if ( !pRec )
 		{
 			F_SAFE_DELETE(pRes);
 			return F_NULL;
 		}

 		return (dbIRecord*)pRec;
	}

	dbResult*			FOXSDK::dbDatabase::GetAsyncResult()
	{
		dbResult *result = NULL;

		mQueueMutex.lock();
		if (mAsyncSQL_Result.empty())
		{
			result = NULL;
		}
		else
		{
			result = mAsyncSQL_Result.front();
			mAsyncSQL_Result.pop();
		}
		mQueueMutex.unlock();

		return result;
	}

	// WARNING: Out of maintianing.
	dbIDatabase*	DatabaseCreate( const fbyte* szDBServer , const fbyte* szLoginName , const fbyte* szPassword , const fbyte* szDBName , fbool bEnableSQLChk /* = F_TRUE */ )
	{
		FOXSDK::dbDatabase* pDB = new FOXSDK::dbDatabase;

		if ( !pDB )
			return F_NULL;

		if ( !pDB->Open( szDBServer , szLoginName , szPassword , szDBName , bEnableSQLChk ) )
		{
			pDB->Release();

			return F_NULL;
		}

		return pDB;
	}

} // end namespace FOXSDK

