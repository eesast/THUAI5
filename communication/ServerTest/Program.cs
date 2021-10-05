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
            // 1.初始化信息，包括：地图形状、玩家数和每个玩家的属性
            MessageToInitialize m2i = new MessageToInitialize();
            m2i.MapSerial = 2;
            m2i.NumberOfValidPlayer = 2;
            // 比较麻烦的一个操作：构造每一个人物的属性
            BasicalCharacterProperty p1 = new BasicalCharacterProperty();
            BasicalCharacterProperty p2 = new BasicalCharacterProperty();
            p1.Guid = 1;
            p1.ActiveSkillType = 1;
            p1.PassiveSkillType = 1;

            p2.Guid = 2;
            p2.ActiveSkillType = 2;
            p2.PassiveSkillType = 2;

            m2i.Property.Add(p1);
            m2i.Property.Add(p2);
            server.SendToClient(m2i);
            Console.WriteLine("StartGame");
            Console.ReadLine();

            // 2.操作指令
            // 添加子弹属性
            MessageToOperate m2o = new MessageToOperate();
            // ??不太理解operateKind是做什么的
            m2o.OperateKind = true;
            m2o.MessageToAddInstance = new MessageToAddInstance();
            m2o.MessageToAddInstance.InstanceType = GameObjType.Bullet;
            m2o.MessageToAddInstance.Guid = 200; // 这个是我随便写的，后续GUID的协议还是要交给逻辑组决定吧
            m2o.MessageToAddInstance.MessageOfBullet = new MessageOfBullet();
            m2o.MessageToAddInstance.MessageOfBullet.Type = BulletType.AtomBomb;
            m2o.MessageToAddInstance.MessageOfBullet.BulletAP = 200;
            m2o.MessageToAddInstance.MessageOfBullet.BoomRange = 20;
            server.SendToClient(m2o);
            Console.WriteLine("Add a bullet");
            Console.ReadLine();

            // 3.刷新指令
            // 更新人物属性
            MessageToRefresh m2r0 = new MessageToRefresh();
            m2r0.GameObjType = GameObjType.Character;
            m2r0.MessageOfCharacter = new ChangeableCharacterProperty();
            m2r0.MessageOfCharacter.AttackRange = 1;
            m2r0.MessageOfCharacter.Buff = BuffType.AddLife;
            m2r0.MessageOfCharacter.BulletNum = 10;
            m2r0.MessageOfCharacter.GemNum = 5;
            m2r0.MessageOfCharacter.Life = 2;
            m2r0.MessageOfCharacter.X = 500;
            m2r0.MessageOfCharacter.Y = 500;
            server.SendToClient(m2r0);
            Console.WriteLine("Set changeable character property");
            Console.ReadLine();
            // 更新子弹属性
            MessageToRefresh m2r = new MessageToRefresh();
            m2r.GameObjType = GameObjType.Bullet;
            m2r.MessageOfBullet = new MessageOfBullet();
            m2r.MessageOfBullet.BulletAP = 100;
            m2r.MessageOfBullet.BoomRange = 10;
            server.SendToClient(m2r);
            Console.WriteLine("Change a bullet");
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

        //static private MessageToClient BasicMessage(long guid, MessageType msgType)
        //{
        //    // 地图信息记录（这里暂时没有好的测试对象）

        //    // GUID信息记录（注意是以“组”加入GUID的）
        //    // 把RepeatedField看成STL中的vector即可
        //    RepeatedField<MessageToClient.Types.OneTeamGUIDs> playerGUIDs = new RepeatedField<MessageToClient.Types.OneTeamGUIDs>();
        //    for(int x = 0; x < 2; x++)
        //    {
        //        playerGUIDs.Add(new MessageToClient.Types.OneTeamGUIDs());
        //        for(int y = 0; y < 4; y++)
        //        {
        //            playerGUIDs[x].TeammateGUIDs.Add(x * 100 + y);
        //        }
        //    }

        //    // 玩家初始属性构造
        //    MessageToClient msg = new MessageToClient();
        //    msg.PlayerID = 0;
        //    msg.TeamID = 0;
        //    msg.MessageType = msgType;
        //    msg.SelfInfo = new GameObjInfo();
        //    msg.SelfInfo.ASkill1 = ActiveSkillType.Askill0;
        //    msg.SelfInfo.ASkill2 = ActiveSkillType.Askill1;
        //    msg.SelfInfo.Bap = 100;
        //    msg.SelfInfo.BoomRange = 20;
        //    msg.SelfInfo.BulletNum = 12;
        //    msg.SelfInfo.CanMove = true;
        //    msg.SelfInfo.CD = 5;
        //    msg.SelfInfo.FacingDirection = 0.0;
        //    msg.SelfInfo.GameObjType = GameObjType.Character;
        //    msg.SelfInfo.Hp = 1000;
        //    msg.SelfInfo.Guid = guid;
        //    msg.SelfInfo.IsDying = false;
        //    msg.SelfInfo.IsMoving = true;
        //    msg.SelfInfo.LifeNum = 10;
        //    msg.SelfInfo.MaxBulletNum = 50;
        //    msg.SelfInfo.MaxHp = 2500;
        //    msg.SelfInfo.MoveSpeed = 10;
        //    msg.SelfInfo.PropType = PropType.Null;
        //    msg.SelfInfo.Radius = 250;
        //    msg.SelfInfo.ShapeType = ShapeType.Circle;
        //    msg.SelfInfo.TeamID = 0;
        //    msg.SelfInfo.X = 5000;
        //    msg.SelfInfo.Y = 5000;

        //    msg.SelfInfo.IsLaid = true;
        //    msg.SelfInfo.Place = PlaceType.Ground;
        //    msg.SelfInfo.PSkill = PassiveSkillType.Pskill0;

        //    return msg;
        //}
    }
}
