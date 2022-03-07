using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.ComponentModel;
using Rebex.Net;
using Path = System.IO.Path;

namespace Installer
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        private Sftp client = new Sftp();
        private readonly string serverIP = "118.195.131.159";
        public MainWindow()
        {
            InitializeComponent();
            Rebex.Licensing.Key = "==AQftydL+GH1P76OPxzLvhcaHlU3VpOwd/6xKpsB+499U==";
            client.Connect(serverIP);
            client.Login("ubuntu","password");
        }

        public void SelectFile(object sender, RoutedEventArgs e)
        {
            System.Windows.Forms.FolderBrowserDialog dialog = new System.Windows.Forms.FolderBrowserDialog();
            System.Windows.Forms.DialogResult result = dialog.ShowDialog();
            if (result == System.Windows.Forms.DialogResult.OK)
            {
                string filePath = dialog.SelectedPath;
                Text text = (Text)this.FindResource("textShow");
                text.FilePath = filePath;
            }
        }

        private void Install(object sender, RoutedEventArgs e)
        {
            string installPath = textBox.Text;
            if (Down1.Visibility == Visibility.Visible)
                Down1.Visibility = Visibility.Hidden;
            if (Down2.Visibility == Visibility.Hidden)
                Down2.Visibility = Visibility.Visible;

            var thread = new Thread(() => { DownloadFile("/home/ubuntu/THUAI5_for_Windows/", @installPath); });
            thread.Start();
        }

        public void DownloadFile(string serverDir, string localDir)
        {
            if (client.DirectoryExists(serverDir))
            {
                client.Download(serverDir, localDir);
            }
            label.Dispatcher.Invoke(
                new Action(
                    delegate
                    {
                        label.Content = "下载完成";
                    }));
        }

        //public delegate void ProgressBarSetter(double value);
        //public void SetProgressBar(double value)
        //{
        //    pb.Value = value;
        //    label.Content = (int)(value / pb.Maximum * 100) + "%";
        //}


        //public void DownloadFile(string httpUrl, string saveUrl)
        //{
        //    WebResponse response = null;
        //    WebRequest request = WebRequest.Create(httpUrl);
        //    response = request.GetResponse();
        //    if (response == null) return;
        //    pb.Maximum = response.ContentLength;

        //    ThreadPool.QueueUserWorkItem((obj) =>
        //    {
        //        Stream netStream = response.GetResponseStream();
        //        Stream fileStream = new FileStream(saveUrl, FileMode.OpenOrCreate);
        //        byte[] read = new byte[1024];
        //        long progressBarValue = 0;
        //        int realReadLen = netStream.Read(read, 0, read.Length);
        //        while (realReadLen > 0)
        //        {
        //            fileStream.Write(read, 0, realReadLen);
        //            progressBarValue += realReadLen;
        //            pb.Dispatcher.BeginInvoke(new ProgressBarSetter(SetProgressBar), progressBarValue);
        //            realReadLen = netStream.Read(read, 0, read.Length);
        //        }

        //        netStream.Close();
        //        fileStream.Close();
        //    }, null);

        //    using (var client = new Sftp())
        //    {
        //        client.Connect("118.195.131.159");
        //        client.Login("ubuntu", "eesast_DOCKER");
        //        client.Download("/home/ubuntu/THUAI5_for_Windows/file.json", saveUrl);
        //    }
        //}

        //private bool IsFileExist(string httpUrl)
        //{
        //    WebResponse response = null;
        //    bool res = false;
        //    try
        //    {
        //        response = WebRequest.Create(httpUrl).GetResponse();
        //        res = response != null;
        //    }
        //    catch (Exception) { return false; }
        //    finally
        //    {
        //        if (response != null) response.Close();
        //    }
        //    return res;
        //}
    }

    public class Text : INotifyPropertyChanged
    {
        public event PropertyChangedEventHandler PropertyChanged;
        private string filePath;

        public string FilePath
        {
            get { return this.filePath; }
            set
            {
                if (this.filePath != value)
                {
                    this.filePath = value;
                    if (PropertyChanged != null)
                        PropertyChanged(this, new PropertyChangedEventArgs("FilePath"));
                }
            }
        }
    }
}
