using Preparation.Interface;
using Preparation.Utility;
using Preparation.GameData;

namespace Preparation.GameObj
{
    /// <summary>
    /// 草丛
    /// </summary>
    public abstract class Grass : GameObj
    {
        public Grass(XYPosition initPos) : base(initPos, Constant.numOfPosGridPerCell, PlaceType.Land) 
        {
            this.CanMove = false;
            this.Type = GameObjType.Grass;
        }
        public override bool IsRigid => false;
        protected override bool IgnoreCollideExecutor(IGameObj targetObj) => true;	//草丛不与任何东西碰撞
        public override ShapeType Shape => ShapeType.Square;
    }
    public class Grass1:Grass
    {
        public Grass1(XYPosition initPos) : base(initPos) 
        {
            this.Place = PlaceType.Grass1;
        }
    }
    public class Grass2 : Grass
    {
        public Grass2(XYPosition initPos) : base(initPos)
        {
            this.Place = PlaceType.Grass2;
        }
    }
    public class Grass3 : Grass
    {
        public Grass3(XYPosition initPos) : base(initPos)
        {
            this.Place = PlaceType.Grass1;
        }
    }
}
