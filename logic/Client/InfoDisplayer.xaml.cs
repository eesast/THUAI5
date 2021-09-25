using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Shapes;
using System.ComponentModel;
namespace Client
{
    /// <summary>
    /// InfoDisplayer.xaml 的交互逻辑
    /// </summary>
    
    public partial class InfoDisplayer : Window
    {
        public InfoDisplayer()
        {
            InitializeComponent();
            WindowStartupLocation = WindowStartupLocation.Manual;
            Closing += new System.ComponentModel.CancelEventHandler((object sender,CancelEventArgs e)=> { MainWindow.flag[teamNumber - 1]--; });
        }

        private void DragWindow(object sender, RoutedEventArgs e)
        {
            DragMove();
        }
        public int teamNumber;
    }
}
