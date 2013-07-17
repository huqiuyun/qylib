#ifndef __QY_ILOG_H__
#define __QY_ILOG_H__

struct IQyLog
{
	typedef enum 
	{
		Cout = 1,
		Cerr = 0x02,
		File = 0x08,
		FileAppend = 0x18,
		UnicodeFile= 0x20,
		VSDebugWindow = 0x40,   // Use only for Visual Studio Debug Window logging - WIN32 must be defined
		Internet = 0x80, // report to server
		All = 0xFFFF,
	} LogType;

	typedef enum
	{
		Unknown = -1,
		Crit = 2,
		Err = 3,
		Warning = 4,
		Info = 6,
		Debug = 7,
		Stack = 8,
		StdErr = 9,
		Bogus = 666
	} LogLevel;

	typedef enum
	{
		Crit_flag = 0x01,
		Err_flag = 0x02,
		Warning_flag = 0x04,
		Info_flag = 0x08,
		Debug_flag = 0x10,
		Stack_flag = 0x20,
		StdErr_flag = 0x40,
		Bogus_flag = 0x80,
		Unknown_flags = 0x100,
		LL_All = 0xFFFF,
	}LogLevelFlags;

	virtual void set_log_file(const char* filename) = 0;
	virtual void enable_type( int type) = 0;
	virtual void enable_level(int levelflag /*LogLevelFlags*/) = 0;
	virtual bool logBuffer(int type , int level , const char* buffer,size_t len) = 0;
	virtual bool logString(int type , int level , const char* buffer) = 0;
	virtual bool logFormat(int type , int level , const char* pszFmt, ...) = 0;
};


#endif /*__QY_ILOG_H__ */