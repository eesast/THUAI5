using Communication.Proto;
using Playback;
using System;
using System.Threading;
using Timothy.FrameRateTask;

namespace Server
{
    /// <summary>
    /// 回放用 Server
    /// </summary>
    class PlayBackServer : ServerBase
    {
        private int[ , ] teamScore;

        public override int TeamCount { get => teamScore.GetLength(0); }
        //public override bool ForManualOperation => true;

        public PlayBackServer(ArgumentOptions options) : base(options)
        {
            teamScore = new int[0, 0];
        }
        public override void WaitForGame()
        {
            try
            {
                if (options.ResultOnly)
                {
                    using (MessageReader mr = new MessageReader(options.FileName))
                    {
                        Console.WriteLine("Parsing playback file...");
                        teamScore = new int[mr.teamCount, mr.playerCount];
                        int infoNo = 0;
                        object cursorLock = new object();
                        var initialTop = Console.CursorTop;
                        var initialLeft = Console.CursorLeft;
                        while (true)
                        {
                            MessageToClient? msg = null;
                            for (int i = 0; i < mr.teamCount; ++i)
                            {
                                for (int j = 0; j < mr.playerCount; ++j)
                                {
                                    msg = mr.ReadOne();
                                    if (msg == null)
                                    {
                                        Console.WriteLine("The game doesn't come to an end because of timing up!");
                                        goto endParse;
                                    }

                                    lock (cursorLock)
                                    {
                                        var curTop = Console.CursorTop;
                                        var curLeft = Console.CursorLeft;
                                        Console.SetCursorPosition(initialLeft, initialTop);
                                        Console.WriteLine($"Parsing messages... Current message number: {infoNo}");
                                        Console.SetCursorPosition(curLeft, curTop);
                                    }

                                    if (msg != null)
                                    {
                                        //teamScore[i] = msg.TeamScore;
                                    }
                                }
                            }

                            ++infoNo;

                            if (msg == null)
                            {
                                Console.WriteLine("No game information in this file!");
                                goto endParse;
                            }
                            if (msg.MessageType == MessageType.EndGame)
                            {
                                Console.WriteLine("Game over normally!");
                                goto endParse;
                            }
                        }

                    endParse:

                        Console.WriteLine($"Successfully parsed {infoNo} informations!");
                    }
                }
                else
                {
                    long timeInterval = GameServer.SendMessageToClientIntervalInMilliseconds;
                    if (options.PlaybackSpeed != 1.0)
                    {
                        options.PlaybackSpeed = Math.Max(0.25, Math.Min(4.0, options.PlaybackSpeed));
                        timeInterval = (int)Math.Round(timeInterval / options.PlaybackSpeed);
                    }
                    using (MessageReader mr = new MessageReader(options.FileName))
                    {
                        teamScore = new int[mr.teamCount, mr.playerCount];
                        int infoNo = 0;
                        object cursorLock = new object();
                        var msgCurTop = Console.CursorTop;
                        var msgCurLeft = Console.CursorLeft;
                        var frt = new FrameRateTaskExecutor<int>
                            (
                            loopCondition: () => true,
                            loopToDo: () =>
                            {
                                MessageToClient? msg = null;
                                for (int i = 0; i < mr.teamCount; ++i)
                                {
                                    for (int j = 0; j < mr.playerCount; ++j)
                                    {
                                        msg = mr.ReadOne();
                                        if (msg == null)
                                        {
                                            Console.WriteLine("The game doesn't come to an end because of timing up!");
                                            return false;
                                        }
                                        serverCommunicator.SendToClient(msg);
                                        lock (cursorLock)
                                        {
                                            var curTop = Console.CursorTop;
                                            var curLeft = Console.CursorLeft;
                                            Console.SetCursorPosition(msgCurLeft, msgCurTop);
                                            Console.WriteLine($"Sending messages... Current message number: {infoNo}.");
                                            Console.SetCursorPosition(curLeft, curTop);
                                        }
                                        if (msg != null)
                                        {
                                            foreach(var item in msg.GameObjMessage)
                                            { 
                                                if(item.MessageOfCharacter != null)
                                                    teamScore[item.MessageOfCharacter.TeamID, item.MessageOfCharacter.PlayerID] = item.MessageOfCharacter.Score;
                                            }
                                        }
                                    }
                                }
                                ++infoNo;
                                if (msg == null)
                                {
                                    Console.WriteLine("No game information in this file!");
                                    return false;
                                }
                                if (msg.MessageType == MessageType.EndGame)
                                {
                                    Console.WriteLine("Game over normally!");
                                    return false;
                                }
                                return true;
                            },
                            timeInterval: timeInterval,
                            finallyReturn: () => 0
                            )
                        { AllowTimeExceed = true, MaxTolerantTimeExceedCount = 5 };

                        Console.WriteLine("The server is well prepared! Please MAKE SURE that you have opened all the clients to watch the game!");
                        Console.WriteLine("If ALL clients have opened, press any key to start.");
                        Console.ReadKey();

                        new Thread
                            (
                                () =>
                                {
                                    var rateCurTop = Console.CursorTop;
                                    var rateCurLeft = Console.CursorLeft;
                                    lock (cursorLock)
                                    {
                                        rateCurTop = Console.CursorTop;
                                        rateCurLeft = Console.CursorLeft;
                                        Console.WriteLine($"Send message to clients frame rate: {frt.FrameRate}");
                                    }
                                    while (!frt.Finished)
                                    {
                                        lock (cursorLock)
                                        {
                                            var curTop = Console.CursorTop;
                                            var curLeft = Console.CursorLeft;
                                            Console.SetCursorPosition(rateCurLeft, rateCurTop);
                                            Console.WriteLine($"Send message to clients frame rate: {frt.FrameRate}");
                                            Console.SetCursorPosition(curLeft, curTop);
                                        }
                                        Thread.Sleep(1000);
                                    }
                                }
                            )
                        { IsBackground = true }.Start();

                        lock (cursorLock)
                        {
                            msgCurLeft = Console.CursorLeft;
                            msgCurTop = Console.CursorTop;
                            Console.WriteLine("Sending messages...");
                        }
                        frt.Start();
                    }
                }
            }
            finally
            {
                teamScore ??= new int[0, 0];
            }
        }

        protected override void OnReceive(MessageToServer msg) { }
        public override int GetTeamScore(long teamID)
        {
            int ret = 0;
            for (int i = 0; i < teamScore.GetLength(1); i++)
            {
                ret += teamScore[teamID, i];
            }
            return ret;
        }
    }
}
