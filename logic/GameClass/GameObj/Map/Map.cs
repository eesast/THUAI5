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
        private readonly Dictionary<uint, BirthPoint> birthPointList;   // 出生点列表
        public Dictionary<uint, BirthPoint> BirthPointList => birthPointList;

        private Dictionary<String, IList<IGameObj>> gameObjDict;
        public Dictionary<String, IList<IGameObj>> GameObjDict => gameObjDict;
        private Dictionary<String, ReaderWriterLockSlim> gameObjLockDict;
        public Dictionary<String, ReaderWriterLockSlim> GameObjLockDict => gameObjLockDict;

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
            gameObjLockDict["player"].EnterReadLock();
            try
            {
                foreach (Character person in gameObjDict["player"])
                {
                    if (playerID == person.ID)
                    {
                        player = person;
                        break;
                    }
                }
            }
            finally { gameObjLockDict["player"].ExitReadLock(); }
            return player;
        }
        public Map(uint[,] mapResource)
        {
            //the two dicts must have same keys
            gameObjDict = new Dictionary<string, IList<IGameObj>>();
            gameObjLockDict = new Dictionary<string, ReaderWriterLockSlim>();
            gameObjDict.Add("player", new List<IGameObj>());
            gameObjLockDict.Add("player", new ReaderWriterLockSlim());
            gameObjDict.Add("bullet", new List<IGameObj>());
            gameObjLockDict.Add("bullet", new ReaderWriterLockSlim());
            gameObjDict.Add("prop", new List<IGameObj>());
            gameObjLockDict.Add("prop", new ReaderWriterLockSlim());
            gameObjDict.Add("gem", new List<IGameObj>());
            gameObjLockDict.Add("gem", new ReaderWriterLockSlim());
            gameObjDict.Add("map", new List<IGameObj>());
            gameObjLockDict.Add("map", new ReaderWriterLockSlim());

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
                                GameObjLockDict["map"].EnterWriteLock();
                                try
                                {
                                    lock (GameObjLockDict["map"])
                                    {
                                        GameObjDict["map"].Add(new Wall(GameData.GetCellCenterPos(i, j)));
                                    }
                                }
                                finally { GameObjLockDict["map"].ExitWriteLock(); }
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
                                GameObjLockDict["map"].EnterWriteLock();
                                try
                                {
                                    lock (GameObjLockDict["map"])
                                    {
                                        GameObjDict["map"].Add(newBirthPoint);
                                    }
                                }
                                finally { GameObjLockDict["map"].ExitWriteLock(); }
                                break;
                            }
                    }
                }
            }
            
        }
    }
}
