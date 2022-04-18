using System;
using System.Threading;

namespace CSharpInterface
{
    public partial class ClientInterface
    {
        public readonly ActiveSkillType activeSkillType = ActiveSkillType.NuclearWeapon;
        public readonly PassiveSkillType passiveSkillType = PassiveSkillType.RecoverAfterBattle;

        int[,] map = null;
        Random random = new Random();
        public void Play()
        {
            Thread.Sleep(10);
            var self = GetSelfInfo();
            var props = GetNoGemProps();
            foreach(var prop in props)
            {
                if (self.X / 1000 == prop.X / 1000 && self.Y / 1000 == prop.Y / 1000)
                {
                    Pick(PropType.Null);
                }
            }
        }
    }
}
