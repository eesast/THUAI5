using System.Collections.Generic;
using System.Threading;
using Preparation.Interface;
using Preparation.Utility;
using Preparation.GameData;
using System;

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

        private readonly List<IGameObj> mapObjList;
        public List<IGameObj> MapObjList => mapObjList;
        private readonly ReaderWriterLockSlim mapObjListLock;
        public ReaderWriterLockSlim MapObjListLock => mapObjListLock;

        private readonly List<IObjOfCharacter> propList;
        public List<IObjOfCharacter> PropList => propList;
        private readonly ReaderWriterLockSlim propListLock;
        public ReaderWriterLockSlim PropListLock => propListLock;

        private readonly List<IObjOfCharacter> gemList;
        public List<IObjOfCharacter> GemList => gemList;
        private readonly ReaderWriterLockSlim gemListLock;
        public ReaderWriterLockSlim GemListLock => gemListLock;

        private readonly Dictionary<uint, BirthPoint> birthPointList;   // 出生点列表
        public Dictionary<uint, BirthPoint> BirthPointList => birthPointList;

        public readonly uint[,] ProtoGameMap;
        public PlaceType GetPlaceType(GameObj obj)
        {
            uint type = ProtoGameMap[obj.Position.x / GameData.numOfPosGridPerCell, obj.Position.y / GameData.numOfPosGridPerCell];
            if (type == 1)
                return PlaceType.Grass1;
            else if (type == 2)
                return PlaceType.Grass2;
            else if (type == 3)
                return PlaceType.Grass3;
            else
                return PlaceType.Land;  //其他情况均返回land
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

        public PlaceType GetPlaceType(XYPosition pos)
        {
            switch (ProtoGameMap[pos.x / GameData.numOfPosGridPerCell, pos.y / GameData.numOfPosGridPerCell])
            {
                case 0:
                case 1:
                case 5:
                case 6:
                case 7:
                case 8:
                case 9:
                case 10:
                case 11:
                case 12:
                case 13:
                    return PlaceType.Land;
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
            try
            {
                foreach (Character person in playerList)
                {
                    if (playerID == person.ID)
                    {
                        player = person;
                        break;
                    }
                }
            }
            finally { playerListLock.ExitReadLock(); }
            return player;
        }
        public Map(uint[,] mapResource)
        {
            //创建列表
            bulletList = new List<IObjOfCharacter>();
            playerList = new List<ICharacter>();
            propList = new List<IObjOfCharacter>();
            gemList = new List<IObjOfCharacter>();
            mapObjList = new List<IGameObj>();
            bulletListLock = new ReaderWriterLockSlim();
            playerListLock = new ReaderWriterLockSlim();
            propListLock = new ReaderWriterLockSlim();
            gemListLock = new ReaderWriterLockSlim();
            mapObjListLock = new ReaderWriterLockSlim();

            ProtoGameMap = new uint[mapResource.GetLength(0), mapResource.GetLength(1)];
            Array.Copy(mapResource, ProtoGameMap, mapResource.Length);
            //for(int i = 0;i< mapResource.GetLength(0); i++)
            //{
            //    for(int j=0;j< mapResource.GetLength(1);j++)
            //    {
            //        Console.Write($" {ProtoGameMap[i, j]}");
            //    }
            //    Console.WriteLine("");
            //}

            birthPointList = new Dictionary<uint, BirthPoint>(MapInfo.numOfBirthPoint);

            //将出生点插入
            for (int i = 0; i < GameData.rows; ++i)
            {
                for (int j = 0; j < GameData.cols; ++j)
                {
                    switch (mapResource[i, j])
                    {
                        case (uint)MapInfo.MapInfoObjType.Wall:
                            {
                                mapObjListLock.EnterWriteLock();
                                try
                                {
                                    lock (mapObjListLock)
                                    {
                                        mapObjList.Add(new Wall(GameData.GetCellCenterPos(i, j)));
                                    }
                                }
                                finally { mapObjListLock.ExitWriteLock(); }
                                break;
                            }
                        case (uint)MapInfo.MapInfoObjType.BirthPoint1:
                        case (uint)MapInfo.MapInfoObjType.BirthPoint2:
                        case (uint)MapInfo.MapInfoObjType.BirthPoint3:
                        case (uint)MapInfo.MapInfoObjType.BirthPoint4:
                        case (uint)MapInfo.MapInfoObjType.BirthPoint5:
                        case (uint)MapInfo.MapInfoObjType.BirthPoint6:
                        case (uint)MapInfo.MapInfoObjType.BirthPoint7:
                        case (uint)MapInfo.MapInfoObjType.BirthPoint8:
                            {
                                BirthPoint newBirthPoint = new BirthPoint(GameData.GetCellCenterPos(i, j));
                                birthPointList.Add(MapInfo.BirthPointEnumToIdx((MapInfo.MapInfoObjType)mapResource[i, j]), newBirthPoint);
                                mapObjListLock.EnterWriteLock();
                                try
                                {
                                    lock (mapObjListLock)
                                    {
                                        mapObjList.Add(newBirthPoint);
                                    }
                                }
                                finally { mapObjListLock.ExitWriteLock(); }
                                break;
                            }
                    }
                }
            }
        }
    }
}
