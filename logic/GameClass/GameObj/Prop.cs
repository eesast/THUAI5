﻿using Preparation.Interface;
using Preparation.Utility;

namespace GameClass.GameObj
{
    public abstract class Prop : ObjOfCharacter
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
    /// 坑人地雷
    /// </summary>
    public abstract class DebuffMine : Prop
    {
        public DebuffMine(XYPosition initPos, int radius) : base(initPos, radius) { }
    }
    #region 所有增益道具
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
    #endregion
    #region 所有坑人地雷
    /// <summary>
    /// 减速
    /// </summary>
    public sealed class MinusSpeed : DebuffMine
    {
        public MinusSpeed(XYPosition initPos, int radius) : base(initPos, radius) { }
        public override PropType GetPropType() => PropType.minusSpeed;
    }
    /// <summary>
    /// 减少攻击力
    /// </summary>
    public sealed class MinusAP : DebuffMine
    {
        public MinusAP(XYPosition initPos, int radius) : base(initPos, radius) { }
        public override PropType GetPropType() => PropType.minusAP;
    }
    /// <summary>
    /// 增加冷却
    /// </summary>
    public sealed class AddCD : DebuffMine
    {
        public AddCD(XYPosition initPos, int radius) : base(initPos, radius) { }
        public override PropType GetPropType() => PropType.addCD;
    }
    /// <summary>
    /// 宝石块
    /// </summary>
    public sealed class GemBlock : BuffProp
    {
        public GemBlock(XYPosition initPos, int radius, int size = 1) : base(initPos, radius)
        {
            this.size = size;
        }
        public override PropType GetPropType() => PropType.Gem;

        private int size; //宝石块大小
        public int Size
        {
            get => size;
            set
            {
                size = value >= 1 ? value : 1;
            }
        }
    }
    #endregion
}
