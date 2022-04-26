using GameClass.GameObj;
using System.Threading;
using Preparation.Utility;
using Preparation.GameData;
using Timothy.FrameRateTask;

namespace GameClass.Skill
{
    public class BecomeVampire : ICommonSkill  //化身吸血鬼
    {
        private const int moveSpeed = GameData.basicMoveSpeed;
        public int MoveSpeed => moveSpeed;

        private const int maxHp = (int)(GameData.basicHp / 6 * 9.5);
        public int MaxHp => maxHp;

        private const int cd = GameData.basicCD;
        public int CD => cd;

        private const int maxBulletNum = GameData.basicBulletNum * 2 / 3;
        public int MaxBulletNum => maxBulletNum;

        // 以上参数以后再改
        public int SkillCD => GameData.commonSkillCD / 3 * 4;
        public int DurationTime => GameData.commonSkillTime;

        private readonly object commonSkillLock = new object();

        public bool SkillEffect(Character player)
        {
            lock (commonSkillLock)
            {
                if (player.TimeUntilCommonSkillAvailable == 0)
                {
                    player.TimeUntilCommonSkillAvailable = SkillCD;
                    new Thread
                    (() =>
                    {
                        player.Vampire += 0.5;
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

                        double tempVam = player.Vampire - 0.5;
                        player.Vampire = tempVam < player.OriVampire ? player.OriVampire : tempVam;
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
    }
    public class BecomeAssassin : ICommonSkill  //化身刺客，隐身
    {
        private const int moveSpeed = GameData.basicMoveSpeed / 3 * 5;
        public int MoveSpeed => moveSpeed;

        private const int maxHp = (int)(GameData.basicHp / 6 * 7.5);
        public int MaxHp => maxHp;

        private const int cd = GameData.basicCD;
        public int CD => cd;

        private const int maxBulletNum = GameData.basicBulletNum;
        public int MaxBulletNum => maxBulletNum;
        // 以上参数以后再改
        public int SkillCD => GameData.commonSkillCD;
        public int DurationTime => GameData.commonSkillTime / 10 * 6;

        private readonly object commonSkillLock = new object();
        public bool SkillEffect(Character player)
        {
            lock (commonSkillLock)
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
    }
    public class NuclearWeapon : ICommonSkill  //核武器
    {
        private const int moveSpeed = GameData.basicMoveSpeed / 6 * 4;
        public int MoveSpeed => moveSpeed;

        private const int maxHp = GameData.basicHp;
        public int MaxHp => maxHp;

        private const int cd = GameData.basicCD;
        public int CD => cd;

        private const int maxBulletNum = GameData.basicBulletNum * 2 / 3;
        public int MaxBulletNum => maxBulletNum;
        // 以上参数以后再改
        public int SkillCD => GameData.commonSkillCD / 3 * 7;
        public int DurationTime => GameData.commonSkillTime / 10;
        private readonly object commonSkillLock = new object();
        public bool SkillEffect(Character player)
        {
            lock (commonSkillLock)
            {
                if (player.TimeUntilCommonSkillAvailable == 0)
                {
                    player.TimeUntilCommonSkillAvailable = SkillCD;
                    new Thread
                    (() =>
                    {
                        BulletType b = player.BulletOfPlayer;
                        double oriAttackRange = player.AttackRange;
                        player.BulletOfPlayer = BulletType.AtomBomb;
                        player.SetAttackRange(2 * GameData.numOfPosGridPerCell);
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
                        player.SetAttackRange(oriAttackRange);
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
    }
    public class SuperFast : ICommonSkill  //3倍速
    {
        private const int moveSpeed = GameData.basicMoveSpeed * 4 / 3;
        public int MoveSpeed => moveSpeed;

        private const int maxHp = GameData.basicHp / 6 * 4;
        public int MaxHp => maxHp;

        private const int cd = GameData.basicCD;
        public int CD => cd;

        private const int maxBulletNum = GameData.basicBulletNum * 4 / 3;
        public int MaxBulletNum => maxBulletNum;
        // 以上参数以后再改
        public int SkillCD => GameData.commonSkillCD;
        public int DurationTime => GameData.commonSkillTime / 10 * 4;
        private readonly object commonSkillLock = new object();
        public bool SkillEffect(Character player)
        {
            lock (commonSkillLock)
            {
                if (player.TimeUntilCommonSkillAvailable == 0)
                {
                    player.TimeUntilCommonSkillAvailable = SkillCD;
                    new Thread
                    (() =>
                    {
                        player.AddMoveSpeed(this.DurationTime, 3.0);
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
