using System;
using System.Collections.Generic;
using System.Threading;
using Preparation.Interface;

namespace Preparation.GameObj
{
    public class Map:IMap
    {
        private List<ICharacter> playerList;
        public List<ICharacter> PlayerList => playerList;
        
        private readonly ReaderWriterLockSlim playerListLock;
        public ReaderWriterLockSlim PlayerListLock => playerListLock;

        private List<IObjOfCharacter> objList;
        public List<IObjOfCharacter> ObjList => objList;

        private readonly ReaderWriterLockSlim objListLock;
        public ReaderWriterLockSlim ObjListLock => objListLock;

    }
}
