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
            messageToServer.ASkill1 = ActiveSkillType.SuperFast;
            messageToServer.PSkill = PassiveSkillType.Vampire;
            clientCommunication.SendMessage(messageToServer);
            Thread.Sleep(1000);
            var k = Console.ReadKey().Key;
            while (k != ConsoleKey.Escape)
            {
                switch(k)
                {
                    case ConsoleKey.A:
                        MessageToServer msgA = new MessageToServer();
                        msgA.MessageType = MessageType.Move;
                        msgA.PlayerID = playerID;
                        msgA.TeamID = teamID;
                        msgA.TimeInMilliseconds = 50;
                        msgA.Angle = Math.PI;
                        clientCommunication.SendMessage(msgA);
                        break;
                    case ConsoleKey.D:
                        MessageToServer msgD = new MessageToServer();
                        msgD.MessageType = MessageType.Move;
                        msgD.PlayerID = playerID;
                        msgD.TeamID = teamID;
                        msgD.TimeInMilliseconds = 50;
                        msgD.Angle = 0;
                        clientCommunication.SendMessage(msgD);
                        break;
                    case ConsoleKey.W:
                        MessageToServer msgW = new MessageToServer();
                        msgW.MessageType = MessageType.Move;
                        msgW.PlayerID = playerID;
                        msgW.TeamID = teamID;
                        msgW.TimeInMilliseconds = 50;
                        msgW.Angle = Math.PI/2;
                        clientCommunication.SendMessage(msgW);
                        break;
                    case ConsoleKey.S:
                        MessageToServer msgS = new MessageToServer();
                        msgS.MessageType = MessageType.Move;
                        msgS.PlayerID = playerID;
                        msgS.TeamID = teamID;
                        msgS.TimeInMilliseconds = 50;
                        msgS.Angle = 3*Math.PI/2;
                        clientCommunication.SendMessage(msgS);
                        break;
                    case ConsoleKey.J:
                        MessageToServer msgJ = new MessageToServer();
                        msgJ.MessageType = MessageType.Attack;
                        msgJ.PlayerID = playerID;
                        msgJ.TeamID = teamID;
                        msgJ.Angle = Math.PI;
                        clientCommunication.SendMessage(msgJ);
                        break;
                    case ConsoleKey.U:
                        MessageToServer msgU = new MessageToServer();
                        msgU.MessageType = MessageType.UseCommonSkill;
                        msgU.PlayerID = playerID;
                        msgU.TeamID = teamID;
                        clientCommunication.SendMessage(msgU);
                        break;
                }
                k = Console.ReadKey().Key;
            }
            //new FrameRateTaskExecutor<int>
            //(
            //    () => true,
            //    () =>
            //    {
            //        MessageToServer msg = new MessageToServer();
            //        msg.MessageType = MessageType.Move;
            //        msg.PlayerID = playerID;
            //        msg.TeamID = teamID;
            //        msg.TimeInMilliseconds = 50;
            //        msg.Angle = 0;
            //        clientCommunication.SendMessage(msg);
            //    },
            //    50,
            //    () => 0,
            //    1000
            //)
            //{
            //    AllowTimeExceed = true
            //}.Start();
            
            Console.ReadKey();
        }
    }
}
