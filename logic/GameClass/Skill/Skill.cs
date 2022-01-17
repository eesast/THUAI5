using GameClass.GameObj;

namespace GameClass.Skill
{
    public interface IPassiveSkill
    {
        public Bullet InitBullet { get; }
        public void SkillEffect(Character player);
    }
    public interface ICommonSkill
    {
        public double AttackRange { get; }
        public int MoveSpeed { get; }
        public int MaxHp { get; }
        public int CD { get; }
        public int MaxBulletNum { get; }
        public bool SkillEffect(Character player);
        public int DurationTime { get; } //技能持续时间
        public int SkillCD { get; }
    }
    //public class UtimateSkill
    //{
    //    /*public double AttackRange { get; }
    //    public double BulletBombRange { get; }
    //    public int MaxBulletNum { get; }
    //    public int AP { get; }
    //    public int MaxHp { get; }
    //    public int MoveSpeed { get; }
    //    public double BulletMoveSpeed { get; }*/
    //    public bool SkillEffect(Character player);
    //    public int CD { get; }
    //}
}
