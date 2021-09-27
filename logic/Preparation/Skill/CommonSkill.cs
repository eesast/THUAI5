using Preparation.GameObj;
using System.Threading;
using Preparation.Utility;

namespace Preparation.Skill
{
    public class BecomeVampire:CommonSkill  //化身吸血鬼
    {
        public override int CD => 30000;
        public override void SkillEffect(Character player)
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
                    Thread.Sleep(10000);
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
            }
            else
            {
                Debugger.Output(player, "CommonSkill is cooling down!");
            }
        }
    }
    public class BecomeAssassin:CommonSkill  //化身刺客，隐身
    {
        public override int CD => 30000;
        public override void SkillEffect(Character player)
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
                    Thread.Sleep(10000);
                    lock (player.SkillLock)
                    {
                        player.Place = player.GetPlaceType();
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
            }
            else
            {
                Debugger.Output(player, "CommonSkill is cooling down!");
            }
        }
    }
}
