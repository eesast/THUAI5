using Preparation.Interface;
using Preparation.Utility;

namespace GameClass.GameObj
{
    /// <summary>
    /// 所有物，玩家可以存进“物品栏”的东西
    /// </summary>
    public abstract class ObjOfCharacter : GameObj, IObjOfCharacter
    {
        private ICharacter? parent = null;  //道具的主人
        public ICharacter? Parent
        {
            get => parent;
            set
            {
                lock (gameObjLock)
                {
                    parent = value;
                }
            }
        }
        // LHR注：本来考虑在构造函数里设置parent属性，见THUAI4在游戏引擎中才设置该属性，作罢。——2021/9/24
        public ObjOfCharacter(XYPosition initPos, int initRadius, PlaceType initPlace) : base(initPos, initRadius, initPlace) { }
    }
}
