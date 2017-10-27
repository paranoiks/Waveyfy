#pragma once
#pragma warning( disable : 4251) 

#ifdef WAVEYFYPLAYERDLL_EXPORTS
#define WAVEYFYPLAYERDLL_API __declspec(dllexport)
#else
#define WAVEYFYPLAYERDLL_API __declspec(dllimport)
#endif

#include "stdafx.h"
#include <memory>
#include <thread>

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

class Player_PIMpl;

class WAVEYFYPLAYERDLL_API WaveyfyPlayer
{
public:
	WaveyfyPlayer();
	bool					SetUpConnection();
	bool					WaveFileInitialise();
	bool					RequestFileList();
	bool					ReceiveFileList(char* buff, int len);
	bool					SendSongChoice(int index);
	bool					ReceiveFileInfo();
	bool					ReceiveSongSize();
	bool					SetDirectSoundNotifications();
	bool					Stream();
	bool					Stop();
	bool					Pause();
	bool					Resume();
	int						GetProgress();
	bool					SetVolume(int volume);
	bool					SendDataRequest();
	bool					CollectPlayBufferData();
	std::shared_ptr<char>	Receive(int bufferSize);
	~WaveyfyPlayer();

private:
	std::unique_ptr<Player_PIMpl>	_playerPImpl = std::make_unique<Player_PIMpl>();
};

extern "C"
{
	WAVEYFYPLAYERDLL_API WaveyfyPlayer* CreateWaveyfyPlayer();
	WAVEYFYPLAYERDLL_API bool SetUpConnection(WaveyfyPlayer* playerObject);
	WAVEYFYPLAYERDLL_API bool WaveFileInitialise(WaveyfyPlayer* playerObject);
	WAVEYFYPLAYERDLL_API bool RequestFileList(WaveyfyPlayer* playerObject);
	WAVEYFYPLAYERDLL_API bool ReceiveFileList(WaveyfyPlayer* playerObject, char* buff, int len);
	WAVEYFYPLAYERDLL_API bool SendSongChoice(WaveyfyPlayer* playerObject, int index);
	WAVEYFYPLAYERDLL_API bool ReceiveFileInfo(WaveyfyPlayer* playerObject);
	WAVEYFYPLAYERDLL_API bool ReceiveSongSize(WaveyfyPlayer* playerObject);
	WAVEYFYPLAYERDLL_API bool SetDirectSoundNotifications(WaveyfyPlayer* playerObject);
	WAVEYFYPLAYERDLL_API bool Stream(WaveyfyPlayer* playerObject);
	WAVEYFYPLAYERDLL_API bool Stop(WaveyfyPlayer* playerObject);
	WAVEYFYPLAYERDLL_API bool Pause(WaveyfyPlayer* playerObject);
	WAVEYFYPLAYERDLL_API bool Resume(WaveyfyPlayer* playerObject);
	WAVEYFYPLAYERDLL_API int GetProgress(WaveyfyPlayer* playerObject);
	WAVEYFYPLAYERDLL_API bool SetVolume(WaveyfyPlayer* playerObject, int volume);
}

