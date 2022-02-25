using System;

namespace CSharpInterface
{
    public partial class ClientInterface
    {
        public readonly ActiveSkillType activeSkillType = ActiveSkillType.NuclearWeapon;
        public readonly PassiveSkillType passiveSkillType = PassiveSkillType.RecoverAfterBattle;
        public void Play()
        {
            var chas = GetCharacters();
            var me = GetSelfInfo();
            var map = GetMap();
            bool f = map[41, 42] == 1;
            if(f)
            {

            }
            else
            {

            }
            foreach (var ch in chas)
            {
                if (((ch.X - me.X) * (ch.X - me.X) + (ch.Y - me.Y) * (ch.Y - me.Y)) <= 10000 * 10000)
                {
                    Attack(Math.Atan2(ch.Y - me.Y, ch.X - me.X));
                }
            }
        }
    }
}
