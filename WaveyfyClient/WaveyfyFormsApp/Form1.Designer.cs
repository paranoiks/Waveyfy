namespace WaveyfyFormsApp
{
    partial class Form1
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.listBox1 = new System.Windows.Forms.ListBox();
            this.checkedListBox1 = new System.Windows.Forms.CheckedListBox();
            this.buttonPLay = new System.Windows.Forms.Button();
            this.buttonPause = new System.Windows.Forms.Button();
            this.songProgress = new System.Windows.Forms.ProgressBar();
            this.volumeSlider = new System.Windows.Forms.TrackBar();
            ((System.ComponentModel.ISupportInitialize)(this.volumeSlider)).BeginInit();
            this.SuspendLayout();
            // 
            // listBox1
            // 
            this.listBox1.FormattingEnabled = true;
            this.listBox1.Location = new System.Drawing.Point(0, 0);
            this.listBox1.Name = "listBox1";
            this.listBox1.Size = new System.Drawing.Size(120, 95);
            this.listBox1.TabIndex = 0;
            this.listBox1.SelectedIndexChanged += new System.EventHandler(this.listBox1_SelectedIndexChanged);
            // 
            // checkedListBox1
            // 
            this.checkedListBox1.FormattingEnabled = true;
            this.checkedListBox1.Location = new System.Drawing.Point(83, 66);
            this.checkedListBox1.Name = "checkedListBox1";
            this.checkedListBox1.Size = new System.Drawing.Size(8, 4);
            this.checkedListBox1.TabIndex = 1;
            // 
            // buttonPLay
            // 
            this.buttonPLay.Location = new System.Drawing.Point(144, 191);
            this.buttonPLay.Name = "buttonPLay";
            this.buttonPLay.Size = new System.Drawing.Size(75, 23);
            this.buttonPLay.TabIndex = 2;
            this.buttonPLay.Text = "PLAY";
            this.buttonPLay.UseVisualStyleBackColor = true;
            this.buttonPLay.Click += new System.EventHandler(this.buttonPLay_Click);
            // 
            // buttonPause
            // 
            this.buttonPause.Location = new System.Drawing.Point(63, 191);
            this.buttonPause.Name = "buttonPause";
            this.buttonPause.Size = new System.Drawing.Size(75, 23);
            this.buttonPause.TabIndex = 3;
            this.buttonPause.Text = "PAUSE";
            this.buttonPause.UseVisualStyleBackColor = true;
            this.buttonPause.Click += new System.EventHandler(this.buttonStop_Click);
            // 
            // songProgress
            // 
            this.songProgress.Location = new System.Drawing.Point(63, 220);
            this.songProgress.Name = "songProgress";
            this.songProgress.Size = new System.Drawing.Size(209, 23);
            this.songProgress.TabIndex = 4;
            // 
            // volumeSlider
            // 
            this.volumeSlider.Location = new System.Drawing.Point(12, 119);
            this.volumeSlider.Maximum = 100;
            this.volumeSlider.Name = "volumeSlider";
            this.volumeSlider.Orientation = System.Windows.Forms.Orientation.Vertical;
            this.volumeSlider.Size = new System.Drawing.Size(45, 124);
            this.volumeSlider.TabIndex = 5;
            this.volumeSlider.Value = 100;
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(284, 261);
            this.Controls.Add(this.volumeSlider);
            this.Controls.Add(this.songProgress);
            this.Controls.Add(this.buttonPause);
            this.Controls.Add(this.buttonPLay);
            this.Controls.Add(this.checkedListBox1);
            this.Controls.Add(this.listBox1);
            this.Name = "Form1";
            this.Text = "Form1";
            this.Load += new System.EventHandler(this.Form1_Load);
            ((System.ComponentModel.ISupportInitialize)(this.volumeSlider)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.ListBox listBox1;
        private System.Windows.Forms.CheckedListBox checkedListBox1;
        private System.Windows.Forms.Button buttonPLay;
        private System.Windows.Forms.Button buttonPause;
        private System.Windows.Forms.ProgressBar songProgress;
        private System.Windows.Forms.TrackBar volumeSlider;
    }
}

