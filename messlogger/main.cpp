// messlogger.cpp : 定义控制台应用程序的入口点。
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
	g_pLogger->Init(30, "\\logs\\log_%date.txt", MessLogger::LEVEL_ALL, true);

	for (uint8_t i = 0; i < 10; i++)
	{
		thread t(log_std_thread);
		t.detach();
	}

	for (uint8_t i = 0; i < 10; i++)
	{
		HANDLE hThread;
		DWORD  threadId;

		hThread = CreateThread(NULL, 0, log_thread, 0, 0, &threadId); // 创建线程
		Sleep(1000);
	}

	DLOG("DLOG TEST 调试 %d %s", 678, "i'm DLOG");
	TLOG("TLOG TEST %d %s", 678, "i'm TLOG");
	ILOG("ILOG TEST %d %s", 678, "i'm ILOG");
	WLOG("WLOG TEST %d %s", 678, "i'm WLOG");
	ELOG("ELOG TEST %d %s", 678, "i'm ELOG");
	FLOG("ILOG TEST %d %s", 678, "i'm FLOG");

	system("pause");

	return 0;
}

DWORD WINAPI log_thread(LPVOID p)
{
	for (int i = 0; i < 100; i++)	{
		DLOG("%d HELLOLKJLKSDFJLAKSJDFLKASJFLKJASDKLFJKL  JASKLJDFKLASJDKF   Dsafad的地方", i);
		TLOG("%d HELLOLKJLKSDFJLAKSJDFLKASJFLKJASDKLFJKL  JASKLJDFKLASJDKF   Dsafad的地方", i);
		ILOG("%d HELLOLKJLKSDFJLAKSJDFLKASJFLKJASDKLFJKL  JASKLJDFKLASJDKF   Dsafad的地方", i);
		WLOG("%d HELLOLKJLKSDFJLAKSJDFLKASJFLKJASDKLFJKL  JASKLJDFKLASJDKF   Dsafad的地方", i);
		ELOG("%d HELLOLKJLKSDFJLAKSJDFLKASJFLKJASDKLFJKL  JASKLJDFKLASJDKF   Dsafad的地方", i);
		FLOG("%d HELLOLKJLKSDFJLAKSJDFLKASJFLKJASDKLFJKL  JASKLJDFKLASJDKF   Dsafad的地方", i);
		LOG_LINE_BREAK();
	}
	return 0;
}

VOID log_std_thread()
{
	for (int i = 0; i < 100; i++)	{
		DLOG("%d HELLOLKJLKSDFJLAKSJDFLKASJFLKJASDKLFJKL  JASKLJDFKLASJDKF   Dsafad的地方", i);
		TLOG("%d HELLOLKJLKSDFJLAKSJDFLKASJFLKJASDKLFJKL  JASKLJDFKLASJDKF   Dsafad的地方", i);
		ILOG("%d HELLOLKJLKSDFJLAKSJDFLKASJFLKJASDKLFJKL  JASKLJDFKLASJDKF   Dsafad的地方", i);
		WLOG("%d HELLOLKJLKSDFJLAKSJDFLKASJFLKJASDKLFJKL  JASKLJDFKLASJDKF   Dsafad的地方", i);
		ELOG("%d HELLOLKJLKSDFJLAKSJDFLKASJFLKJASDKLFJKL  JASKLJDFKLASJDKF   Dsafad的地方", i);
		FLOG("%d HELLOLKJLKSDFJLAKSJDFLKASJFLKJASDKLFJKL  JASKLJDFKLASJDKF   Dsafad的地方", i);
		LOG_LINE_BREAK();
	}
}
