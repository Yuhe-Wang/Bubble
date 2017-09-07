// Size.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "Size.h"
#include "math.h"
//head files of C++ feature
#include <map>
#include <string>

long In;
float Ave=0.4f; //for initial distribution
float Variance=0.1f; //for Gaussian distribution
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

		Ave=mp["Ave="];
		Variance=mp["Variance="];
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
	double X;
	do
	{
		X=Variance*GaussSampling()+Ave;
	} while (X<Ave-3*Variance||X>Ave+3*Variance);
	return X;
	//X=Ave+Variance*(2*random()-1); //Uniform Distribution
}

double random() //Random number generator
{
	/*static long In=(long)time(NULL);*/
	In=16807*(In%127773)-2836*(In/127773);
	if(In<0) In+=2147483647;
	return (double)In/2147483647;
}

double GaussSampling()
{
	static double u,v,r2;
	static bool phase=0;
	if(phase==0)
	{
		do 
		{
			u=2*random()-1;
			v=2*random()-1;
			r2=u*u+v*v;
		} while (r2>=1.0||r2==0);
		return u*sqrt(-2*log(r2)/r2);
	}else
	{
		phase=!phase;
		return v*sqrt(-2*log(r2)/r2);
	}
}