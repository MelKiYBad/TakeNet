#pragma once

#include <WinSock.h>
#include <vector>
#include <thread>
#include <chrono>
#include "TakeNet_BitStream.h"

class TakeNet_UDP_Server{
public:
	struct NetStats{
		__int64 BytesSent;
		__int64 BytesRecv;
		__int64 TimeWhenStarted;
	};
private:
	SOCKET Server;
	WSADATA Wsd;
	uint64_t WaitTime[1];
	int MaxConnections;
	std::thread *pThread_1; bool ThreadController_1;
	std::thread *pThread_2; bool ThreadController_2;
	std::vector<sockaddr_in> pClientList;
	std::vector<__int64> pClientTimeUpdate;
	std::vector<bool> pClientInuse;
	std::vector<TakeNet_BitStream> pPackets;
	std::vector<__int64> pClientPing;
	NetStats pNetStats;
private:
	bool WaitState(uint64_t *desc, int id, int ms);
	int GetClientIDFromBitStream(TakeNet_BitStream *bsPacket);
	int GetFreeIDFromClientInuse(void);
protected:
	int NetReceiveFromUpdate(void);
	int NetTimeoutUpdate(void);
public:
	TakeNet_UDP_Server(void);
	TakeNet_UDP_Server(unsigned short Port,int MaximunConnections);
	~TakeNet_UDP_Server(void);
	bool Startup(unsigned short Port,int MaximunConnections);
	void Shutdown(void);
	void SendData(TakeNet_BitStream pSendBitStream, int ClientID);
	void BroadcastData(TakeNet_BitStream pSendBitStream);
	void DisconnectClient(int ClientID);
	int GetClientPing(int ClientID);
	int GetActiveConnections(void);
	char *GetClientIPAddress(int ClientID);
	char *GetIPAddressFromBitStream(TakeNet_BitStream *pBitStream);
	TakeNet_BitStream GetPacket(int PacketID,int &ClientID);
	int GetPacketsCount(void);
	NetStats GetServerNetStats(void);
};