#pragma once

#include "stdafx.h"
#include "AudioController.h"
#include "Server.h"


//Enumerate the types of packets sent from the client to the server
enum PacketTypeClientToServer
{
	//A packet requesting a list of all the songs from the server
	CLIENT_TO_SERVER_REQUEST_SONG_LIST = 1,
	//A packet containing the number of the chosen song
	CLIENT_TO_SERVER_CHOOSE_SONG = 2,
	//A packet requesting a chunk of raw data
	CLIENT_TO_SERVER_GET_DATA = 3,
	//A packet telling the server to terminate the connection
	CLIENT_TO_SERVER_TERMINATE = 4
};

//Enumerate the types of packets sent from the server to the client
enum PacketTypeServerToClient
{
	//A packet containing the total number of songs on the server
	SERVER_TO_CLIENT_FILES_NUMBER = 1,
	//A packet containing the name of a song
	SERVER_TO_CLIENT_FILE_NAME = 2,
	//A packet containing the Wave Format information
	SERVER_TO_CLIENT_WAVE_FORMAT = 3,
	//A packet containing the size of the Data chunk in bytes
	SERVER_TO_CLIENT_DATA_SIZE = 4,
	//A packet containing a chunk of raw data
	SERVER_TO_CLIENT_DATA_CHUNK = 5
};


class Client
{
public:
	Client();
	Client(SOCKET socket, std::shared_ptr<Server> server, std::shared_ptr<AudioController> audioController);
	std::vector<std::string> ListFiles();
	bool SendFilesListToClient(std::vector<std::string> files);
	bool OpenFileAndSendFileInfo(std::string fileName);
	SOCKET GetSocket();
	void SendNextTwoSeconds();
	std::shared_ptr<char> Receive(int bufferSize);
	int Send(std::shared_ptr<char> buffer, int bufferSize, int flags);
	~Client();

private:
	SOCKET								_socket;	
	std::shared_ptr<Server>				_server;
	std::shared_ptr<AudioController>	_audioController;
	HANDLE								_fileHandle;
	std::string							_fmtChunk;
	std::string							_fileName;
	WAVEFORMATEX						_waveFormat;
	DWORD								_dataChunkSize;
	DWORD								_dataChunkPosition;
	std::shared_ptr<BYTE>				_dataBuffer;
	bool								_terminated;

	std::shared_ptr<char>				_overflowBuffer;
	int									_overflowSize;

	int									_currentOffset;
};

