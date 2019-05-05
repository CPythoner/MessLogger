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

bool MessLogger::Init(unsigned int uiMaxKeepDays /*= DEFAULT_KEY_DAYS*/, /* ��־����ʱ�� */ 
	char* szLogFilePath /*= NULL*/, /* ��־�ļ�·�� */ 
	LogLevel level /*= DEFAULT_LOGGER_LEVEL*/, /* ��־���� */
	bool bEnableLogThreadID /*= true ��ӡ�߳�ID*/)
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

	// ��־·��
	if (NULL == szLogFilePath) {
		GetModuleFileNameA(NULL, szFilePath, MAX_PATH);
		PathRemoveFileSpecA(szFilePath);	// ȥ���ļ�����ȡ·��
		strcat_s(szFilePath, DEFAULT_LOGGER_FORMAT);	// Ĭ���ļ�·��
	}
	else {
		if (szLogFilePath[0] == '\\' || szLogFilePath[0] == '/') {	// ���·��
			GetModuleFileNameA(NULL, szFilePath, MAX_PATH);
			PathRemoveFileSpecA(szFilePath);	// ȥ���ļ�����ȡ·��
			strcat_s(szFilePath, szLogFilePath);
		}
		else if (szLogFilePath[1] == ':') {	// ����·��
			strcpy_s(szFilePath, szLogFilePath);
		}
		else {
			printf("�ļ���ʽ���Ϸ���");
			goto END;
		}
	}

	// ��ȡ�ļ�����Ҳ�����ļ�����ʽ
	strcpy_s(m_szLogFilenameFormat, PathFindFileNameA(szFilePath));
	// ���� "%date" ��λ�ã����滻
	CurDateToString(szDateString);
	StrReplace(szFilePath, m_szLogFilePath, MAX_PATH, "%date", szDateString);
	strcpy_s(szFilePath, m_szLogFilePath);	// ������־�ļ�·��

	PathRemoveFileSpecA(szFilePath);	// ȥ���ļ�����ȡ·��
	strcpy_s(m_szLogFileDirectory, szFilePath);
	if (!PathFileExistsA(szFilePath)) {	// �ļ��������򴴽�
		// ����Ŀ¼
		int result = SHCreateDirectoryExA(NULL, szFilePath, NULL);
		if (ERROR_SUCCESS != result) {
			printf("�ļ��д���ʧ�ܡ�");
			goto END;
		}
	}
	else {	// �ļ��д��ڣ������ļ���
		if (m_uiMaxKeepDays > 0) {
			CheckLogFile();
		}
	}

	// �ر���־
	if (LogLevel::LEVEL_OFF == level) 
		goto END;

	m_pFile = _fsopen(m_szLogFilePath, "a+", _SH_DENYNO);	// ���һ��������ʾ����ʽ���ļ�
	if (m_pFile == NULL)
	{
		printf("�ļ���ʧ�ܡ�");
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
		printf("��־�ļ�δ�򿪡�");
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
		printf("MessLogger δ��ʼ����");
		return;
	}

	if (NULL == m_pFile) {
		printf("��־�ļ�δ�򿪡�");
		return;
	}

	fprintf_s(m_pFile, "%s", msg);
	fflush(m_pFile);
}

void MessLogger::LogVaList(char* format, va_list argptr)
{
	char *msg = NULL;
	// ��ȡ��ʽ���ַ�������
	int vaSize = _vscprintf(format, argptr) + 1;

	msg = (char*)calloc(vaSize, sizeof(char));
	if (NULL == msg) {
		printf("�����ڴ�ʧ�ܡ�");
		goto END;
	}

	vaSize = vsprintf_s(msg, vaSize, format, argptr);

	if (vaSize < 0)	{
		printf("�������Ϸ���");
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
	va_end(argptr);	//��ղ���ָ��
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
	va_end(argptr);	//��ղ���ָ��

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

	// �������Ϸ���
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
	// ��ȡ��ǰϵͳʱ��
	SYSTEMTIME lpTime = GetSystemDateTime();
	sprintf_s(datestr, 10, "%d%02d%02d",
		lpTime.wYear,
		lpTime.wMonth,
		lpTime.wDay);
}

void MessLogger::CheckLogFile()
{
	// �Դ���ʱ��Ϊ׼
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
				printf("�ļ�ɾ��ʧ�ܡ�");
				goto END;
			}
		}
	}
	while (!_findnext(handle, &fileinfo))    
	{
		if (GetDaysSinceFileCreate(fileinfo) >= (int)m_uiMaxKeepDays) {
			sprintf_s(szFilename, "%s\\%s", m_szLogFileDirectory, fileinfo.name);
			if (!DeleteFileA(szFilename)) {
				printf("�ļ�ɾ��ʧ�ܡ�");
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