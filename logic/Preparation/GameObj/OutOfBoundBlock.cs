using Preparation.Interface;
using Preparation.Utility;

namespace Preparation.GameObj
{
    /// <summary>
    /// 逻辑墙
    /// </summary>
    public class OutOfBoundBlock: GameObj, IOutOfBound
    {
        public OutOfBoundBlock(XYPosition initPos) : base(initPos, int.MaxValue, PlaceType.Land) 
        {
            this.CanMove = false;
            this.Type = GameObjType.OutOfBoundBlock;
        }

        public override bool IsRigid => true;
        public override ShapeType Shape => ShapeType.Square;
    }
}
