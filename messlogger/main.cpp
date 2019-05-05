// messlogger.cpp : �������̨Ӧ�ó������ڵ㡣
//
#include <windows.h>
#include <tchar.h>
#include <iostream>
#include <thread>

#include "MessLogger.h"

using namespace std;

ENABLE_THREAD_SAFE

MessLogger *g_pLogger = MessLogger::GetInstance();

DWORD WINAPI log_thread(LPVOID);
VOID log_std_thread();

int _tmain(int argc, _TCHAR* argv[])
{
	// �Զ�ռ��ʽ���ļ�
	//FILE * fp = _fsopen("D:\\_vts_repo\\vts-for-windows\\module_solution\\modules\\fun_modules\\fun_messlogger\\branches\\v1.0.0\\Release\\logs\\lanxum_20190425.log", "a+", _SH_DENYRW);	// ���һ��������ʾ����ʽ���ļ�
	//if (fp == NULL)
	//	printf("�ļ���ʧ�ܡ�");


	g_pLogger->Init(30, "\\logs\\lanxum_%date.log", MessLogger::LEVEL_ALL, true);

	for (uint8_t i = 0; i < 10; i++)
	{
		thread t(log_std_thread);
		t.detach();
	}

	for (uint8_t i = 0; i < 10; i++)
	{
		HANDLE hThread;
		DWORD  threadId;

		hThread = CreateThread(NULL, 0, log_thread, 0, 0, &threadId); // �����߳�
		Sleep(1000);
	}

	DLOG("DLOG TEST ���� %d %s", 678, "i'm DLOG");
	TLOG("TLOG TEST %d %s", 678, "i'm TLOG");
	ILOG("ILOG TEST %d %s", 678, "i'm ILOG");
	WLOG("WLOG TEST %d %s", 678, "i'm WLOG");
	ELOG("ELOG TEST %d %s", 678, "i'm ELOG");
	FLOG("ILOG TEST %d %s", 678, "i'm FLOG");

	system("pause");

	//fclose(fp);
	return 0;
}

DWORD WINAPI log_thread(LPVOID p)
{
	for (int i = 0; i < 100; i++)	{
		DLOG("%d HELLOLKJLKSDFJLAKSJDFLKASJFLKJASDKLFJKL  JASKLJDFKLASJDKF   Dsafad�ĵط�", i);
		TLOG("%d HELLOLKJLKSDFJLAKSJDFLKASJFLKJASDKLFJKL  JASKLJDFKLASJDKF   Dsafad�ĵط�", i);
		ILOG("%d HELLOLKJLKSDFJLAKSJDFLKASJFLKJASDKLFJKL  JASKLJDFKLASJDKF   Dsafad�ĵط�", i);
		WLOG("%d HELLOLKJLKSDFJLAKSJDFLKASJFLKJASDKLFJKL  JASKLJDFKLASJDKF   Dsafad�ĵط�", i);
		ELOG("%d HELLOLKJLKSDFJLAKSJDFLKASJFLKJASDKLFJKL  JASKLJDFKLASJDKF   Dsafad�ĵط�", i);
		FLOG("%d HELLOLKJLKSDFJLAKSJDFLKASJFLKJASDKLFJKL  JASKLJDFKLASJDKF   Dsafad�ĵط�", i);
		LOG_LINE_BREAK();
	}
	return 0;
}

VOID log_std_thread()
{
	for (int i = 0; i < 100; i++)	{
		DLOG("%d HELLOLKJLKSDFJLAKSJDFLKASJFLKJASDKLFJKL  JASKLJDFKLASJDKF   Dsafad�ĵط�", i);
		TLOG("%d HELLOLKJLKSDFJLAKSJDFLKASJFLKJASDKLFJKL  JASKLJDFKLASJDKF   Dsafad�ĵط�", i);
		ILOG("%d HELLOLKJLKSDFJLAKSJDFLKASJFLKJASDKLFJKL  JASKLJDFKLASJDKF   Dsafad�ĵط�", i);
		WLOG("%d HELLOLKJLKSDFJLAKSJDFLKASJFLKJASDKLFJKL  JASKLJDFKLASJDKF   Dsafad�ĵط�", i);
		ELOG("%d HELLOLKJLKSDFJLAKSJDFLKASJFLKJASDKLFJKL  JASKLJDFKLASJDKF   Dsafad�ĵط�", i);
		FLOG("%d HELLOLKJLKSDFJLAKSJDFLKASJFLKJASDKLFJKL  JASKLJDFKLASJDKF   Dsafad�ĵط�", i);
		LOG_LINE_BREAK();
	}
}
