#include "Medusa.h"

#include "FileCheck.h"




Medusa::Medusa(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
	Enable_Debug();

	_Model = new QStandardItemModel();
	ui.tableView->setModel(_Model);
	ui.tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.tableView->horizontalHeader()->setSectionsClickable(false);
	ui.tableView->verticalHeader()->setDefaultSectionSize(25);
	ui.tableView->setSelectionBehavior(QAbstractItemView::SelectRows);

	_Model->setColumnCount(5);
	_Model->setHeaderData(0, Qt::Horizontal, u8"Index");
	_Model->setHeaderData(1, Qt::Horizontal, u8"PID");
	_Model->setHeaderData(2, Qt::Horizontal, u8"Name");
	_Model->setHeaderData(3, Qt::Horizontal, u8"Path");
	_Model->setHeaderData(4, Qt::Horizontal, u8"Desciption");
	ui.tableView->setColumnWidth(0, 50);
	ui.tableView->setColumnWidth(1, 70);
	ui.tableView->setColumnWidth(2, 180);
	ui.tableView->setColumnWidth(3, 900);
	ui.tableView->setColumnWidth(4, 400);
	ui.tableView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);


	
	QTextCodec* codec = QTextCodec::codecForName("UTF-8");
	QTextCodec::setCodecForLocale(codec);

	_TableView_Action_R3Inject.setMenu(&_TableView_Menu_R3Inject);

	_TableView_Menu_R3Inject.setTitle("Inject DLL");
	_TableView_Menu_R3Inject.addAction("CreateRemoteThread+LoadLibraryA");
	_TableView_Menu_R3Inject.addAction("NtCreateRemoteThread+syscall+shellcode+ldrloadlibaby");


	_Hook_QAction_Check.setMenu(&_HookCheck);
	_HookCheck.setTitle("HookScanner");
	_HookCheck.addAction("R3HookScannerSimple(Y/N)");
	_HookCheck.addAction("R3HookScanner"); 
	_HookCheck.addAction("R3QuickCheckALL");

	ui.tableView->addAction(&_TableView_Action_R3Inject);
	ui.tableView->addAction(&_Hook_QAction_Check);


	Set_SLOTS();



	HANDLE m_hDevice = CreateFileA("\\\\.\\IO_Control", GENERIC_READ | GENERIC_WRITE, 0,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE != m_hDevice)
	{
		_Driver_Loaded = true;
		CloseHandle(m_hDevice);
	}
	ChangeTab();
}


void Medusa::Set_SLOTS()
{
	connect(ui.tabWidget, SIGNAL(tabBarClicked(int)), SLOT(ChangeTab()));//����

	connect(&_TableView_Menu_R3Inject, SIGNAL(triggered(QAction*)), SLOT(ProcessRightMenu(QAction*)));//��������Ҽ��˵�
	connect(&_HookCheck, SIGNAL(triggered(QAction*)), SLOT(ProcessRightMenu(QAction*)));//��������Ҽ��˵�


	connect(ui.menuMenu, SIGNAL(triggered(QAction*)), SLOT(DriverLoad(QAction*)));
	//connect(ui.menuLoad_Driver, SIGNAL(triggered(QAction*)), SLOT(DriverLoad(QAction*)));
}

void Medusa::DriverLoad(QAction* action)
{
	if (!_Driver_Loaded)
	{
		if (action->text() == "NtLoad")
		{
			std::fstream check_file("MedusaKernel.sys", std::ios::in);
			if (check_file.is_open())
			{
				check_file.close();

				char temp_str[MAX_PATH];
				if (GetCurrentDirectoryA(MAX_PATH, temp_str) > 0)
				{
					_Driver_Load.Init(temp_str + std::string("\\MedusaKernel.sys"));
					if (_Driver_Load.Nt_Register_Driver())
					{
						if (_Driver_Load.Nt_Start_Driver())
						{
							_Driver_Loaded = true;
							ChangeTab();
							QMessageBox::information(this, "success", "driver loaded");
							return;
						}
					}
				}
			}
			QMessageBox::information(this, "error", "cannot load driver");
		}
		if (action->text() == "NormalLoad")
		{
			std::fstream check_file("MedusaKernel.sys", std::ios::in);
			if (check_file.is_open())
			{
				check_file.close();

				char temp_str[MAX_PATH];
				if (GetCurrentDirectoryA(MAX_PATH, temp_str) > 0)
				{
					_Driver_Load.Init(temp_str + std::string("\\MedusaKernel.sys"));
					if (_Driver_Load.Register_Driver())
					{
						if (_Driver_Load.Start_Driver())
						{
							_Driver_Loaded = true;
							ChangeTab();
							QMessageBox::information(this, "success", "driver loaded");
							return;
						}
					}
				}
			}
			QMessageBox::information(this, "error", "cannot load driver");
		}
	}
	
	if (action->text() == "UnLoadMedusaDriver")
	{
		if (_Driver_Load._NtModule)
		{
			if (_Driver_Load.Nt_Stop_Driver() && _Driver_Load.Nt_UnRegister_Driver())
			{
				_Driver_Loaded = false;
				QMessageBox::information(this, "success", "driver unload");
				exit(0);
			}
		}
		else
		{
			if (_Driver_Load.Stop_Driver() && _Driver_Load.UnRegister_Driver())
			{
				_Driver_Loaded = false;
				QMessageBox::information(this, "success", "driver unload");
				exit(0);
			}
		}
		
	}



	//������������
	if (action->text() == "Nt")
	{
		QFileDialog file_path;
		QString temp_str = file_path.getOpenFileName();
		if (temp_str.size() == 0)
		{
			return;
		}
		temp_str = QDir::toNativeSeparators(temp_str);
		if (_Driver_Load_Other.Init(temp_str.toStdString()))
		{
			if (_Driver_Load_Other.Nt_Register_Driver())
			{
				if (_Driver_Load_Other.Nt_Start_Driver())
				{
					QMessageBox::information(this, "success", "driver loaded");
					return;
				}
			}
		}
	}
	if (action->text() == "Normal")
	{
		QFileDialog file_path;
		QString temp_str = file_path.getOpenFileName();
		if (temp_str.size() == 0)
		{
			return;
		}
		temp_str = QDir::toNativeSeparators(temp_str);
		_Driver_Load_Other.Init(temp_str.toStdString());
		if (_Driver_Load_Other.Register_Driver())
		{
			if (_Driver_Load_Other.Start_Driver())
			{
				QMessageBox::information(this, "success", "driver loaded");
				return;
			}
		}
	}
	if (action->text() == "Unload")
	{
		if (_Driver_Load_Other._NtModule)
		{
			if (_Driver_Load_Other.Nt_Stop_Driver() && _Driver_Load_Other.Nt_UnRegister_Driver())
			{
				QMessageBox::information(this, "success", "driver unload");
			}
		}
		else
		{
			if (_Driver_Load_Other.Stop_Driver() && _Driver_Load_Other.UnRegister_Driver())
			{
				QMessageBox::information(this, "success", "driver unload");
			}
		}
	}
}

void Medusa::ProcessRightMenu(QAction* action)
{
	if (action->text() == "CreateRemoteThread+LoadLibraryA")
	{
		HANDLE handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, 
			_Process._Process_List_R3.at(ui.tableView->currentIndex().row()).PID);
		if (handle != NULL)
		{
			QFileDialog file_path;
			QString temp_str = file_path.getOpenFileName();
			if (temp_str.size() == 0)
			{
				return;
			}
			temp_str = QDir::toNativeSeparators(temp_str);
			int Path_Len = temp_str.length();
			PVOID New_Get_Addr = VirtualAllocEx(handle, NULL, Path_Len, MEM_COMMIT, PAGE_READWRITE);
			if (New_Get_Addr)
			{
				if (WriteProcessMemory(handle, New_Get_Addr, temp_str.toStdString().data(), Path_Len, 0))
				{
					PROC Get_Load_Addr = GetProcAddress(GetModuleHandle(L"kernel32.dll"), "LoadLibraryA");
					HANDLE New_Hand = CreateRemoteThread(handle, NULL, 0,
						(LPTHREAD_START_ROUTINE)Get_Load_Addr, New_Get_Addr, 0, NULL);
					if (New_Hand != NULL)
					{
						QMessageBox::information(this, "dll inject", "success");
						CloseHandle(New_Hand);
						CloseHandle(handle);
						return;
					}
				}
			}
			QMessageBox::information(this, "dll inject", "unsuccess");
			CloseHandle(handle);
		}
		
	}

	if (action->text() == "R3QuickCheckALL")
	{
		FileCheck temp_check;
		std::string temp_str;
		ui.progressBar->setMaximum(_Process._Process_List_R3.size());
		ui.progressBar->setValue(0);
		for (auto x : _Process._Process_List_R3)
		{
			if (x.PID == 0 || x.PID == 4)
			{
				continue;
			}
			if (!temp_check.CheckSimple(x.PID))
			{
				temp_str = temp_str + QString::fromWCharArray(x.Name).toStdString() + "\r\n";
			}
			ui.progressBar->setValue(ui.progressBar->value()+1);
		}
		temp_str = temp_str + "\r\n detected hook";
		QMessageBox::information(this, "Ret", temp_str.data());
	}
	if (action->text() == "R3HookScannerSimple(Y/N)")
	{
		if (_Process._Process_List_R3.at(ui.tableView->currentIndex().row()).PID == 0 
			|| _Process._Process_List_R3.at(ui.tableView->currentIndex().row()).PID == 4)
		{
			return;
		}
		FileCheck temp_check;
		if (temp_check.CheckSimple(_Process._Process_List_R3.at(ui.tableView->currentIndex().row()).PID))
		{
			QMessageBox::information(this, "Ret", "not detected hook");
		}
		else
		{
			QMessageBox::information(this, "Ret", "detected hook");
		}
	}
	if (action->text() == "R3HookScanner")
	{
		if (_Process._Process_List_R3.at(ui.tableView->currentIndex().row()).PID == 0
			|| _Process._Process_List_R3.at(ui.tableView->currentIndex().row()).PID == 4)
		{
			return;
		}
		_HookScanner._Model->removeRows(0, _Model->rowCount());
		FileCheck temp_check;
		std::vector<_CheckDifferent> temp_vector = temp_check.CheckPlain(_Process._Process_List_R3.at(ui.tableView->currentIndex().row()).PID);
		int i = 0;
		for (auto x : temp_vector)
		{
			_HookScanner._Model->setVerticalHeaderItem(i, new QStandardItem);
			_HookScanner._Model->setData(_HookScanner._Model->index(i, 0), i);
			_HookScanner._Model->setData(_HookScanner._Model->index(i, 1), QString::fromWCharArray(x.Name));
			std::ostringstream ret;
			ret << std::hex <<"0x" << x.Addr;
			_HookScanner._Model->setData(_HookScanner._Model->index(i, 2), ret.str().data());
			_HookScanner._Model->setData(_HookScanner._Model->index(i, 3), String_TO_HEX(std::string(x.FileHex,20)).data());
			_HookScanner._Model->setData(_HookScanner._Model->index(i, 4), String_TO_HEX(std::string(x.MemoryHex, 20)).data());
			_HookScanner._Model->setData(_HookScanner._Model->index(i, 5), QString::fromWCharArray(x.Path));
			i++;
		}
		_HookScanner.show();
		

		//QMessageBox::information(this, "1", "2");
	}

}

void Medusa::ChangeTab()
{
	if (ui.tabWidget->currentIndex() == 0)
	{
		GetProcessList();
	}
}


void Medusa::GetProcessList()
{
	_Model->removeRows(0, _Model->rowCount());
	_Process.GetProcessList(_Driver_Loaded);
	int i = 0;
	if (_Driver_Loaded)
	{
		for (auto x : _Process._Process_List_R0)
		{
			bool found = false;
			PROCESS_LIST temp_list;

			int j = 0;
			for (; j < _Process._Process_List_R3.size(); j++)
			{
				if (_Process._Process_List_R3.at(j).PID == x.PID)
				{
					found = true;
					break;
				}
			}
			if (found)
			{
				_Model->setVerticalHeaderItem(i, new QStandardItem);
				_Model->setData(_Model->index(i, 0), i);
				_Model->setData(_Model->index(i, 1), _Process._Process_List_R3.at(j).PID);
				_Model->setData(_Model->index(i, 2), QString::fromWCharArray(_Process._Process_List_R3.at(j).Name));
				if (std::wstring(_Process._Process_List_R3.at(j).Path) != L"")
				{
					_Model->setData(_Model->index(i, 3), QString::fromWCharArray(_Process._Process_List_R3.at(j).Path));
					std::wstring retStr;
					if (_Process.QueryValue(L"FileDescription", _Process._Process_List_R3.at(j).Path, retStr))
					{
						_Model->setData(_Model->index(i, 4), QString::fromWCharArray(retStr.data()));
					}
				}
				else
				{
					_Model->setData(_Model->index(i, 3), QString::fromWCharArray(x.Path));
					std::wstring retStr;
					if (_Process.QueryValue(L"FileDescription", x.Path, retStr))
					{
						_Model->setData(_Model->index(i, 4), QString::fromWCharArray(retStr.data()));
					}
				}

			}
			else
			{
				_Model->setVerticalHeaderItem(i, new QStandardItem);
				_Model->setData(_Model->index(i, 0), i);
				_Model->setData(_Model->index(i, 1), x.PID);
				_Model->setData(_Model->index(i, 2), QString::fromWCharArray(x.Name));
				_Model->setData(_Model->index(i, 3), QString::fromWCharArray(x.Path));


				if (x.Check == false)
				{
					_Model->item(i, 0)->setBackground(QColor(Qt::red));
					_Model->item(i, 1)->setBackground(QColor(Qt::red));
					_Model->item(i, 2)->setBackground(QColor(Qt::red));
					_Model->item(i, 3)->setBackground(QColor(Qt::red));
					std::wstring retStr;
					if (_Process.QueryValue(L"FileDescription", x.Path, retStr))
					{
						_Model->setData(_Model->index(i, 4), QString::fromWCharArray(retStr.data()));
						_Model->item(i, 4)->setBackground(QColor(Qt::red));
					}
				}
				else
				{
					_Model->item(i, 0)->setBackground(QColor(Qt::green));
					_Model->item(i, 1)->setBackground(QColor(Qt::green));
					_Model->item(i, 2)->setBackground(QColor(Qt::green));
					_Model->item(i, 3)->setBackground(QColor(Qt::green));
					std::wstring retStr;
					if (_Process.QueryValue(L"FileDescription", x.Path, retStr))
					{
						_Model->setData(_Model->index(i, 4), QString::fromWCharArray(retStr.data()));
						_Model->item(i, 4)->setBackground(QColor(Qt::green));
					}
				}
			}



			i++;
		}
		ui.label->setText(QString((std::string("R3 get process number:") +
			std::to_string(_Process._Process_List_R3.size()) +
			std::string("=====R0 get process number:") +
			std::to_string(_Process._Process_List_R0.size())).data()));
	}
	else
	{
		for (auto x : _Process._Process_List_R3)
		{
			_Model->setVerticalHeaderItem(i, new QStandardItem);
			_Model->setData(_Model->index(i, 0), i);
			_Model->setData(_Model->index(i, 1), x.PID);
			_Model->setData(_Model->index(i, 2), QString::fromWCharArray(x.Name));
			_Model->setData(_Model->index(i, 3), QString::fromWCharArray(x.Path));

			std::wstring retStr;
			if (_Process.QueryValue(L"FileDescription", x.Path, retStr))
			{
				_Model->setData(_Model->index(i, 4), QString::fromWCharArray(retStr.data()));
			}
			i++;
		}

		ui.label->setText(QString((std::string("R3 get process number:") +
			std::to_string(_Process._Process_List_R3.size())).data()));
	}
}
