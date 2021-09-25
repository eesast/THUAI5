using Preparation.Interface;
using Preparation.Utility;

namespace Preparation.GameObj
{
    public abstract class Prop : ObjOfCharacter //LHR摆烂中...抽象类已写完
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
}
