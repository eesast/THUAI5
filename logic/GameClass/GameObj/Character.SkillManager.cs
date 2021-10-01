using GameClass.Skill;

namespace GameClass.GameObj
{
    public partial class Character
    {
        private delegate bool ActiveSkill(Character player); //返回值：是否成功释放了技能
        private delegate void PassiveSkill(Character player);
        ActiveSkill commonSkill;
        public bool UseCommonSkill()
        {
            return commonSkill(this);
        }
        private bool isCommonSkillAvailable = true; //普通技能可用标志
        public bool IsCommonSkillAvailable
        {
            get => isCommonSkillAvailable;
            set
            {
                lock (gameObjLock)
                    isCommonSkillAvailable = value;
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
