using Preparation.Utility;
using Preparation.GameData;
using Preparation.Interface;

namespace GameClass.GameObj
{
    public class GemWell : GameObj
    {
        public GemWell(XYPosition initPos) : base(initPos, GameData.numOfPosGridPerCell, PlaceType.Land)
        {
            this.CanMove = false;
            this.Type = GameObjType.GemWell;
        }
        public override bool IsRigid => false;
        protected override bool IgnoreCollideExecutor(IGameObj targetObj) => true;	//草丛不与任何东西碰撞
        public override ShapeType Shape => ShapeType.Square;
    }
}
