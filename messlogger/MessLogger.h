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
	const int DEFAULT_WAIT_TIMEOUT = 2000;	// Ĭ�ϳ�ʱʱ��2s
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

	bool Init(unsigned int uiMaxKeepDays = DEFAULT_KEY_DAYS,		// ��־����ʱ��
						char* szLogFilePath = NULL,					// ��־�ļ�·��, Ϊ�ձ�ʾĬ�� logs/log_%date.log
						LogLevel level = DEFAULT_LOGGER_LEVEL,		// ��־����
						bool bEnableLogThreadID = true				// ��ӡ�߳�ID
						);
	bool UnInit(void);

public:	
	static MessLogger* GetInstance();
	void Log(LogLevel level, char* szFile, char* szFunc, int iLine, char* format, ...);		// д���ʽ�����ַ��������У�����
	void LogLineBreak();	// д���з�������

private:
	// д��־��һЩ���ܺ���
	void LogPure(char* msg);							// д�ַ���������
	void LogVaList(char* format, va_list argptr);		// д��va_list
	void LogFormat(char* format, ...);		// д��ʽ�����ַ���
	void LogDateTime();						// д���ں�ʱ��
	void LogLogLevel(LogLevel level);		// д��־����
	void LogThreadID();						// д�߳�ID

	const char* LogLevelToString(LogLevel level);
	SYSTEMTIME GetSystemDateTime();
	char *StrReplace(char *in, char *out, int outsize, const char *src, const char *dst);
	void CurDateToString(char *datestr);
	void CheckLogFile();
	int GetDaysSinceFileCreate(_finddata_t fileinfo);	

private:
	static MessLogger *_instance;
	//	�Ѹ��ƹ��캯����=��������Ϊ˽��,��ֹ������
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
	static const int DEFAULT_KEY_DAYS = 30;	// 0 ��ʾ��ɾ����־�ļ�
	const char * DEFAULT_LOGGER_FORMAT = "\\logs\\log_%date.log";

	// ������ز���
	unsigned int m_uiMaxKeepDays;			// ��־����ʱ��
	char m_szLogFilePath[MAX_PATH];			// ��־��·��
	bool m_bLogThreadID;					// �Ƿ��ӡ�߳�ID
	char m_szLogFilenameFormat[MAX_PATH];	// ��־���Ƹ�ʽ
	char m_szLogFileDirectory[MAX_PATH];	// ��־Ŀ¼
};

// �˺궨�屣֤ȡ MessLogger ʵ��ʱ�̰߳�ȫ
#define ENABLE_THREAD_SAFE 	extern LogLocker *g_InstanceMutex = new LogLocker();

// �� __FILE__ ȫ·����ȡ���ļ���
#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)

// д�뻻�з�
#define  LOG_LINE_BREAK() MessLogger::GetInstance()->LogLineBreak();

// д���ʽ�����ַ���
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


