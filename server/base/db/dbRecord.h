#ifndef _DBRECORD_H_
#define _DBRECORD_H_

#include "dbRecordSet.h"
#include <mysql.h>
#include "dbConfig.h"





namespace FOXSDK
{
	class dbField;

	class dbRecord : public dbIRecord
	{

		friend class dbRecordset;

	protected:

		// Constructor & Destructor
		dbRecord( dbRecordset& res , fuint32 unAmountField );
		dbRecord( dbRecordset& res , MYSQL_ROW row , unsigned long* len , fuint32& unNumFields );
		dbRecord( dbRecordset& res , MYSQL_FIELD* fields , fuint32& unNumFields );

		virtual ~dbRecord();

	private:

		dbRecordset&	mRes;

		// Hide copy-constructor and assignment operator
		dbRecord( const dbRecord& rec );
		dbRecord& operator= ( const dbRecord& rec );

	public:

		// Interface of IRecord

		virtual	fvoid			Release()
		{ 
			delete this;
		}
		
		virtual fulong			GetFieldCount()
		{
			return mFields.size();
		}

		virtual dbIData&		Field( fuint32 unIndex );
		virtual dbIData&		Field( fuint32 unIndex ) const
		{ 
			return this->Field( unIndex ); 
		}

		virtual dbIData&		Field( const fbyte* pszName );
		virtual dbIData&		Field( const fbyte* pszName ) const
		{
			return this->Field(pszName);
		}


		virtual dbIData&		Key();

		virtual fbool			Update( fbool bSync = F_TRUE )
		{
			return mRes.UpdateRecord( this , bSync);
		}

		virtual fbool			Delete( fbool bArchive = F_FALSE )
		{
			return mRes.DeleteRecord(this, bArchive);
		}

		virtual fbool			Insert()
		{
			return mRes.InsertRecord(this);
		}

		virtual fvoid			ClsEditFlag();

	private:


		vector<dbField>		mFields;

		// Operation
	public:
		const fbyte*			KeyName();
		fbool					BuildSQLOperation( fbyte* pszOperationSQL );

	private:
		fvoid					BuildSQLCondition( fbyte* pszConditionSQL );
	};
}





#endif

