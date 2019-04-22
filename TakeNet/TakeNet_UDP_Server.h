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
		__int64 GetConnectionAlive(void){
			__int64 now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
			return now-TimeWhenStarted;
		}
	};
private:
	SOCKET Server;
	WSADATA Wsd;
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