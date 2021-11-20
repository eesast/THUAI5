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
using System.Threading;
using System.IO;
using Communication.ClientCommunication;
using Communication.Proto;
using HPSocket;

namespace Client
{
    /// <summary>
    /// MainWindow.xaml 的交互逻辑
    /// </summary>
    public partial class MainWindow : Window
    {
        public MainWindow()
        {
            timer = new DispatcherTimer
            {
                Interval = new TimeSpan(20000)//每20ms刷新一次
            };
            timer.Tick += new EventHandler(Refresh);    //定时器初始化
            InitializeComponent();
            timer.Start();
            isClientStocked = false;
            isInitialized = false;
            playerData = new();
            bulletData = new();
            propData = new();
            myMessages = new();
            messageToServers = new();
            WindowStartupLocation = WindowStartupLocation.CenterScreen;
            args = Environment.GetCommandLineArgs();
            //注：队伍用边框区分，人物编号以背景颜色区分
            //角色死亡则对应信息框变灰
            //被动技能和buff在人物编号后用彩色文字注明
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

        private void ClickToPauseOrContinue(object sender, RoutedEventArgs e)
        {
            if (!isClientStocked)
            {
                isClientStocked = true;
                PorC.Content = "▶";
            }
            else
            {
                isClientStocked = false;
                PorC.Content = "⏸";
            }
        }

        private void ClickToSetMode(object sender, RoutedEventArgs e)
        {

        }

        //其他比赛信息

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
#pragma warning disable CS8604 // 引用类型参数可能为 null。
                _ = Process.Start(sr.ReadLine());
#pragma warning restore CS8604 // 引用类型参数可能为 null。
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

        private void ClickForHelp(object sender, RoutedEventArgs e)
        {

        }
        private void SetServer(object sender, RoutedEventArgs e)
        {
            //打开server
            Server.Background = Brushes.Green;
        }

        private void ClickToConnect(object sender, RoutedEventArgs e)
        {
            try
            {
                long playerID, teamID;
                playerID = 0; // Convert.ToInt64(args[1]);
                teamID = 0;// Convert.ToInt64(args[2]);
                communicator = new ClientCommunication();
                communicator.OnReceive += () =>
                {
                    if (communicator.TryTake(out IGameMessage msg) && msg.PacketType == PacketType.MessageToClient)
                    {
                        MessageProcedure((MessageToClient)msg.Content);
                    }
                };
                communicator.Connect("127.0.0.1", 7777);
                MessageToServer messageToServer = new MessageToServer();
                messageToServer.MessageType = MessageType.AddPlayer;
                messageToServer.PlayerID = playerID;
                messageToServer.TeamID = teamID;
                messageToServer.ASkill1 = ActiveSkillType.BecomeAssassin;
                messageToServer.PSkill = PassiveSkillType.Vampire;
                communicator.SendMessage(messageToServer);
            }
            catch (Exception exc)
            {
                ErrorDisplayer error = new("发生错误。以下是系统报告\n"+exc.Message);
                error.Show();
            }
        }
        //定时器事件，刷新地图
        private void Refresh(object? sender, EventArgs e)
        {
            t++;
        }
        private void DrawPic(MessageToClient msg)
        {
            
            Ellipse m = new Ellipse();
            m.Margin = new(200, 200, 0, 0);
            m.Fill = Brushes.Black;
            m.Height = 13;
            m.Width = 13;
            UpperLayerOfMap.Children.Add(m);
            playerData.Clear();
            bulletData.Clear();
            propData.Clear();
            if (msg.MessageType == MessageType.StartGame && msg.MessageType == MessageType.Gaming)
            {
                foreach (MessageToClient.Types.GameObjMessage i in msg.GameObjMessage)
                {
                    switch (i.ObjCase)
                    {
                        case MessageToClient.Types.GameObjMessage.ObjOneofCase.MessageOfCharacter:
                            {
                                playerData.Add(i.MessageOfCharacter);
                                Ellipse icon = new Ellipse();
                                icon.Margin = new(i.MessageOfCharacter.Y, i.MessageOfCharacter.X, 0, 0);
                                icon.Fill = Brushes.Black;
                                icon.Height = 13;
                                icon.Width = 13;
                                UpperLayerOfMap.Children.Add(icon);
                                break;
                            }
                    }
                }
            }
            
        }
        private void MessageProcedure(MessageToClient msg)
        {
            //System.Windows.Application.Current.Dispatcher.Invoke(DrawPic);
            Thread thread = new Thread(o => {
                DrawPic((MessageToClient)o);
           });
            thread.Start(msg);
        }
        //以下为Mainwindow自定义属性
        private DispatcherTimer timer;//定时器
        private ClientCommunication? communicator;
        String[] args;
        UInt64 t = 0;
        static int onetime = 0;
        private bool isClientStocked;
        private bool isInitialized;

        private Int64 playerID;
        private Int64 teamID;

        private List<MessageOfCharacter>? playerData;
        private List<MessageOfBullet>? bulletData;
        private List<MessageOfProp>? propData;
        private Stack<string>? myMessages;
        private Queue<MessageToServer>? messageToServers;

    }
}
//2021-10-23
//目前没有画图。并且，Client端能够开始游戏，但不能停止游戏，也不会收到游戏停止的消息。加上该功能后记得游戏停止时把Begin钮变红,isGameRunning置false.
//2021-10-25
//调整了一些提示出现的逻辑，并且修改了计时器，使得Error弹窗不再频繁弹出
//服务器逻辑：
//开始时Client发送一个装有技能和人物编号的信息给Server(AddPlayer)（注1）。
//Server已经指定了应该连接的玩家人数，当发送以上信息的人数达到指定人数后，Server将【自动开始游戏】并给每个玩家发送一个MessageToInitialize信息。
//发送initialize信息后Server持续发送MessageToRefresh。第一帧的MessageToRefresh.MessageType为StartGame,最后一帧为EndGame,中间为Gaming。
//注1：Client发送信息后，若ID不合法，仍能连接上Server。但会接到一个MessageToOneClient,信息类型为InvalidPlayer。相反，若ID合法，信息类型为ValidPlayer.