#include "AES.h"

int ClientInit(SOCKET &ConnectSocket, PCSTR ip, PCSTR port) {
	WSADATA wsaData;
	struct addrinfo *result = NULL, *ptr = NULL, hints;
	int iResult;

	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		return -1;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	iResult = getaddrinfo(ip, port, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return -1;
	}

	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

		// Create a SOCKET for connecting to server
		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
			ptr->ai_protocol);
		if (ConnectSocket == INVALID_SOCKET) {
			printf("socket failed with error: %ld\n", WSAGetLastError());
			WSACleanup();
			return -1;
		}

		iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			closesocket(ConnectSocket);
			ConnectSocket = INVALID_SOCKET;
			continue;
		}
		break;
	}

	freeaddrinfo(result);

	if (ConnectSocket == INVALID_SOCKET) {
		printf("Unable to connect to server!\n");
		WSACleanup();
		return -1;
	}

	return 0;
}

int ServerInit(SOCKET &ListenSocket, struct addrinfo *result, PCSTR port) {
	WSADATA wsaData;
	ListenSocket = INVALID_SOCKET;
	result = NULL;
	struct addrinfo hints;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		return -1;
	}
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	iResult = getaddrinfo(NULL, port, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return -1;
	}

	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {
		printf("socket failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return -1;
	}

	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return -1;
	}

	freeaddrinfo(result);

	iResult = listen(ListenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR) {
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return -1;
	}

	return 0;
}

int Accept(SOCKET &ListenSocket, SOCKET &ClientSocket) {
	ClientSocket = accept(ListenSocket, NULL, NULL);
	if (ClientSocket == INVALID_SOCKET) {
		printf("accept failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return -1;
	}

	return 0;
}

int Receive(BYTE chunk[], DWORD &out_len, SOCKET &ClientSocket) {
	int iResult = recv(ClientSocket, (char *)&out_len, sizeof(out_len), 0);
	if (out_len > 0)
		iResult = recv(ClientSocket, (char *)chunk, out_len, 0);

	return 0;
}

int Send(BYTE chunk[], DWORD &out_len, SOCKET &ConnectSocket) {
	int iResult = send(ConnectSocket, (char *)&out_len, sizeof(out_len), 0);
	if (out_len > 0)
		iResult = send(ConnectSocket, (char *)chunk, out_len, 0);

	return 0;
}

int SendFile(wchar_t *file, SOCKET &socket) {
	HANDLE hInpFile = CreateFileW(file, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (hInpFile == INVALID_HANDLE_VALUE) {
		printf("Cannot open input file!\n");
		return (-1);
	}

	BOOL bResult = FALSE;
	const size_t chunk_size = BLOCK_LEN;
	BYTE chunk[chunk_size] = { 0 };
	DWORD out_len = 0;

	while (bResult = ReadFile(hInpFile, chunk, chunk_size, &out_len, NULL)) {
		Send(chunk, out_len, socket);
		if (0 == out_len) break;
	}

	CloseHandle(hInpFile);

	return 0;
}

int ReceiveFile(wchar_t *file, SOCKET &socket) {
	HANDLE hOutFile = CreateFileW(file, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hOutFile == INVALID_HANDLE_VALUE) {
		printf("Cannot open output file!\n");
		system("pause");
		return (-1);
	}

	BOOL bResult = FALSE;
	const size_t chunk_size = BLOCK_LEN;
	BYTE chunk[chunk_size] = { 0 };
	DWORD out_len = 0;

	while (true) {
		Receive(chunk, out_len, socket);
		if (0 == out_len) break;

		DWORD written = 0;
		if (!WriteFile(hOutFile, chunk, out_len, &written, NULL)) {
			printf("writing failed!\n");
			break;
		}
	}

	CloseHandle(hOutFile);

	return 0;
}

DWORD AES_Encrypt(wchar_t *scr, wchar_t *keyfile, wchar_t *des) {
	HANDLE file = CreateFileW(keyfile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (file == INVALID_HANDLE_VALUE) {
		printf("Cannot open key file!\n");
		return (-1);
	}

	wchar_t *key_str = NULL;
	DWORD out_len = 0;
	ReadFile(file, key_str, 32, &out_len, NULL);
	CloseHandle(file);
	size_t len = lstrlenW(key_str);

	HANDLE hInpFile = CreateFileW(scr, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (hInpFile == INVALID_HANDLE_VALUE) {
		printf("Cannot open input file!\n");
		return (-1);
	}

	HANDLE hOutFile = CreateFileW(des, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hOutFile == INVALID_HANDLE_VALUE) {
		printf("Cannot open output file!\n");
		system("pause");
		return (-1);
	}

	DWORD dwStatus = 0;
	BOOL bResult = FALSE;
	wchar_t info[] = L"Microsoft Enhanced RSA and AES Cryptographic Provider";
	HCRYPTPROV hProv;
	if (!CryptAcquireContextW(&hProv, NULL, info, PROV_RSA_AES, CRYPT_VERIFYCONTEXT)) {
		dwStatus = GetLastError();
		printf("CryptAcquireContext failed: %x\n", dwStatus);
		CryptReleaseContext(hProv, 0);
		return dwStatus;
	}
	HCRYPTHASH hHash;
	if (!CryptCreateHash(hProv, CALG_SHA_256, 0, 0, &hHash)) {
		dwStatus = GetLastError();
		printf("CryptCreateHash failed: %x\n", dwStatus);
		CryptReleaseContext(hProv, 0);
		return dwStatus;
	}

	if (!CryptHashData(hHash, (BYTE*)key_str, len, 0)) {
		DWORD err = GetLastError();
		printf("CryptHashData Failed : %#x\n", err);
		return (-1);
	}

	HCRYPTKEY hKey;
	if (!CryptDeriveKey(hProv, CALG_AES_256, hHash, 0, &hKey)) {
		dwStatus = GetLastError();
		printf("CryptDeriveKey failed: %x\n", dwStatus);
		CryptReleaseContext(hProv, 0);
		return dwStatus;
	}

	const size_t chunk_size = BLOCK_LEN;
	BYTE chunk[chunk_size] = { 0 };
	out_len = 0;

	BOOL isFinal = FALSE;
	DWORD readTotalSize = 0;

	DWORD inputSize = GetFileSize(hInpFile, NULL);

	while (bResult = ReadFile(hInpFile, chunk, chunk_size, &out_len, NULL)) {
		if (0 == out_len) {
			break;
		}
		readTotalSize += out_len;
		if (readTotalSize == inputSize) {
			isFinal = TRUE;
		}

		if (!CryptEncrypt(hKey, NULL, isFinal, 0, chunk, &out_len, chunk_size)) {
			printf("[-] CryptEncrypt failed\n");
			break;
		}
		DWORD written = 0;
		if (!WriteFile(hOutFile, chunk, out_len, &written, NULL)) {
			printf("writing failed!\n");
			break;
		}

		memset(chunk, 0, chunk_size);
	}

	CryptReleaseContext(hProv, 0);
	CryptDestroyKey(hKey);
	CryptDestroyHash(hHash);

	CloseHandle(hInpFile);
	CloseHandle(hOutFile);

	return 0;
}

DWORD AES_Decrypt(wchar_t *scr, wchar_t *keyfile, wchar_t *des) {
	HANDLE file = CreateFileW(keyfile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (file == INVALID_HANDLE_VALUE) {
		printf("Cannot open key file!\n");
		return (-1);
	}

	wchar_t *key_str = NULL;
	DWORD out_len = 0;
	ReadFile(file, key_str, 32, &out_len, NULL);
	CloseHandle(file);

	size_t len = lstrlenW(key_str);

	HANDLE hInpFile = CreateFileW(scr, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (hInpFile == INVALID_HANDLE_VALUE) {
		printf("Cannot open input file!\n");
		return (-1);
	}

	HANDLE hOutFile = CreateFileW(des, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hOutFile == INVALID_HANDLE_VALUE) {
		printf("Cannot open output file!\n");
		system("pause");
		return (-1);
	}

	DWORD dwStatus = 0;
	BOOL bResult = FALSE;
	wchar_t info[] = L"Microsoft Enhanced RSA and AES Cryptographic Provider";
	HCRYPTPROV hProv;
	if (!CryptAcquireContextW(&hProv, NULL, info, PROV_RSA_AES, CRYPT_VERIFYCONTEXT)) {
		dwStatus = GetLastError();
		printf("CryptAcquireContext failed: %x\n", dwStatus);
		CryptReleaseContext(hProv, 0);
		return dwStatus;
	}
	HCRYPTHASH hHash;
	if (!CryptCreateHash(hProv, CALG_SHA_256, 0, 0, &hHash)) {
		dwStatus = GetLastError();
		printf("CryptCreateHash failed: %x\n", dwStatus);
		CryptReleaseContext(hProv, 0);
		return dwStatus;
	}

	if (!CryptHashData(hHash, (BYTE*)key_str, len, 0)) {
		DWORD err = GetLastError();
		printf("CryptHashData Failed : %#x\n", err);
		return (-1);
	}

	HCRYPTKEY hKey;
	if (!CryptDeriveKey(hProv, CALG_AES_256, hHash, 0, &hKey)) {
		dwStatus = GetLastError();
		printf("CryptDeriveKey failed: %x\n", dwStatus);
		CryptReleaseContext(hProv, 0);
		return dwStatus;
	}

	const size_t chunk_size = BLOCK_LEN;
	BYTE chunk[chunk_size] = { 0 };
	out_len = 0;

	BOOL isFinal = FALSE;
	DWORD readTotalSize = 0;

	DWORD inputSize = GetFileSize(hInpFile, NULL);

	while (bResult = ReadFile(hInpFile, chunk, chunk_size, &out_len, NULL)) {
		if (0 == out_len) {
			break;
		}

		readTotalSize += out_len;
		if (readTotalSize == inputSize) {
			isFinal = TRUE;
		}

		if (!CryptDecrypt(hKey, NULL, isFinal, 0, chunk, &out_len)) {
			printf("[-] CryptDecrypt failed\n");
			break;
		}

		DWORD written = 0;
		if (!WriteFile(hOutFile, chunk, out_len, &written, NULL)) {
			printf("writing failed!\n");
			break;
		}
		memset(chunk, 0, chunk_size);
	}

	CryptReleaseContext(hProv, 0);
	CryptDestroyKey(hKey);
	CryptDestroyHash(hHash);

	CloseHandle(hInpFile);
	CloseHandle(hOutFile);

	return 0;
}