
#include "dbField.h"


using namespace FOXSDK;


dbField::dbField( const dbFieldInfo& info ):
mInfo( info ) ,
mChanged( F_FALSE )
{

}		

dbField::dbField( const dbField& field ): 
mInfo( field.mInfo ) ,
mChanged( F_FALSE )
{
	mI64Val	= field.mI64Val;
	mStrVal	= field.mStrVal;
	MEMCPY( mValue , field.mValue , MAX_INT16 );
}


dbField::~dbField()
{
	
}


fvoid			dbField::FireError()
{
	//logManager::Print( "WARNNING: datatype mismatch , return unstable value" );
}

dbField& dbField::operator = (const dbField& field)
{
	if( this == &field )
		return *this;

	if ( mInfo.Type != field.mInfo.Type )
	{
		FireError();
	}

	mI64Val = field.mI64Val;
	mStrVal	= field.mStrVal;

	mChanged = F_FALSE;

	return *this;
}		


dbField::operator fbyte() const
{
	if ( (UNSIGNED_FLAG & mInfo.Attr)
		|| mInfo.Type == FIELD_TYPE_SHORT 
		|| mInfo.Type == FIELD_TYPE_LONG 
		|| mInfo.Type == FIELD_TYPE_FLOAT 
		|| mInfo.Type == FIELD_TYPE_DOUBLE )
	{
		FireError();
	}

	return (fbyte)mI64Val;
}



dbField::operator fubyte() const
{ 
	if ( !(UNSIGNED_FLAG & mInfo.Attr)
		||mInfo.Type == FIELD_TYPE_SHORT 
		|| mInfo.Type == FIELD_TYPE_LONG 
		|| mInfo.Type == FIELD_TYPE_LONGLONG 
		|| mInfo.Type == FIELD_TYPE_FLOAT 
		|| mInfo.Type == FIELD_TYPE_DOUBLE )
	{
		FireError();
	}

	return (fubyte)mI64Val;
}


dbField::operator fint16() const
{ 
	if ( (UNSIGNED_FLAG & mInfo.Attr)
		|| mInfo.Type == FIELD_TYPE_LONG 
		|| mInfo.Type == FIELD_TYPE_LONGLONG 
		|| mInfo.Type == FIELD_TYPE_FLOAT 
		|| mInfo.Type == FIELD_TYPE_DOUBLE )
	{
		FireError();
	}

	return (fint16)mI64Val; 
}

dbField::operator fuint16() const
{ 
	if ( !(UNSIGNED_FLAG & mInfo.Attr)
		|| mInfo.Type == FIELD_TYPE_LONG 
		|| mInfo.Type == FIELD_TYPE_LONGLONG 
		|| mInfo.Type == FIELD_TYPE_FLOAT 
		|| mInfo.Type == FIELD_TYPE_DOUBLE )
	{
		FireError();
	}
	return (fuint16)mI64Val;
}

//dbField::operator flong() const
//{
//	if ( (UNSIGNED_FLAG & mInfo.Attr)
//		|| mInfo.Type == FIELD_TYPE_LONGLONG 
//		|| mInfo.Type == FIELD_TYPE_FLOAT 
//		|| mInfo.Type == FIELD_TYPE_DOUBLE )
//	{
//		FireError();
//	}
//	return mI64Val;
//}
//
//dbField::operator fulong() const
//{ 
//	if ( !(UNSIGNED_FLAG & mInfo.Attr)
//		|| mInfo.Type == FIELD_TYPE_LONGLONG 
//		|| mInfo.Type == FIELD_TYPE_FLOAT 
//		|| mInfo.Type == FIELD_TYPE_DOUBLE )
//	{
//		FireError();
//	}
//
//	return mI64Val;
//}
//

dbField::operator fint32() const
{ 
	if ( (UNSIGNED_FLAG & mInfo.Attr)
		|| mInfo.Type == FIELD_TYPE_LONGLONG 
		|| mInfo.Type == FIELD_TYPE_FLOAT 
		|| mInfo.Type == FIELD_TYPE_DOUBLE )
	{
		FireError();
	}

	return (fint32)mI64Val;
}

dbField::operator fuint32() const
{
	if ( !(UNSIGNED_FLAG & mInfo.Attr)
		|| mInfo.Type == FIELD_TYPE_LONGLONG 
		|| mInfo.Type == FIELD_TYPE_FLOAT 
		|| mInfo.Type == FIELD_TYPE_DOUBLE )
	{
		FireError();
	}

	return (fuint32)mI64Val;
}

dbField::operator fint64() const
{
	if ( (UNSIGNED_FLAG & mInfo.Attr)
		|| mInfo.Type == FIELD_TYPE_FLOAT 
		|| mInfo.Type == FIELD_TYPE_DOUBLE )
	{
		FireError();
	}

	return mI64Val; 
}

dbField::operator fuint64() const
{
	if ( !(UNSIGNED_FLAG & mInfo.Attr)
		|| mInfo.Type == FIELD_TYPE_FLOAT 
		|| mInfo.Type == FIELD_TYPE_DOUBLE )
	{
		FireError();
	}

	return (fuint64)mI64Val; 
}

dbField::operator freal32() const
{
	if ( mInfo.Type != FIELD_TYPE_FLOAT )
	{
		FireError();
	}

	return (freal32)mDVal;
}

dbField::operator freal64() const
{ 
	if ( mInfo.Type != FIELD_TYPE_DOUBLE 
		&&  mInfo.Type != FIELD_TYPE_FLOAT )
	{
		FireError();
	}

	return mDVal;
}

dbField::operator fbyte* () const
{
	if (mInfo.Type == FIELD_TYPE_STRING || mInfo.Type == FIELD_TYPE_VAR_STRING ||
		mInfo.Type == MYSQL_TYPE_TINY_BLOB || mInfo.Type == MYSQL_TYPE_MEDIUM_BLOB ||
		mInfo.Type == MYSQL_TYPE_LONG_BLOB || mInfo.Type == MYSQL_TYPE_BLOB )
		return (char*)mStrVal.c_str();

	FireError();

	return (fbyte*)mStrVal.c_str();
}

dbField::operator const fbyte* () const
{
	if (mInfo.Type == FIELD_TYPE_STRING || mInfo.Type == FIELD_TYPE_VAR_STRING)
		return mStrVal.c_str();

	FireError();

	return mStrVal.c_str();
}





//////////////////////////////////////////////////////////////////////////



dbIData&	dbField::operator = ( fbyte cOp )
{
	if ( UNSIGNED_FLAG & mInfo.Attr )
	{
		FireError();
	}

	mI64Val = cOp;
	mChanged = F_TRUE;

	return *this;
}

dbIData&	dbField::operator = ( fubyte ucOp )
{
	if ( !(UNSIGNED_FLAG & mInfo.Attr))
	{
		FireError();
	}

	mI64Val = ucOp;
	mChanged = F_TRUE;
	
	return *this;
}

dbIData&	dbField::operator = ( fint16 sOp )
{	
	if ( (UNSIGNED_FLAG & mInfo.Attr)
		|| mInfo.Type == FIELD_TYPE_TINY )
	{
		FireError();
	}

	mI64Val = sOp;
	mChanged = F_TRUE;
	
	return *this;
}

dbIData&	dbField::operator = ( fuint16 usOp )
{	
	if ( !(UNSIGNED_FLAG & mInfo.Attr)
		|| mInfo.Type == FIELD_TYPE_TINY )
	{
		FireError();
	}

	mI64Val = usOp;
	mChanged = F_TRUE;

	return *this;
}

//dbIData&	dbField::operator = ( flong lOp )
//{	
//	if ( (UNSIGNED_FLAG & mInfo.Attr)
//		|| mInfo.Type == FIELD_TYPE_TINY
//		|| mInfo.Type == FIELD_TYPE_SHORT )
//	{
//		FireError();
//	}
//
//	mI64Val = lOp;
//	mChanged = F_TRUE;
//
//	return *this;
//}
//
//dbIData&	dbField::operator = ( fulong ulOp )
//{
//	if ( !(UNSIGNED_FLAG & mInfo.Attr)
//		|| mInfo.Type == FIELD_TYPE_TINY
//		|| mInfo.Type == FIELD_TYPE_SHORT )
//	{
//		FireError();
//	}
//
//	mI64Val = ulOp;
//	mChanged = F_TRUE;
//
//	return *this;
//}

dbIData&	dbField::operator = ( fint32 iOp )
{	
	if ( (UNSIGNED_FLAG & mInfo.Attr)
		|| mInfo.Type == FIELD_TYPE_TINY
		|| mInfo.Type == FIELD_TYPE_SHORT )
	{
		FireError();
	}

	mI64Val = iOp;
	mChanged = F_TRUE;

	return *this;
}

dbIData&	dbField::operator = ( fuint32 uiOp )
{	
	if ( !(UNSIGNED_FLAG & mInfo.Attr)
		|| mInfo.Type == FIELD_TYPE_TINY
		|| mInfo.Type == FIELD_TYPE_SHORT )
	{
		FireError();
	}

	mI64Val = uiOp;
	mChanged = F_TRUE;
	
	return *this;
}

dbIData&	dbField::operator = ( fint64 i64Op )
{
	if ( (UNSIGNED_FLAG & mInfo.Attr)
		|| mInfo.Type == FIELD_TYPE_TINY
		|| mInfo.Type == FIELD_TYPE_SHORT
		|| mInfo.Type == FIELD_TYPE_LONG )
	{
		FireError();
	}

	mI64Val = i64Op;
	mChanged = F_TRUE;
	
	return *this;
}

dbIData&	dbField::operator = ( fuint64 ui64Op )
{
	if ( !(UNSIGNED_FLAG & mInfo.Attr)
		|| mInfo.Type == FIELD_TYPE_TINY
		|| mInfo.Type == FIELD_TYPE_SHORT
		|| mInfo.Type == FIELD_TYPE_LONG )
	{
		FireError();
	}

	mI64Val = ui64Op;
	mChanged = F_TRUE;

	return *this;
}

dbIData&	dbField::operator = ( freal32 fOp )
{
	if ( mInfo.Type != FIELD_TYPE_FLOAT
		|| mInfo.Type != FIELD_TYPE_DOUBLE ) 
	{
		FireError();
	}

	mDVal = fOp;
	mChanged = F_TRUE;

	return *this;
}

dbIData&	dbField::operator = ( freal64 dbOp )
{
	if ( mInfo.Type != FIELD_TYPE_DOUBLE ) 
	{
		FireError();
	}

	mDVal = dbOp;
	mChanged = F_TRUE;

	return *this;
}

dbIData&	dbField::operator = (const char* pszVal)
{
	if ( mInfo.Type != FIELD_TYPE_STRING 
		&& mInfo.Type != FIELD_TYPE_VAR_STRING )
	{
		FireError();
	}

	mStrVal = pszVal;
	mChanged = F_TRUE;

	return *this;
}

//////////////////////////////////////////////////////////////////

fbyte*		dbField::GetValue()
{
	return mValue;
}

fbool		dbField::SetValue( const fbyte* pszValue )
{
	switch ( mInfo.Type )
	{	
	case FIELD_TYPE_TINY:
	case FIELD_TYPE_SHORT:
	case FIELD_TYPE_LONG:
	case FIELD_TYPE_LONGLONG:
#ifdef WIN32
		mI64Val = pszValue ? _atoi64(pszValue) : 0;
#else
		mI64Val = pszValue ? atoll(pszValue) : 0;
#endif
		
		break;

	case FIELD_TYPE_FLOAT:
	case FIELD_TYPE_DOUBLE:
		mDVal = pszValue ?  atof(pszValue) : 0.0f;
		break;

	case FIELD_TYPE_STRING:
	case FIELD_TYPE_VAR_STRING:
	case MYSQL_TYPE_TINY_BLOB:
	case MYSQL_TYPE_MEDIUM_BLOB:
	case MYSQL_TYPE_LONG_BLOB:
	case MYSQL_TYPE_BLOB:
		
		MEMSET( mValue , 0 , mInfo.MaxLen );

		if ( pszValue )
		{
			mStrVal = pszValue;
			MEMCPY( mValue , pszValue , mInfo.RealLen );
		}
		else
		{
			mStrVal = "";
		}
		
		break;
	default:
		//logManager::Print( "ERROR: dbField::SetValue unknow field type:%u", mInfo.Type );
		return F_FALSE;
	}

	mChanged = F_TRUE;

	return F_TRUE;
}

