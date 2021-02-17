#include <windows.h>
#include <dbt.h>
#include <iostream>
#include <initguid.h>
#include <Usbiodef.h>
#include <SetupAPI.h>
#include <vector>
#include <Cfgmgr32.h>
#include <conio.h>
#include <string>

#pragma comment (lib, "user32.lib" )
#pragma comment (lib, "Setupapi.lib")

using namespace std;

#define CLS_NAME "DUMMY_CLASS"

struct UsbDeviceDesc
{
	DEVINST devInst;
	string friendlyName;
	wstring name;
	HANDLE handle;
	bool safety;
	bool removable;
};

vector<UsbDeviceDesc> deviceArray;

bool caseUnsensCmp(wstring s1, wstring s2)
{
	if (s1.size() != s2.size())
		return false;
	for (int i = 0; i < s1.size(); i++)
		if (toupper(s1[i]) != toupper(s2[i]))
			return false;
	return true;
}

string handleToFriendlyName(HANDLE handle)
{
	for (auto it : deviceArray)
		if (it.handle == handle)
			return it.friendlyName;
}

bool nameToSafety(wstring name)
{
	for (auto it : deviceArray)
		if (caseUnsensCmp(it.name, name))
			return it.safety;
	return false;
}

void setSafety(HANDLE handle, bool safety)
{
	for (int i = 0; i < deviceArray.size(); i++)
		if (deviceArray[i].handle == handle)
			deviceArray[i].safety = safety;
}

void deleteByName(wstring name)
{
	for (int i = 0; i < deviceArray.size(); i++)
		if (caseUnsensCmp(deviceArray[i].name, name))
		{
			deviceArray.erase(deviceArray.begin() + i);
			return;
		}
}


string getFriendlyName(wchar_t* name)
{
	HDEVINFO deviceList = SetupDiCreateDeviceInfoList(NULL, NULL);
	SP_DEVICE_INTERFACE_DATA deviceInterfaceData; 
	SetupDiOpenDeviceInterfaceW(deviceList, name, NULL, &deviceInterfaceData); 
	SP_DEVINFO_DATA deviceInfo; 
	ZeroMemory(&deviceInfo, sizeof(SP_DEVINFO_DATA));
	deviceInfo.cbSize = sizeof(SP_DEVINFO_DATA);
	SetupDiEnumDeviceInfo(deviceList, 0, &deviceInfo);

	DWORD size = 0;
	SetupDiGetDeviceRegistryPropertyA(deviceList, &deviceInfo, SPDRP_DEVICEDESC, NULL, NULL, NULL, &size); 
	BYTE* buffer = new BYTE[size];
	SetupDiGetDeviceRegistryPropertyA(deviceList, &deviceInfo, SPDRP_DEVICEDESC, NULL, buffer, size, NULL);
	string deviceDesc = (char*)buffer;
	delete[] buffer;

	return deviceDesc;
}

bool getRemoveability(wchar_t* name)
{
	HDEVINFO deviceList = SetupDiCreateDeviceInfoList(NULL, NULL);
	SP_DEVICE_INTERFACE_DATA deviceInterfaceData;
	SetupDiOpenDeviceInterfaceW(deviceList, name, NULL, &deviceInterfaceData);
	SP_DEVINFO_DATA deviceInfo;
	ZeroMemory(&deviceInfo, sizeof(SP_DEVINFO_DATA));
	deviceInfo.cbSize = sizeof(SP_DEVINFO_DATA);
	SetupDiEnumDeviceInfo(deviceList, 0, &deviceInfo);
	DWORD properties;
	SetupDiGetDeviceRegistryPropertyA(deviceList, &deviceInfo, SPDRP_CAPABILITIES, NULL, (PBYTE)&properties, sizeof(DWORD), NULL);
	return properties & CM_DEVCAP_REMOVABLE;
}

string getFriendlyName(PDEV_BROADCAST_DEVICEINTERFACE_A info)
{
	wchar_t* name = (wchar_t*)info->dbcc_name;
	return getFriendlyName(name);
}

string getInstId(const wchar_t* name)
{
	HDEVINFO deviceList = SetupDiCreateDeviceInfoList(NULL, NULL);
	SP_DEVICE_INTERFACE_DATA deviceInterfaceData;
	SetupDiOpenDeviceInterfaceW(deviceList, name, NULL, &deviceInterfaceData);
	SP_DEVINFO_DATA deviceInfo;
	ZeroMemory(&deviceInfo, sizeof(SP_DEVINFO_DATA));
	deviceInfo.cbSize = sizeof(SP_DEVINFO_DATA);
	SetupDiEnumDeviceInfo(deviceList, 0, &deviceInfo);

	BYTE buffer[BUFSIZ];
	SetupDiGetDeviceInstanceIdA(deviceList, &deviceInfo, (PSTR)buffer, BUFSIZ, NULL);
	return (char*)buffer;
}

bool device_safe_removed()
{
	if (MessageBox(0, "Хотите ли Вы извлечь выбранное устройтсво?", "Предупреждение", MB_ICONQUESTION | MB_YESNO) == IDYES)
		return true;
	else
		return false;
}

LRESULT FAR PASCAL WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	//LRESULT – это возвращаемое значение. CALLBACK нужно писать, так как это функция обратного вызова.

	/*Указатель типа far — 32-битовый; он содержит как адрес сегмента, так и смещение. При использовании указателей типа far
	допустимы обращения к памяти в пределах 1-мегабайтного адресного пространства процессора Intel 8086/8088,
	однако значение указателя типа far циклически изменяется в пределах одного 64-килобайтного сегмента.

	Применение модификатора pascal к идентификатору приводит к тому, что идентификатор преобразуется к верхнему регистру и к нему
	не добавляется символ подчеркивания. Этот идентификатор может использоваться для именования в программе на языке Си
	глобального объекта, который используется также в программе на языке Паскаль. В объектном коде, сгенерированном компилятором языка Си,
	и в объектном коде, сгенерированном компилятором языка Паскаль, идентификатор будет представлен идентично.
	Если модификатор pascal применяется к идентификатору функции, то он оказывает влияние также и на передачу аргументов.
	Засылка аргументов в стек производится в этом случае не в обратном порядке, как принято в компиляторах языка Си в СП MSC и СП ТС,
	а в прямом—первым засылается в стек первый аргумент.
	Функции типа pascal не могут иметь переменное число аргументов, как, например, функция printf.
	Поэтому нельзя использовать завершающее многоточие в списке параметров функции типа pascal.

	https://docs.microsoft.com/en-us/previous-versions/windows/desktop/legacy/ms633573(v=vs.85)
	*/

	bool result = false;
	if (message == WM_DEVICECHANGE) 
	{


		if (wParam == DBT_DEVICEARRIVAL) 
		{
			PDEV_BROADCAST_HDR lpdb = (PDEV_BROADCAST_HDR)lParam;
			if (lpdb->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE)
			{
				PDEV_BROADCAST_DEVICEINTERFACE_A info = (PDEV_BROADCAST_DEVICEINTERFACE_A)lpdb;
				cout << "Подключено устройство: \"" << getFriendlyName(info) << "\"" << endl;

				HANDLE deviceHandle = CreateFileW((wchar_t*)info->dbcc_name, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
				DEV_BROADCAST_HANDLE deviceFilter;
				deviceFilter.dbch_size = sizeof(deviceFilter);
				deviceFilter.dbch_devicetype = DBT_DEVTYP_HANDLE;
				deviceFilter.dbch_handle = deviceHandle;
				HDEVNOTIFY notifyHandle = RegisterDeviceNotificationW(hWnd, &deviceFilter, DEVICE_NOTIFY_WINDOW_HANDLE);
				CloseHandle(deviceHandle);

				DEVINST devInst;
				CM_Locate_DevNodeA(&devInst, (DEVINSTID_A)getInstId((wchar_t*)info->dbcc_name).c_str(), CM_LOCATE_DEVNODE_NORMAL);

				UsbDeviceDesc tempDesc;
				tempDesc.devInst = devInst;
				tempDesc.handle = deviceHandle;
				tempDesc.name = (wchar_t*)info->dbcc_name;
				tempDesc.safety = false;
				tempDesc.friendlyName = getFriendlyName(info);
				tempDesc.removable = getRemoveability((wchar_t*)info->dbcc_name);
				deviceArray.push_back(tempDesc);
			}
			if (lpdb->dbch_devicetype == DBT_DEVTYP_VOLUME)
			{
				PDEV_BROADCAST_VOLUME info = (PDEV_BROADCAST_VOLUME)lpdb;
				if (info->dbcv_unitmask)
				{
					cout << "Список логических дисков на подключенном устройстве: ";
					for (int i = 0; i < 31; i++)
						if (info->dbcv_unitmask & (DWORD)pow(2, i))
							cout << (char)('A' + i) << ' ';
					cout << '\n';
				}

			}
		}
		if (wParam == DBT_DEVICEQUERYREMOVE)
		{
			PDEV_BROADCAST_HDR lpdb = (PDEV_BROADCAST_HDR)lParam;
			result = device_safe_removed();
			if (lpdb->dbch_devicetype == DBT_DEVTYP_HANDLE && result)
			{
				PDEV_BROADCAST_HANDLE info = (PDEV_BROADCAST_HANDLE)lpdb;
				setSafety(info->dbch_handle, true);
			}
			else
			{
				printf("Извлечение устройства запрещено.\n");
				return BROADCAST_QUERY_DENY;
			}
		}
		if (wParam == DBT_DEVICEQUERYREMOVEFAILED)
		{
			PDEV_BROADCAST_HDR lpdb = (PDEV_BROADCAST_HDR)lParam;
			if (lpdb->dbch_devicetype == DBT_DEVTYP_HANDLE)
			{
				PDEV_BROADCAST_HANDLE info = (PDEV_BROADCAST_HANDLE)lpdb;
				cout << "Не удалось безопасно извлечь устройство \"" << handleToFriendlyName(info->dbch_handle) << "\"" << endl;
				setSafety(info->dbch_handle, false);
			}
		}
		if (wParam == DBT_DEVICEREMOVECOMPLETE)
		{
			PDEV_BROADCAST_HDR lpdb = (PDEV_BROADCAST_HDR)lParam;
			if (lpdb->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE)
			{
				PDEV_BROADCAST_DEVICEINTERFACE_A info = (PDEV_BROADCAST_DEVICEINTERFACE_A)lpdb;
				cout << "Устройство \"" << getFriendlyName(info) << "\" было отключено"
					<< (nameToSafety((wchar_t*)info->dbcc_name) ? " в безопасном режиме!" : "!") << '\n';
				deleteByName((wchar_t*)info->dbcc_name);
			}
			if (lpdb->dbch_devicetype == DBT_DEVTYP_HANDLE)
			{
				PDEV_BROADCAST_HANDLE info = (PDEV_BROADCAST_HANDLE)lpdb;
				UnregisterDeviceNotification(info->dbch_hdevnotify);
			}
		}
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

DWORD WINAPI ThreadProc(_In_ LPVOID lpParameter)
// Аннотации 

//_in_там указывается, является ли параметр входом или выходом ( _out_) для функции / метода.
{
	HWND hWnd = NULL;
	WNDCLASSEXW wx; 
	ZeroMemory(&wx, sizeof(wx));

	wx.cbSize = sizeof(WNDCLASSEX);
	wx.lpfnWndProc = (WNDPROC)(WndProc);
	wx.lpszClassName = (LPCWSTR)CLS_NAME;

	GUID guid = GUID_DEVINTERFACE_USB_DEVICE; 

	if (RegisterClassExW(&wx)) 
		hWnd = CreateWindowW((LPCWSTR)CLS_NAME, (LPCWSTR)("DevNotifWnd"), WS_ICONIC, 0, 0, CW_USEDEFAULT, 0, 0, NULL, GetModuleHandle(0), (void*)&guid);
	

	DEV_BROADCAST_DEVICEINTERFACE_A filter;
	filter.dbcc_size = sizeof(filter);
	filter.dbcc_classguid = guid;
	filter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
	RegisterDeviceNotificationW(hWnd, &filter, DEVICE_NOTIFY_WINDOW_HANDLE);

	{
		HDEVINFO devicesHandle = SetupDiGetClassDevsA(&GUID_DEVINTERFACE_USB_DEVICE, NULL, NULL, DIGCF_DEVICEINTERFACE | DIGCF_PRESENT); 
		SP_DEVINFO_DATA deviceInfo;
		ZeroMemory(&deviceInfo, sizeof(SP_DEVINFO_DATA));
		deviceInfo.cbSize = sizeof(SP_DEVINFO_DATA);
		DWORD deviceNumber = 0;
		SP_DEVICE_INTERFACE_DATA devinterfaceData; 
		devinterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
		while (SetupDiEnumDeviceInterfaces(devicesHandle, NULL, &GUID_DEVINTERFACE_USB_DEVICE, deviceNumber++, &devinterfaceData)) 
		{
			DWORD bufSize = 0;
			SetupDiGetDeviceInterfaceDetailW(devicesHandle, &devinterfaceData, NULL, NULL, &bufSize, NULL);
			BYTE* buffer = new BYTE[bufSize];
			PSP_DEVICE_INTERFACE_DETAIL_DATA_W devinterfaceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA_W)buffer;
			devinterfaceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA_W);
			SetupDiGetDeviceInterfaceDetailW(devicesHandle, &devinterfaceData, devinterfaceDetailData, bufSize, NULL, NULL);


			wchar_t* name = devinterfaceDetailData->DevicePath;

			DEVINST devInst;
			CM_Locate_DevNodeA(&devInst, (DEVINSTID_A)getInstId(devinterfaceDetailData->DevicePath).c_str(), CM_LOCATE_DEVNODE_NORMAL);

			HANDLE deviceHandle = CreateFileW(name, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			DEV_BROADCAST_HANDLE deviceFilter;
			deviceFilter.dbch_size = sizeof(deviceFilter);
			deviceFilter.dbch_devicetype = DBT_DEVTYP_HANDLE;
			deviceFilter.dbch_handle = deviceHandle;
			HDEVNOTIFY notifyHandle = RegisterDeviceNotificationW(hWnd, &deviceFilter, DEVICE_NOTIFY_WINDOW_HANDLE); 
			CloseHandle(deviceHandle);

			UsbDeviceDesc tmpUsbDev;
			tmpUsbDev.devInst = devInst;
			tmpUsbDev.friendlyName = getFriendlyName(name);
			tmpUsbDev.handle = deviceHandle;
			tmpUsbDev.name = name;
			tmpUsbDev.safety = false;
			tmpUsbDev.removable = getRemoveability(name);
			deviceArray.push_back(tmpUsbDev);
		}
	}

	MSG msg;
	while (GetMessageW(&msg, hWnd, 0, 0))
	{
		TranslateMessage(&msg); 
		DispatchMessage(&msg);
	}
	return 0;
}

int main()
{
	SetConsoleCP(1251);
	SetConsoleOutputCP(1251);
	cout << "Shift + 1 - Список устройств." << endl << "Shift + 2 - Отключение устройства." << endl << "Shift + 0 - Выход." << endl;	
	CreateThread(NULL, NULL, ThreadProc, NULL, NULL, NULL);
	RegisterHotKey(NULL, 0, MOD_SHIFT, 0x31);
	RegisterHotKey(NULL, 1, MOD_SHIFT, 0x32);
	RegisterHotKey(NULL, 2, MOD_SHIFT, 0x30);
	MSG msg = { 0 };
	while (1)
	{
		while (GetMessageA(&msg, NULL, 0, 0) != 0)
		{
			if (msg.message == WM_HOTKEY)
			{
				if (msg.wParam == 0)
				{
					int id = 1;
					cout << endl << "Список подключённых USB устройств : " << endl;
					for (auto it : deviceArray)
						cout << id++ << " - " << it.friendlyName << (it.removable ? " (Можно отключить)" : "") << endl;
					cout << endl;
				}
				if (msg.wParam == 1)
				{
					cout << "Введите номер устройства из списка для удаления: ";
					int choise = 0;
					while (!(cin >> choise))
					{
						
						cin.clear();
						rewind(stdin);
					}
					if (choise > deviceArray.size() || choise <= 0)
					{
						cout << "Нет устройства по данному номеру в списке" << endl;
						break;
					}
					if (deviceArray[choise - 1].removable)
						CM_Request_Device_EjectW(deviceArray[choise - 1].devInst, NULL, NULL, NULL, NULL);
					else
						cout << "Это устройство нельзя отключить!" << endl;
					Sleep(300);
				}
				if (msg.wParam == 2)
				{
					UnregisterHotKey(NULL, 0);
					UnregisterHotKey(NULL, 1);
					UnregisterHotKey(NULL, 2);
					return 0;
				}
			}
		}
	}
	return 0;
}