using System;
using System.Threading;
using Preparation.GameData;
using Timothy.FrameRateTask;
using Gaming;
using Communication.Proto;
using GameClass.GameObj;
using System.IO;
using Playback;
using System.Collections.Generic;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;

namespace Server
{
    public class GameServer : ServerBase
    {
        protected readonly Game game;
        private uint spectatorMinTeamID = 2022;
        private uint spectatorMinPlayerID = 2022;
        private List<Tuple<uint, uint>> spectatorList = new List<Tuple<uint, uint>>();
        public override int TeamCount => options.TeamCount;
        protected long[,] communicationToGameID; //通信用的ID映射到游戏内的ID,[i,j]表示team：i，player：j的id。
        private readonly object messageToAllClientsLock = new();
        public static readonly long SendMessageToClientIntervalInMilliseconds = 50;
        private readonly Semaphore endGameInfoSema = new(0, 1);
        private MessageWriter? mwr = null;
        private HttpSender? httpSender;
        public override int GetTeamScore(long teamID)
        {
            return game.GetTeamScore(teamID);
        }
        public override void WaitForGame()
        {
            _ = endGameInfoSema.WaitOne();  //开始等待游戏开始
            mwr?.Dispose();
        }
        private uint GetBirthPointIdx(long teamID, long playerID)       //获取出生点位置
        {
            return (uint)((teamID * options.PlayerCountPerTeam) + playerID);
        }
        protected readonly object addPlayerLock = new();
        private bool AddPlayer(MessageToServer msg)
        {
            if (msg.PlayerID >= spectatorMinPlayerID && msg.TeamID >= spectatorMinTeamID)
            {
                //观战模式
                Tuple<uint, uint> tp = new Tuple<uint, uint>((uint)msg.TeamID, (uint)msg.PlayerID);
                if (!spectatorList.Contains(tp))
                {
                    spectatorList.Add(tp);
                    Console.WriteLine("A new spectator comes to watch this game.");
                }
                return false;
            }
            if (game.GameMap.Timer.IsGaming)  //游戏运行中，不能添加玩家
                return false;
            if (!ValidTeamIDAndPlayerID(msg.TeamID, msg.PlayerID))  //玩家id是否正确
                return false;
            if (communicationToGameID[msg.TeamID, msg.PlayerID] != GameObj.invalidID)  //是否已经添加了该玩家
                return false;

            Preparation.Utility.PassiveSkillType passiveSkill;
            switch (msg.PSkill)
            {
                case PassiveSkillType.Vampire:
                    passiveSkill = Preparation.Utility.PassiveSkillType.Vampire;
                    break;
                case PassiveSkillType.SpeedUpWhenLeavingGrass:
                    passiveSkill = Preparation.Utility.PassiveSkillType.SpeedUpWhenLeavingGrass;
                    break;
                case PassiveSkillType.RecoverAfterBattle:
                    passiveSkill = Preparation.Utility.PassiveSkillType.RecoverAfterBattle;
                    break;
                default:
                    passiveSkill = Preparation.Utility.PassiveSkillType.Null;
                    break;
            }
            Preparation.Utility.ActiveSkillType commonSkill;
            switch (msg.ASkill1)
            {
                case ActiveSkillType.SuperFast:
                    commonSkill = Preparation.Utility.ActiveSkillType.SuperFast;
                    break;
                case ActiveSkillType.NuclearWeapon:
                    commonSkill = Preparation.Utility.ActiveSkillType.NuclearWeapon;
                    break;
                case ActiveSkillType.BecomeVampire:
                    commonSkill = Preparation.Utility.ActiveSkillType.BecomeVampire;
                    break;
                case ActiveSkillType.BecomeAssassin:
                    commonSkill = Preparation.Utility.ActiveSkillType.BecomeAssassin;
                    break;
                default:
                    commonSkill = Preparation.Utility.ActiveSkillType.Null;
                    break;
            }
            lock (addPlayerLock)
            {
                Game.PlayerInitInfo playerInitInfo = new(GetBirthPointIdx(msg.TeamID, msg.PlayerID), msg.TeamID, msg.PlayerID, passiveSkill, commonSkill);
                long newPlayerID = game.AddPlayer(playerInitInfo);
                if (newPlayerID == GameObj.invalidID)
                    return false;
                communicationToGameID[msg.TeamID, msg.PlayerID] = newPlayerID;
            }
            return true;
        }
        private void ReadyToStart(MessageToServer msgRecieve, bool isValid)
        {
            lock (addPlayerLock)
            {
                CheckStart();       //检查是否该开始游戏了
            }
        }
        protected override void OnReceive(MessageToServer msg)
        {
#if DEBUG 
            Console.WriteLine($"Receive message: from teamID {msg.TeamID} , playerID {msg.PlayerID}: {msg.MessageType}, args: {msg.TimeInMilliseconds} {msg.Angle}");
            Console.WriteLine($"The Content is {msg}");
#endif
            if (msg.TimeInMilliseconds < 0)
            {
                if (msg.Angle >= 0.0) msg.Angle -= Math.PI;
                else msg.Angle += Math.PI;
                if (msg.TimeInMilliseconds == int.MinValue) msg.TimeInMilliseconds = int.MaxValue;
                else msg.TimeInMilliseconds = -msg.TimeInMilliseconds;
            }
            if (double.IsNaN(msg.Angle) || double.IsInfinity(msg.Angle))
                msg.Angle = 0.0;
                
            switch (msg.MessageType)
            {
                case MessageType.AddPlayer:
                    ReadyToStart(msg, AddPlayer(msg));
                    break;
                case MessageType.Move:
                    if (ValidTeamIDAndPlayerID(msg.TeamID, msg.PlayerID))
                    {
                        game.MovePlayer(communicationToGameID[msg.TeamID, msg.PlayerID], (int)msg.TimeInMilliseconds, msg.Angle);
                    }
                    break;
                case MessageType.Attack:
                    if (ValidTeamIDAndPlayerID(msg.TeamID, msg.PlayerID))
                    {
                        game.Attack(communicationToGameID[msg.TeamID, msg.PlayerID], msg.Angle);
                    }
                    break;
                case MessageType.UseCommonSkill:
                    if (ValidTeamIDAndPlayerID(msg.TeamID, msg.PlayerID))
                    {
                        //这里返回了是否成功使用技能的值，应该需要发给client
                        bool isSuccess = game.UseCommonSkill(communicationToGameID[msg.TeamID, msg.PlayerID]);
                    }
                    break;
                case MessageType.Send:
                    SendMessageToTeammate(msg);
                    break;
                case MessageType.Pick:
                    if (ValidTeamIDAndPlayerID(msg.TeamID, msg.PlayerID))
                    {
                        if (msg.PropType == Communication.Proto.PropType.Gem)
                            game.PickGem(communicationToGameID[msg.TeamID, msg.PlayerID]);
                        else
                        {
                            game.PickProp(communicationToGameID[msg.TeamID, msg.PlayerID], ProtoProp2UtilityProp(msg.PropType));
                        }
                    }
                    break;
                case MessageType.UseGem:
                    if (ValidTeamIDAndPlayerID(msg.TeamID, msg.PlayerID))
                    {
                        if(msg.GemSize > 0)
                            game.UseGem(communicationToGameID[msg.TeamID, msg.PlayerID], msg.GemSize);
                        else game.UseGem(communicationToGameID[msg.TeamID, msg.PlayerID]);
                    }
                    break;
                case MessageType.UseProp:
                    if (ValidTeamIDAndPlayerID(msg.TeamID, msg.PlayerID))
                    {
                        game.UseProp(communicationToGameID[msg.TeamID, msg.PlayerID]);
                    }
                    break;
                case MessageType.ThrowGem:
                    if (ValidTeamIDAndPlayerID(msg.TeamID, msg.PlayerID))
                    {
                        game.ThrowGem(communicationToGameID[msg.TeamID, msg.PlayerID],(int)msg.TimeInMilliseconds ,msg.Angle, msg.GemSize);
                    }
                    break;
                case MessageType.ThrowProp:
                    if (ValidTeamIDAndPlayerID(msg.TeamID, msg.PlayerID))
                    {
                        game.ThrowProp(communicationToGameID[msg.TeamID, msg.PlayerID], (int)msg.TimeInMilliseconds, msg.Angle);
                    }
                    break;
                default:
                    break;
            }
        }
        private bool ValidTeamIDAndPlayerID(long teamID, long playerID)
        {
            return teamID >= 0 && teamID < options.TeamCount && playerID >= 0 && playerID < options.PlayerCountPerTeam;
        }
        private MessageToClient.Types.GameObjMessage MapMsg(Map map)
        {
            MessageToClient.Types.GameObjMessage msgOfMap = new MessageToClient.Types.GameObjMessage();
            msgOfMap.MessageOfMap = new MessageOfMap();
            for (int i = 0; i < GameData.rows; i++)
            {
                msgOfMap.MessageOfMap.Row.Add(new MessageOfMap.Types.Row());
                for (int j = 0; j < GameData.cols; j++)
                {
                    msgOfMap.MessageOfMap.Row[i].Col.Add((int)map.ProtoGameMap[i, j]);
                }
            }
            return msgOfMap;
        }
        private void SendMessageToAllClients(MessageType msgType, bool requiredGaming = true)
        {
            if (requiredGaming && !game.GameMap.Timer.IsGaming)
                return;
            var gameObjList = game.GetGameObj();
            game.ClearLists(new Preparation.Utility.GameObjIdx[2] { Preparation.Utility.GameObjIdx.BombedBullet, Preparation.Utility.GameObjIdx.PickedProp });
            MessageToClient messageToClient = new MessageToClient();
            messageToClient.GameObjMessage.Add(MapMsg(game.GameMap));
            lock (messageToAllClientsLock)
            {
                switch (msgType)
                {
                    case MessageType.Gaming:
                    case MessageType.StartGame:
                    case MessageType.EndGame:
                        foreach (GameObj gameObj in gameObjList)
                        {
                            messageToClient.GameObjMessage.Add(CopyInfo.Auto(gameObj));
                        }
                        messageToClient.MessageType = msgType;
                        mwr?.WriteOne(messageToClient);
                        break;
                    default:
                        break;
                }
            }
            serverCommunicator.SendToClient(messageToClient);
        }
        private void SendMessageToTeammate(MessageToServer msgToServer)
        {
            if (!ValidTeamIDAndPlayerID(msgToServer.TeamID, msgToServer.PlayerID))
                return;
            if (msgToServer.Message.Length > 256)
            {
#if DEBUG
                Console.WriteLine("Message string is too long!");
#endif
            }
            else
            {
                MessageToOneClient msg = new MessageToOneClient();
                msg.PlayerID = msgToServer.ToPlayerID;
                msg.TeamID = msgToServer.TeamID;
                msg.Message = msgToServer.Message;
                msg.MessageType = MessageType.Send;
#if DEBUG
                Console.WriteLine(msg);
#endif
                serverCommunicator.SendToClient(msg);
            }

            return;
        }
        private void OnGameEnd()
        {
            SendMessageToAllClients(MessageType.EndGame, false);
            game.ClearAllLists();
            mwr?.Flush();
            if(options.ResultFileName != DefaultArgumentOptions.FileName)
                SaveGameResult(options.ResultFileName + ".json");
            SendGameResult();
            endGameInfoSema.Release();
        }
        protected virtual void SendGameResult()		// 天梯的 Server 给网站发消息记录比赛结果
        {
            var scores = new JObject[options.TeamCount];
            for (ushort i = 0; i < options.TeamCount; ++i)
            {
                scores[i] = new JObject { ["team_id"] = i.ToString(), ["score"] = GetTeamScore(i) };
            }
            httpSender?.SendHttpRequest
                (
                    new JObject
                    {
                        ["result"] = new JArray(scores)
                    }
                );
        }
        private void SaveGameResult(string path)
        {
            Dictionary<string, int> result = new Dictionary<string, int>();
            for (int i = 0; i < TeamCount; i++)
            {
                result.Add("Team" + i.ToString(), GetTeamScore(i));
            }
            JsonSerializer serializer = new JsonSerializer();
            using (StreamWriter sw = new StreamWriter(path))
            {
                using (JsonWriter writer = new JsonTextWriter(sw))
                {
                    serializer.Serialize(writer, result);
                }
            }
        }

        private void CheckStart()
        {
            if (game.GameMap.Timer.IsGaming)
                return;
            foreach (var id in communicationToGameID)
            {
                if (id == GameObj.invalidID) return;     //如果有未初始化的玩家，不开始游戏
            }

            Thread.Sleep((int)GameData.frameDuration); //发送信息后，暂停一帧时间

            new Thread
            (
                () =>
                {
                    Console.WriteLine("Game Start!");
                    game.StartGame((int)options.GameTimeInSecond * 1000);
                    OnGameEnd();
                }
            )
            { IsBackground = true }.Start();

            while (!game.GameMap.Timer.IsGaming)
                Thread.Sleep(1); //游戏未开始，等待

            SendMessageToAllClients(MessageType.StartGame);     //发送开始游戏信息
            game.AllPlayerUsePassiveSkill();
            //定时向client发送游戏情况
            new Thread
            (
                () =>
                {
                    //用一次frameratetask膜一次 ↓
                    FrameRateTaskExecutor<int> xfgg = new(
                        () => game.GameMap.Timer.IsGaming,
                        () =>
                        {
                            SendMessageToAllClients(MessageType.Gaming);
                        },
                        SendMessageToClientIntervalInMilliseconds,
                        () => 0
                    )
                    {
                        AllowTimeExceed = true,
                        MaxTolerantTimeExceedCount = 5,
                        TimeExceedAction = overExceed =>
                        {
                            if (overExceed)
                            {
                                Console.WriteLine("Fetal error: your computer runs too slow that server cannot send message at a frame rate of 20!!!");
                            }
#if DEBUG
                            else
                            {
                                Console.WriteLine("Debug info: Send message to clients time exceed for once.");
                            }
#endif
                        }
                    };

                    xfgg.Start();
#if DEBUG
                    new Thread
                        (
                            () =>
                            {
                                while (!xfgg.Finished)
                                {
                                    Console.WriteLine($"Send message to clients frame rate: {xfgg.FrameRate}");
                                    Thread.Sleep(1000);
                                }
                            }
                        )
                    { IsBackground = true }.Start();
#endif
                }
            )
            { IsBackground = true }.Start();
        }
        private Preparation.Utility.PropType ProtoProp2UtilityProp(Communication.Proto.PropType propType)
        {
            switch(propType)
            {
                case PropType.AddLife:
                    return Preparation.Utility.PropType.addLIFE;
                case PropType.AddSpeed:
                    return Preparation.Utility.PropType.addSpeed;
                case PropType.Gem:
                    return Preparation.Utility.PropType.Gem;
                case PropType.Shield:
                    return Preparation.Utility.PropType.Shield;
                case PropType.Spear:
                    return Preparation.Utility.PropType.Spear;
                default:
                    return Preparation.Utility.PropType.Null;
            }
        }
        public GameServer(ArgumentOptions options) : base(options)
        {
            if (options.mapResource == DefaultArgumentOptions.MapResource)
                this.game = new Game(MapInfo.defaultMap, options.TeamCount);
            else
            {
                uint[,] map = new uint[GameData.rows, GameData.cols];
                try
                {
                    string? line;
                    int i = 0, j = 0;
                    using (StreamReader sr = new StreamReader(options.mapResource))
                    {
                        while (!sr.EndOfStream && i < GameData.rows)
                        {
                            if ((line = sr.ReadLine()) != null)
                            {
                                string[] nums = line.Split(' ');
                                foreach (string item in nums)
                                {
                                    if (item.Length > 1)//以兼容原方案
                                    {
                                        map[i, j] = (uint)int.Parse(item);
                                    }
                                    else
                                    {
                                        //2022-04-22 by LHR 十六进制编码地图方案（防止地图编辑员瞎眼x
                                        map[i, j] = (uint)Preparation.Utility.MapEncoder.Hex2Dec(char.Parse(item));
                                    }
                                    j++;
                                    if (j >= GameData.cols)
                                    {
                                        j = 0;
                                        break;
                                    }
                                }
                                i++;
                            }
                        }
                    }
                }
                catch
                {
                    map = MapInfo.defaultMap;
                }
                finally { this.game = new Game(map, options.TeamCount); }
            }
            communicationToGameID = new long[options.TeamCount, options.PlayerCountPerTeam];
            //创建server时先设定待加入人物都是invalid
            for (int i = 0; i < communicationToGameID.GetLength(0); i++)
            {
                for (int j = 0; j < communicationToGameID.GetLength(1); j++)
                {
                    communicationToGameID[i, j] = GameObj.invalidID;
                }
            }

            if (options.FileName != DefaultArgumentOptions.FileName)
            {
                try
                {
                    mwr = new MessageWriter(options.FileName, options.TeamCount, options.PlayerCountPerTeam);
                }
                catch
                {
                    Console.WriteLine($"Error: Cannot create the playback file: {options.FileName}!");
                }
            }

            if(options.Url != DefaultArgumentOptions.Url && options.Token != DefaultArgumentOptions.Token)
            {
                this.httpSender = new HttpSender(options.Url, options.Token, "PUT");
            }
        }
    }
}
