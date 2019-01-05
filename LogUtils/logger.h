#pragma once
/*****************************************************************
	create by 一路随云
	mail:531662161@qq.com
	 2019/01/05 11:15  参考网上开源征途日志的单元 zlogger 的实现 修改为C++11 支持 win linux 并且增加颜色区分效果
*/

#include <string>
#include <climits>
#include <mutex>
#ifndef linux
#include <windows.h>
#endif
namespace kadin{

class Logger
{
public:
	~Logger(void);
	Logger(const char* name = "UNNAMED");
public:
	typedef enum
	{
		LEVEL_OFF   = INT_MAX,
		LEVEL_FATAL = 60000,
		LEVEL_ERROR = 50000,
		LEVEL_WARN  = 40000,
		LEVEL_TRACE  = 30000,
		LEVEL_INFO  = 20000,
		LEVEL_DEBUG = 10000,
		LEVEL_ALL   = INT_MIN
	} log_level;

	static const int kMAX_PATH_LEN = 256;
	
	void setLevel(const std::string &level);
	void setFileName(const std::string &file);
	void stopWriteToConsole();
	void trace(const char * pattern, ...);
	void debug(const char * pattern, ...);
	void error(const char * pattern, ...);
	void info(const char * pattern, ...);
	void fatal(const char * pattern, ...);
	void warn(const char * pattern, ...);
	void logva(const log_level level,const char * pattern,va_list vp);
	void log(const log_level level,const char * pattern,...);
	void setLevel(const log_level level);
private:
	std::mutex _write_mutex;
	volatile log_level      _log_level;
	FILE *		_console;
	FILE *		_file;
	int         _log_day;
	std::string _name;
	std::string _file_name;
#ifndef linux
	HANDLE _console_handle;
	unsigned short _console_color = 0;
#endif
};

}