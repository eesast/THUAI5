using System.Collections.Generic;
using System.Threading;
using Preparation.Interface;
using Preparation.Utility;
using Preparation.GameData;

namespace Preparation.GameObj
{
    public partial class Map:IMap
    {
        private List<ICharacter> playerList;
        public List<ICharacter> PlayerList => playerList;
        
        private readonly ReaderWriterLockSlim playerListLock;
        public ReaderWriterLockSlim PlayerListLock => playerListLock;

        private List<IObjOfCharacter> objList;
        public List<IObjOfCharacter> ObjList => objList;

        private readonly ReaderWriterLockSlim objListLock;
        public ReaderWriterLockSlim ObjListLock => objListLock;
        public bool IsWall(XYPosition pos)
        {
            return MapInfo.defaultMap[pos.x / Constant.numOfPosGridPerCell, pos.y / Constant.numOfPosGridPerCell] == 1;
        }
        public bool OutOfBound(XYPosition pos)
        {
            return pos.x >= Constant.lengthOfMap || pos.x <= 0 || pos.y >= Constant.lengthOfMap || pos.y <= 0;
        }
        public IOutOfBound GetOutOfBound(XYPosition pos)
        {
            return new OutOfBoundBlock(pos);
        }
        public IGameObj GetCell(XYPosition pos)
        {
            if (MapInfo.defaultMap[pos.x / Constant.numOfPosGridPerCell, pos.y / Constant.numOfPosGridPerCell] == 1)
                return new Wall(pos);
            else if (MapInfo.defaultMap[pos.x / Constant.numOfPosGridPerCell, pos.y / Constant.numOfPosGridPerCell] == 2)
                return new Grass1(pos);
            else if (MapInfo.defaultMap[pos.x / Constant.numOfPosGridPerCell, pos.y / Constant.numOfPosGridPerCell] == 3)
                return new Grass2(pos);
            else if (MapInfo.defaultMap[pos.x / Constant.numOfPosGridPerCell, pos.y / Constant.numOfPosGridPerCell] == 4)
                return new Grass3(pos);
            else return new OutOfBoundBlock(pos);
        }
    }
}
