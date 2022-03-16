using System;

namespace CSharpInterface
{
    public partial class ClientInterface
    {
        public readonly ActiveSkillType activeSkillType = ActiveSkillType.NuclearWeapon;
        public readonly PassiveSkillType passiveSkillType = PassiveSkillType.RecoverAfterBattle;

        int[,] map = null;
        int direction = 0; // 0 1 2 3：上左下右
        Random random = new Random();
        public void Play()
        {
            if (map == null)
                map = GetMap();
            var self = GetSelfInfo();
            if(direction == 0)
            {
                if (map[self.X / 1000 - 1, self.Y / 1000] == 1 || (map[self.X / 1000 - 1, self.Y / 1000] >= 5 && map[self.X / 1000 - 1, self.Y / 1000] <= 12))
                    direction = 1;
                else
                {
                    MoveUp(50);
                }
            }
            if (direction == 1)
            {
                if (map[self.X / 1000, self.Y / 1000 - 1] == 1 || (map[self.X / 1000, self.Y / 1000 - 1] >= 5 && map[self.X / 1000, self.Y / 1000 - 1] <= 12))
                    direction = 2;
                else
                {
                    MoveLeft(50);
                }
            }
            if (direction == 2)
            {
                if (map[self.X / 1000 + 1, self.Y / 1000] == 1 || (map[self.X / 1000 + 1, self.Y / 1000] >= 5 && map[self.X / 1000 + 1, self.Y / 1000] <= 12))
                    direction = 3;
                else
                {
                    MoveDown(50);
                }
            }
            if (direction == 3)
            {
                if (map[self.X / 1000, self.Y / 1000 + 1] == 1 || (map[self.X / 1000, self.Y / 1000 + 1] >= 5 && map[self.X / 1000, self.Y / 1000 + 1] <= 12))
                    direction = 0;
                else
                {
                    MoveRight(50);
                }
            }
            if (random.Next(14) == 13)
                Attack(random.NextDouble() * Math.PI * 2);
        }
    }
}
