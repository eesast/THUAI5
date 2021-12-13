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
            DrawMap();
            timer.Start();
            isClientStocked = true;
            playerData = new List<MessageToClient.Types.GameObjMessage>();
            bulletData = new List<MessageToClient.Types.GameObjMessage>();
            propData = new List<MessageToClient.Types.GameObjMessage>();
            myMessages = new();
            WindowStartupLocation = WindowStartupLocation.CenterScreen;
            communicator = new ClientCommunication();
            communicator.OnReceive += OnReceive;
            //注：队伍用边框区分，人物编号以背景颜色区分
            //角色死亡则对应信息框变灰
            //被动技能和buff在人物编号后用彩色文字注明
        }

        //基础窗口函数
        private void ClickToClose(object sender, RoutedEventArgs e)
        {
            if (communicator.Client.IsConnected)
            {
                _ = communicator.Stop();
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
        private void Attack(object sender,RoutedEventArgs e)
        {
            MessageToServer msgJ = new MessageToServer();
            msgJ.MessageType = MessageType.Attack;
            msgJ.PlayerID = playerID;
            msgJ.TeamID = teamID;
            foreach(var i in playerData)
            {
                //待补充
            }
            communicator.SendMessage(msgJ);
        }
        private void DrawMap()
        {
            for (int i = 0; i < defaultMap.GetLength(0); i++)
            {
                for (int j = 0; j < defaultMap.GetLength(1); j++)
                {
                    Rectangle rectangle = new Rectangle();
                    rectangle.Width = 13;
                    rectangle.Height = 13;
                    rectangle.HorizontalAlignment = HorizontalAlignment.Left;
                    rectangle.VerticalAlignment = VerticalAlignment.Top;
                    rectangle.Margin = new Thickness(13 * (j + 0.5), 13 * (i + 0.5), 0, 0);
                    switch (defaultMap[i, j])
                    {
                        case 1:
                            rectangle.Fill = Brushes.Brown;
                            rectangle.Stroke = Brushes.Brown;
                            break;
                        case 2:
                        case 3:
                        case 4:
                            rectangle.Fill = Brushes.Green;
                            rectangle.Stroke = Brushes.Green;
                            break;
                    }
                    UnderLayerOfMap.Children.Add(rectangle);
                }
            }
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
        private void ClickToConnect(object sender, RoutedEventArgs e)
        {
            if (!communicator.Client.IsConnected)//第一次连接失败后第二次连接会出错
            {
                try
                {
                    using (var sr = new StreamReader(".\\ConnectInfo.txt"))
                    {
#pragma warning disable CS8602 // 解引用可能出现空引用。
                        string[] comInfo = sr.ReadLine().Split(' ');
#pragma warning restore CS8602 // 解引用可能出现空引用。
                        if (comInfo[0] == "" || comInfo[1] == "" || comInfo[2] == "" || comInfo[3] == ""|| comInfo[4] == ""|| comInfo[5] == "")
                        {
                            throw new Exception("Input data not sufficent");
                        }
                        playerID = Convert.ToInt64(comInfo[2]);
                        teamID = Convert.ToInt64(comInfo[3]);
                        Connect.Background = Brushes.Gray;
                        if (!communicator.Connect(comInfo[0], Convert.ToUInt16(comInfo[1])))//没加错误处理
                        {
                            Connect.Background = Brushes.Aqua;
                            Exception exc = new("TimeOut");
                            throw exc;
                        }
                        else if (communicator.Client.IsConnected)
                        {
                            MessageToServer msg = new();
                            msg.MessageType = MessageType.AddPlayer;
                            msg.PlayerID = playerID;
                            msg.TeamID = teamID;
                            switch(Convert.ToInt64(comInfo[4]))
                            {
                                case 0:
                                    msg.PSkill = PassiveSkillType.NullPassiveSkillType;
                                    break;
                                case 1:
                                    msg.PSkill = PassiveSkillType.RecoverAfterBattle;
                                    break;
                                case 2:
                                    msg.PSkill = PassiveSkillType.SpeedUpWhenLeavingGrass;
                                    break;
                                case 3:
                                    msg.PSkill = PassiveSkillType.Vampire;
                                    break;
                                case 4:
                                    msg.PSkill = PassiveSkillType.Pskill3;
                                    break;
                                case 5:
                                    msg.PSkill = PassiveSkillType.Pskill4;
                                    break;
                                default:
                                    msg.PSkill = PassiveSkillType.Pskill5;
                                    break;
                            }
                            switch(Convert.ToInt64(comInfo[5]))
                            {
                                case 0:
                                    msg.ASkill1 = ActiveSkillType.NullActiveSkillType;
                                    msg.ASkill2 = ActiveSkillType.NullActiveSkillType;
                                    break;
                                case 1:
                                    msg.ASkill1 = ActiveSkillType.BecomeAssassin;
                                    msg.ASkill2 = ActiveSkillType.NullActiveSkillType;
                                    break;
                                case 2:
                                    msg.ASkill1 = ActiveSkillType.BecomeVampire;
                                    msg.ASkill2 = ActiveSkillType.NullActiveSkillType;
                                    break;
                                case 3:
                                    msg.ASkill1 = ActiveSkillType.NuclearWeapon;
                                    msg.ASkill2 = ActiveSkillType.NullActiveSkillType;
                                    break;
                                case 4:
                                    msg.ASkill1 = ActiveSkillType.SuperFast;
                                    msg.ASkill2 = ActiveSkillType.NullActiveSkillType;
                                    break;
                                case 5:
                                    msg.ASkill1 = ActiveSkillType.Askill4;
                                    msg.ASkill2 = ActiveSkillType.NullActiveSkillType;
                                    break;
                                default:
                                    msg.ASkill1 = ActiveSkillType.Askill5;
                                    msg.ASkill2 = ActiveSkillType.NullActiveSkillType;
                                    break;
                            }
                            communicator.SendMessage(msg);
                            Connect.Background = Brushes.Green;
                            isClientStocked = false;
                            PorC.Content = "⏸";
                        }//建立连接的同时加入人物
                    }
                }
                catch (Exception exc)
                {
                    if (exc.Message == "Length<4")
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
            else
            {
                //_=communicator.Stop();
                //Connect.Background = Brushes.Aqua;
                MessageBox.Show("您已连接服务器！");
            }
        }

        private void KeyBoardControl(object sender, KeyEventArgs e)
        {
            switch (e.Key)
            {
                case Key.W:
                    MessageToServer msgA = new MessageToServer();
                    msgA.MessageType = MessageType.Move;
                    msgA.PlayerID = playerID;
                    msgA.TeamID = teamID;
                    msgA.TimeInMilliseconds = 50;
                    msgA.Angle = Math.PI;
                    communicator.SendMessage(msgA);
                    break;
                case Key.S:
                    MessageToServer msgD = new MessageToServer();
                    msgD.MessageType = MessageType.Move;
                    msgD.PlayerID = playerID;
                    msgD.TeamID = teamID;
                    msgD.TimeInMilliseconds = 50;
                    msgD.Angle = 0;
                    communicator.SendMessage(msgD);
                    break;
                case Key.D:
                    MessageToServer msgW = new MessageToServer();
                    msgW.MessageType = MessageType.Move;
                    msgW.PlayerID = playerID;
                    msgW.TeamID = teamID;
                    msgW.TimeInMilliseconds = 50;
                    msgW.Angle = Math.PI / 2;
                    communicator.SendMessage(msgW);
                    break;
                case Key.A:
                    MessageToServer msgS = new MessageToServer();
                    msgS.MessageType = MessageType.Move;
                    msgS.PlayerID = playerID;
                    msgS.TeamID = teamID;
                    msgS.TimeInMilliseconds = 50;
                    msgS.Angle = 3 * Math.PI / 2;
                    communicator.SendMessage(msgS);
                    break;
                case Key.J:
                    MessageToServer msgJ = new MessageToServer();
                    msgJ.MessageType = MessageType.Attack;
                    msgJ.PlayerID = playerID;
                    msgJ.TeamID = teamID;
                    msgJ.Angle = Math.PI;
                    communicator.SendMessage(msgJ);
                    break;
                case Key.U:
                    MessageToServer msgU = new MessageToServer();
                    msgU.MessageType = MessageType.UseCommonSkill;
                    msgU.PlayerID = playerID;
                    msgU.TeamID = teamID;
                    communicator.SendMessage(msgU);
                    break;
                case Key.K:
                    MessageToServer msgK = new MessageToServer();
                    msgK.MessageType = MessageType.UseGem;
                    msgK.PlayerID = playerID;
                    msgK.TeamID = teamID;
                    communicator.SendMessage(msgK);
                    break;
                case Key.L:
                    MessageToServer msgL = new MessageToServer();
                    msgL.MessageType = MessageType.ThrowGem;
                    msgL.PlayerID = playerID;
                    msgL.TeamID = teamID;
                    msgL.GemSize = 1;
                    msgL.TimeInMilliseconds = 3000;
                    msgL.Angle = Math.PI;
                    communicator.SendMessage(msgL);
                    break;
                case Key.P:
                    MessageToServer msgP = new MessageToServer();
                    msgP.MessageType = MessageType.Pick;
                    msgP.PlayerID = playerID;
                    msgP.TeamID = teamID;
                    msgP.PropType = Communication.Proto.PropType.Gem;
                    communicator.SendMessage(msgP);
                    break;
                default:
                    break;
            }
        }
        private void OnReceive()
        {
            if (communicator.TryTake(out IGameMessage msg) && msg.PacketType == PacketType.MessageToClient)
            {
                lock (drawPicLock)  //加锁是必要的，画图操作和接收信息操作不能同时进行，否则画图时foreach会有bug
                {
                    playerData.Clear();
                    propData.Clear();
                    bulletData.Clear();
                    MessageToClient content = (MessageToClient)msg.Content;
                    switch (content.MessageType)
                    {
                        case MessageType.InitialLized:
                            break;
                        case MessageType.StartGame:
                            foreach (MessageToClient.Types.GameObjMessage obj in content.GameObjMessage)
                            {   
                                switch (obj.ObjCase)
                                {
                                    case MessageToClient.Types.GameObjMessage.ObjOneofCase.MessageOfCharacter:
                                        if (obj.MessageOfCharacter.PlayerID == playerID && obj.MessageOfCharacter.TeamID == teamID)
                                            myInfo = obj;
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
                                        if (obj.MessageOfCharacter.PlayerID == playerID && obj.MessageOfCharacter.TeamID == teamID)
                                            myInfo = obj;
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
                            break;
                    }
                }
            }
        }
        private bool CanSee(MessageOfCharacter msg)
        {
            if (myInfo != null)
            {
                if (myInfo.MessageOfCharacter.Guid == msg.Guid) //自己能看见自己
                    return true;
            }
            if (msg.IsInvisible)
                return false;
            if (msg.Place == PlaceType.Land)
                return true;
            if (myInfo != null)
            {
                if (msg.Place != myInfo.MessageOfCharacter.Place)
                    return false;
            }
            return true;
        }
        private bool CanSee(MessageOfBullet msg)
        {
            if (msg.Place == PlaceType.Land)
                return true;
            if (myInfo != null)
            {
                if (msg.Place != myInfo.MessageOfCharacter.Place)
                    return false;
            }
            return true;
        }
        private bool CanSee(MessageOfProp msg)
        {
            if (msg.Place == PlaceType.Land)
                return true;
            if (myInfo != null)
            {
                if (msg.Place != myInfo.MessageOfCharacter.Place)
                    return false;
            }
            return true;
        }

        //定时器事件，刷新地图
        private void Refresh(object? sender, EventArgs e)
        {
            if (!isClientStocked)
            {
                try
                {
                    UpperLayerOfMap.Children.Clear();
                    if (!communicator.Client.IsConnected)
                    {
                        throw new Exception("Client is unconnected.");
                    }
                    else
                    {
                        lock (drawPicLock) //加锁是必要的，画图操作和接收信息操作不能同时进行
                        {
                            foreach (var data in playerData)
                            {
                                if (CanSee(data.MessageOfCharacter))
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
                            }
                            foreach (var data in bulletData)
                            {
                                if (CanSee(data.MessageOfBullet))
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
                            }
                            foreach (var data in propData)
                            {
                                if (CanSee(data.MessageOfProp))
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
                        }
                    }
                }
                catch (Exception exc)
                {
                    ErrorDisplayer error = new("发生错误。以下是系统报告\n" + exc.ToString());
                    error.Show();
                    isClientStocked = true;
                    PorC.Content = "\\xfgg/";
                }
            }
        }
        //定时器事件，刷新地图
        //以下为Mainwindow自定义属性
        private DispatcherTimer timer;//定时器
        private ClientCommunication communicator;

        private bool isClientStocked;

        private Int64 playerID;
        private Int64 teamID;

        private List<MessageToClient.Types.GameObjMessage> playerData;
        private List<MessageToClient.Types.GameObjMessage> bulletData;
        private List<MessageToClient.Types.GameObjMessage> propData;
        private object drawPicLock = new object();
        private MessageToClient.Types.GameObjMessage? myInfo;  //这个client自己的message

        private Stack<string>? myMessages;

        /// <summary>
        /// 50*50
        /// 1:Wall; 2:Grass1; 3:Grass2 ; 4:Grass3 
        /// </summary>
        public static uint[,] defaultMap = new uint[,]
        {
            {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 2, 2, 2, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 2, 2, 2, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 2, 2, 2, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 2, 2, 2, 0, 0, 0, 1, 0, 0, 0, 0, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 2, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 13, 13, 13, 13, 13, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 13, 13, 13, 13, 13, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 13, 13, 13, 13, 13, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 13, 13, 13, 13, 13, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 13, 13, 13, 13, 13, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 4, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 4, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 4, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 4, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 9, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 11, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
         };

    }
}
//2021-10-23
//目前没有画图。并且，Client端能够开始游戏，但不能停止游戏，也不会收到游戏停止的消息。加上该功能后记得游戏停止时把Begin钮变红,isGameRunning置false.
//2021-10-25
//调整了一些提示出现的逻辑，并且修改了计时器，使得Error弹窗不再频繁弹出。
//2021-11-22
//更改显示方式，现在会在底层图层加载地图；为更新侧边栏人物信息和鼠标点击攻击做好了准备，但缺少ID属性；报错现在会显示时间，程序不再检测communicator是否为空，而是检测是否已连接；
//放弃了log窗口设计；可以注册主动技能和被动技能，主动技能2号暂不采用；messageToServer弃用，改为包装后立刻发送。。
//下一步的更新目标:引入生成地图机制；更新侧边栏人物信息和鼠标点击攻击；快捷键；未完成的按钮。
