using System;
using System.Collections.Generic;
using System.Threading;
using Preparation.Interface;
using Preparation.Utility;
using Preparation.GameData;

namespace GameEngine
{
	internal class CollisionChecker
	{
		/// <summary>
		/// 碰撞检测，如果这样行走是否会与之碰撞，返回与之碰撞的物体
		/// </summary>
		/// <param name="obj">移动的物体</param>
		/// <param name="moveVec">移动的位移向量</param>
		/// <returns>和它碰撞的物体</returns>
		public IGameObj? CheckCollision(IMoveable obj, Vector moveVec)
		{
			XYPosition nextPos = obj.Position + Vector.Vector2XY(moveVec);
			if (!obj.IsRigid)
			{
				if (gameMap.IsOutOfBound(obj)) 
					return gameMap.GetOutOfBound(nextPos);
				return null;
			}
			//在某列表中检查碰撞，列表内的物体不应包含墙体
			Func<IEnumerable<IGameObj>, ReaderWriterLockSlim, IGameObj?> CheckCollisionInList =
				(IEnumerable<IGameObj> lst, ReaderWriterLockSlim listLock) =>
				{
					IGameObj? collisionObj = null;
					listLock.EnterReadLock();
					try
					{
						foreach (var listObj in lst)
						{
							if (obj.WillCollideWith(listObj, nextPos))
							{
								collisionObj = listObj;
								break;
							}
						}
					}
					finally { listLock.ExitReadLock(); }
					return collisionObj;
				};

			IGameObj? collisionObj = null;
			foreach (var list in lists)
			{
				if ((collisionObj = CheckCollisionInList(list.Item1, list.Item2)) != null)
				{
					return collisionObj;
				}
			}

			//对墙体检查
			if (gameMap.IsWall(nextPos))
				return gameMap.GetCell(nextPos);
			return null;
		}
		/// <summary>
		/// /// 可移动物体（圆）向矩形物体移动时，可移动且不会碰撞的最大距离。直接用double计算，防止误差
		/// </summary>
		/// <param name="obj"></param>
		/// <param name="square">矩形的中心坐标</param>
		/// <returns></returns>
		private double MaxMoveToSquare(IMoveable obj,IGameObj square)
        {
			double tmpMax;
			double angle = Math.Atan2(square.Position.y - obj.Position.y, square.Position.x - obj.Position.x);
			if (obj.WillCollideWith(square, obj.Position))
				tmpMax = 0;
			else tmpMax = 
				XYPosition.Distance(obj.Position, square.Position) - obj.Radius -
				square.Radius / Math.Min(Math.Abs(Math.Cos(angle)), Math.Abs(Math.Sin(angle)));
			return tmpMax;
		}

		/// <summary>
		/// 寻找最大可能移动距离
		/// </summary>
		/// <param name="obj">移动物体，默认obj.Rigid为true</param>
		/// <param name="nextPos">下一步要到达的位置</param>
		/// <param name="moveVec">移动的位移向量，默认与nextPos协调</param>
		/// <returns>最大可能的移动距离</returns>
		public double FindMax(IMoveable obj, XYPosition nextPos, Vector moveVec)
		{
			double maxLen = (double)uint.MaxValue;
			double tmpMax = maxLen; //暂存最大值

			// 先找只考虑墙的最大距离。需要明确的是，objlist中不应当添加墙
			var desination = moveVec;
			double maxOnlyConsiderWall = maxLen;
			while(desination.length>0)
            {
				if (gameMap.IsWall(Vector.Vector2XY(desination) + obj.Position))
				{
					maxOnlyConsiderWall = MaxMoveToSquare(obj, gameMap.GetCell(Vector.Vector2XY(desination) + obj.Position));
				}
				desination.length -= GameData.numOfPosGridPerCell / (Math.Abs(Math.Cos(desination.angle)));
            }

			tmpMax = maxLen;
			double maxIgnoringWall = maxLen;
			foreach (var listWithLock in lists)
			{
				var lst = listWithLock.Item1;
				var listLock = listWithLock.Item2;
				listLock.EnterReadLock();
				try
				{
					foreach (IGameObj listObj in lst)
					{
						//如果再走一步发生碰撞
						if (obj.WillCollideWith(listObj, nextPos))
						{
							{
								switch (listObj.Shape)  //默认obj为圆形
								{
									case ShapeType.Circle:
										{
											//计算两者之间的距离
											double mod = XYPosition.Distance(listObj.Position, obj.Position);
											int orgDeltaX = listObj.Position.x - obj.Position.x;
											int orgDeltaY = listObj.Position.y - obj.Position.y;

											if (mod < listObj.Radius + obj.Radius)    //如果两者已经重叠
											{
												tmpMax = 0;
											}
											else if ((listObj.Position - obj.Position).ToVector2() * moveVec.ToVector2() <= 0)
												continue;      //如果相对位置和运动方向反向，那么不会发生碰撞
											else if (mod / (Math.Cos(Math.Atan2(orgDeltaY, orgDeltaX) - moveVec.angle))>obj.Radius+listObj.Radius) // 如果沿着moveVec方向移动不会撞，就真的不会撞
												continue;
											else
											{
												double tmp = mod - obj.Radius - listObj.Radius;
												//计算能走的最长距离，好像这么算有一点误差？
												tmp = tmp / Math.Cos(Math.Atan2(orgDeltaY, orgDeltaX) - moveVec.angle);
												if (tmp < 0 || tmp > uint.MaxValue || tmp == double.NaN)
												{
													tmpMax = uint.MaxValue;
												}
												else tmpMax = tmp;
											}
											break;
										}
									case ShapeType.Square:
										{
											if (obj.WillCollideWith(listObj, obj.Position)) 
												tmpMax = 0;
											else tmpMax = MaxMoveToSquare(obj, listObj);
											break;
										}
									default:
										tmpMax = uint.MaxValue;
										break;
								}
								if (tmpMax < maxIgnoringWall)
									maxIgnoringWall = tmpMax;
							}
						}
					}
				}
				finally 
				{
					maxLen = Math.Max(maxOnlyConsiderWall, maxIgnoringWall);
					listLock.ExitReadLock(); 
				}
			}
			return maxLen;
		}

		IMap gameMap;
		private Tuple<IEnumerable<IGameObj>, ReaderWriterLockSlim>[] lists;

		public CollisionChecker(IMap gameMap)
		{
			this.gameMap = gameMap;
			lists = new Tuple<IEnumerable<IGameObj>, ReaderWriterLockSlim>[]
			{
				new Tuple<IEnumerable<IGameObj>, ReaderWriterLockSlim>(gameMap.ObjList, gameMap.ObjListLock),
				new Tuple<IEnumerable<IGameObj>, ReaderWriterLockSlim>(gameMap.PlayerList, gameMap.PlayerListLock)
			};
		}
	}
}
