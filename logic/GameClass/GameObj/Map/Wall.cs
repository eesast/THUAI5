using Preparation.Utility;
using Preparation.GameData;

namespace GameClass.GameObj
{
    /// <summary>
    /// 墙体
    /// </summary>
    public class Wall : GameObj
    {
        public Wall(XYPosition initPos) : base(initPos, GameData.numOfPosGridPerCell, PlaceType.Land)
        {
            this.CanMove = false;
            this.Type = GameObjType.Wall;
        }
        public override bool IsRigid => true;
        public override ShapeType Shape => ShapeType.Square;
    }
}
