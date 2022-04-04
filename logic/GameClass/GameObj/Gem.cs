using Preparation.Interface;
using Preparation.Utility;
using Preparation.GameData;

namespace GameClass.GameObj
{
    public sealed class Gem: Prop  //宝石算作一种特殊的道具
    {
        public override PropType GetPropType()
        {
            return PropType.Gem;
        }
        public Gem(XYPosition initPos,int size = 1) : base(initPos) 
        {
            this.size = size;
        }
        /// <summary>
        /// 一个宝石块的大小
        /// </summary>
        private int size = 1;
        public int Size => size;

        public void TryAddGemSize(int addSize=1)
        {
            if (this.size >= GameData.MaxGemSize)
                return;
            else this.size++;
        }
        public override ShapeType Shape => ShapeType.Circle;
        protected override bool IgnoreCollideExecutor(IGameObj targetObj)
        {
            if (targetObj.Type == GameObjType.BirthPoint || targetObj.Type == GameObjType.Prop || targetObj.Type == GameObjType.Bullet || targetObj.Type == GameObjType.Character) return true;
            return false;
        }
    }
}
