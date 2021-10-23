using System;
using Communication.ClientCommunication;
using Communication.Proto;
using Timothy.FrameRateTask;
using System.Threading;

namespace Test
    
{
    class Program
    {
        static void Main(string[] args)
        {
            long playerID, teamID;
            playerID = Convert.ToInt64(args[0]);
            teamID = Convert.ToInt64(args[1]);
            Console.WriteLine();
            ClientCommunication clientCommunication = new ClientCommunication();
            clientCommunication.Connect("127.0.0.1", 7777);
            MessageToServer messageToServer = new MessageToServer();
            messageToServer.MessageType = MessageType.AddPlayer;
            messageToServer.PlayerID = playerID;
            messageToServer.TeamID = teamID;
            messageToServer.ASkill1 = ActiveSkillType.BecomeAssassin;
            messageToServer.PSkill = PassiveSkillType.Vampire;
            clientCommunication.SendMessage(messageToServer);
            Thread.Sleep(1000);
            new FrameRateTaskExecutor<int>
            (
                () => true,
                () =>
                {
                    MessageToServer msg = new MessageToServer();
                    msg.MessageType = MessageType.Move;
                    msg.PlayerID = playerID;
                    msg.TeamID = teamID;
                    msg.TimeInMilliseconds = 50;
                    msg.Angle = 0;
                    clientCommunication.SendMessage(msg);
                },
                50,
                () => 0,
                1000
            )
            {
                AllowTimeExceed = true
            }.Start();

            Console.ReadKey();
        }
    }
}
