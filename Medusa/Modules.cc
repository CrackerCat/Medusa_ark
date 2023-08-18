#include "Modules.h"



Modules::Modules(QWidget* parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	_Model = new QStandardItemModel();
	ui.tableView->setModel(_Model);
	ui.tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.tableView->horizontalHeader()->setSectionsClickable(false);
	ui.tableView->verticalHeader()->setDefaultSectionSize(25);
	ui.tableView->setSelectionBehavior(QAbstractItemView::SelectRows);

	_Model->setColumnCount(6);
	_Model->setHeaderData(0, Qt::Horizontal, u8"Index");
	_Model->setHeaderData(1, Qt::Horizontal, u8"Name");
	_Model->setHeaderData(2, Qt::Horizontal, u8"Addr");
	_Model->setHeaderData(3, Qt::Horizontal, u8"Size");
	_Model->setHeaderData(4, Qt::Horizontal, u8"Path");
	_Model->setHeaderData(5, Qt::Horizontal, u8"Desciption");

	ui.tableView->setColumnWidth(0, 50);
	ui.tableView->setColumnWidth(1, 150);
	ui.tableView->setColumnWidth(2, 150);
	ui.tableView->setColumnWidth(3, 150);
	ui.tableView->setColumnWidth(4, 150);
	ui.tableView->setColumnWidth(5, 400);

	//this->setWindowFlags(Qt::FramelessWindowHint);
}

std::vector<MODULEENTRY32W> Modules::GetUserMoudleListR3(ULONG64 PID)
{
	std::vector<MODULEENTRY32W> temp_vector;

	HANDLE        hModuleSnap = INVALID_HANDLE_VALUE;
	MODULEENTRY32 me32 = { sizeof(MODULEENTRY32) };

	hModuleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, PID);
	if (hModuleSnap == INVALID_HANDLE_VALUE)
	{
		int a = GetLastError();
		return temp_vector;
	}

	if (!Module32First(hModuleSnap, &me32))
	{
		CloseHandle(hModuleSnap);
		return temp_vector;
	}

	do {
		temp_vector.push_back(me32);

	} while (Module32Next(hModuleSnap, &me32));
	CloseHandle(hModuleSnap);
	return temp_vector;
}


#define TEST_GetALLUserModule CTL_CODE(FILE_DEVICE_UNKNOWN,0x7104,METHOD_BUFFERED ,FILE_ANY_ACCESS)
#define TEST_GetALLUserModuleNumber CTL_CODE(FILE_DEVICE_UNKNOWN,0x7105,METHOD_BUFFERED ,FILE_ANY_ACCESS)

std::vector<UserModule> Modules::GetUserMoudleListR0(ULONG64 PID)
{
	std::vector<UserModule> temp_vector;

	HANDLE m_hDevice = CreateFileA("\\\\.\\IO_Control", GENERIC_READ | GENERIC_WRITE, 0,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == m_hDevice)
	{
		return temp_vector;
	}
	do
	{
		DWORD process_number = 0;
		DeviceIoControl(m_hDevice, TEST_GetALLUserModuleNumber, &PID, 8, 0, 0, &process_number, NULL);
		if (!process_number)
		{
			break;
		}

		DWORD dwRet = 0;
		UserModule* temp_list = (UserModule*)new char[process_number * sizeof(UserModule)];
		if (!temp_list)
		{
			break;
		}

		DeviceIoControl(m_hDevice, TEST_GetALLUserModule, &PID, 8, temp_list, sizeof(UserModule) * process_number, &dwRet, NULL);
		if (dwRet)
		{
			for (int i = 0; i < process_number; i++)
			{
				temp_vector.push_back(temp_list[i]);
			}
		}
		delete temp_list;
	} while (false);
	CloseHandle(m_hDevice);
	return temp_vector;
}