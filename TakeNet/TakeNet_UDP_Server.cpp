#include "TakeNet_UDP_Server.h"

int TakeNet_UDP_Server::GetClientIDFromBitStream(TakeNet_BitStream *bsPacket){
	int ClientID = -1;
	for(int i = 0;i<(int)MaxConnections;i++){
		if(pClientInuse[i]){
			if((pClientList[i].sin_addr.s_addr == bsPacket->GetFromAddr().sin_addr.s_addr) && (pClientList[i].sin_port == bsPacket->GetFromAddr().sin_port)){
				ClientID = i; return ClientID;
			}
		}
	} return ClientID;
}

int TakeNet_UDP_Server::GetFreeIDFromClientInuse(void){
	for(int i = 0;i<MaxConnections;i++){
		if(!pClientInuse[i]){
			return i;
		}
	} return -1;
}

int TakeNet_UDP_Server::NetReceiveFromUpdate(void){
	while(ThreadController_1){
		std::vector<char> pRecvBytesData;
		TakeNet_BitStream pPacketBitStream;
		sockaddr_in clientaddr;
		int clientlen = sizeof(clientaddr);
		char RecvBytes[64];

		while(int bytes = recvfrom(Server,RecvBytes,64,0,(sockaddr*)&clientaddr,&clientlen)){
			for(int i = 0;i<bytes;i++){
				pRecvBytesData.push_back(RecvBytes[i]);
			} if(bytes < 64){
				break;
			}
		} pNetStats.BytesRecv += pRecvBytesData.size();
			
		pPacketBitStream.Setup((unsigned char*)pRecvBytesData.data(),pRecvBytesData.size(),clientaddr);
		pPackets.push_back(pPacketBitStream);
		std::this_thread::sleep_for(std::chrono::nanoseconds(1));
	}
	return 0;
}

int TakeNet_UDP_Server::NetTimeoutUpdate(void){
	while(ThreadController_2){
		for(int i = 0;i<MaxConnections;i++){
			if(pClientInuse[i]){
				__int64 now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
				if((now-pClientTimeUpdate[i]) > 5000){
					TakeNet_BitStream pPacketBitStream;
					pPacketBitStream.Write<short>(ID_PACKET_TIMEOUT);
					pPacketBitStream.ReplaceFromAddr(pClientList[i]);
					pPackets.push_back(pPacketBitStream);
					pClientTimeUpdate[i] = now;
				}
			}
		} std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
	return 0;
}

TakeNet_UDP_Server::TakeNet_UDP_Server(void){
	MaxConnections = 0;
	memset(&Server,0,sizeof(SOCKET));
	memset(&Wsd,0,sizeof(WSADATA));
	pThread_1 = NULL; ThreadController_1 = false;
	pThread_2 = NULL; ThreadController_2 = false;
	memset(&pNetStats,0,sizeof(NetStats));
}

TakeNet_UDP_Server::TakeNet_UDP_Server(unsigned short Port,int MaximunConnections){
	Startup(Port,MaximunConnections);
}

TakeNet_UDP_Server::~TakeNet_UDP_Server(void){
	Shutdown();
}

bool TakeNet_UDP_Server::Startup(unsigned short Port,int MaximunConnections){
	WSAStartup(MAKEWORD(2,2),&Wsd);
	Server = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
	if(Server == INVALID_SOCKET){
		return false;
	}

	sockaddr_in LocalAddr;
	memset(&LocalAddr,0,sizeof(sockaddr_in));
	LocalAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	LocalAddr.sin_family = AF_INET;
	LocalAddr.sin_port = htons(Port);
	if(bind(Server,(sockaddr*)&LocalAddr,sizeof(LocalAddr)) != 0){
		return false;
	}

	ThreadController_1 = true; pThread_1 = new std::thread([this]{this->NetReceiveFromUpdate();});
	ThreadController_2 = true; pThread_2 = new std::thread([this]{this->NetTimeoutUpdate();});

	MaxConnections = MaximunConnections;
	for(int i = 0;i<MaxConnections;i++){
		sockaddr_in pure_addr; memset(&pure_addr,0,sizeof(sockaddr_in));
		pClientList.push_back(pure_addr);
		pClientTimeUpdate.push_back(0);
		pClientInuse.push_back(false);
		pClientPing.push_back(0);
	} pNetStats.TimeWhenStarted = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();

	return true;
}

void TakeNet_UDP_Server::Shutdown(void){
	TakeNet_BitStream bsDisconnect;
	bsDisconnect.Write<short>(ID_PACKET_DISCONNECT);
	BroadcastData(bsDisconnect);
	closesocket(Server);
	WSACleanup();
	MaxConnections = 0;
	ThreadController_1 = false;
	ThreadController_2 = false;
	memset(&Server,0,sizeof(SOCKET));
	memset(&Wsd,0,sizeof(WSADATA));
	pPackets.erase(pPackets.begin(),pPackets.end());
	pClientList.erase(pClientList.begin(),pClientList.end());
	pClientTimeUpdate.erase(pClientTimeUpdate.begin(),pClientTimeUpdate.end());
	pClientInuse.erase(pClientInuse.begin(),pClientInuse.end());
	pClientPing.erase(pClientPing.begin(),pClientPing.end());
	memset(&pNetStats,0,sizeof(NetStats));

	if(pThread_1 != NULL){
		pThread_1->detach();
		delete pThread_1;
		pThread_1 = NULL;
	}

	if(pThread_2 != NULL){
		pThread_2->detach();
		delete pThread_2;
		pThread_2 = NULL;
	}
}

void TakeNet_UDP_Server::SendData(TakeNet_BitStream pSendBitStream, int ClientID){
	if((ClientID < MaxConnections) && (ClientID > -1)){
		if(pClientInuse[ClientID]){
			pNetStats.BytesSent += pSendBitStream.GetSize();
			sendto(Server,(char*)pSendBitStream.GetData(),pSendBitStream.GetSize(),0,(sockaddr*)&pClientList[ClientID],sizeof(pClientList[ClientID]));
		}
	}
}

void TakeNet_UDP_Server::BroadcastData(TakeNet_BitStream pSendBitStream){
	for(int i = 0;i<MaxConnections;i++){
		SendData(pSendBitStream,i);
	}
}

void TakeNet_UDP_Server::DisconnectClient(int ClientID){
	TakeNet_BitStream bsDisconnect;
	bsDisconnect.Write<short>(ID_PACKET_DISCONNECT);
	bsDisconnect.ReplaceFromAddr(pClientList[ClientID]);
	SendData(bsDisconnect,ClientID);
	pPackets.push_back(bsDisconnect);
}

int TakeNet_UDP_Server::GetClientPing(int ClientID){
	if((ClientID < MaxConnections) && (ClientID > -1)){
		return (int)pClientPing[ClientID];
	} return 0;
}

int TakeNet_UDP_Server::GetActiveConnections(void){
	int actives = 0;
	for(int i = 0;i<MaxConnections;i++){
		if(pClientInuse[i]){
			actives++;
		}
	} return actives;
}

char *TakeNet_UDP_Server::GetIPAddressFromBitStream(TakeNet_BitStream *pBitStream){
	return inet_ntoa(pBitStream->GetFromAddr().sin_addr);
}

char *TakeNet_UDP_Server::GetClientIPAddress(int ClientID){
	if((ClientID < MaxConnections) && (ClientID > -1)){
		return inet_ntoa(pClientList[ClientID].sin_addr);
	} return (char*)"0.0.0.0";
}

TakeNet_BitStream TakeNet_UDP_Server::GetPacket(int PacketID,int &ClientID){
	TakeNet_BitStream bsPacket;
	bsPacket.Setup(pPackets[PacketID].GetData(),pPackets[PacketID].GetSize(),pPackets[PacketID].GetFromAddr());

	ClientID = GetClientIDFromBitStream(&bsPacket);
	short packet_id; bsPacket.Read<short>(packet_id);

	if(packet_id == ID_PACKET_CONNECT){
		if(GetActiveConnections() >= MaxConnections){
			TakeNet_BitStream bsOut;
			short PACKET_ID = ID_PACKET_NO_FREE_CONNECTIONS;
			bsOut.Write<short>(PACKET_ID);
			pNetStats.BytesSent += bsOut.GetSize();
			sendto(Server,(char*)bsOut.GetData(),bsOut.GetSize(),0,(sockaddr*)&bsPacket.GetFromAddr(),sizeof(bsPacket.GetFromAddr()));
			bsPacket.Replace<short>((unsigned char*)&PACKET_ID,0);
			ClientID = GetActiveConnections();
		}else{
			TakeNet_BitStream bsOut;
			bsOut.Write<short>(ID_PACKET_CONNECT);
			pNetStats.BytesSent += bsOut.GetSize();
			sendto(Server,(char*)bsOut.GetData(),bsOut.GetSize(),0,(sockaddr*)&bsPacket.GetFromAddr(),sizeof(bsPacket.GetFromAddr()));
			int FreeClientID = GetFreeIDFromClientInuse();
			if(FreeClientID != -1){
				pClientList[FreeClientID] = bsPacket.GetFromAddr();
				pClientTimeUpdate[FreeClientID] = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
				pClientInuse[FreeClientID] = true;
				ClientID = GetClientIDFromBitStream(&bsPacket);
			}
		}
	}else if(packet_id == ID_PACKET_DISCONNECT){
		if((ClientID != -1)){
			if(pClientInuse[ClientID]){
				sockaddr_in pure_addr; memset(&pure_addr,0,sizeof(sockaddr_in));
				pClientList[ClientID] = pure_addr;
				pClientTimeUpdate[ClientID] = 0;
				pClientInuse[ClientID] = false;
			}
		}

	}else if(packet_id == ID_PACKET_PING){
		if((ClientID != -1)){
			if(pClientInuse[ClientID]){
				TakeNet_BitStream bsOut;
				bsOut.Write<short>(ID_PACKET_PING);
				pNetStats.BytesSent += bsOut.GetSize();
				sendto(Server,(char*)bsOut.GetData(),bsOut.GetSize(),0,(sockaddr*)&bsPacket.GetFromAddr(),sizeof(bsPacket.GetFromAddr()));
				__int64 now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
				pClientPing[ClientID] = (now-pClientTimeUpdate[ClientID])-1000;
				if(pClientPing[ClientID] < 0){
					pClientPing[ClientID] = 0;
				}
			}
		}

	}else if(packet_id == ID_PACKET_TIMEOUT){
		if((ClientID != -1)){
			if(pClientInuse[ClientID]){
				sockaddr_in pure_addr; memset(&pure_addr,0,sizeof(sockaddr_in));
				pClientList[ClientID] = pure_addr;
				pClientTimeUpdate[ClientID] = 0;
				pClientInuse[ClientID] = false;
			}
		}
	}

	if((ClientID != -1)){
		if(pClientInuse[ClientID]){
			pClientTimeUpdate[ClientID] = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
		}
	}

	bsPacket.VoidBitStreamPosition();
	pPackets.erase(pPackets.begin()+PacketID);

	if((ClientID == -1)){
		bsPacket.VoidBitStream();
		bsPacket.VoidBitStreamPosition();
	}

	return bsPacket;
}

int TakeNet_UDP_Server::GetPacketsCount(void){
	return pPackets.size();
}

TakeNet_UDP_Server::NetStats TakeNet_UDP_Server::GetServerNetStats(void){
	return pNetStats;
}