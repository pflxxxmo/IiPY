#pragma comment (lib, "Setupapi.lib")
#include <iostream>
#include <iomanip>
#include <windows.h>
#include <setupapi.h>
#include <locale.h>
#include <tchar.h>


using namespace std;

int main()
{
	setlocale(LC_ALL, "Russian");
	HDEVINFO DeviceInfoSet = NULL;
	SP_DEVINFO_DATA DeviceInfoData;

	DeviceInfoSet = SetupDiGetClassDevs(NULL, TEXT("PCI"), NULL, DIGCF_PRESENT | DIGCF_ALLCLASSES);
	ZeroMemory(&DeviceInfoData, sizeof(SP_DEVINFO_DATA));
	DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

	DWORD DeviceIndex = 0;
	const int PropertyBufferSize=1024;

	cout << setw(3) << "¹" << setw(2) << "|" << setw(45) << "Device Name" << setw(47) << "|" << setw(24) << "Company Name" << setw(17) << "|" << setw(10) << "Vendor ID" << setw(2) << "|" << setw(10) << "Device ID" << setw(2) << "|" << endl;
	cout << "==================================================================================================================================================================" << endl;

	while (SetupDiEnumDeviceInfo(DeviceInfoSet, DeviceIndex, &DeviceInfoData))
	{
		DeviceIndex++;
		TCHAR deviceID[PropertyBufferSize], deviceName[PropertyBufferSize], companyName[PropertyBufferSize];
		ZeroMemory(&deviceID, sizeof(deviceID));
		ZeroMemory(&deviceName, sizeof(deviceName));
		ZeroMemory(&companyName, sizeof(companyName));

		if (!SetupDiGetDeviceInstanceId(DeviceInfoSet, &DeviceInfoData, deviceID, sizeof(deviceID), NULL))
		{
			cout << GetLastError();
		}

		if (!SetupDiGetDeviceRegistryProperty(DeviceInfoSet, &DeviceInfoData, SPDRP_DEVICEDESC, NULL, (PBYTE)deviceName, sizeof(deviceName), NULL))
		{
			cout << GetLastError();
		}

		if (!SetupDiGetDeviceRegistryProperty(DeviceInfoSet, &DeviceInfoData, SPDRP_MFG, NULL, (PBYTE)companyName, sizeof(companyName), NULL))
		{
			cout << GetLastError();
		}
		
		string venAndDevId(deviceID);

		cout << setw(3) << DeviceIndex << setw(2) << "|" << setw(90) << deviceName << setw(2) << "|" << setw(39) << companyName << setw(2) <<  "|" << setw(8) << venAndDevId.substr(8, 4).c_str() << setw(4) << "|" << setw(8) << venAndDevId.substr(17, 4).c_str() << setw(4) << "|" << endl;
	}
	cout << "==================================================================================================================================================================" << endl;

	if (DeviceInfoSet) {
		SetupDiDestroyDeviceInfoList(DeviceInfoSet);
	}

	return 0;
}