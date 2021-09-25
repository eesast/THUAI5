using Preparation.GameObj;
using Preparation.Utility;

namespace Preparation
{
    public static class Constant
    {
        /// <summary>
        /// 基础常数与常方法
        /// </summary>
        public const int numOfPosGridPerCell = 1000;            // 每格的【坐标单位】数
        public const int numOfStepPerSecond = 20;               // 每秒行走的步数

        public static XYPosition GetCellCenterPos(int x, int y)   // 求格子的中心坐标
        {
            XYPosition ret = new XYPosition(x * numOfPosGridPerCell + numOfPosGridPerCell / 2,
                y * numOfPosGridPerCell + numOfPosGridPerCell / 2);
            return ret;
        }
        public static int PosGridToCellX(XYPosition pos)       // 求坐标所在的格子的x坐标
        {
            return pos.x / numOfPosGridPerCell;
        }
        public static int PosGridToCellY(XYPosition pos)      // 求坐标所在的格子的y坐标
        {
            return pos.y / numOfPosGridPerCell;
        }
        /// <summary>
        /// 玩家相关
        /// </summary>
        public const int basicAp = 1000;	// 初始攻击力
        public const int basicHp = 6000;	// 初始血量
        public const int basicCD = 1000;    // 初始冷却
        public const int basicBulletNum = 12;   // 初始子弹量（如果是射手）
        public const int MinAP = 0; // 最小攻击力
        public const int MaxAP = int.MaxValue;  //最大攻击力
        /// <summary>
        /// 道具相关
        /// </summary>
        public const int MinPropTypeNum = 1;
        public const int MaxPropTypeNum = 10;
    }
}