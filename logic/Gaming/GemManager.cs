using GameEngine;
using Preparation.Utility;
using GameClass.GameObj;
using Preparation.GameData;
using System;
using System.Collections.Generic;
using Timothy.FrameRateTask;
using System.Threading;

namespace Gaming
{
    public partial class Game
    {
        private readonly GemManager gemManager;
        private class GemManager  //单独用GemManager处理宝石
        {
            private readonly Map gameMap;
            private bool isProducingGem = false;
            private MoveEngine moveEngine;
            private readonly List<XYPosition> gemWellList;
            public void StartProducingGem()
            {
                if (isProducingGem)
                    return;

                isProducingGem = true;
#if DEBUG
                Console.WriteLine("Start producing gems!");
#endif
                ProduceGemsInWell();
                /*
                 自动生成宝石。
                宝石的生成应该分为两类：
                1、宝石井附近生成。
                2、地图上随机生成。
                地图上随机生成还没写。
                 */
            }
            private void ProduceGemsInWell()
            {
                int len = gemWellList.Count;
                Random r = new Random(Environment.TickCount);
                new Thread
                (
                    () =>
                    {
                        new FrameRateTaskExecutor<int>
                        (
                            () => gameMap.Timer.IsGaming,
                            () =>
                            {
                                int rand = r.Next(0, len);
                                XYPosition randPos = gemWellList[rand];
                                bool flag = false;  //是否已经有宝石在指定位置了？
                                gameMap.GemListLock.EnterReadLock();
                                try
                                {
                                    foreach (Gem gem in gameMap.GemList)
                                    {
                                        if (gem.Position.x == randPos.x && gem.Position.y == randPos.y)
                                        {
                                            gem.TryAddGemSize();
                                            flag = true;
                                            break;
                                        }
                                    }
                                }
                                finally { gameMap.GemListLock.ExitReadLock(); }

                                if (!flag)
                                {
                                    Gem newGem = new Gem(randPos);
                                    gameMap.GemListLock.EnterWriteLock();
                                    try
                                    {
                                        gameMap.GemList.Add(newGem);
                                    }
                                    finally { gameMap.GemListLock.ExitWriteLock(); }
                                }
                            },
                            GameData.GemProduceTime,
                            () => 0
                        )
                        {
                            AllowTimeExceed = true
                        }.Start();
                    }
                )
                { IsBackground = true }.Start();
                
            }
            public void RemoveGem(Gem? gem)
            {
                if (gem != null)
                {
                    gameMap.GemListLock.EnterWriteLock();
                    try
                    {
                        gameMap.GemList.Remove(gem);
                    }
                    finally { gameMap.GemListLock.ExitWriteLock(); }
                }
            }
            public bool PickGem(Character player)
            {
                if (!player.IsAvailable)
                    return false;
                Gem? gem = null;
                gameMap.GemListLock.EnterReadLock();
                try
                {
                    foreach (Gem g in gameMap.GemList)
                    {
                        if (GameData.IsInTheSameCell(g.Position,player.Position))
                        {
                            gem = g;
                            break;
                        }
                    }
                }
                finally { gameMap.GemListLock.ExitReadLock(); }

                RemoveGem(gem);

                if (gem != null)
                {
                    player.GemNum += gem.Size;
                    return true;
                }
                return false;
            }

            public void ThrowGem(Character player, int moveMillisecondTime, double angle, int size=1)
            {
                if (!player.IsAvailable)
                    return;
                if (size > player.GemNum || size <= 0)
                    return;
                Gem gem = new Gem(player.Position, size);
                gameMap.GemListLock.EnterWriteLock();
                try
                {
                    gameMap.GemList.Add(gem);
                }
                finally { gameMap.GemListLock.ExitWriteLock(); }
                moveEngine.MoveObj(gem, moveMillisecondTime, angle);
            }

            public void UseGem(Character character, int num)
            {
                if (!character.IsAvailable)
                    return;
                if (num > character.GemNum)
                    num = character.GemNum;

                if(num > 0)
                {
                    character.GemNum -= num;
                    character.Score += GemToScore(num);
                }
            }


            /// <summary>
            /// 宝石转化为积分，有没有更好的函数？
            /// </summary>
            /// <param name="num"></param>
            /// <returns></returns>
            private int GemToScore(int num)
            {
                //先用分段线性
                if (num < 10)
                    return num * GameData.gemToScore;
                else if (num < 20)
                    return 2 * num * GameData.gemToScore;
                else return 3 * num * GameData.gemToScore;
            }

            public GemManager(Map gameMap)  //宝石不能扔过墙
            {
                this.gameMap = gameMap;
                this.moveEngine = new MoveEngine
                (
                    gameMap: gameMap,
                    OnCollision: (obj, collision, moveVec) =>
                    {
                        return MoveEngine.AfterCollision.MoveMax;
                    },
                    EndMove: obj =>
                     {
                         Debugger.Output(obj, " end move at " + obj.Position.ToString() + " At time: " + Environment.TickCount64);
                     }
                );

                gemWellList = new List<XYPosition>();
                for(int i=0; i<MapInfo.defaultMap.GetLength(0);i++)
                {
                    for(int j=0;j<MapInfo.defaultMap.GetLength(1);j++)
                    {
                        if(MapInfo.defaultMap[i,j]==(int)MapInfo.MapInfoObjType.GemWell)
                        {
                            gemWellList.Add(GameData.GetCellCenterPos(i, j));
                        }
                    }
                }
            }
        }
    }
}
