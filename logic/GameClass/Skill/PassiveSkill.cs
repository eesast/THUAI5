using System;
using System.Threading;
using GameClass.GameObj;
using Preparation.GameData;
using Preparation.Utility;
using Timothy.FrameRateTask;

namespace GameClass.Skill  //被动技能开局时就释放，持续到游戏结束
{
    public class RecoverAfterBattle:PassiveSkill  //脱战回血
    {
        public override void SkillEffect(Character player)
        {
            const int recoverDegree = 1;  //每帧回复血量
            int nowHP = player.HP;
            int lastHP = nowHP;
            long waitTime = 0;
            const long interval = 20000; //每隔interval时间不受伤害，角色即开始回血
            new Thread
            (
                () =>
                {
                    new FrameRateTaskExecutor<int>
                    (
                        () => true,
                        ()=>
                        {
                            lastHP = nowHP;  //lastHP等于上一帧的HP
                            nowHP = player.HP;  //nowHP更新为这一帧的HP
                            if (lastHP > nowHP)  //这一帧扣血了
                            {
                                waitTime = 0;
                            }
                            else if (waitTime < interval)
                            {
                                waitTime += GameData.frameDuration;
                            }

                            if (waitTime >= interval)  //回复时，每帧(50ms)回复1，即1s回复20。
                                player.TryAddHp(recoverDegree);
                        },
                        timeInterval: GameData.frameDuration,
                        () => 0,
                        maxTotalDuration: GameData.gameDuration
                    )
                    {
                        AllowTimeExceed = true,
                        MaxTolerantTimeExceedCount = ulong.MaxValue,
                        TimeExceedAction = b =>
                        {
                            if (b) Console.WriteLine("Fetal Error: The computer runs so slow that passive skill time exceeds!!!!!!");

#if DEBUG
                            else
                            {
                                Console.WriteLine("Debug info: passive skill time exceeds for once.");
                            }
#endif
                        }
                    }.Start();
                }
            )
            { IsBackground=true }.Start();
        }
    }
    public class SpeedUpWhenLeavingGrass:PassiveSkill //出草丛时加速并免疫减速，但隐身时出草丛不会有该效果
    {
        public override void SkillEffect(Character player)
        {
            PlaceType nowPlace = player.Place;
            PlaceType lastPlace = nowPlace;
            bool beginSpeedUp = false;
            int speedUpTimes = 0;
            const int maxSpeedUpTimes = (int)(2000 / GameData.frameDuration); //加速时间：2s
            new Thread
            (
                () =>
                {
                    new FrameRateTaskExecutor<int>
                    (
                        () => true,
                        () =>
                        {
                            lastPlace = nowPlace;
                            nowPlace = player.Place;
                            if ((lastPlace == PlaceType.Grass1 || lastPlace == PlaceType.Grass2 || lastPlace == PlaceType.Grass3) && nowPlace == PlaceType.Land)
                            {
                                beginSpeedUp = true;
                                speedUpTimes = 0;
                            }
                            if(beginSpeedUp)
                            {
                                if (speedUpTimes <= maxSpeedUpTimes)
                                {
                                    speedUpTimes++;
                                    player.SetMoveSpeed(player.OrgMoveSpeed * 3);  //3倍速
                                }
                                else
                                {
                                    speedUpTimes = 0;
                                    beginSpeedUp = false;
                                    player.SetMoveSpeed(player.OrgMoveSpeed);
                                }
                            }
                        },
                        timeInterval: GameData.frameDuration,
                        () => 0,
                        maxTotalDuration: GameData.gameDuration
                    )
                    {
                        AllowTimeExceed = true,
                        MaxTolerantTimeExceedCount = ulong.MaxValue,
                        TimeExceedAction = b =>
                        {
                            if (b) Console.WriteLine("Fetal Error: The computer runs so slow that passive skill time exceeds!!!!!!");

#if DEBUG
                            else
                            {
                                Console.WriteLine("Debug info: passive skill time exceeds for once.");
                            }
#endif
                        }
                    }.Start();
                }
            )
            { IsBackground = true }.Start();
        }
    }
    public class Vampire:PassiveSkill  //被动就是吸血
    {
        public override void SkillEffect(Character player)
        {
            player.oriVampire = 0.1;
        }
    }
}
