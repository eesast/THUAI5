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
            isGameRunning = false;
            isClientStocked = false;
            isInitialized = false;
            try
            {
                using StreamReader sr = new("ConnectInfo.txt");
                string[] comInfo = sr.ReadLine().Split(' ');
                communicator = new ClientCommunication();
                playerID = Convert.ToInt64(comInfo[2]);
                teamID = Convert.ToInt64(comInfo[3]);
                if (!communicator.Connect(comInfo[0], Convert.ToUInt16(comInfo[1])))//没加错误处理
                {
                    Exception exc = new("TimeOut");
                    throw exc;
                }
                else if(communicator.Client.IsConnected)
                {
                    MessageToServer msg = new();
                    msg.MessageType = MessageType.AddPlayer;
                    msg.PlayerID = playerID;
                    msg.TeamID = teamID;
                    communicator.SendMessage(msg);
                }//建立连接的同时加入人物
            }
            catch (Exception exc)
            {
                ErrorDisplayer error = new("与服务器建立连接时出错：\n" + exc.Message);
                error.Show();
            }
            WindowStartupLocation = WindowStartupLocation.CenterScreen;
            //注：队伍用边框区分，人物编号以背景颜色区分
            //角色死亡则对应信息框变灰
            //被动技能和buff在人物编号后用彩色文字注明
        }

        //基础窗口函数
        private void ClickToClose(object sender, RoutedEventArgs e)
        {
            _ = communicator.Stop();
            communicator.Dispose();
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
            if (communicator.Client.IsConnected && (!isGameRunning))
            {
                MessageToServer msg = new();
                msg.MessageType = MessageType.StartGame;
                msg.PlayerID = playerID;
                msg.TeamID = teamID;
                communicator.SendMessage(msg);
                Begin.Background = Brushes.Gray;//未完成初始化但已按下按钮，显示为灰色。完成后显示为黄色。
            }
        }

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
            try
            {
                if (communicator.Client.IsConnected)
                {
                    playerData = new();
                    bulletData = new();
                    propData = new();
                    myMessages = new();
                    messageToServers = new();
                    items = new();
                    if (!isInitialized)
                    {
                        IGameMessage msg = communicator.Take();
                        if (msg.PacketType == PacketType.MessageToInitialize)
                        {
                            MessageToInitialize messageToInitialize = msg.Content as MessageToInitialize;
                            Map.Source = new BitmapImage(new Uri(Convert.ToString(messageToInitialize.MapSerial) + ".png", UriKind.Relative));
                            MessageToServer reply = new();
                            reply.MessageType = MessageType.InitialLized;
                            reply.PlayerID = playerID;
                            reply.TeamID = teamID;
                            communicator.SendMessage(reply);
                            isGameRunning = true;
                            Begin.Background = Brushes.Yellow;
                            Begin.Content = "⚪";
                        }
                        //若收到初始化信息，初始化，发送“已收到”并将IsGameRunning置为真,按钮置黄,标志变成⚪。
                    }
                    else if (!isClientStocked)
                    {
                        IGameMessage msg = communicator.Take();
                        switch (msg.PacketType)
                        {
                            case PacketType.MessageToOneClient:
                                {
                                    MessageToOneClient messageToOneClient = msg.Content as MessageToOneClient;
                                    if (messageToOneClient.PlayerID == playerID && messageToOneClient.TeamID == teamID)
                                    {
                                        myMessages.Push(messageToOneClient.Message);
                                    }
                                }
                                break;
                            case PacketType.MessageToClient:
                                {
                                    bulletData.Clear();
                                    propData.Clear();
                                    playerData.Clear();
                                    items.Clear();
                                    MessageToClient messageToClient = msg.Content as MessageToClient;
                                    foreach (MessageToClient.Types.GameObjMessage i in messageToClient.GameObjMessage)
                                    {
                                        switch (i.ObjCase)
                                        {
                                            case MessageToClient.Types.GameObjMessage.ObjOneofCase.MessageOfBullet:
                                                {
                                                    bulletData.Add(i.MessageOfBullet);
                                                    Ellipse item = new Ellipse();
                                                    item.Stroke = Brushes.Black;
                                                    item.Fill = Brushes.DarkBlue;
                                                    item.HorizontalAlignment = HorizontalAlignment.Left;
                                                    item.VerticalAlignment = VerticalAlignment.Top;
                                                    item.Margin = new((i.MessageOfBullet.Y * 13 / 1000) - 4, (i.MessageOfBullet.X * 13 / 1000) - 4, 0, 0);//确认坐标轴方向
                                                    item.Width = 8;
                                                    item.Height = 8;
                                                    UpperLayerOfMap.Children.Add(item);
                                                    items.Add(item);
                                                    break;
                                                }
                                            case MessageToClient.Types.GameObjMessage.ObjOneofCase.MessageOfCharacter:
                                                {
                                                    playerData.Add(i.MessageOfCharacter);
                                                    Ellipse item = new Ellipse();
                                                    item.Stroke = Brushes.Orange;
                                                    item.Fill = Brushes.Orange;//目前同种游戏实例的颜色都是一样的。
                                                    item.HorizontalAlignment = HorizontalAlignment.Left;
                                                    item.VerticalAlignment = VerticalAlignment.Top;
                                                    item.Margin = new((i.MessageOfBullet.Y * 13 / 1000) - 6.5, (i.MessageOfBullet.X * 13 / 1000) - 6.5, 0, 0);
                                                    item.Width = 13;
                                                    item.Height = 13;
                                                    UpperLayerOfMap.Children.Add(item);
                                                    items.Add(item);
                                                    break;
                                                }
                                            case MessageToClient.Types.GameObjMessage.ObjOneofCase.MessageOfProp:
                                                {
                                                    propData.Add(i.MessageOfProp);
                                                    Ellipse item = new Ellipse();
                                                    item.Stroke = Brushes.Blue;
                                                    item.Fill = Brushes.Blue;
                                                    item.HorizontalAlignment = HorizontalAlignment.Left;
                                                    item.VerticalAlignment = VerticalAlignment.Top;
                                                    item.Margin = new((i.MessageOfBullet.Y * 13 / 1000) - 4, (i.MessageOfBullet.X * 13 / 1000) - 4, 0, 0);
                                                    item.Width = 8;
                                                    item.Height = 8;
                                                    UpperLayerOfMap.Children.Add(item);
                                                    items.Add(item);
                                                    break;
                                                }
                                            default: break;//目前不会对侧边栏的人物信息做出改动。
                                        }
                                    }
                                }
                                break;
                            default:
                                break;
                        }
                        while (messageToServers.Count != 0)
                        {
                            communicator.SendMessage(messageToServers.Dequeue());
                        }
                        //对于MyMessage，进栈和出栈都在API中完成。
                    }
                }
            }
            catch (Exception exc)
            {
                ErrorDisplayer error = new("发生错误。以下是系统报告\n" + exc.Message);
                error.Show();
            }
        }
        //以下为Mainwindow自定义属性
        private readonly DispatcherTimer timer;//定时器
        private readonly ClientCommunication communicator;
        private bool isGameRunning;
        private bool isClientStocked;
        private readonly bool isInitialized;

        private readonly Int64 playerID;
        private readonly Int64 teamID;

        private List<MessageOfCharacter> playerData;
        private List<MessageOfBullet> bulletData;
        private List<MessageOfProp> propData;
        private Stack<string> myMessages;
        private Queue<MessageToServer> messageToServers;

        private List<Ellipse> items;
    }
}
//2021-10-23
//目前没有画图。并且，Client端能够开始游戏，但不能停止游戏，也不会收到游戏停止的消息。