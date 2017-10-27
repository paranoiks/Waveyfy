#pragma once

#include <windows.h>
#include <mmsystem.h>
#include <mmreg.h>
#include <dsound.h>
#include <iostream>

#define fourccRIFF MAKEFOURCC('R', 'I', 'F', 'F')
#define fourccDATA MAKEFOURCC('d', 'a', 't', 'a')
#define fourccFMT  MAKEFOURCC('f', 'm', 't', ' ')
#define fourccWAVE MAKEFOURCC('W', 'A', 'V', 'E')
#define fourccXWMA MAKEFOURCC('X', 'W', 'M', 'A')


class AudioController
{
public:
	AudioController();
	bool LoadWaveFile(TCHAR* filename, HANDLE& fileHandle, WAVEFORMATEX& waveFormat, std::shared_ptr<char> waveFormatBuffer, DWORD& dataChunkSize, DWORD& dataChunkPosition, std::shared_ptr<BYTE>& dataBuffer);
	HRESULT ReadChunkData(HANDLE fileHandle, void * buffer, DWORD buffersize, DWORD bufferoffset);
	HRESULT FindChunk(HANDLE fileHandle, FOURCC fourcc, DWORD & chunkSize, DWORD & chunkDataPosition);
	~AudioController();

private:	
	DWORD					dataBufferSize;
};

