using System;
using Preparation.Utility;

namespace Preparation.Interface
{
    public interface ICharacter:IGameObj
    {
        object PropLock { get; }
        public long TeamID { get; }
        public int HP { get; set; }
        public double Vampire { get; }

    }
}
