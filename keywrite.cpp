// keywrite.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// The one and only application object

CWinApp theApp;

using namespace std;

char szKey[32]={0};

BOOL GetProductKey()
{
	BOOL retval,result=FALSE;
	PROCESS_INFORMATION pi={0};
	STARTUPINFOA si={0};
	SECURITY_ATTRIBUTES sa={0};
	HANDLE hReadPipe,hWritePipe;
	DWORD retcode = -1;
	CFile fp;

	sa.bInheritHandle=TRUE;
	sa.nLength=sizeof SECURITY_ATTRIBUTES;
	sa.lpSecurityDescriptor=NULL;
	retval=CreatePipe(&hReadPipe,&hWritePipe,&sa,0);
	if(retval)
	{
		si.cb=sizeof STARTUPINFO;
		si.wShowWindow=SW_HIDE;
		si.dwFlags=STARTF_USESHOWWINDOW|STARTF_USESTDHANDLES;
		si.hStdOutput=si.hStdError=hWritePipe;
		retval=CreateProcessA(NULL,"cmd.exe /c Check.exe",&sa,&sa,TRUE,0,NULL,0,&si,&pi);
		if(retval)
		{
			DWORD dwLen,dwRead;
			WaitForSingleObject(pi.hThread,INFINITE);//等待命令行执行完毕
			GetExitCodeProcess(pi.hProcess,&retcode);
			CloseHandle(pi.hThread);
			CloseHandle(pi.hProcess);
			if (retcode != 0)
			{
				goto end;
			}
			dwLen=GetFileSize(hReadPipe,NULL);
			char *buff=new char [dwLen+1];
			char dpk[30]={0};
			char* vptr,*token="Product key:       ";
			memset(buff,0,dwLen+1);
			retval=ReadFile(hReadPipe,buff,dwLen,&dwRead,NULL);
			vptr=strstr(buff+700,token);
			if (vptr)
			{
				vptr +=strlen(token);
				strncpy(szKey,vptr,29);
				result = TRUE;
			}
			delete buff;
		}
end:
		CloseHandle(hWritePipe);
		CloseHandle(hReadPipe);
	}
	return result;
}

int _tmain(int argc, _TCHAR* argv[])
{
	if (GetProductKey())
	{
		char szDPK[20]={0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
						0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
						0x1d,0x00,0x00,0x00};
		FILE* fp=fopen("key.bin","w+");
		if (fp == NULL)
		{
			printf("Can't write key.bin to disk!\n");
			return -1;
		}
		fwrite(szDPK,20,1,fp);
		fwrite(szKey,29,1,fp);
		fclose(fp);
		printf("5x5 Key: %s\n",szKey);
	}
	else
	{
		printf("This device has no key in bios.\n");
		return -2;
	}
	return 0;
}

