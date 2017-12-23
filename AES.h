#pragma once

#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <wincrypt.h>
#include <stdio.h>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>

#pragma comment(lib, "crypt32.lib")
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

using namespace std;

#define BLOCK_LEN 256

int ClientInit(SOCKET &ConnectSocket, PCSTR ip, PCSTR port); // Tạo 1 client kết nối đến server

int ServerInit(SOCKET &ListenSocket, struct addrinfo *result, PCSTR port); // Tạo 1 server lắng nghe

int Accept(SOCKET &ListenSocket, SOCKET &ClientSocket); // Nhận kết nối từ 1 client

int Receive(BYTE chunk[], DWORD &out_len, SOCKET &ClientSocket); // Nhận dữ liệu

int Send(BYTE chunk[], DWORD &out_len, SOCKET &ConnectSocket); // Gửi dữ liệu

int SendFile(wchar_t *file, SOCKET &socket);

int ReceiveFile(wchar_t *file, SOCKET &socket);

DWORD AES_Encrypt(wchar_t *scr, wchar_t *keyfile, wchar_t *des); // Mã hóa bằng AES 256

DWORD AES_Decrypt(wchar_t *scr, wchar_t *keyfile, wchar_t *des); // Giải mã bằng AES 256