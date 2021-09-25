using System;
using Communication.Proto;
using Communication.ServerCommunication;
using Google.Protobuf.Collections;


namespace ServerTest
{
    class Test
    {
        static void Main(string[] args)
        {
            ServerCommunication server = new ServerCommunication();
            server.Listen(7777); // 监听端口
            server.OnConnect += delegate ()
            {
               Console.WriteLine("A client connects");
            };
            server.OnReceive += delegate ()
            {
                IGameMessage msg;
                if(server.TryTake(out msg))
                {
                    MessageToServer m2s = msg.Content as MessageToServer; // 强制类型转换
                    Console.WriteLine($"Receive a message from {m2s.PlayerID}");
                    Console.WriteLine($"Message type::{m2s.MessageType}");
                    if (m2s.MessageType == MessageType.Send) // 有大量需要send枚举值的操作，因此这里需要判断一下
                    {
                        Console.WriteLine(m2s.Message);
                    }
                }
                else
                {
                    Console.WriteLine("fail to dequeue.");
                }
            };
            Console.WriteLine("============================");
            Console.ReadLine();

            // 解析client发来的信息并组装后发给server
            // 单人信息发送示例
            MessageToOneClient m21c = new MessageToOneClient();
            m21c.PlayerID = 0;
            m21c.TeamID = 0;
            m21c.MessageType = MessageType.ValidPlayer;
            m21c.Guid = 888;
            server.SendToClient(m21c);
            Console.WriteLine($"Send to {m21c.PlayerID} {m21c.TeamID}");
            Console.WriteLine("============================");
            Console.ReadLine();

            // 一般游戏指令
            server.SendToClient(BasicMessage(0, MessageType.StartGame));
            Console.WriteLine("StartGame");
            Console.ReadLine();

            server.SendToClient(BasicMessage(0, MessageType.Gaming));
            Console.WriteLine("Gaming");
            Console.ReadLine();

            // 单播测试
            m21c = new MessageToOneClient();
            m21c.Guid = 100;
            m21c.PlayerID = 0;
            m21c.TeamID = 0;
            m21c.MessageType = MessageType.Send;
            m21c.Message = "Hey! this is a message from server!";
            server.SendToClient(m21c);
            Console.WriteLine("send a message to one client.");

            // 结束游戏
            server.SendToClient(BasicMessage(666, MessageType.EndGame));
            Console.WriteLine("GameOver");
            Console.ReadLine();
            server.Dispose();
            server.Stop();
        }

        static private MessageToClient BasicMessage(long guid, MessageType msgType)
        {
            // 地图信息记录（这里暂时没有好的测试对象）

            // GUID信息记录（注意是以“组”加入GUID的）
            // 把RepeatedField看成STL中的vector即可
            RepeatedField<MessageToClient.Types.OneTeamGUIDs> playerGUIDs = new RepeatedField<MessageToClient.Types.OneTeamGUIDs>();
            for(int x = 0; x < 2; x++)
            {
                playerGUIDs.Add(new MessageToClient.Types.OneTeamGUIDs());
                for(int y = 0; y < 4; y++)
                {
                    playerGUIDs[x].TeammateGUIDs.Add(x * 100 + y);
                }
            }

            // 玩家初始属性构造
            MessageToClient msg = new MessageToClient();
            msg.PlayerID = 0;
            msg.TeamID = 0;
            msg.MessageType = msgType;
            msg.SelfInfo = new GameObjInfo();
            msg.SelfInfo.ASkill1 = ActiveSkillType.Askill0;
            msg.SelfInfo.ASkill2 = ActiveSkillType.Askill1;
            msg.SelfInfo.Bap = 100;
            msg.SelfInfo.BoomRange = 20;
            msg.SelfInfo.BulletNum = 12;
            msg.SelfInfo.CanMove = true;
            msg.SelfInfo.CD = 5;
            msg.SelfInfo.FacingDirection = 0.0;
            msg.SelfInfo.GameObjType = GameObjType.Character;
            msg.SelfInfo.Hp = 1000;
            msg.SelfInfo.Guid = guid;
            msg.SelfInfo.IsDying = false;
            msg.SelfInfo.IsMoving = true;
            msg.SelfInfo.LifeNum = 10;
            msg.SelfInfo.MaxBulletNum = 50;
            msg.SelfInfo.MaxHp = 2500;
            msg.SelfInfo.MoveSpeed = 10;
            msg.SelfInfo.PropType = PropType.Null;
            msg.SelfInfo.Radius = 250;
            msg.SelfInfo.ShapeType = ShapeType.Circle;
            msg.SelfInfo.TeamID = 0;
            msg.SelfInfo.X = 5000;
            msg.SelfInfo.Y = 5000;

            msg.SelfInfo.IsLaid = true;
            msg.SelfInfo.Place = PlaceType.Ground;
            msg.SelfInfo.PSkill = PassiveSkillType.Pskill0;

            return msg;
        }
    }
}
