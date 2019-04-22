#pragma once

#include <WinSock.h>
#include <vector>
#include <thread>
#include <chrono>
#include "TakeNet_BitStream.h"

class TakeNet_UDP_Client{
private:
	SOCKET Client;
	WSADATA Wsd;
	sockaddr_in host;
	uint64_t WaitTime[1];
	std::thread *pThread_1; bool ThreadController_1;
	std::thread *pThread_2; bool ThreadController_2;
	std::vector<TakeNet_BitStream> pPackets;
	bool IsConnected;
private:
	bool WaitState(uint64_t *desc, int id, int ms);
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
};