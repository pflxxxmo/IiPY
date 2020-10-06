#pragma comment(lib, "PowrProf.lib")
#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <WinBase.h>
#include <iostream>
#include <iomanip>
#include <powrprof.h>
#include <thread>   
#include <conio.h>


using namespace std;

int a;

void BatteryFlagStatus(SYSTEM_POWER_STATUS systemPowerStatus)
{
	switch (systemPowerStatus.BatteryFlag)
	{
		case 1:
			cout << "|High-the battery capacity is at more than 66%" << setw(7) << "|" << endl;
			break;
		case 2:
			cout << "|Low-the battery capacity is at less than 33%" << setw(6) << "|" << endl;
			break;
		case 4:
			cout << "|Critical-the battery capacity is at less than 5%" << setw(2) << "|" << endl;
			break;
		case 8:
			cout << "|Charging" << setw(42) << "|" << endl;
			break;
		case 128:
			cout << "|No system battery" << setw(33) << "|" << endl;
			break;
		case 255:
			cout << "|Unknown status" << setw(36) << "|" << endl;
			break;
	}
}

void getBatteryInfo()
{
	SYSTEM_POWER_STATUS systemPowerStatus;
	while (a!=3) {
		system("cls");

		cout << "__________________________________________________" << endl;

		GetSystemPowerStatus(&systemPowerStatus);
		cout << "|Battery charge: " << (int)systemPowerStatus.BatteryLifePercent << "%"<<setw(31)<<"|" << endl;
		if (systemPowerStatus.BatteryLifeTime != -1) {
			char minutes[3];
			sprintf(minutes, "%02d", systemPowerStatus.BatteryLifeTime / 60 % 60);
			cout << "|Remaining battery time: " << systemPowerStatus.BatteryLifeTime / 60 / 60 << "hours " << minutes << "minutes" << setw(10) << "|" << endl;
		}
		if (systemPowerStatus.ACLineStatus == 1)
			cout << "|The device is charging." << setw(27) << "|" << endl;
		else if (systemPowerStatus.ACLineStatus == 0)
			cout << "|The device is running on battery." << setw(17) << "|" << endl;
		else if (systemPowerStatus.ACLineStatus == 255)
			cout << "|Unknown status." << setw(35) << "|" << endl;
		if (systemPowerStatus.SystemStatusFlag == 0)
			cout << "|Battery saver is off." << setw(29) << "|" << endl;
		else if (systemPowerStatus.SystemStatusFlag == 1)
			cout << "|Battery saver on."<< setw(33) << "|" << endl;
		BatteryFlagStatus(systemPowerStatus);

		cout << "|_________________________________________________|" << endl;
		cout << "|Press 1 to Sleep" << setw(34) << "|" << endl;
		cout << "|Press 2 to Hibernate" << setw(30) << "|" << endl;
		cout << "|Press 3 to Exit" << setw(35) << "|" << endl;
		cout << "|_________________________________________________|" << endl;
		Sleep(1000);
	}
}


int main()
{
	thread log(getBatteryInfo);

	while (a != 3) {
		if (a = _getch()) {
			a -= '0';
			switch (a) {

			case(1):
				SetSuspendState(FALSE, FALSE, FALSE);
				break;

			case(2):
				SetSuspendState(TRUE, FALSE, FALSE);
				break;
			}
		}
	}
	log.join();
}