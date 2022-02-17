using System;
using Communication.ClientCommunication;
using Communication.Proto;
using System.Collections.Generic;
using System.Threading;

namespace CSharpInterface
{
    public partial class ClientInterface
    {
        private long teamID;
        private long playerID;
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

        public List<MessageOfCharacter> GetCharacters()
        {
            List<MessageOfCharacter> list = new List<MessageOfCharacter>();
            playerDataLock.EnterReadLock();
            try
            {
                foreach (var data in playerData)
                {
                    if (CanSee(data.MessageOfCharacter))
                    {
                        list.Add(data.MessageOfCharacter);
                    }
                }
            }
            finally { playerDataLock.ExitReadLock();}
            return list;
        }
        public List<MessageOfBullet> GetBullets()
        {
            List<MessageOfBullet> list = new List<MessageOfBullet>();
            bulletDataLock.EnterReadLock();
            try
            {
                foreach (var data in playerData)
                {
                    if (CanSee(data.MessageOfBullet))
                    {
                        list.Add(data.MessageOfBullet);
                    }
                }
            }
            finally { bulletDataLock.ExitReadLock();}
            return list;
        }
        public List<MessageOfProp> GetNoGemProps()
        {
            List<MessageOfProp> list = new List<MessageOfProp>();
            propDataLock.EnterReadLock();
            try
            {
                foreach (var data in playerData)
                {
                    if (CanSee(data.MessageOfProp) && data.MessageOfProp.Type != Communication.Proto.PropType.Gem)
                    {
                        list.Add(data.MessageOfProp);
                    }
                }
            }
            finally { propDataLock.ExitReadLock();}
            return list;
        }
        public List<MessageOfProp> GetGems()
        {
            List<MessageOfProp> list = new List<MessageOfProp>();
            propDataLock.EnterReadLock();
            try
            {
                foreach (var data in playerData)
                {
                    if (CanSee(data.MessageOfProp) && data.MessageOfProp.Type == Communication.Proto.PropType.Gem)
                    {
                        list.Add(data.MessageOfProp);
                    }
                }
            }
            finally { propDataLock.ExitReadLock(); }
            return list;
        }
        public List<Wall> GetWalls()
        {
            List<Wall> list = new List<Wall>();
            for(int i = 0;i<Map.map.GetLength(0);i++)
            {
                for(int j=0;j<Map.map.GetLength(1);j++)
                {
                    if(Map.map[i,j] ==1)
                    {
                        list.Add(new Wall(i * 1000 + 500, j * 1000 + 500));
                    }
                }
            }
            return list;
        }
        public List<BirthPoint> GetBirthPoints()
        {
            List<BirthPoint> list = new List<BirthPoint>();
            for (int i = 0; i < Map.map.GetLength(0); i++)
            {
                for (int j = 0; j < Map.map.GetLength(1); j++)
                {
                    if (Map.map[i, j] >=5 && Map.map[i, j] <= 12)
                    {
                        list.Add(new BirthPoint(i * 1000 + 500, j * 1000 + 500));
                    }
                }
            }
            return list;
        }
        public MessageOfCharacter GetSelfInfo()
        {
            MessageOfCharacter message = new MessageOfCharacter(myInfo.MessageOfCharacter);
            return message;
        }

        public void MovePlayer(int timeInMilliseconds, double angle)
        {
            MessageToServer msg = new MessageToServer();
            msg.MessageType = MessageType.Move;
            msg.PlayerID = playerID;
            msg.TeamID = teamID;
            msg.TimeInMilliseconds = timeInMilliseconds;
            msg.Angle = angle;
            clientCommunication.SendMessage(msg);
        }
        public void MoveUp(int timeInMilliseconds)
        {
            MessageToServer msg = new MessageToServer();
            msg.MessageType = MessageType.Move;
            msg.PlayerID = playerID;
            msg.TeamID = teamID;
            msg.TimeInMilliseconds = timeInMilliseconds;
            msg.Angle = Math.PI;
            clientCommunication.SendMessage(msg);
        }
        public void MoveDown(int timeInMilliseconds)
        {
            MessageToServer msg = new MessageToServer();
            msg.MessageType = MessageType.Move;
            msg.PlayerID = playerID;
            msg.TeamID = teamID;
            msg.TimeInMilliseconds = timeInMilliseconds;
            msg.Angle = 0;
            clientCommunication.SendMessage(msg);
        }
        public void MoveLeft(int timeInMilliseconds)
        {
            MessageToServer msg = new MessageToServer();
            msg.MessageType = MessageType.Move;
            msg.PlayerID = playerID;
            msg.TeamID = teamID;
            msg.TimeInMilliseconds = timeInMilliseconds;
            msg.Angle = 3 * Math.PI / 2;
            clientCommunication.SendMessage(msg);
        }
        public void MoveRight(int timeInMilliseconds)
        {
            MessageToServer msg = new MessageToServer();
            msg.MessageType = MessageType.Move;
            msg.PlayerID = playerID;
            msg.TeamID = teamID;
            msg.TimeInMilliseconds = timeInMilliseconds;
            msg.Angle = Math.PI / 2;
            clientCommunication.SendMessage(msg);
        }
        public void Attack(double angle)
        {
            MessageToServer msg = new MessageToServer();
            msg.MessageType = MessageType.Attack;
            msg.PlayerID = playerID;
            msg.TeamID = teamID;
            msg.Angle = angle;
            clientCommunication.SendMessage(msg);
        }
        public void Pick(PropType prop)
        {
            MessageToServer msg = new MessageToServer();
            msg.MessageType = MessageType.Pick;
            msg.PlayerID = playerID;
            msg.TeamID = teamID;
            switch(prop)
            {
                case PropType.Gem:
                    msg.PropType = Communication.Proto.PropType.Gem;
                    break;
                case PropType.addHP:
                    msg.PropType = Communication.Proto.PropType.AddHp;
                    break;
                case PropType.addAP:
                    msg.PropType = Communication.Proto.PropType.AddAp;
                    break;
                case PropType.addSpeed:
                    msg.PropType = Communication.Proto.PropType.AddSpeed;
                    break;
                default:
                    msg.PropType = Communication.Proto.PropType.NullPropType;
                    break;
                
            }
            clientCommunication.SendMessage(msg);
        }
        public void UseProp()
        {
            MessageToServer msg = new MessageToServer();
            msg.MessageType = MessageType.UseProp;
            msg.PlayerID = playerID;
            msg.TeamID = teamID;
            clientCommunication.SendMessage(msg);
        }
        public void UseGem(int size = int.MaxValue)
        {
            MessageToServer msg = new MessageToServer();
            msg.MessageType = MessageType.UseGem;
            msg.PlayerID = playerID;
            msg.TeamID = teamID;
            msg.GemSize = size;
            clientCommunication.SendMessage(msg);
        }
        public void Send(string str)
        {
            MessageToServer msg = new MessageToServer();
            msg.MessageType = MessageType.Send;
            msg.PlayerID = playerID;
            msg.TeamID = teamID;
            msg.Message = str;
            clientCommunication.SendMessage(msg);
        }
        public void ThrowProp(int timeInMilliseconds, double angle)
        {
            MessageToServer msg = new MessageToServer();
            msg.MessageType = MessageType.ThrowProp;
            msg.PlayerID = playerID;
            msg.TeamID = teamID;
            msg.TimeInMilliseconds = timeInMilliseconds;
            msg.Angle = angle;
            clientCommunication.SendMessage(msg);
        }
        public void ThrowGem(int timeInMilliseconds, double angle, int size = 1)
        {
            MessageToServer msg = new MessageToServer();
            msg.MessageType = MessageType.UseGem;
            msg.PlayerID = playerID;
            msg.TeamID = teamID;
            msg.TimeInMilliseconds = timeInMilliseconds;
            msg.Angle = angle;
            msg.GemSize = size;
            clientCommunication.SendMessage(msg);
        }
        public void UseSkill()
        {
            MessageToServer msg = new MessageToServer();
            msg.MessageType = MessageType.UseCommonSkill;
            msg.PlayerID = playerID;
            msg.TeamID = teamID;
            clientCommunication.SendMessage(msg);
        }
    }
}
