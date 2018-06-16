#include <conio.h>
#include <Windows.h>
#include "stdafx.h"
#include <winsock.h>	// Use winsock.h if you're at WinSock 1.1
#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>
#pragma comment(lib, "Ws2_32.lib")

using namespace std;

bool mode = 1;				// 0 - Passive && 1 - Active
void SetColor(int);
SOCKET sendPort(SOCKET, string);
void logInServer(SOCKET);
void downloadFile(string, SOCKET, string);
void uploadFile(string, SOCKET, string);
void deleteFile(string, SOCKET);
string nlst(SOCKET, string, string);
void list(SOCKET, string, string);
void printHeader();
void printFooter();
long GetFileSize(std::string);
void replylogcode(int);
void errexit(const char *, ...);
void pause();

int _tmain(int argc, char* argv[])
{
	printHeader();

	WORD wVersionRequested;
	WSADATA wsaData;
	int retcode;
	printf("WSAStartup()\n");

	wVersionRequested = MAKEWORD(2, 2);	// Use MAKEWORD(1,1) if you're at WinSock 1.1
	retcode = WSAStartup(wVersionRequested, &wsaData);
	if (retcode != 0)
		errexit("Startup failed: %d\n", retcode);

	printf("Return Code: %i\n", retcode);
	printf("Version Used: %i.%i\n", LOBYTE(wsaData.wVersion), HIBYTE(wsaData.wVersion));
	printf("Version Supported:  %i.%i\n", LOBYTE(wsaData.wHighVersion), HIBYTE(wsaData.wHighVersion));
	printf("Implementation: %s\n", wsaData.szDescription);
	printf("System Status: %s\n", wsaData.szSystemStatus);
	printf("\n");

	if (LOBYTE(wsaData.wVersion) != LOBYTE(wVersionRequested) ||
		HIBYTE(wsaData.wVersion) != HIBYTE(wVersionRequested))
	{
		printf("Supported version is too low\n");
		WSACleanup();
		return 0;
	}

	// TODO

	// Get IP address & port
	string IP = "192.168.126.2";
	int port = 21;
	cout << "Enter IP address: ";
	getline(cin, IP);
	cin.clear();
	cout << "Enter port number: ";
	cin >> port;
	cin.ignore();

	// Initialize SOCKADDR_IN
	SOCKADDR_IN client;
	int clientSize = sizeof(client);					// Required for acepted call
	client.sin_addr.s_addr = inet_addr(IP.c_str());		// IP address
	client.sin_port = htons(port);						// Port
	client.sin_family = AF_INET;						// IPv4 Socket

														// Initialize socket to connect
	SOCKET socket_descriptor;
	socket_descriptor = socket(AF_INET, SOCK_STREAM, NULL);
	// Test socket
	if (socket_descriptor == INVALID_SOCKET) {
		cout << "Failed creating socket!!!" << endl;
		pause();
		exit(0);
	}

	// Connect to ftp server
	if (connect(socket_descriptor, (SOCKADDR*)&client, clientSize) != 0) {			// Cannot connect
																					// MessageBoxA(NULL, "Failed to Connect", "Error", MB_OK | MB_ICONERROR);
		cout << "Failed to connect!!!" << endl;
		pause();
		exit(0);
	}
	else
		cout << "Sucess to connect!!!" << endl;

	// Receive welcome message from the server
	char Buffer[4096];
	int code;
	memset(Buffer, '\0', sizeof(Buffer));						// Replace all element to '\0'
	if (recv(socket_descriptor, Buffer, 4095, NULL) > 0) {
		sscanf(Buffer, "%d", &code);
		if (code != 220) {										// Code for welcome message
			replylogcode(code);
			pause();
			exit(0);
		}
		else {
			SetColor(2);
			puts(Buffer);
			SetColor(15);
			memset(Buffer, '\0', sizeof(Buffer));				// Replace all element to '\0'
		}
	}

	// LOGIN
	logInServer(socket_descriptor);

	// ----- COMMAND LIST -----

	// Enter command
	while (1) {
		// Get command
		string command;
		cout << "ftp> ";
		getline(cin, command);
		if (command.length() == 0) {
			cout << endl;
			continue;
		}

		// Get function
		char cmd[8];
		memset(cmd, '\0', sizeof(cmd));
		sscanf(command.c_str(), "%s", &cmd);
		_strlwr(cmd);

		// ls command
		if (cmd[0] == 'l' && cmd[1] == 's') {
			if (command.length() > 3) {
				string file;
				for (int i = 0; i < command.length() - 1; i++) {
					if (command[i] == ' ' && command[i + 1] != ' ') {
						for (int j = i + 1; j < command.length(); j++) {
							if (command[j] == ' ' || command[j] == 0) {
								break;
							}
							file.push_back(command[j]);
						}
						// Print data
						string res = nlst(socket_descriptor, IP, file);
						SetColor(14);
						cout << res;
						SetColor(15);
						break;
					}
				}
				if (file.length() == 0) {
					string res = nlst(socket_descriptor, IP, "");
					SetColor(14);
					cout << res;
					SetColor(15);
				}
			}
			else {
				string res = nlst(socket_descriptor, IP, "");
				SetColor(14);
				cout << res;
				SetColor(15);
			}
		}

		// dir command
		if (cmd[0] == 'd' && cmd[1] == 'i' && cmd[2] == 'r') {
			if (command.length() > 4) {
				string file;
				for (int i = 0; i < command.length() - 1; i++) {
					if (command[i] == ' ' && command[i + 1] != ' ') {
						for (int j = i + 1; j < command.length(); j++) {
							if (command[j] == ' ' || command[j] == 0) {
								break;
							}
							file.push_back(command[j]);
						}
						list(socket_descriptor, IP, file);
						break;
					}
				}
				if (file.length() == 0)
					list(socket_descriptor, IP, "");
			}
			else
				list(socket_descriptor, IP, "");
		}

		// Upload file
		if (cmd[0] == 'p' && cmd[1] == 'u' && cmd[2] == 't') {
			if (command.length() <= 5) {
				// Get file's name
				string file;
				cout << "Enter the file's name (directory): ";
				getline(cin, file);
				cin.clear();
				uploadFile(file, socket_descriptor, IP);
			}
			else {
				string file;
				for (int i = 0; i < command.length(); i++) {
					if (command[i] == ' ' && command[i + 1] != ' ') {
						for (int j = i + 1; j < command.length(); j++)
							file.push_back(command[j]);
						break;
					}
				}
				uploadFile(file, socket_descriptor, IP);
			}
		}

		// Upload many files
		if (cmd[0] == 'm' && cmd[1] == 'p' && cmd[2] == 'u' && cmd[3] == 't') {
			if (command.length() > 5) {
				// Count number of files
				int count = 0;
				for (int i = 0; i < command.length(); i++)
					if (command[i] == ' ')
						count++;

				// Initialize arrays of file's name
				vector<string> file(count);
				int index = -1;
				for (int i = 0; i < command.length() - 1; i++) {
					if (command[i] == ' ' && command[i + 1] != ' ') {
						index++;
						continue;
					}
					if (index != -1) {
						file[index].push_back(command[i]);
					}
				}

				// Check file's name == '*' ?
				for (int i = 0; i < count; i++) {
					for (int j = 0; j < file[i].length(); j++) {
						if (file[i][j] == '*') {
							// Test dir
							TCHAR buff[1024];
							if (!GetCurrentDirectory(1024, buff)) {
								cout << "The local directory is not available!!!" << endl;
								continue;
							}

							// Prepare string for use with FindFile functions.  First, copy the
							// string to a buffer, then append '\*' to the directory name.
							strcat(buff, TEXT("\\*"));

							// Find the first file in the directory.
							HANDLE hFind = INVALID_HANDLE_VALUE;
							WIN32_FIND_DATA ffd;
							LARGE_INTEGER filesize;
							vector<string> subFile;

							hFind = FindFirstFile(buff, &ffd);

							if (INVALID_HANDLE_VALUE == hFind)
							{
								cout << "Cannot find files!!!" << endl;
								break;
							}

							do
							{
								if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
								{
									//_tprintf(TEXT("  %s   <DIR>\n"), ffd.cFileName);
								}
								else
								{
									filesize.LowPart = ffd.nFileSizeLow;
									filesize.HighPart = ffd.nFileSizeHigh;
									//_tprintf(TEXT("  %s   %ld bytes\n"), ffd.cFileName, filesize.QuadPart);
									subFile.push_back(ffd.cFileName);
								}
							} while (FindNextFile(hFind, &ffd) != 0);

							// Find file[i][j] in subFile
							file[i].erase(file[i].begin() + j);
							for (int k = 0; k < subFile.size(); k++) {
								size_t found = subFile[k].find(file[i]);
								if (found != string::npos) 
									file.push_back(subFile[k]);
							}

							// Delete file[i]
							file.erase(file.begin() + i);

							break;
						}
					}
				}

				// Remove duplicate files
				for (int i = 0; i < file.size() - 1; i++) {
					for (int j = i + 1; j < file.size(); j++) {
						if (file[i] == file[j]) {
							file.erase(file.begin() + j);
							j--;
						}
					}
				}

				// Send all files
				for (int i = 0; i < file.size(); i++) {
					cout << "Send " << file[i] << "?";
					char c;
					cin >> c;
					if (c == 'y' || c == 'Y') {
						uploadFile(file[i], socket_descriptor, IP);
					}
				}
			}
			else {
				while (1) {
					// Get file's name
					string file;
					cout << "Enter the file's name (directory) (Press '0' to exit): ";
					getline(cin, file);
					cin.clear();
					if (file[0] == '0') {
						break;
					}
					uploadFile(file, socket_descriptor, IP);
				}
			}
		}

		// Download file
		if (cmd[0] == 'g' && cmd[1] == 'e' && cmd[2] == 't') {
			if (command.length() <= 5) {
				// Get file's name
				string file;
				cout << "Enter the file's name (directory): ";
				getline(cin, file);
				cin.clear();
				downloadFile(file, socket_descriptor, IP);
			}
			else {
				string file;
				for (int i = 0; i < command.length(); i++) {
					if (command[i] == ' ' && command[i + 1] != ' ') {
						for (int j = i + 1; j < command.length(); j++)
							file.push_back(command[j]);
						break;
					}
				}
				downloadFile(file, socket_descriptor, IP);
			}
		}

		// Download many files
		if (cmd[0] == 'm' && cmd[1] == 'g' && cmd[2] == 'e' && cmd[3] == 't') {
			if (command.length() > 5) {
				// Count number of files
				int count = 0;
				for (int i = 0; i < command.length() - 1; i++)
					if (command[i] == ' ' && command[i + 1] != ' ')
						count++;

				// Initialize arrays of file's name
				vector<string> file(count);
				int index = -1;
				for (int i = 0; i < command.length(); i++) {
					if (command[i] == ' ') {
						index++;
						continue;
					}
					if (index != -1) {
						file[index].push_back(command[i]);
					}
				}

				// Check file's name == '*' ?
				for (int i = 0; i < count; i++) {
					for (int j = 0; j < file[i].length(); j++) {
						if (file[i][j] == '*') {
							string subFile = nlst(socket_descriptor, IP, file[i]);
							string temp;
							for (int k = 0; k < subFile.length(); k++) {
								if (subFile[k] == '\r' && subFile[k + 1] == '\n') {
									file.push_back(temp);
									temp.clear();
									k++;
									continue;
								}
								temp.push_back(subFile[k]);
							}
							file.erase(file.begin() + i);
							i--;
							break;
						}
					}
				}

				// Remove duplicate files
				for (int i = 0; i < file.size() - 1; i++) {
					for (int j = i + 1; j < file.size(); j++) {
						if (file[i] == file[j]) {
							file.erase(file.begin() + j);
							j--;
						}
					}
				}

				// Receive all files
				for (int i = 0; i < file.size(); i++) {
					cout << "Receive " << file[i] << "?";
					char c;
					cin >> c;
					if (c == 'y' || c == 'Y') {
						downloadFile(file[i], socket_descriptor, IP);
					}
				}
			}
			else {
				while (1) {
					// Get file's name
					string file;
					cout << "Enter the file's name (directory) (Press '0' to exit): ";
					getline(cin, file);
					cin.clear();
					if (file[0] == '0') {
						break;
					}
					downloadFile(file, socket_descriptor, IP);
				}
			}
		}

		// Change directory on server
		if (cmd[0] == 'c' && cmd[1] == 'd') {
			// Get directory
			string dir;
			for (int i = 0; i < command.length() - 1; i++)
				if (command[i] == ' ' && command[i + 1] != ' ') {
					for (int j = i + 1; j < command.length(); j++) {
						dir.push_back(command[j]);
					}
					break;
				}

			// Send CWD command
			dir = "CWD " + dir + "\r\n";
			send(socket_descriptor, dir.c_str(), dir.size(), NULL);

			// Receive message
			if (recv(socket_descriptor, Buffer, 4095, NULL) > 0) {
				sscanf(Buffer, "%d", &code);
				if (code != 250) {										// Code for CWD sucess
					replylogcode(code);
				}
				else {
					SetColor(2);
					printf("%s", Buffer);
					SetColor(15);
					memset(Buffer, '\0', sizeof(Buffer));				// Replace all element to '\0'
				}
			}

		}

		// Change directory on client
		if (cmd[0] == 'l' && cmd[1] == 'c' && cmd[2] == 'd') {
			// Get directory
			string dir;
			for (int i = 0; i < command.length() - 1; i++)
				if (command[i] == ' ' && command[i + 1] != ' ') {
					for (int j = i + 1; j < command.length(); j++) {
						dir.push_back(command[j]);
					}
					break;
				}

			// Test dir
			TCHAR buff[1024];
			if (!GetCurrentDirectory(1024, buff)) {
				cout << "The local directory is not available!!!" << endl;
				continue;
			}

			// Case dir is empty
			if (dir.length() == 0) {
				cout << "Local directory now ";
				puts(buff);
				continue;
			}

			// Set directory
			if (!SetCurrentDirectory(dir.c_str())) {
				cout << "Cannot set directory!!!" << endl;
			}
			else {
				cout << "Local directory now " << dir << endl;
			}
		}

		// Delete file on server
		if (cmd[0] == 'd' && cmd[1] == 'e' && cmd[2] == 'l' && cmd[3] == 'e' && cmd[4] == 't' && cmd[5] == 'e') {
			if (command.length() <= 7) {
				// Get file's name
				string file;
				cout << "Enter the file's name: ";
				getline(cin, file);
				cin.clear();

				deleteFile(file, socket_descriptor);
			}
			else {
				// Get file's name
				string file;
				for (int i = 5; i < command.length(); i++) {
					if (command[i] == ' ' && command[i + 1] != ' ') {
						for (int j = i + 1; j < command.length(); j++)
							file.push_back(command[j]);
						break;
					}
				}

				// Test file
				if (file == "") {
					cout << "Syntax error!!!" << endl;
					continue;
				}
				else {
					deleteFile(file, socket_descriptor);
				}
			}
		}

		// Delete many files on server
		if (cmd[0] == 'm' && cmd[1] == 'd' && cmd[2] == 'e' && cmd[3] == 'l' && cmd[4] == 'e' && cmd[5] == 't' && cmd[6] == 'e') {
			if (command.length() <= 8) {
				while (1) {
					// Get file's name
					string file;
					cout << "Enter the file's name (Press '0' to exit): ";
					getline(cin, file);
					cin.clear();
					if (file[0] == '0') {
						break;
					}
					deleteFile(file, socket_descriptor);
				}
			}
			else {
				// Count number of files
				int count = 0;
				for (int i = 0; i < command.length(); i++)
					if (command[i] == ' ')
						count++;

				// Initialize arrays of file's name
				vector<string> file(count);
				int index = -1;
				for (int i = 0; i < command.length(); i++) {
					if (command[i] == ' ') {
						index++;
						continue;
					}
					if (index != -1) {
						file[index].push_back(command[i]);
					}
				}

				// Check file's name == '*' ?
				for (int i = 0; i < count; i++) {
					for (int j = 0; j < file[i].length(); j++) {
						if (file[i][j] == '*') {
							string subFile = nlst(socket_descriptor, IP, file[i]);
							string temp;
							for (int k = 0; k < subFile.length(); k++) {
								if (subFile[k] == '\r' && subFile[k + 1] == '\n') {
									file.push_back(temp);
									temp.clear();
									k++;
									continue;
								}
								temp.push_back(subFile[k]);
							}
							file.erase(file.begin() + i);
							i--;
							break;
						}
					}
				}

				// Remove duplicate files
				for (int i = 0; i < file.size() - 1; i++) {
					for (int j = i + 1; j < file.size(); j++) {
						if (file[i] == file[j]) {
							file.erase(file.begin() + j);
							j--;
						}
					}
				}

				// Delete all files
				for (int i = 0; i < file.size(); i++) {
					cout << "Delete " << file[i] << "?";
					char c;
					cin >> c;
					if (c == 'y' || c == 'Y') {
						deleteFile(file[i], socket_descriptor);
					}
				}
			}
		}

		// Create folder on server
		if (cmd[0] == 'm' && cmd[1] == 'k' && cmd[2] == 'd' && cmd[3] == 'i' && cmd[4] == 'r') {
			if (command.length() <= 6) {
				// Get file's name
				string file;
				cout << "Enter the file's name: ";
				getline(cin, file);
				cin.clear();

				// Send message
				file = "XMKD " + file + "\r\n";
				send(socket_descriptor, file.c_str(), file.size(), NULL);

				// Receive message
				if (recv(socket_descriptor, Buffer, 4095, NULL) > 0) {
					sscanf(Buffer, "%d", &code);
					if (code != 257) {										// Code for create sucess
						replylogcode(code);
					}
					else {
						SetColor(2);
						printf("%s", Buffer);
						SetColor(15);
						memset(Buffer, '\0', sizeof(Buffer));				// Replace all element to '\0'
					}
				}
			}
			else {
				// Get file's name
				string file;
				for (int i = 5; i < command.length(); i++) {
					if (command[i] == ' ' && command[i + 1] != ' ') {
						for (int j = i + 1; j < command.length(); j++)
							file.push_back(command[j]);
						break;
					}
				}

				// Test file
				if (file == "") {
					cout << "Syntax error!!!" << endl;
					continue;
				}
				else {
					// Send message
					file = "XMKD " + file + "\r\n";
					send(socket_descriptor, file.c_str(), file.size(), NULL);

					// Receive message
					if (recv(socket_descriptor, Buffer, 4095, NULL) > 0) {
						sscanf(Buffer, "%d", &code);
						if (code != 257) {										// Code for welcome message
							replylogcode(code);
						}
						else {
							SetColor(2);
							printf("%s", Buffer);
							SetColor(15);
							memset(Buffer, '\0', sizeof(Buffer));				// Replace all element to '\0'
						}
					}
				}
			}
		}

		// Delete folder on server
		if (cmd[0] == 'r' && cmd[1] == 'm' && cmd[2] == 'd' && cmd[3] == 'i' && cmd[4] == 'r') {
			if (command.length() <= 6) {
				// Get file's name
				string file;
				cout << "Enter the file's name: ";
				getline(cin, file);
				cin.clear();

				// Send message
				file = "XRMD " + file + "\r\n";
				send(socket_descriptor, file.c_str(), file.size(), NULL);

				// Receive message
				if (recv(socket_descriptor, Buffer, 4095, NULL) > 0) {
					sscanf(Buffer, "%d", &code);
					if (code != 250) {										// Code for delete sucess
						replylogcode(code);
					}
					else {
						SetColor(2);
						printf("%s", Buffer);
						SetColor(15);
						memset(Buffer, '\0', sizeof(Buffer));				// Replace all element to '\0'
					}
				}
			}
			else {
				// Get file's name
				string file;
				for (int i = 5; i < command.length(); i++) {
					if (command[i] == ' ' && command[i + 1] != ' ') {
						for (int j = i + 1; j < command.length(); j++)
							file.push_back(command[j]);
						break;
					}
				}

				// Test file
				if (file == "") {
					cout << "Syntax error!!!" << endl;
					continue;
				}
				else {
					// Send message
					file = "XRMD " + file + "\r\n";
					send(socket_descriptor, file.c_str(), file.size(), NULL);

					// Receive message
					if (recv(socket_descriptor, Buffer, 4095, NULL) > 0) {
						sscanf(Buffer, "%d", &code);
						if (code != 250) {										// Code for delete sucess
							replylogcode(code);
						}
						else {
							SetColor(2);
							printf("%s", Buffer);
							SetColor(15);
							memset(Buffer, '\0', sizeof(Buffer));				// Replace all element to '\0'
						}
					}
				}
			}
		}

		// Current directory
		if (cmd[0] == 'p' && cmd[1] == 'w' && cmd[2] == 'd') {
			send(socket_descriptor, "XPWD\r\n", 6, NULL);
			// Receive message
			if (recv(socket_descriptor, Buffer, 4095, NULL) > 0) {
				sscanf(Buffer, "%d", &code);
				if (code != 257) {										// Code for current directory
					replylogcode(code);
				}
				else {
					SetColor(2);
					printf("%s", Buffer);
					SetColor(15);
					memset(Buffer, '\0', sizeof(Buffer));				// Replace all element to '\0'
				}
			}
		}

		// Change user
		if (cmd[0] == 'u' && cmd[1] == 's' && cmd[2] == 'e' && cmd[3] == 'r') {
			logInServer(socket_descriptor);
		}

		// Change mode
		if (cmd[0] == 'p' && cmd[1] == 'a' && cmd[2] == 's' && cmd[3] == 's' && cmd[4] == 'i' && cmd[5] == 'v' && cmd[6] == 'e') {
			cout << "PASSIVE MODE activated!!!" << endl;
			mode = 0;
		}
		if (cmd[0] == 'a' && cmd[1] == 'c' && cmd[2] == 't' && cmd[3] == 'i' && cmd[4] == 'v' && cmd[5] == 'e') {
			cout << "ACTIVE MODE activated!!!" << endl;
			mode = 1;
		}

		// Command list
		if (cmd[0] == 'h' && cmd[1] == 'e' && cmd[2] == 'l' && cmd[3] == 'p') {
			SetColor(3);
			cout << "--> active" << endl;
			cout << "--> cd" << endl;
			cout << "--> delete" << endl;
			cout << "--> dir" << endl;
			cout << "--> exit" << endl;
			cout << "--> get" << endl;
			cout << "--> help" << endl;
			cout << "--> lcd" << endl;
			cout << "--> ls" << endl;
			cout << "--> mdelete" << endl;
			cout << "--> mget" << endl;
			cout << "--> mkdir" << endl;
			cout << "--> mput" << endl;
			cout << "--> passive" << endl;
			cout << "--> put" << endl;
			cout << "--> pwd" << endl;
			cout << "--> quit" << endl;
			cout << "--> rmdir" << endl;
			cout << "--> user" << endl;
			SetColor(15);
		}

		// Quit || Exit
		if ((cmd[0] == 'q' && cmd[1] == 'u' && cmd[2] == 'i' && cmd[3] == 't') ||
			(cmd[0] == 'e' && cmd[1] == 'x' && cmd[2] == 'i' && cmd[3] == 't')) {
			// Send quit message
			send(socket_descriptor, "QUIT\r\n", 6, NULL);

			// Receive message
			if (recv(socket_descriptor, Buffer, 4095, NULL) > 0) {
				sscanf(Buffer, "%d", &code);
				if (code != 221) {										// Code for good bye
					replylogcode(code);
					pause();
					exit(0);
				}
				else {
					SetColor(2);
					printf("%s", Buffer);
					SetColor(15);
					memset(Buffer, '\0', sizeof(Buffer));				// Replace all element to '\0'
				}
			}
			break;
		}
	}

	// Close Socket to end the progress
	retcode = closesocket(socket_descriptor);
	if (retcode == SOCKET_ERROR)
		errexit("Close socket failed: %d\n", WSAGetLastError());

	// WSACleanup() is used to terminate use of socket services.
	retcode = WSACleanup();
	if (retcode == SOCKET_ERROR)
		errexit("Cleanup failed: %d\n", WSAGetLastError());

	printFooter();
	pause();
	return 0;
}

void SetColor(int ForgC)
{
	WORD wColor;
	//This handle is needed to get the current background attribute

	HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	//csbi is used for wAttributes word

	if (GetConsoleScreenBufferInfo(hStdOut, &csbi))
	{
		//To mask out all but the background attribute, and to add the color
		wColor = (csbi.wAttributes & 0xF0) + (ForgC & 0x0F);
		SetConsoleTextAttribute(hStdOut, wColor);
	}
	return;
}

SOCKET sendPort(SOCKET socket_descriptor, string IP)
{
	// Initialize variables
	int retcode;
	char Buffer[4096];
	int code;
	int port = 0;
	memset(Buffer, '\0', sizeof(Buffer));						// Replace all element to '\0'

	SOCKET socket_descriptor1 = socket(AF_INET, SOCK_STREAM, NULL);
	sockaddr_in temp;
	temp.sin_family = AF_INET;						// IPv4 Socket
	temp.sin_addr.s_addr = inet_addr(IP.c_str());	// Your ID address
	temp.sin_port = htons(port);

	// The bind function associates a local address with a socket.
	bind(socket_descriptor1, (SOCKADDR*)&temp, sizeof(temp));

	//The listten function places a socket in a state in which it is listening for an incoming connection.
	listen(socket_descriptor1, 3);					// 3 is the maximum length of the queue of pending connections

													// If PORT have not been specified, let the system decide
	if (port == 0) {
		// Create Port
		struct sockaddr_in temp1;
		int length = sizeof(temp1);
		getsockname(socket_descriptor1, (struct sockaddr *)&temp1, &length);
		port = ntohs(temp1.sin_port);
	}

	// Send PORT message
	sprintf(Buffer, "PORT %d,%d,%d,%d,%d,%d\r\n\0", temp.sin_addr.s_net, temp.sin_addr.s_host,
		temp.sin_addr.s_lh, temp.sin_addr.s_impno, (port & 65280) >> 8, port & 255);
	send(socket_descriptor, Buffer, sizeof(Buffer), NULL);

	// Receive respond
	// Receive message
	if (recv(socket_descriptor, Buffer, 4095, NULL) > 0) {
		sscanf(Buffer, "%d", &code);
		if (code != 200) {										// Code for set port sucess
			replylogcode(code);
		}
		else {
			return socket_descriptor1;
		}
	}
}

void logInServer(SOCKET socket_descriptor) {
	//----- LOGIN -----

	// Initialize variables
	char Buffer[4096];
	int code;
	memset(Buffer, '\0', sizeof(Buffer));						// Replace all element to '\0'

																// Get user's name
	string user;
	cout << "Enter user's name: ";
	//cin.ignore();
	getline(cin, user);
	user = "USER " + user + "\r\n";

	// Send user's name and receive respond from server
	send(socket_descriptor, user.c_str(), user.size(), NULL);
	if (recv(socket_descriptor, Buffer, 4095, NULL) > 0) {
		sscanf(Buffer, "%d", &code);
		if (code != 331) {										// Code for received user's name --> Enter password
			replylogcode(code);
		}
		else {
			SetColor(2);
			//puts(Buffer);
			printf("%s", Buffer);
			SetColor(15);
			memset(Buffer, '\0', sizeof(Buffer));				// Replace all element to '\0'
		}
	}

	// Get password
	string pass;
	char p;
	cout << "Enter password: ";
	//cin.ignore();
	//cin.clear();
	//getline(cin, pass);
	p = _getch();
	while (p != 13) {
		if (p != '\0')
			pass.push_back(p);
		//cout << "*";
		//cin.clear();
		p = _getch();
	}
	pass = "PASS " + pass + "\r\n";
	cout << endl;

	// Send password and receive respond from server
	send(socket_descriptor, pass.c_str(), pass.size(), NULL);
	if (recv(socket_descriptor, Buffer, 4095, NULL) > 0) {
		sscanf(Buffer, "%d", &code);
		if (code != 230) {										// Code for received user's name --> Enter password
			replylogcode(code);
		}
		else {
			SetColor(2);
			//puts(Buffer);
			printf("%s", Buffer);
			SetColor(15);
			memset(Buffer, '\0', sizeof(Buffer));				// Replace all element to '\0'
		}
	}
}

void downloadFile(string file, SOCKET socket_descriptor, string IP) {
	// Initialize variables
	int retcode;
	char Buffer[4096];
	int code;
	memset(Buffer, '\0', sizeof(Buffer));						// Replace all element to '\0'
	int i1, i2, i3, i4, p1, p2;

	// Send ACTIVE mode
	if (mode == 1) {
		SOCKET socket_descriptor1 = sendPort(socket_descriptor, IP);
		SOCKET socket_descriptor2;
		sockaddr_in data;
		int dataSize = sizeof(data);

		// Format file's name
		for (int i = file.length() - 1; i >= 0; i--) {
			if (file[i] == '\\') {
				file.erase(file.begin(), file.begin() + i + 1);
				break;
			}
		}

		// Open file
		FILE* f = fopen(file.c_str(), "wt");

		// Send "STOR filename" command
		file = "RETR " + file + "\r\n";
		send(socket_descriptor, file.c_str(), file.size(), NULL);

		// Receive message
		if (recv(socket_descriptor, Buffer, 4095, NULL) > 0) {
			sscanf(Buffer, "%d", &code);
			if (code != 150) {										// Code for open data chanel
				replylogcode(code);
			}
			else {
				// Print message
				SetColor(2);
				printf("%s", Buffer);
				SetColor(15);
				memset(Buffer, '\0', sizeof(Buffer));				// Replace all element to '\0'

				// Accept the socket
				socket_descriptor2 = accept(socket_descriptor1, (sockaddr*)&data, &dataSize);
				closesocket(socket_descriptor1);

				// Receive data via data chanel
				while (1) {
					char buff[4096];
					memset(buff, 0, 4096);
					int size = recv(socket_descriptor2, buff, 4096, NULL);
					if (size < 4096) {
						if (size > 0)
							buff[size] = '\0';
						fwrite(buff, size, 1, f);
						break;
					}
					else {
						cin.clear();
						fwrite(buff, 4096, 1, f);
					}
				}

				// Close file
				fclose(f);

				// Close Socket to end the progress
				retcode = closesocket(socket_descriptor2);
				if (retcode == SOCKET_ERROR)
					errexit("Close socket failed: %d\n", WSAGetLastError());

				// Receive message
				if (recv(socket_descriptor, Buffer, 4095, NULL) > 0) {
					sscanf(Buffer, "%d", &code);
					if (code != 226) {										// Code for sucessfully transfered
						replylogcode(code);
					}
					else {
						SetColor(2);
						printf("%s", Buffer);
						SetColor(15);
						memset(Buffer, '\0', sizeof(Buffer));				// Replace all element to '\0'
					}
				}
			}
		}
	}
	else {
		// Send PASSIVE MODE
		send(socket_descriptor, "PASV\r\n", 6, NULL);

		// Receive message
		if (recv(socket_descriptor, Buffer, 4095, NULL) > 0) {
			sscanf(Buffer, "%d", &code);
			if (code != 227) {										// Code for passive mode
				replylogcode(code);
				return;
			}
			else {
				// Get data port and IP address
				sscanf_s(Buffer, "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d).\r\n", &i1, &i2, &i3, &i4, &p1, &p2);
				memset(Buffer, '\0', sizeof(Buffer));				// Replace all element to '\0'
				int dataPort = (p1 * 256) + p2;

				//Opening new data connection to appempt to STOR file in server root dir.
				// Initialize SOCKADDR_IN
				SOCKADDR_IN data;
				int dataSize = sizeof(data);						// Required for acepted call
				data.sin_addr.s_addr = inet_addr(IP.c_str());		// IP address
				data.sin_port = htons(dataPort);					// Port
				data.sin_family = AF_INET;							// IPv4 Socket

																	// Initialize socket to connect
				SOCKET socket_descriptor1;
				socket_descriptor1 = socket(AF_INET, SOCK_STREAM, NULL);
				// Test socket
				if (socket_descriptor1 == INVALID_SOCKET) {
					cout << "Failed creating socket!!!" << endl;
					return;
				}

				// Connect to ftp server
				if (connect(socket_descriptor1, (SOCKADDR*)&data, dataSize) != 0) {			// Cannot connect
																							// MessageBoxA(NULL, "Failed to Connect", "Error", MB_OK | MB_ICONERROR);
					cout << "Failed to connect data!!!" << endl;
					return;
				}
				else
					cout << "Sucess to connect data!!!" << endl;

				// Format file's name
				for (int i = file.length() - 1; i >= 0; i--) {
					if (file[i] == '\\') {
						file.erase(file.begin(), file.begin() + i + 1);
						break;
					}
				}

				// Open file
				FILE* f = fopen(file.c_str(), "wt");
				//FILE* f1 = fopen(file.c_str(), "rt");

				// Send "STOR filename" command
				//string file1 = file;
				file = "RETR " + file + "\r\n";
				send(socket_descriptor, file.c_str(), file.size(), NULL);

				// Receive message
				if (recv(socket_descriptor, Buffer, 4095, NULL) > 0) {
					sscanf(Buffer, "%d", &code);
					if (code != 150) {										// Code for open data chanel
						replylogcode(code);
					}
					else {
						// Print message
						SetColor(2);
						printf("%s", Buffer);
						SetColor(15);
						memset(Buffer, '\0', sizeof(Buffer));				// Replace all element to '\0'

																			// Receive data via data chanel
						while (1) {
							char buff[4096];
							memset(buff, 0, 4096);
							int size = recv(socket_descriptor1, buff, 4096, NULL);
							if (size < 4096) {
								buff[size] = '\0';
								fwrite(buff, size, 1, f);
								break;
							}
							else {
								cin.clear();
								fwrite(buff, 4096, 1, f);
							}
						}

						// Close file
						fclose(f);

						// Close Socket to end the progress
						retcode = closesocket(socket_descriptor1);
						if (retcode == SOCKET_ERROR)
							errexit("Close socket failed: %d\n", WSAGetLastError());

						// Receive message
						if (recv(socket_descriptor, Buffer, 4095, NULL) > 0) {
							sscanf(Buffer, "%d", &code);
							if (code != 226) {										// Code for sucessfully transfered
								replylogcode(code);
							}
							else {
								SetColor(2);
								printf("%s", Buffer);
								SetColor(15);
								memset(Buffer, '\0', sizeof(Buffer));				// Replace all element to '\0'
							}
						}
					}
				}
			}
		}
	}
}

void uploadFile(string file, SOCKET socket_descriptor, string IP) {
	// Initialize variables
	int retcode;
	char Buffer[4096];
	int code;
	memset(Buffer, '\0', sizeof(Buffer));						// Replace all element to '\0'

																// Open file
	FILE* f = fopen(file.c_str(), "rb");
	if (f == NULL) {
		cout << "File not exsisted!!!" << endl;
		return;
	}

	// Get file's size
	long fileSize = GetFileSize(file);

	// Send ACTIVE mode
	if (mode == 1) {
		SOCKET socket_descriptor1 = sendPort(socket_descriptor, IP);
		SOCKET socket_descriptor2;
		sockaddr_in data;
		int dataSize = sizeof(data);

		// Format file's name
		for (int i = file.length() - 1; i >= 0; i--) {
			if (file[i] == '\\') {
				file.erase(file.begin(), file.begin() + i + 1);
				break;
			}
		}

		// Send "STOR filename" command
		file = "STOR " + file + "\r\n";
		send(socket_descriptor, file.c_str(), file.size(), NULL);

		// Receive message
		if (recv(socket_descriptor, Buffer, 4095, NULL) > 0) {
			sscanf(Buffer, "%d", &code);
			if (code != 150) {										// Code for open data chanel
				replylogcode(code);
				return;
			}
			else {
				// Print message
				SetColor(2);
				printf("%s", Buffer);
				SetColor(15);
				memset(Buffer, '\0', sizeof(Buffer));				// Replace all element to '\0'

				// Accept the socket
				socket_descriptor2 = accept(socket_descriptor1, (sockaddr*)&data, &dataSize);
				closesocket(socket_descriptor1);

				// Send data via data chanel
				while (fileSize > 0) {
					char buff[4100];
					if (fileSize >= 4096) {
						fread(buff, 4096, 1, f);
						send(socket_descriptor2, buff, 4096, NULL);
					}
					else {
						fread(buff, fileSize, 1, f);
						buff[fileSize] = '\0';
						send(socket_descriptor2, buff, fileSize, NULL);
					}

					fileSize -= 4096;
				}

				// Close file
				fclose(f);

				// Close Socket to end the progress
				retcode = closesocket(socket_descriptor2);
				if (retcode == SOCKET_ERROR)
					errexit("Close socket failed: %d\n", WSAGetLastError());

				// Receive message
				if (recv(socket_descriptor, Buffer, 4095, NULL) > 0) {
					sscanf(Buffer, "%d", &code);
					if (code != 226) {										// Code for sucessfully transfered
						replylogcode(code);
						return;
					}
					else {
						SetColor(2);
						printf("%s", Buffer);
						SetColor(15);
						memset(Buffer, '\0', sizeof(Buffer));				// Replace all element to '\0'
					}
				}
			}
		}
	}
	else {
		// Send PASSIVE MODE
		send(socket_descriptor, "PASV\r\n", 6, NULL);

		// Receive message
		if (recv(socket_descriptor, Buffer, 4095, NULL) > 0) {
			sscanf(Buffer, "%d", &code);
			if (code != 227) {										// Code for passive mode
				replylogcode(code);
				return;
			}
			else {
				// Get data port
				int i1, i2, i3, i4, p1, p2;
				sscanf_s(Buffer, "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d).\r\n", &i1, &i2, &i3, &i4, &p1, &p2);
				memset(Buffer, '\0', sizeof(Buffer));				// Replace all element to '\0'

				int dataPort = (p1 * 256) + p2;

				//Opening new data connection to appempt to STOR file in server root dir.
				// Initialize SOCKADDR_IN
				SOCKADDR_IN data;
				int dataSize = sizeof(data);						// Required for acepted call
				data.sin_addr.s_addr = inet_addr(IP.c_str());		// IP address
				data.sin_port = htons(dataPort);					// Port
				data.sin_family = AF_INET;							// IPv4 Socket

																	// Initialize socket to connect
				SOCKET socket_descriptor1;
				socket_descriptor1 = socket(AF_INET, SOCK_STREAM, NULL);
				// Test socket
				if (socket_descriptor1 == INVALID_SOCKET) {
					cout << "Failed creating socket!!!" << endl;
					return;
				}

				// Connect to ftp server
				if (connect(socket_descriptor1, (SOCKADDR*)&data, dataSize) != 0) {			// Cannot connect
																							// MessageBoxA(NULL, "Failed to Connect", "Error", MB_OK | MB_ICONERROR);
					cout << "Failed to connect data!!!" << endl;
					return;
				}
				else
					cout << "Sucess to connect data!!!" << endl;

				// Format file's name
				for (int i = file.length() - 1; i >= 0; i--) {
					if (file[i] == '\\') {
						file.erase(file.begin(), file.begin() + i + 1);
						break;
					}
				}

				// Send "STOR filename" command
				file = "STOR " + file + "\r\n";
				send(socket_descriptor, file.c_str(), file.size(), NULL);

				// Receive message
				if (recv(socket_descriptor, Buffer, 4095, NULL) > 0) {
					sscanf(Buffer, "%d", &code);
					if (code != 150) {										// Code for open data chanel
						replylogcode(code);
						return;
					}
					else {
						// Print message
						SetColor(2);
						puts(Buffer);
						SetColor(15);
						memset(Buffer, '\0', sizeof(Buffer));				// Replace all element to '\0'

																			// Send data via data chanel
						while (fileSize > 0) {
							char buff[4100];
							if (fileSize >= 4096) {
								fread(buff, 4096, 1, f);
								send(socket_descriptor1, buff, 4096, NULL);
								//recv(socket_descriptor1, Buffer, 4095, NULL);
							}
							else {
								fread(buff, fileSize, 1, f);
								buff[fileSize] = '\0';
								send(socket_descriptor1, buff, fileSize, NULL);
								//recv(socket_descriptor1, Buffer, 4095, NULL);
							}

							fileSize -= 4096;
						}

						// Close file
						fclose(f);

						// Close Socket to end the progress
						retcode = closesocket(socket_descriptor1);
						if (retcode == SOCKET_ERROR)
							errexit("Close socket failed: %d\n", WSAGetLastError());

						// Receive message
						if (recv(socket_descriptor, Buffer, 4095, NULL) > 0) {
							sscanf(Buffer, "%d", &code);
							if (code != 226) {										// Code for sucessfully transfered
								replylogcode(code);
								return;
							}
							else {
								SetColor(2);
								//puts(Buffer);
								printf("%s", Buffer);
								SetColor(15);
								memset(Buffer, '\0', sizeof(Buffer));				// Replace all element to '\0'
							}
						}


					}
				}
			}
		}
	}
}

void deleteFile(string file, SOCKET socket_descriptor) {
	// Initialize variables
	int retcode;
	char Buffer[4096];
	int code;
	memset(Buffer, '\0', sizeof(Buffer));						// Replace all element to '\0'

																// Send message
	file = "DELE " + file + "\r\n";
	send(socket_descriptor, file.c_str(), file.size(), NULL);

	// Receive respond
	if (recv(socket_descriptor, Buffer, 4095, NULL) > 0) {
		sscanf(Buffer, "%d", &code);
		if (code != 250) {										// Code for delete sucess
			replylogcode(code);
			return;
		}
		else {
			SetColor(2);
			printf("%s", Buffer);
			SetColor(15);
			memset(Buffer, '\0', sizeof(Buffer));				// Replace all element to '\0'
		}
	}
}

string nlst(SOCKET socket_descriptor, string IP, string file) {
	// Initialize variables
	int retcode;
	char Buffer[4096];
	int code;
	memset(Buffer, '\0', sizeof(Buffer));						// Replace all element to '\0'
	string res;

	// Send ACTIVE mode
	if (mode == 1) {
		SOCKET socket_descriptor1 = sendPort(socket_descriptor, IP);
		SOCKET socket_descriptor2;
		sockaddr_in data;
		int dataSize = sizeof(data);

		// Send command
		string mess = "NLST";
		if (file == "") {
			mess += "\r\n";
		}
		else
			mess = mess + ' ' + file + "\r\n";
		send(socket_descriptor, mess.c_str(), mess.size(), NULL);

		// Receive message
		if (recv(socket_descriptor, Buffer, 4095, NULL) > 0) {
			sscanf(Buffer, "%d", &code);
			if (code != 150) {										// Code for open data chanel
				replylogcode(code);
				return res;
			}
			else {
				// Accept the socket
				socket_descriptor2 = accept(socket_descriptor1, (sockaddr*)&data, &dataSize);
				closesocket(socket_descriptor1);

				// Print message
				SetColor(2);
				printf("%s", Buffer);
				SetColor(15);
				memset(Buffer, '\0', sizeof(Buffer));				// Replace all element to '\0'

				// Receive data
				while (1) {
					if (recv(socket_descriptor2, Buffer, 4095, NULL) > 0) {
						// Save to res
						res += Buffer;
						memset(Buffer, '\0', sizeof(Buffer));				// Replace all element to '\0'
					}
					else
						break;
				}

				// Receive message
				if (recv(socket_descriptor, Buffer, 4095, NULL) > 0) {
					sscanf(Buffer, "%d", &code);
					if (code != 226) {										// Code for sucessfully transfered
						replylogcode(code);
						return res;
					}
					else {
						SetColor(2);
						//puts(Buffer);
						printf("%s", Buffer);
						SetColor(15);
						memset(Buffer, '\0', sizeof(Buffer));				// Replace all element to '\0'
					}
				}

				// Close Socket
				retcode = closesocket(socket_descriptor2);
				if (retcode == SOCKET_ERROR)
					errexit("Close socket failed: %d\n", WSAGetLastError());

				return res;
			}
		}
	}
	else {
		// Send PASSIVE MODE
		send(socket_descriptor, "PASV\r\n", 6, NULL);

		// Receive message
		if (recv(socket_descriptor, Buffer, 4095, NULL) > 0) {
			sscanf(Buffer, "%d", &code);
			if (code != 227) {										// Code for welcome message
				replylogcode(code);
				return res;
			}
			else {
				// Get data port
				int i1, i2, i3, i4, p1, p2;
				sscanf_s(Buffer, "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d).\r\n", &i1, &i2, &i3, &i4, &p1, &p2);
				memset(Buffer, '\0', sizeof(Buffer));				// Replace all element to '\0'

				int dataPort = (p1 * 256) + p2;

				//Opening new data connection to appempt to STOR file in server root dir.
				// Initialize SOCKADDR_IN
				SOCKADDR_IN data;
				int dataSize = sizeof(data);						// Required for acepted call
				data.sin_addr.s_addr = inet_addr(IP.c_str());		// IP address
				data.sin_port = htons(dataPort);					// Port
				data.sin_family = AF_INET;							// IPv4 Socket

																	// Initialize socket to connect
				SOCKET socket_descriptor1;
				socket_descriptor1 = socket(AF_INET, SOCK_STREAM, NULL);
				// Test socket
				if (socket_descriptor1 == INVALID_SOCKET) {
					cout << "Failed creating socket!!!" << endl;
					return res;
				}

				// Connect to ftp server
				if (connect(socket_descriptor1, (SOCKADDR*)&data, dataSize) != 0) {			// Cannot connect
																							// MessageBoxA(NULL, "Failed to Connect", "Error", MB_OK | MB_ICONERROR);
					cout << "Failed to connect data!!!" << endl;
					return res;
				}
				else
					cout << "Sucess to connect data!!!" << endl;

				// Send command
				string mess = "NLST";
				if (file == "") {
					mess += "\r\n";
				}
				else
					mess = mess + ' ' + file + "\r\n";
				send(socket_descriptor, mess.c_str(), mess.size(), NULL);

				// Receive message
				if (recv(socket_descriptor, Buffer, 4095, NULL) > 0) {
					sscanf(Buffer, "%d", &code);
					if (code != 150) {										// Code for open data chanel
						replylogcode(code);
						return res;
					}
					else {
						// Print message
						SetColor(2);
						printf("%s", Buffer);
						SetColor(15);
						memset(Buffer, '\0', sizeof(Buffer));				// Replace all element to '\0'

						// Receive data
						while (1) {
							if (recv(socket_descriptor1, Buffer, 4095, NULL) > 0) {
								// Save to res
								res += Buffer;

								memset(Buffer, '\0', sizeof(Buffer));				// Replace all element to '\0'
							}
							else
								break;
						}

						// Receive message
						if (recv(socket_descriptor, Buffer, 4095, NULL) > 0) {
							sscanf(Buffer, "%d", &code);
							if (code != 226) {										// Code for sucessfully transfered
								replylogcode(code);
								return res;
							}
							else {
								SetColor(2);
								//puts(Buffer);
								printf("%s", Buffer);
								SetColor(15);
								memset(Buffer, '\0', sizeof(Buffer));				// Replace all element to '\0'
							}
						}

						// Close Socket
						retcode = closesocket(socket_descriptor1);
						if (retcode == SOCKET_ERROR)
							errexit("Close socket failed: %d\n", WSAGetLastError());

						// Return res
						return res;
					}
				}
			}
		}
	}
}

void list(SOCKET socket_descriptor, string IP, string file) {
	// Initialize variables
	int retcode;
	char Buffer[4096];
	int code;
	memset(Buffer, '\0', sizeof(Buffer));						// Replace all element to '\0'

																// Send ACTIVE mode
	if (mode == 1) {
		SOCKET socket_descriptor1 = sendPort(socket_descriptor, IP);
		SOCKET socket_descriptor2;
		sockaddr_in data;
		int dataSize = sizeof(data);

		// Send command
		string mess = "LIST";
		if (file == "") {
			mess += "\r\n";
		}
		else
			mess = mess + ' ' + file + "\r\n";
		send(socket_descriptor, mess.c_str(), mess.size(), NULL);

		// Receive message
		if (recv(socket_descriptor, Buffer, 4095, NULL) > 0) {
			sscanf(Buffer, "%d", &code);
			if (code != 150) {										// Code for open data chanel
				replylogcode(code);
				return;
			}
			else {
				// Accept the socket
				socket_descriptor2 = accept(socket_descriptor1, (sockaddr*)&data, &dataSize);
				closesocket(socket_descriptor1);

				// Print message
				SetColor(2);
				printf("%s", Buffer);
				SetColor(15);
				memset(Buffer, '\0', sizeof(Buffer));				// Replace all element to '\0'

																	// Receive data
				while (1) {
					if (recv(socket_descriptor2, Buffer, 4095, NULL) > 0) {
						// Print data
						SetColor(14);
						printf("%s", Buffer);
						SetColor(15);
						memset(Buffer, '\0', sizeof(Buffer));				// Replace all element to '\0'
					}
					else
						break;
				}

				// Receive message
				if (recv(socket_descriptor, Buffer, 4095, NULL) > 0) {
					sscanf(Buffer, "%d", &code);
					if (code != 226) {										// Code for sucessfully transfered
						replylogcode(code);
						return;
					}
					else {
						SetColor(2);
						//puts(Buffer);
						printf("%s", Buffer);
						SetColor(15);
						memset(Buffer, '\0', sizeof(Buffer));				// Replace all element to '\0'
					}
				}

				// Close Socket
				retcode = closesocket(socket_descriptor2);
				if (retcode == SOCKET_ERROR)
					errexit("Close socket failed: %d\n", WSAGetLastError());
			}
		}
	}
	else {
		// Send PASSIVE MODE
		send(socket_descriptor, "PASV\r\n", 6, NULL);

		// Receive message
		if (recv(socket_descriptor, Buffer, 4095, NULL) > 0) {
			sscanf(Buffer, "%d", &code);
			if (code != 227) {										// Code for welcome message
				replylogcode(code);
				return;
			}
			else {
				// Get data port
				int i1, i2, i3, i4, p1, p2;
				sscanf_s(Buffer, "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d).\r\n", &i1, &i2, &i3, &i4, &p1, &p2);
				memset(Buffer, '\0', sizeof(Buffer));				// Replace all element to '\0'

				int dataPort = (p1 * 256) + p2;

				//Opening new data connection to appempt to STOR file in server root dir.
				// Initialize SOCKADDR_IN
				SOCKADDR_IN data;
				int dataSize = sizeof(data);						// Required for acepted call
				data.sin_addr.s_addr = inet_addr(IP.c_str());		// IP address
				data.sin_port = htons(dataPort);					// Port
				data.sin_family = AF_INET;							// IPv4 Socket

																	// Initialize socket to connect
				SOCKET socket_descriptor1;
				socket_descriptor1 = socket(AF_INET, SOCK_STREAM, NULL);
				// Test socket
				if (socket_descriptor1 == INVALID_SOCKET) {
					cout << "Failed creating socket!!!" << endl;
					return;
				}

				// Connect to ftp server
				if (connect(socket_descriptor1, (SOCKADDR*)&data, dataSize) != 0) {			// Cannot connect
																							// MessageBoxA(NULL, "Failed to Connect", "Error", MB_OK | MB_ICONERROR);
					cout << "Failed to connect data!!!" << endl;
					return;
				}
				else
					cout << "Sucess to connect data!!!" << endl;

				// Send command
				string mess = "LIST";
				if (file == "") {
					mess += "\r\n";
				}
				else
					mess = mess + ' ' + file + "\r\n";
				send(socket_descriptor, mess.c_str(), mess.size(), NULL);

				// Receive message
				if (recv(socket_descriptor, Buffer, 4095, NULL) > 0) {
					sscanf(Buffer, "%d", &code);
					if (code != 150) {										// Code for open data chanel
						replylogcode(code);
						return;
					}
					else {
						// Print message
						SetColor(2);
						printf("%s", Buffer);
						SetColor(15);
						memset(Buffer, '\0', sizeof(Buffer));				// Replace all element to '\0'

																			// Receive data
						while (1) {
							if (recv(socket_descriptor1, Buffer, 4095, NULL) > 0) {
								// Print data
								SetColor(14);
								printf("%s", Buffer);
								SetColor(15);
								memset(Buffer, '\0', sizeof(Buffer));				// Replace all element to '\0'
							}
							else
								break;
						}

						// Receive message
						if (recv(socket_descriptor, Buffer, 4095, NULL) > 0) {
							sscanf(Buffer, "%d", &code);
							if (code != 226) {										// Code for sucessfully transfered
								replylogcode(code);
								return;
							}
							else {
								SetColor(2);
								//puts(Buffer);
								printf("%s", Buffer);
								SetColor(15);
								memset(Buffer, '\0', sizeof(Buffer));				// Replace all element to '\0'
							}
						}

						// Close Socket
						retcode = closesocket(socket_descriptor1);
						if (retcode == SOCKET_ERROR)
							errexit("Close socket failed: %d\n", WSAGetLastError());
					}
				}
			}
		}
	}
}

void printHeader() {
	SetColor(14);
	cout << "--------- WELCOME TO FTP SERVICE ---------" << endl;
	SetColor(15);
}

void printFooter() {
	SetColor(14);
	cout << "--------- THANK YOU FOR USING OUR SERVICE ---------" << endl;
	SetColor(15);
}

long GetFileSize(string filename)
{
	struct stat stat_buf;
	int rc = stat(filename.c_str(), &stat_buf);
	return rc == 0 ? stat_buf.st_size : -1;
}

void replylogcode(int code)
{
	SetColor(12);
	switch (code) {
	case 200:
		printf("Command okay");
		break;
	case 500:
		printf("Syntax error, command unrecognized.");
		printf("This may include errors such as command line too long.");
		break;
	case 501:
		printf("Syntax error in parameters or arguments.");
		break;
	case 202:
		printf("Command not implemented, superfluous at this site.");
		break;
	case 426:
		printf("Connection closed; aborted transfer.");
		break;
	case 502:
		printf("Command not implemented.");
		break;
	case 503:
		printf("Bad sequence of commands.");
		break;
	case 530:
		printf("Not logged in.");
		break;
	case 550:
		printf("File not found or already existed.");
		break;
	}
	printf("\n");
	SetColor(15);
}

void errexit(const char *format, ...)
{
	va_list	args;

	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
	WSACleanup();
	pause();
	exit(1);
}

void pause(void)
{
	/*char c;
	printf("Press Enter to continue\n");
	scanf("%c", &c);*/
	system("pause");
}