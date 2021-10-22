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
        private static readonly ConcurrentDictionary<long, IntPtr> playerDict = new(); // 储存当前所有玩家的id
        // 本来加这个字典的目的是为了防止子弹和道具的重复构造，但了解了guid的机制后，感觉没什么必要...
        // private static readonly ConcurrentDictionary<int, GameObjType> instanceDict = new ConcurrentDictionary<int, GameObjType>(); // 储存所有的子弹和道具信息
        private static readonly AutoResetEvent allConnectionClosed = new(false); // 是否所有玩家都已经断开了连接

        private readonly BlockingCollection<IGameMessage> msgQueue; // 储存信息的线程安全队列 
        private readonly TcpPackServer server;
        public event OnReceiveCallback OnReceive;      // 用于赋给server的事件 发送信息时
        public event OnConnectCallback OnConnect;      // 收到client的请求连接信息时

        public ServerCommunication(string endpoint = "127.0.0.1")
        {
            server = new TcpPackServer();
            msgQueue = new BlockingCollection<IGameMessage>();

            // 第一步连接时，server还收不到任何关于client的信息，因此只有先让client连接上以后并发送一条信息，才能反馈client
            server.OnAccept += delegate (IServer sender, IntPtr connId, IntPtr client)
            {
                OnConnect?.Invoke();
#if DEBUG
                Console.WriteLine($"Now the connect number is {server.ConnectionCount} (Maybe there are repeated clients.)");
#endif 
                return HandleResult.Ok;
            };

            // 原先的想法是，在server.OnAccept()中加入对人数和是否有玩家重复的信息
            // 但这一操作只能判断人数，不能判别人的具体信息，还是要在OnReceive中进行，这样通信负载是不是有些过大?
            

            server.OnReceive += delegate (IServer sender, IntPtr connId, byte[] bytes)
            {
                Message message = new();
                // 信息解析
                message.Deserialize(bytes);

                // 信息判断是否有重合
                MessageToServer m2s = message.Content as MessageToServer;
                // 为避免重复构造，以下的操作均在初始化条件下进行（消息类型需要在client端手动指定）。AddPlayer的操作每局游戏每个玩家仅进行一次
                if (m2s.MessageType == MessageType.AddPlayer)
                {
                    // 添加的过程可能需要加锁
                    lock (this)
                    {
                        // 不太理解原来为什么是<<32.感觉<<16就够用了。可能和dotnet版本有关，之前的写法会报警告
                        long key = (m2s.PlayerID | m2s.TeamID << 16);
                        MessageToOneClient messageToOneClient = new();
                        messageToOneClient.PlayerID = m2s.PlayerID;
                        messageToOneClient.TeamID = m2s.TeamID;

                        if (playerDict.ContainsKey(key))
                        {
                            messageToOneClient.MessageType = MessageType.InvalidPlayer;
                            Console.WriteLine($"More than one client claims to have the same ID {m2s.TeamID} {m2s.PlayerID} with connId {connId}. And this client won't be able to receive message from server."); // 这种情况可以强制退出游戏吗...
                            SendToClient(messageToOneClient, connId);
                            return HandleResult.Ok;
                        }
                        else
                        {
                            playerDict.TryAdd(key, connId);
                            messageToOneClient.MessageType = MessageType.ValidPlayer;
                            SendToClient(messageToOneClient);
                        }
                    }
                }

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
                foreach(long id in playerDict.Keys)
                {
                    if (playerDict[id] == connId)
                    {
                        if (!playerDict.TryRemove(id, out IntPtr temp))
                        {
                            return HandleResult.Error;
                        }
                        // 关于此处连接数（上文也一样的问题），实际上此处应该加一个mutex，但不加也无伤大雅
                        Console.WriteLine($"Player {id >> 16} {id & 0xffff} closed the connection");
                        break;
                    }
                }
                // 虽然有着重复队伍名称和玩家编号的client确实收不到信息，但还是会连在server上，这里的ConnectionCount也有谜之bug...
#if DEBUG
                Console.WriteLine($"Now the connect number is { server.ConnectionCount }");
#endif
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
                Console.WriteLine($"The Csharp server starts to listen to port {port}");
            }
            else
            {
                Console.WriteLine("Failed to start Csharp server.");
            }
            return isListenning;
        }

        // 以下提供了"向client发送信息"的多个重载函数,针对性更强,可根据逻辑需求任意使用
        // 当然，以下代码可能有一些不精简的地方，以后可能会稍作改动
        // 我的见解是这样的：server中的SendToClient应该不需要指定oneof中的内容（应该是在游戏逻辑中指定，指定完毕以后就没有另一种选项的存储空间了），实际上server只需要做两件事情：1.发送信息 2.视情况适时报出警告或错误
        // 此处需要说明一下protobuf中的oneof语法在C#中的使用：proto编译生成的cs文件会自动生成一个枚举值（除了oneof中的不同类型还有一个None），以供使用者随时判断
        // 10-16 更改：去掉oneof机制后感觉语句简单了很多，甚至感觉重载也没什么意义...可以直接搞一个接口统一所有操作

        /// <summary>
        /// 发送单人信息
        /// </summary>
        /// <param name="m21c">要发送的单播信息</param>
        /// <returns></returns>
        public void SendToClient(MessageToOneClient m21c)
        {
            Message message = new();
            message.Content = m21c;
            message.PacketType = PacketType.MessageToOneClient;

            // 判断对应的玩家编号是否存在，不存在就报错
            // 关于THUAI4这里为什么要使用ulong?
            long key = m21c.PlayerID | (m21c.TeamID << 16);
            byte[] bytes;
            message.Serialize(out bytes); // 生成字节流
            SendOperationUniCast(bytes, key);
        }

        /// <summary>
        /// 发送单人信息（这是针对不合法玩家的，因为不合法玩家在字典中没有键值对，所以只能通过connId发送）
        /// </summary>
        /// <param name="m21c"></param>
        public void SendToClient(MessageToOneClient m21c, IntPtr connId)
        {
            Message message = new();
            message.Content = m21c;
            message.PacketType = PacketType.MessageToOneClient;

            byte[] bytes;
            message.Serialize(out bytes); // 生成字节流
            SendOperationUniCast(bytes, connId);
        }

        /// <summary>
        /// 需要发送的初始化信息
        /// </summary>
        /// <param name="m2i">初始化信息</param>
        public void SendToClient(MessageToInitialize m2i)
        {
            Message message = new();
            message.Content = m2i;
            message.PacketType = PacketType.MessageToInitialize;

            // 初始化应该不需要加太多判断的信息，即使加也应该在逻辑内容中加，所以我就直接发送了...
            byte[] bytes;
            message.Serialize(out bytes);
            SendOperationBroadCast(bytes);
        }

        public void SendToClient(MessageToClient m2c)
        {
            Message message = new();
            message.Content = m2c;
            message.PacketType = PacketType.MessageToClient;

            // 此处我希望可以看到oneof所包裹的信息(利用枚举值)，但似乎做不到？这点和之前使用oneof有不一样
            byte[] bytes;
            message.Serialize(out bytes);
            SendOperationBroadCast(bytes);
        }

        /// <summary>
        /// 需要发送的更新子弹的信息
        /// </summary>
        /// <param name="m2rb">子弹信息</param>
        //public void SendToClient(MessageToRefreshBullet m2rb)
        //{
        //    Message message = new Message();
        //    message.Content = m2rb;
        //    message.PacketType = PacketType.MessageToRefreshBullet;
        //    byte[] bytes;
        //    message.Serialize(out bytes);
        //    SendOperationBroadCast(bytes);
        //}

        ///// <summary>
        ///// 需要发送的更新人物的信息
        ///// </summary>
        ///// <param name="m2rc">更新信息</param>
        //public void SendToClient(MessageToRefreshCharacter m2rc)
        //{
        //    Message message = new Message();
        //    message.Content = m2rc;
        //    message.PacketType = PacketType.MessgaeToRefreshCharacter;

        //    byte[] bytes;
        //    message.Serialize(out bytes);
        //    SendOperationBroadCast(bytes);
        //}

        ///// <summary>
        ///// 需要发送的更新道具的信息
        ///// </summary>
        ///// <param name="m2rp"></param>
        //public void SendToClient(MessageToRefreshProp m2rp)
        //{
        //    Message message = new Message();
        //    message.Content = m2rp;
        //    message.PacketType = PacketType.MessageToRefreshProp;

        //    byte[] bytes;
        //    message.Serialize(out bytes);
        //    SendOperationBroadCast(bytes);
        //}

        // 解释一下：此处是我的锅，之前没有很好地理解THUAI4中单播和广播的机制，现在才发现THUAI4中的server貌似是不区分单播和广播的
        // 在agent端才会根据消息的枚举类型确定单播还是广播
        // 因此在THUAI5中我们在server端就需要把这些事做好

        /// <summary>
        /// 上面的多个函数只是给用户调用发送的接口，这里才是真正的发送操作（广播）
        /// </summary>
        /// <param name="bytes">由对象信息转化而来的字节流</param>
        private void SendOperationBroadCast(byte[] bytes)
        {
            foreach (var connId in playerDict.Values)
            {
                if (!server.Send(connId, bytes, bytes.Length))
                {
                    Console.WriteLine($"failed to send to: {connId}");
                    // TODO     
                }
            }
        }

        /// <summary>
        /// 真正的发送操作（单播）
        /// </summary>
        /// <param name="bytes">由对象信息转化而来的字节流</param>
        /// <param name="key">某玩家在字典中对应的键</param>
        private void SendOperationUniCast(byte[] bytes,long key)
        {
            IntPtr connId;
            _ = playerDict.TryGetValue(key, out connId);
            if (server.Send(connId, bytes, bytes.Length))
            {
                Console.WriteLine($"Only send to {key >> 16} {key & 0xffff} with connId {connId}");    
            }
            else
            {
                Console.WriteLine($"failed to send to: {connId}");
            }
        }

        /// <summary>
        /// 一个非常愚蠢的重载...直接通过connId发送信息
        /// </summary>
        /// <param name="bytes">要发送的字节流信息</param>
        /// <param name="connId">client的连接Id</param>
        private void SendOperationUniCast(byte[] bytes,IntPtr connId)
        {
            if (!server.Send(connId, bytes, bytes.Length))
            {
                Console.WriteLine($"failed to send to: {connId}");
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
