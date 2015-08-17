#ifndef _MATHBASE_H_
#define _MATHBASE_H_


#include "baseDefine.h"


#include <math.h>


namespace FOXSDK
{
	const freal32 PI = 3.14159265359f;

	const freal32 PI2 = 3.14159265359f * 2.0f;

	//! Constant for reciprocal of PI.
	const freal32 RECIPROCAL_PI	= 1.0f/PI;

	//! Constant for half of PI.
	const freal32 HALF_PI = PI/2.0f;

	const freal64 PI64 = 3.1415926535897932384626433832795028841971693993751;

	//! Constant for 64bit reciprocal of PI.
	const freal64 RECIPROCAL_PI64 = 1.0/PI64;

	//! 32bit Constant for converting from degrees to radians
	const freal32 DEGTORAD = PI / 180.0f;

	//! 32bit constant for converting from radians to degrees (formally known as GRAD_PI)
	const freal32 RADTODEG = 180.0f / PI;

	//! 64bit constant for converting from degrees to radians (formally known as GRAD_PI2)
	const freal64 DEGTORAD64 = PI64 / 180.0;

 	enum MathQuadSplitMode
 	{
 		// Diagonal goes from top-left to bottom-right
 		Math_TopLeftToBottomRight,	
 
 		// Diagonal goes from bottom-left to top-right
 		Math_BottomLeftToTopRight
 	};
 
 
 	enum MathPixelFormat
 	{
 		// Each pixel is 3 bytes. RGB in that order.
 		MPF_RGB,
 		// Each pixel is 4 bytes. RGBA in that order.
 		MPF_RGBA
 	};

	class MathVector3;

	struct MathBase 
	{
		static map< fint32 , freal32 > mCosMap;
		static map< fint32 , freal32 > mSinMap;

		static fvoid		Init()
		{
			for ( fint32 i = -360 ; i <= 360 ; i++ )
			{
				mCosMap[i] = (freal32)cos( DEGTORAD64 * i );
				mSinMap[i] = (freal32)sin( DEGTORAD64 * i );
			}
		}

		static freal32		Sin( freal64 degrees )
		{
			return mSinMap[ (fint32)degrees ];
		}

		static freal32		Cos( freal64 degrees )
		{
			return mCosMap[ (fint32)degrees ];
		}

		static fint32		GetRandomInt( fint32 min , fint32 max )
		{
			if ( max == min )
			{
				return max;
			}

			return rand() % ( max - min ) + min;
		}

		static freal32		GetRandomFloat( freal32 lowBound , freal32 highBound )
		{
			if( lowBound >= highBound ) // bad input
				return lowBound;

			// get random float in [0, 1] interval
			float f = ( rand() % 10000 ) * 0.0001f; 

			// return float in [lowBound, highBound] interval. 
			return ( f * (highBound - lowBound) ) + lowBound; 
		}

		static fulong		FtoDw( freal32 f )
		{
			return *((fulong*)&f);
		}

//		static fvoid		GetRandomVector(
//			MathVector3* out ,
//			MathVector3* min ,
//			MathVector3* max );
	};

	//! position 
	struct MathPos
	{
		freal32 x , y;

		MathPos(): 
		x(0) , y(0)
		{}

		MathPos( freal32 x , freal32 y )
		{
			this->x = x;
			this->y = y;
		}

		MathPos& operator+=(const MathPos& other) { x+=other.x; y+=other.y; return *this; }
		MathPos& operator-=(const MathPos& other) { x-=other.x; y-=other.y; return *this; }

	};

	//! vector three
	class MathVector3
	{
	public:

		MathVector3():
		x( 0.0f ) , y( 0.0f ) , z( 0.0f )
		{}

		MathVector3( freal32 fx, freal32 fy, freal32 fz ):
		x(fx), y(fy), z(fz)
		{}

		freal32 x , y , z;

		MathVector3 operator+(const MathVector3& other) const { return MathVector3(x + other.x, y + other.y, z + other.z); }
		MathVector3& operator+=(const MathVector3& other) { x+=other.x; y+=other.y; z+=other.z; return *this; }
		MathVector3 operator+(const freal32 val) const { return MathVector3(x + val, y + val, z + val); }
		MathVector3& operator+=(const freal32 val) { x+=val; y+=val; z+=val; return *this; }

		MathVector3 operator-(const MathVector3& other) const { return MathVector3(x - other.x, y - other.y, z - other.z); }
		MathVector3& operator-=(const MathVector3& other) { x-=other.x; y-=other.y; z-=other.z; return *this; }
		MathVector3 operator-(const freal32 val) const { return MathVector3(x - val, y - val, z - val); }
		MathVector3& operator-=(const freal32 val) { x-=val; y-=val; z-=val; return *this; }

		MathVector3 operator*(const MathVector3& other) const { return MathVector3(x * other.x, y * other.y, z * other.z); }
		MathVector3& operator*=(const MathVector3& other) { x*=other.x; y*=other.y; z*=other.z; return *this; }
		MathVector3 operator*(const freal32 v) const { return MathVector3(x * v, y * v, z * v); }
		MathVector3& operator*=(const freal32 v) { x*=v; y*=v; z*=v; return *this; }

		MathVector3 operator/(const MathVector3& other) const { return MathVector3(x / other.x, y / other.y, z / other.z); }
		MathVector3& operator/=(const MathVector3& other) { x/=other.x; y/=other.y; z/=other.z; return *this; }
		MathVector3 operator/(const freal32 v) const { freal32 i=1.0f/v; return MathVector3(x * i, y * i, z * i); }
		MathVector3& operator/=(const freal32 v) { freal32 i=1.0f/v; x*=i; y*=i; z*=i; return *this; }

		fvoid	Set( freal32 fx, freal32 fy, freal32 fz )
		{
		  x = fx;
		  y = fy;
		  z = fz;
		}

		fvoid	RotateXYBy( freal64 degrees, const MathVector3& center )
		{
		  freal32 cs = MathBase::Cos( degrees );
		  freal32 sn = MathBase::Sin( degrees );

		  x -= center.x;
		  y -= center.y;

		  Set( (freal32)(x * cs - y * sn) , (freal32)(x * sn + y * cs) , (freal32)z );

		  x += center.x;
		  y += center.y;
		}

		fvoid	RotateYZBy( freal64 degrees, const MathVector3& center )
		{
		  freal32 cs = MathBase::Cos( degrees );
		  freal32 sn = MathBase::Sin( degrees );

		  z -= center.z;
		  y -= center.y;

		  Set( (freal32)x , (freal32)(y * cs - z * sn) , (freal32)(y * sn + z * cs) );

		  z += center.z;
		  y += center.y;
		}

		fvoid	RotateXZBy( freal64 degrees, const MathVector3& center )
		{
		  freal64 cs = MathBase::Cos( degrees );
		  freal64 sn = MathBase::Sin( degrees );

		  x -= center.x;
		  z -= center.z;

		  Set( (freal32)(x * cs - z * sn) , (freal32)y , (freal32)(x * sn + z * cs) );

		  x += center.x;
		  z += center.z;
		}

		MathVector3 CrossProduct( const MathVector3& p ) const
		{
		  return MathVector3( y * p.z - z * p.y, z * p.x - x * p.z, x * p.y - y * p.x );
		}

		MathVector3& Normalize()
		{
		  freal32 length = (freal32)( x*x + y*y + z*z );
		  if ( Equals( length , 0.f ) )
			  return *this;

		  length = 1.f / sqrtf ( length );
		  x = x * length;
		  y = y * length;
		  z = z * length;

		  return *this;
		}

		fbool Equals( const freal32 a, const freal32 b, const freal32 tolerance = 0.000001f )
		{
		return (a + tolerance >= b) && (a - tolerance <= b);
		}

// 		fbool Equals( const MathVector3& other , const freal32 tolerance = 0.000001f ) const
// 		{
// 		return Equals( x , other.x , tolerance ) &&
// 			Equals( y , other.y , tolerance ) &&
// 			Equals( z, other.z , tolerance );
// 		}


	};


	//! Rectangle
	struct MathRect
	{
		MathRect(): 
		top(0) , bottom(0) , left(0) , right(0)
		{}

		MathRect( freal32 left , freal32 top , freal32 right , freal32 bottom )
		{
			this->left = left;
			this->top = top;
			this->right = right;
			this->bottom = bottom;
		}

		freal32 top , bottom , left , right;

		MathRect operator+(const MathRect& other) const { return MathRect( left + other.left, top + other.top, right + other.right, bottom + other.bottom ); }
		MathRect& operator+=(const MathRect& other) { top+=other.top; bottom+=other.bottom; left+=other.left; right+=other.right; return *this; }


		fvoid			Clear()
		{
			top = 0.0f;
			bottom = 0.0f;
			left = 0.0f;
			right = 0.0f;
		}


		freal32			Width() const
		{
			return right - left;
		}


		freal32			Height() const
		{
			return bottom - top;
		}


		MathRect		GetIntersection( const MathRect& rect ) const
		{
			//! check for total exclusion
			if ((right > rect.left) &&
				(left < rect.right) &&
				(bottom > rect.top) &&
				(top < rect.bottom))
			{
				MathRect temp;

				//! fill in temp with the intersection
				temp.left = (left > rect.left) ? left : rect.left;
				temp.right = (right < rect.right) ? right : rect.right;
				temp.top = (top > rect.top) ? top : rect.top;
				temp.bottom = (bottom < rect.bottom) ? bottom : rect.bottom;

				return temp;
			}
			else
			{
				return MathRect( 0.0f , 0.0f , 0.0f , 0.0f );
			}

		}

		fvoid			Offset( freal32 x , freal32 y )
		{
			this->left += x;
			this->right += x;
			this->top += y;
			this->bottom += y;
		}

		fbool			IsPointIn( freal32 x , freal32 y )
		{
			return ( x >= left && y >= top && x <= right && y <= bottom );
		}

		fbool			IsPointIn( MathPos& pos )
		{
			return ( pos.x >= left && pos.y >= top && pos.x <= right && pos.y <= bottom );
		}

	};


	//! Ray
	struct MathRay
	{
		MathRay()
		{

		}

		MathVector3 origin;
		MathVector3 direction;
	};

	//! Mouse
	struct MathMouse
	{
		MathMouse():
		Left( F_FALSE ) , Right( F_FALSE ) , Middle( F_FALSE ) ,
		Wheel( 0 )
	{

	}

		fbool			Left , Right , Middle;
		
		fint32			Wheel;

		MathPos			Position;
	};




};



#endif



