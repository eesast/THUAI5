using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;
using System.ComponentModel;
using System.Net;
using System.IO;
using Path = System.IO.Path;

namespace Installer
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        public MainWindow()
        {
            InitializeComponent();
        }

        private readonly string remoteUrl = "https://cloud.tsinghua.edu.cn/f/964b7d0e222b491ca2ac/?dl=1";

        public void SelectFile(object sender, RoutedEventArgs e)
        {
            System.Windows.Forms.FolderBrowserDialog dialog = new System.Windows.Forms.FolderBrowserDialog();
            System.Windows.Forms.DialogResult result = dialog.ShowDialog();
            if (result == System.Windows.Forms.DialogResult.OK)
            {
                string filePath = dialog.SelectedPath;
                Text text = (Text)FindResource("textShow");
                text.FilePath = filePath;
            }
        }

        private void Install(object sender, RoutedEventArgs e)
        {
            string installPath = Path.Combine(textBox.Text, "THUAI5_EE.zip");
            if (IsFileExist(remoteUrl) && installPath != "THUAI5_EE.zip")
            {
                if (Down1.Visibility == Visibility.Visible)
                    Down1.Visibility = Visibility.Hidden;
                if (Down2.Visibility == Visibility.Hidden)
                    Down2.Visibility = Visibility.Visible;
                DownloadFile(remoteUrl, @installPath);
            }
        }

        public delegate void ProgressBarSetter(double value);
        public void SetProgressBar(double value)
        {
            pb.Value = value;
            label.Content = (int)(value / pb.Maximum * 100) + "%";
        }

        public void DownloadFile(string httpUrl, string saveUrl)
        {
            WebResponse response = null;
            WebRequest request = WebRequest.Create(httpUrl);
            response = request.GetResponse();
            if (response == null) return;
            pb.Maximum = response.ContentLength;

            ThreadPool.QueueUserWorkItem((obj) =>
            {
                Stream netStream = response.GetResponseStream();
                Stream fileStream = new FileStream(saveUrl, FileMode.Create);
                byte[] read = new byte[1024];
                long progressBarValue = 0;
                int realReadLen = netStream.Read(read, 0, read.Length);
                while (realReadLen > 0)
                {
                    fileStream.Write(read, 0, realReadLen);
                    progressBarValue += realReadLen;
                    pb.Dispatcher.BeginInvoke(new ProgressBarSetter(SetProgressBar), progressBarValue);
                    realReadLen = netStream.Read(read, 0, read.Length);
                }

                netStream.Close();
                fileStream.Close();
            }, null);
        }

        private bool IsFileExist(string httpUrl)
        {
            WebResponse response = null;
            bool res = false;
            try
            {
                response = WebRequest.Create(httpUrl).GetResponse();
                res = response != null;
            }
            catch (Exception) { return false; }
            finally
            {
                if (response != null) response.Close();
            }
            return res;
        }
    }

    public class Text : INotifyPropertyChanged
    {
        public event PropertyChangedEventHandler PropertyChanged;
        private string filePath;

        public string FilePath
        {
            get { return filePath; }
            set
            {
                if (filePath != value)
                {
                    filePath = value;
                    PropertyChanged?.Invoke(this, new PropertyChangedEventArgs("FilePath"));
                }
            }
        }
    }
}
