
#include <iostream>
//#include <stdafx.h>
#include <winSock2.h>

#pragma comment(lib, "ws2_32.lib")


#pragma warning(disable: 4996)

SOCKET Connections[100];
int Counter = 0;

enum Packet {
	P_ChatMessage,
	P_Test
};

bool ProcessPacket(int index, Packet packettype) {
	switch (packettype) {
	case P_ChatMessage:
	{
		int msg_size;
		recv(Connections[index], (char*)&msg_size, sizeof(int), NULL);
		char* msg = new char[msg_size + 1];
		msg[msg_size] = '\0';
		recv(Connections[index], msg, msg_size, NULL);//принимаем сообщение отправленное клиентом
		for (int i = 0; i < Counter; ++i) {
			if (i == index) {
				continue;
			}
			Packet msgtype = P_ChatMessage;
			send(Connections[i], (char*)&msgtype, sizeof(Packet), NULL);
			send(Connections[i], (char*)&msg_size, sizeof(int), NULL);
			send(Connections[i], msg, msg_size, NULL);//отправлем принятое сообщение всем клинтам,
			//за исключением того, кто отправил сообщение
		}
		delete[] msg;
		break;
	}
	default:
		std::cout << "Unrecognized packet: " << packettype << "\n";
		break;
	}
	return true;
}

void ClientHandler(int index) {
	Packet packettype;
	while (true) {
		recv(Connections[index], (char*)&packettype, sizeof(Packet), NULL);
		if (!ProcessPacket(index, packettype)) {
			break;
		}
	}
	closesocket(Connections[index]);
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

	SOCKET sListen = socket(AF_INET, SOCK_STREAM, NULL);//создание сокета (семейство интернет протоколов,
																		 //протокол устанавливающий сединеие, 
																		 // 3-ий параметр пока не нужен, поэтому NULL)

	bind(sListen, (SOCKADDR*)&addr, sizeof(addr));//привязываем адрес к сокету
	listen(sListen, SOMAXCONN);//прослушивание, 2-ой параметр - это максимально допустимое число запросов, ожидающих обработку

	//принимаем соединение, создаем новый сокет, чтобы удерживать соединие с клиентом
	SOCKET newConnection;
	for (int i = 0; i < 100; ++i) {
		newConnection = accept(sListen, (SOCKADDR*)&addr, &sizeofaddr);

		if (newConnection == 0) {
			std::cout << "Error #2\n";
		}
		else {
			std::cout << "Client Connected!\n";
			std::string msg = "Hello. It's my first network program!";
			int msg_size = msg.size();
			Packet msgtype = P_ChatMessage;
			send(newConnection, (char*)&msgtype, sizeof(Packet), NULL);
			send(newConnection, (char*)&msg_size, sizeof(int), NULL);
			send(newConnection, msg.c_str(), msg_size, NULL);

			Connections[i] = newConnection;
			++Counter;
			CreateThread(NULL
					   , NULL 
					   , (LPTHREAD_START_ROUTINE) ClientHandler //указатель на функцию, с которой следует начать выполнение нового потока
					   , (LPVOID)(i) //аргумент для переданной ранее функции
					   , NULL
					   , NULL);

			Packet testpacket = P_Test;
			send(newConnection, (char*)&testpacket, sizeof(Packet),NULL);
		}
	}

	system("pause");
	return 0;
}

