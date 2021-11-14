using System.Collections.Generic;
using GameClass.GameObj;
using System.Threading;
using Preparation.GameData;
using Preparation.Utility;
using System;
using Timothy.FrameRateTask;
using GameEngine;

namespace Gaming
{
    public partial class Game
    {
        private readonly PropManager propManager;
        private class PropManager
        {
            private readonly Map gameMap;

            private MoveEngine moveEngine;

            private bool isProducingProp = false;

            private readonly List<XYPosition> availableCellForGenerateProp;
            public void StartProducing()
            {
                if (isProducingProp)
                    return;
                isProducingProp = true;
                ProduceProp();
            }

            public void UseProp(Character player)
            {
                if (!player.IsAvailable)
                    return;
                player.UseProp();
            }

            /// <summary>
            /// 
            /// </summary>
            /// <param name="player"></param>
            /// <param name="propType">若不指定，则自动判断可捡起什么道具</param>
            /// <returns></returns>
            public bool PickProp(Character player,PropType propType=PropType.Null)
            {
                Prop? pickProp = null;
                if(propType==PropType.Null)  //自动检查有无道具可捡
                {
                    gameMap.PropListLock.EnterReadLock();
                    try
                    {
                        foreach(Prop prop in gameMap.PropList)
                        {
                            if(GameData.IsInTheSameCell(prop.Position,player.Position))
                            {
                                pickProp = prop;
                            }
                        }
                    }
                    finally { gameMap.PropListLock.ExitReadLock(); }
                }
                else
                {
                    gameMap.PropListLock.EnterReadLock();
                    try
                    {
                        foreach(Prop prop in gameMap.PropList)
                        {
                            if(prop.GetPropType()==propType)
                            {
                                if (GameData.IsInTheSameCell(prop.Position, player.Position))
                                {
                                    pickProp = prop;
                                }
                            }
                        }
                    }
                    finally { gameMap.PropListLock.ExitReadLock(); }
                }

                if (pickProp != null)
                {
                    player.PropInventory = pickProp;
                    gameMap.PropListLock.EnterWriteLock();
                    try
                    {
                        gameMap.PropList.Remove(pickProp);
                    }
                    finally { gameMap.PropListLock.ExitWriteLock(); }
                    return true;
                }
                else return false;
            }

            public void ThrowProp(Character player,int timeInMilliseconds,double angle)
            {
                if (!gameMap.Timer.IsGaming)
                    return;
                if (player.PropInventory == null)
                    return;
                Prop prop = player.PropInventory;
                player.PropInventory = null;
                gameMap.PropListLock.EnterWriteLock();
                try
                {
                    gameMap.PropList.Add(prop);
                }
                finally { gameMap.PropListLock.ExitWriteLock(); }
                moveEngine.MoveObj(prop, timeInMilliseconds, angle);
            }
            private void ProduceProp()
            {
                int len = availableCellForGenerateProp.Count;
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
                                XYPosition randPos = availableCellForGenerateProp[rand];

                                gameMap.PropListLock.EnterWriteLock();
                                try
                                {
                                    switch (r.Next(0, 10))
                                    {
                                        case 0:
                                            gameMap.PropList.Add(new AddAP(randPos));
                                            break;
                                        case 1:
                                            gameMap.PropList.Add(new AddCD(randPos));
                                            break;
                                        case 2:
                                            gameMap.PropList.Add(new AddHP(randPos));
                                            break;
                                        case 3:
                                            gameMap.PropList.Add(new AddLIFE(randPos));
                                            break;
                                        case 4:
                                            gameMap.PropList.Add(new AddSpeed(randPos));
                                            break;
                                        case 5:
                                            gameMap.PropList.Add(new MinusAP(randPos));
                                            break;
                                        case 6:
                                            gameMap.PropList.Add(new MinusCD(randPos));
                                            break;
                                        case 7:
                                            gameMap.PropList.Add(new MinusSpeed(randPos));
                                            break;
                                        case 8:
                                            gameMap.PropList.Add(new Shield(randPos));
                                            break;
                                        case 9:
                                            gameMap.PropList.Add(new Spear(randPos));
                                            break;
                                        default:
                                            break;
                                    }
                                }
                                finally { gameMap.PropListLock.ExitWriteLock(); }
                            },
                            GameData.PropProduceTime,
                            () => 0
                        ).Start();
                    }
                )
                { IsBackground = true }.Start();
            }
            public PropManager(Map gameMap)  //道具不能扔过墙
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
                availableCellForGenerateProp = new List<XYPosition>();
                for (int i = 0; i < MapInfo.defaultMap.GetLength(0); i++)
                {
                    for (int j = 0; j < MapInfo.defaultMap.GetLength(1); j++)
                    {
                        if (   MapInfo.defaultMap[i, j] == (int)MapInfo.MapInfoObjType.Null
                            || MapInfo.defaultMap[i, j] == (int)MapInfo.MapInfoObjType.Grass1
                            || MapInfo.defaultMap[i, j] == (int)MapInfo.MapInfoObjType.Grass2
                            || MapInfo.defaultMap[i, j] == (int)MapInfo.MapInfoObjType.Grass3 )
                        {
                            availableCellForGenerateProp.Add(GameData.GetCellCenterPos(i, j));
                        }
                    }
                }
            }
        }
    }
}
