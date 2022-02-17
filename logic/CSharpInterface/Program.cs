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
        private object takeMsgLock = new object();
        private bool isGaming = false;
        private Int64 gameDuration;
        private ClientCommunication clientCommunication;

        private void MessageProcessing(MessageToClient content)
        {
            lock (takeMsgLock)
            {
                playerData.Clear();
                propData.Clear();
                bulletData.Clear();
                switch (content.MessageType)
                {
                    case MessageType.InitialLized:
                        isGaming = true;
                        break;
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
                        isGaming = false;
                        break;
                }
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
            try
            {
                gameDuration = Convert.ToInt64(args[2]);
            }
            catch
            {
                gameDuration = 300000;
            }
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
                    OperaionAtEachFrame();
                },
                50,
                () => 0,
                maxTotalDuration: gameDuration
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
