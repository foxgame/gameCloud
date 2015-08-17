#include "dbRecord.h"

using namespace FOXSDK;


dbRecord::dbRecord( dbRecordset& res , fuint32 unAmountField ): 
mRes( res )
{	
	for ( fuint32 i = 0 ; i < unAmountField ; i++ )
	{
		dbField objField( res.GetFieldInfo(i) );
		mFields.push_back( objField );
	}
}


dbRecord::dbRecord( dbRecordset& res , MYSQL_ROW row , unsigned long* len , fuint32& unNumFields ):
mRes( res )
{	
	for ( fuint32 i = 0 ; i < unNumFields ; i++ )
	{
		dbFieldInfo& info = res.GetFieldInfo(i);
		dbField objField( info );

		dbFieldInfo& pp = (dbFieldInfo&)objField.mInfo;
		pp.RealLen = len[ i ];
		objField.SetValue( row[i] );
		objField.TagChanged( F_FALSE );

		mFields.push_back( objField );
	}
}


dbRecord::dbRecord( dbRecordset& res , MYSQL_FIELD* fields , fuint32& unNumFields ):
mRes( res )
{	
	for ( fuint32 i = 0 ; i < unNumFields ; i++ )
	{
		dbField objField( res.GetFieldInfo(i) );
		objField.SetValue( fields[i].def );
		objField.TagChanged( F_FALSE );

		mFields.push_back( objField );
	}
}


dbRecord::~dbRecord()
{
	this->Update();

	mRes.Remove(this); 
	mFields.clear();
}



dbIData&			dbRecord::Field( fuint32 unIndex )
{
	if (unIndex >= mFields.size() )
	{
		//logManager::Print( "ERROR: index[%u] out of Record." , unIndex );
		unIndex	= 0;
	}

	return mFields[unIndex];
}


dbIData&			dbRecord::Field( const fbyte* pszName )
{
	if ( pszName )
	{
		for( fuint32 i = 0 ; i < mFields.size() ; i++ )
		{
			dbField& field = mFields[i];

			if ( 0 == strcmp( field.GetName() , pszName ) )
				return field;
		}

		// not found this field
		//logManager::Print( "ERROR:  not found field[%s] in Record" , pszName);
	}

	return this->Field( static_cast<unsigned int>(0) );
}


dbIData&			dbRecord::Key()
{
	return this->Field( mRes.mKeyIndex ); 
}


const fbyte*		dbRecord::KeyName()
{
	dbField& field = mFields[mRes.mKeyIndex]; 

	return field.GetName();
}


fvoid				dbRecord::ClsEditFlag()
{
	for ( fuint32 i = 0 ; i < mFields.size() ; i++ )
		mFields[i].TagChanged( F_FALSE );
}	



fbool				dbRecord::BuildSQLOperation( fbyte* pszOperationSQL )
{
	pszOperationSQL[0]	='\0';

	fbyte szFormat[1024] = "";
	fbool bFirst = F_TRUE;
	fbool bFlag = F_TRUE;

	for (fuint32 i = 0 ; i < mFields.size() ; i++ )
	{	
		dbField& field = mFields[i];
	
		if ( !field.IsChanged() )
			continue;

		switch ( field.GetType() )
		{
		case FIELD_TYPE_STRING:
		case FIELD_TYPE_VAR_STRING:
			if ( field.mStrVal.length() <= 0 ) 
				bFlag = F_FALSE;
			else
				sprintf( szFormat , "='%s'", field.mStrVal.c_str() );
			break;

		case FIELD_TYPE_FLOAT:
			sprintf( szFormat , "=%.2f", field.mDVal );
			break;

		case FIELD_TYPE_DOUBLE:
			sprintf( szFormat , "=%.2f", field.mDVal );
			break;

		case FIELD_TYPE_TINY:
			if ( (field.GetAttr() & UNSIGNED_FLAG) != 0 )
				sprintf( szFormat , "=%llu", field.mI64Val );
			else
				sprintf( szFormat , "=%lld", field.mI64Val );
			break;

		case FIELD_TYPE_SHORT:
			if ( (field.GetAttr()&UNSIGNED_FLAG) != 0 )
				sprintf( szFormat , "=%llu", field.mI64Val );
			else
				sprintf( szFormat , "=%lld", field.mI64Val );
			break;

		case FIELD_TYPE_LONG:
			if ( (field.GetAttr()&UNSIGNED_FLAG) != 0 )
				sprintf( szFormat , "=%llu", field.mI64Val );
			else
				sprintf( szFormat , "=%lld", field.mI64Val );
			break;

		case FIELD_TYPE_LONGLONG:
			if ( (field.GetAttr()&UNSIGNED_FLAG) != 0 )
				sprintf( szFormat , "=%llu", field.mI64Val );
			else
				sprintf( szFormat , "=%lld", field.mI64Val );
			break;

		default:
			//logManager::Print( "Error: unknow type in CRecord::BuildUpdateOpration()");
			break;
		}

		if ( bFlag )
		{	
			if ( !bFirst )
				strcat( pszOperationSQL , "," );
			else 
				bFirst = F_FALSE;

			strcat( pszOperationSQL , field.GetName() );
			strcat( pszOperationSQL , szFormat );
		}
		else
			bFlag = F_TRUE;
	}

	if ( pszOperationSQL[0] == '\0' )
		return F_FALSE;

	return F_TRUE;
}



fvoid				dbRecord::BuildSQLCondition( fbyte* pszConditionSQL )
{
	//memset( pszConditionSQL , 0L , sizeof(pszConditionSQL) );
	pszConditionSQL[0] ='\0';

	sprintf( pszConditionSQL ,"%s=%ld" , this->KeyName() , (flong)this->Key() );
}
