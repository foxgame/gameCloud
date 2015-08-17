#ifndef _DBDATABASE_H_
#define _DBDATABASE_H_

#include "baseDefine.h"

#include <time.h>
#include <queue>
#include <mysql.h>

//#include "dbRecordSet.h"
#include "dbConfig.h"


namespace FOXSDK
{
	const fulong DB_MAX_DBACCESSTIME = 10;
	const fulong DB_MAX_STRINGSIZE = 256;

	// ************************** class dbDatabase *************************
	class dbDatabase : public dbIDatabase
	{
	public:

		dbDatabase();
		virtual ~dbDatabase();

		virtual	fvoid			Release()
			{ delete this; }
		//! Start a thread for database service.
		// This thread will connect mysql server and deal with the request send through ExecuteAsyncSQL()
		fbool					Open( const fbyte* szDBServer, const fbyte* szLoginName,
									const fbyte* szPassword, const fbyte* szDBName,
									fbool bSQLChk = F_TRUE );

		//! Stop the database service thread and close the mysql connection.
		fvoid					Close();


        virtual void			ConvertBinaryToString( char* pBinaryData , int nLen , char* buffer );

		virtual fvoid			GetStatInfo(DB_INFO& infoDB)
			{ this->UpdateStatInfo(infoDB); }

		virtual fuint32			GetRequestSize()
			{ return (fuint32)mAsyncSQL_Request.size(); }

		virtual fuint32			GetResultSize()
			{ return (fuint32)mAsyncSQL_Result.size(); }

		fint64					GetLastInsertedID();

		MYSQL*					GetDBHandle()
			{ return mDbc; }

		fbool					IsOpen()
			{ return mOpen; }

		virtual fbool			IsValid()
		{
			return mDbc && mOpen;
		}

		//! The following three method: Execut(), ExecutSQL() and ExecuteSyncSQL() do
		// exactly the same thing. 
		// Keeping them for compitable.
		fbool					Execute( const fbyte* szSQL , DB_MODE eMode );
		MYSQL_RES*				ExecuteSQL( const fbyte* szSQL );
		fbool					ExecuteSyncSQL( const fbyte* pszSQL );
		
		// Execute a SQL routin asynchronously, you need to keep watch over the 
		// result of GetAsyncResult() for any query result.
		// Modification routine does not lead to any result.
		// All possible error will be write to the log using logManager::Print();
		fbool					ExecuteAsyncSQL( const fbyte* pszSQL , fvoid * pUser , fvoid* pUserF , SQL_RESULT_CALLBACK callback );

		// Get the oldest result.
		// Result NULL if there is no any result.
		virtual dbResult*		GetAsyncResult();

	protected:

		// WARNING: Out of maintaining.
		fvoid					UpdateStatInfo( DB_INFO& infoDB );
		// WARNING: Out of maintaining.
		fvoid					StatisticSQL( const fbyte* pszSQL , int dwRun );

		// WARNING: Out of maintaining.
		virtual dbIRecordset*	CreateRecordset( const fbyte* szSQL , DB_MODE eMode );
		// WARNING: Out of maintaining.
		virtual dbIRecord*		MakeDefaultRecord( const fbyte* szTable , fulong id /* = 0 */ );
		// WARNING: Out of maintaining.
		fbool					CheckSQL( const fbyte* szSQL );

		// Interface of crtIThreadEvent
		virtual fint32			OnThreadCreate();
		virtual fint32			OnThreadDestroy();
		virtual fint32			OnThreadEvent();
		virtual fint32			OnThreadProcess();

		//! Do the exact connecting to the mysql server.
		// The will setup some mysql parameters.
		MYSQL*					Connect( const fbyte* szHost , const fbyte* szUser , const fbyte* szPasswd , const fbyte* szDB , 
			fuint32 uiPort = MYSQL_PORT , fbyte* szSocket = F_NULL , fuint32 uiFlag = F_NULL );
        
        
	private:
		MYSQL*			mDbc;
		// Two Locks may lead to deadlocks, but can highly improve our efficiency.
        
		mutex   mDBMutex;
		mutex   mQueueMutex;
        
		fbool			mOpen;
		fbyte			mHost[DB_MAX_STRINGSIZE];
		fbyte			mUser[DB_MAX_STRINGSIZE];
		fbyte			mPasswd[DB_MAX_STRINGSIZE];
		fbyte			mDBName[DB_MAX_STRINGSIZE];
		clock_t			mAccess;
		DB_INFO			mInfoDB;
		//crtFastTimer	mTime;

		// DB Request List
		list<std::string>				mSetAsyncSQL; 

		// DB Request List
		queue<dbRequest*>			mAsyncSQL_Request;

		// DB Result List
		queue<dbResult*>			mAsyncSQL_Result; 
	};
};


#endif

