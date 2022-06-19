using System.Threading;
using Playback;
using Timothy.FrameRateTask;
using System;
using System.Collections.Generic;
using Communication.Proto;
using System.Windows;

namespace Client
{
    public class PlaybackClient
    {
        private readonly string fileName;
        private readonly double playbackSpeed;
        private readonly int frameTimeInMilliseconds;
        public MessageReader? Reader;
        private SemaphoreSlim sema;
        public SemaphoreSlim Sema => sema;
        public PlaybackClient(string fileName, double playbackSpeed = 1.0, int frameTimeInMilliseconds = 50)
        {
            this.fileName = fileName;
            this.playbackSpeed = playbackSpeed;
            this.frameTimeInMilliseconds = frameTimeInMilliseconds;
            this.sema = new SemaphoreSlim(1, 1);
            try
            {
                Reader = new MessageReader(this.fileName);
            }
            catch (Exception ex)
            {
                Reader = null;
                Console.WriteLine(ex.Message);
                return;
            }
        }

        public int[, ]? ReadDataFromFile(Dictionary<GameObjType, List<MessageToClient.Types.GameObjMessage>> dataDict, object dataLock)
        {
            if (Reader == null)
                return null;
            Sema.Wait();
            bool endFile = false;
            bool mapFlag = false;    // 是否获取了地图
            int[,] map = new int[50, 50];
            long frame = (long)(this.frameTimeInMilliseconds / this.playbackSpeed);
            var mapCollecter = new MessageReader(this.fileName);
            while(!mapFlag)
            {
                var msg = mapCollecter.ReadOne();
                if (msg == null)
                    throw new Exception("Map messgae is not in the playback file!");
                foreach (MessageToClient.Types.GameObjMessage obj in msg.GameObjMessage)
                {
                    if (obj.ObjCase == MessageToClient.Types.GameObjMessage.ObjOneofCase.MessageOfMap)
                    {
                        try
                        {
                            for (int i = 0; i < 50; i++)
                            {
                                for (int j = 0; j < 50; j++)
                                {
                                    map[i, j] = obj.MessageOfMap.Row[i].Col[j];
                                }
                            }
                        }
                        catch
                        {
                            mapFlag = false;
                        }
                        finally
                        {
                            mapFlag = true;
                        }
                        break;
                    }
                }
            };

            new Thread(() =>
            {
                new FrameRateTaskExecutor<int>
                (
                    () => !endFile,
                    () =>
                    {
                        var content = Reader.ReadOne();
                        if(content == null)
                            endFile = true;
                        else
                        {
                            lock (dataLock)  
                            {
                                dataDict[GameObjType.Character].Clear();
                                dataDict[GameObjType.Prop].Clear();
                                dataDict[GameObjType.Bullet].Clear();
                                dataDict[GameObjType.BombedBullet].Clear();
                                switch (content.MessageType)
                                {
                                    case MessageType.StartGame:
                                        foreach (MessageToClient.Types.GameObjMessage obj in content.GameObjMessage)
                                        {
                                            switch (obj.ObjCase)
                                            {
                                                case MessageToClient.Types.GameObjMessage.ObjOneofCase.MessageOfCharacter:
                                                    dataDict[GameObjType.Character].Add(obj);
                                                    break;
                                                case MessageToClient.Types.GameObjMessage.ObjOneofCase.MessageOfBullet:
                                                    dataDict[GameObjType.Bullet].Add(obj);
                                                    break;
                                                case MessageToClient.Types.GameObjMessage.ObjOneofCase.MessageOfProp:
                                                    dataDict[GameObjType.Prop].Add(obj);
                                                    break;
                                                case MessageToClient.Types.GameObjMessage.ObjOneofCase.MessageOfBombedBullet:
                                                    dataDict[GameObjType.BombedBullet].Add(obj);
                                                    break;
                                            }
                                        }
                                        break;
                                    case MessageType.Gaming:

                                        foreach (MessageToClient.Types.GameObjMessage obj in content.GameObjMessage)
                                        {
                                            switch (obj.ObjCase)
                                            {
                                                case MessageToClient.Types.GameObjMessage.ObjOneofCase.MessageOfCharacter:
                                                    dataDict[GameObjType.Character].Add(obj);
                                                    break;
                                                case MessageToClient.Types.GameObjMessage.ObjOneofCase.MessageOfBullet:
                                                    dataDict[GameObjType.Bullet].Add(obj);
                                                    break;
                                                case MessageToClient.Types.GameObjMessage.ObjOneofCase.MessageOfProp:
                                                    dataDict[GameObjType.Prop].Add(obj);
                                                    break;
                                                case MessageToClient.Types.GameObjMessage.ObjOneofCase.MessageOfBombedBullet:
                                                    dataDict[GameObjType.BombedBullet].Add(obj);
                                                    break;
                                            }
                                        }
                                        break;
                                    case MessageType.EndGame:
                                        foreach (MessageToClient.Types.GameObjMessage obj in content.GameObjMessage)
                                        {
                                            switch (obj.ObjCase)
                                            {
                                                case MessageToClient.Types.GameObjMessage.ObjOneofCase.MessageOfCharacter:
                                                    dataDict[GameObjType.Character].Add(obj);
                                                    break;
                                                case MessageToClient.Types.GameObjMessage.ObjOneofCase.MessageOfBullet:
                                                    dataDict[GameObjType.Bullet].Add(obj);
                                                    break;
                                                case MessageToClient.Types.GameObjMessage.ObjOneofCase.MessageOfProp:
                                                    dataDict[GameObjType.Prop].Add(obj);
                                                    break;
                                                case MessageToClient.Types.GameObjMessage.ObjOneofCase.MessageOfBombedBullet:
                                                    dataDict[GameObjType.BombedBullet].Add(obj);
                                                    break;
                                            }
                                        }
                                        break;
                                }
                            }
                        }
                    },
                frame,
                () =>
                {
                    Sema.Release();
                    MessageBox.Show("Game Over!");
                    return 1;
                }
                )
                { AllowTimeExceed = true }.Start();
            })
            { IsBackground = true }.Start();
            return map;
        }
    }   
}
