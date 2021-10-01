using System.Collections.Generic;
using System.Threading;
using Preparation.Interface;
using Preparation.Utility;
using Preparation.GameData;

namespace GameClass.GameObj
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
            return MapInfo.defaultMap[pos.x / GameData.numOfPosGridPerCell, pos.y / GameData.numOfPosGridPerCell] == 1;
        }
        public bool IsOutOfBound(IGameObj obj)
        {
            return obj.Position.x >= GameData.lengthOfMap - obj.Radius
                || obj.Position.x <= obj.Radius
                || obj.Position.y >= GameData.lengthOfMap - obj.Radius
                || obj.Position.y <= obj.Radius;
        }
        public IOutOfBound GetOutOfBound(XYPosition pos)
        {
            return new OutOfBoundBlock(pos);
        }
        public IGameObj GetCell(XYPosition pos)
        {
            if (MapInfo.defaultMap[pos.x / GameData.numOfPosGridPerCell, pos.y / GameData.numOfPosGridPerCell] == 1)
                return new Wall(pos);
            else if (MapInfo.defaultMap[pos.x / GameData.numOfPosGridPerCell, pos.y / GameData.numOfPosGridPerCell] == 2)
                return new Grass1(pos);
            else if (MapInfo.defaultMap[pos.x / GameData.numOfPosGridPerCell, pos.y / GameData.numOfPosGridPerCell] == 3)
                return new Grass2(pos);
            else if (MapInfo.defaultMap[pos.x / GameData.numOfPosGridPerCell, pos.y / GameData.numOfPosGridPerCell] == 4)
                return new Grass3(pos);
            else return new OutOfBoundBlock(pos);
        }
    }
}
