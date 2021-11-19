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
            playerData = new List<MessageToClient.Types.GameObjMessage>();
            bulletData = new List<MessageToClient.Types.GameObjMessage>();
            propData = new List<MessageToClient.Types.GameObjMessage>();
            myMessages = new();
            messageToServers = new();
            WindowStartupLocation = WindowStartupLocation.CenterScreen;
            //注：队伍用边框区分，人物编号以背景颜色区分
            //角色死亡则对应信息框变灰
            //被动技能和buff在人物编号后用彩色文字注明
        }

        //基础窗口函数
        private void ClickToClose(object sender, RoutedEventArgs e)
        {
            if (communicator != null)
            {
                if (communicator.Client.IsConnected)
                {
                    _ = communicator.Stop();
                }
                communicator.Dispose();
            }
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
            try
            {
                if(communicator == null)
                { 
                    throw new Exception("Error:communicator is unexpectedly null");
                }
                else if (communicator.Client.IsConnected && (!isGameRunning))
                {
                    MessageToServer msg = new();
                    msg.MessageType = MessageType.StartGame;
                    msg.PlayerID = playerID;
                    msg.TeamID = teamID;
                    communicator.SendMessage(msg);
                    //isGameRunning = true; 开始游戏应在收到消息后才开始
                    Begin.Background = Brushes.Gray;//未完成初始化但已按下按钮，显示为灰色。完成后显示为黄色。
                }
            }
            catch (Exception exc)
            {
                ErrorDisplayer error = new(exc.ToString());
                error.Show();
                Begin.Background = Brushes.Crimson;
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
#pragma warning disable CS8604 // 引用类型参数可能为 null。
                _ = Process.Start(sr.ReadLine());
#pragma warning restore CS8604 // 引用类型参数可能为 null。
            }
            catch (Exception exc)
            {
                ErrorDisplayer error = new("发生错误。以下是系统报告:\n" + exc.ToString());
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
                ErrorDisplayer error = new("发生错误。以下是系统报告\n" + exc.ToString());
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
            Connect.Background = Brushes.Gray;
            try
            {
                using (var sr = new StreamReader("ConnectInfo.txt"))
                {
#pragma warning disable CS8602 // 解引用可能出现空引用。
                    string[] comInfo = sr.ReadLine().Split(' ');
#pragma warning restore CS8602 // 解引用可能出现空引用。
                    if (comInfo[0] == "" || comInfo[1] == "" || comInfo[2] == "" || comInfo[3] == "")
                    {
                        throw new Exception("Length<4");
                    }
                    communicator = new ClientCommunication();
                    communicator.OnReceive += OnReceive;
                    playerID = Convert.ToInt64(comInfo[2]);
                    teamID = Convert.ToInt64(comInfo[3]);
                    if (!communicator.Connect(comInfo[0], Convert.ToUInt16(comInfo[1])))//没加错误处理
                    {
                        Exception exc = new("TimeOut");
                        throw exc;
                    }
                    else if (communicator.Client.IsConnected)
                    {
                        Connect.Background = Brushes.Green;
                        MessageToServer msg = new();
                        msg.MessageType = MessageType.AddPlayer;
                        msg.PlayerID = playerID;
                        msg.TeamID = teamID;
                        communicator.SendMessage(msg);
                        Connect.Background = Brushes.Green;
                    }//建立连接的同时加入人物
                }
            }
            catch (Exception exc)
            {
                if(exc.Message== "Length<4")
                {
                    ConnectRegister crg = new();
                    crg.State.Text = "配置非法，请重新输入或检查配置文件。";
                    crg.Show();
                }
                else
                {
                    ErrorDisplayer error = new("与服务器建立连接时出错：\n" + exc.ToString());
                    error.Show();
                    Connect.Background = Brushes.Aqua;
                    if (communicator != null)
                    {
                        if (communicator.Client.IsConnected)
                        {
                            _ = communicator.Stop();
                        }
                        communicator.Dispose();
                        communicator = null;
                    }
                }
            }
        }

        private void OnReceive()
        {
            if (communicator.TryTake(out IGameMessage msg) && msg.PacketType == PacketType.MessageToClient)
            {
                lock (drawPicLock)
                {
                    playerData.Clear();
                    propData.Clear();
                    bulletData.Clear();
                    MessageToClient content = (MessageToClient)msg.Content;
                    switch (content.MessageType)
                    {
                        case MessageType.InitialLized:
                            //这里要设置地图类型
                            //可地图还没写呢...
                            break;
                        case MessageType.StartGame:
                            isGameRunning = true;
                            foreach (MessageToClient.Types.GameObjMessage obj in content.GameObjMessage)
                            {
                                switch (obj.ObjCase)
                                {
                                    case MessageToClient.Types.GameObjMessage.ObjOneofCase.MessageOfCharacter:
                                        playerData.Add(obj);
                                        break;
                                    case MessageToClient.Types.GameObjMessage.ObjOneofCase.MessageOfBullet:
                                        bulletData.Add(obj);
                                        break;
                                    case MessageToClient.Types.GameObjMessage.ObjOneofCase.MessageOfProp:
                                        propData.Add(obj);
                                        break;
                                }
                            }
                            break;
                        case MessageType.Gaming:

                            foreach (MessageToClient.Types.GameObjMessage obj in content.GameObjMessage)
                            {
                                switch (obj.ObjCase)
                                {
                                    case MessageToClient.Types.GameObjMessage.ObjOneofCase.MessageOfCharacter:
                                        playerData.Add(obj);
                                        break;
                                    case MessageToClient.Types.GameObjMessage.ObjOneofCase.MessageOfBullet:
                                        bulletData.Add(obj);
                                        break;
                                    case MessageToClient.Types.GameObjMessage.ObjOneofCase.MessageOfProp:
                                        propData.Add(obj);
                                        break;
                                }
                            }
                            break;
                        case MessageType.EndGame:
                            foreach (MessageToClient.Types.GameObjMessage obj in content.GameObjMessage)
                            {
                                switch (obj.ObjCase)
                                {
                                    case MessageToClient.Types.GameObjMessage.ObjOneofCase.MessageOfCharacter:
                                        playerData.Add(obj);
                                        break;
                                    case MessageToClient.Types.GameObjMessage.ObjOneofCase.MessageOfBullet:
                                        bulletData.Add(obj);
                                        break;
                                    case MessageToClient.Types.GameObjMessage.ObjOneofCase.MessageOfProp:
                                        propData.Add(obj);
                                        break;
                                }
                            }
                            isGameRunning = false;
                            break;
                    }
                }
            }
        }
        //定时器事件，刷新地图
        private void Refresh(object? sender, EventArgs e)
        {
            try
            {
                if(isGameRunning)
                {
                    UpperLayerOfMap.Children.Clear();
                    if (communicator==null)
                    {
                        throw new Exception("Error: communicator is unexpectly null during a running game");
                    }
                    else if (communicator.Client.IsConnected)
                    {
                        lock(drawPicLock)
                        { 
                            foreach (var data in playerData)
                            {
                                Ellipse icon = new Ellipse();
                                icon.Width = 13;
                                icon.Height = 13;
                                icon.HorizontalAlignment = HorizontalAlignment.Left;
                                icon.VerticalAlignment = VerticalAlignment.Top;
                                icon.Margin = new Thickness(data.MessageOfCharacter.Y * 13.0 / 1000.0, data.MessageOfCharacter.X * 13.0 / 1000.0, 0, 0);
                                icon.Fill = Brushes.Black;
                                UpperLayerOfMap.Children.Add(icon);
                            }
                            foreach (var data in bulletData)
                            {
                                Ellipse icon = new Ellipse();
                                icon.Width = 10;
                                icon.Height = 10;
                                icon.HorizontalAlignment = HorizontalAlignment.Left;
                                icon.VerticalAlignment = VerticalAlignment.Top;
                                icon.Margin = new Thickness(data.MessageOfBullet.Y * 13.0 / 1000.0, data.MessageOfBullet.X * 13.0 / 1000.0, 0, 0);
                                icon.Fill = Brushes.Red;
                                UpperLayerOfMap.Children.Add(icon);
                            }
                            foreach (var data in propData)
                            {
                                if (data.MessageOfProp.Type == PropType.Gem)
                                {
                                    Ellipse icon = new Ellipse();
                                    icon.Width = 10;
                                    icon.Height = 10;
                                    icon.HorizontalAlignment = HorizontalAlignment.Left;
                                    icon.VerticalAlignment = VerticalAlignment.Top;
                                    icon.Margin = new Thickness(data.MessageOfProp.Y * 13.0 / 1000.0, data.MessageOfProp.X * 13.0 / 1000.0, 0, 0);
                                    icon.Fill = Brushes.Purple;
                                    UpperLayerOfMap.Children.Add(icon);
                                }
                                else
                                {
                                    Ellipse icon = new Ellipse();
                                    icon.Width = 10;
                                    icon.Height = 10;
                                    icon.HorizontalAlignment = HorizontalAlignment.Left;
                                    icon.VerticalAlignment = VerticalAlignment.Top;
                                    icon.Margin = new Thickness(data.MessageOfProp.Y * 13.0 / 1000.0, data.MessageOfProp.X * 13.0 / 1000.0, 0, 0);
                                    icon.Fill = Brushes.Gray;
                                    UpperLayerOfMap.Children.Add(icon);
                                }
                            }
                        }
                        {
                            //if (!isInitialized)
                            //{
                            //    //一般来说new不会失败，所以下面的警告忽略。
                            //    IGameMessage msg = communicator.Take();
                            //    if (msg.PacketType == PacketType.MessageToInitialize)
                            //    {
                            //        MessageToInitialize? messageToInitialize = msg.Content as MessageToInitialize;
                            //        if (messageToInitialize == null)
                            //        {
                            //            throw new Exception("Error:Null Map Serial");
                            //        }
                            //        Map.Source = new BitmapImage(new Uri(Convert.ToString(messageToInitialize.MapSerial) + ".png", UriKind.Relative));
                            //        MessageToServer reply = new();
                            //        reply.MessageType = MessageType.InitialLized;
                            //        reply.PlayerID = playerID;
                            //        reply.TeamID = teamID;
                            //        communicator.SendMessage(reply);
                            //        Begin.Background = Brushes.Yellow;
                            //        Begin.Content = "⚪";
                            //    }
                            //    //若收到初始化信息，初始化，发送“已收到”并将IsGameRunning置为真,按钮置黄,标志变成⚪。
                            //}
                            //else if (!isClientStocked)
                            //{
                            //    IGameMessage msg = communicator.Take();
                            //    switch (msg.PacketType)
                            //    {
                            //        case PacketType.MessageToOneClient:
                            //            {
                            //                MessageToOneClient? messageToOneClient = msg.Content as MessageToOneClient;
                            //                if (messageToOneClient != null && messageToOneClient.PlayerID == playerID && messageToOneClient.TeamID == teamID)
                            //                {
                            //                    myMessages.Push(messageToOneClient.Message);
                            //                }
                            //            }
                            //            break;
                            //        case PacketType.MessageToClient:
                            //            {
                            //                bulletData.Clear();
                            //                propData.Clear();
                            //                playerData.Clear();
                            //                while (UpperLayerOfMap.Children.Count != 0)
                            //                {
                            //                    UpperLayerOfMap.Children.RemoveAt(0);
                            //                }
                            //                MessageToClient? messageToClient = msg.Content as MessageToClient;
                            //                if (messageToClient != null)
                            //                {
                            //                    foreach (MessageToClient.Types.GameObjMessage i in messageToClient.GameObjMessage)
                            //                    {
                            //                        switch (i.ObjCase)
                            //                        {
                            //                            case MessageToClient.Types.GameObjMessage.ObjOneofCase.MessageOfBullet:
                            //                                {
                            //                                    bulletData.Add(i.MessageOfBullet);
                            //                                    Ellipse item = new Ellipse();
                            //                                    item.Stroke = Brushes.Black;
                            //                                    item.Fill = Brushes.DarkBlue;
                            //                                    item.HorizontalAlignment = HorizontalAlignment.Left;
                            //                                    item.VerticalAlignment = VerticalAlignment.Top;
                            //                                    item.Margin = new((i.MessageOfBullet.Y * 13 / 1000) - 4, (i.MessageOfBullet.X * 13 / 1000) - 4, 0, 0);//确认坐标轴方向
                            //                                    item.Width = 8;
                            //                                    item.Height = 8;
                            //                                    UpperLayerOfMap.Children.Add(item);
                            //                                    break;
                            //                                }
                            //                            case MessageToClient.Types.GameObjMessage.ObjOneofCase.MessageOfCharacter:
                            //                                {
                            //                                    playerData.Add(i.MessageOfCharacter);
                            //                                    Ellipse item = new Ellipse();
                            //                                    item.Stroke = Brushes.Orange;
                            //                                    item.Fill = Brushes.Orange;//目前同种游戏实例的颜色都是一样的。
                            //                                    item.HorizontalAlignment = HorizontalAlignment.Left;
                            //                                    item.VerticalAlignment = VerticalAlignment.Top;
                            //                                    item.Margin = new((i.MessageOfBullet.Y * 13 / 1000) - 6.5, (i.MessageOfBullet.X * 13 / 1000) - 6.5, 0, 0);
                            //                                    item.Width = 13;
                            //                                    item.Height = 13;
                            //                                    UpperLayerOfMap.Children.Add(item);
                            //                                    break;
                            //                                }
                            //                            case MessageToClient.Types.GameObjMessage.ObjOneofCase.MessageOfProp:
                            //                                {
                            //                                    propData.Add(i.MessageOfProp);
                            //                                    Ellipse item = new Ellipse();
                            //                                    item.Stroke = Brushes.Blue;
                            //                                    item.Fill = Brushes.Blue;
                            //                                    item.HorizontalAlignment = HorizontalAlignment.Left;
                            //                                    item.VerticalAlignment = VerticalAlignment.Top;
                            //                                    item.Margin = new((i.MessageOfBullet.Y * 13 / 1000) - 4, (i.MessageOfBullet.X * 13 / 1000) - 4, 0, 0);
                            //                                    item.Width = 8;
                            //                                    item.Height = 8;
                            //                                    UpperLayerOfMap.Children.Add(item);
                            //                                    break;
                            //                                }
                            //                            default: break;//目前不会对侧边栏的人物信息做出改动。
                            //                        }
                            //                    }
                            //                }
                            //            }
                            //            break;
                            //        default:
                            //            break;
                            //    }
                            //    while (messageToServers.Count != 0)
                            //    {
                            //        communicator.SendMessage(messageToServers.Dequeue());
                            //    }
                            //    //对于MyMessage，进栈和出栈都在API中完成。
                            //}
                        }
                    }
                }
            }
            catch (Exception exc)
            {
                ErrorDisplayer error = new("发生错误。以下是系统报告\n" + exc.ToString());
                error.Show();
                isGameRunning = false;
            }
        }
        //以下为Mainwindow自定义属性
        private DispatcherTimer timer;//定时器
        private ClientCommunication? communicator;

        private bool isGameRunning;
        private bool isClientStocked;
        private bool isInitialized;

        private Int64 playerID;
        private Int64 teamID;

        private List<MessageToClient.Types.GameObjMessage> playerData;
        private List<MessageToClient.Types.GameObjMessage> bulletData;
        private List<MessageToClient.Types.GameObjMessage> propData;
        private object drawPicLock = new object();

        private Stack<string>? myMessages;
        private Queue<MessageToServer>? messageToServers;


    }
}
//2021-10-23
//目前没有画图。并且，Client端能够开始游戏，但不能停止游戏，也不会收到游戏停止的消息。加上该功能后记得游戏停止时把Begin钮变红,isGameRunning置false.
//2021-10-25
//调整了一些提示出现的逻辑，并且修改了计时器，使得Error弹窗不再频繁弹出。