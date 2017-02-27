#include<WinSock2.h>
#include<WS2tcpip.h>
#include<iostream>
#include<string>
#include<cstdbool>

using namespace std;

const char SERVER_ADDRESS[] = "127.0.0.1";
const int SERVER_PORT = 5000;
const int BUFFER_SIZE = 2048;
const char MY_ERROR[] = "Error!";
const char USER_NOT_FOUND[] = "User not found!";
const char WRONG_PASSWORD[] = "You have typed password. Please retry!";
const char ASK_PASSWORD[] = "Password: ";
const char SUCCESS[] = "Success!";


int main(int argc, char* argv[])
{
	if (argc < 5)
	{
		cerr << "Missing arguments!" << endl;

		return 1;
	}

	WSADATA wsa_data;
	if (WSAStartup(MAKEWORD(2, 2), &wsa_data))
	{
		cerr << "Version is not supported!" << endl;
	}

	SOCKET client;
	client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	int timeout = 10000;
	setsockopt(client, SOL_SOCKET, SO_RCVTIMEO, (const char *) &timeout, sizeof(int));

	sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons((u_short) SERVER_PORT);
	if (inet_pton(AF_INET, SERVER_ADDRESS, (void*) &(server_addr.sin_addr.s_addr)) != 1)
	{
		cerr << "Can not convert little-endian to big-endian" << endl;

		return 1;
	}

	for (int i = 1; i < argc; ++i)
	{
		if (argv[i][0] != '-') continue;

		switch (argv[i][1])
		{
		case 'a':
			if (inet_pton(AF_INET, argv[i + 1], (void*) &(server_addr.sin_addr.s_addr)) != 1)
			{
				cerr << "Can not convert little-endian to big-endian" << endl;

				return 1;
			}

			break;

		case 'p':
			server_addr.sin_port = htons((u_short) stoi(argv[i + 1]));
			break;
		}
	}

	if (connect(client, (sockaddr *) &server_addr, sizeof(server_addr)))
	{
		cerr << "Error! Can not connect to server! Error: " << WSAGetLastError() << endl;

		return 1;
	}

	cout << "Connected!" << endl;

	char buffer[BUFFER_SIZE];
	int ret;

	char username[BUFFER_SIZE];

	cout << "Username: " << endl;
	cin >> username;

	ret = send(client, username, strlen(username), 0);
	if (ret == SOCKET_ERROR)
	{
		cerr << "Error: " << WSAGetLastError() << endl;
	}
	else if (ret > 0)
	{
		ret = recv(client, buffer, BUFFER_SIZE, 0);

		if (ret == SOCKET_ERROR)
		{
			if (WSAGetLastError() == WSAETIMEDOUT)
			{
				cerr << "Timeout!" << endl;
			}
			else
			{
				cerr << "Error: " << WSAGetLastError() << endl;
			}
		}
		else if (ret > 0)
		{
			buffer[ret] = 0;

			if (strcmp(buffer, USER_NOT_FOUND))
			{
				cout << USER_NOT_FOUND << endl;
			}
			else if (strcmp(buffer, ASK_PASSWORD))
			{
				cout << ASK_PASSWORD << endl;

				char password[BUFFER_SIZE];
				cin >> password;

				ret = send(client, password, strlen(password), 0);

				if (ret == SOCKET_ERROR)
				{
					cerr << "Error: " << WSAGetLastError() << endl;
				}
				else if (ret > 0)
				{
					ret = recv(client, buffer, BUFFER_SIZE, 0);

					if (ret == SOCKET_ERROR)
					{
						if (WSAGetLastError() == WSAETIMEDOUT)
						{
							cerr << "Timeout!" << endl;
						}
						else
						{
							cerr << "Error: " << WSAGetLastError() << endl;
						}
					}
					else if (ret > 0)
					{
						buffer[0] = 0;

						if (strcmp(buffer, WRONG_PASSWORD))
						{
							cerr << WRONG_PASSWORD << endl;
						}
						else if (strcmp(buffer, SUCCESS))
						{
							cout << SUCCESS << endl;
						}
						else
						{
							cerr << "Error!" << endl;
						}
					}
				}
			}
		}
	}

	shutdown(client, SD_SEND);
	closesocket(client);

	WSACleanup();

	return 0;
}