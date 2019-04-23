#pragma once

#ifdef WIN32
#include <WinSock.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

typedef int SOCKET;
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define WSADATA int
#define WSAStartup(wsdlin,wsdlin2) wsdlin = 0
#define WSACleanup(wsdlin)
#define closesocket(s) close(s)
#endif

#include <vector>
#include <thread>
#include <chrono>
#include "TakeNet_BitStream.h"

class TakeNet_UDP_Client{
public:
	struct NetStats{
		__int64 BytesSent;
		__int64 BytesRecv;
		__int64 TimeWhenStarted;
		__int64 GetConnectionAlive(void){
			__int64 now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
			return now-TimeWhenStarted;
		}
	};
private:
	SOCKET Client;
	WSADATA Wsd;
	sockaddr_in host;
	std::thread *pThread_1; bool ThreadController_1;
	std::thread *pThread_2; bool ThreadController_2;
	std::vector<TakeNet_BitStream> pPackets;
	NetStats pNetStats;
	bool IsConnected;
protected:
	int NetReceiveFromUpdate(void);
	int NetPingUpdate(void);
public:
	TakeNet_UDP_Client(void);
	TakeNet_UDP_Client(char *IpAddress,unsigned short Port);
	~TakeNet_UDP_Client(void);
	bool Connect(char *IpAddress,unsigned short Port);
	void Disconnect(void);
	void SendData(TakeNet_BitStream pSendBitStream);
	TakeNet_BitStream GetPacket(int PacketID);
	int GetPacketsCount(void);
	NetStats GetClientNetStats(void);
};