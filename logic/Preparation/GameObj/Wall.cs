using Preparation.Interface;
using Preparation.Utility;

namespace Preparation.GameObj
{
    /// <summary>
    /// 墙体
    /// </summary>
    public class Wall : GameObj
    {
        public Wall(XYPosition initPos, int radius) : base(initPos, radius, PlaceType.Land) 
        {
            this.CanMove = false;
            this.Type = GameObjType.Wall;
        }
        public override bool IsRigid => true;
        public override ShapeType Shape => ShapeType.Square;
    }
}
