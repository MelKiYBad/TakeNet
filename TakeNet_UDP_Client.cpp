#include "TakeNet_UDP_Client.h"

bool TakeNet_UDP_Client::WaitState(uint64_t *desc, int id, int ms){
	uint64_t then =  std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	if(desc[id] <= 0){
		desc[id] = then;
	}
	if(then-desc[id] > ms){
		desc[id] = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		return true;
	} return false;
}

int TakeNet_UDP_Client::NetReceiveFromUpdate(void){
	while(ThreadController_1){
		std::vector<char> pRecvBytesData;
		TakeNet_BitStream pPacketBitStream;
		char RecvBytes[64]; int hostlen = sizeof(host);
		int bytes = 0;

		while(bytes = recvfrom(Client,RecvBytes,64,0,(sockaddr*)&host,&hostlen)){
			if(bytes == -1){
				break;
			}else{
				for(int i = 0;i<bytes;i++){
					pRecvBytesData.push_back(RecvBytes[i]);
				} if(bytes < 64){
					break;
				}
			}
		}

		if(bytes == -1){
			TakeNet_BitStream bsTimeout;
			bsTimeout.Write<short>(ID_PACKET_TIMEOUT);
			pPackets.push_back(bsTimeout);
		}
			
		pPacketBitStream.Setup((unsigned char*)pRecvBytesData.data(),pRecvBytesData.size());
		pPackets.push_back(pPacketBitStream);
		std::this_thread::sleep_for(std::chrono::nanoseconds(1));
	}
	return 0;
}

int TakeNet_UDP_Client::NetPingUpdate(void){
	while(ThreadController_2){
		if(IsConnected){
			TakeNet_BitStream bsOut;
			bsOut.Write<short>(ID_PACKET_PING);
			sendto(Client,(char*)bsOut.GetData(),bsOut.GetSize(),0,(sockaddr*)&host,sizeof(host));
		} std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
	return 0;
}

TakeNet_UDP_Client::TakeNet_UDP_Client(void){
	ThreadController_1 = false;
	ThreadController_2 = false;
	memset(&Client,0,sizeof(SOCKET));
	memset(&Wsd,0,sizeof(WSADATA));
	memset(&host,0,sizeof(sockaddr_in));
	pThread_1 = NULL;
	pThread_2 = NULL;
	IsConnected = false;
}

TakeNet_UDP_Client::TakeNet_UDP_Client(char *IpAddress,unsigned short Port){
	Connect(IpAddress,Port);
}

TakeNet_UDP_Client::~TakeNet_UDP_Client(void){
	Disconnect();
}

bool TakeNet_UDP_Client::Connect(char *IpAddress,unsigned short Port){
	if(!IsConnected){
		WSAStartup(MAKEWORD(2,2),&Wsd);
		Client = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
		if(Client == INVALID_SOCKET){
			return false;
		}

		hostent *hst = gethostbyname(IpAddress);
		host.sin_addr.s_addr = ((in_addr*)hst->h_addr_list[0])->s_addr;
		host.sin_family = AF_INET;
		host.sin_port = htons(Port);

		TakeNet_BitStream bsOut;
		bsOut.Write<short>(ID_PACKET_CONNECT);
		sendto(Client,(char*)bsOut.GetData(),bsOut.GetSize(),0,(sockaddr*)&host,sizeof(host));
		ThreadController_1 = true; pThread_1 = new std::thread([this]{this->NetReceiveFromUpdate();});

		return true;
	}

	return false;
}

void TakeNet_UDP_Client::Disconnect(void){
	if(IsConnected){
		TakeNet_BitStream bsOut;
		bsOut.Write<short>(ID_PACKET_DISCONNECT);
		sendto(Client,(char*)bsOut.GetData(),bsOut.GetSize(),0,(sockaddr*)&host,sizeof(host));
	}

	closesocket(Client);
	WSACleanup();

	ThreadController_1 = false;
	ThreadController_2 = false;
	memset(&Client,0,sizeof(SOCKET));
	memset(&Wsd,0,sizeof(WSADATA));
	memset(&host,0,sizeof(sockaddr_in));

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

	IsConnected = false;
}

void TakeNet_UDP_Client::SendData(TakeNet_BitStream pSendBitStream){
	if(IsConnected){
		sendto(Client,(char*)pSendBitStream.GetData(),pSendBitStream.GetSize(),0,(sockaddr*)&host,sizeof(host));
	}
}

TakeNet_BitStream TakeNet_UDP_Client::GetPacket(int PacketID){
	TakeNet_BitStream bsPacket;
	bsPacket.Setup(pPackets[PacketID].GetData(),pPackets[PacketID].GetSize(),pPackets[PacketID].GetFromAddr());

	short packet_id; bsPacket.Read<short>(packet_id);
	if(packet_id == ID_PACKET_CONNECT){
		IsConnected = true;
		ThreadController_2 = true; pThread_2 = new std::thread([this]{this->NetPingUpdate();});

	}else if((packet_id == ID_PACKET_NO_FREE_CONNECTIONS) || (packet_id == ID_PACKET_DISCONNECT) || (packet_id == ID_PACKET_TIMEOUT)){
		Disconnect();

	}

	bsPacket.VoidBitStreamPosition();
	pPackets.erase(pPackets.begin()+PacketID);
	return bsPacket;
}

int TakeNet_UDP_Client::GetPacketsCount(void){
	return pPackets.size();
}