// A basic example of playing a wave file using the DirectSound APIs
//
// This example does not stream audio.  It simply loads all of the 
// audio data from file and loads it into the DirectSound secondary
// buffer for playing.  However, various hints are given in the comments
// about how to handle streaming.
//
// There are also routines in here that show how to load and handle .WAV
// files.
//
// Based on the example at http://www.rastertek.com/dx10tut14.html. Code
// added to support running as a console application and the file handling
// code updated to handle any RIFF file.
//
// No attempt has been made in this example to use any modern C++ features
// other than nullptr.

#include <windows.h>
#include <mmsystem.h>
#include <mmreg.h>
#include <dsound.h>
#include <iostream>

using namespace std;

#pragma comment(lib, "dsound.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "winmm.lib")

// Change this line to play a different file.  Modify this to a suitable location.
TCHAR * audioFilename = TEXT("E:\\Programming\\University\\Waveyfy\\DirectSoundExample\\long.wav");

#define fourccRIFF MAKEFOURCC('R', 'I', 'F', 'F')
#define fourccDATA MAKEFOURCC('d', 'a', 't', 'a')
#define fourccFMT  MAKEFOURCC('f', 'm', 't', ' ')
#define fourccWAVE MAKEFOURCC('W', 'A', 'V', 'E')
#define fourccXWMA MAKEFOURCC('X', 'W', 'M', 'A')

IDirectSound8 *			directSound = nullptr;
IDirectSoundBuffer *	primaryBuffer = nullptr;
IDirectSoundBuffer8 *	secondaryBuffer = nullptr;
BYTE *					dataBuffer = nullptr;
DWORD					dataBufferSize;
DWORD				    averageBytesPerSecond;

// Search the file for the chunk we want  
// Returns the size of the chunk and its location in the file

HRESULT FindChunk(HANDLE fileHandle, FOURCC fourcc, DWORD & chunkSize, DWORD & chunkDataPosition)
{
	HRESULT hr = S_OK;
	DWORD chunkType;
	DWORD chunkDataSize;
	DWORD riffDataSize = 0;
	DWORD fileType;
	DWORD bytesRead = 0;
	DWORD offset = 0;

	if (SetFilePointer(fileHandle, 0, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
	{
		return HRESULT_FROM_WIN32(GetLastError());
	}
	while (hr == S_OK)
	{
		if (ReadFile(fileHandle, &chunkType, sizeof(DWORD), &bytesRead, NULL) == 0)
		{
			hr = HRESULT_FROM_WIN32(GetLastError());
		}
		if (ReadFile(fileHandle, &chunkDataSize, sizeof(DWORD), &bytesRead, NULL) == 0)
		{
			hr = HRESULT_FROM_WIN32(GetLastError());
		}
		switch (chunkType)
		{
		case fourccRIFF:
			riffDataSize = chunkDataSize;
			chunkDataSize = 4;
			if (ReadFile(fileHandle, &fileType, sizeof(DWORD), &bytesRead, NULL) == 0)
			{
				hr = HRESULT_FROM_WIN32(GetLastError());
			}
			break;

		default:
			if (SetFilePointer(fileHandle, chunkDataSize, NULL, FILE_CURRENT) == INVALID_SET_FILE_POINTER)
			{
				return HRESULT_FROM_WIN32(GetLastError());
			}
		}

		offset += sizeof(DWORD) * 2;
		if (chunkType == fourcc)
		{
			chunkSize = chunkDataSize;
			chunkDataPosition = offset;
			return S_OK;
		}

		offset += chunkDataSize;
		if (bytesRead >= riffDataSize)
		{
			return S_FALSE;
		}
	}
	return S_OK;
}

// Read a chunk of data of the specified size from the file at the specifed location into the supplied buffer

HRESULT ReadChunkData(HANDLE fileHandle, void * buffer, DWORD buffersize, DWORD bufferoffset)
{
	HRESULT hr = S_OK;
	DWORD bytesRead;

	if (SetFilePointer(fileHandle, bufferoffset, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
	{
		return HRESULT_FROM_WIN32(GetLastError());
	}
	if (ReadFile(fileHandle, buffer, buffersize, &bytesRead, NULL) == 0)
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
	}
	return hr;
}

bool Initialise()
{
	HRESULT result;
	DSBUFFERDESC bufferDesc;
	WAVEFORMATEX waveFormat;

	// Initialize the direct sound interface pointer for the default sound device.
	result = DirectSoundCreate8(NULL, &directSound, NULL);
	if (FAILED(result))
	{
		return false;
	}

	// Set the cooperative level to priority so the format of the primary sound buffer can be modified.
	// We use the handle of the desktop window since we are a console application.  If you do write a 
	// graphical application, you should use the HWnd of the graphical application. 
	result = directSound->SetCooperativeLevel(GetDesktopWindow(), DSSCL_PRIORITY);
	if (FAILED(result))
	{
		return false;
	}

	// Setup the primary buffer description.
	bufferDesc.dwSize = sizeof(DSBUFFERDESC);
	bufferDesc.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLVOLUME;
	bufferDesc.dwBufferBytes = 0;
	bufferDesc.dwReserved = 0;
	bufferDesc.lpwfxFormat = NULL;
	bufferDesc.guid3DAlgorithm = GUID_NULL;

	// Get control of the primary sound buffer on the default sound device.
	result = directSound->CreateSoundBuffer(&bufferDesc, &primaryBuffer, NULL);
	if (FAILED(result))
	{
		return false;
	}

	// Setup the format of the primary sound bufffer.
	// In this case it is a .WAV file recorded at 44,100 samples per second in 16-bit stereo (cd audio format).
	// Really, we should set this up from the wave file format loaded from the file.
	waveFormat.wFormatTag = WAVE_FORMAT_PCM;
	waveFormat.nSamplesPerSec = 44100;
	waveFormat.wBitsPerSample = 16;
	waveFormat.nChannels = 2;
	waveFormat.nBlockAlign = (waveFormat.wBitsPerSample / 8) * waveFormat.nChannels;
	waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
	waveFormat.cbSize = 0;

	// Set the primary buffer to be the wave format specified.
	result = primaryBuffer->SetFormat(&waveFormat);
	if (FAILED(result))
	{
		return false;
	}
	return true;
}

void Shutdown()
{
	// Destroy the data buffer
	if (dataBuffer != nullptr)
	{
		delete[] dataBuffer;
		dataBuffer = nullptr;
	}

	// Release the primary sound buffer pointer.
	if (primaryBuffer != nullptr)
	{
		primaryBuffer->Release();
		primaryBuffer = nullptr;
	}

	// Release the direct sound interface pointer.
	if (directSound != nullptr)
	{
		directSound->Release();
		directSound = nullptr;
	}
}

// Load the wave file into memory and setup the secondary buffer.

bool LoadWaveFile(TCHAR * filename)
{
	WAVEFORMATEXTENSIBLE wfx = { 0 };
	WAVEFORMATEX waveFormat;
	DSBUFFERDESC bufferDesc;
	HRESULT result;
	IDirectSoundBuffer * tempBuffer;

	DWORD chunkSize;
	DWORD chunkPosition;
	DWORD filetype;
	HRESULT hr = S_OK;

	// Open the wave file
	HANDLE fileHandle = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (fileHandle == INVALID_HANDLE_VALUE)
	{
		return false;
	}
	if (SetFilePointer(fileHandle, 0, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
	{
		return false;
	}
	// Make sure we have a RIFF wave file
	FindChunk(fileHandle, fourccRIFF, chunkSize, chunkPosition);
	ReadChunkData(fileHandle, &filetype, sizeof(DWORD), chunkPosition);
	if (filetype != fourccWAVE)
	{
		return false;
	}
	// Locate the 'fmt ' chunk, and copy its contents into a WAVEFORMATEXTENSIBLE structure. 
	FindChunk(fileHandle, fourccFMT, chunkSize, chunkPosition);
	ReadChunkData(fileHandle, &wfx, chunkSize, chunkPosition);
	// Find the audio data chunk
	FindChunk(fileHandle, fourccDATA, chunkSize, chunkPosition);
	dataBufferSize = chunkSize;
	// Read the audio data from the 'data' chunk.  This is the data that needs to be copied into
	// the secondary buffer for playing
	dataBuffer = new BYTE[dataBufferSize];
	ReadChunkData(fileHandle, dataBuffer, dataBufferSize, chunkPosition);
	CloseHandle(fileHandle);

	// Set the wave format of the secondary buffer that this wave file will be loaded onto.
	// The value of wfx.Format.nAvgBytesPerSec will be very useful to you since it gives you
	// an approximate value for how many bytes it takes to hold one second of audio data.
	waveFormat.wFormatTag =  wfx.Format.wFormatTag;
	waveFormat.nSamplesPerSec = wfx.Format.nSamplesPerSec;
	waveFormat.wBitsPerSample = wfx.Format.wBitsPerSample;
	waveFormat.nChannels = wfx.Format.nChannels;
	waveFormat.nBlockAlign = wfx.Format.nBlockAlign;
	waveFormat.nAvgBytesPerSec = wfx.Format.nAvgBytesPerSec;
	waveFormat.cbSize = 0;

	// Set the buffer description of the secondary sound buffer that the wave file will be loaded onto. In
	// this example, we setup a buffer the same size as that of the audio data.  For the assignment, your
	// secondary buffer should only be large enough to hold approximately four seconds of data. 
	bufferDesc.dwSize = sizeof(DSBUFFERDESC);
	bufferDesc.dwFlags = DSBCAPS_CTRLVOLUME | DSBCAPS_GLOBALFOCUS | DSBCAPS_CTRLPOSITIONNOTIFY;
	bufferDesc.dwBufferBytes = dataBufferSize;
	bufferDesc.dwReserved = 0;
	bufferDesc.lpwfxFormat = &waveFormat;
	bufferDesc.guid3DAlgorithm = GUID_NULL;

	// Create a temporary sound buffer with the specific buffer settings.
	result = directSound->CreateSoundBuffer(&bufferDesc, &tempBuffer, NULL);
	if (FAILED(result))
	{
		return false;
	}

	// Test the buffer format against the direct sound 8 interface and create the secondary buffer.
	result = tempBuffer->QueryInterface(IID_IDirectSoundBuffer8, (void**)&secondaryBuffer);
	if (FAILED(result))
	{
		return false;
	}

	// Release the temporary buffer.
	tempBuffer->Release();
	tempBuffer = nullptr;

	return true;
}

void ReleaseSecondaryBuffer()
{
	// Release the secondary sound buffer.
	if (secondaryBuffer != nullptr)
	{
		(secondaryBuffer)->Release();
		secondaryBuffer = nullptr;
	}
}

bool PlayWaveFile()
{
	HRESULT result;
	unsigned char * bufferPtr1;
	unsigned long   bufferSize1;
	unsigned char * bufferPtr2;
	unsigned long   bufferSize2;
	BYTE * dataBufferPtr = dataBuffer;
	DWORD soundBytesOutput = 0;
	bool fillFirstHalf = true;
	LPDIRECTSOUNDNOTIFY8 directSoundNotify;
	DSBPOSITIONNOTIFY positionNotify[2];

	// Set position of playback at the beginning of the sound buffer.
	result = secondaryBuffer->SetCurrentPosition(0);
	if (FAILED(result))
	{
		return false;
	}

	// Set volume of the buffer to 100%.
	result = secondaryBuffer->SetVolume(DSBVOLUME_MAX);
	if (FAILED(result))
	{
		return false;
	}

	// Create an event for notification that playing has stopped.  This is only useful
	// when your audio file fits in the entire secondary buffer (as in this example).  
	// For the assignment, you are going to need notifications when the playback has reached the 
	// first quarter of the buffer or the third quarter of the buffer so that you know when 
	// you should copy more data into the secondary buffer. 
	HANDLE playEventHandles[1];
	playEventHandles[0] = CreateEvent(NULL, FALSE, FALSE, NULL);

	result = secondaryBuffer->QueryInterface(IID_IDirectSoundNotify8, (LPVOID*)&directSoundNotify);
	if (FAILED(result))
	{
		return false;
	}
	// This notification is used to indicate that we have finished playing the buffer of audio. In
	// the assignment, you will need two different notifications as mentioned above. 
	positionNotify[0].dwOffset = DSBPN_OFFSETSTOP;
	positionNotify[0].hEventNotify = playEventHandles[0];
	directSoundNotify->SetNotificationPositions(1, positionNotify);
	directSoundNotify->Release();

	// Now we can fill our secondary buffer and play it.  In the assignment, you will not be able to fill
	// the buffer all at once since the secondary buffer will not be large enough.  Instead, you will need to
	// loop through the data that you have retrieved from the server, filling different sections of the 
	// secondary buffer as you receive notifications.
    
	// Lock the first part of the secondary buffer to write wave data into it. In this case, we lock the entire
	// buffer, but for the assignment, you will only want to lock the half of the buffer that is not being played.
	// You will definately want to look up the methods for the IDIRECTSOUNDBUFFER8 interface to see what these
	// methods do and what the parameters are used for. 
	result = secondaryBuffer->Lock(0, dataBufferSize, (void**)&bufferPtr1, (DWORD*)&bufferSize1, (void**)&bufferPtr2, (DWORD*)&bufferSize2, 0);
	if (FAILED(result))
	{
		return false;
	}
	// Copy the wave data into the buffer. If you need to insert some silence into the buffer, insert values of 0.
	memcpy(bufferPtr1, dataBuffer, bufferSize1);
	if (bufferPtr2 != NULL)
	{
		memcpy(bufferPtr2, dataBuffer, bufferSize2);
	}
	// Unlock the secondary buffer after the data has been written to it.
	result = secondaryBuffer->Unlock((void*)bufferPtr1, bufferSize1, (void*)bufferPtr2, bufferSize2);
	if (FAILED(result))
	{
		return false;
	}
	// Play the contents of the secondary sound buffer. If you want play to go back to the start of the buffer
	// again, set the last parameter to DSBPLAY_LOOPING instead of 0.  If play is already in progress, then 
	// play will just continue. 
	result = secondaryBuffer->Play(0, 0, 0);
	if (FAILED(result))
	{
		return false;
	}
	// Wait for notifications.  In this case, we only have one notification so we could use WaitForSingleObject,
	// but for the assignment you will need more than one notification, so you will need WaitForMultipleObjects
	result = WaitForMultipleObjects(1, playEventHandles, FALSE, INFINITE);
	// In this case, we have been notified that playback has finished so we can just finish. In the assignment,
	// you should use the appropriate notification to determine which part of the secondary buffer needs to be
	// filled and handle it accordingly.
	CloseHandle(playEventHandles[0]);
	return true;
}

int main()
{
	bool result;

	// Initialize direct sound and the primary sound buffer.
	result = Initialise();
	if (!result)
	{
		cout << "Unable to initialise DirectSound" << endl;
		Shutdown();
		return -1;
	}

	// Load a wave audio file onto a secondary buffer.
	result = LoadWaveFile(audioFilename);
	if (!result)
	{
		cout << "Unable to load wave file" << endl;
		Shutdown();
		return -1;
	}

	// Play the wave file now that it has been loaded.
	result = PlayWaveFile();
	if (!result)
	{
		cout << "Unable to play file" << endl;
		ReleaseSecondaryBuffer();
		Shutdown();
		return -1;
	}

	ReleaseSecondaryBuffer();
	Shutdown();
	return 0;
}