using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Threading;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Shapes;
using System.IO;

namespace Client
{
    /// <summary>
    /// ConnectRegister.xaml 的交互逻辑
    /// </summary>
    public partial class ConnectRegister : Window
    {
        public ConnectRegister()
        {
            InitializeComponent();
        }

        private void Save(object sender, RoutedEventArgs e)
        {
            try
            {
                if (!File.Exists("ConnectInfo.txt"))
                {
                    File.Create("ConnectInfo.txt");
                    State.Text = "Created File";
                }

                using StreamWriter sw = new("ConnectInfo.txt");
                sw.WriteLine(IPBox.Text + " " + PortBox.Text+" "+PlayerIDBox.Text+" "+TeamIDBox.Text);
                State.Text = "Info Registered.";   
            }
            catch(Exception exc)
            {
                State.Text = "Error:" + exc;
            }
        }
    }
}
