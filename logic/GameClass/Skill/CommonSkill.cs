using GameClass.GameObj;
using System.Threading;
using Preparation.Utility;
using Preparation.GameData;
using Timothy.FrameRateTask;

namespace GameClass.Skill
{
    public class BecomeVampire : ICommonSkill  //化身吸血鬼：1*标准技能cd，1*标准持续时间
    {
        private const int moveSpeed = GameData.basicMoveSpeed;
        public int MoveSpeed => moveSpeed;

        private const int maxHp = GameData.basicHp / 6 * 10;
        public int MaxHp => maxHp;

        private const int cd = GameData.basicCD;
        public int CD => cd;

        private const int maxBulletNum = GameData.basicBulletNum;
        public int MaxBulletNum => maxBulletNum;
        // 以上参数以后再改
        public int SkillCD => GameData.commonSkillCD;
        public int DurationTime => GameData.commonSkillTime;
        public bool SkillEffect(Character player)
        {
            if (player.TimeUntilCommonSkillAvailable == 0)
            {
                player.TimeUntilCommonSkillAvailable = SkillCD;
                new Thread
                (() =>
                {
                    player.Vampire += 1.0;
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

                    player.Vampire = player.oriVampire;
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
    public class BecomeAssassin : ICommonSkill  //化身刺客，隐身：1*标准技能cd，1*标准持续时间
    {
        private const int moveSpeed = GameData.basicMoveSpeed / 3 * 4;
        public int MoveSpeed => moveSpeed;

        private const int maxHp = GameData.basicHp / 6 * 5;
        public int MaxHp => maxHp;

        private const int cd = GameData.basicCD;
        public int CD => cd;

        private const int maxBulletNum = GameData.basicBulletNum;
        public int MaxBulletNum => maxBulletNum;
        // 以上参数以后再改
        public int SkillCD => GameData.commonSkillCD;
        public int DurationTime => GameData.commonSkillTime / 2;
        public bool SkillEffect(Character player)
        {
            if (player.TimeUntilCommonSkillAvailable == 0)
            {
                player.TimeUntilCommonSkillAvailable = SkillCD;
                new Thread
                (() =>
                {
                    player.IsInvisible = true;
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

                    player.IsInvisible = false;
                    Debugger.Output(player, "returns to normal.");

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
    public class NuclearWeapon : ICommonSkill  //核武器：1*标准技能cd，0.5*标准持续时间
    {
        private const int moveSpeed = GameData.basicMoveSpeed / 6 * 5;
        public int MoveSpeed => moveSpeed;

        private const int maxHp = GameData.basicHp;
        public int MaxHp => maxHp;

        private const int cd = GameData.basicCD;
        public int CD => cd;

        private const int maxBulletNum = GameData.basicBulletNum;
        public int MaxBulletNum => maxBulletNum;
        // 以上参数以后再改
        public int SkillCD => GameData.commonSkillCD;
        public int DurationTime => GameData.commonSkillTime / 5;
        public bool SkillEffect(Character player)
        {
            if (player.TimeUntilCommonSkillAvailable == 0)
            {
                player.TimeUntilCommonSkillAvailable = SkillCD;
                new Thread
                (() =>
                {
                    Bullet b = player.BulletOfPlayer.Clone(player);
                    player.BulletOfPlayer = new AtomBomb(player);
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

                    player.BulletOfPlayer = b;
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
    public class SuperFast : ICommonSkill  //3倍速：1*标准技能cd，1*标准持续时间
    {
        private const int moveSpeed = GameData.basicMoveSpeed;
        public int MoveSpeed => moveSpeed;

        private const int maxHp = (int)(0.6 * GameData.basicHp);
        public int MaxHp => maxHp;

        private const int cd = GameData.basicCD;
        public int CD => cd;

        private const int maxBulletNum = GameData.basicBulletNum;
        public int MaxBulletNum => maxBulletNum;
        // 以上参数以后再改
        public int SkillCD => GameData.commonSkillCD;
        public int DurationTime => GameData.commonSkillTime / 10 * 3;
        public bool SkillEffect(Character player)
        {
            if (player.TimeUntilCommonSkillAvailable == 0)
            {
                player.TimeUntilCommonSkillAvailable = SkillCD;
                new Thread
                (() =>
                {
                    player.SetMoveSpeed(3 * player.OrgMoveSpeed);
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

                    player.SetMoveSpeed(player.OrgMoveSpeed);
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
    public class NoCommonSkill : ICommonSkill  //这种情况不该发生，定义着以防意外
    {
        private const int moveSpeed = GameData.basicMoveSpeed;
        public int MoveSpeed => moveSpeed;

        private const int maxHp = GameData.basicHp;
        public int MaxHp => maxHp;

        private const int cd = GameData.basicCD;
        public int CD => cd;

        private const int maxBulletNum = GameData.basicBulletNum;
        public int MaxBulletNum => maxBulletNum;
        // 以上参数以后再改
        public int SkillCD => GameData.commonSkillCD;
        public int DurationTime => GameData.commonSkillTime;
        public bool SkillEffect(Character player)
        {
            return false;
        }
    }
}
