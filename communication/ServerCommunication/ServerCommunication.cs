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
        private static readonly ConcurrentDictionary<ulong, IntPtr> dict = new ConcurrentDictionary<UInt64, IntPtr>(); // 储存当前所有玩家的信息
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
                return HandleResult.Ok;
            };

            server.OnReceive += delegate (IServer sender, IntPtr connId, byte[] bytes)
            {
                Message message = new Message();
                //MessageToServer m2s = message.Content as MessageToServer;
                //ulong key = ((ulong)m2s.PlayerID | (ulong)m2s.TeamID << 32);
                //if (dict.ContainsKey(key))
                //{
                //    Console.WriteLine($"More than one client claims to have the same ID {m2s.TeamID} {m2s.PlayerID}."); // 这种情况可以强制退出游戏吗...
                //    return HandleResult.Error;
                //}
                //dict.TryAdd(key, connId); // 此处有多次发送的问题

                message.Deserialize(bytes);
                try
                {
                    msgQueue.Add(message);//理论上这里可能抛出异常ObjectDisposedException或InvalidOperationException
                }
                catch (Exception e)
                {
                    Console.WriteLine("Exception occured when adding an item to the queue:" + e.Message);
                }
                OnReceive?.Invoke();
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
        public void SendToClient(MessageToClient msg)
        {   
            // 构造信息
            Message message = new Message();
            message.Content = msg;
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
        public void SendToClient(MessageToOneClient msg)
        {
            Message message = new Message();
            message.Content = msg;
            message.PacketType = PacketType.MessageToOneClient;

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
