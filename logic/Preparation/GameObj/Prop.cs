using Preparation.Interface;
using Preparation.Utility;

namespace Preparation.GameObj
{
    public abstract class Prop : ObjOfCharacter //LHR摆烂中...抽象类及所有增益道具已写完
    {
        protected bool laid = false;
        public bool Laid => laid;   // 道具是否放置在地图上

        public override bool IsRigid => true;

        protected override bool IgnoreCollideExecutor(IGameObj targetObj) => true;	//道具不与任何东西碰撞

        public override ShapeType Shape => ShapeType.Square;

        public abstract PropType GetPropType();

        public Prop(XYPosition initPos, int radius) : base(initPos, radius, PlaceType.Land) 
        {
            this.CanMove = false;
            this.Type = GameObjType.Prop;
        }
    }
    /// <summary>
    /// 增益道具
    /// </summary>
    public abstract class BuffProp : Prop
    {
        public BuffProp(XYPosition initPos, int radius) : base(initPos, radius) { }
    }
    /// <summary>
    /// 增加HP
    /// </summary>
    public sealed class AddHP : BuffProp
    {
        public AddHP(XYPosition initPos, int radius) : base(initPos, radius) { }
        public override PropType GetPropType() => PropType.addHP;
    }
    /// <summary>
    /// 增加AP
    /// </summary>
    public sealed class AddAP : BuffProp
    {
        public AddAP(XYPosition initPos, int radius) : base(initPos, radius) { }
        public override PropType GetPropType() => PropType.addAP;
    }
    /// <summary>
    /// 增加速度
    /// </summary>
    public sealed class AddSpeed : BuffProp
    {
        public AddSpeed(XYPosition initPos, int radius) : base(initPos, radius) { }
        public override PropType GetPropType() => PropType.addSpeed;
    }
    /// <summary>
    /// 复活甲
    /// </summary>
    public sealed class AddLIFE : BuffProp
    {
        public AddLIFE(XYPosition initPos, int radius) : base(initPos, radius) { }
        public override PropType GetPropType() => PropType.addLIFE;
    }
    /// <summary>
    /// 减少冷却
    /// </summary>
    public sealed class MinusCD : BuffProp
    {
        public MinusCD(XYPosition initPos, int radius) : base(initPos, radius) { }
        public override PropType GetPropType() => PropType.minusCD;
    }
    /// <summary>
    /// 宝石
    /// </summary>
    public sealed class Gem : BuffProp
    {
        public Gem(XYPosition initPos, int radius) : base(initPos, radius) { }
        public override PropType GetPropType() => PropType.Gem;
    }
    /// <summary>
    /// 护盾
    /// </summary>
    public sealed class Shield : BuffProp
    {
        public Shield(XYPosition initPos, int radius) : base(initPos, radius) { }
        public override PropType GetPropType() => PropType.Shield;
    }
    /// <summary>
    /// 矛
    /// </summary>
    public sealed class Spear : BuffProp
    {
        public Spear(XYPosition initPos, int radius) : base(initPos, radius) { }
        public override PropType GetPropType() => PropType.Spear;
    }
}
