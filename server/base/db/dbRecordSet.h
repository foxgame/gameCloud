#ifndef _DBRECORDSET_H_
#define _DBRECORDSET_H_


#include "dbField.h"
#include "dbDataBase.h"

#include <mysql.h>



namespace FOXSDK
{

	class dbRecord;
	class dbRecordset : public dbIRecordset
	{
		friend class dbDatabase;
		friend class dbRecord;

    protected:

		// Constructor & Destructor
		dbRecordset( dbDatabase& dbtool , DB_MODE eMode );
		virtual ~dbRecordset();

		// Interface of IRecordset
	public:

		virtual fvoid			Release()
		{
			delete this;
		}
		
		virtual dbIRecord*		GetRecord() const;
		virtual fulong			CountRecord() const
		{
			return mRecord.size(); 
		}

		virtual fvoid			Move( fuint32 nIndex );
		virtual fbool			Update( fbool bSync = F_TRUE );
		virtual fvoid			ClsEditFlag();
		virtual dbIRecord*		MakeDefRecord();

		// Operation
	protected:
		
		fbool					Create( const fbyte* pszSQL );
		fbool					Create( MYSQL_RES* pRES );
		fvoid					Remove( dbRecord* pRec );
		dbRecord*				MakeDefaultRecord( const fbyte* pszTable );

		fbyte*					GetTableName()
		{
			return mTableName;
		}


		size_t					FieldsCount() const
		{ 
			return mFieldInfo.size();
		}

		dbFieldInfo&	GetFieldInfo( fint32 nIndex )
		{
			return mFieldInfo[ nIndex ];
		}

		fvoid				BuildSQLCondition( fbyte* pszConditionSQL );
		fvoid				BuildSQLOperation( fbyte* pszOperationSQL );

		fbool				UpdateRecord( dbRecord* pRec , fbool bSync = F_TRUE );
		fbool				DeleteRecord( dbRecord* pRec , fbool bArchive = F_FALSE );
		fbool				InsertRecord( dbRecord* pRec );

	private:


		// Hide copy-constructor and assignment operator
		dbRecordset( const dbRecordset& res );
		dbRecordset& operator= ( const dbRecordset& res );


		dbDatabase&				mDB;
		DB_MODE					mMode;

		fint32					mCursor;

		fbyte					mSQL[DB_MAX_SQL];
		fbyte					mTableName[32];
		fuint32					mKeyIndex;
		fuint32					mAutoIncIndex;

		vector< dbFieldInfo >	mFieldInfo;
		vector< dbRecord* >		mRecord;
	};


};



#endif



