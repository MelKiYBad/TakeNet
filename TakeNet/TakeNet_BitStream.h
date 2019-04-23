#pragma once

#ifdef WIN32
#include <WinSock.h>
#else
#include <netinet/in.h>
#endif

#include <vector>

enum TAKENET_PACKET{
	ID_PACKET_CONNECT,
	ID_PACKET_DISCONNECT,
	ID_PACKET_NO_FREE_CONNECTIONS,
	ID_PACKET_TIMEOUT,
	ID_PACKET_PING,
	ID_TAKENET_PACKET_ENUM
};

class TakeNet_BitStream{
private:
	std::vector<unsigned char> pBytes;
	sockaddr_in from_addr;
	int n_pos;
public:
	TakeNet_BitStream(void){
		memset(&from_addr,0,sizeof(sockaddr_in));
		n_pos = 0;
	}

	TakeNet_BitStream(unsigned char *pData,size_t Size){
		Setup(pData,Size);
	}

	~TakeNet_BitStream(void){
		VoidBitStream();
		VoidBitStreamPosition();
		memset(&from_addr,0,sizeof(sockaddr_in));
	}

	void Setup(unsigned char *pData,size_t Size){
		n_pos = 0;
		for(size_t i = 0;i<Size;i++){
			pBytes.push_back(pData[i]);
		}
	}

	void Setup(unsigned char *pData,size_t Size,sockaddr_in f_addr){
		n_pos = 0;
		for(size_t i = 0;i<Size;i++){
			pBytes.push_back(pData[i]);
		} memcpy(&from_addr,&f_addr,sizeof(sockaddr_in));
	}

	template<typename type>
	void Replace(unsigned char *pData,size_t Position){
		if(pBytes.size() > 0){
			int type_size = sizeof(type);
			for(int i = 0;i<type_size;i++){
				pBytes[Position+i] = pData[i];
			}
		}
	}

	void ReplaceFromAddr(sockaddr_in from){
		memcpy(&from_addr,&from,sizeof(sockaddr_in));
	}

	template <typename type>
	void Read(type &pData){
		if(pBytes.size() > 0){
			int type_size = sizeof(type);
			memcpy(&pData,pBytes.data()+n_pos,type_size);
			n_pos += type_size;
		}else{
			pData = -1;
		}
	}

	template <typename type>
	void Read(type &pData,size_t DataSize){
		if(pBytes.size() > 0){
			int type_size = DataSize;
			memcpy(&pData,pBytes.data()+n_pos,type_size);
			n_pos += type_size;
		}else{
			pData = -1;
		}
	}
	
	void ReadString(char *string,size_t size){
		if(pBytes.size() > 0){
			for(size_t i = 0;i<size;i++){
				Read<char>(string[i],1);
			}
		}
	}

	template <typename type>
	void Write(type pData){
		int type_size = sizeof(type);
		unsigned char *t = (unsigned char*)&pData;
		for(int i = 0;i<type_size;i++){
			pBytes.push_back(t[i]);
		}
	}

	template <typename type>
	void Write(type pData,size_t DataSize){
		int type_size = DataSize;
		unsigned char *t = (unsigned char*)&pData;
		for(int i = 0;i<type_size;i++){
			pBytes.push_back(t[i]);
		}
	}
	
	void WriteString(char *string, size_t size){
		for(size_t i = 0;i<size;i++){
			Write<char>(string[i],1);
		}
	}

	void VoidBitStreamPosition(void){
		n_pos = 0;
	}

	void VoidBitStream(void){
		if(pBytes.size() > 0){
			pBytes.erase(pBytes.begin(),pBytes.end());
		}
	}

	unsigned char *GetData(void){
		return pBytes.data();
	}

	size_t GetSize(void){
		return pBytes.size();
	}

	sockaddr_in GetFromAddr(void){
		return from_addr;
	}
};
