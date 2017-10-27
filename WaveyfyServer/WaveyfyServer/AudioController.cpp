#include "stdafx.h"
#include<sstream>

AudioController::AudioController()
{
}

/// <summary>
/// Load the wave file and extract some data from it
/// Put the Wave Format data in the waveFormatBuffer
/// Put the song size in bytes in the dataChunkSize
/// Put the raw data for the song in the dataBuffer
/// </summary>
bool AudioController::LoadWaveFile(TCHAR* filename, HANDLE& fileHandle, WAVEFORMATEX& waveFormat, std::shared_ptr<char> waveFormatBuffer, DWORD& dataChunkSize, DWORD& dataChunkPosition, std::shared_ptr<BYTE>& dataBuffer)
{
	WAVEFORMATEXTENSIBLE wfx = { 0 };
	DSBUFFERDESC bufferDesc;
	HRESULT result;
	IDirectSoundBuffer * tempBuffer;

	DWORD chunkSize;
	DWORD chunkPosition;
	DWORD filetype;
	HRESULT hr = S_OK;

	// Open the wave file
	fileHandle = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
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
	// get the chunk size and position
	dataChunkSize = chunkSize;
	dataChunkPosition = chunkPosition;
	//get the data itself
	dataBuffer = (std::shared_ptr<BYTE>)(new BYTE[chunkSize]);
	ReadChunkData(fileHandle, dataBuffer.get(), chunkSize, chunkPosition);

	CloseHandle(fileHandle);

	// Set the wave format of the secondary buffer that this wave file will be loaded onto.
	// The value of wfx.Format.nAvgBytesPerSec will be very useful to you since it gives you
	// an approximate value for how many bytes it takes to hold one second of audio data.
	waveFormat.wFormatTag = wfx.Format.wFormatTag;
	waveFormat.nSamplesPerSec = wfx.Format.nSamplesPerSec;
	waveFormat.wBitsPerSample = wfx.Format.wBitsPerSample;
	waveFormat.nChannels = wfx.Format.nChannels;
	waveFormat.nBlockAlign = wfx.Format.nBlockAlign;
	waveFormat.nAvgBytesPerSec = wfx.Format.nAvgBytesPerSec;
	waveFormat.cbSize = 0;
	
	//once we have the Wave Format data in the waveFormat variable
	//we copy it over to the buffer provided
	int pointerIndex = 0;
	memcpy(waveFormatBuffer.get() + pointerIndex, &waveFormat.wFormatTag, sizeof(waveFormat.wFormatTag));
	pointerIndex += sizeof(waveFormat.wFormatTag);
	memcpy(waveFormatBuffer.get() + pointerIndex, &waveFormat.nSamplesPerSec, sizeof(waveFormat.nSamplesPerSec));
	pointerIndex += sizeof(waveFormat.nSamplesPerSec);
	memcpy(waveFormatBuffer.get() + pointerIndex, &waveFormat.wBitsPerSample, sizeof(waveFormat.wBitsPerSample));
	pointerIndex += sizeof(waveFormat.wBitsPerSample);
	memcpy(waveFormatBuffer.get() + pointerIndex, &waveFormat.nChannels, sizeof(waveFormat.nChannels));
	pointerIndex += sizeof(waveFormat.nChannels);
	memcpy(waveFormatBuffer.get() + pointerIndex, &waveFormat.nBlockAlign, sizeof(waveFormat.nBlockAlign));
	pointerIndex += sizeof(waveFormat.nBlockAlign);
	memcpy(waveFormatBuffer.get() + pointerIndex, &waveFormat.nAvgBytesPerSec, sizeof(waveFormat.nAvgBytesPerSec));
	pointerIndex += sizeof(waveFormat.nAvgBytesPerSec);
	memcpy(waveFormatBuffer.get() + pointerIndex, &waveFormat.cbSize, sizeof(waveFormat.cbSize));
	pointerIndex += sizeof(waveFormat.cbSize);

	return true;
}

/// <summary>
/// Read a chunk of data
/// </summary>
HRESULT AudioController::ReadChunkData(HANDLE fileHandle, void * buffer, DWORD buffersize, DWORD bufferoffset)
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

/// <summary>
/// Find a chunk of a given type
/// </summary>
HRESULT AudioController::FindChunk(HANDLE fileHandle, FOURCC fourcc, DWORD & chunkSize, DWORD & chunkDataPosition)
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

AudioController::~AudioController()
{
}
