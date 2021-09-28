using Preparation.GameObj;
using System.Threading;
using Preparation.Utility;
using Preparation.GameData;

namespace Preparation.Skill 
{
    public class BecomeVampire: CommonSkill  //化身吸血鬼：1*标准技能cd，1*标准持续时间
    {
        public override int CD => Constant.commonSkillCD;
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
                    Debugger.Output(player, "become vampire!");
                    Thread.Sleep(Constant.commonSkillTime); 
                    lock (player.SkillLock)
                    {
                        player.Vampire = 0;
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
        public override int CD => Constant.commonSkillCD;
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
                    Debugger.Output(player, "become assassin!");
                    Thread.Sleep(Constant.commonSkillTime); 
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
        public override int CD => Constant.commonSkillCD;
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
                        player.BulletOfPlayer = new AtomBomb(player, Constant.bulletRadius, b.MoveSpeed, (int)(1.5 * b.AP));
                        player.IsCommonSkillAvailable = false;
                    }
                    Debugger.Output(player, "become assassin!");
                    Thread.Sleep(Constant.commonSkillTime / 2);
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
}
