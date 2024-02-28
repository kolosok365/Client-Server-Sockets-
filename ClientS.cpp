
#include <iostream>
//#include <stdafx.h>

#pragma comment(lib, "ws2_32.lib")
#include <winSock2.h>
#include <string>
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma warning(disable: 4996)

SOCKET Connection;

enum Packet {
	P_ChatMessage,
	P_Test
};

bool ProcessPacket(Packet packettype) {
	switch (packettype)
	{
	case P_ChatMessage:
	{
		int msg_size;
		recv(Connection, (char*)&msg_size, sizeof(int), NULL);
		char* msg = new char[msg_size + 1];
		msg[msg_size] = '\0';
		recv(Connection, msg, msg_size, NULL);
		std::cout << msg << "\n";
		delete[] msg;
		break;
	}
	case P_Test: 
	{
		std::cout << "Test packet.\n";
		break;
	}
	default:
		std::cout << "Unrecognized packet: " << packettype << "\n";
		break;
	}
	return true;
}

//функция для принятия отправленного сервером сообщения
void ClientHandler() {
	Packet packettype;
	while (true) {
		recv(Connection, (char*)&packettype, sizeof(Packet), NULL);

		if (!ProcessPacket(packettype)) {
			break;
		}
	}
	closesocket(Connection);
}



int main(int argc, char* argv[]) {
	WSAData wsadata;
	WORD DLLVersion = MAKEWORD(2, 1);
	if (WSAStartup(DLLVersion, &wsadata) != 0) {
		std::cout << "Error\n";
		exit(1);
	}

	SOCKADDR_IN addr;//структура для хранение адреса
	int sizeofaddr = sizeof(addr);
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");//структура хранит IP адрес
	addr.sin_port = htons(1111);//хранит порт
	addr.sin_family = AF_INET;//для интернет протоколов указывается константа AF_INET

	//создаем новый сокет для соединия с сервером
    Connection = socket(AF_INET, SOCK_STREAM, NULL);
	if (connect(Connection, (SOCKADDR*)&addr, sizeof(addr)) != 0) {//присоединение к серверу
		std::cout << "Error: filed connect to server.\n";
		return 1;
	}

	std::cout << "Connected!\n";

	CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ClientHandler, NULL, NULL, NULL);

	std::string msg1;

	while (true) {
		std::getline(std::cin, msg1);
		int msg_size = msg1.size();
		Packet packettype = P_ChatMessage;
		send(Connection, (char*)&packettype, sizeof(Packet), NULL);
		send(Connection, (char*)&msg_size, sizeof(int), NULL);
		send(Connection, msg1.c_str(), msg_size, NULL);
		Sleep(10000);
	}

	system("pause");
	return 0;
}