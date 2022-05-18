using System;
using Communication.ClientCommunication;
using Communication.Proto;
using Timothy.FrameRateTask;
using System.Threading;
using System.Collections.Generic;

namespace CSharpInterface
{
    public partial class ClientInterface
    {
        private List<MessageToClient.Types.GameObjMessage> playerData;
        private List<MessageToClient.Types.GameObjMessage> bulletData;
        private List<MessageToClient.Types.GameObjMessage> propData;
        private MessageToClient.Types.GameObjMessage? myInfo = null;
        private ReaderWriterLockSlim playerDataLock;
        private ReaderWriterLockSlim bulletDataLock;
        private ReaderWriterLockSlim propDataLock;
        private bool isGaming = false;
        private ClientCommunication clientCommunication;

        private void MessageProcessing(MessageToClient content)
        {
            playerDataLock.EnterWriteLock();
            bulletDataLock.EnterWriteLock();
            propDataLock.EnterWriteLock();
            try
            {
                playerData.Clear();
                propData.Clear();
                bulletData.Clear();
                switch (content.MessageType)
                {
                    case MessageType.StartGame:
                        isGaming = true;
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
                                case MessageToClient.Types.GameObjMessage.ObjOneofCase.MessageOfMap:
                                    if (Map.flag)
                                        break;
                                    else
                                    {
                                        lock(Map.mapLock)
                                        {
                                            for(int i=0;i<50;i++)
                                            {
                                                for(int j=0;j<50;j++)
                                                {
                                                    Map.map[i, j] = obj.MessageOfMap.Row[i].Col[j];
                                                }
                                            }
                                            Map.flag = true;
                                        }
                                    }
                                    break;
                            }
                        }
                        break;
                    case MessageType.Gaming:
                        isGaming = true;
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
                                case MessageToClient.Types.GameObjMessage.ObjOneofCase.MessageOfMap:
                                    if (Map.flag)
                                        break;
                                    else
                                    {
                                        lock (Map.mapLock)
                                        {
                                            for (int i = 0; i < 50; i++)
                                            {
                                                for (int j = 0; j < 50; j++)
                                                {
                                                    Map.map[i, j] = obj.MessageOfMap.Row[i].Col[j];
                                                }
                                            }
                                            Map.flag = true;
                                        }
                                    }
                                    break;
                            }
                        }
                        break;
                    case MessageType.EndGame:
                        isGaming = false;
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
            finally 
            {
                playerDataLock.ExitWriteLock();
                bulletDataLock.ExitWriteLock();
                propDataLock.ExitWriteLock();
            }
        }
        private void main(string[] args)
        {
            clientCommunication = new ClientCommunication();
            clientCommunication.Connect("127.0.0.1", 7777);
            this.teamID = Convert.ToInt64(args[0]);
            this.playerID = Convert.ToInt64(args[1]);
            playerData = new List<MessageToClient.Types.GameObjMessage>();
            bulletData = new List<MessageToClient.Types.GameObjMessage>();
            propData = new List<MessageToClient.Types.GameObjMessage>();
            playerDataLock = new ReaderWriterLockSlim();
            bulletDataLock = new ReaderWriterLockSlim();
            propDataLock = new ReaderWriterLockSlim();
            MessageToServer messageToServer = new MessageToServer();
            messageToServer.MessageType = MessageType.AddPlayer;
            messageToServer.PlayerID = this.playerID;
            messageToServer.TeamID = this.teamID;
            switch(this.activeSkillType)
            {
                case ActiveSkillType.BecomeVampire:
                    messageToServer.ASkill1 = Communication.Proto.ActiveSkillType.BecomeVampire;
                    break;
                case ActiveSkillType.BecomeAssassin:
                    messageToServer.ASkill1 = Communication.Proto.ActiveSkillType.BecomeAssassin;
                    break;
                case ActiveSkillType.NuclearWeapon:
                    messageToServer.ASkill1 = Communication.Proto.ActiveSkillType.NuclearWeapon;
                    break;
                case ActiveSkillType.SuperFast:
                    messageToServer.ASkill1 = Communication.Proto.ActiveSkillType.SuperFast;
                    break;
                default:
                    messageToServer.ASkill1 = Communication.Proto.ActiveSkillType.NullActiveSkillType;
                    break;
            }
            switch(this.passiveSkillType)
            {
                case PassiveSkillType.Vampire:
                    messageToServer.PSkill = Communication.Proto.PassiveSkillType.Vampire;
                    break;
                case PassiveSkillType.SpeedUpWhenLeavingGrass:
                    messageToServer.PSkill = Communication.Proto.PassiveSkillType.SpeedUpWhenLeavingGrass;
                    break;
                case PassiveSkillType.RecoverAfterBattle:
                    messageToServer.PSkill = Communication.Proto.PassiveSkillType.RecoverAfterBattle;
                    break;
                default:
                    messageToServer.PSkill = Communication.Proto.PassiveSkillType.NullPassiveSkillType;
                    break;
            }

            clientCommunication.OnReceive += () =>
            {
                if (clientCommunication.TryTake(out IGameMessage msg) && msg.PacketType == PacketType.MessageToClient)
                {
                    MessageProcessing((MessageToClient)msg.Content);
                }
            };
            clientCommunication.SendMessage(messageToServer);

            while (!isGaming)
            {
                Thread.Sleep(500);
            }
            new FrameRateTaskExecutor<int>
            (
                () => isGaming,
                () =>
                {
                    Play();
                },
                50,
                () => 0,
                maxTotalDuration: Constants.GameDuration
            )
            {
                AllowTimeExceed = true
            }.Start();
        }
        public ClientInterface(string[] args)
        {
            main(args);
        }
        public static void Main(string[] args)
        {
            new ClientInterface(args);
        }
    }
}
