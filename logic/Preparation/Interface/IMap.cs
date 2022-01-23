using System.Collections.Generic;
using System.Threading;
using Preparation.Utility;

namespace Preparation.Interface
{
    public interface IMap
    {
        ITimer Timer { get; }
        List<ICharacter> PlayerList { get; }
        List<IObjOfCharacter> BulletList { get; }
        List<IGameObj> MapObjList { get; }
        ReaderWriterLockSlim PlayerListLock { get; }
        ReaderWriterLockSlim BulletListLock { get; }
        ReaderWriterLockSlim MapObjListLock { get; }
        public bool IsOutOfBound(IGameObj obj);
        public IOutOfBound GetOutOfBound(XYPosition pos); //返回新建的一个OutOfBound对象
    }
}
