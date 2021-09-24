using Preparation.Utility;

namespace Preparation.Interface
{
    public enum PlaceType  //位置标志，包括陆地，草丛，以及角色技能带来的隐身。
    {
        Land = 0,
        Grass1 = 1,
        Grass2 = 2,
        Grass3 = 3,
        Invisible = 4
    }
    public enum ShapeType
    {
        Circle = 0,      //仍然，子弹和人物为圆形，格子为方形
        Square = 1
    }
    public interface IGameObj
    {
        public long ID { get; }
        public XYPosition Position { get; } //if Square, Pos equals the center
        public double FacingDirection { get; }
        public bool IsRigid { get; }
        public ShapeType Shape { get; }
        public bool CanMove { get; set; }
        public bool IsMoving { get; set; }
        public bool IsResetting { get; set; } //reviving
        public bool IsAvailable { get; } 
        public int Radius { get; } //if Square, Radius equals half length of one side
        public PlaceType Place { get; set; }  

    }
}
