#include "stdafx.h"
#include "WaveyfyPlayer.h"

extern "C" WAVEYFYPLAYERDLL_API WaveyfyPlayer* CreateWaveyfyPlayer()
{
	return new WaveyfyPlayer();
}
extern "C" WAVEYFYPLAYERDLL_API bool SetUpConnection(WaveyfyPlayer* playerObject)
{
	return playerObject->SetUpConnection();
}
extern "C" WAVEYFYPLAYERDLL_API bool WaveFileInitialise(WaveyfyPlayer* playerObject)
{
	return playerObject->WaveFileInitialise();
}
extern "C" WAVEYFYPLAYERDLL_API bool RequestFileList(WaveyfyPlayer* playerObject)
{
	return playerObject->RequestFileList();
}
extern "C" WAVEYFYPLAYERDLL_API bool ReceiveFileList(WaveyfyPlayer* playerObject, char* buff, int len)
{
	return playerObject->ReceiveFileList(buff, len);
}
extern "C" WAVEYFYPLAYERDLL_API bool SendSongChoice(WaveyfyPlayer* playerObject, int index)
{
	return playerObject->SendSongChoice(index);
}
extern "C" WAVEYFYPLAYERDLL_API bool ReceiveFileInfo(WaveyfyPlayer* playerObject)
{
	return playerObject->ReceiveFileInfo();
}
extern "C" WAVEYFYPLAYERDLL_API bool ReceiveSongSize(WaveyfyPlayer* playerObject)
{
	return playerObject->ReceiveSongSize();
}
extern "C" WAVEYFYPLAYERDLL_API bool SetDirectSoundNotifications(WaveyfyPlayer* playerObject)
{
	return playerObject->SetDirectSoundNotifications();
}
extern "C" WAVEYFYPLAYERDLL_API bool Stream(WaveyfyPlayer* playerObject)
{
	return playerObject->Stream();
}
extern "C" WAVEYFYPLAYERDLL_API bool Stop(WaveyfyPlayer* playerObject)
{
	return playerObject->Stop();
}
extern "C" WAVEYFYPLAYERDLL_API bool Pause(WaveyfyPlayer* playerObject)
{
	return playerObject->Pause();
}
extern "C" WAVEYFYPLAYERDLL_API bool Resume(WaveyfyPlayer* playerObject)
{
	return playerObject->Resume();
}
extern "C" WAVEYFYPLAYERDLL_API int GetProgress(WaveyfyPlayer* playerObject)
{
	return playerObject->GetProgress();
}
extern "C" WAVEYFYPLAYERDLL_API bool SetVolume(WaveyfyPlayer* playerObject, int volume)
{
	return playerObject->SetVolume(volume);
}