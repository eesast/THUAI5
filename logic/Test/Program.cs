using System;
using Communication.ClientCommunication;
using Communication.Proto;
using Timothy.FrameRateTask;
using System.Threading;

namespace Test

{
    class Program
    {
        static void MessageProcessing(MessageToClient msgToClient)
        {
            switch(msgToClient.MessageType)
            {
                case MessageType.StartGame:
                case MessageType.Gaming:
                
                    foreach(MessageToClient.Types.GameObjMessage obj in msgToClient.GameObjMessage)
                    {
                        switch(obj.ObjCase)
                        {
                            //case MessageToClient.Types.GameObjMessage.ObjOneofCase.MessageOfCharacter:
                            //    Console.WriteLine($"GUID:{obj.MessageOfCharacter.Guid} Character is at ({obj.MessageOfCharacter.X},{obj.MessageOfCharacter.Y}).");
                            //    break;
                            case MessageToClient.Types.GameObjMessage.ObjOneofCase.MessageOfBullet:
                                Console.WriteLine($"GUID:{obj.MessageOfBullet.Guid} is at ({obj.MessageOfBullet.X},{obj.MessageOfBullet.Y})");
                                break;
                        }
                    }
                    break;
                case MessageType.EndGame:
                    Console.WriteLine("Game Ended! Press ESC to quit.");
                    break;
            }
        }
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
            messageToServer.PSkill = PassiveSkillType.SpeedUpWhenLeavingGrass;

            clientCommunication.OnReceive += 
            () =>
            {
                if (clientCommunication.TryTake(out IGameMessage msg) && msg.PacketType == PacketType.MessageToClient)
                {
                    MessageProcessing((MessageToClient)msg.Content);
                }
            };
            clientCommunication.SendMessage(messageToServer);

            Thread.Sleep(1000);

            var k = Console.ReadKey().Key;
            while (k != ConsoleKey.Escape)
            {
                switch (k)
                {
                    case ConsoleKey.W:
                        MessageToServer msgA = new MessageToServer();
                        msgA.MessageType = MessageType.Move;
                        msgA.PlayerID = playerID;
                        msgA.TeamID = teamID;
                        msgA.TimeInMilliseconds = 50;
                        msgA.Angle = Math.PI;
                        clientCommunication.SendMessage(msgA);
                        break;
                    case ConsoleKey.S:
                        MessageToServer msgD = new MessageToServer();
                        msgD.MessageType = MessageType.Move;
                        msgD.PlayerID = playerID;
                        msgD.TeamID = teamID;
                        msgD.TimeInMilliseconds = 50;
                        msgD.Angle = 0;
                        clientCommunication.SendMessage(msgD);
                        break;
                    case ConsoleKey.D:
                        MessageToServer msgW = new MessageToServer();
                        msgW.MessageType = MessageType.Move;
                        msgW.PlayerID = playerID;
                        msgW.TeamID = teamID;
                        msgW.TimeInMilliseconds = 50;
                        msgW.Angle = Math.PI / 2;
                        clientCommunication.SendMessage(msgW);
                        break;
                    case ConsoleKey.A:
                        MessageToServer msgS = new MessageToServer();
                        msgS.MessageType = MessageType.Move;
                        msgS.PlayerID = playerID;
                        msgS.TeamID = teamID;
                        msgS.TimeInMilliseconds = 50;
                        msgS.Angle = 3 * Math.PI / 2;
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
                    case ConsoleKey.K:
                        MessageToServer msgK = new MessageToServer();
                        msgK.MessageType = MessageType.UseGem;
                        msgK.PlayerID = playerID;
                        msgK.TeamID = teamID;
                        clientCommunication.SendMessage(msgK);
                        break;
                    case ConsoleKey.L:
                        MessageToServer msgL = new MessageToServer();
                        msgL.MessageType = MessageType.ThrowGem;
                        msgL.PlayerID = playerID;
                        msgL.TeamID = teamID;
                        msgL.GemSize = 1;
                        msgL.TimeInMilliseconds = 3000;
                        msgL.Angle = Math.PI;
                        clientCommunication.SendMessage(msgL);
                        break;
                    case ConsoleKey.P:
                        MessageToServer msgP = new MessageToServer();
                        msgP.MessageType = MessageType.Pick;
                        msgP.PlayerID = playerID;
                        msgP.TeamID = teamID;
                        msgP.PropType = PropType.Gem;
                        clientCommunication.SendMessage(msgP);
                        break;
                    case ConsoleKey.O:
                        MessageToServer msgO = new MessageToServer();
                        msgO.MessageType = MessageType.Pick;
                        msgO.PlayerID = playerID;
                        msgO.TeamID = teamID;
                        clientCommunication.SendMessage(msgO);
                        break;
                    case ConsoleKey.I:
                        MessageToServer msgI = new MessageToServer();
                        msgI.MessageType = MessageType.UseProp;
                        msgI.PlayerID = playerID;
                        msgI.TeamID = teamID;
                        clientCommunication.SendMessage(msgI);
                        break;
                    case ConsoleKey.Y:
                        MessageToServer msgY = new MessageToServer();
                        msgY.MessageType = MessageType.ThrowProp;
                        msgY.PlayerID = playerID;
                        msgY.TeamID = teamID;
                        msgY.TimeInMilliseconds = 3000;
                        msgY.Angle = Math.PI;
                        clientCommunication.SendMessage(msgY);
                        break;
                    default:
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
