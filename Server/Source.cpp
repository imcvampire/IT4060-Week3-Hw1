#include<WinSock2.h>
#include<WS2tcpip.h>
#include<iostream>
#include<string>
#include<cstdbool>

#include"Users.h"

using namespace std;

const char SERVER_ADDRESS[] = "127.0.0.1";
const int SERVER_PORT = 5000;
const int BUFFER_SIZE = 2048;
const char MY_ERROR[] = "Error!";
const char USER_NOT_FOUND[] = "User not found!";
const char WRONG_PASSWORD[] = "You have typed password. Please retry!";
const char ASK_PASSWORD[] = "Password: ";
const char SUCCESS[] = "Success!";

boolean verify_username(char* username)
{
	for (int i = 0; i < size(USERS); i++)
	{
		if (strcmp(username, USERS[i][0])) return true;
	}
	return false;
}

boolean verify_password(char* username, char* password)
{
	for (int i = 0; i < size(USERS); i++)
	{
		if (strcmp(username, USERS[i][0]))
		{
			if (strcmp(password, USERS[i][1])) return true;
			return false;
		}
	}
	return false;
}

int main(int argc, char* argv[])
{
	if (argc < 3)
	{
		cerr << "Missing arguments!" << endl;

		return 1;
	}

	WSADATA	wsa_data;
	if (WSAStartup(MAKEWORD(2, 2), &wsa_data))
	{
		cerr << "Version is not supported!" << endl;
	}

	SOCKET listen_socket;
	listen_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons((u_short) SERVER_PORT);

	if (inet_pton(AF_INET, SERVER_ADDRESS, (void*) &(server_addr.sin_addr.s_addr)) != 1)
	{
		cerr << "Can not convert little-endian to big-endian" << endl;

		return 1;
	}

	// Start from first argument
	for (int i = 1; i <= argc && argv[i][0] == '-'; ++i)
	{
		if (argv[i][1] == 'p')
		{
			server_addr.sin_port = htons((u_short) stoi(argv[i + 1]));

			break;
		}
	}

	if (bind(listen_socket, (sockaddr *) &server_addr, sizeof(server_addr)))
	{
		cerr << "Can not bind to this address!" << endl;

		return 1;
	}

	if (listen(listen_socket, 10))
	{
		cerr << "Can not listen!" << endl;

		return 1;
	}

	cout << "Server started!" << endl;

	sockaddr_in client_addr;
	char buffer_username[BUFFER_SIZE];
	char buffer_password[BUFFER_SIZE];
	int ret;
	int client_addr_len = sizeof(client_addr);

	for (;;)
	{
		SOCKET connect_socket = accept(listen_socket, (sockaddr *) &client_addr, &client_addr_len);

		ret = recv(connect_socket, buffer_username, BUFFER_SIZE, 0);

		if (ret == SOCKET_ERROR)
		{
			cerr << "Error: " << WSAGetLastError() << endl;
		}
		else if (ret > 0)
		{
			buffer_username[ret] = 0;

			if (verify_username(buffer_username))
			{
				ret = send(listen_socket, ASK_PASSWORD, strlen(ASK_PASSWORD), 0);

				if (ret == SOCKET_ERROR)
				{
					cerr << "Can not ask client password! Error: " << WSAGetLastError() << endl;
				}
				else if (ret > 0)
				{
					ret = recv(listen_socket, buffer_password, BUFFER_SIZE, 0);

					if (ret == SOCKET_ERROR)
					{
						cerr << "Can not receive password from client! Error: " << WSAGetLastError() << endl;
					}
					else if (ret > 0)
					{
						buffer_password[0] = 0;

						if (verify_password(buffer_username, buffer_password))
						{
							send(listen_socket, SUCCESS, strlen(SUCCESS), 0);
						}
						else
						{
							send(listen_socket, WRONG_PASSWORD, strlen(WRONG_PASSWORD), 0);
						}
					}
				}
			}
			else
			{
				send(listen_socket, USER_NOT_FOUND, strlen(USER_NOT_FOUND), 0);
			}
		}

		shutdown(connect_socket, SD_SEND);
		closesocket(connect_socket);
	}

	cout << "Bye!" << endl;

	closesocket(listen_socket);

	WSACleanup();

	return 0;
}
