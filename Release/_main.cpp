#define MAKE_SERVER // MAKE_CLIENT

#include <iostream>
#include "TakeNet_BitStream.h"
#include "TakeNet_UDP_Client.h"
#include "TakeNet_UDP_Server.h"

int main(int argc,char **argv){

#ifdef MAKE_SERVER
	TakeNet_UDP_Server *pServer = new TakeNet_UDP_Server();
	pServer->Startup(4800,32);

	for(;;){
		for(int i = 0;i<pServer->GetPacketsCount();i++){
			int ClientID; TakeNet_BitStream pReceiveBitStream = pServer->GetPacket(i,ClientID);
			short PacketID; pReceiveBitStream.Read<short>(PacketID);
		
			switch(PacketID){
				case ID_PACKET_CONNECT:{
					std::cout << "Connected: " << ClientID << std::endl;
				} break;

				case ID_PACKET_DISCONNECT:{
					std::cout << "Disconnected: " << ClientID << std::endl;
				} break;

				case ID_PACKET_TIMEOUT:{
					std::cout << "Timeout: " << ClientID << std::endl;
				} break;

				case ID_PACKET_NO_FREE_CONNECTIONS:{
					std::cout << "No free for: " << ClientID << std::endl;
				} break;

				case ID_PACKET_PING:{
					std::cout << "Ping: " << ClientID << std::endl;
				} break;
			}  pReceiveBitStream.VoidBitStream();
		}
	}

	pServer->Shutdown();
	delete pServer;

#elif defined MAKE_CLIENT

	TakeNet_UDP_Client *pClient = new TakeNet_UDP_Client();
	pClient->Connect((char*)"127.0.0.1",4800);

	for(;;){
		for(int i = 0;i<pClient->GetPacketsCount();i++){
			TakeNet_BitStream pReceiveBitStream = pClient->GetPacket(i);
			short PacketID; pReceiveBitStream.Read<short>(PacketID);

			switch(PacketID){
				case ID_PACKET_CONNECT:{
					std::cout << "Client connected" << std::endl;
				} break;

				case ID_PACKET_DISCONNECT:{
					std::cout << "Client disconnected" << std::endl;
				} break;

				case ID_PACKET_TIMEOUT:{
					std::cout << "Client timeout" << std::endl;
				} break;

				case ID_PACKET_NO_FREE_CONNECTIONS:{
					std::cout << "Server is full" << std::endl;
				} break;

				case ID_PACKET_PING:{
					std::cout << "Client ping" << std::endl;
				} break;
			} pReceiveBitStream.VoidBitStream();
		}
	}

	pClient->Disconnect();
	delete pClient;

#endif

	system("PAUSE");
	return 0;
}
