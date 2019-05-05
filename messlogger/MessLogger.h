#pragma once

#include <windows.h>
#include <stdio.h>
#include <io.h>

class LogLocker;
extern LogLocker *g_InstanceMutex;

class LogLocker
{
public:
	LogLocker() {
		m_mutex = ::CreateMutex(
			NULL,              // default security attributes
			FALSE,             // initially not owned
			NULL);             // unnamed mutex
	}
	~LogLocker() {
		::CloseHandle(m_mutex);
	}

	bool TryLock() const {
		DWORD dwResult = ::WaitForSingleObject(m_mutex, DEFAULT_WAIT_TIMEOUT);
		switch (dwResult)
		{
		case WAIT_OBJECT_0:
			return true;
			break;
		case WAIT_TIMEOUT:
			printf("WAIT_TIMEOUT");
			break;
		case WAIT_ABANDONED:
			printf("WAIT_ABANDONED");
			break;
		default:
			printf("WaitForSingleObject failed, error: %d.", ::GetLastError());
			break;
		}

		return false;
	}
	void UnLock() const {
		::ReleaseMutex(m_mutex);
	}
private:
	HANDLE m_mutex;
	const int DEFAULT_WAIT_TIMEOUT = 2000;	// 默认超时时间2s
};

class MessLogger
{
public:
	/******** Logger Level ********/
	enum LogLevel
	{
		LEVEL_ALL = 0,
		LEVEL_TRACE = 1,
		LEVEL_DEBUG = 2,
		LEVEL_INFO = 3,
		LEVEL_WARN = 4,
		LEVEL_ERROR = 5,
		LEVEL_FATAL = 6,
		LEVEL_OFF = 7
	};

public:
	MessLogger(void);
	~MessLogger(void);

	bool Init(unsigned int uiMaxKeepDays = DEFAULT_KEY_DAYS,		// 日志保存时间
						char* szLogFilePath = NULL,					// 日志文件路径, 为空表示默认 logs/log_%date.log
						LogLevel level = DEFAULT_LOGGER_LEVEL,		// 日志级别
						bool bEnableLogThreadID = true				// 打印线程ID
						);
	bool UnInit(void);

public:	
	static MessLogger* GetInstance();
	void Log(LogLevel level, char* szFile, char* szFunc, int iLine, char* format, ...);		// 写入格式化的字符串，换行，带锁
	void LogLineBreak();	// 写换行符，带锁

private:
	// 写日志的一些功能函数
	void LogPure(char* msg);							// 写字符串，带锁
	void LogVaList(char* format, va_list argptr);		// 写入va_list
	void LogFormat(char* format, ...);		// 写格式化的字符串
	void LogDateTime();						// 写日期和时间
	void LogLogLevel(LogLevel level);		// 写日志级别
	void LogThreadID();						// 写线程ID

	const char* LogLevelToString(LogLevel level);
	SYSTEMTIME GetSystemDateTime();
	char *StrReplace(char *in, char *out, int outsize, const char *src, const char *dst);
	void CurDateToString(char *datestr);
	void CheckLogFile();
	int GetDaysSinceFileCreate(_finddata_t fileinfo);	

private:
	static MessLogger *_instance;
	//	把复制构造函数和=操作符设为私有,防止被复制
	MessLogger(const MessLogger&) {}
	MessLogger& operator=(const MessLogger&) {}

	LogLevel m_logLevel;
	FILE *m_pFile;
	bool m_bInited;
	LogLocker m_logMutex;

	static const LogLevel DEFAULT_LOGGER_LEVEL =
#ifdef _DEBUG
		LogLevel::LEVEL_DEBUG;
#else
		LogLevel::LEVEL_INFO;
#endif // _DEBUG
	static const int DEFAULT_KEY_DAYS = 30;	// 0 表示不删除日志文件
	const char * DEFAULT_LOGGER_FORMAT = "\\logs\\log_%date.log";

	// 配置相关参数
	unsigned int m_uiMaxKeepDays;			// 日志保留时间
	char m_szLogFilePath[MAX_PATH];			// 日志文路径
	bool m_bLogThreadID;					// 是否打印线程ID
	char m_szLogFilenameFormat[MAX_PATH];	// 日志名称格式
	char m_szLogFileDirectory[MAX_PATH];	// 日志目录
};

// 此宏定义保证取 MessLogger 实例时线程安全
#define ENABLE_THREAD_SAFE 	extern LogLocker *g_InstanceMutex = new LogLocker();

// 从 __FILE__ 全路径中取出文件名
#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)

// 写入换行符
#define  LOG_LINE_BREAK() MessLogger::GetInstance()->LogLineBreak();

// 写入格式化的字符串
#define TLOG(...)	\
	MessLogger::GetInstance()->Log(MessLogger::GetInstance()->LEVEL_TRACE, __FILENAME__, __FUNCTION__, __LINE__, __VA_ARGS__); 
#define DLOG(...)	\
	MessLogger::GetInstance()->Log(MessLogger::GetInstance()->LEVEL_DEBUG, __FILENAME__, __FUNCTION__, __LINE__, __VA_ARGS__); 
#define ILOG(...)	\
	MessLogger::GetInstance()->Log(MessLogger::GetInstance()->LEVEL_INFO, __FILENAME__, __FUNCTION__, __LINE__, __VA_ARGS__); 
#define WLOG(...)	\
	MessLogger::GetInstance()->Log(MessLogger::GetInstance()->LEVEL_WARN, __FILENAME__, __FUNCTION__, __LINE__, __VA_ARGS__); 
#define ELOG(...)	\
	MessLogger::GetInstance()->Log(MessLogger::GetInstance()->LEVEL_ERROR, __FILENAME__, __FUNCTION__, __LINE__, __VA_ARGS__); 
#define FLOG(...)	\
	MessLogger::GetInstance()->Log(MessLogger::GetInstance()->LEVEL_FATAL, __FILENAME__, __FUNCTION__, __LINE__, __VA_ARGS__); 


