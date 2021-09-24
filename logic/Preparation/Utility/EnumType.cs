using Preparation.GameObj;
using Preparation.Interface;

namespace Preparation.Utility
{
    /// <summary>
    /// 存放所有用到的枚举类型
    /// </summary>
    public enum GameObjType
    {
        Character = 0,
        Wall = 1,
        Prop = 2,
        Bullet = 3,
        BirthPoint = 4,
        OutOfBoundBlock = 5,
        Grass = 6
    }
    public enum ShapeType
    {
        Circle = 0,      //仍然，子弹和人物为圆形，格子为方形
        Square = 1
    }
    public enum PlaceType  //位置标志，包括陆地，草丛，以及角色技能带来的隐身。
    {
        Land = 0,
        Grass1 = 1,
        Grass2 = 2,
        Grass3 = 3,
        Invisible = 4
    }
    public enum BulletType //子弹的类型
    {
        Bullet0 = 0,    //普通子弹
        Bullet1 = 1     //爆弾
    }
    public enum PropType    // 道具的类型
    {
        Null = 0,
        Accelerate = 1,
        plusAP = 2,
        minusCD = 3,
        addHP = 4,
        Shield = 5,
        addLIFE = 6,
        Spear = 7,
        Decelerate = 8,
        minusAP = 9,
        addCD = 10
    }
    public enum PassiveSkillType   // 被动技能
    {
        PSkill0 = 0,
        PSkill1 = 1,
        PSkill2 = 2,
        PSkill3 = 3,
        PSkill4 = 4,
        PSkill5 = 5
    }
    public enum ActiveSkillType    // 主动技能
    {
        ASkill0 = 0,
        ASkill1 = 1,
        ASkill2 = 2,
        ASkill3 = 3,
        ASkill4 = 4,
        ASkill5 = 5
    }
    /*public enum JobType : int   // 职业，貌似被废弃了。——LHR
    {
        Job0 = 0,
        Job1 = 1,
        Job2 = 2,
        Job3 = 3,
        Job4 = 4,
        Job5 = 5,
        Job6 = 6,
        InvalidJobType = int.MaxValue
    }*/
}
