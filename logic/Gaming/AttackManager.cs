using System;
using System.Threading;
using Preparation.GameObj;
using Preparation.Utility;
using GameEngine;

namespace Gaming
{
    public class AttackManager
    {
        Map gameMap;
        MoveEngine moveEngine;
        private void BombOnePlayer(Bullet bullet,Character playerBeingShot)
        {
            if(playerBeingShot.BeAttack(bullet))
            {
                playerBeingShot.CanMove = false;
                playerBeingShot.IsResetting = true;
                gameMap.PlayerListLock.EnterWriteLock();
                try
                {
                    gameMap.PlayerList.Remove(playerBeingShot);
                }
                finally 
                { 
                    gameMap.PlayerListLock.ExitWriteLock(); 
                }
                playerBeingShot.Reset();
                ((Character?)bullet.Parent)?.AddScore(Constant.addScoreWhenKillOneLevelPlayer);  //给击杀者加分

                new Thread
                    (() =>
                    {

                        Thread.Sleep(Constant.deadRestoreTime);

                        playerBeingShot.AddShield(Constant.shieldTimeAtBirth);  //复活加个盾

                            gameMap.PlayerListLock.EnterWriteLock();
                        try
                        {
                            gameMap.PlayerList.Add(playerBeingShot);
                        }
                        finally { gameMap.PlayerListLock.ExitWriteLock(); }

                        if (gameMap.Timer.IsGaming)
                        {
                            playerBeingShot.CanMove = true;
                        }
                        playerBeingShot.IsResetting = false;
                    }
                    )
                { IsBackground = true }.Start();*/
            }
        }
        private void BulletBomb(Bullet bullet, GameObj? objBeingShot)
        {
            Debugger.Output(bullet, "bombed!");
            bullet.CanMove = false;
            gameMap.ObjListLock.EnterWriteLock();
            try
            {
                foreach (Obj obj in gameMap.ObjList)
                {
                    if (obj.ID == bullet.ID)
                    {
                        gameMap.ObjList.Remove(obj);
                        break;
                    }
                }
            }
            finally { gameMap.ObjListLock.ExitWriteLock(); }
            if(objBeingShot != null)
            {
                if(objBeingShot is Character)
                {
                    BombOnePlayer(bullet, (Character)objBeingShot);
                    bullet.Parent.HP = (int)(bullet.Parent.HP + bullet.Parent.Vampire * bullet.Ap);  //造成伤害根据吸血率来吸血
                }
                /*else if (objBeingShot is Bullet)        //子弹不能相互引爆，若要更改这一设定，取消注释即可。
                {
                    new Thread(() => { BulletBomb((Bullet)objBeingShot, null); }) { IsBackground = true }.Start();
                }*/

            }
            //子弹爆炸

        }
    }
}
