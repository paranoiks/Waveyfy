using System;
using System.Runtime.InteropServices;
using System.Text;

namespace WaveyfyFormsApp
{
    class WaveyfyPlayerApp
    {
        [DllImport("WaveyfyClient.DLL", CallingConvention = CallingConvention.Cdecl)]
        static private extern IntPtr CreateWaveyfyPlayer();

        [DllImport("WaveyfyClient.DLL", CallingConvention = CallingConvention.Cdecl)]
        static private extern bool SetUpConnection(IntPtr playerObject);

        [DllImport("WaveyfyClient.DLL", CallingConvention = CallingConvention.Cdecl)]
        static private extern bool WaveFileInitialise(IntPtr playerObject);

        [DllImport("WaveyfyClient.DLL", CallingConvention = CallingConvention.Cdecl)]
        static private extern bool RequestFileList(IntPtr playerObject);

        [DllImport("WaveyfyClient.DLL", CallingConvention = CallingConvention.Cdecl)]
        static private extern bool ReceiveFileList(IntPtr playerObject, StringBuilder buff, int len);

        [DllImport("WaveyfyClient.DLL", CallingConvention = CallingConvention.Cdecl)]
        static private extern bool SendSongChoice(IntPtr playerObject, int index);

        [DllImport("WaveyfyClient.DLL", CallingConvention = CallingConvention.Cdecl)]
        static private extern bool ReceiveFileInfo(IntPtr playerObject);

        [DllImport("WaveyfyClient.DLL", CallingConvention = CallingConvention.Cdecl)]
        static private extern bool ReceiveSongSize(IntPtr playerObject);

        [DllImport("WaveyfyClient.DLL", CallingConvention = CallingConvention.Cdecl)]
        static private extern bool SetDirectSoundNotifications(IntPtr playerObject);

        [DllImport("WaveyfyClient.DLL", CallingConvention = CallingConvention.Cdecl)]
        static private extern bool Stream(IntPtr playerObject);

        [DllImport("WaveyfyClient.DLL", CallingConvention = CallingConvention.Cdecl)]
        static private extern bool Stop(IntPtr playerObject);

        [DllImport("WaveyfyClient.DLL", CallingConvention = CallingConvention.Cdecl)]
        static private extern bool Pause(IntPtr playerObject);

        [DllImport("WaveyfyClient.DLL", CallingConvention = CallingConvention.Cdecl)]
        static private extern bool Resume(IntPtr playerObject);

        [DllImport("WaveyfyClient.DLL", CallingConvention = CallingConvention.Cdecl)]
        static private extern int GetProgress(IntPtr playerObject);

        [DllImport("WaveyfyClient.DLL", CallingConvention = CallingConvention.Cdecl)]
        static private extern bool SetVolume(IntPtr playerObject, int volume);

        private IntPtr _waveyfyPlayerObject = IntPtr.Zero;

        public WaveyfyPlayerApp()
        {
            _waveyfyPlayerObject = CreateWaveyfyPlayer();
        }

        public bool SetUpConnection()
        {
            return SetUpConnection(_waveyfyPlayerObject);
        }

        public bool WaveFileInitialise()
        {
            return WaveFileInitialise(_waveyfyPlayerObject);
        }

        public bool RequestFileList()
        {
            return RequestFileList(_waveyfyPlayerObject);
        }

        public bool ReceiveFileList(StringBuilder buff, int len)
        {
            return ReceiveFileList(_waveyfyPlayerObject, buff, len);
        }

        public bool SendSongChoice(int index)
        {
            return SendSongChoice(_waveyfyPlayerObject, index);
        }

        public bool ReceiveFileInfo()
        {
            return ReceiveFileInfo(_waveyfyPlayerObject);
        }

        public bool ReceiveSongSize()
        {
            return ReceiveSongSize(_waveyfyPlayerObject);
        }

        public bool SetDirectSoundNotifications()
        {
            return SetDirectSoundNotifications(_waveyfyPlayerObject);
        }

        public bool Stream()
        {
            return Stream(_waveyfyPlayerObject);
        }

        public bool Stop()
        {
            return Stop(_waveyfyPlayerObject);
        }

        public bool Pause()
        {
            return Pause(_waveyfyPlayerObject);
        }

        public bool Resume()
        {
            return Resume(_waveyfyPlayerObject);
        }

        public int GetProgress()
        {
            return GetProgress(_waveyfyPlayerObject);
        }

        public bool SetVolume(int index)
        {
            return SetVolume(_waveyfyPlayerObject, index);
        }

    }
}
