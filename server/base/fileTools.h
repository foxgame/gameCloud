
#ifndef _FILETOOLS_H_
#define _FILETOOLS_H_

#include "baseDefine.h"

typedef vector<string> fileVector;
typedef vector<string>::iterator fileVectorIter;

class fileTools
{
public:
    
    enum traverse_order_t
    {
        DEPTH_FIRST = 1,
        BREADTH_FIRST = 2,
    };
    
    enum 
	{ 
		UNLIMITED_DEPTH = -1 
	};
    
    static bool getSubFiles( const std::string& path, fileVector& files, int max_depth = UNLIMITED_DEPTH, bool include_sub_dirs = false, traverse_order_t order = BREADTH_FIRST );
    
};

#endif
