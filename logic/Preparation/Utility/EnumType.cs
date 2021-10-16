
namespace Preparation.Utility
{
    /// <summary>
    /// 存放所有用到的枚举类型
    /// </summary>
    public enum GameObjType
    {
        Null = 0,
        Character = 1,
        Wall = 2,
        Prop = 3,
        Bullet = 4,
        BirthPoint = 5,
        OutOfBoundBlock = 6,
        Grass = 7
    }
    public enum ShapeType
    {
        Null = 0,
        Circle = 1,      //仍然，子弹和人物为圆形，格子为方形
        Square = 2
    }
    public enum PlaceType  //位置标志，包括陆地，草丛，以及角色技能带来的隐身。游戏中每一帧都要刷新各个物体的该属性
    {
        Null = 0,
        Land = 1,
        Grass1 = 2,
        Grass2 = 3,
        Grass3 = 4,
        Invisible = 5
    }
    public enum BulletType //工厂方式
    {
        Null = 0,
        Bullet0 = 1,    //普通子弹
        AtomBomb = 2     //原子弹
    }
    public enum PropType    // 道具的类型
    {
        Null = 0,
        addHP = 1,
        addAP = 2,
        addSpeed = 3,
        addLIFE = 4,
        minusCD = 5,
        Gem = 6,    // 新增：宝石
        Shield = 7,
        Spear = 8,
        minusSpeed = 9,
        minusAP = 10,
        addCD = 11,
    }
    public enum PassiveSkillType   // 被动技能
    {
        Null = 0,
        RecoverAfterBattle = 1,
        SpeedUpWhenLeavingGrass = 2,
        Vampire = 3,
        PSkill3 = 4,
        PSkill4 = 5,
        PSkill5 = 6
    }
    public enum ActiveSkillType    // 主动技能
    {
        Null = 0,
        BecomeVampire = 1,
        BecomeAssassin = 2,
        NuclearWeapon = 3,
        SuperFast = 4,
        ASkill4 = 5,
        ASkill5 = 6
    }
    public enum BuffType    //buff
    {
        Null = 0,
        MoveSpeed = 1,
        AP = 2,
        CD = 3,
        AddLIFE = 4,
        Shield = 5,
        Spear = 6
    }
    /*public enum JobType : int   // 职业，废弃。——LHR
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
