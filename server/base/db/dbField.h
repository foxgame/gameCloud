#ifndef _DBFIELD_H_
#define _DBFIELD_H_


#include "dbConfig.h"

#include <mysql.h>

namespace FOXSDK
{

	struct dbFieldInfo
	{
		string      Name;
		fuint32     Type;
		fuint32     Attr;
		fuint32		Len;
		fuint32		RealLen;
		fuint32		MaxLen;

		dbFieldInfo()
		: Type( 0 ) , Attr( 0 ) , Len( 0 ) , RealLen( 0 ) , MaxLen( 0 )
		{
		}
	};



	class dbField : public dbIData
	{
		friend class dbRecord;

		// Constructor & Destructor
	public:

		dbField( const dbFieldInfo& info );
		dbField( const dbField& field );
		virtual ~dbField();

		dbField& operator = ( const dbField& field );

	public:

		// Interface

		virtual fvoid				Release()
		{
			delete this;
		}

		virtual						operator fbyte() const;
		virtual						operator fubyte() const;

		virtual						operator fint16() const;
		virtual						operator fuint16() const;

//		virtual						operator flong() const;
//		virtual						operator fulong() const;

		virtual						operator fint32() const;
		virtual						operator fuint32() const;

		virtual						operator fint64() const;
		virtual						operator fuint64() const;

		virtual						operator freal32() const;
		virtual						operator freal64() const;

		virtual						operator fbyte*() const;
		virtual						operator const fbyte*() const;

		// overload assignment operator

		virtual dbIData&			operator = ( fbyte chOp );
		virtual dbIData&			operator = ( fubyte chOp );

		virtual dbIData&			operator = ( fint16 nOp );
		virtual dbIData&			operator = ( fuint16 unOp );

//		virtual dbIData&			operator = ( flong lOp );
//		virtual dbIData&			operator = ( fulong lOp );

		virtual dbIData&			operator = ( fint32 iOp );
		virtual dbIData&			operator = ( fuint32 uOp );

		virtual dbIData&			operator = ( fint64 uOp );
		virtual dbIData&			operator = ( fuint64 uOp );

		virtual dbIData&			operator = ( freal32 fltOp );
		virtual dbIData&			operator = ( freal64 dblOp );

		virtual dbIData&			operator = ( const fbyte* szVal );

		// Attributes
	public:

		const fbyte*				GetName() const
		{
			return mInfo.Name.c_str();
		}
		
		fuint32						GetType() const
		{
			return mInfo.Type;
		}

		fuint32						GetAttr() const
		{
			return mInfo.Attr;
		}

		fulong                      GetMaxLen() const
		{
			return mInfo.Len;
		}

		fulong                      GetRealLen() const
		{
			return mInfo.RealLen;
		}

		fbool						IsChanged() const
		{
			return mChanged;
		}

		fvoid						TagChanged( fbool bChange )
		{
			mChanged = bChange;
		}

		fbool						SetValue( const fbyte* pszValue );
		fbyte*						GetValue();


	private:

		static	fvoid				FireError();

		dbFieldInfo					mInfo;

		union
		{
			fint64		mI64Val;
			freal64		mDVal;
		};

		fbyte			mValue[ MAX_INT16 ];
		string			mStrVal;

		fbool			mChanged;
	};


};



#endif

