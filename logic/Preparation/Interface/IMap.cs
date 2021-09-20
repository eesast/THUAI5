using System.Collections.Generic;
using System.Threading;

namespace Preparation.Interface
{
    public interface IMap:IGameObj
    {
        ITimer Timer { get; }
        List<ICharacter> PlayerList { get; }
        List<IObjOfCharacter> ObjList { get; }
        ReaderWriterLockSlim PlayerListLock { get; }
        ReaderWriterLockSlim ObjListLock { get; }
    }
}
