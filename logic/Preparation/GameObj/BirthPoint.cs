using Preparation.Interface;
using Preparation.Utility;

namespace Preparation.GameObj
{
    /// <summary>
    /// 出生点
    /// </summary>
    public class BirthPoint : GameObj
    {
        public BirthPoint(XYPosition initPos, int radius) : base(initPos, radius, PlaceType.Land) 
        {
            this.CanMove = false;
            this.Type = GameObjType.BirthPoint;
        }
        public override bool IsRigid => false; // 与THUAI4不同，改成了非刚体
        protected override bool IgnoreCollideExecutor(IGameObj targetObj) => true;  // 出生点不与任何东西碰撞
        public override ShapeType Shape => ShapeType.Square; // 与THUAI4不同，改成了方形
    }
}
