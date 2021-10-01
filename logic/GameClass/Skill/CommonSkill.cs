using GameClass.GameObj;
using System.Threading;
using Preparation.Utility;
using Preparation.GameData;
using Timothy.FrameRateTask;

namespace GameClass.Skill
{
    public class BecomeVampire : CommonSkill  //化身吸血鬼：1*标准技能cd，1*标准持续时间
    {
        public override int CD => GameData.commonSkillCD;
        public override int DurationTime => GameData.commonSkillTime;
        public override bool SkillEffect(Character player)
        {
            if (player.TimeUntilCommonSkillAvailable == 0)
            {
                player.TimeUntilCommonSkillAvailable = CD;
                new Thread
                (() =>
                {
                    lock (player.SkillLock)
                    {
                        player.Vampire = 1.0;
                    }
                    Debugger.Output(player, "becomes vampire!");

                    new FrameRateTaskExecutor<int>
                    (
                        () => true,
                        () =>
                        {
                            player.TimeUntilCommonSkillAvailable -= (int)GameData.frameDuration;
                        },
                        timeInterval: GameData.frameDuration,
                        () => 0,
                        maxTotalDuration: (long)(DurationTime)
                    )
                    {
                        AllowTimeExceed = true,
                        MaxTolerantTimeExceedCount = ulong.MaxValue,
                    }.Start();

                    lock (player.SkillLock)
                    {
                        player.Vampire = player.oriVampire;
                    }
                    Debugger.Output(player, "return to normal.");

                    new FrameRateTaskExecutor<int>
                     (
                         () => player.TimeUntilCommonSkillAvailable > 0,
                         () =>
                         {
                             player.TimeUntilCommonSkillAvailable -= (int)GameData.frameDuration;
                         },
                         timeInterval: GameData.frameDuration,
                         () => 0,
                         maxTotalDuration: (long)(CD - DurationTime)
                     )
                    {
                        AllowTimeExceed = true,
                        MaxTolerantTimeExceedCount = ulong.MaxValue,
                    }.Start();

                    player.TimeUntilCommonSkillAvailable = 0;
                    Debugger.Output(player, "CommonSkill is ready.");
                }
                )
                { IsBackground = true }.Start();

                return true;
            }
            else
            {
                Debugger.Output(player, "CommonSkill is cooling down!");
                return false;
            }
        }
    }
    public class BecomeAssassin : CommonSkill  //化身刺客，隐身：1*标准技能cd，1*标准持续时间
    {
        public override int CD => GameData.commonSkillCD;
        public override int DurationTime => GameData.commonSkillTime;
        public override bool SkillEffect(Character player)
        {
            if (player.TimeUntilCommonSkillAvailable == 0)
            {
                player.TimeUntilCommonSkillAvailable = CD;
                new Thread
                (() =>
                {
                    lock (player.SkillLock)
                    {
                        player.Place = PlaceType.Invisible;
                    }
                    Debugger.Output(player, "becomes assassin!");
                    new FrameRateTaskExecutor<int>
                    (
                        () => true,
                        () =>
                        {
                            player.TimeUntilCommonSkillAvailable -= (int)GameData.frameDuration;
                        },
                        timeInterval: GameData.frameDuration,
                        () => 0,
                        maxTotalDuration: (long)(DurationTime)
                    )
                    {
                        AllowTimeExceed = true,
                        MaxTolerantTimeExceedCount = ulong.MaxValue,
                    }.Start();

                    lock (player.SkillLock)
                    {
                        player.Place = MapInfo.GetPlaceType(player);
                    }
                    Debugger.Output(player, "return to normal.");

                    new FrameRateTaskExecutor<int>
                     (
                         () => player.TimeUntilCommonSkillAvailable > 0,
                         () =>
                         {
                             player.TimeUntilCommonSkillAvailable -= (int)GameData.frameDuration;
                         },
                         timeInterval: GameData.frameDuration,
                         () => 0,
                         maxTotalDuration: (long)(CD - DurationTime)
                     )
                    {
                        AllowTimeExceed = true,
                        MaxTolerantTimeExceedCount = ulong.MaxValue,
                    }.Start();

                    player.TimeUntilCommonSkillAvailable = 0;
                    Debugger.Output(player, "CommonSkill is ready.");
                }
                )
                { IsBackground = true }.Start();

                return true;
            }
            else
            {
                Debugger.Output(player, "CommonSkill is cooling down!");
                return false;
            }
        }
    }
    public class NuclearWeapon : CommonSkill  //核武器：1*标准技能cd，0.5*标准持续时间
    {
        public override int CD => GameData.commonSkillCD;
        public override int DurationTime => GameData.commonSkillTime / 2;
        public override bool SkillEffect(Character player)
        {
            if (player.TimeUntilCommonSkillAvailable == 0)
            {
                player.TimeUntilCommonSkillAvailable = CD;
                new Thread
                (() =>
                {
                    Bullet b = player.BulletOfPlayer;
                    lock (player.SkillLock)
                    {
                        player.BulletOfPlayer = new AtomBomb(player, GameData.bulletRadius, b.MoveSpeed, (int)(1.5 * b.AP));
                    }
                    Debugger.Output(player, "uses atombomb!");
                    new FrameRateTaskExecutor<int>
                    (
                        () => true,
                        () =>
                        {
                            player.TimeUntilCommonSkillAvailable -= (int)GameData.frameDuration;
                        },
                        timeInterval: GameData.frameDuration,
                        () => 0,
                        maxTotalDuration: (long)(DurationTime)
                    )
                    {
                        AllowTimeExceed = true,
                        MaxTolerantTimeExceedCount = ulong.MaxValue,
                    }.Start();

                    lock (player.SkillLock)
                    {
                        player.BulletOfPlayer = b;
                    }
                    Debugger.Output(player, "return to normal.");

                    new FrameRateTaskExecutor<int>
                     (
                         () => player.TimeUntilCommonSkillAvailable > 0,
                         () =>
                         {
                             player.TimeUntilCommonSkillAvailable -= (int)GameData.frameDuration;
                         },
                         timeInterval: GameData.frameDuration,
                         () => 0,
                         maxTotalDuration: (long)(CD - DurationTime)
                     )
                    {
                        AllowTimeExceed = true,
                        MaxTolerantTimeExceedCount = ulong.MaxValue,
                    }.Start();

                    player.TimeUntilCommonSkillAvailable = 0;
                    Debugger.Output(player, "CommonSkill is ready.");
                }
                )
                { IsBackground = true }.Start();

                return true;
            }
            else
            {
                Debugger.Output(player, "CommonSkill is cooling down!");
                return false;
            }
        }
    }

    public class SuperFast : CommonSkill  //3倍速：1*标准技能cd，1*标准持续时间
    {
        public override int CD => GameData.commonSkillCD;
        public override int DurationTime => GameData.commonSkillTime;
        public override bool SkillEffect(Character player)
        {
            if (player.TimeUntilCommonSkillAvailable == 0)
            {
                player.TimeUntilCommonSkillAvailable = CD;
                new Thread
                (() =>
                {
                    lock (player.SkillLock)
                    {
                        player.SetMoveSpeed(3 * player.OrgMoveSpeed);
                    }
                    Debugger.Output(player, "moves very fast!");
                    new FrameRateTaskExecutor<int>
                    (
                        () => true,
                        () =>
                        {
                            player.TimeUntilCommonSkillAvailable -= (int)GameData.frameDuration;
                        },
                        timeInterval: GameData.frameDuration,
                        () => 0,
                        maxTotalDuration: (long)(DurationTime)
                    )
                    {
                        AllowTimeExceed = true,
                        MaxTolerantTimeExceedCount = ulong.MaxValue,
                    }.Start();

                    lock (player.SkillLock)
                    {
                        player.SetMoveSpeed(player.OrgMoveSpeed);
                    }
                    Debugger.Output(player, "return to normal.");

                    new FrameRateTaskExecutor<int>
                     (
                         () => player.TimeUntilCommonSkillAvailable > 0,
                         () =>
                         {
                             player.TimeUntilCommonSkillAvailable -= (int)GameData.frameDuration;
                         },
                         timeInterval: GameData.frameDuration,
                         () => 0,
                         maxTotalDuration: (long)(CD - DurationTime)
                     )
                    {
                        AllowTimeExceed = true,
                        MaxTolerantTimeExceedCount = ulong.MaxValue,
                    }.Start();

                    player.TimeUntilCommonSkillAvailable = 0;
                    Debugger.Output(player, "CommonSkill is ready.");
                }
                )
                { IsBackground = true }.Start();

                return true;
            }
            else
            {
                Debugger.Output(player, "CommonSkill is cooling down!");
                return false;
            }
        }
    }
}
