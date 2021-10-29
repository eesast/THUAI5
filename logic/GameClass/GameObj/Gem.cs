using Preparation.Interface;
using Preparation.Utility;
using Preparation.GameData;

namespace GameClass.GameObj
{
    public sealed class Gem:Prop
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
        protected override bool IgnoreCollideExecutor(IGameObj targetObj) => true;	//道具不与任何东西碰撞
    }
}
