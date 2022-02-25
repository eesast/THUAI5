using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows;
using System.Windows.Threading;
using System.Diagnostics;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Shapes;
using System.IO;
using Communication.ClientCommunication;
using Communication.Proto;

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
            SetStatusBar();
            DrawMap();
            timer.Start();
            isClientStocked = true;
            drawPicLock = new();
            playerData = new List<MessageToClient.Types.GameObjMessage>();
            bulletData = new List<MessageToClient.Types.GameObjMessage>();
            propData = new List<MessageToClient.Types.GameObjMessage>();
            bombedBulletData = new List<MessageToClient.Types.GameObjMessage>();
            WindowStartupLocation = WindowStartupLocation.CenterScreen;
            communicator = new ClientCommunication();
            communicator.OnReceive += OnReceive;
            //注：队伍用边框区分，人物编号以背景颜色区分
            //角色死亡则对应信息框变灰
            //被动技能和buff在人物编号后用彩色文字注明
        }

        private void SetStatusBar()
        {
            for(int i=0;i<8;i++)
            {
                StatusBars[i] = new(MainGrid,2+80*(i%2),40+171*(i/2),764+78*(i%2),513-169*(i/2));
            }
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
            if (communicator.Client.IsConnected&&myInfo!=null)
            {
                MessageToServer msgJ = new()
                {
                    MessageType = MessageType.Attack,
                    PlayerID = playerID,
                    TeamID = teamID
                };
                double mouseY = Mouse.GetPosition(UpperLayerOfMap).X * 1000 / 13;
                double mouseX = Mouse.GetPosition(UpperLayerOfMap).Y * 1000 / 13;
                msgJ.Angle = Math.Atan2(mouseY - myInfo.MessageOfCharacter.Y, mouseX - myInfo.MessageOfCharacter.X);
                communicator.SendMessage(msgJ);
            }
        }
        private void DrawMap()
        {
            for (int i = 0; i < defaultMap.GetLength(0); i++)
            {
                for (int j = 0; j < defaultMap.GetLength(1); j++)
                {
                    Rectangle rectangle = new()
                    {
                        Width = 13,
                        Height = 13,
                        HorizontalAlignment = HorizontalAlignment.Left,
                        VerticalAlignment = VerticalAlignment.Top,
                        Margin = new Thickness(13 * (j + 0.5), 13 * (i + 0.5), 0, 0)
                    };
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
                        case 5:
                        case 6:
                        case 7:
                        case 8:
                        case 9:
                        case 10:
                        case 11:
                        case 12:
                            rectangle.Fill = Brushes.Yellow;
                            rectangle.Stroke = Brushes.Yellow;
                            break;
                        case 13:
                            rectangle.Fill = Brushes.LightPink;
                            rectangle.Stroke= Brushes.LightPink;
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
                try
                {
                    if (communicator.Client.IsConnected)
                    {
                        isClientStocked = false;
                        PorC.Content = "⏸";
                    }
                    else throw new Exception("Unconnected");
                }
                catch (Exception ex)
                {
                    ErrorDisplayer error = new("发生错误。以下是系统报告:\n" + ex.ToString());
                    error.Show();
                }
            }
        }

        private void ClickToSetMode(object sender, RoutedEventArgs e)
        {
            log = new(" ");
            log.Show();
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
            PleaseWait();
        }

        private void ClickForUpdate(object sender, RoutedEventArgs e)
        {
            PleaseWait();
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
            PleaseWait();
        }
        private void ClickToConnect(object sender, RoutedEventArgs e)
        {
            if (!communicator.Client.IsConnected)//第一次连接失败后第二次连接会出错
            {
                try
                {
                    using (var sr = new StreamReader(".\\ConnectInfo.txt"))
                    {
                        string[] comInfo = sr.ReadLine().Split(' ');
                        if (comInfo[0] == "" ||
                            comInfo[1] == "" ||
                            comInfo[2] == "" ||
                            comInfo[3] == "" ||
                            comInfo[4] == "" ||
                            comInfo[5] == "")
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
                            msg.PSkill = Convert.ToInt64(comInfo[4]) switch
                            {
                                0 => PassiveSkillType.NullPassiveSkillType,
                                1 => PassiveSkillType.RecoverAfterBattle,
                                2 => PassiveSkillType.SpeedUpWhenLeavingGrass,
                                3 => PassiveSkillType.Vampire,
                                4 => PassiveSkillType.Pskill3,
                                5 => PassiveSkillType.Pskill4,
                                _ => PassiveSkillType.Pskill5,
                            };
                            switch (Convert.ToInt64(comInfo[5]))
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
                            Connect.Background = Brushes.Transparent;
                            isClientStocked = false;
                            PorC.Content = "⏸";
                        }//建立连接的同时加入人物
                    }
                }
                catch (Exception exc)
                {
                    if (exc.Message == "Input data not sufficent")
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
                    }
                }
            }
            else
            {
                _ = communicator.Stop();
                Connect.Background = Brushes.Aqua;

            }
        }

        private void KeyBoardControl(object sender, KeyEventArgs e)
        {
            switch (e.Key)
            {
                case Key.W:
                    MessageToServer msgA = new()
                    {
                        MessageType = MessageType.Move,
                        PlayerID = playerID,
                        TeamID = teamID,
                        TimeInMilliseconds = 50,
                        Angle = Math.PI
                    };
                    communicator.SendMessage(msgA);
                    break;
                case Key.S:
                    MessageToServer msgD = new()
                    {
                        MessageType = MessageType.Move,
                        PlayerID = playerID,
                        TeamID = teamID,
                        TimeInMilliseconds = 50,
                        Angle = 0
                    };
                    communicator.SendMessage(msgD);
                    break;
                case Key.D:
                    MessageToServer msgW = new()
                    {
                        MessageType = MessageType.Move,
                        PlayerID = playerID,
                        TeamID = teamID,
                        TimeInMilliseconds = 50,
                        Angle = Math.PI / 2
                    };
                    communicator.SendMessage(msgW);
                    break;
                case Key.A:
                    MessageToServer msgS = new()
                    {
                        MessageType = MessageType.Move,
                        PlayerID = playerID,
                        TeamID = teamID,
                        TimeInMilliseconds = 50,
                        Angle = 3 * Math.PI / 2
                    };
                    communicator.SendMessage(msgS);
                    break;
                case Key.J:
                    MessageToServer msgJ = new()
                    {
                        MessageType = MessageType.Attack,
                        PlayerID = playerID,
                        TeamID = teamID,
                        Angle = Math.PI
                    };
                    communicator.SendMessage(msgJ);
                    break;
                case Key.U:
                    MessageToServer msgU = new()
                    {
                        MessageType = MessageType.UseCommonSkill,
                        PlayerID = playerID,
                        TeamID = teamID
                    };
                    communicator.SendMessage(msgU);
                    break;
                case Key.K:
                    MessageToServer msgK = new()
                    {
                        MessageType = MessageType.UseGem,
                        PlayerID = playerID,
                        TeamID = teamID
                    };
                    communicator.SendMessage(msgK);
                    break;
                case Key.L:
                    MessageToServer msgL = new()
                    {
                        MessageType = MessageType.ThrowGem,
                        PlayerID = playerID,
                        TeamID = teamID,
                        GemSize = 1,
                        TimeInMilliseconds = 3000,
                        Angle = Math.PI
                    };
                    communicator.SendMessage(msgL);
                    break;
                case Key.P:
                    MessageToServer msgP = new()
                    {
                        MessageType = MessageType.Pick,
                        PlayerID = playerID,
                        TeamID = teamID,
                        PropType = Communication.Proto.PropType.Gem
                    };
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
                    bombedBulletData.Clear();
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
                                    case MessageToClient.Types.GameObjMessage.ObjOneofCase.MessageOfBombedBullet:
                                        bombedBulletData.Add(obj);
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
                                    case MessageToClient.Types.GameObjMessage.ObjOneofCase.MessageOfBombedBullet:
                                        bombedBulletData.Add(obj);
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
                                    case MessageToClient.Types.GameObjMessage.ObjOneofCase.MessageOfBombedBullet:
                                        bombedBulletData.Add(obj);
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
                    if (log != null)
                    {
                        string temp = "";
                        for (int i = 0; i < playerData.Count; i++)
                        {
                            temp += Convert.ToString(playerData[i].MessageOfCharacter.TeamID) + "\n";
                        }
                        log.Content = temp; 
                    }
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
                                StatusBars[data.MessageOfCharacter.TeamID * 4 + data.MessageOfCharacter.PlayerID].SetValue(data.MessageOfCharacter);
                                if (CanSee(data.MessageOfCharacter))
                                {
                                    Ellipse icon = new()
                                    {
                                        Width = 13,
                                        Height = 13,
                                        HorizontalAlignment = HorizontalAlignment.Left,
                                        VerticalAlignment = VerticalAlignment.Top,
                                        Margin = new Thickness(data.MessageOfCharacter.Y * 13.0 / 1000.0, data.MessageOfCharacter.X * 13.0 / 1000.0, 0, 0),
                                        Fill = Brushes.Black
                                    };
                                    UpperLayerOfMap.Children.Add(icon);
                                }
                            }
                            foreach (var data in bulletData)
                            {
                                if (CanSee(data.MessageOfBullet))
                                {
                                    Ellipse icon = new()
                                    {
                                        Width = 10,
                                        Height = 10,
                                        HorizontalAlignment = HorizontalAlignment.Left,
                                        VerticalAlignment = VerticalAlignment.Top,
                                        Margin = new Thickness(data.MessageOfBullet.Y * 13.0 / 1000.0, data.MessageOfBullet.X * 13.0 / 1000.0, 0, 0),
                                        Fill = Brushes.Red
                                    };
                                    UpperLayerOfMap.Children.Add(icon);
                                }
                            }
                            foreach (var data in propData)
                            {
                                if (CanSee(data.MessageOfProp))
                                {
                                    if (data.MessageOfProp.Type == PropType.Gem)
                                    {
                                        Ellipse icon = new()
                                        {
                                            Width = 10,
                                            Height = 10,
                                            HorizontalAlignment = HorizontalAlignment.Left,
                                            VerticalAlignment = VerticalAlignment.Top,
                                            Margin = new Thickness(data.MessageOfProp.Y * 13.0 / 1000.0, data.MessageOfProp.X * 13.0 / 1000.0, 0, 0),
                                            Fill = Brushes.Purple
                                        };
                                        UpperLayerOfMap.Children.Add(icon);
                                    }
                                    else
                                    {
                                        Ellipse icon = new()
                                        {
                                            Width = 10,
                                            Height = 10,
                                            HorizontalAlignment = HorizontalAlignment.Left,
                                            VerticalAlignment = VerticalAlignment.Top,
                                            Margin = new Thickness(data.MessageOfProp.Y * 13.0 / 1000.0, data.MessageOfProp.X * 13.0 / 1000.0, 0, 0),
                                            Fill = Brushes.Gray
                                        };
                                        UpperLayerOfMap.Children.Add(icon);
                                    }
                                }
                            }
                            foreach(var data in bombedBulletData)
                            {
                                switch(data.MessageOfBombedBullet.Type)
                                {
                                    case BulletType.FastBullet:
                                        {
                                            Ellipse icon = new Ellipse();
                                            icon.Width = 65;
                                            icon.Height = 65;
                                            icon.HorizontalAlignment = HorizontalAlignment.Left;
                                            icon.VerticalAlignment = VerticalAlignment.Top;
                                            icon.Margin = new Thickness(data.MessageOfBombedBullet.Y * 13.0 / 1000.0, data.MessageOfBombedBullet.X * 13.0 / 1000.0, 0, 0);
                                            icon.Fill = Brushes.Red;
                                            UpperLayerOfMap.Children.Add(icon);
                                            break;
                                        }
                                    case BulletType.AtomBomb:
                                        {
                                            Ellipse icon = new Ellipse();
                                            icon.Width = 3 * 65;
                                            icon.Height = 3 * 65;
                                            icon.HorizontalAlignment = HorizontalAlignment.Left;
                                            icon.VerticalAlignment = VerticalAlignment.Top;
                                            icon.Margin = new Thickness(data.MessageOfBombedBullet.Y * 13.0 / 1000.0, data.MessageOfBombedBullet.X * 13.0 / 1000.0, 0, 0);
                                            icon.Fill = Brushes.Red;
                                            UpperLayerOfMap.Children.Add(icon);
                                            break;
                                        }
                                    case BulletType.OrdinaryBullet:
                                        {
                                            Ellipse icon = new Ellipse();
                                            icon.Width = 65;
                                            icon.Height = 65;
                                            icon.HorizontalAlignment = HorizontalAlignment.Left;
                                            icon.VerticalAlignment = VerticalAlignment.Top;
                                            icon.Margin = new Thickness(data.MessageOfBombedBullet.Y * 13.0 / 1000.0, data.MessageOfBombedBullet.X * 13.0 / 1000.0, 0, 0);
                                            icon.Fill = Brushes.Red;
                                            UpperLayerOfMap.Children.Add(icon);
                                            break;
                                        }
                                    case BulletType.LineBullet:
                                        {
                                            break;
                                        }
                                    default:
                                        {
                                            Ellipse icon = new Ellipse();
                                            icon.Width = 65;
                                            icon.Height = 65;
                                            icon.HorizontalAlignment = HorizontalAlignment.Left;
                                            icon.VerticalAlignment = VerticalAlignment.Top;
                                            icon.Margin = new Thickness(data.MessageOfBombedBullet.Y * 13.0 / 1000.0, data.MessageOfBombedBullet.X * 13.0 / 1000.0, 0, 0);
                                            icon.Fill = Brushes.Red;
                                            UpperLayerOfMap.Children.Add(icon);
                                            break;
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
                    PorC.Content = "▶";
                }
            }
            counter++;
        }
        //定时器事件，刷新地图
        /// <summary>
        ///敬请期待函数
        /// </summary>
        private void PleaseWait()
        {
            try
            {
                throw new Exception("敬请期待");
            }
            catch(Exception exc)    
            {
                ErrorDisplayer error = new(exc.Message);
                error.Show();
            }
        }
        //以下为Mainwindow自定义属性
        private readonly DispatcherTimer timer;//定时器
        private long counter;//预留的取时间变量
        private readonly StatusBar[] StatusBars = new StatusBar[8];
        private readonly ClientCommunication communicator;

        private bool isClientStocked;

        private long playerID;
        private long teamID;


        private List<MessageToClient.Types.GameObjMessage> playerData;
        private List<MessageToClient.Types.GameObjMessage> bulletData;
        private List<MessageToClient.Types.GameObjMessage> propData;
        private List<MessageToClient.Types.GameObjMessage> bombedBulletData;
        private object drawPicLock = new object();
        private MessageToClient.Types.GameObjMessage? myInfo = null;  //这个client自己的message
        private ErrorDisplayer log;

        /// <summary>
        /// 50*50
        /// 1:Wall; 2:Grass1; 3:Grass2 ; 4:Grass3 
        /// </summary>
        public static uint[,] defaultMap = new uint[,]
         {
            {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
            {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
            {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
            {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
            {1,0,0,0,0,0,0,5,0,0,0,0,0,0,0,0,0,0,6,0,0,0,0,0,0,0,0,0,0,7,0,0,0,0,0,0,0,0,0,0,8,0,0,0,0,0,0,0,0,1},
            {1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
            {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
            {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
            {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
            {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,1},
            {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1},
            {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1},
            {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1},
            {1,0,0,0,0,0,0,0,0,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,0,0,0,0,0,0,0,0,1,1,1,1,1,0,1},
            {1,0,0,0,0,0,0,0,0,2,2,2,2,2,2,2,2,2,2,2,2,13,13,13,13,13,13,2,2,2,2,2,2,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
            {1,0,0,0,0,0,0,0,0,2,2,2,2,2,2,2,2,2,2,2,2,13,13,13,13,13,13,2,2,2,2,2,2,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
            {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
            {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,2,2,2,2,2,2,2,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
            {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,2,2,2,2,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
            {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,13,13,13,13,13,13,13,13,13,13,13,13,13,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
            {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,13,13,13,13,13,13,13,13,13,13,13,13,13,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
            {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,13,13,13,13,13,13,13,13,13,13,13,13,13,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
            {1,0,0,0,0,0,0,0,4,4,4,4,4,4,4,4,4,4,13,13,13,13,13,1,1,1,1,13,13,13,13,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
            {1,0,0,0,0,0,0,0,4,0,4,0,4,0,0,0,0,0,13,13,13,13,13,1,1,1,1,13,13,13,13,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
            {1,0,0,0,0,0,0,0,4,0,4,0,4,0,0,0,0,0,13,13,13,13,13,1,1,1,1,13,13,13,13,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
            {1,0,0,0,0,0,0,0,4,0,4,0,4,0,0,0,0,0,13,13,13,13,13,13,13,13,13,13,13,13,13,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
            {1,0,0,0,0,0,0,0,4,0,4,0,4,0,0,0,0,0,13,13,13,13,13,13,13,13,13,13,13,13,13,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
            {1,0,0,0,0,0,0,0,4,0,4,0,4,0,0,0,0,0,13,13,13,13,13,13,13,13,13,13,13,13,13,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
            {1,0,0,0,0,0,0,0,4,0,4,0,4,0,0,0,0,0,13,13,13,13,13,13,13,13,13,13,13,13,13,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
            {1,0,0,0,0,0,0,0,4,4,4,4,4,4,4,4,4,4,13,13,13,13,13,13,13,13,13,13,13,13,13,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
            {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,3,3,3,3,13,0,0,1},
            {1,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,3,3,3,3,3,13,0,0,1},
            {1,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,3,3,3,3,13,0,0,1},
            {1,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,3,3,3,3,13,0,0,1},
            {1,0,1,1,13,13,13,13,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,3,3,3,3,13,0,0,1},
            {1,0,1,1,13,13,13,13,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,3,3,3,3,13,0,0,1},
            {1,0,1,1,13,13,13,13,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,3,3,3,3,13,0,0,1},
            {1,0,1,1,13,13,13,13,13,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,3,3,3,3,13,0,0,1},
            {1,0,0,0,0,0,0,0,0,0,13,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,3,3,3,3,13,0,0,1},
            {1,0,0,0,0,0,0,0,0,0,13,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,3,3,3,3,13,0,0,1},
            {1,0,0,0,0,0,0,0,0,1,13,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,3,3,3,3,13,0,0,1},
            {1,0,0,0,0,0,0,0,1,0,13,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
            {1,0,0,0,0,0,0,1,1,1,13,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
            {1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1 },
            {1,0,0,0,0,0,0,9,0,0,0,0,0,0,0,0,0,0,10,0,0,0,0,0,0,0,0,0,11,0,0,0,0,0,0,0,0,0,0,0,12,0,0,0,0,0,0,0,0,1},
            {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
            {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
            {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
            {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
            {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
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
//2022-1-28
//加入状态栏；加入鼠标双击攻击（测试功能）
