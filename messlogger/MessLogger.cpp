#include "MessLogger.h"
#include <Shlwapi.h>    
#include <shlobj.h>
#include <stdio.h>
#include <share.h>
#include <time.h>
#include <io.h>

#pragma comment(lib, "shlwapi.lib")

MessLogger* MessLogger::_instance = 0;
extern LogLocker *g_InstanceMutex;

MessLogger::MessLogger()
	:m_logLevel(DEFAULT_LOGGER_LEVEL),
	m_bInited(false),
	m_bLogThreadID(false),
	m_uiMaxKeepDays(0)
{
	memset(m_szLogFilePath, 0, MAX_PATH);
	memset(m_szLogFilenameFormat, 0, MAX_PATH);
	memset(m_szLogFileDirectory, 0, MAX_PATH);
}

MessLogger::~MessLogger()
{
	if (m_bInited) {
		UnInit();
	}
}

bool MessLogger::Init(unsigned int uiMaxKeepDays /*= DEFAULT_KEY_DAYS*/, /* 日志保存时间 */ 
	char* szLogFilePath /*= NULL*/, /* 日志文件路径 */ 
	LogLevel level /*= DEFAULT_LOGGER_LEVEL*/, /* 日志级别 */
	bool bEnableLogThreadID /*= true 打印线程ID*/)
{
	if (!m_logMutex.TryLock()) {
		printf("TryLock failed.");
		return false;
	}
	
	bool result = false;
	if (m_bInited)
		goto END;

	m_uiMaxKeepDays = uiMaxKeepDays;
	m_logLevel = level <= LogLevel::LEVEL_ALL ? LogLevel::LEVEL_ALL : (level >= LogLevel::LEVEL_OFF ? LogLevel::LEVEL_OFF : level);
	m_bLogThreadID = bEnableLogThreadID;
	char szFilePath[MAX_PATH] = { 0 };
	char szRelativePath[MAX_PATH] = { 0 };	
	char szDateString[10] = { 0 };	

	// 日志路径
	if (NULL == szLogFilePath) {
		GetModuleFileNameA(NULL, szFilePath, MAX_PATH);
		PathRemoveFileSpecA(szFilePath);	// 去掉文件名获取路径
		strcat_s(szFilePath, DEFAULT_LOGGER_FORMAT);	// 默认文件路径
	}
	else {
		if (szLogFilePath[0] == '\\' || szLogFilePath[0] == '/') {	// 相对路径
			GetModuleFileNameA(NULL, szFilePath, MAX_PATH);
			PathRemoveFileSpecA(szFilePath);	// 去掉文件名获取路径
			strcat_s(szFilePath, szLogFilePath);
		}
		else if (szLogFilePath[1] == ':') {	// 绝对路径
			strcpy_s(szFilePath, szLogFilePath);
		}
		else {
			printf("文件格式不合法。");
			goto END;
		}
	}

	// 获取文件名，也就是文件名格式
	strcpy_s(m_szLogFilenameFormat, PathFindFileNameA(szFilePath));
	// 查找 "%date" 的位置，并替换
	CurDateToString(szDateString);
	StrReplace(szFilePath, m_szLogFilePath, MAX_PATH, "%date", szDateString);
	strcpy_s(szFilePath, m_szLogFilePath);	// 保存日志文件路径

	PathRemoveFileSpecA(szFilePath);	// 去掉文件名获取路径
	strcpy_s(m_szLogFileDirectory, szFilePath);
	if (!PathFileExistsA(szFilePath)) {	// 文件不存在则创建
		// 创建目录
		int result = SHCreateDirectoryExA(NULL, szFilePath, NULL);
		if (ERROR_SUCCESS != result) {
			printf("文件夹创建失败。");
			goto END;
		}
	}
	else {	// 文件夹存在，清理文件夹
		if (m_uiMaxKeepDays > 0) {
			CheckLogFile();
		}
	}

	// 关闭日志
	if (LogLevel::LEVEL_OFF == level) 
		goto END;

	m_pFile = _fsopen(m_szLogFilePath, "a+", _SH_DENYNO);	// 最后一个参数表示共享方式打开文件
	if (m_pFile == NULL)
	{
		printf("文件打开失败。");
		goto END;
	}

	result = true;

END:
	m_bInited = true;
	m_logMutex.UnLock();
	return result;
}

bool MessLogger::UnInit(void)
{
	if (!m_logMutex.TryLock()) {
		printf("TryLock failed.");
		return false;
	}

	bool result = false;
	if (NULL == m_pFile) {
		printf("日志文件未打开。");
		goto END;
	}
	else {
		fclose(m_pFile);
		m_pFile = NULL;
	}
	
	result = true;
END:
	m_bInited = false;
	m_logMutex.UnLock();
	return true;
}

void MessLogger::LogPure(char* msg)
{
	if (!m_bInited)	{
		printf("MessLogger 未初始化。");
		return;
	}

	if (NULL == m_pFile) {
		printf("日志文件未打开。");
		return;
	}

	fprintf_s(m_pFile, "%s", msg);
	fflush(m_pFile);
}

void MessLogger::LogVaList(char* format, va_list argptr)
{
	char *msg = NULL;
	// 获取格式化字符串长度
	int vaSize = _vscprintf(format, argptr) + 1;

	msg = (char*)calloc(vaSize, sizeof(char));
	if (NULL == msg) {
		printf("申请内存失败。");
		goto END;
	}

	vaSize = vsprintf_s(msg, vaSize, format, argptr);

	if (vaSize < 0)	{
		printf("参数不合法。");
		goto END;
	}

	LogPure(msg);

END:
	if (msg) {
		free(msg);
		msg = NULL;
	}
}

void MessLogger::LogFormat(char* format, ...)
{
	va_list argptr;
	va_start(argptr, format);
	LogVaList(format, argptr);
	va_end(argptr);	//清空参数指针
}

void MessLogger::LogDateTime()
{
	SYSTEMTIME lpTime = GetSystemDateTime();
	LogFormat("[%d.%02d.%02d% 02d:%02d:%02d.%03d] ",
		lpTime.wYear,
		lpTime.wMonth,
		lpTime.wDay,
		lpTime.wHour,
		lpTime.wMinute,
		lpTime.wSecond,
		lpTime.wMilliseconds);
}

void MessLogger::LogLogLevel(LogLevel level)
{
	LogFormat("[% 5s] ", LogLevelToString(level));
}

void MessLogger::LogThreadID()
{
	LogFormat("[Tid:%.6X] ", GetCurrentThreadId());
}

void MessLogger::Log(LogLevel level, char* szFile, char* szFunc, int iLine, char* format, ...)
{
	if (level < m_logLevel)
		return;

	if (!m_logMutex.TryLock()) {
		printf("TryLock failed.");
		return;
	}

	LogDateTime();
	LogLogLevel(level);
	if (m_bLogThreadID)
		LogThreadID();

	va_list argptr;
	va_start(argptr, format);
	LogVaList(format, argptr);
	va_end(argptr);	//清空参数指针

	LogFormat(" (%s %s %d)\n", szFile, szFunc, iLine);

	m_logMutex.UnLock();
}

void MessLogger::LogLineBreak()
{
	if (!m_logMutex.TryLock()) {
		printf("TryLock failed.");
		return;
	}
	LogPure("\n");
	m_logMutex.UnLock();
}

const char* MessLogger::LogLevelToString(LogLevel level)
{
	if (level < LogLevel::LEVEL_ALL || level > LogLevel::LEVEL_OFF)
		return "UNKOWN";

	switch (level)
	{
	case MessLogger::LEVEL_ALL:
		return "ALL";
		break;
	case MessLogger::LEVEL_TRACE:
		return "TRACE";
		break;
	case MessLogger::LEVEL_DEBUG:
		return "DEBUG";
		break;
	case MessLogger::LEVEL_INFO:
		return "INFO";
		break;
	case MessLogger::LEVEL_WARN:
		return "WARN";
		break;
	case MessLogger::LEVEL_ERROR:
		return "ERROR";
		break;
	case MessLogger::LEVEL_FATAL:
		return "FATAL";
		break;
	case MessLogger::LEVEL_OFF:
		return "OFF";
		break;
	default:
		return "UNKOWN";
		break;
	}
}

SYSTEMTIME MessLogger::GetSystemDateTime()
{
	SYSTEMTIME lpTime = { 0 };
	GetLocalTime(&lpTime);
	return lpTime;
}

char * MessLogger::StrReplace(char *in, char *out, int outsize, const char *src, const char *dst)
{
	char *p = in;
	unsigned int  len = outsize - 1;

	// 检查参数合法性
	if ((NULL == src) || (NULL == dst) || (NULL == in) || (NULL == out))
		return NULL;

	if ((strcmp(in, "") == 0) || (strcmp(src, "") == 0))
		return NULL;

	if (outsize <= 0)
		return NULL;

	while ((*p != '\0') && (len > 0)) {
		if (strncmp(p, src, strlen(src)) != 0) {
			int n = strlen(out);

			out[n] = *p;
			out[n + 1] = '\0';

			p++;
			len--;
		}
		else {
			strcat_s(out, outsize, dst);
			p += strlen(src);
			len -= strlen(dst);
		}
	}

	return out;
}

void MessLogger::CurDateToString(char *datestr)
{
	// 获取当前系统时间
	SYSTEMTIME lpTime = GetSystemDateTime();
	sprintf_s(datestr, 10, "%d%02d%02d",
		lpTime.wYear,
		lpTime.wMonth,
		lpTime.wDay);
}

void MessLogger::CheckLogFile()
{
	// 以创建时间为准
	long handle = -1;                                   
	struct _finddata_t fileinfo;                   
	char szFilename[MAX_PATH] = { 0 };
	char szFormat[MAX_PATH] = { 0 };

	StrReplace(m_szLogFilenameFormat, szFormat, MAX_PATH, "%date", "*");
	sprintf_s(szFilename, "%s\\%s", m_szLogFileDirectory, szFormat);
	handle = _findfirst(szFilename, &fileinfo);  
	if (-1 == handle)
		goto END;
	else {
		if (GetDaysSinceFileCreate(fileinfo) >= (int)m_uiMaxKeepDays) {
			sprintf_s(szFilename, "%s\\%s", m_szLogFileDirectory, fileinfo.name);
			if (!DeleteFileA(szFilename)) {
				printf("文件删除失败。");
				goto END;
			}
		}
	}
	while (!_findnext(handle, &fileinfo))    
	{
		if (GetDaysSinceFileCreate(fileinfo) >= (int)m_uiMaxKeepDays) {
			sprintf_s(szFilename, "%s\\%s", m_szLogFileDirectory, fileinfo.name);
			if (!DeleteFileA(szFilename)) {
				printf("文件删除失败。");
				goto END;
			}
		}
	}

END:
	if (-1 != handle)
		_findclose(handle);    
}

int MessLogger::GetDaysSinceFileCreate(_finddata_t fileinfo)
{
	time_t createTime = fileinfo.time_create;
	time_t loctimer = time(NULL);
	return (int)(difftime(loctimer, createTime) / 24 / 3600);
}

MessLogger* MessLogger::GetInstance()
{
	if (NULL == _instance) {
		if (!g_InstanceMutex->TryLock()) {
			printf("TryLock failed.");
			return NULL;
		}
		if (NULL == _instance) {
			_instance = new MessLogger();
		}
		g_InstanceMutex->UnLock();
	}

	return _instance;
}