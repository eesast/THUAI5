using GameClass.GameObj;
using System.Threading;
using Preparation.Utility;
using Preparation.GameData;
using Timothy.FrameRateTask;

namespace GameClass.Skill
{
    public class BecomeVampire : CommonSkill  //化身吸血鬼：1*标准技能cd，1*标准持续时间
    {
        private const double attackRange = GameData.basicAttackRange;
        public override double AttackRange => attackRange;

        private const int moveSpeed = GameData.basicMoveSpeed;
        public override int MoveSpeed => moveSpeed;

        private const int maxHp = GameData.basicHp;
        public override int MaxHp => maxHp;

        private const int cd = GameData.basicCD;
        public override int CD => cd;

        private const int maxBulletNum = GameData.basicBulletNum;
        public override int MaxBulletNum => maxBulletNum;
        // 以上参数以后再改
        public override int SkillCD => GameData.commonSkillCD;
        public override int DurationTime => GameData.commonSkillTime;
        public override bool SkillEffect(Character player)
        {
            if (player.TimeUntilCommonSkillAvailable == 0)
            {
                player.TimeUntilCommonSkillAvailable = SkillCD;
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
                        () => !player.IsResetting,
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
                         () => player.TimeUntilCommonSkillAvailable > 0 && !player.IsResetting,
                         () =>
                         {
                             player.TimeUntilCommonSkillAvailable -= (int)GameData.frameDuration;
                         },
                         timeInterval: GameData.frameDuration,
                         () => 0,
                         maxTotalDuration: (long)(SkillCD - DurationTime)
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
        private const double attackRange = GameData.basicAttackRange;
        public override double AttackRange => attackRange;

        private const int moveSpeed = GameData.basicMoveSpeed;
        public override int MoveSpeed => moveSpeed;

        private const int maxHp = GameData.basicHp;
        public override int MaxHp => maxHp;

        private const int cd = GameData.basicCD;
        public override int CD => cd;

        private const int maxBulletNum = GameData.basicBulletNum;
        public override int MaxBulletNum => maxBulletNum;
        // 以上参数以后再改
        public override int SkillCD => GameData.commonSkillCD;
        public override int DurationTime => GameData.commonSkillTime;
        public override bool SkillEffect(Character player)
        {
            if (player.TimeUntilCommonSkillAvailable == 0)
            {
                player.TimeUntilCommonSkillAvailable = SkillCD;
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
                        () => !player.IsResetting,
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
                         () => player.TimeUntilCommonSkillAvailable > 0 && !player.IsResetting,
                         () =>
                         {
                             player.TimeUntilCommonSkillAvailable -= (int)GameData.frameDuration;
                         },
                         timeInterval: GameData.frameDuration,
                         () => 0,
                         maxTotalDuration: (long)(SkillCD - DurationTime)
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
        private const double attackRange = GameData.basicAttackRange;
        public override double AttackRange => attackRange;

        private const int moveSpeed = GameData.basicMoveSpeed;
        public override int MoveSpeed => moveSpeed;

        private const int maxHp = GameData.basicHp;
        public override int MaxHp => maxHp;

        private const int cd = GameData.basicCD;
        public override int CD => cd;

        private const int maxBulletNum = GameData.basicBulletNum;
        public override int MaxBulletNum => maxBulletNum;
        // 以上参数以后再改
        public override int SkillCD => GameData.commonSkillCD;
        public override int DurationTime => GameData.commonSkillTime / 2;
        public override bool SkillEffect(Character player)
        {
            if (player.TimeUntilCommonSkillAvailable == 0)
            {
                player.TimeUntilCommonSkillAvailable = SkillCD;
                new Thread
                (() =>
                {
                    Bullet b = player.BulletOfPlayer.Clone(player);
                    lock (player.SkillLock)
                    {
                        player.BulletOfPlayer = new AtomBomb(player, GameData.bulletRadius, b.MoveSpeed, (int)(1.5 * b.AP));
                    }
                    Debugger.Output(player, "uses atombomb!");
                    new FrameRateTaskExecutor<int>
                    (
                        () => !player.IsResetting,
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
                         () => player.TimeUntilCommonSkillAvailable > 0 && !player.IsResetting,
                         () =>
                         {
                             player.TimeUntilCommonSkillAvailable -= (int)GameData.frameDuration;
                         },
                         timeInterval: GameData.frameDuration,
                         () => 0,
                         maxTotalDuration: (long)(SkillCD - DurationTime)
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
        private const double attackRange = GameData.basicAttackRange;
        public override double AttackRange => attackRange;

        private const int moveSpeed = GameData.basicMoveSpeed;
        public override int MoveSpeed => moveSpeed;

        private const int maxHp = GameData.basicHp;
        public override int MaxHp => maxHp;

        private const int cd = GameData.basicCD;
        public override int CD => cd;

        private const int maxBulletNum = GameData.basicBulletNum;
        public override int MaxBulletNum => maxBulletNum;
        // 以上参数以后再改
        public override int SkillCD => GameData.commonSkillCD;
        public override int DurationTime => GameData.commonSkillTime;
        public override bool SkillEffect(Character player)
        {
            if (player.TimeUntilCommonSkillAvailable == 0)
            {
                player.TimeUntilCommonSkillAvailable = SkillCD;
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
                        () => !player.IsResetting,
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
                         () => player.TimeUntilCommonSkillAvailable > 0 && !player.IsResetting,
                         () =>
                         {
                             player.TimeUntilCommonSkillAvailable -= (int)GameData.frameDuration;
                         },
                         timeInterval: GameData.frameDuration,
                         () => 0,
                         maxTotalDuration: (long)(SkillCD - DurationTime)
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
    public class NoCommonSkill : CommonSkill  //这种情况不该发生，定义着以防意外
    {
        private const double attackRange = GameData.basicAttackRange;
        public override double AttackRange => attackRange;

        private const int moveSpeed = GameData.basicMoveSpeed;
        public override int MoveSpeed => moveSpeed;

        private const int maxHp = GameData.basicHp;
        public override int MaxHp => maxHp;

        private const int cd = GameData.basicCD;
        public override int CD => cd;

        private const int maxBulletNum = GameData.basicBulletNum;
        public override int MaxBulletNum => maxBulletNum;
        // 以上参数以后再改
        public override int SkillCD => GameData.commonSkillCD;
        public override int DurationTime => GameData.commonSkillTime;
        public override bool SkillEffect(Character player)
        {
            return false;
        }
    }
}
