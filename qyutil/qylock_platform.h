#ifndef __QY_THREAD_PLATFORM_H__
#define __QY_THREAD_PLATFORM_H__

/**
* 
* @filebrief: 封装线程操作
* 
* @author: welbon
* 
* 
* @date: 2010-09-14
*/


#if defined(WIN32) || defined(_WIN32) || defined(WINCE)
#include "qylock_win.hxx"
#else //!//defined(WIN32) || defined(_WIN32)
#include "qylock_posix.hxx"
#endif


#endif //__QY_THREAD_PLATFORM_H__

