using System;
using System.Threading;
using Communication.Proto;
using Communication.ServerCommunication;

namespace Server
{
    public abstract class ServerBase
    {
        protected readonly ArgumentOptions options;

        protected readonly ServerCommunication serverCommunicator;
        public abstract int TeamCount { get; }
        public abstract int GetTeamScore(long teamID);
        public abstract void WaitForGame();
        protected abstract void OnReceive(MessageToServer msg);

        public ServerBase(ArgumentOptions options)
        {
            //队伍数量在 1~4 之间，总人数不超过 8
            if (options.TeamCount > 4) 
                options.TeamCount = 4;
            if (options.TeamCount < 1) 
                options.TeamCount = 1;
            if (options.PlayerCountPerTeam * options.TeamCount > 8) 
                options.PlayerCountPerTeam = (ushort)(8 / options.TeamCount);
            if (options.PlayerCountPerTeam < 1) 
                options.PlayerCountPerTeam = 1;
            this.options = options;
            this.serverCommunicator = new ServerCommunication();
            while (!serverCommunicator.Listen(options.ServerPort))
            {
                Console.WriteLine("Server listen failed!");
                Thread.Sleep(1000);
            }

            Console.WriteLine("Server begins to listen!");

            serverCommunicator.OnConnect += delegate ()
            {
                Console.WriteLine("Successfully connected!");
            };

            serverCommunicator.OnReceive += delegate ()
            {
                if (serverCommunicator.TryTake(out IGameMessage msg) && msg.PacketType == PacketType.MessageToServer)
                {
                    this.OnReceive((MessageToServer)msg.Content);
                }
            };
        }
    }
}
