// Size.cpp : Two Delta functions
//

#include "stdafx.h"
#include "Size.h"
//head files of C++ feature
#include <map>
#include <string>

long In;
float Ave1=0.1f; //for initial distribution
float Ave2=0.2f; //for Gaussian distribution
//initialization
BOOL APIENTRY DllMain( HMODULE hModule,
					  DWORD  ul_reason_for_call,
					  LPVOID lpReserved
					  )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
		In=(long)time(NULL);
		loadConfig();
		break;
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	

	return TRUE;
}

void loadConfig()
{
	FILE *fp=fopen("initConfg.txt","r");
	if(fp)
	{
		using std::map;
		using std::string;
		char name[50];//,value[50];
		float temp;
		int ch=0;
		map<string,float> mp;
		while (fscanf(fp,"%s %f",name,&temp)!=-1)
		{
			while((ch=getc(fp))!='\n' && ch!=-1){};
			mp[name]=temp;
		}

		Ave1=mp["Ave1="];
		Ave2=mp["Ave2="];
		fclose(fp);
	}
	else
	{
		printf("WARNING:\nThe 'initConfg.txt' file doesn't exist in current directory!\nSo the program will use default values, which may be not good.\n\nPress any key to continue.\n\n");
		getchar();
	}
}

SIZE_API double fnSize(void)
{
	static bool phase=0;
	phase=!phase;
	if(phase) return Ave1;
	else return Ave2;
}