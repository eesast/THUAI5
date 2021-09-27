using System;
using Preparation.Utility;
using Preparation.GameObj;

namespace Preparation.Skill
{
    public abstract class PassiveSkill
    {
        public abstract double AttackRange { get; }
        public abstract double BulletBombRange { get; }
        public abstract int MaxBulletNum { get; }
        public abstract double BulletMoveSpeed { get; }
    }
    public abstract class CommonSkill
    {/*
        public abstract double AttackRange { get; }
        public abstract double BulletBombRange { get; }
        public abstract int MaxBulletNum { get; }
        public abstract int AP { get; }
        public abstract int MaxHp { get; }
        public abstract int MoveSpeed { get; }
        public abstract double BulletMoveSpeed { get; }*/
        public abstract void SkillEffect(Character player);
        public abstract int CD { get; }
    }
    public abstract class UtimateSkill
    {
        public abstract double AttackRange { get; }
        public abstract double BulletBombRange { get; }
        public abstract int MaxBulletNum { get; }
        public abstract int AP { get; }
        public abstract int MaxHp { get; }
        public abstract int MoveSpeed { get; }
        public abstract double BulletMoveSpeed { get; }
        public abstract void SkillEffect(Character player);
        public abstract int CD { get; }
    }
}
