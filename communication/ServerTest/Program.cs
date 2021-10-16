using System;
using Communication.Proto;
using Communication.ServerCommunication;
using Google.Protobuf.Collections;


namespace ServerTest
{
    class Test
    {
        /// <summary>
        /// args[0] 监听端口(选填，默认7777)
        /// </summary>
        /// <param name="args"></param>
        static void Main(string[] args)
        {
            ushort port = 7777;
            if(args.Length!=0)
            {
                port = ushort.Parse(args[0]);
            }

            ServerCommunication server = new ServerCommunication();
            server.Listen(port); // 监听端口
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
                    Console.WriteLine($"Receive a message from {m2s.TeamID} {m2s.PlayerID}");
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
            // 1.初始化信息，包括：地图形状、玩家数和每个玩家的属性
            MessageToInitialize m2i = new MessageToInitialize();
            m2i.MapSerial = 2;
            m2i.NumberOfValidPlayer = 2;
            
            server.SendToClient(m2i);
            Console.WriteLine("StartGame");
            Console.ReadLine();

            // 2.人物设置（这里我默认人物也应该在初始化的类型中）
            MessageToRefreshCharacter m2rc = new MessageToRefreshCharacter();
            for (int i = 0; i < m2i.NumberOfValidPlayer; i++)
            {
                MessageOfCharacter mc = new MessageOfCharacter();
                mc.ActiveSkillType = ActiveSkillType.SuperFast;
                // 人物属性需要自己加
                m2rc.MessageOfCharacter.Add(mc);
            }
            server.SendToClient(m2i);
            Console.WriteLine("AddPlayers");
            Console.ReadLine();

            // 3.子弹与道具设置
            MessageToRefreshBullet m2rb = new MessageToRefreshBullet();
            for(int i = 0;i<10;i++)
            {
                MessageOfBullet mb = new MessageOfBullet();
                mb.FacingDirection = 100;
                // 子弹属性需要自己加
                m2rb.MessageOfBullet.Add(mb);
            }
            server.SendToClient(m2rb);
            Console.WriteLine("AddBullets");
            Console.ReadLine();

            MessageToRefreshProp m2rp= new MessageToRefreshProp();
            for (int i = 0; i < 10; i++)
            {
                MessageOfProp mp = new MessageOfProp();
                mp.Type = PropType.AddAp;
                // 子弹属性需要自己加
                m2rp.MessageOfProp.Add(mp);
            }
            server.SendToClient(m2rp);
            Console.WriteLine("AddProps");
            Console.ReadLine();

            // 4.单播测试
            m21c = new MessageToOneClient();
            m21c.Guid = 100;
            m21c.PlayerID = 0;
            m21c.TeamID = 0;
            m21c.MessageType = MessageType.Send;
            m21c.Message = "Hey! this is a message from server!";
            server.SendToClient(m21c);
            Console.WriteLine("send a message to one client.");
            Console.ReadLine();

            // 5.结束游戏
            m21c.Guid = 1;
            m21c.PlayerID = 0;
            m21c.TeamID = 0;
            m21c.MessageType = MessageType.EndGame;
            server.SendToClient(m21c);
            Console.WriteLine("GameOver");
            Console.ReadLine();
            server.Dispose();
            server.Stop();
        }
    }
}
