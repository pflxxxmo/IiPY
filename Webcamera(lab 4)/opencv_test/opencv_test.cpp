#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <stdio.h>
#include "locale.h"
#include <setupapi.h>
#include <devguid.h>
#include <iostream>
#include <iomanip>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>


using namespace std;
using namespace cv;

#pragma comment(lib, "setupapi.lib")


//q - 81
//s - 83
//f - 70
//v - 86

void makePhoto();
void makeVidio();

void printCameraInfo()
{
	SP_DEVINFO_DATA DeviceInfoData;
	ZeroMemory(&DeviceInfoData, sizeof(SP_DEVINFO_DATA));
	DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
	HDEVINFO DeviceInfoSet = SetupDiGetClassDevsA(&GUID_DEVCLASS_IMAGE, "USB", NULL, DIGCF_PRESENT);
	if (DeviceInfoSet == INVALID_HANDLE_VALUE) {
		exit(1);
	}
	DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
	SetupDiEnumDeviceInfo(DeviceInfoSet, 0, &DeviceInfoData);
	const int PropertyBufferSize = 1024;

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

	cout <<"Name: "<< deviceName<<'\n' 
		 << "Manufacture: "<< companyName<<'\n' 
		 << "Vendor ID: "<< venAndDevId.substr(8, 4).c_str()<<'\n'
		 << "Device ID: "<< venAndDevId.substr(17, 4).c_str() << '\n';

}

int main()
{
	setlocale(LC_ALL,"");
	::ShowWindow(::GetConsoleWindow(), SW_HIDE);
	while (true)
	{
		if (GetAsyncKeyState(70))
		{
			cout << 'f' << endl;
			makePhoto();
			Sleep(100);
		}
		if (GetAsyncKeyState(86))
		{
			cout << 'v' << endl;
			makeVidio();
			Sleep(100);
		}
		if (GetAsyncKeyState(81))
		{
			Sleep(100);
			break;
		}
		if (GetAsyncKeyState(VK_ESCAPE))
		{
			VideoCapture cap(0);
			system("CLS");
			::ShowWindow(::GetConsoleWindow(), SW_SHOW);

			printCameraInfo();

			getchar();
			::ShowWindow(::GetConsoleWindow(), SW_HIDE);

		}
	}
	return 0;
}

void makePhoto()
{
	VideoCapture cap(0);
	if (cap.isOpened())
	{
		Mat img;
		cap >> img;
		if (!img.empty())
		{
			String fileName = "E:\\" + to_string(rand()) + ".jpg";
			imwrite(fileName, img);
		}
	}
}

void makeVidio()
{
	VideoCapture cap(0);
	static int counter;
	double dWidth = cap.get(CAP_PROP_FRAME_WIDTH);
	double dHeight = cap.get(CAP_PROP_FRAME_HEIGHT);

	VideoWriter video("E:\\" + to_string(rand()) + ".avi", VideoWriter::fourcc('M', 'J', 'P', 'G'), 25, Size(dWidth, dHeight));
	while (!GetAsyncKeyState(83))
	{
		Mat frame;
		cap >> frame;
		if (frame.empty())
			break;
		video.write(frame);
	}
	cap.release();
	video.release();
}