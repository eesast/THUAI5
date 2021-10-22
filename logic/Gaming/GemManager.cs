using System;
using System.Collections.Generic;
using GameClass.GameObj;

namespace Gaming
{
    public partial class Game
    {
        private readonly GemManager gemManager;
        private class GemManager  //单独用GemManager处理宝石
        {
            private readonly Map gameMap;
            private readonly bool isProducing = false;
            public void StartProducingGem()
            {
                if (isProducing)
                    return;
                
                /*
                 自动生成宝石。
                宝石的生成应该分为两类：
                1、宝石井附近生成。
                2、地图上随机生成。
                 */



            }

            /*
             还应该有throw宝石，pick宝石，use宝石的操作。

            throw宝石：可以指定throw出的宝石个数，扔出去的是一个“宝石块”。“n-宝石块”——即人物捡起后，宝石数量会增加n，而不只是增加1。
            宝石块可以在宝石类中定义。宝石类可以这样设计，设置一个属性：人物捡起后增加多少宝石。普通的宝石人物捡起后只增加1宝石，而宝石块可以增加指定数目的宝石。


            use宝石是将一定数量的宝石直接转化为得分。
             */

            public GemManager(Map gameMap)
            {
                this.gameMap = gameMap;
            }
        }
    }
}
