#pragma once
///////////////////////////////////////////////////////
//	***             * * * *      ****        ****    //
//	***  	      *         *     **          **     //
//	***  	     *           *    **          **     //
//	***  	     *           *    **************     //
//	***	     **  *           *    **          **     //
//	***********   *         *     **          **     //
//	***********	    * * * *      ****        ****    // 
///////////////////////////////////////////////////////2009-08-20 @loach
/*
*
* @author loach.h
*
* @date 2009-08-20
*/

#include "qyutil/qyconfig.h"

DEFINE_NAMESPACE(qy)
/**
 * @author loach
 *
 * @date 2007-09-10
*/
class QYUTIL_API QyFile
{
public:
	bool fileExists(const char* filename);
	bool isFileExists(const char* filename);

	bool isPathExists(const char* filename);

	/**
	 * @return 0 dir , 1 file , other 
	*/
	int  isPathFile(const char* filename);

	size_t getFileSize(const char* filename);
};

END_NAMESPACE(qy)