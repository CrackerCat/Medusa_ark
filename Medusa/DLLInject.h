#pragma once
#include <windows.h>
#include <fstream>


class DLLInject
{
public:
	DLLInject() = default;
	~DLLInject() = default;
public:
	bool R3CreateThread(ULONG64 PID);
	bool R3APCInject(ULONG64 PID);
	bool R3MapInject(ULONG64 PID);
private:
	std::string Read_ALL(std::string file_name)
	{
		std::string result;
		//n �� binary �� ios ���ﶨ����Զ����Ʒ�ʽ���ļ����ڶ��룬�� | �� �������
		std::fstream file(file_name, std::ios::in | std::ios::binary);
		if (file.is_open() == false)
		{
			return "";
		}
		std::string str((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
		file.close();
		result = str;
		return result;
	}
};

