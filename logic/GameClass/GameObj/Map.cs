using System.Collections.Generic;
using System.Threading;
using Preparation.Interface;
using Preparation.Utility;
using Preparation.GameData;

namespace GameClass.GameObj
{
    public partial class Map : IMap
    {
        private readonly List<ICharacter> playerList;
        public List<ICharacter> PlayerList => playerList;

        private readonly ReaderWriterLockSlim playerListLock;
        public ReaderWriterLockSlim PlayerListLock => playerListLock;

        private readonly List<IObjOfCharacter> bulletList;
        public List<IObjOfCharacter> BulletList => bulletList;

        private readonly ReaderWriterLockSlim bulletListLock;
        public ReaderWriterLockSlim BulletListLock => bulletListLock;

        private readonly List<IObjOfCharacter> propList;
        public List<IObjOfCharacter> PropList => propList;

        private readonly ReaderWriterLockSlim propListLock;
        public ReaderWriterLockSlim PropListLock => propListLock;

        private readonly Dictionary<uint, BirthPoint> birthPointList;   // 出生点列表
        public Dictionary<uint, BirthPoint> BirthPointList => birthPointList;
        // 出生点列表暂不需要锁
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
        public PlaceType GetPlaceType(XYPosition pos)
        {
            switch (MapInfo.defaultMap[pos.x / GameData.numOfPosGridPerCell, pos.y / GameData.numOfPosGridPerCell])
            {
                case 0:
                    return PlaceType.Land;
                case 1:
                    return PlaceType.Null;
                case 2:
                    return PlaceType.Grass1;
                case 3:
                    return PlaceType.Grass2;
                case 4:
                    return PlaceType.Grass3;
                default:
                    return PlaceType.Null;
            }
        }
        public Character? FindPlayer(long playerID)
        {
            Character? player = null;
            playerListLock.EnterReadLock();
            foreach (Character person in playerList)
            {
                if (playerID == person.ID)
                {
                    player = person;
                    break;
                }
            }
            return player;
        }
        public Map(uint[,] mapResource)
        {
            //创建列表
            bulletList = new List<IObjOfCharacter>();
            playerList = new List<ICharacter>();
            propList = new List<IObjOfCharacter>();
            bulletListLock = new ReaderWriterLockSlim();
            playerListLock = new ReaderWriterLockSlim();
            propListLock = new ReaderWriterLockSlim();

            //birthPointList = new Dictionary<uint, BirthPoint>(MapInfo.numOfBirthPoint);

            ////将墙等游戏对象插入到游戏中
            //for (int i = 0; i < rows; ++i)
            //{
            //    for (int j = 0; j < cols; ++j)
            //    {
            //        switch (mapResource[i, j])
            //        {
            //            case (uint)MapInfo.MapInfoObjType.BirthPoint1:
            //            case (uint)MapInfo.MapInfoObjType.BirthPoint2:
            //            case (uint)MapInfo.MapInfoObjType.BirthPoint3:
            //            case (uint)MapInfo.MapInfoObjType.BirthPoint4:
            //            case (uint)MapInfo.MapInfoObjType.BirthPoint5:
            //            case (uint)MapInfo.MapInfoObjType.BirthPoint6:
            //            case (uint)MapInfo.MapInfoObjType.BirthPoint7:
            //            case (uint)MapInfo.MapInfoObjType.BirthPoint8:
            //                {
            //                    BirthPoint newBirthPoint = new BirthPoint(GameData.GetCellCenterPos(i, j));
            //                    birthPointList.Add(MapInfo.BirthPointEnumToIdx((MapInfo.MapInfoObjType)mapResource[i, j]), newBirthPoint);
            //                    break;
            //                }
            //        }
            //    }
            //}
        }
    }
}
