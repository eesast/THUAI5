using GameClass.GameObj;

namespace GameClass.Skill
{
    public abstract class PassiveSkill
    {
         public abstract Bullet InitBullet { get; }
         public abstract void SkillEffect(Character player);
    }
    public abstract class CommonSkill
    {
        public abstract double AttackRange { get; }
        public abstract int MoveSpeed { get; }
        public abstract int MaxHp { get; }
        public abstract int CD { get; }
        public abstract int MaxBulletNum { get; }
        public abstract bool SkillEffect(Character player);
        public abstract int DurationTime { get; } //技能持续时间
        public abstract int SkillCD { get; }
    }
    //public abstract class UtimateSkill
    //{
    //    /*public abstract double AttackRange { get; }
    //    public abstract double BulletBombRange { get; }
    //    public abstract int MaxBulletNum { get; }
    //    public abstract int AP { get; }
    //    public abstract int MaxHp { get; }
    //    public abstract int MoveSpeed { get; }
    //    public abstract double BulletMoveSpeed { get; }*/
    //    public abstract bool SkillEffect(Character player);
    //    public abstract int CD { get; }
    //}
}
