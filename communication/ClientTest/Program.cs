using System;
using Communication.ClientCommunication;
using Communication.Proto;

namespace ClientTest
{
    class ClientTest
    {
        static void Main(string[] args)
        {
            try
            {
                Console.WriteLine($"my params are {int.Parse(args[0])} and {int.Parse(args[1])}");
            }
            catch(Exception e)
            {
                Console.WriteLine("no params!");
                Environment.Exit(0); // 我也不知道这样做合适不合适...
            }
            ClientCommunication client = new ClientCommunication();
            client.OnReceive += delegate ()
            { 
                IGameMessage msg = client.Take();
                MessageToOneClient m2one = msg.Content as MessageToOneClient; // 强制转换消息类型，但如果无法转换也不会报错，会返回null
                Console.WriteLine($"Message type: {msg.PacketType}");
                Console.WriteLine(m2one);
            };

            if (client.Connect("127.0.0.1",7777))
            {
                Console.WriteLine("success to connect to the server.");
            }
            else
            {
                Console.WriteLine("fail to connect to the server.");
            }

            // 解析命令行并发送信息
            MessageToServer m2s = new MessageToServer();
            m2s.TeamID = int.Parse(args[0]);
            m2s.PlayerID = int.Parse(args[1]);
            client.SendMessage(m2s);

            Console.ReadLine();
            client.Stop();
            client.Dispose();

        }
    }   
}
