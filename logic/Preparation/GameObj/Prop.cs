using Preparation.Interface;
using Preparation.Utility;

namespace Preparation.GameObj
{
    public abstract class Prop : GameObj	// 负责人LHR摆烂中...
    {
		public const int MinPropTypeNum = 1;
		public const int MaxPropTypeNum = 10;

		protected bool laid = false;	//道具是否已放置
		public bool Laid => laid;

		public override bool IsRigid => true;

		protected override bool IgnoreCollideExecutor(IGameObj targetObj) => true;	//道具不与任何东西碰撞

        public abstract PropType GetPropType();

		public void ResetPosition(XYPosition pos)
		{
			Position = pos;
		}
		public void ResetMoveSpeed(int newMoveSpeed)
		{
			MoveSpeed = newMoveSpeed;
		}

		public Prop(XYPosition initPos, int radius) : base(initPos, radius, PlaceType.Land) 
		{
			this.CanMove = false;
			this.Type = GameObjType.Prop;
		}
	}
}
