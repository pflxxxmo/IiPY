#include <iostream>
#include <iomanip>
#include <windows.h>
#include <ntddscsi.h>
#include <conio.h>
#include <sstream>

using namespace std;

#define BYTE_SIZE 8

string busType[] = { "UNKNOWN", "SCSI", "ATAPI", "ATA", "ONE_TREE_NINE_FOUR", "SSA", "FIBRE", "USB", "RAID", "ISCSI", "SAS", "SATA", "SD", "MMC" };

void getDeviceInfo(HANDLE diskHandle, STORAGE_PROPERTY_QUERY storageProtertyQuery)
{
	STORAGE_DESCRIPTOR_HEADER storageDescriptorHeader = { 0 };
	DWORD dwBytesReturned = 0;
	if (!DeviceIoControl(diskHandle, IOCTL_STORAGE_QUERY_PROPERTY,
		&storageProtertyQuery, sizeof(STORAGE_PROPERTY_QUERY),
		&storageDescriptorHeader, sizeof(STORAGE_DESCRIPTOR_HEADER),
		&dwBytesReturned, NULL))
	{
		printf("%d", GetLastError());
		CloseHandle(diskHandle);
		exit(-1);
	}


	
	const DWORD dwOutBufferSize = storageDescriptorHeader.Size;
	BYTE* pOutBuffer = new BYTE[dwOutBufferSize];
	ZeroMemory(pOutBuffer, dwOutBufferSize);


	
	if (!DeviceIoControl(diskHandle, IOCTL_STORAGE_QUERY_PROPERTY,
		&storageProtertyQuery, sizeof(STORAGE_PROPERTY_QUERY),
		pOutBuffer, dwOutBufferSize,
		&dwBytesReturned, NULL))
	{
		printf("%d", GetLastError());
		CloseHandle(diskHandle);
		exit(-1);
	}

	STORAGE_DEVICE_DESCRIPTOR *pDeviceDescriptor = (STORAGE_DEVICE_DESCRIPTOR*)pOutBuffer;
	const DWORD dwSerialNumberOffset = pDeviceDescriptor->SerialNumberOffset;
	const DWORD dwModelOffset = pDeviceDescriptor->ProductIdOffset;
	const DWORD dwProductRevisionOffset = pDeviceDescriptor->ProductRevisionOffset;


	if (dwSerialNumberOffset != 0)
	{
		cout << "====================================================================" << endl;
		cout << "|" << setw(48) << "Hard disk drive information" << setw(19) << "|" << endl;
		cout << "====================================================================" << endl;

		cout << "|" << setw(12) << "Model" << setw(9) << "|" << (char*)(pOutBuffer + dwModelOffset) << setw(28) << "|" << endl;
		cout << "--------------------------------------------------------------------" << endl;

		cout << "|" << setw(16) << "Manufacture" << setw(5) << "|" << "Seagate Technology LLC" << setw(24) << "|" << endl;
		cout << "--------------------------------------------------------------------" << endl;

		cout << "|" << setw(17) << "Serial number" << setw(4) << "|" << (char*)(pOutBuffer + dwSerialNumberOffset) << setw(26) << "|" << endl;
		cout << "--------------------------------------------------------------------" << endl;

		cout << "|" << setw(14) << "Firmware" << setw(7) << "|" << (char*)(pOutBuffer + dwProductRevisionOffset) << setw(42) << "|" << endl;
		cout << "--------------------------------------------------------------------" << endl;

		cout << "|" << setw(16) << "Interface type" << setw(5) << "|" << busType[pDeviceDescriptor->BusType] << setw(42) << "|" << endl;
		cout << "--------------------------------------------------------------------" << endl;
	}
}

void getMemoryInfo() {

	char dd[5];

	unsigned __int64 totalNumberOfBytes = 0;
	unsigned __int64 totalNumberOfBytesOnDisk = 0;

	unsigned __int64 totalNumberOfFreeBytes = 0;
	unsigned __int64 totalNumberOfFreeBytesOnDisk = 0;

	unsigned __int64 freeBytesAvailable = 0;
	unsigned __int64 freeBytesAvailableOnDisk = 0;

	for (int i = 0; i < 26; i++)
	{
		dd[0] = char(65 + i); dd[1] = ':'; dd[2] = '\\'; dd[3] = '\\'; dd[4] = '\0';
		bool GetDiskFreeSpaceFlag = GetDiskFreeSpaceExA(dd,
			(PULARGE_INTEGER)&freeBytesAvailableOnDisk,
			(PULARGE_INTEGER)&totalNumberOfBytesOnDisk,
			(PULARGE_INTEGER)&totalNumberOfFreeBytesOnDisk);
		if (GetDiskFreeSpaceFlag != 0)
		{
			totalNumberOfBytes += totalNumberOfBytesOnDisk;
			totalNumberOfFreeBytes += totalNumberOfFreeBytesOnDisk;
		}
	}

	cout << "|" << setw(16) << "Total memory" << setw(5) << "|" << totalNumberOfBytes << " bytes(" << (double)totalNumberOfBytes / (1024 * 1024 * 1024) << " Gb)" << setw(16) << "|" << endl;
	cout << "--------------------------------------------------------------------" << endl;

	cout << "|" << setw(15) << "Free memory" << setw(6) << "|" << totalNumberOfFreeBytes << " bytes(" << (double)(totalNumberOfFreeBytes) / (1024 * 1024 * 1024) << " Gb)" << setw(16) << "|" << endl;
	cout << "--------------------------------------------------------------------" << endl;

	cout << "|" << setw(17) << "Occupied memory" << setw(4) << "|" << totalNumberOfBytes - totalNumberOfFreeBytes << " bytes(" << (double)(totalNumberOfBytes - totalNumberOfFreeBytes) / (1024 * 1024 * 1024) << " Gb)" << setw(16) << "|" << endl;
	cout << "--------------------------------------------------------------------" << endl;
}

void getAtaPioDmaSupportStandarts(HANDLE diskHandle) {

	UCHAR identifyDataBuffer[512 + sizeof(ATA_PASS_THROUGH_EX)] = { 0 };

	ATA_PASS_THROUGH_EX &PTE = *(ATA_PASS_THROUGH_EX*)identifyDataBuffer;	
	PTE.Length = sizeof(PTE);
	PTE.TimeOutValue = 10;									
	PTE.DataTransferLength = 512;							
	PTE.DataBufferOffset = sizeof(ATA_PASS_THROUGH_EX);		 
	PTE.AtaFlags = ATA_FLAGS_DATA_IN;						

	IDEREGS *ideRegs = (IDEREGS*)PTE.CurrentTaskFile;
	ideRegs->bCommandReg = 0xEC;

	
	if (!DeviceIoControl(diskHandle,
		IOCTL_ATA_PASS_THROUGH,								
		&PTE,
		sizeof(identifyDataBuffer),
		&PTE,
		sizeof(identifyDataBuffer),
		NULL,
		NULL)) {
		cout << GetLastError() << std::endl;
		return;
	}
	 
	WORD* data = (WORD*)(identifyDataBuffer + sizeof(ATA_PASS_THROUGH_EX));
	short ataSupportByte = data[80];
	int i = 2 * BYTE_SIZE;
	int bitArray[2 * BYTE_SIZE];
	
	while (i--) {
		bitArray[i] = ataSupportByte & 32768 ? 1 : 0;
		ataSupportByte = ataSupportByte << 1;
	}


	cout << "|" << setw(17) << "Supported modes" << setw(4) << "|" << setw(1) << "ATA Support:";
	 
	for (int i = 8; i >= 4; i--) {
		if (bitArray[i] == 1) {
			cout << "ATA" << i;
			if (i != 5) {
				cout << ", ";
			}
		}
	}
	cout << setw(12) << "|" << endl;

	
	unsigned short dmaSupportedBytes = data[63];
	int i2 = 2 * BYTE_SIZE;
	
	while (i2--) {
		bitArray[i2] = dmaSupportedBytes & 32768 ? 1 : 0;
		dmaSupportedBytes = dmaSupportedBytes << 1;
	}


	
	cout << "|" << setw(21) << "|" << "DMA Support:";
	for (int i = 0; i < 8; i++) {
		if (bitArray[i] == 1) {
			cout << "DMA" << i;
			if (i != 2) cout << ", ";
		}
	}
	cout << setw(18) << "|" << endl;

	unsigned short pioSupportedBytes = data[63];
	int i3 = 2 * BYTE_SIZE;
	 
	while (i3--) {
		bitArray[i3] = pioSupportedBytes & 32768 ? 1 : 0;
		pioSupportedBytes = pioSupportedBytes << 1;
	}

	
	cout << "|" << setw(21) << "|" << "PIO Support:";
	for (int i = 0; i < 2; i++) {
		if (bitArray[i] == 1) {
			cout << "PIO" << i + 3;
			if (i != 1) cout << ", ";
		}
	}
	cout << setw(24) << "|" << endl;
}

void getMemoryTransferMode(HANDLE diskHandle, STORAGE_PROPERTY_QUERY storageProtertyQuery) {
	STORAGE_ADAPTER_DESCRIPTOR adapterDescriptor;			
	if (!DeviceIoControl(diskHandle,
		IOCTL_STORAGE_QUERY_PROPERTY,						 
		&storageProtertyQuery,
		sizeof(storageProtertyQuery),
		&adapterDescriptor,
		sizeof(STORAGE_DESCRIPTOR_HEADER),
		NULL,
		NULL)) {
		cout << GetLastError();
		exit(-1);
	}
	else {
		cout << "|" << setw(21) << "|" << "Transfer mode:";
		adapterDescriptor.AdapterUsesPio ? cout << "PIO" : cout << "DMA";
		cout << setw(29) << "|" << endl;
		cout << "====================================================================" << endl;
	}
}

void init(HANDLE& diskHandle, char* name) { 
	diskHandle = CreateFile(name, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);
	if (diskHandle == INVALID_HANDLE_VALUE) {
		cout << "all disks are shown";
		_getch();
		return;
	}
}

int main()
{
	STORAGE_PROPERTY_QUERY storagePropertyQuery;				
	ZeroMemory(&storagePropertyQuery, sizeof(STORAGE_PROPERTY_QUERY));
	HANDLE diskHandle;

	const string name = "//./PhysicalDrive";
	int number = 0;

	stringstream* res_name = new stringstream;

	*res_name << name << number;

	init(diskHandle, (char*)res_name->str().c_str());

	delete res_name;
	res_name = new stringstream;

	getDeviceInfo(diskHandle, storagePropertyQuery);
	getMemoryInfo();
	getAtaPioDmaSupportStandarts(diskHandle);
	getMemoryTransferMode(diskHandle, storagePropertyQuery);

	_getch();
	cout << endl;

	return 0;
}