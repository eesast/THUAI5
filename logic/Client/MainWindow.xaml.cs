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
using Communication.ClientCommunication;

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
            timer.Interval = new TimeSpan(1000000 / 60);
            timer.Tick += new EventHandler(Refresh);    //定时器初始化
            InitializeComponent();
            timer.Start();
            communicator = new ClientCommunication();
            isConnected = false;
            try
            {
                using StreamReader sr = new("ConnectInfo.txt");
                string[] comInfo = sr.ReadLine().Split(' ');
                if (!communicator.Connect(comInfo[0], Convert.ToUInt16(comInfo[1])))//后一个参数处，注意系统...提示
                {
                    Exception exc = new("TimeOut");
                    throw exc;
                }
                else isConnected = true;
            }
            catch(Exception exc)
            {
                ErrorDisplayer error = new("与服务器建立连接时出错：\n" + exc.Message);
                error.Show();
            }
            WindowStartupLocation = WindowStartupLocation.CenterScreen;
            //绘制地图
                      //设想用配置文件加载地图。但要做好配置文件加密。。。
                            //注：队伍用边框区分，人物编号以背景颜色区分
                            //角色死亡则对应信息框变灰
                            //被动技能和buff在人物编号后用彩色文字注明
                            //饼：允许玩家自定义人物名称
            
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

        //可能需要通信协助
        private void ClickToSetConnect(object sender, RoutedEventArgs e)
        {
            ConnectRegister crg = new();
            crg.Show();
            
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
                if (!File.Exists("VSRoute.txt"))
                {
                    File.Create("VSRoute.txt");
                    Exception ex = new("没有路径存储文件，已为您创建。请将VS路径输入该文件，并重新操作。");
                    throw ex;
                }//创建路径文件 
                using StreamReader sr = new("VSRoute.txt");
                _ = Process.Start(sr.ReadLine());
            }
            catch (Exception exc)
            {
                ErrorDisplayer error = new("发生错误。以下是系统报告:\n" + exc.Message);
                error.Show();
            }
        }

        private void ClickToVisitEESAST(object sender, RoutedEventArgs e)
        {
            try
            {
                _ = Process.Start("C:\\Program Files (x86)\\Microsoft\\Edge\\Application\\msedge.exe", "https://eesast.com");
            }
            catch (Exception exc)
            {
                ErrorDisplayer error = new("发生错误。以下是系统报告\n" + exc.Message);
                error.Show();
            }
        }

        //定时器事件，刷新地图
        private void Refresh(object sender, EventArgs e)
        {
            //for debug
            if (isConnected)
            {
               
            }
        }
        //以下为Mainwindow自定义属性
        private readonly DispatcherTimer timer;//定时器
        private readonly ClientCommunication communicator;
        private readonly bool isConnected;
    }
}
