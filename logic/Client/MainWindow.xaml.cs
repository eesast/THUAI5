﻿using System;
using System.Collections.Generic;
using System.Windows.Controls;
using System.Windows;
using System.Windows.Threading;
using System.Diagnostics;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Shapes;
using System.Threading;
using System.IO;
using Communication.ClientCommunication;
using Communication.Proto;
using CommandLine;
//注：把13改成upperlayer.length/50
namespace Client
{
    /// <summary>
    /// MainWindow.xaml 的交互逻辑
    /// </summary>
    public partial class MainWindow : Window
    {
        public MainWindow()
        {
            unitHeight = unitWidth = unit = 13;
            bonusflag = true;
            timer = new DispatcherTimer
            {
                Interval = new TimeSpan(50000)//每50ms刷新一次
            };
            timer.Tick += new EventHandler(Refresh);    //定时器初始化
            InitializeComponent();
            timer.Start();
            SetStatusBar();
            isClientStocked = true;
            isPlaybackMode = false;
            drawPicLock = new();
            dataDict = new Dictionary<GameObjType, List<MessageToClient.Types.GameObjMessage>>();
            dataDict.Add(GameObjType.Character, new List<MessageToClient.Types.GameObjMessage>());
            dataDict.Add(GameObjType.Bullet, new List<MessageToClient.Types.GameObjMessage>());
            dataDict.Add(GameObjType.Prop, new List<MessageToClient.Types.GameObjMessage>());
            dataDict.Add(GameObjType.BombedBullet, new List<MessageToClient.Types.GameObjMessage>());
            WindowStartupLocation = WindowStartupLocation.CenterScreen;
            ReactToCommandline();
        }
        private void ReactToCommandline()
        {
            string[] args = Environment.GetCommandLineArgs();
            if(args.Length == 2)
            {
                Playback(args[1]);
                return;
            }
            _ = Parser.Default.ParseArguments<ArgumentOptions>(args).WithParsed(o => { options = o; });
            if (options == null || options.cl == false)
            {
                communicator = new ClientCommunication();
                communicator.OnReceive += OnReceive;
            }
            else
            {
                if (options.PlaybackFile == DefaultArgumentOptions.FileName)
                {
                    try
                    {
                        communicator = new ClientCommunication();
                        communicator.OnReceive += OnReceive;
                        string[] comInfo = new string[6];
                        comInfo[0] = options.Ip;
                        comInfo[1] = options.Port;
                        comInfo[2] = options.PlayerID;
                        comInfo[3] = options.TeamID;
                        comInfo[4] = options.Hardware;
                        comInfo[5] = options.Software;
                        ConnectToServer(comInfo);
                    }
                    catch
                    {
                        communicator = new ClientCommunication();
                        communicator.OnReceive += OnReceive;
                    }
                }
                else
                {
                    Playback(options.PlaybackFile, options.PlaybackSpeed);
                }
                
            }
        }
        private void Playback(string fileName, double pbSpeed = 2.0)
        {
            var pbClient = new PlaybackClient(fileName, pbSpeed);
            int[,]? map;
            if ((map = pbClient.ReadDataFromFile(dataDict, drawPicLock)) != null)
            {
                isClientStocked = false;
                isPlaybackMode = true;
                defaultMap = map;
                mapFlag = true;        
            }
            else
            {
                MessageBox.Show("Failed to read the playback file!");
                isClientStocked = true;
            }
        }
        private void SetStatusBar()
        {
            StatusBars = new StatusBar[8];
            for (int i=0;i<8;i++)
            {
                StatusBars[i] = new(MainGrid,i/2+1,i%2);
            }
        }
        //基础窗口函数
        private void ClickToClose(object sender, RoutedEventArgs e)
        {  
            Application.Current.Shutdown();
        }
        private void DrawLaser(Point source, double theta, double range,double Width)//三个参数分别为攻击者的位置，攻击方位角（窗口坐标）和攻击半径
        {
            Point[] endPoint = new Point[4];
            Point target = new();
            target.X = source.X + range * Math.Cos(theta);
            target.Y = source.Y + range * Math.Sin(theta);
            endPoint[0].X = source.X + Width * Math.Cos(theta - Math.PI / 2);
            endPoint[0].Y = source.Y + Width * Math.Sin(theta - Math.PI / 2);
            endPoint[1].X = target.X + Width * Math.Cos(theta - Math.PI / 2);
            endPoint[1].Y = target.Y + Width * Math.Sin(theta - Math.PI / 2);
            endPoint[2].X = target.X + Width * Math.Cos(theta + Math.PI / 2);
            endPoint[2].Y = target.Y + Width * Math.Sin(theta + Math.PI / 2);
            endPoint[3].X = source.X + Width * Math.Cos(theta + Math.PI / 2);
            endPoint[3].Y = source.Y + Width * Math.Sin(theta + Math.PI / 2);
            Polygon laserIcon = new();
            laserIcon.Stroke = System.Windows.Media.Brushes.Red;
            laserIcon.Fill = System.Windows.Media.Brushes.Red;
            laserIcon.StrokeThickness = 2;
            laserIcon.HorizontalAlignment = HorizontalAlignment.Left;
            laserIcon.VerticalAlignment = VerticalAlignment.Top;
            PointCollection laserEndPoints = new();
            for (int i = 0; i < 4; i++)
            {
                laserEndPoints.Add(endPoint[i]);
            }
            laserIcon.Points = laserEndPoints;
            UpperLayerOfMap.Children.Add(laserIcon);
        }
        private void DrawProp(MessageToClient.Types.GameObjMessage data, string text)
        {
            TextBox icon = new()
            {
                FontSize=10,
                Width = 20,
                Height = 20,
                Text = text,
                HorizontalAlignment = HorizontalAlignment.Left,
                VerticalAlignment = VerticalAlignment.Top,
                Margin = new Thickness(data.MessageOfProp.Y * unitWidth / 1000.0 - unitWidth / 2, data.MessageOfProp.X * unitHeight / 1000.0 - unitHeight / 2, 0, 0),
                Background = Brushes.Transparent,
                BorderBrush=Brushes.Transparent,
                IsReadOnly = true
            };
            UpperLayerOfMap.Children.Add(icon);
        }
        private void ClickToMinimize(object sender, RoutedEventArgs e)
        {
            WindowState = WindowState.Minimized;
        }
        private void ClickToMaxmize(object sender, RoutedEventArgs e)
        {
            if (WindowState != WindowState.Maximized)
                WindowState = WindowState.Maximized;
            else WindowState = WindowState.Normal;
        }
        private void DragWindow(object sender, RoutedEventArgs e)
        {
            DragMove();          
        }

        private void Attack(object sender,RoutedEventArgs e)
        {
            if (!isPlaybackMode)
            {
                if (communicator.Client.IsConnected && myInfo != null)
                {
                    MessageToServer msgJ = new()
                    {
                        MessageType = MessageType.Attack,
                        PlayerID = playerID,
                        TeamID = teamID
                    };
                    double mouseY = Mouse.GetPosition(UpperLayerOfMap).X * 1000 / unitWidth;
                    double mouseX = Mouse.GetPosition(UpperLayerOfMap).Y * 1000 / unitHeight;
                    msgJ.Angle = Math.Atan2(mouseY - myInfo.MessageOfCharacter.Y, mouseX - myInfo.MessageOfCharacter.X);
                    communicator.SendMessage(msgJ);
                }
            }
        }
        private void ZoomMap()
        {
            for (int i = 0; i < 50; i++)
            {
                for (int j = 0; j < 50; j++)
                {
                    if (mapPatches[i, j] != null)
                    {
                        mapPatches[i, j].Width = UpperLayerOfMap.ActualWidth / 50;
                        mapPatches[i, j].Height = UpperLayerOfMap.ActualHeight / 50;
                        mapPatches[i, j].HorizontalAlignment = HorizontalAlignment.Left;
                        mapPatches[i, j].VerticalAlignment = VerticalAlignment.Top;
                        mapPatches[i, j].Margin = new Thickness(UpperLayerOfMap.ActualWidth / 50 * j,
                                                                UpperLayerOfMap.ActualHeight / 50 * i,
                                                                0,
                                                                0);
                    }
                }
            }
        }
        private void DrawMap()
        {
            for (int i = 0; i < defaultMap.GetLength(0); i++)
            {
                for (int j = 0; j < defaultMap.GetLength(1); j++)
                {
                    mapPatches[i,j] = new()
                    {
                        Width = unitWidth,
                        Height = unitHeight,
                        HorizontalAlignment = HorizontalAlignment.Left,
                        VerticalAlignment = VerticalAlignment.Top,
                        Margin = new Thickness(Width * (j ), Height * (i ), 0, 0)
                    };
                    switch (defaultMap[i, j])
                    {
                        case 1:
                            mapPatches[i, j].Fill = Brushes.Brown;
                            mapPatches[i, j].Stroke = Brushes.Brown;
                            break;
                        case 2:
                        case 3:
                        case 4:
                            mapPatches[i, j].Fill = Brushes.Green;
                            mapPatches[i, j].Stroke = Brushes.Green;
                            break;
                        case 5:
                        case 6:
                        case 7:
                        case 8:
                        case 9:
                        case 10:
                        case 11:
                        case 12:
                            mapPatches[i, j].Fill = Brushes.Yellow;
                            mapPatches[i, j].Stroke = Brushes.Yellow;
                            break;
                        case 13:
                            mapPatches[i, j].Fill = Brushes.LightPink;
                            mapPatches[i, j].Stroke= Brushes.LightPink;
                            break;

                    }
                    UnderLayerOfMap.Children.Add(mapPatches[i, j]);
                }
            }
            hasDrawed = true;
        }
        //Client控制函数

        private void ClickToPauseOrContinue(object sender, RoutedEventArgs e)
        {
            if (!isClientStocked)
            {
                isClientStocked = true;
                PorC.Content = "▶";
            }
            else if(!isPlaybackMode)
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
            //try
            //{
            //    if (!File.Exists("VSRoute.txt"))
            //    {
            //        File.Create("VSRoute.txt");
            //        Exception ex = new("没有路径存储文件，已为您创建。请将VS路径输入该文件，并重新操作。");
            //        throw ex;
            //    }//创建路径文件 
            //    using StreamReader sr = new("VSRoute.txt");
            //    _ = Process.Start(sr.ReadLine());
            //}
            //catch (Exception exc)
            //{
            //    ErrorDisplayer error = new("发生错误。以下是系统报告:\n" + exc.ToString());
            //    error.Show();
            //}
            PleaseWait();
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
        private void ConnectToServer(string[] comInfo)
        {
            if (!isPlaybackMode)
            {
                if (comInfo.Length != 6)
                    throw new Exception("注册信息有误！");
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
                    //建立连接的同时加入人物
                }
            }
        }
        private void ClickToConnect(object sender, RoutedEventArgs e)
        {
            if (!isPlaybackMode)
            {
                if (!communicator.Client.IsConnected)
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
                            ConnectToServer(comInfo);
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
                    //_ = communicator.Stop();
                    //Connect.Background = Brushes.Aqua;
                    MessageBox.Show("您已连接服务器！！");
                }
            }
        }

        private void KeyBoardControl(object sender, KeyEventArgs e)
        {
            if(!isPlaybackMode)
            {
                if (communicator.Client.IsConnected)
                {
                    switch (e.Key)
                    {
                        case Key.W:
                        case Key.NumPad8:
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
                        case Key.NumPad2:
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
                        case Key.NumPad6:
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
                        case Key.NumPad4:
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
                        case Key.O:
                            MessageToServer msgO = new()
                            {
                                MessageType = MessageType.Pick,
                                PlayerID = playerID,
                                TeamID = teamID,
                            };
                            communicator.SendMessage(msgO);
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
                        case Key.I:
                            MessageToServer msgI = new()
                            {
                                MessageType = MessageType.UseProp,
                                PlayerID = playerID,
                                TeamID = teamID
                            };
                            communicator.SendMessage(msgI);
                            break;
                        case Key.Y:
                            MessageToServer msgY = new()
                            {
                                MessageType = MessageType.ThrowProp,
                                PlayerID = playerID,
                                TeamID = teamID,
                                TimeInMilliseconds = 3000,
                                Angle = Math.PI
                            };
                            communicator.SendMessage(msgY);
                            break;
                        case Key.NumPad7:
                            MessageToServer msg7 = new()
                            {
                                MessageType = MessageType.Move,
                                PlayerID = playerID,
                                TeamID = teamID,
                                TimeInMilliseconds = 50,
                                Angle = 5 * Math.PI / 4
                            };
                            communicator.SendMessage(msg7);
                            break;
                        case Key.NumPad9:
                            MessageToServer msg9 = new()
                            {
                                MessageType = MessageType.Move,
                                PlayerID = playerID,
                                TeamID = teamID,
                                TimeInMilliseconds = 50,
                                Angle = 3 * Math.PI / 4
                            };
                            communicator.SendMessage(msg9);
                            break;
                        case Key.NumPad3:
                            MessageToServer msg3 = new()
                            {
                                MessageType = MessageType.Move,
                                PlayerID = playerID,
                                TeamID = teamID,
                                TimeInMilliseconds = 50,
                                Angle = Math.PI / 4
                            };
                            communicator.SendMessage(msg3);
                            break;
                        case Key.NumPad1:
                            MessageToServer msg1 = new()
                            {
                                MessageType = MessageType.Move,
                                PlayerID = playerID,
                                TeamID = teamID,
                                TimeInMilliseconds = 50,
                                Angle = 7 * Math.PI / 4
                            };
                            communicator.SendMessage(msg1);
                            break;
                        default:
                            break;
                    }
                }
            }
        }
        private void GetMap(MessageToClient.Types.GameObjMessage obj)
        {
            int[,] map = new int[50, 50];
            try
            {
                for (int i = 0; i < 50; i++)
                {
                    for (int j = 0; j < 50; j++)
                    {
                        map[i, j] = obj.MessageOfMap.Row[i].Col[j];
                    }
                }
            }
            catch
            {
                mapFlag = false;
            }
            finally
            {
                defaultMap = map;
                mapFlag = true;
            }
        }
        private void OnReceive()
        {
            if (communicator.TryTake(out IGameMessage msg) && msg.PacketType == PacketType.MessageToClient)
            {
                lock (drawPicLock)  //加锁是必要的，画图操作和接收信息操作不能同时进行，否则画图时foreach会有bug
                {
                    dataDict[GameObjType.Character].Clear();
                    dataDict[GameObjType.Prop].Clear();
                    dataDict[GameObjType.Bullet].Clear();
                    dataDict[GameObjType.BombedBullet].Clear();
                    MessageToClient content = (MessageToClient)msg.Content;
                    switch (content.MessageType)
                    {
                        case MessageType.StartGame:
                            foreach (MessageToClient.Types.GameObjMessage obj in content.GameObjMessage)
                            {
                                switch (obj.ObjCase)
                                {
                                    case MessageToClient.Types.GameObjMessage.ObjOneofCase.MessageOfCharacter:
                                        if (obj.MessageOfCharacter.PlayerID == playerID && obj.MessageOfCharacter.TeamID == teamID)
                                        {
                                            myInfo = obj;
                                        }
                                        dataDict[GameObjType.Character].Add(obj);
                                        break;
                                    case MessageToClient.Types.GameObjMessage.ObjOneofCase.MessageOfBullet:
                                        dataDict[GameObjType.Bullet].Add(obj);
                                        break;
                                    case MessageToClient.Types.GameObjMessage.ObjOneofCase.MessageOfProp:
                                        dataDict[GameObjType.Prop].Add(obj);
                                        break;
                                    case MessageToClient.Types.GameObjMessage.ObjOneofCase.MessageOfBombedBullet:
                                        dataDict[GameObjType.BombedBullet].Add(obj);
                                        break;
                                    case MessageToClient.Types.GameObjMessage.ObjOneofCase.MessageOfMap:
                                        GetMap(obj);
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
                                        {
                                            myInfo = obj;
                                        }
                                        dataDict[GameObjType.Character].Add(obj);
                                        break;
                                    case MessageToClient.Types.GameObjMessage.ObjOneofCase.MessageOfBullet:
                                        dataDict[GameObjType.Bullet].Add(obj);
                                        break;
                                    case MessageToClient.Types.GameObjMessage.ObjOneofCase.MessageOfProp:
                                        dataDict[GameObjType.Prop].Add(obj);
                                        break;
                                    case MessageToClient.Types.GameObjMessage.ObjOneofCase.MessageOfBombedBullet:
                                        dataDict[GameObjType.BombedBullet].Add(obj);
                                        break;
                                    case MessageToClient.Types.GameObjMessage.ObjOneofCase.MessageOfMap:
                                        if (!mapFlag)
                                            GetMap(obj);
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
                                        dataDict[GameObjType.Character].Add(obj);
                                        break;
                                    case MessageToClient.Types.GameObjMessage.ObjOneofCase.MessageOfBullet:
                                        dataDict[GameObjType.Bullet].Add(obj);
                                        break;
                                    case MessageToClient.Types.GameObjMessage.ObjOneofCase.MessageOfProp:
                                        dataDict[GameObjType.Prop].Add(obj);
                                        break;
                                    case MessageToClient.Types.GameObjMessage.ObjOneofCase.MessageOfBombedBullet:
                                        dataDict[GameObjType.BombedBullet].Add(obj);
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
            if (msg.IsResetting)
                return false;
            if (playerID >= 2022 || teamID >= 2022)
                return true;
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
            Bonus();
            if (WindowState == WindowState.Maximized)
                MaxButton.Content = "❐";
            else
                MaxButton.Content = "🗖";
            if (StatusBars != null)
                for (int i = 0; i < 8; i++)
                {
                    StatusBars[i].SetFontSize(12 * UpperLayerOfMap.ActualHeight / 650);
                }
            
            //完成窗口信息更新
            if (!isClientStocked)
            {
                unit = Math.Sqrt(UpperLayerOfMap.ActualHeight * UpperLayerOfMap.ActualWidth) / 50;
                unitHeight = UpperLayerOfMap.ActualHeight/50;
                unitWidth = UpperLayerOfMap.ActualWidth/50;
                try
                {
                    if (log != null)
                    {
                        string temp = "";
                        for (int i = 0; i < dataDict[GameObjType.Character].Count; i++)
                        {
                            temp += Convert.ToString(dataDict[GameObjType.Character][i].MessageOfCharacter.TeamID) + "\n";
                        }
                        log.Content = temp; 
                    }
                    UpperLayerOfMap.Children.Clear();
                    if ((communicator == null || !communicator.Client.IsConnected) && !isPlaybackMode)
                    {
                        UnderLayerOfMap.Children.Clear();
                        throw new Exception("Client is unconnected.");
                    }
                    else
                    {
                        lock (drawPicLock) //加锁是必要的，画图操作和接收信息操作不能同时进行
                        {
                            if (!hasDrawed && mapFlag)
                                DrawMap();
                            foreach (var data in dataDict[GameObjType.Character])
                            {
                                StatusBars[data.MessageOfCharacter.TeamID * 4 + data.MessageOfCharacter.PlayerID].SetValue(data.MessageOfCharacter);
                                if (CanSee(data.MessageOfCharacter))
                                {
                                    Ellipse icon = new()
                                    {
                                        Width = unitWidth,
                                        Height = unitHeight,
                                        HorizontalAlignment = HorizontalAlignment.Left,
                                        VerticalAlignment = VerticalAlignment.Top,
                                        Margin = new Thickness(data.MessageOfCharacter.Y * unitWidth / 1000.0- unitWidth/2, data.MessageOfCharacter.X * unitHeight / 1000.0- unitHeight/2, 0, 0),
                                    };
                                    if (data.MessageOfCharacter.TeamID == 0)
                                        icon.Fill = Brushes.Black;
                                    else if (data.MessageOfCharacter.TeamID == 1)
                                        icon.Fill = Brushes.BlueViolet;
                                    else if (data.MessageOfCharacter.TeamID == 2)
                                        icon.Fill = Brushes.DarkOrange;
                                    else icon.Fill = Brushes.Cyan;
                                    UpperLayerOfMap.Children.Add(icon);
                                }
                            }
                            foreach (var data in dataDict[GameObjType.Bullet])
                            {
                                if (CanSee(data.MessageOfBullet))
                                {
                                    Ellipse icon = new()
                                    {
                                        Width = 10,
                                        Height = 10,
                                        HorizontalAlignment = HorizontalAlignment.Left,
                                        VerticalAlignment = VerticalAlignment.Top,
                                        Margin = new Thickness(data.MessageOfBullet.Y * unitWidth / 1000.0-unitWidth / 2, data.MessageOfBullet.X * unitHeight / 1000.0-unitHeight / 2, 0, 0),
                                        Fill = Brushes.Red
                                    };
                                    UpperLayerOfMap.Children.Add(icon);
                                }
                            }
                            foreach (var data in dataDict[GameObjType.Prop])
                            {
                                if (CanSee(data.MessageOfProp))
                                {
                                    switch (data.MessageOfProp.Type)
                                    {
                                        case PropType.Gem:
                                            DrawProp(data, "📱");
                                            break;
                                        case PropType.Shield:
                                            DrawProp(data, "🛡");
                                            break;
                                        case PropType.Spear:
                                            DrawProp(data, "🗡");
                                            break;
                                        case PropType.AddSpeed:
                                            DrawProp(data, "⛸");
                                            break;
                                        case PropType.AddLife:
                                            DrawProp(data, "♥");
                                            break;
                                        default:
                                            DrawProp(data, "♨");
                                            break;
                                    }
                                }
                            }
                            foreach(var data in dataDict[GameObjType.BombedBullet])
                            {
                                switch(data.MessageOfBombedBullet.Type)
                                {
                                    case BulletType.FastBullet:
                                        {
                                            Ellipse icon = new();
                                            double bombRange = data.MessageOfBombedBullet.BombRange / 1000;
                                            icon.Width = bombRange * unitWidth;
                                            icon.Height = bombRange * unitHeight;
                                            icon.HorizontalAlignment = HorizontalAlignment.Left;
                                            icon.VerticalAlignment = VerticalAlignment.Top;
                                            icon.Margin = new Thickness(data.MessageOfBombedBullet.Y * unitWidth / 1000.0 - bombRange * unitWidth / 2, data.MessageOfBombedBullet.X * unitHeight / 1000.0 - bombRange * unitHeight / 2, 0, 0);
                                            icon.Fill = Brushes.Red;
                                            UpperLayerOfMap.Children.Add(icon);
                                            break;
                                        }
                                    case BulletType.AtomBomb:
                                        {
                                            Ellipse icon = new Ellipse();
                                            double bombRange = data.MessageOfBombedBullet.BombRange / 1000;
                                            icon.Width = bombRange * unitWidth;
                                            icon.Height = bombRange * unitHeight;
                                            icon.HorizontalAlignment = HorizontalAlignment.Left;
                                            icon.VerticalAlignment = VerticalAlignment.Top;
                                            icon.Margin = new Thickness(data.MessageOfBombedBullet.Y * unitWidth / 1000.0 - bombRange * unitWidth / 2, data.MessageOfBombedBullet.X * unitHeight / 1000.0 - bombRange * unitHeight / 2, 0, 0);
                                            icon.Fill = Brushes.Red;
                                            UpperLayerOfMap.Children.Add(icon);
                                            break;
                                        }
                                    case BulletType.OrdinaryBullet:
                                        {
                                            Ellipse icon = new Ellipse();
                                            double bombRange = data.MessageOfBombedBullet.BombRange / 1000;
                                            icon.Width = bombRange * unitWidth;
                                            icon.Height = bombRange * unitHeight;
                                            icon.HorizontalAlignment = HorizontalAlignment.Left;
                                            icon.VerticalAlignment = VerticalAlignment.Top;
                                            icon.Margin = new Thickness(data.MessageOfBombedBullet.Y * unitWidth / 1000.0 - bombRange * unitWidth / 2, data.MessageOfBombedBullet.X * unitHeight / 1000.0 - bombRange * unitHeight / 2, 0, 0);
                                            icon.Fill = Brushes.Red;
                                            UpperLayerOfMap.Children.Add(icon);
                                            break;
                                        }
                                    case BulletType.LineBullet:
                                        {
                                            double bombRange = data.MessageOfBombedBullet.BombRange / 1000;
                                            DrawLaser(new Point(data.MessageOfBombedBullet.Y * unitWidth / 1000.0, data.MessageOfBombedBullet.X * unitHeight / 1000.0), -data.MessageOfBombedBullet.FacingDirection + Math.PI/2, bombRange * unitHeight, 0.5 * unitWidth);
                                            break;
                                        }
                                    default:
                                        break;
                                }
                            }
                        }
                        ZoomMap();
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
        /// <summary>
        /// 彩蛋
        /// </summary>
        private void Bonus()
        {
            if (bonusflag)
            {
                Random rd = new();
                if (rd.Next(0, 10000) == 2022)
                {
                    for (int i = 0; i < 40; i++)
                    {
                        ErrorDisplayer ex = new("すぐにけせ");
                        ex.WindowStartupLocation = WindowStartupLocation.Manual;
                        ex.Left = rd.Next(300, 1200);
                        ex.Top = rd.Next(200, 600);
                        ex.Show();
                        Thread.Sleep((50 - i) * 10 + 100);
                    }
                    Thread.Sleep(1000);
                    Application.Current.Shutdown();
                }
                else bonusflag = false;
            }
        }
        //以下为Mainwindow自定义属性
        private readonly DispatcherTimer timer;//定时器
        private long counter;//预留的取时间变量
        private StatusBar[] StatusBars;
        private ClientCommunication communicator;
        private readonly Rectangle[,] mapPatches=new Rectangle[50,50];

        private bool isClientStocked;
        private bool isPlaybackMode;

        private long playerID;
        private long teamID;

        private double unit;//显示粗略的大小
        private double unitHeight;
        private double unitWidth;
        private Dictionary<GameObjType, List<MessageToClient.Types.GameObjMessage>> dataDict;
        private object drawPicLock = new object();
        private MessageToClient.Types.GameObjMessage? myInfo = null;  //这个client自己的message
        private ErrorDisplayer log;

        private bool bonusflag;
        /// <summary>
        /// 50*50
        /// 1:Wall; 2:Grass1; 3:Grass2 ; 4:Grass3 
        /// </summary>
        private bool mapFlag = false;
        private bool hasDrawed = false;
        public int[,] defaultMap = new int[,]
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
            {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,2,2,2,2,2,2,2,2,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
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

        ArgumentOptions? options = null;
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
