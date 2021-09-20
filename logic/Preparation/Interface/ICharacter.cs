using System;

namespace Preparation.Interface
{
    public interface ICharacter:IGameObj
    {
        public long TeamID { get; }
    }
}
