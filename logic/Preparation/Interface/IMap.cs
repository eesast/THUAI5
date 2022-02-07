using System.Collections.Generic;
using System.Threading;
using Preparation.Utility;
using System;

namespace Preparation.Interface
{
    public interface IMap
    {
        ITimer Timer { get; }
        Dictionary<String, IList<IGameObj>> GameObjDict { get; }
        Dictionary<String, ReaderWriterLockSlim> GameObjLockDict { get; }
        public bool IsOutOfBound(IGameObj obj);
        public IOutOfBound GetOutOfBound(XYPosition pos); //返回新建的一个OutOfBound对象
    }
}
