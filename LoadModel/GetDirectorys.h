#pragma once
#include <io.h>
#include <fstream>
#include <string>
#include <vector>
#include <iostream>
using namespace std;

//获取所有的文件夹路径和文件夹名称
void GetAllFiles(string path, vector<string>& files, vector<string>& directoryName)
{
	_int64 hFile = 0;
	//文件信息
	struct _finddata_t fileinfo;
	string p;
	if ((hFile = _findfirst(p.assign(path).append("//*").c_str(), &fileinfo)) != -1)
	{
		do
		{
			if ((fileinfo.attrib & _A_SUBDIR))
			{
				if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0)
				{
					files.push_back(p.assign(path).append("//").append(fileinfo.name));
					directoryName.push_back(fileinfo.name);
					//GetAllFiles(p.assign(path).append("\\").append(fileinfo.name), files);
				}
			}
			else
			{
				//files.push_back(p.assign(path).append("\\").append(fileinfo.name));
			}
		} while (_findnext(hFile, &fileinfo) == 0);
		_findclose(hFile);
	}
}