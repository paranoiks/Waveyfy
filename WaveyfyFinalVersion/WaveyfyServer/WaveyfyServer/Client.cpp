#include "Client.h"
#include "stdafx.h"
#include <thread>
Client::Client()
{
}

Client::Client(SOCKET socket, std::shared_ptr<Server> server, std::shared_ptr<AudioController> audioController)
{
	_server = server;
	_terminated = false;
	_socket = socket;
	_audioController = audioController;
	_dataBuffer = nullptr;
	std::vector<std::string> files = ListFiles();

	//initialise the overflow variables
	//the overflow buffer will be used when receiving packets of different sizes and putting them together
	//see the comments in Receive() for further details
	_overflowBuffer = (std::shared_ptr<char>)(new char[DEFAULT_BUFLEN]);
	memset(_overflowBuffer.get(), 0, DEFAULT_BUFLEN);
	_overflowSize = 0;

	//the currentOffset tracks how many bytes of raw data we have sent to the client
	//it needs to be reset every time the client chooses a new song
	_currentOffset = 0;
	
	//while the conncetion with the client is active receive packets
	//every packet comming from a client starts with an int which tells us what type it is
	while (true)
	{
		if (_terminated)
		{
			break;
		}
		//receive the packet and read the first 4 bytes to determine the packet type
		std::shared_ptr<char> buffer = Receive(DEFAULT_BUFLEN);
		if (_terminated)
		{
			break;
		}

		int packetType = 0;
		memcpy(&packetType, buffer.get(), sizeof(int));		

		if (packetType == CLIENT_TO_SERVER_REQUEST_SONG_LIST)
		{
			//if the client requested a list of the songs send it to them
			SendFilesListToClient(files);
		}
		else if (packetType == CLIENT_TO_SERVER_CHOOSE_SONG)
		{
			//if the client has chosen a song, get its number (the second 4 bytes in the packet)
			//after that open the corresponding file and send some data from it to the client
			int songNumber;
			memcpy(&songNumber, buffer.get() + sizeof(int), sizeof(int));
			OpenFileAndSendFileInfo(files[songNumber]);
			//reset the currentOffset every time a new song is played, since it tracks how many bytes of raw data we have sent
			_currentOffset = 0;
		}
		else if (packetType == CLIENT_TO_SERVER_GET_DATA)
		{
			//if the client asks for a chunk of raw data, check if there is actually any data to send
			if (_currentOffset >= _dataChunkSize)
			{
				break;
			}
			//if we still haven't reached the end of the song, send the next two secodns worth of data to the client
			SendNextTwoSeconds();
		}
		else if (packetType == CLIENT_TO_SERVER_TERMINATE)
		{
			//TERMINATE
		}
	}
}

/// <summary>
/// Get all the files in a directory
/// the code is borrowed from here: http://stackoverflow.com/questions/20860822/finding-the-file-path-of-all-files-in-a-folder
/// Only look fro files with the ".wav" extension
/// </summary>
std::vector<std::string> Client::ListFiles()
{
	char buffer[MAX_PATH];
	GetModuleFileNameA(NULL, buffer, MAX_PATH);
	std::string::size_type pos = std::string(buffer).find_last_of("\\/");
	std::string result = std::string(buffer).substr(0, pos);
	result = result += "\\";

	std::vector<std::string> names;
	char search_path[200];
	sprintf_s(search_path, "%s*.wav*", result.c_str());
	WIN32_FIND_DATAA fd;
	HANDLE hFind = ::FindFirstFileA(search_path, &fd);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			// read all (real) files in current folder, delete '!' read other 2 default folder . and ..
			if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				names.push_back(result + fd.cFileName);
			}
		} while (::FindNextFileA(hFind, &fd));
		::FindClose(hFind);
	}
		
	return names;
}

/// <summary>
/// Send a list of all the songs to the client
/// </summary>
bool Client::SendFilesListToClient(std::vector<std::string> files)
{
	std::shared_ptr<char> buffer(new char[DEFAULT_BUFLEN]);

	//send a packet to the client, telling it to expect a packet with the number of songs
	int packetType = SERVER_TO_CLIENT_FILES_NUMBER;
	memcpy(buffer.get(), &packetType, sizeof(int));
	Send(buffer, DEFAULT_BUFLEN, 0);

	//send a packet to the client containing the number of songs
	int numberOfFiles = files.size();
	memcpy(buffer.get(), &numberOfFiles, sizeof(int));
	Send(buffer, DEFAULT_BUFLEN, 0);

	//send two packets for each file
	//the first paket is a "cotrol" one, telling the client what to expect next
	//the second packet contains the name of the file, terminated with '\0'
	for (int i = 0; i < files.size(); i++)
	{
		packetType = SERVER_TO_CLIENT_FILE_NAME;
		memcpy(buffer.get(), &packetType, sizeof(int));
		Send(buffer, DEFAULT_BUFLEN, 0);

		memcpy(buffer.get(), files[i].c_str(), files[i].size());
		memset(buffer.get() + strlen(files[i].c_str()), '\0', 1);
		Send(buffer, DEFAULT_BUFLEN, 0);
	}

	return false;
}

/// <summary>
/// Open the file chosen by the client and send its info to the client
/// first send the Wave Format data, then send the size of the data chunk in bytes
/// </summary>
bool Client::OpenFileAndSendFileInfo(std::string fileName)
{
	std::shared_ptr<char> waveFormatBuffer(new char[DEFAULT_BUFLEN]);

	//send a packet to the client, telling it to expect a packet containing the wave format data next
	int packetType = SERVER_TO_CLIENT_WAVE_FORMAT;
	memcpy(waveFormatBuffer.get(), &packetType, sizeof(int));
	Send(waveFormatBuffer, DEFAULT_BUFLEN, 0);

	//open the wave file and get the wave format data and the size of the data chunk from it
	std::wstring* widestr = new std::wstring(fileName.begin(), fileName.end());	
	wchar_t* widecstr = const_cast<wchar_t*>(widestr->c_str());
	bool fileOpened = _audioController->LoadWaveFile(widecstr, _fileHandle, _waveFormat, waveFormatBuffer, _dataChunkSize, _dataChunkPosition, _dataBuffer);
	if (fileOpened)
	{
		//send the buffer, which now contains the wave format data
		Send(waveFormatBuffer, DEFAULT_BUFLEN, 0);
	}
	
	//c_str does not change owner => NO SHARED POINTER on widecstr	
	delete widestr;
	widestr = nullptr;	

	std::shared_ptr<char> chunkSizeBuffer(new char[DEFAULT_BUFLEN]);
	//send a packet to the client, telling it to expect a packet with the size of the data chunk 
	packetType = SERVER_TO_CLIENT_DATA_SIZE;
	memcpy(chunkSizeBuffer.get(), &packetType, sizeof(int));
	Send(chunkSizeBuffer, DEFAULT_BUFLEN, 0);
	//send a packet to the client, containing the size of the data chunk in bytes
	memcpy(chunkSizeBuffer.get(), &_dataChunkSize, sizeof(DWORD));
	Send(chunkSizeBuffer, DEFAULT_BUFLEN, 0);

	return false;
}

/// <summary>
/// Send the raw data needed to play the next two seconds of the song
/// </summary>
void Client::SendNextTwoSeconds()
{
	std::shared_ptr<char> currentDataChunk(new char[DEFAULT_BUFLEN]);
	
	//if the current offset is bigger than the data chunk size, then there is no more raw data to read
	//we have reached the end of the song
	if (_currentOffset >= _dataChunkSize)
	{
		return;
	}

	int i = _currentOffset;

	//repeat until we have send 2 seconds worth of data
	while (i < _currentOffset + _waveFormat.nAvgBytesPerSec * 2)
	{
		//send a packet to the client, telling it to expect a packet with raw data
		int packetType = SERVER_TO_CLIENT_DATA_CHUNK;
		memcpy(currentDataChunk.get(), &packetType, sizeof(int));
		Send(currentDataChunk, DEFAULT_BUFLEN, 0);

		//if there are still DEFAULT_BUFLEN (1024) bytes left to read from the data send them
		//otherwise send a packet full of zeroes (will be played as silence in the client)
		if (i < _dataChunkSize - DEFAULT_BUFLEN)
		{
			memcpy(currentDataChunk.get(), _dataBuffer.get() + i, DEFAULT_BUFLEN);
		}
		else
		{
			memset(currentDataChunk.get(), 0, DEFAULT_BUFLEN);
		}
		
		int bytesSent = Send(currentDataChunk, DEFAULT_BUFLEN, 0);
		i += bytesSent;
	}

	//increment the counter that shows how many bytes have we read from the song so far
	_currentOffset += i - _currentOffset;
}

/// <summary>
/// Works the same way as the receive function in the client class
/// When running on different machines the packets sent between the two applications have varying sizes
/// Therefore we need to collect packets until we can "build" a whole one (of size DEFAULT_BUFLEN)
/// </summary>
std::shared_ptr<char> Client::Receive(int bufferSize)
{
	//intialise a buffer, that will be used in this function only (in the calls to recv())
	std::unique_ptr<char> buffer(new char[DEFAULT_BUFLEN]);
	memset(buffer.get(), 0, DEFAULT_BUFLEN);
		
	//initialise the buffer that will be returned by the function
	std::shared_ptr<char> realBuffer(new char[DEFAULT_BUFLEN]);
	memset(realBuffer.get(), 0, DEFAULT_BUFLEN);

	int totalBytesReceived = 0;
	//put whatever we have in the overflow buffer in the real one
	//the overflow buffer will contain any bytes that we received inside the loop but couldn't fit in the "real" buffer
	if (_overflowSize != 0)
	{
		memcpy(realBuffer.get(), _overflowBuffer.get(), _overflowSize);
		totalBytesReceived = _overflowSize;
	}

	//while the total size of the received packets is less that 1024 we keep receiving
	while (totalBytesReceived < DEFAULT_BUFLEN)
	{
		int bytesReceived = recv(_socket, buffer.get(), DEFAULT_BUFLEN, 0);
		if (bytesReceived == SOCKET_ERROR)
		{
			_terminated = true;
			return nullptr;
		}
		if (bytesReceived < 0)
		{			
			continue;
		}
		//if we have received a total of more than 1024 bytes, put some in the overflow buffer
		if (totalBytesReceived + bytesReceived > DEFAULT_BUFLEN)
		{
			int copySize = DEFAULT_BUFLEN - totalBytesReceived;
			memcpy(realBuffer.get() + totalBytesReceived, buffer.get() , copySize);
			memcpy(_overflowBuffer.get(), buffer.get() + copySize, bytesReceived - copySize);
			_overflowSize = bytesReceived - copySize;
			break;
		}
		else if (totalBytesReceived + bytesReceived == DEFAULT_BUFLEN)
		{
			memcpy(realBuffer.get() + totalBytesReceived, buffer.get(), bytesReceived);
			memset(_overflowBuffer.get(), 0, DEFAULT_BUFLEN);
			_overflowSize = 0;
			break;
		}
		//if we still have space in the "real" buffer, put whatever we have received in it
		memcpy(realBuffer.get() + totalBytesReceived, buffer.get(), bytesReceived);
		totalBytesReceived += bytesReceived;
	}

	//once the buffer contains 1024 bytes return it
	return realBuffer;
}

SOCKET Client::GetSocket()
{
	return _socket;
}

int Client::Send(std::shared_ptr<char> buffer, int bufferSize, int flags)
{	
	return send(_socket, buffer.get(), bufferSize, flags);
}

Client::~Client()
{
}
