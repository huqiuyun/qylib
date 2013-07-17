#ifndef __QY_PARSER_H__
#define __QY_PARSER_H__

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
* 此库可以由读者用于学习之用,不能用于商业用途
* 如果用在商业用途,一块法律追究自己承担!
*
* 作者： loach.h(胡秋云)
*
* 时间： 2009-09-21
*/

#include "qyutil/qyconfig.h"

class QyParserA;
class QyParserW;

#ifdef UNICODE
typedef QyParserW QyParser;
#else
typedef QyParserA QyParser;
#endif

// declare CParserW_qy, et. al.
#include "qyutil/qyparser-template-def-unichar.h"
#include "qyutil/qyparserT.h"
#include "qyutil/qyparser-template-undef.h"

  // declare CParserA_qy, et. al.
#include "qyutil/qyparser-template-def-char.h"
#include "qyutil/qyparserT.h"
#include "qyutil/qyparser-template-undef.h"



#endif /* __QY_PARSER_H__ */
