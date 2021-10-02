using GameClass.Skill;

namespace GameClass.GameObj
{
    public partial class Character
    {
        public object SkillLock => gameObjLock;
        private delegate bool ActiveSkill(Character player); //返回值：是否成功释放了技能
        private delegate void PassiveSkill(Character player);
        ActiveSkill commonSkill;
        public bool UseCommonSkill()
        {
            return commonSkill(this);
        }
        private int timeUntilCommonSkillAvailable = 0; //还剩多少时间可以使用普通技能
        public int TimeUntilCommonSkillAvailable
        {
            get => timeUntilCommonSkillAvailable;
            set
            {
                lock (SkillLock)
                    TimeUntilCommonSkillAvailable = value < 0 ? 0 : value;
            }
        }
        PassiveSkill passiveSkill;
        public void UsePassiveSkill()
        {
            passiveSkill(this);
            return;
        }
    }
}
