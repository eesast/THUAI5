using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using HPSocket;
using HPSocket.Tcp;
using Communication.Proto;
using System.Threading;

namespace Communication.ServerCommunication
{
    public delegate void OnReceiveCallback(); 
    public delegate void OnConnectCallback();

    public sealed class ServerCommunication:IDisposable // 提供释放资源的接口
    {
        private static readonly ConcurrentDictionary<ulong, IntPtr> playerDict = new ConcurrentDictionary<ulong, IntPtr>(); // 储存当前所有玩家的id
        private static readonly ConcurrentDictionary<int, GameObjType> instanceDict = new ConcurrentDictionary<int, GameObjType>(); // 储存所有的子弹和道具信息
        private static AutoResetEvent allConnectionClosed = new AutoResetEvent(false); // 是否所有玩家都已经断开了连接

        private BlockingCollection<IGameMessage> msgQueue; // 储存信息的线程安全队列 
        private TcpPackServer server;
        public event OnReceiveCallback OnReceive;      // 用于赋给server的事件 发送信息时
        public event OnConnectCallback OnConnect;      // 收到client的请求连接信息时

        public ServerCommunication(string endpoint = "127.0.0.1")
        {
            server = new TcpPackServer();
            msgQueue = new BlockingCollection<IGameMessage>();

            server.OnAccept += delegate (IServer sender, IntPtr connId, IntPtr client)
            {
                OnConnect?.Invoke();
                Console.WriteLine($"Now the connect number is {server.ConnectionCount}");
                return HandleResult.Ok;
            };

            // 原先的想法是，在server.OnAccept()中加入对人数和是否有玩家重复的信息
            // 但这一操作只能判断人数，不能判别人的具体信息，还是要在OnReceive中进行，这样通信负载是不是有些过大?
            

            server.OnReceive += delegate (IServer sender, IntPtr connId, byte[] bytes)
            {
                Message message = new Message();
                // 信息解析
                message.Deserialize(bytes);

                // 信息判断是否有重合
                MessageToServer m2s = message.Content as MessageToServer;
                ulong key = ((ulong)m2s.PlayerID | (ulong)m2s.TeamID << 32);

                if (playerDict.ContainsKey(key))
                {
                    Console.WriteLine($"More than one client claims to have the same ID {m2s.TeamID} {m2s.PlayerID}. And this client won't be able to receive message from server."); // 这种情况可以强制退出游戏吗...
                    return HandleResult.Error;
                }
                playerDict.TryAdd(key, connId); // 此处有多次发送的问题
                try
                {
                    msgQueue.Add(message); 
                }
                catch (Exception e)
                {
                    Console.WriteLine("Exception occured when adding an item to the queue:" + e.Message);
                }
                OnReceive?.Invoke();
                return HandleResult.Ok;
            };

            // 有玩家退出时的操作(不知道这个原先在Agent中的功能迁移到Server中是否还有必要)
            server.OnClose += delegate (IServer sender, IntPtr connId, SocketOperation socketOperation, int errorCode)
            {
                foreach(ulong id in playerDict.Keys)
                {
                    if (playerDict[id] == connId)
                    {
                        if (!playerDict.TryRemove(id, out IntPtr temp))
                        {
                            return HandleResult.Error;
                        }
                        // 关于此处连接数（上文也一样的问题），实际上此处应该加一个mutex，但不加也无伤大雅
                        Console.WriteLine($"Player {id >> 32} {id & 0xffffffff} closed the connection");
                        break;
                    }
                }
                // 虽然有着重复队伍名称和玩家编号的client确实收不到信息，但还是会连在server上，这里的ConnectionCount也有谜之bug...
                Console.WriteLine($"Now the connect number is { server.ConnectionCount }");
                if (playerDict.IsEmpty)
                {
                    allConnectionClosed.Set();
                }
                return HandleResult.Ok;
            };
        }

        /// <summary>
        /// 监听某一端口的操作
        /// </summary>
        public bool Listen(ushort port = 7777)
        {
            server.Port = port;
            bool isListenning = server.Start();
            if(isListenning)
            {
                Console.WriteLine($"The Csharp server starts to listenning to port {port}");
            }
            else
            {
                Console.WriteLine("Failed to start Csharp server.");
            }
            return isListenning;
        }

        // 以下提供了"向client发送信息"的多个重载函数,针对性更强,可根据逻辑需求任意使用
        // 当然，以下代码可能有一些不精简的地方，以后可能会稍作改动

        /// <summary>
        /// 发送单人信息
        /// </summary>
        /// <param name="m21c">要发送的单播信息</param>
        /// <returns></returns>
        public void SendToClient(MessageToOneClient m21c)
        {
            Message message = new Message();
            message.Content = m21c;
            message.PacketType = PacketType.MessageToOneClient;

            // 判断对应的玩家编号是否存在，不存在就报错
            // 关于这里为什么要使用ulong?
            ulong key = ((ulong)m21c.PlayerID | ((ulong)m21c.TeamID << 32));
            if (!playerDict.ContainsKey(key))
            {
                Console.WriteLine($"Error: No such player corresponding to ID {m21c.TeamID} {m21c.PlayerID}");
                // 这里需不需要return??
                return;
            }
            byte[] bytes;
            message.Serialize(out bytes); // 生成字节流
            SendOperation(bytes);
        }

        public void SendToClient(MessageToInitialize m2i)
        {
            Message message = new Message();
            message.Content = m2i;
            message.PacketType = PacketType.MessageToInitialize;

            // 初始化应该不需要加太多判断的信息，即使加也应该在逻辑内容中加，所以我就直接发送了...
            byte[] bytes;
            message.Serialize(out bytes);
            SendOperation(bytes);
        }

        // 这里我的见解是这样的：server中的SendToClient应该不需要指定oneof中的内容（应该是在游戏逻辑中指定），实际上server只需要做两件事情：1.发送信息 2.视情况适时报出警告或错误
        // 此处需要说明一下protobuf中的oneof语法在C#中的使用：proto编译生成的cs文件会自动生成一个枚举值（除了oneof中的不同类型还有一个None），以供使用者随时判断
        public void SendToClient(MessageToAddInstance m2a)
        {
            switch(m2a.MessageOfInstanceCase)
            {
                case MessageToAddInstance.MessageOfInstanceOneofCase.MessageOfBullet:
                    instanceDict.TryAdd(m2a.Guid, GameObjType.Bullet);
                    break;
                case MessageToAddInstance.MessageOfInstanceOneofCase.MessageOfProp:
                    instanceDict.TryAdd(m2a.Guid, GameObjType.Prop);
                    break;
                default: // 此时的枚举类型为MessageToAddInstance.MessageOfInstanceOneofCase.None，也就是说，此时的oneof信息没有被成功指定
                    Console.WriteLine("Instance type hasn't been assigned");
                    return;
            }
            Message message = new Message();
            message.Content = m2a;
            message.PacketType = PacketType.MessageToAddInstance;

            int key = m2a.Guid;
            if(instanceDict.ContainsKey(key))
            {
                Console.WriteLine($"Repeated construction with guid:{key} and type:{m2a.MessageOfInstanceCase}");
                return;
            }

            byte[] bytes;
            message.Serialize(out bytes); // 生成字节流
            SendOperation(bytes);
        }

        // 销毁某一物体的guid(玩家除外)
        public void SendToClient(MessageToDestroyInstance m2d)
        {
            Message message = new Message();
            message.Content = m2d;
            message.PacketType = PacketType.MessageToDestroyInstance;

            // 有一个问题是，既然都用上字典了，那为什么还需要遍历??我再研究一下
            foreach (int id in instanceDict.Keys)
            {
                if (id == m2d.Guid)
                {
                    instanceDict.TryRemove(id, out GameObjType tmp);
                    Console.WriteLine($"Instance with guid:{id} and type:{tmp} has been destroyed.");
                    byte[] bytes;
                    message.Serialize(out bytes); // 生成字节流
                    SendOperation(bytes);
                    return;
                }
            }
            Console.WriteLine($"No instance with guid:{m2d.Guid}");
        }

        // 操作指令
        public void SendToClient(MessageToOperate m2o)
        {
            Message message = new Message();
            message.Content = m2o;
            message.PacketType = PacketType.MessageToOperate;

            // 这两个发送应该也没什么大忌，就先直接发送吧...
            byte[] bytes;
            message.Serialize(out bytes);
            SendOperation(bytes);
        }

        // 刷新指令
        public void SendToClient(MessageToRefresh m2r)
        {
            Message message = new Message();
            message.Content = m2r;
            message.PacketType = PacketType.MessageToRefresh;

            byte[] bytes;
            message.Serialize(out bytes);
            SendOperation(bytes);
        }


        /// <summary>
        /// 上面的多个函数只是给用户调用发送的接口，这里才是真正的发送操作
        /// </summary>
        /// <param name="bytes">由对象信息转化而来的字节流</param>
        private void SendOperation(byte[] bytes)
        {
            List<IntPtr> IDs = server.GetAllConnectionIds();
            foreach (var connId in IDs)
            {
                if (!server.Send(connId, bytes, bytes.Length))
                {
                    Console.WriteLine($"failed to send to: {connId}");
                    // TODO     
                }
            }
        }

        /// <summary>
        /// 以引用形式返回信息
        /// </summary>
        /// <param name="msg">需要使用的单条信息</param>
        /// <returns>是否提取成功</returns>
        public bool TryTake(out IGameMessage msg)
        {
            try
            {
                return msgQueue.TryTake(out msg);
            }
            catch (Exception e)
            {
                Console.WriteLine("Exception occured when using 'TryTake' method in server:" + e.Message);
                msg = null;
                return false;
            }
        }
       
        /// <summary>
        /// 以返回值形式返回信息
        /// </summary>
        /// <returns>返回的信息</returns>
        public IGameMessage Take()
        {
            try
            {
                return msgQueue.Take();
            }
            catch (Exception e)
            {
                Console.WriteLine("Exception occured when using 'Take' method in server:" + e.Message);
                return null;
            }
        }

        /// <summary>
        /// 单纯关闭连接(具体区别还要再问一下)
        /// </summary>
        /// <returns></returns>
        public bool Stop()
        {
            return server.Stop();
        }

        /// <summary>
        /// 关闭并释放资源
        /// </summary>
        public void Dispose()
        {
            server.Dispose();
            msgQueue.Dispose();
            GC.SuppressFinalize(this); // 手动回收垃圾
        }
    }
}
