//
//  logManager.cpp
//  test
//
//  Created by fox on 13-1-9.
//
//

#include "logManager.h"

using namespace FOXSDK;

#ifdef DEBUG
fint32  FOXSDK::gLogLevel = (fint32)LOGLEVEL4;
#else
fint32  FOXSDK::gLogLevel = (fint32)LOGLEVEL4;
#endif // DEBUG
//mutex   gMutex;

fvoid   FOXSDK::log( fint32 level , const fbyte* str , ... )
{
    if ( level < gLogLevel )
    {
        return;
    }

    va_list va;
    static char sstr[255];
    static char sbuf[255];
    memset( sstr , 0 , 255 );
    memset( sbuf , 0 , 255 );

    va_start( va , str );
    vsprintf( sstr , str , va );
    va_end(va);

    printf( "log:level %d %s \n" , level , sstr );
}


fvoid   FOXSDK::logFile( fint32 level , const fbyte* str , ... )
{

}



