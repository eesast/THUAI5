using Preparation.Utility;

namespace Preparation.Interface
{
    public interface IGameObj
    {
        public GameObjType Type { get; set; } //将THUAI4的getgameobjtype方法改为此接口属性
        public long ID { get; }
        public XYPosition Position { get; } // if Square, Pos equals the center
        public double FacingDirection { get; }
        public bool IsRigid { get; }
        public ShapeType Shape { get; }
        public bool CanMove { get; set; }
        public bool IsMoving { get; set; }
        public bool IsResetting { get; set; } // reviving
        public bool IsAvailable { get; }
        public int Radius { get; } // if Square, Radius equals half length of one side
        public PlaceType Place { get; set; }  

    }
}
