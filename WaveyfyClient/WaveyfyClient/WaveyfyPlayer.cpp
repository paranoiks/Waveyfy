#include "stdafx.h"
#include "WaveyfyPlayer.h"
#include <fstream>
#include <string>
#include <iostream>

class Player_PIMpl
{
public:
	void Initialise()
	{
		_secondaryBuffer = nullptr;
		_playing = false;
		_serverTerminated = false;

		//the first packet we expect is going to be a control packet 
		//(this variable will be switched to true on the first line of Receive())
		_packetIsControl = false;

		//initialise the overflow buffer that will be used in receiving packets of different sizes and putting them together
		//see the comments in Receive() for further info
		_overflowBuffer = (std::shared_ptr<char>)(new char[DEFAULT_BUFLEN]);
		memset(_overflowBuffer.get(), 0, DEFAULT_BUFLEN);
		_overflowSize = 0;

		_stopped = false;
		_paused = false;

		SetUpConnection();
		WaveFileInitialise();
	}

	/// <summary>
	/// Set up a connection with the server
	/// </summary>
	bool SetUpConnection()
	{
		std::string hostAddressString;
		std::ifstream myfile("ip.txt");
		if (myfile.is_open())
		{
			while (getline(myfile, hostAddressString))
			{
			}
			myfile.close();
		}
		else
		{
		}

		WSADATA wsaData;
		int status;
		struct addrinfo * data;
		struct addrinfo hints;

		status = WSAStartup(MAKEWORD(WINSOCK_MAJOR_VERSION, WINSOCK_MINOR_VERSION), &wsaData);
		if (status != 0)
		{
			std::cout << "Unable to start sockets layer" << std::endl;
		}
		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_IP;
		hints.ai_flags = AI_PASSIVE;
		if (getaddrinfo(hostAddressString.c_str(), hostPort, &hints, &data))
		{
			std::cout << "Unable to translate host name to address" << std::endl;
			WSACleanup();
		}

		_connectSocket = socket(data->ai_family, data->ai_socktype, data->ai_protocol);
		auto c = connect(_connectSocket, data->ai_addr, data->ai_addrlen);

		return true;
	}

	/// <summary>
	/// Initialise an empty wave file
	/// </summary>
	/// <param name="disposing"></param>
	bool WaveFileInitialise()
	{
		HRESULT result;

		// Initialize the direct sound interface pointer for the default sound device.
		result = DirectSoundCreate8(NULL, &_directSound, NULL);
		if (FAILED(result))
		{
			return false;
		}

		// Set the cooperative level to priority so the format of the primary sound buffer can be modified.
		// We use the handle of the desktop window since we are a console application.  If you do write a 
		// graphical application, you should use the HWnd of the graphical application. 
		result = _directSound->SetCooperativeLevel(GetDesktopWindow(), DSSCL_PRIORITY);
		if (FAILED(result))
		{
			return false;
		}

		// Setup the primary buffer description.
		_bufferDesc.dwSize = sizeof(DSBUFFERDESC);
		_bufferDesc.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLVOLUME;
		_bufferDesc.dwBufferBytes = 0;
		_bufferDesc.dwReserved = 0;
		_bufferDesc.lpwfxFormat = NULL;
		_bufferDesc.guid3DAlgorithm = GUID_NULL;

		// Get control of the primary sound buffer on the default sound device.
		result = _directSound->CreateSoundBuffer(&_bufferDesc, &_primaryBuffer, NULL);
		if (FAILED(result))
		{
			return false;
		}

		// Setup the format of the primary sound bufffer.
		// In this case it is a .WAV file recorded at 44,100 samples per second in 16-bit stereo (cd audio format).
		// Really, we should set this up from the wave file format loaded from the file.
		_waveFormat.wFormatTag = WAVE_FORMAT_PCM;
		_waveFormat.nSamplesPerSec = 44100;
		_waveFormat.wBitsPerSample = 16;
		_waveFormat.nChannels = 2;
		_waveFormat.nBlockAlign = (_waveFormat.wBitsPerSample / 8) * _waveFormat.nChannels;
		_waveFormat.nAvgBytesPerSec = _waveFormat.nSamplesPerSec * _waveFormat.nBlockAlign;
		_waveFormat.cbSize = 0;

		// Set the primary buffer to be the wave format specified.
		result = _primaryBuffer->SetFormat(&_waveFormat);
		if (FAILED(result))
		{
			return false;
		}
		return true;
	}

	/// <summary>
	/// Send a packet to the server, requesting a list with the songs
	/// </summary>
	/// <param name="disposing"></param>
	bool RequestFileList()
	{
		std::unique_ptr<char> buffer(new char[DEFAULT_BUFLEN]);
		int packetType = CLIENT_TO_SERVER_REQUEST_SONG_LIST;
		memcpy(buffer.get(), &packetType, sizeof(int));
		send(_connectSocket, buffer.get(), DEFAULT_BUFLEN, 0);

		return true;
	}

	/// <summary>
	/// Receive a all the file names from the server
	/// </summary>
	bool ReceiveFileList(char* buff, int len)
	{
		int packetType = -1;
		std::shared_ptr<char> buffer(new char[DEFAULT_BUFLEN]);
		//wait until we receive a control packet telling us that the next packet will contain the number of files
		while (packetType != SERVER_TO_CLIENT_FILES_NUMBER)
		{
			buffer = Receive(DEFAULT_BUFLEN);
			if (_serverTerminated)
			{
				return false;
			}

			memcpy(&packetType, buffer.get(), sizeof(int));
		}

		//receive the packet with the number of files in it
		buffer = Receive(DEFAULT_BUFLEN);
		if (_serverTerminated)
		{
			return false;
		}

		int numberOfFiles = -1;
		memset(&numberOfFiles, 0, sizeof(int));

		memcpy(&numberOfFiles, buffer.get(), sizeof(int));

		//get all the file paths and put them in a big buffer
		//put a * deliminator between the file names in the buffer (* is not allowed in windows directory and file names)
		int currentOffset = 0;
		for (int i = 0; i < numberOfFiles; i++)
		{
			packetType = -1;
			while (packetType != SERVER_TO_CLIENT_FILE_NAME)
			{
				buffer = Receive(DEFAULT_BUFLEN);
				if (_serverTerminated)
				{
					return false;
				}
				memcpy(&packetType, buffer.get(), sizeof(int));
			}
			buffer = Receive(DEFAULT_BUFLEN);
			if (_serverTerminated)
			{
				return false;
			}

			int nameLength = strlen(buffer.get());
			memcpy(buff + currentOffset, buffer.get(), nameLength);
			currentOffset += nameLength;
			memset(buff + currentOffset, '*', 1);
			currentOffset++;
		}

		return true;
	}

	/// <summary>
	/// Send a packet to the server, with the number of the chosen song
	/// </summary>
	bool SendSongChoice(int index)
	{
		while (_playing)
		{

		}
		int choice = index;
		std::unique_ptr<char> buffer(new char[DEFAULT_BUFLEN]);

		int type = CLIENT_TO_SERVER_CHOOSE_SONG;
		//set the first 4 bytes of the buffer to CLIENT_TO_SERVER_CHOOSE_SONG, to tell the server what type of packet this is
		memcpy(buffer.get(), &type, sizeof(int));
		//set the second 4 bytes of the buffer to the number of the song we have chosen
		memcpy(buffer.get() + sizeof(int), &choice, sizeof(int));

		send(_connectSocket, buffer.get(), DEFAULT_BUFLEN, 0);

		return true;
	}

	/// <summary>
	/// Receive info about the wave file from the server
	/// </summary>
	bool ReceiveFileInfo()
	{
		HRESULT result;
		IDirectSoundBuffer * tempBuffer;

		int packetType = -1;
		//wait until we have received a control packet, showing us that the next packet will contain the Wave Format data
		while (packetType != SERVER_TO_CLIENT_WAVE_FORMAT)
		{
			std::shared_ptr<char> receivedBuffer(Receive(DEFAULT_BUFLEN));
			if (_serverTerminated)
			{
				return false;
			}

			memcpy(&packetType, receivedBuffer.get(), sizeof(int));

			if (packetType == SERVER_TO_CLIENT_WAVE_FORMAT)
			{
				break;
			}
		}

		std::shared_ptr<char> receivedBuffer(Receive(DEFAULT_BUFLEN));
		if (_serverTerminated)
		{
			return false;
		}

		std::shared_ptr<char> tempRecvBuf(new char[DEFAULT_BUFLEN]);
		memcpy(tempRecvBuf.get(), receivedBuffer.get(), DEFAULT_BUFLEN);

		//now that we have the Wave Format data in the tempRecvBuf, use memcpy to get in the _waveFormat variable
		int pointerIndex = 0;
		memcpy(&_waveFormat.wFormatTag, tempRecvBuf.get(), sizeof(_waveFormat.wFormatTag));
		pointerIndex += sizeof(_waveFormat.wFormatTag);
		memcpy(&_waveFormat.nSamplesPerSec, tempRecvBuf.get() + pointerIndex, sizeof(_waveFormat.nSamplesPerSec));
		pointerIndex += sizeof(_waveFormat.nSamplesPerSec);
		memcpy(&_waveFormat.wBitsPerSample, tempRecvBuf.get() + pointerIndex, sizeof(_waveFormat.wBitsPerSample));
		pointerIndex += sizeof(_waveFormat.wBitsPerSample);
		memcpy(&_waveFormat.nChannels, tempRecvBuf.get() + pointerIndex, sizeof(_waveFormat.nChannels));
		pointerIndex += sizeof(_waveFormat.nChannels);
		memcpy(&_waveFormat.nBlockAlign, tempRecvBuf.get() + pointerIndex, sizeof(_waveFormat.nBlockAlign));
		pointerIndex += sizeof(_waveFormat.nBlockAlign);
		memcpy(&_waveFormat.nAvgBytesPerSec, tempRecvBuf.get() + pointerIndex, sizeof(_waveFormat.nAvgBytesPerSec));
		pointerIndex += sizeof(_waveFormat.nAvgBytesPerSec);
		memcpy(&_waveFormat.cbSize, tempRecvBuf.get() + pointerIndex, sizeof(_waveFormat.cbSize));
		pointerIndex += sizeof(_waveFormat.cbSize);

		//calculate the buffer size to be around 4 seconds worth of data (round down to the nearest multiple of 1024)
		_playBufferSize = (((2 * _waveFormat.nAvgBytesPerSec) / DEFAULT_BUFLEN) * DEFAULT_BUFLEN) * 2;

		//create the buffer description
		_bufferDesc.dwSize = sizeof(DSBUFFERDESC);
		_bufferDesc.dwFlags = DSBCAPS_CTRLVOLUME | DSBCAPS_GLOBALFOCUS | DSBCAPS_CTRLPOSITIONNOTIFY;
		_bufferDesc.dwBufferBytes = _playBufferSize;
		_bufferDesc.dwReserved = 0;
		_bufferDesc.lpwfxFormat = &_waveFormat;
		_bufferDesc.guid3DAlgorithm = GUID_NULL;

		//do checks
		result = _directSound->CreateSoundBuffer(&_bufferDesc, &tempBuffer, NULL);
		result = tempBuffer->QueryInterface(IID_IDirectSoundBuffer8, (void**)&_secondaryBuffer);

		tempBuffer->Release();
		tempBuffer = nullptr;

		//set the current position to the start and the volume to max
		result = _secondaryBuffer->SetCurrentPosition(0);
		result = _secondaryBuffer->SetVolume(DSBVOLUME_MAX);

		return true;
	}

	/// <summary>
	/// Get the song size from the server
	/// </summary>
	bool ReceiveSongSize()
	{
		//receive twice to get rid of the control packet
		int packetType = -1;
		std::shared_ptr<char> receiveBuffer(new char[DEFAULT_BUFLEN]);
		while (packetType != SERVER_TO_CLIENT_DATA_SIZE)
		{
			receiveBuffer = Receive(DEFAULT_BUFLEN);
			if (_serverTerminated)
			{
				return false;
			}

			memcpy(&packetType, receiveBuffer.get(), sizeof(int));
		}
		receiveBuffer = Receive(DEFAULT_BUFLEN);
		if (_serverTerminated)
		{
			return false;
		}

		//copy the song size inside a member variable
		memcpy(&_songSize, receiveBuffer.get(), sizeof(DWORD));

		return true;
	}

	bool SetDirectSoundNotifications()
	{
		HRESULT result;

		//SET ALL THE EVENTS
		_playEventHandles[0] = CreateEvent(NULL, FALSE, FALSE, NULL);
		_playEventHandles[1] = CreateEvent(NULL, FALSE, FALSE, NULL);
		_playEventHandles[2] = CreateEvent(NULL, FALSE, FALSE, NULL);

		result = _secondaryBuffer->QueryInterface(IID_IDirectSoundNotify8, (LPVOID*)&_directSoundNotify);
		//set a notification at 1/4 of the buffer
		_positionNotify[0].dwOffset = _playBufferSize / 4;
		_positionNotify[0].hEventNotify = _playEventHandles[0];

		//set a notification at 3/4 of the buffer
		_positionNotify[1].dwOffset = (_playBufferSize / 4) * 3;
		_positionNotify[1].hEventNotify = _playEventHandles[1];

		//set a notification at the exact position where the song will end
		int exactPosition = _songSize % _playBufferSize;
		_positionNotify[2].dwOffset = exactPosition;
		_positionNotify[2].hEventNotify = _playEventHandles[2];

		_directSoundNotify->SetNotificationPositions(3, _positionNotify);
		_directSoundNotify->Release();

		return true;
	}

	/// <summary>
	/// Handle the streaming of raw data from the server
	/// on every step of the loop, the client asks the server for the next 2 sedonds worth of data
	/// after that the client receives packets until it has 2 seconds worth of raw data
	/// copies everything in the secondary buffer and plays it
	/// </summary>
	bool Stream()
	{
		//we are currently playing, the playback has not beena rtificially stopped
		_playing = true;
		_stopped = false;

		//these are left to be raw pointers as we don't know what hapens to them in the Lock/Unlock functions
		//when trying to make them smart pointers, the code threw an exception
		unsigned char * bufferPtr1;
		unsigned long   bufferSize1;
		unsigned char * bufferPtr2;
		unsigned long   bufferSize2;

		//a variable to know whether we are currently copying into the first or the second half of the buffer
		bool secondHalf = false;

		while (true)
		{
			//if the playback has been stopped or paused break the loop
			if (_stopped || _paused)
			{
				break;
			}

			//a variable to know if we have reached the final part (the final 2 seconds) of the song
			_finalPart = false;

			//send a packet to the server, asking for data
			SendDataRequest();

			//collect the packets from the server (total of 2 seconds worth of packets)
			CollectPlayBufferData();

			//if teh server has exited for some reason, return
			if (_serverTerminated)
			{
				return false;
			}

			HRESULT result;

			_currentPlayBufferOffset = 0;

			//a pointer that points at either 0 or 1/2 of the buffer, depending on whether we need to be fillinf the first or second half
			int halfPointer = secondHalf ? _playBufferSize / 2 : 0;
			result = _secondaryBuffer->Lock(halfPointer, _playBufferSize / 2, (void**)&bufferPtr1, (DWORD*)&bufferSize1, (void**)&bufferPtr2, (DWORD*)&bufferSize2, 0);
			secondHalf = !secondHalf;

			memcpy(bufferPtr1, _currentPlayBuffer.get(), bufferSize1);
			if (bufferPtr2 != NULL)
			{
				memcpy(bufferPtr2, _currentPlayBuffer.get(), bufferSize2);
			}

			result = _secondaryBuffer->Unlock((void*)bufferPtr1, bufferSize1, (void*)bufferPtr2, bufferSize2);

			result = _secondaryBuffer->Play(0, 0, DSBPLAY_LOOPING);


			if (!_finalPart)
			{
				while (true)
				{
					if (_stopped)
					{
						break;
					}
					//set the timeout to 1000 to make sure that the above check will be made every second
					//that;s in case another thread stops the playback and this one is stuck waiting
					//only break if we get the first or second notifications (the ones at 1/4 and 3/4 respectively)
					int r = WaitForMultipleObjects(3, _playEventHandles, FALSE, 1000);
					if (r == 0 || r == 1)
					{
						break;
					}
				}
			}
			else
			{
				while (true)
				{
					if (_stopped)
					{
						break;
					}
					//only break if we hit the third notification (the one showing the end of the song)
					int r = WaitForMultipleObjects(3, _playEventHandles, FALSE, 1000);
					if (r == 2)
					{
						_primaryBuffer->Stop();
						_secondaryBuffer->Stop();
						break;
					}
				}
				break;
			}
		}

		//the loop has stopped for some reason, meaning we ar enot playing anymore
		_playing = false;
		return true;
	}

	/// <summary>
	/// Send a packet to the server asking for more data (2 more seconds)
	/// </summary>
	bool SendDataRequest()
	{
		std::unique_ptr<char> buffer(new char[DEFAULT_BUFLEN]);

		//set the packet type
		int packetType = CLIENT_TO_SERVER_GET_DATA;
		memcpy(buffer.get(), &packetType, sizeof(int));

		send(_connectSocket, buffer.get(), DEFAULT_BUFLEN, 0);

		return true;
	}

	/// <summary>
	/// Receive packets with raw data from the server, until we have 2 seconds worth of data
	/// </summary>
	bool CollectPlayBufferData()
	{
		//if the playback is stopped just exit
		if (_stopped)
		{
			return true;
		}

		//reset the play buffer
		_currentPlayBuffer = (std::shared_ptr<char>)(new char[_playBufferSize / 2]);

		while (_currentPlayBufferOffset < _playBufferSize / 2)
		{
			//again, if the playback has been stopped while we are in this loop, break out of it
			if (_stopped)
			{
				break;
			}

			//receive twice to get rid of the control packet
			std::shared_ptr<char> recvBuffer(new char[DEFAULT_BUFLEN]);
			int packetType = -1;
			while (packetType != SERVER_TO_CLIENT_DATA_CHUNK)
			{
				recvBuffer = Receive(DEFAULT_BUFLEN);
				if (_serverTerminated)
				{
					return false;
				}

				memcpy(&packetType, recvBuffer.get(), sizeof(int));
			}
			recvBuffer = Receive(DEFAULT_BUFLEN);
			if (_serverTerminated)
			{
				return false;
			}

			//copy the raw data in the play buffer
			memcpy(_currentPlayBuffer.get() + _currentPlayBufferOffset, recvBuffer.get(), DEFAULT_BUFLEN);

			_currentPlayBufferOffset += DEFAULT_BUFLEN;

			//totalPlayBufferOffset gets set to 0 only once - when we start playing a new song
			_totalPlayBufferOffset += DEFAULT_BUFLEN;

			//if we have more bytes than the size of the whole song, that means we are now playing the final 2-second section
			if (_totalPlayBufferOffset >= _songSize)
			{
				_finalPart = true;
				break;
			}
		}

		return true;
	}

	/// <summary>
	/// Works the same way as the receive function in the server class
	/// When running on different machines the packets sent between the two applications have varying sizes
	/// Therefore we need to collect packets until we can "build" a whole one (of size DEFAULT_BUFLEN)
	/// </summary>
	std::shared_ptr<char> Receive(int bufferSize)
	{
		//switch the type of packet
		_packetIsControl = !_packetIsControl;

		//intialise a buffer, that will be used in this function only (in the calls to recv())
		std::shared_ptr<char> buffer(new char[DEFAULT_BUFLEN]);
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
			int bytesReceived = recv(_connectSocket, buffer.get(), DEFAULT_BUFLEN, 0);
			if (bytesReceived == SOCKET_ERROR)
			{
				_serverTerminated = true;
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
				memcpy(realBuffer.get() + totalBytesReceived, buffer.get(), copySize);
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

	/// <summary>
	/// Tell the client to stop playing
	/// </summary>
	bool Stop()
	{
		_stopped = true;
		if (_secondaryBuffer != nullptr)
		{
			_secondaryBuffer->Stop();
			_totalPlayBufferOffset = 0;
		}

		return true;
	}

	/// <summary>
	/// Tell the client to pause play
	/// </summary>
	bool Pause()
	{
		_paused = true;
		_secondaryBuffer->Stop();

		return true;
	}

	/// <summary>
	/// Tell the client to resume playing
	/// </summary>
	bool Resume()
	{
		_paused = false;
		_secondaryBuffer->Play(0, 0, DSBPLAY_LOOPING);

		return true;
	}

	/// <summary>
	/// Return the progress of the song (in %)
	/// </summary>
	int GetProgress()
	{
		int progress;
		float ratio = (float)_totalPlayBufferOffset / (float)_songSize;
		progress = ratio * 100;

		return progress;
	}

	/// <summary>
	/// Set the volume of the DirectSound player
	/// </summary>
	bool SetVolume(int index)
	{
		float ratio = (float)index / 100.0f;
		_secondaryBuffer->SetVolume((DSBVOLUME_MAX - DSBVOLUME_MIN) * ratio + DSBVOLUME_MIN);

		return true;
	}

private:
	SOCKET							_connectSocket;
	IDirectSound8 *					_directSound = nullptr;
	IDirectSoundBuffer *			_primaryBuffer = nullptr;
	IDirectSoundBuffer8 *			_secondaryBuffer = nullptr;
	WAVEFORMATEX					_waveFormat;
	DSBUFFERDESC					_bufferDesc;

	std::shared_ptr<char>			_overflowBuffer;
	int								_overflowSize;

	DWORD							_songSize;

	std::shared_ptr<char>			_currentPlayBuffer;
	int								_currentPlayBufferOffset = 0;
	int								_totalPlayBufferOffset = 0;
	int								_playBufferSize;
	bool							_finalPart;

	//variables showing whether the playback is currently stopped or paused
	bool							_stopped;
	bool							_paused;
	bool							_playing;
	bool							_serverTerminated;

	//_packetIsControl tells us if the next packet coming from the server is going to be a control packet or not
	//control packets contain data describing the packet coming after them
	bool							_packetIsControl;

	HANDLE							_playEventHandles[3];
	LPDIRECTSOUNDNOTIFY8			_directSoundNotify;
	DSBPOSITIONNOTIFY				_positionNotify[3];

};

WaveyfyPlayer::WaveyfyPlayer()
{
	_playerPImpl->Initialise();
}

/// <summary>
/// Set up a connection with the server
/// </summary>
bool WaveyfyPlayer::SetUpConnection()
{
	return _playerPImpl->SetUpConnection();
}

/// <summary>
/// Initialise an empty wave file
/// </summary>
/// <param name="disposing"></param>
bool WaveyfyPlayer::WaveFileInitialise()
{
	return _playerPImpl->WaveFileInitialise();
}

/// <summary>
/// Send a packet to the server, requesting a list with the songs
/// </summary>
/// <param name="disposing"></param>
bool WaveyfyPlayer::RequestFileList()
{
	return _playerPImpl->RequestFileList();
}

/// <summary>
/// Receive a all the file names from the server
/// </summary>
bool WaveyfyPlayer::ReceiveFileList(char* buff, int len)
{
	return _playerPImpl->ReceiveFileList(buff, len);
}

/// <summary>
/// Send a packet to the server, with the number of the chosen song
/// </summary>
bool WaveyfyPlayer::SendSongChoice(int index)
{
	return _playerPImpl->SendSongChoice(index);
}

/// <summary>
/// Receive info about the wave file from the server
/// </summary>
bool WaveyfyPlayer::ReceiveFileInfo()
{
	return _playerPImpl->ReceiveFileInfo();
}

/// <summary>
/// Get the song size from the server
/// </summary>
bool WaveyfyPlayer::ReceiveSongSize()
{
	return _playerPImpl->ReceiveSongSize();
}

bool WaveyfyPlayer::SetDirectSoundNotifications()
{
	return _playerPImpl->SetDirectSoundNotifications();
}

/// <summary>
/// Handle the streaming of raw data from the server
/// on every step of the loop, the client asks the server for the next 2 sedonds worth of data
/// after that the client receives packets until it has 2 seconds worth of raw data
/// copies everything in the secondary buffer and plays it
/// </summary>
bool WaveyfyPlayer::Stream()
{
	return _playerPImpl->Stream();
}

/// <summary>
/// Send a packet to the server asking for more data (2 more seconds)
/// </summary>
bool WaveyfyPlayer::SendDataRequest()
{
	return _playerPImpl->SendDataRequest();
}

/// <summary>
/// Receive packets with raw data from the server, until we have 2 seconds worth of data
/// </summary>
bool WaveyfyPlayer::CollectPlayBufferData()
{
	return _playerPImpl->CollectPlayBufferData();
}

/// <summary>
/// Works the same way as the receive function in the server class
/// When running on different machines the packets sent between the two applications have varying sizes
/// Therefore we need to collect packets until we can "build" a whole one (of size DEFAULT_BUFLEN)
/// </summary>
std::shared_ptr<char> WaveyfyPlayer::Receive(int bufferSize)
{	
	return _playerPImpl->Receive(bufferSize);
}

/// <summary>
/// Tell the client to stop playing
/// </summary>
bool WaveyfyPlayer::Stop()
{
	return _playerPImpl->Stop();
}

/// <summary>
/// Tell the client to pause play
/// </summary>
bool WaveyfyPlayer::Pause()
{
	return _playerPImpl->Pause();
}

/// <summary>
/// Tell the client to resume playing
/// </summary>
bool WaveyfyPlayer::Resume()
{
	return _playerPImpl->Resume();
}

/// <summary>
/// Return the progress of the song (in %)
/// </summary>
int WaveyfyPlayer::GetProgress()
{
	return _playerPImpl->GetProgress();
}

/// <summary>
/// Set the volume of the DirectSound player
/// </summary>
bool WaveyfyPlayer::SetVolume(int index)
{
	return _playerPImpl->SetVolume(index);
}

WaveyfyPlayer::~WaveyfyPlayer()
{
}
