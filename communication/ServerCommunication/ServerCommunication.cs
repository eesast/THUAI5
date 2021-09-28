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
        private static readonly ConcurrentDictionary<ulong, IntPtr> dict = new ConcurrentDictionary<ulong, IntPtr>(); // 储存当前所有玩家的信息
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

                if (dict.ContainsKey(key))
                {
                    Console.WriteLine($"More than one client claims to have the same ID {m2s.TeamID} {m2s.PlayerID}. And this client won't be able to receive message from server."); // 这种情况可以强制退出游戏吗...
                    return HandleResult.Error;
                }
                dict.TryAdd(key, connId); // 此处有多次发送的问题
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
                foreach(ulong id in dict.Keys)
                {
                    if (dict[id] == connId)
                    {
                        if (!dict.TryRemove(id, out IntPtr temp))
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
                if (dict.IsEmpty)
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

        /// <summary>
        /// 发送全局信息
        /// </summary>
        public void SendToClient(MessageToClient m2c)
        {   
            // 构造信息
            Message message = new Message();
            message.Content = m2c;
            message.PacketType = PacketType.MessageToClient;

            byte[] bytes;
            message.Serialize(out bytes); // 生成字节流
            SendOperation(bytes);
        }

        /// <summary>
        /// 发送单人信息
        /// </summary>
        /// <param name="msg"></param>
        /// <returns></returns>
        public void SendToClient(MessageToOneClient m21c)
        {
            Message message = new Message();
            message.Content = m21c;
            message.PacketType = PacketType.MessageToOneClient;

            // 判断对应的玩家编号是否存在，不存在就报错
            ulong key = ((ulong)m21c.PlayerID | ((ulong)m21c.TeamID << 32));
            if (!dict.ContainsKey(key))
            {
                Console.WriteLine($"Error: No such player corresponding to ID {m21c.TeamID} {m21c.PlayerID}");
            }
            byte[] bytes;
            message.Serialize(out bytes); // 生成字节流
            SendOperation(bytes);
        }

        /// <summary>
        /// 上面那两个函数只是给用户调用发送的接口，这里才是真正的发送操作
        /// </summary>
        /// <param name="bytes"></param>
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
        /// <param name="msg"></param>
        /// <returns></returns>
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
        /// <returns></returns>
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
