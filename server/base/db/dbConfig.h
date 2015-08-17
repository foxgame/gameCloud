#ifndef _DBIDB_H_
#define _DBIDB_H_


#include "baseDefine.h"
#include "logManager.h"

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/container/set.hpp>
#include <boost/thread/thread.hpp>
using namespace boost;
using namespace boost::system;
using namespace boost::asio::ip;
using namespace boost::asio;

namespace FOXSDK
{
    
	const fint32		DB_MAX_SQL = 65535;
	const fint32		DB_MAX_HASH = 100;
    
	struct DB_INFO
	{
		fulong		TotalSQL;
		fulong		SQLPer5Min;
		fulong		SQLRunPer5Min;
		fulong		SQLAvgPer5Min;
		fulong		SQLMaxPer5Min;
        
		fulong		SelectPer5Min;
		fulong		SelectRunPer5Min;
		fulong		SelectAvgPer5Min;
		fulong		SelectMaxPer5Min;
        
		fulong		UpdatePer5Min;
		fulong		UpdateRunPer5Min;
		fulong		UpdateAvgPer5Min;
		fulong		UpdateMaxPer5Min;
        
		fulong		InsertPer5Min;
		fulong		InsertRunPer5Min;
		fulong		InsertAvgPer5Min;
		fulong		InsertMaxPer5Min;
        
		fulong		DeletePer5Min;
		fulong		DeleteRunPer5Min;
		fulong		DeleteAvgPer5Min;
		fulong		DeleteMaxPer5Min;
	};
    
    
    
	class dbIData
	{
	public:
        
		virtual	fvoid				Release() = 0;

		virtual fbyte*				GetValue() = 0;
		virtual fulong				GetRealLen() const = 0;
		virtual fulong				GetMaxLen() const = 0;
        virtual fulong				GetType() const = 0;
        
		// conversion function
		virtual						operator fbyte() const = 0;
		virtual						operator fubyte() const = 0;
        
		virtual						operator fint16() const = 0;
		virtual						operator fuint16() const = 0;
        
//		virtual						operator flong() const = 0;
//		virtual						operator fulong() const = 0;
        
		virtual						operator fint32() const = 0;
		virtual						operator fuint32() const = 0;
        
		virtual						operator fint64() const = 0;
		virtual						operator fuint64() const = 0;
        
		virtual						operator freal32() const = 0;
		virtual						operator freal64() const = 0;
        
		virtual						operator fbyte*() const = 0;
		virtual						operator const fbyte*() const = 0;
        
		// overload assignment operator
        
		virtual dbIData&			operator = ( fbyte chOp ) = 0;
		virtual dbIData&			operator = ( fubyte chOp ) = 0;
        
		virtual dbIData&			operator = ( fint16 nOp ) = 0;
		virtual dbIData&			operator = ( fuint16 unOp ) = 0;
        
//		virtual dbIData&			operator = ( flong lOp ) = 0;
//		virtual dbIData&			operator = ( fulong lOp ) = 0;
        
		virtual dbIData&			operator = ( fint32 iOp ) = 0;
		virtual dbIData&			operator = ( fuint32 uOp ) = 0;
        
		virtual dbIData&			operator = ( fint64 uOp ) = 0;
		virtual dbIData&			operator = ( fuint64 uOp ) = 0;
        
		virtual dbIData&			operator = ( freal32 fltOp ) = 0;
		virtual dbIData&			operator = ( freal64 dblOp ) = 0;
        
		virtual dbIData&			operator = ( const fbyte* szVal ) = 0;
	};
    
    
    
	class dbIRecord
	{
	public:
        
		virtual	fvoid				Release() = 0;
        
		// return field data by field index
		virtual dbIData&			Field( fuint32 idx ) = 0;
		virtual dbIData&			Field( fuint32 idx ) const = 0;
        
		// return field data by field name
		virtual dbIData&			Field( const fbyte* pszName ) = 0;
		virtual dbIData&			Field( const fbyte* pszName ) const = 0;
        
		virtual fulong				GetFieldCount() = 0;

		// return the key data of this record
		virtual dbIData&			Key() = 0;
        
		virtual fbool				Update( fbool bSync = F_TRUE ) = 0;
		virtual fbool				Delete( fbool bArchive = F_FALSE ) = 0;
		virtual fbool				Insert() = 0;
        
		// clear the edit flag of all field in this record, so update operation will ignored.
		virtual fvoid				ClsEditFlag() = 0;
        
	};
    
	class dbIRecordset
	{
	public:
        
		virtual	fvoid				Release() = 0;
        
		// get record from record set
		virtual dbIRecord*			GetRecord() const = 0;
        
		// count record in set.	 return -1: error
		virtual fulong				CountRecord() const = 0;
        
		// move to specified record
		virtual fvoid				Move( fuint32 index ) = 0;
        
		// update this record by "update, delete or insert" sql
		// bFlag = Sync or Archive
		virtual fbool				Update( fbool bFlag = F_TRUE ) = 0;
        
		// clear the edit flag of all field in this record, so update operation will ignored.
		virtual fvoid				ClsEditFlag	() = 0;
        
		// make a new default record, it doesn`t exist in dbtool
		virtual dbIRecord*			MakeDefRecord() = 0;
	};
    
    
	struct dbResult
	{
	public:
        
		fuint32			AccountID;
		fuint32			UserID;
		fvoid			(*callback)( dbResult*);
		fvoid*			pData;
		fvoid*			pDataF;
		fbool   		Falg;
		dbIRecordset*	pResult;
        
        
		fvoid			Release()
		{
            if ( pResult )
            {
                pResult->Release();
            }
            
			pResult = F_NULL;
            
			delete this;
		}
        
		dbResult& operator = ( dbResult* result )
		{
			if ( this == result )
			{
				return *this;
			}
            
			this->AccountID = result->AccountID;
			this->UserID = result->UserID;
			this->pResult = result->pResult;
			this->callback = result->callback;
			this->Falg = result->Falg;
			this->pResult = result->pResult;
            
			return *this;
		}
        };
        
        typedef fvoid (*SQL_RESULT_CALLBACK)( dbResult* );
        typedef fvoid (*SQL_RESULT_CALLBACK_STRUCT)( fvoid* );
        
        
        //Request
        struct dbRequest
        {
            fbyte           Desrible[DB_MAX_SQL];
            fvoid*			pData;
			fvoid*			pDataF;
            fvoid          (*callback)( dbResult* );
        };
        
        
        enum DB_MODE
        {
            DB_MODE_NONE = 0,
            DB_MODE_READ,
            DB_MODE_EDIT
        };
        
        
        class dbIDatabase
        {
        public:
            virtual	fvoid				Release() = 0;
            
            virtual void				ConvertBinaryToString( char* pBinaryData , int nLen , char* buffer ) = 0;
            
            
            // create a record set by specified "select" sql
            virtual dbIRecordset*		CreateRecordset( const fbyte* szSQL , DB_MODE eMode ) = 0;
            
            // Execute SP without return value
            virtual fbool				Execute( const fbyte* szSQL , DB_MODE eMode ) = 0;
            
            // make a new default record by "insert" sql.
            virtual dbIRecord*			MakeDefaultRecord( const fbyte* szTable , fulong id = 0 ) = 0;
            
            // get statistic info of dbtool running 
            virtual fvoid				GetStatInfo( DB_INFO& infoDB ) = 0;
            
            //get status of dbtool 
            virtual fuint32				GetRequestSize() = 0;
            
            virtual fuint32				GetResultSize() = 0;
            
            virtual fbool				ExecuteAsyncSQL( const fbyte* pszSQL , fvoid* pUser , fvoid* pUserF , SQL_RESULT_CALLBACK callback ) =0 ;
            
            virtual dbResult*			GetAsyncResult()= 0;

			virtual fbool				IsValid() = 0;      
        };
        
        
        extern "C" dbIDatabase*	DatabaseCreate( const fbyte* szDBServer , const fbyte* szLoginName , const fbyte* szPassword , const fbyte* szDBName , fbool bEnableSQLChk = F_TRUE ); 
        
        
    };
    
#endif
    
    
