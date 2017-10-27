using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace WaveyfyFormsApp
{
    public partial class Form1 : Form
    {
        private WaveyfyPlayerApp wpa;
        private Thread t;

        private bool playing = false;

        public Form1()
        {
            InitializeComponent();
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            StartDLL();           

            //start a timer that will be used to update the progress bar every 100 miliseconds
            System.Windows.Forms.Timer progressTimer = new System.Windows.Forms.Timer();
            progressTimer.Tick += MyTimer_Tick;
            progressTimer.Interval = 100;
            progressTimer.Start();

            //set an event to be fired when the form is closed
            this.FormClosing += Form1_FormClosing;

            //set an event to be fired when the volume slider is moved
            volumeSlider.ValueChanged += VolumeSlider_ValueChanged;   
        }

        /// <summary>
        /// Create a new instance of the player and set up the song list
        /// </summary>
        private void StartDLL()
        {
            wpa = new WaveyfyPlayerApp();

            //request the file list from the server
            wpa.RequestFileList();

            //receive the file list in the designated buffer
            //all files will be separated by an *
            StringBuilder str = new StringBuilder(3000);
            wpa.ReceiveFileList(str, 3000);

            //split the received string on the * symbol
            string[] fileNames = str.ToString().Split('*');
            for (int i = 0; i < fileNames.Count() - 1; i++)
            {
                int lastBackslash = fileNames[i].LastIndexOf('\\') + 1;
                int lastDot = fileNames[i].LastIndexOf('.');
                int length = lastDot - lastBackslash;

                string name = fileNames[i].Substring(lastBackslash, length);

                //add each string in the list box
                Invoke((MethodInvoker)(() => listBox1.Items.Add(name)));
            }
        }

        /// <summary>
        /// When a new song has been chosen, stop the playback, join the current thread and then create a new one
        /// </summary>
        /// <param name="index"></param>
        private void SongChosen(int index)
        {
            if (t != null)
            {
                wpa.Stop();
                t.Join();
            }
            t = new Thread(new ThreadStart(() => SongChosenNewThread(index)));
            t.Start();            
        }

        /// <summary>
        /// Call all the methods neede to get the playback started
        /// </summary>
        /// <param name="index"></param>
        private void SongChosenNewThread(int index)
        {
            playing = false;
            bool result;

            //send our song choice to the server
            result = wpa.SendSongChoice(index);
            if(!result)
            {
                return;
            }

            //receive the file info about the song
            result = wpa.ReceiveFileInfo();
            if(!result)
            {
                return;
            }

            //receive the size of the song in bytes
            result = wpa.ReceiveSongSize();
            if(!result)
            {
                return;
            }

            //set the direct sound notifications for the song
            result = wpa.SetDirectSoundNotifications();
            if(!result)
            {
                return;
            }

            //set the volume of the playback to match the slider (this has to be ran on a differenent thread => Invoke())
            Invoke((MethodInvoker)(() => wpa.SetVolume(volumeSlider.Value)));
            playing = true;

            //start the streaming of data
            result = wpa.Stream();
            if(!result)
            {
                return;
            }
        }

        /// <summary>
        /// When the volume slider has been moved, change the volume level of the playback
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void VolumeSlider_ValueChanged(object sender, EventArgs e)
        {
            Console.WriteLine("VOLUME " + volumeSlider.Value);
            wpa.SetVolume(volumeSlider.Value);
        }

        /// <summary>
        /// When the form starts closing, stop the playback and join the thread
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void Form1_FormClosing(object sender, FormClosingEventArgs e)
        {
            if (t != null)
            {
                wpa.Stop();
                t.Join();
            }
        }

        /// <summary>
        /// Update the progress bar every 100 miliseconds
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void MyTimer_Tick(object sender, EventArgs e)
        {
            if (!playing)
            {
                return;
            }
            int progress = wpa.GetProgress();
            songProgress.Value = progress;
        }

        //when the selected index of the list box has been changed, we want to start playing the new song
        private void listBox1_SelectedIndexChanged(object sender, EventArgs e)
        {
            SongChosen(listBox1.SelectedIndex);
        }

        //when the stop button has been clicked, pause the playback
        private void buttonStop_Click(object sender, EventArgs e)
        {            
            wpa.Pause();
        }

        //when the pause button has been clicked, pause the playback
        private void buttonPause_Click(object sender, EventArgs e)
        {
            wpa.Pause();
        }

        //when the resume button has been clicked, resume the playback
        private void buttonPLay_Click(object sender, EventArgs e)
        {
            wpa.Resume();
        }
    }
}
