using Preparation.GameObj;
using Preparation.Interface;

namespace Preparation.Utility
{
    public static class Constant
    {
		public const int numOfPosGridPerCell = 1000;            //每格的【坐标单位】数
		public const int numOfStepPerSecond = 20;               //每秒行走的步数

		public static XYPosition GetCellCenterPos(int x, int y)   //求格子的中心坐标
		{
			XYPosition ret = new XYPosition(x * numOfPosGridPerCell + numOfPosGridPerCell / 2,
				y * numOfPosGridPerCell + numOfPosGridPerCell / 2);
			return ret;
		}
		public static int PosGridToCellX(XYPosition pos)       //求坐标所在的格子的x坐标
		{
			return pos.x / numOfPosGridPerCell;
		}
		public static int PosGridToCellY(XYPosition pos)      //求坐标所在的格子的y坐标
		{
			return pos.y / numOfPosGridPerCell;
		}
	}
}
