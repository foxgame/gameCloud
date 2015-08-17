
#include "dbRecordSet.h"
#include "dbRecord.h"


using namespace FOXSDK;

const fuint32 INVALID_KEY = UINT_MAX;

dbRecordset::dbRecordset( dbDatabase& dbtool , DB_MODE eMode ):
mDB( dbtool ) ,
mCursor( 0 ),
mKeyIndex( INVALID_KEY ) ,
mAutoIncIndex( INVALID_KEY ) ,
mMode( eMode )
{
	memset( mSQL , 0L , sizeof(mSQL) );

}



dbRecordset::~dbRecordset()
{
	while ( mRecord.size() != 0 )
	{
		dbRecord* pRec = mRecord[mRecord.size() - 1];

		F_SAFE_RELEASE( pRec );
	}

	mCursor = 0;
	mKeyIndex = INVALID_KEY;
	mMode = DB_MODE_NONE;
	mFieldInfo.clear();
	mRecord.clear();
}


fbool			dbRecordset::Create( const fbyte* pszSQL )
{	
	if ( !pszSQL )
		return F_FALSE;

	if ( mDB.IsOpen() )
		return F_FALSE;

	MYSQL_RES* pRES = mDB.ExecuteSQL( pszSQL );

	// save SQL statement
	memcpy( mSQL, pszSQL, strlen(pszSQL) );
	//_strupr_s( mSQL , DB_MAX_SQL);

	return this->Create( pRES );
}


fbool			dbRecordset::Create( MYSQL_RES* pRES )
{
	if ( !pRES )
		return F_FALSE;

	// 填写字段信息
	MYSQL_FIELD* fields = mysql_fetch_fields( pRES );

	if ( !fields )
	{
		mysql_free_result( pRES );
		return F_FALSE;
	}
	fuint32 unNumFields = mysql_num_fields( pRES );

	for ( fuint32 i = 0 ; i < unNumFields ; i++ )
	{
		dbFieldInfo fieldInfo;
		fieldInfo.Name = fields[i].name;
		fieldInfo.Type = fields[i].type;
		fieldInfo.Attr = fields[i].flags;
		fieldInfo.Len = fields[i].length;
		fieldInfo.MaxLen = fields[i].max_length;
        
		mFieldInfo.push_back( fieldInfo );

		if ( fields[i].flags & PRI_KEY_FLAG )
		{
			mKeyIndex = i;
		}

		if ( fields[i].flags & AUTO_INCREMENT_FLAG )
		{
			mAutoIncIndex = i;
		}
	}

	fint64 ulNumRecord = mysql_num_rows( pRES );

	for ( fulong r = 0 ; r < ulNumRecord ; r++ )
	{
		mysql_data_seek( pRES , r );

		MYSQL_ROW row = mysql_fetch_row( pRES );
		unsigned long* len = mysql_fetch_lengths( pRES );

		if ( !row )
			continue;

 		dbRecord* pRec = new dbRecord( *this , row , len , unNumFields );
 
 		mRecord.push_back( pRec );
	}

	// move first!
	this->Move(0);

	// get table name
	strcpy( mTableName , fields[0].table );

	// free MYSQL_RES
	mysql_free_result( pRES );

	return F_TRUE;
}


fvoid			dbRecordset::Remove( dbRecord* pRec )
{	
	std::vector< dbRecord* >::iterator itor = std::find( mRecord.begin() , mRecord.end() , pRec );

	if ( itor != mRecord.end() )
	{
		mRecord.erase(itor);
	}
}


dbRecord*		dbRecordset::MakeDefaultRecord( const fbyte* pszTable )
{	
	if ( !pszTable )
		return F_NULL;

	if ( !mDB.IsOpen() )
		return F_NULL;

	fbyte szSQL[100];
	sprintf( szSQL , "SELECT * FROM %s LIMIT 0" , pszTable );


	MYSQL_RES* pRES = mDB.ExecuteSQL( szSQL );
	
	if ( !pRES )
		return F_NULL;

	// 填写字段信息
	MYSQL_FIELD* fields = mysql_fetch_fields( pRES );
	MYSQL_ROW row = mysql_fetch_row( pRES );
	unsigned long* len = mysql_fetch_lengths( pRES );

	if ( !fields )
		return F_NULL;

	fuint32 unNumFields = mysql_num_fields( pRES );

	for (fuint32 i = 0 ; i < unNumFields ; i++ )
	{
		dbFieldInfo fieldInfo;
		fieldInfo.Name = fields[i].name;
		fieldInfo.Type = fields[i].type;
		fieldInfo.Attr = fields[i].flags;
		fieldInfo.Len = fields[i].length;
		fieldInfo.MaxLen = fields[i].max_length;
		fieldInfo.RealLen = len[i];

		mFieldInfo.push_back(fieldInfo);

		if ( fields[i].flags & PRI_KEY_FLAG )
		{
			mKeyIndex	= i;
		}
		if ( fields[i].flags & AUTO_INCREMENT_FLAG )
		{
			mAutoIncIndex= i;
		}
	}

	// 填写缺省记录数据
 	dbRecord* pDefRec = new dbRecord( *this , fields , unNumFields );
 	mRecord.push_back( pDefRec );

	// move first!
	this->Move(0);

	// save SQL statement
	memcpy( mSQL , szSQL , strlen(szSQL) );
	//_strupr_s( mSQL , DB_MAX_SQL);

	// get table name
	strcpy( mTableName , fields[0].table );

	// free MYSQL_RES
	mysql_free_result( pRES );

	return pDefRec;
}



dbIRecord*		dbRecordset::GetRecord() const
{	
	if ( 0 > mCursor )
		return F_NULL;

	return mRecord[mCursor];
}


fvoid			dbRecordset::Move( fuint32 nIndex )
{	
	if ( mRecord.empty() )
	{
		mCursor = -1;

		return;
	}

	if ( nIndex >= mRecord.size() )
		mCursor = (fint32)mRecord.size() - 1;
	else
		mCursor = nIndex;
}


fbool			dbRecordset::Update( fbool bSync /* = F_TRUE */ )
{	
 	std::vector< dbRecord* >::iterator itor = mRecord.begin();
 
 	for (; itor != mRecord.end() ; ++itor )
 	{
 		dbRecord* pRec = *itor;

 		if ( pRec )
 			continue;
 
 		this->UpdateRecord( pRec , bSync );
 	}

	return F_TRUE;
}



fvoid			dbRecordset::ClsEditFlag()
{
	for ( fuint32 i = 0 ; i< mRecord.size() ; i++ )
	{
 		dbRecord* pRec = mRecord[i];
 	
 		if (!pRec)
 			continue;
 
 		pRec->ClsEditFlag();
	}
}


dbIRecord*		dbRecordset::MakeDefRecord()
{
	if ( !mDB.IsOpen() )
		return F_NULL;

	if ( DB_MODE_EDIT != mMode )
		return F_NULL;

	// new a default record, but it isn`t in dbtool	
	fuint32 unAmountField = (fuint32)mFieldInfo.size();

 	dbRecord* pRec = new dbRecord( *this , unAmountField );

 	if ( !pRec )
 		return F_NULL;
 
 	return pRec;
}


fvoid			dbRecordset::BuildSQLCondition( fbyte* pszConditionSQL )
{
	if ( !pszConditionSQL )
		return;

	//memset( pszConditionSQL , 0L , sizeof(pszConditionSQL) );
	pszConditionSQL[0]	='\0';

 	dbRecord* pRec = mRecord[mCursor];
 	
 	if ( !pRec )
 		return;

	sprintf( pszConditionSQL , "%s=%s", pRec->KeyName() , (char*)pRec->Key() );
}


fvoid			dbRecordset::BuildSQLOperation( fbyte* pszOperationSQL )
{
	if ( !pszOperationSQL )
		return;

	//memset(pszOperationSQL , 0L , sizeof(pszOperationSQL) );
	pszOperationSQL[0]	='\0';

 	dbRecord* pRec = mRecord[mCursor];
 
 	if ( !pRec )
 		return;
 
 	if  ( DB_MODE_EDIT == mMode )
 		pRec->BuildSQLOperation( pszOperationSQL );
}


fbool			dbRecordset::UpdateRecord( dbRecord* pRec , fbool bSync /* = F_TRUE */ )
{
	if ( !mDB.IsOpen() )
		return F_FALSE;

	if ( DB_MODE_EDIT != mMode )
		return F_FALSE;

 	if ( !pRec )
 		return F_FALSE;

	// build sql operate
	fbyte szOperationSQL[1024];

	if ( !pRec->BuildSQLOperation(szOperationSQL) )
		return F_FALSE;

	// build sql condition
	fbyte szConditionSQL[128];
	pRec->BuildSQLCondition( szConditionSQL );

	fbyte szSQL[2048] = "";
	sprintf( szSQL , "UPDATE %s SET %s WHERE %s LIMIT 1" , mTableName , szOperationSQL , szConditionSQL );


	fbool ret = F_FALSE;

	//20070511 		if (bSync)
	// 			ret = mDB.ExecuteSyncSQL(szSQL);
	// 		else
	// 			ret = mDB.ExecuteAsyncSQL(szSQL);


	// clear edit flag
	if ( ret )
		this->ClsEditFlag();

	return ret;
}



fbool			dbRecordset::DeleteRecord( dbRecord* pRec , fbool bArchive /* = F_FALSE */ )
 {
 	if ( !mDB.IsOpen() )
 		return F_FALSE;
 
 	if ( DB_MODE_EDIT != mMode )
 		return F_FALSE;
 
 	if ( bArchive )
 	{
 		// build operate sql
 		fbyte szOperationSQL[1024];
 		pRec->BuildSQLOperation( szOperationSQL );
 
 		fbyte szConditionSQL[128];
 		pRec->BuildSQLCondition( szConditionSQL );
 
 		fbyte szSQL[2048];
 		sprintf( szSQL , "UPDATE %s SET %s WHERE %s LIMIT 1", mTableName , szOperationSQL , szConditionSQL );

 		if ( !mDB.ExecuteSyncSQL(szSQL) )
 			return F_FALSE;
 	}
 	else
 	{
 		// build cdffition sql
 		fbyte szConditionSQL[128];
 		pRec->BuildSQLCondition( szConditionSQL );
 
 		fbyte szSQL[1024];
 		sprintf( szSQL , "DELETE FROM %s WHERE %s LIMIT 1" , mTableName , szConditionSQL );
 		
		if ( !mDB.ExecuteSyncSQL(szSQL) )
 			return F_FALSE;
 	}
 
 	std::vector< dbRecord* >::iterator itor = std::find( mRecord.begin() , mRecord.end() , pRec );
 
	if ( itor != mRecord.end() )
 		mRecord.erase( itor );
 
 	F_SAFE_RELEASE( pRec );
 
	return F_TRUE; 
}


 
///////////////////////////////////////////////////////////////////////
fbool			dbRecordset::InsertRecord( dbRecord* pRec )
{
	if ( F_NULL == pRec )
		return F_NULL;

	if ( !mDB.IsOpen() )
		return F_NULL;

	if ( DB_MODE_EDIT != mMode )
		return F_NULL;

	fbyte szOperationSQL[1024];
	pRec->BuildSQLOperation( szOperationSQL );

	// INSERT a new record into table
	fbyte szSQL[128];
	sprintf( szSQL , "INSERT INTO %s SET %s", mTableName , szOperationSQL );
	if ( mDB.ExecuteSyncSQL(szSQL) )
		return F_NULL;

	if ( mAutoIncIndex != INVALID_KEY ) 
	{
		pRec->Field( mAutoIncIndex ) = (flong)mDB.GetLastInsertedID();
	}

	mRecord.push_back( pRec );

	return F_TRUE;
}

