using System;

namespace Preparation.Utility
{
    public struct XYPosition
    {
        public int x;
        public int y;
        public XYPosition(int x, int y)
        {
            this.x = x;
            this.y = y;
        }
        public override string ToString()
        {
            return "(" + x.ToString() + "," + y.ToString() + ")";
        }
        public static XYPosition operator +(XYPosition p1, XYPosition p2)
        {
            return new XYPosition(p1.x + p2.x, p1.y + p2.y);
        }
        public static XYPosition operator -(XYPosition p1, XYPosition p2)
        {
            return new XYPosition(p1.x - p2.x, p1.y - p2.y);
        }

        public static double Distance(XYPosition p1,XYPosition p2)
        {
            return Math.Sqrt((p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y));
        }
    }
}
