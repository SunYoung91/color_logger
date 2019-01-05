#include <stdarg.h>
#include <stdio.h>
#include "logger.h"
#include <mutex>
#include <time.h>
#ifndef linux
#include <iomanip>
#include <windows.h>
#endif

#define GET_YEAR now.tm_year + 1900
#define GET_MON now.tm_mon + 1

#define LOG_TO(error_level) \
va_list vp;\
if (_log_level > error_level) return;\
va_start(vp, pattern);\
logva(error_level, pattern, vp);\
va_end(vp);\



#define LOCK_GUARD()  std::lock_guard<std::mutex> lock_guard(_write_mutex)
namespace kadin {


	Logger::Logger(const char* name)
	{
		_console = stdout;
		_file = NULL;

		_name = name;
		_file_name = "";
		_log_level = LEVEL_ALL;
		_log_day = 0;

#ifndef linux
		_console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
#endif
	}

	Logger::~Logger()
	{
		if (NULL != _file)
		{
			fclose(_file);
		}
	}

	void Logger::stopWriteToConsole()
	{
		LOCK_GUARD();
		_console = NULL;
	}

	void Logger::setFileName(const std::string &file)
	{
		LOCK_GUARD();
		_log_day = 0;
		_file_name = file;
	}


	void Logger::setLevel(const log_level level)
	{
		LOCK_GUARD();
		_log_level = level;
	}

	void Logger::setLevel(const std::string &level)
	{
		if ("off" == level) setLevel(LEVEL_OFF);
		else if ("fatal" == level) setLevel(LEVEL_FATAL);
		else if ("error" == level) setLevel(LEVEL_ERROR);
		else if ("warn" == level) setLevel(LEVEL_WARN);
		else if ("info" == level) setLevel(LEVEL_INFO);
		else if ("trace" == level) setLevel(LEVEL_TRACE);
		else if ("debug" == level) setLevel(LEVEL_DEBUG);
		else if ("all" == level) setLevel(LEVEL_ALL);
	}

	void Logger::logva(const log_level level, const char * pattern, va_list vp)
	{
		char   szName[kMAX_PATH_LEN] = { 0 };

		if (_log_level > level)
		{
			return;
		}

		struct tm now;
		time_t now_t = time(0);
#ifdef linux
		localtime_r(&now_t, &now);
#else
		localtime_s(&now, &now_t);
#endif

		LOCK_GUARD();

		va_list vp2;
		va_copy(vp2, vp);
		if (!_file_name.empty())
		{
			if (_log_day != now.tm_mday)
			{
				if (NULL != _file)
				{
					fclose(_file);
					_file = NULL;
				}
				_log_day = now.tm_mday;
				if (_file_name.size() > 4 && _file_name.substr(_file_name.size() - 4) != ".log")
				{
					_file_name += ".log";
				}

				snprintf(szName, sizeof(szName) - 1, "%s_%04d%02d%02d.log", _file_name.substr(0, _file_name.size() - 4).c_str(), GET_YEAR,
					GET_MON, now.tm_mday);
#ifdef linux
				_file = fopen(szName, "at");
#else
				fopen_s(&_file, szName, "at");

#endif
			}
		}

#ifdef linux
		std::string color = "\033[0m[";

		switch (level)
		{
		case LEVEL_DEBUG: color = "\033[32m["; break;
		case LEVEL_INFO: color = "\033[33m["; break;
		case LEVEL_TRACE: color = "\033[36m["; break;
		case LEVEL_WARN: color = "\033[35m["; break;
		case LEVEL_ERROR: color = "\033[31m["; break;
		case LEVEL_FATAL: color = "\033[31m["; break;
		default:break;
		}
		color += _name;
#else
		unsigned short console_color;
		switch (level)
		{
		case LEVEL_DEBUG: console_color = FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY; break;
		case LEVEL_INFO: console_color = FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY; break;
		case LEVEL_TRACE: console_color = FOREGROUND_GREEN | FOREGROUND_INTENSITY; break;
		case LEVEL_WARN: console_color = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY; break;
		case LEVEL_ERROR: console_color = FOREGROUND_RED | FOREGROUND_INTENSITY; break;
		case LEVEL_FATAL: console_color = FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_RED | BACKGROUND_RED | BACKGROUND_INTENSITY;   break;
		default:
			console_color = _console_color;
		}

		if (_console_handle != INVALID_HANDLE_VALUE && console_color != _console_color) {
			SetConsoleTextAttribute(_console_handle, console_color);
		}

		_console_color = console_color;

		std::string color = "[" + _name;
#endif

		color += "]";

		char fmt[64] = { 0 };
		snprintf(fmt, sizeof(fmt) - 1, "%04d-%02d-%02d %02d:%02d:%02d", GET_YEAR, GET_MON, now.tm_mday, now.tm_hour, now.tm_min, now.tm_sec);
		color += fmt;

		if (NULL != _console)
		{
			fprintf(_console, "%s", color.c_str());
			vfprintf(_console, pattern, vp);
			fprintf(_console, "\n");
			fflush(_console);
		}

		if (NULL != _file)
		{
			fprintf(_file, "%s", color.c_str());
			vfprintf(_file, pattern, vp2);
			fprintf(_file, "\n");
			fflush(_file);
		}
		va_end(vp2);
	}


	void Logger::log(const log_level level, const char * pattern, ...)
	{
		LOG_TO(level);
	}

	void Logger::fatal(const char * pattern, ...) {

		LOG_TO(LEVEL_FATAL);
	}


	void Logger::error(const char * pattern, ...)
	{
		LOG_TO(LEVEL_ERROR);
	}


	void Logger::warn(const char * pattern, ...)
	{
		LOG_TO(LEVEL_WARN);
	}


	void Logger::info(const char * pattern, ...)
	{
		LOG_TO(LEVEL_INFO);
	}

	void Logger::trace(const char * pattern, ...)
	{
		LOG_TO(LEVEL_TRACE);
	}

	void Logger::debug(const char * pattern, ...)
	{
		LOG_TO(LEVEL_DEBUG);
	}

};