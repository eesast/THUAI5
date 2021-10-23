using System;
using System.Threading;
using System.Collections.Generic;
using GameClass.GameObj;
using Preparation.Utility;
using Preparation.GameData;
using Timothy.FrameRateTask;
using Preparation.Interface;

namespace Gaming
{
    public partial class Game
    {
        public struct PlayerInitInfo
        {
            public uint birthPointIndex;
            public long teamID;
            public PassiveSkillType passiveSkill;
            public ActiveSkillType commonSkill;
            public PlayerInitInfo(uint birthPointIndex, long teamID, PassiveSkillType passiveSkill, ActiveSkillType commonSkill)
            {
                this.birthPointIndex = birthPointIndex;
                this.teamID = teamID;
                this.passiveSkill = passiveSkill;
                this.commonSkill = commonSkill;
            }
        }

        private readonly List<Team> teamList;
        public List<Team> TeamList => teamList;
        private readonly Map gameMap;
        public Map GameMap => gameMap;
        private readonly int numOfTeam;
        public long AddPlayer(PlayerInitInfo playerInitInfo)
        {
            if (!Team.teamExists(playerInitInfo.teamID))
                /*  || !MapInfo.ValidBirthPointIdx(playerInitInfo.birthPointIdx)
                  || gameMap.BirthPointList[playerInitInfo.birthPointIdx].Parent != null)*/
                return GameObj.invalidID;

            XYPosition pos = gameMap.BirthPointList[playerInitInfo.birthPointIndex].Position;
            //Console.WriteLine($"x,y: {pos.x},{pos.y}");
            Character newPlayer = new(pos, GameData.characterRadius, gameMap.GetPlaceType(pos), playerInitInfo.passiveSkill, playerInitInfo.commonSkill);
            gameMap.BirthPointList[playerInitInfo.birthPointIndex].Parent = newPlayer;
            gameMap.PlayerListLock.EnterWriteLock();
            try
            {
                gameMap.PlayerList.Add(newPlayer);
            }
            finally
            {
                gameMap.PlayerListLock.ExitWriteLock();
            }
            //Console.WriteLine($"Playerlist length:{gameMap.PlayerList.Count}");
            teamList[(int)playerInitInfo.teamID].AddPlayer(newPlayer);
            newPlayer.TeamID = playerInitInfo.teamID;

            new Thread  //检查人物位置，同时装子弹。
            (
                () =>
                {
                    while (!gameMap.Timer.IsGaming)
                        Thread.Sleep(newPlayer.CD);
                    long lastTime = Environment.TickCount64;
                    new FrameRateTaskExecutor<int>
                    (
                        loopCondition: () => gameMap.Timer.IsGaming,
                        loopToDo: () =>
                        {
                            if (!newPlayer.IsResetting)
                            {
                                if (newPlayer.Place != PlaceType.Invisible)
                                    newPlayer.Place = gameMap.GetPlaceType(newPlayer.Position);

                                long nowTime = Environment.TickCount64;
                                if (nowTime - lastTime >= newPlayer.CD)
                                {
                                    _ = newPlayer.TryAddBulletNum();
                                    lastTime = nowTime;
                                }
                            }
                        },
                        timeInterval: GameData.checkInterval,
                        finallyReturn: () => 0
                    )
                    {
                        AllowTimeExceed = true
                        /*MaxTolerantTimeExceedCount = 5,
                        TimeExceedAction = exceedTooMuch =>
                        {
                            if (exceedTooMuch) Console.WriteLine("The computer runs too slow that it cannot check the color below the player in time!");
                        }*/
                    }.Start();
                }
            )
            { IsBackground = true }.Start();

            return newPlayer.ID;
        }
        public bool StartGame(int milliSeconds)
        {
            if (gameMap.Timer.IsGaming)
                return false;
            gameMap.PlayerListLock.EnterReadLock();
            try
            {
                foreach (Character player in gameMap.PlayerList)
                {
                    player.CanMove = true;
                    //这里bug了，不信可以取消注释试试看0.0
                    //player.AddShield(GameData.shieldTimeAtBirth);
                }
            }
            finally { gameMap.PlayerListLock.ExitReadLock(); }

            propManager.StartProducing();


            //开始游戏
            if (!gameMap.Timer.StartGame(milliSeconds))
                return false;

            //清除所有对象
            gameMap.PlayerListLock.EnterWriteLock();
            try
            {
                foreach (Character player in gameMap.PlayerList)
                {
                    player.CanMove = false;
                }
                gameMap.PlayerList.Clear();
            }
            finally { gameMap.PlayerListLock.ExitWriteLock(); }
            gameMap.BulletListLock.EnterWriteLock();
            try
            {
                gameMap.BulletList.Clear();
            }
            finally { gameMap.BulletListLock.ExitWriteLock(); }
            gameMap.PropListLock.EnterWriteLock();
            try
            {
                gameMap.PropList.Clear();
            }
            finally { gameMap.PropListLock.ExitWriteLock(); }
            return true;
        }
        public void MovePlayer(long playerID, int moveTimeInMilliseconds, double angle)
        {
            if (!gameMap.Timer.IsGaming)
                return;
            Character? player = gameMap.FindPlayer(playerID);
            if (player != null)
            {
                moveManager.MovePlayer(player, moveTimeInMilliseconds, angle);
#if DEBUG
                Console.WriteLine($"PlayerID:{playerID} move to ({player.Position.x},{player.Position.y})!");
#endif
            }
            else
            {
#if DEBUG
                Console.WriteLine($"PlayerID:{playerID} player does not exists!");
#endif
            }
        }
        public void Attack(long playerID, double angle)
        {
            if (!gameMap.Timer.IsGaming)
                return;
            Character? player = gameMap.FindPlayer(playerID);
            if (player != null)
            {
                _ = attackManager.Attack(player, angle);
            }
        }

        public bool UseCommonSkill(long playerID)
        {
            if (!gameMap.Timer.IsGaming)
                return false;
            Character? player = gameMap.FindPlayer(playerID);
            if (player != null)
            {
                return skillManager.UseCommonSkill(player);
            }
            else return false;
        }
        public int GetTeamScore(long teamID)
        {
            return teamList[(int)teamID].Score;
        }
        public List<IGameObj> GetGameObj()
        {
            var gameObjList = new List<IGameObj>();
            gameMap.PlayerListLock.EnterReadLock();
            try
            {
                gameObjList.AddRange(gameMap.PlayerList);
            }
            finally { gameMap.PlayerListLock.ExitReadLock(); }

            gameMap.BulletListLock.EnterReadLock();
            try
            {
                gameObjList.AddRange(gameMap.BulletList);
            }
            finally { gameMap.BulletListLock.ExitReadLock(); }

            gameMap.PropListLock.EnterReadLock();
            try
            {
                gameObjList.AddRange(gameMap.PropList);
            }
            finally { gameMap.PropListLock.ExitReadLock(); }

            return gameObjList;
        }
        public Game(uint[,] mapResource, int numOfTeam)
        {
            //if (numOfTeam > maxTeamNum) throw new TeamNumOverFlowException();

            gameMap = new Map(mapResource);

            //加入队伍
            this.numOfTeam = numOfTeam;
            teamList = new List<Team>();
            for (int i = 0; i < numOfTeam; ++i)
            {
                teamList.Add(new Team());
            }

            skillManager = new SkillManager();
            attackManager = new AttackManager(gameMap);
            moveManager = new MoveManager(gameMap);
            propManager = new PropManager(gameMap);
            gemManager = new GemManager(gameMap);
        }
    }
}
