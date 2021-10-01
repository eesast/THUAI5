using GameClass.GameObj;
using System.Threading;
using Preparation.Utility;
using Preparation.GameData;

namespace GameClass.Skill 
{
    public class BecomeVampire: CommonSkill  //化身吸血鬼：1*标准技能cd，1*标准持续时间
    {
        public override int CD => GameData.commonSkillCD;
        public override bool SkillEffect(Character player)
        {
            if (player.IsCommonSkillAvailable)
            {
                new Thread
                (() =>
                {
                    lock (player.SkillLock)
                    {
                        player.Vampire = 1.0;
                        player.IsCommonSkillAvailable = false;
                    }
                    Debugger.Output(player, "becomes vampire!");
                    Thread.Sleep(GameData.commonSkillTime); 
                    lock (player.SkillLock)
                    {
                        player.Vampire = player.oriVampire;
                    }
                    Debugger.Output(player, "return to normal.");
                    Thread.Sleep(CD);
                    lock(player.SkillLock)
                    {
                        player.IsCommonSkillAvailable = true;
                    }
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
    public class BecomeAssassin: CommonSkill  //化身刺客，隐身：1*标准技能cd，1*标准持续时间
    {
        public override int CD => GameData.commonSkillCD;
        public override bool SkillEffect(Character player)
        {
            if (player.IsCommonSkillAvailable)
            {
                new Thread
                (() =>
                {
                    lock (player.SkillLock)
                    {
                        player.Place = PlaceType.Invisible;
                        player.IsCommonSkillAvailable = false;
                    }
                    Debugger.Output(player, "becomes assassin!");
                    Thread.Sleep(GameData.commonSkillTime); 
                    lock (player.SkillLock)
                    {
                        player.Place = MapInfo.GetPlaceType(player);
                    }
                    Debugger.Output(player, "return to normal.");
                    Thread.Sleep(CD);
                    lock(player.SkillLock)
                    {
                        player.IsCommonSkillAvailable = true;
                    }
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
        public override bool SkillEffect(Character player)
        {
            if (player.IsCommonSkillAvailable)
            {
                new Thread
                (() =>
                {
                    Bullet b = player.BulletOfPlayer;
                    lock (player.SkillLock)
                    {
                        player.BulletOfPlayer = new AtomBomb(player, GameData.bulletRadius, b.MoveSpeed, (int)(1.5 * b.AP));
                        player.IsCommonSkillAvailable = false;
                    }
                    Debugger.Output(player, "uses atombomb!");
                    Thread.Sleep(GameData.commonSkillTime / 2);
                    lock (player.SkillLock)
                    {
                        player.BulletOfPlayer = b;
                    }
                    Debugger.Output(player, "return to normal.");
                    Thread.Sleep(CD);
                    lock (player.SkillLock)
                    {
                        player.IsCommonSkillAvailable = true;
                    }
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
        public override bool SkillEffect(Character player)
        {
            if (player.IsCommonSkillAvailable)
            {
                new Thread
                (() =>
                {
                    lock (player.SkillLock)
                    {
                        player.SetMoveSpeed(3 * player.OrgMoveSpeed);
                        player.IsCommonSkillAvailable = false;
                    }
                    Debugger.Output(player, "moves very fast!");
                    Thread.Sleep(GameData.commonSkillTime);
                    lock (player.SkillLock)
                    {
                        player.SetMoveSpeed(player.OrgMoveSpeed);
                    }
                    Debugger.Output(player, "return to normal.");
                    Thread.Sleep(CD);
                    lock (player.SkillLock)
                    {
                        player.IsCommonSkillAvailable = true;
                    }
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
