using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Threading;
using System.Windows.Controls;
using System.Windows.Data;
using System.Diagnostics;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.IO;

namespace Client
{
    /// <summary>
    /// MainWindow.xaml 的交互逻辑
    /// </summary>
    public partial class MainWindow : Window
    {     
        public MainWindow()
        {
            timer = new DispatcherTimer();
            timer.Interval = new TimeSpan(10000000);
            timer.Tick += new EventHandler(Refresh);    //定时器初始化
            InitializeComponent();
            timer.Start();
            WindowStartupLocation = WindowStartupLocation.CenterScreen;
            //绘制地图
            try
            {
                for (int x = 0; x < 50; x++)
                {
                    for (int y = 0; y < 50; y++)
                    {
                        textBox[x, y] = new TextBox();
                        textBox[x, y].Text = "";
                        textBox[x, y].Background = Brushes.Black;
                        textBox[x, y].BorderBrush = Brushes.Black;
                        textBox[x, y].Height = 10;
                        textBox[x, y].Width = 10;
                        textBox[x, y].HorizontalAlignment = HorizontalAlignment.Left;
                        textBox[x, y].VerticalAlignment = VerticalAlignment.Top;
                        textBox[x, y].Margin = new Thickness(10 * x, 10 * y, 0, 0);
                        textBox[x, y].IsEnabled = true;
                        Map.Children.Add(textBox[x, y]);
                    }
                }
            }
            catch (Exception exc)
            {
                ErrorDisplayer error = new ErrorDisplayer("发生错误。以下是系统报告\n" + exc.Message);
                error.Show();
            }
        }

        //基础窗口函数
        private void ClickToClose(object sender, RoutedEventArgs e)
        {
            Application.Current.Shutdown();
        }

        private void ClickToMinimize(object sender, RoutedEventArgs e)
        {
            WindowState = WindowState.Minimized;
        }
        private void DragWindow(object sender, RoutedEventArgs e)
        {
            DragMove();
        }

        //Client控制函数
        private void ClickToBegin(object sender, RoutedEventArgs e)
        {
        
        }

        private void ClickToPause(object sender, RoutedEventArgs e)
        {

        }

        private void ClickToSetMode(object sender, RoutedEventArgs e)
        {
           
        }

        //其他比赛信息
        private void CallTeam1Message(object sender, RoutedEventArgs e)
        {
            InfoDisplayer infoDisplayer1 = new InfoDisplayer();
            infoDisplayer1.teamNumber = 1;
            if (flag[0] == 0)
            {
                infoDisplayer1.Title = "team2信息";
                infoDisplayer1.Left = 18;
                infoDisplayer1.Top = 61;
                infoDisplayer1.Show();
                flag[0]++;
            }
        }


        private void CallTeam2Message(object sender, RoutedEventArgs e)
        {
            InfoDisplayer infoDisplayer2 = new InfoDisplayer();
            infoDisplayer2.teamNumber = 2;
            if (flag[1] == 0)
            {
                infoDisplayer2.Title = "team2信息";
                infoDisplayer2.Left = 1218;
                infoDisplayer2.Top = 61;
                infoDisplayer2.Show();
                flag[1]++;
            }
        }

        //可能需要通信协助
        private void ClickToSetConnect(object sender, RoutedEventArgs e)
        {

        }

        //以下两个函数可能需要网站协助
        private void ClickToCheckLadder(object sender, RoutedEventArgs e)
        {

        }

        private void ClickForUpdate(object sender, RoutedEventArgs e)
        {

        }

        //杂项功能
        private void ClickToEnterVS(object sender, RoutedEventArgs e)
        {
            try
            { 
                FileStream route = new FileStream("VSRoute.txt", FileMode.OpenOrCreate, FileAccess.ReadWrite);//创建路径文件 
                StreamReader Route = new StreamReader(route);
                string s = Route.ReadLine();
                Process.Start(s);
            }
            catch (Exception exc)
            {
                ErrorDisplayer error = new ErrorDisplayer("发生错误。以下是系统报告:\n" + exc.Message);
                error.Show();
            }
        }

        private void ClickToVisitEESAST(object sender, RoutedEventArgs e)
        {
            try
            {
                Process.Start("https://eesast.com/login");
            }
            catch (Exception exc)
            {
                ErrorDisplayer error = new ErrorDisplayer("发生错误。以下是系统报告\n" + exc.Message);
                error.Show();
            }
        }

        //定时器事件，刷新地图
        private void Refresh(object sender, EventArgs e)
        {
            //for debug
            i = (i + 1) % 2;
            switch(i)
            {
                case 0:
                    textBox[25, 25].Background = Brushes.White;
                    break;
                case 1:
                    textBox[25, 25].Background = Brushes.Black;
                    break;
            }
        }
        //以下为Mainwindow自定义属性
        private TextBox[,] textBox = new TextBox[50, 50];
        private DispatcherTimer timer;//定时器
        private static int i;//for debug
        public static int[] flag = new int[2];//防止重复唤出窗口
    }
}
