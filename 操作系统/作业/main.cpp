#include <iostream>
#include <windows.h>
#include <string>
#include <fstream>
#include <ctime>
#include "md5.h"
#include "md5.cpp"
#include <sys\stat.h>
using namespace std;
int cnt = 0;
ofstream LogFile;
struct File
{
	string Name;
	string md5;
	bool del;
}file[1010];
void ListFilesInDirectory(string strDir)
{
	if(strDir.empty())
	{
		return;
	}
	HANDLE hFind;
	WIN32_FIND_DATA findData;
	string strTempDir = strDir + ("\\*");
	hFind = FindFirstFile(strTempDir.c_str(), &findData);
	if(hFind==INVALID_HANDLE_VALUE)
	{
		return;
	}
	do
	{
		if(strcmp(findData.cFileName, ".")==0||strcmp(findData.cFileName, "..")==0)continue;
		string strFileName;
		strFileName.assign(strDir);
		strFileName.append("\\");
		strFileName.append(findData.cFileName);
		file[cnt++]={strFileName, md5file(strFileName.c_str()), false};
		if(findData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
		{
			ListFilesInDirectory(strFileName.c_str());
		}
	}while(FindNextFile(hFind, &findData)!=0);
	FindClose(hFind);
	return;	
}
void remove()
{
	int count = 0;
	for(int i = 0;i<cnt-1;i++)
	{
		if(file[i].del)continue;
		bool flag = false;
		for(int j = i+1;j<cnt;j++)
		{
			if(file[i].md5==file[j].md5)
			{
				DeleteFile(file[j].Name.c_str());
				if(!flag)
				{
					flag = true;
					struct stat buf;
					int result = stat(file[i].Name.c_str(), &buf);
					LogFile<<"重复文件属性如下:"<<endl;
					LogFile<<"group"<<++count<<":"<<endl;
					LogFile<<"名称:"<<file[i].Name<<endl;
					LogFile<<"文件大小:"<<buf.st_size<<"字节"<<endl;
    				LogFile<<"所在磁盘盘符 :";
    				LogFile<<char(buf.st_dev + 'A')<<endl;
    				LogFile<<"文件创建时间:"<<ctime(&buf.st_ctime);
    				LogFile<<"访问日期:"<<ctime(&buf.st_atime);
    				LogFile<<"最后修改日期:"<<ctime(&buf.st_mtime);
				}
				file[j].del = true;
			}
		}
	}
}
int main()
{
	LogFile.open("log_file.txt", ofstream::out);	
    ListFilesInDirectory(".\\test");
	for(int i = 0;i<cnt;i++)cout<<file[i].Name<<" "<<file[i].md5<<endl;
	remove();
	bool flag = false;
	for(int i = 0;i<cnt;i++)
	{
		if(file[i].del)
		{
			flag = true;
			LogFile<<file[i].Name<<" has been deleted"<<endl;
		}
	}
	if(!flag)LogFile<<"没有重复文件"<<endl;
	LogFile.close();
    return 0;
}