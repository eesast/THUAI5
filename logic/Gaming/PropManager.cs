using System;
using GameClass.GameObj;
using System.Threading;

namespace Gaming
{
    public partial class Game
    {
        private readonly PropManager propManager;
        private class PropManager
        {
            readonly Map gameMap;
            private readonly bool isProducingProp = false;
            public void StartProducing()
            {
                if (isProducingProp)
                    return;

                //可能没必要
                //gameMap.PropListLock.EnterWriteLock();
                //try
                //{
                //    gameMap.PropList.Clear();
                //}
                //finally { gameMap.PropListLock.ExitWriteLock(); }

                new Thread
                (
                    () =>
                    {
                        //产生道具，调用ProduceOneProp


                    }
                ).Start();
            }

            private void ProduceOneProp()
            {
                //随机产生道具


            }
            public PropManager(Map gameMap)
            {
                this.gameMap = gameMap;
            }
        }
    }
}
