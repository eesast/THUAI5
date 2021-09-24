using System;
using Preparation.Interface;
using Preparation.Utility;

namespace Preparation.GameObj
{
    public abstract partial class Character : GameObj, ICharacter	//负责人LHR摆烂中...
    {
        public const int basicAp = 1000;
        public const int basicHp = 6000;
        public const int basicCD = 1000;

		private long teamID = long.MaxValue;
		public long TeamID
		{
			get => teamID;
			set
			{
				lock (gameObjLock)
				{
					teamID = value;
					//Debugger.Output(this, " joins in the tream: " + value.ToString());
				}
			}
		}
	}
}
