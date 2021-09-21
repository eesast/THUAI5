using Preparation.GameObj;
using Preparation.Interface;

namespace Preparation.Utility
{
    public enum GameObjType
    {
        Player = 0,
        Object = 1
    }
    public enum ShapeType
    {
        Circle = 0,      //仍然，子弹和人物为圆形，格子为方形
        Square = 1
    }
    public enum PlaceType  //位置标志，包括陆地，草丛，以及角色技能带来的隐身。
    {
        Land = 0,
        Grass1 = 1,
        Grass2 = 2,
        Grass3 = 3,
        Invisible = 4
    }
}
