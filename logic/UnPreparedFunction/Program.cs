using System;

namespace Function
{
    internal class Program
    {
        static int[,] map;
        static readonly double P1 = 0.7;//不建议小于0.2，建议加入错误处理
        static readonly double Pturn = 0.33;
        static int count = 0;
        static int grasscount = 0;
        static bool[] connect = new bool[4];
        static bool flag = false;
        static void CreateMap(int size)
        {
            for (int i = 0; i < 4; i++)
            {
                connect[i] = false;
            }
            count = 0;
            flag = false;
            map = new int[size, size];
            Grow(size / 5, size / 2, size, size * size * P1 * Pturn, 3);
            Grow(size * 13 / 20, size / 2 - size * 13 / 100, size, 2 * size * size * P1 * Pturn, 1);
            Grow(size * 13 / 20, size / 2 + size * 13 / 100, size, size * size * P1, 2);
            for (int i = 0; i < size; i++)
            {
                int j;
                for (j = 0; j < size; j++)
                {
                    if (map[i, j] != 0) break;
                }
                if (j == size) flag = true;
            }
            if (!flag)
            {
                for (int i = 0; i < size; i++)
                {
                    int j;
                    for (j = 0; j < size; j++)
                    {
                        if (map[j, i] != 0) break;
                    }
                    if (j == size) flag = true;
                }
            }
            if (!flag)
            {
                int count = 0;
                for (int i = 1; i < 4; i++)
                {
                    if (connect[i]) count++;
                }
                if (count < 2) flag = true;
            }
            if (flag)
                CreateMap(size);
        }
        static int Rand(int n)
        {
            Random r = new();
            return r.Next(0, n);
        }
        static void Grow(int x, int y, int size, double lim, int sign)
        {
            if (count <= lim)
            {
                if (x >= size)
                {
                    Grow(x - 1, y, size, lim, sign);
                }
                else if (y >= size)
                {
                    Grow(x, y - 1, size, lim, sign);
                }
                else if (x < 0)
                {
                    Grow(x + 1, y, size, lim, sign);
                }
                else if (y < 0)
                {
                    Grow(x, y + 1, size, lim, sign);
                }
                else
                {
                    if (map[x, y] != sign)
                    {
                        if (map[x, y] == 0)
                        {
                            count++;
                            map[x, y] = sign;
                        }
                        else
                        {
                            connect[sign] = true;
                        }
                    }
                    int r = Rand(4);
                    switch (r)
                    {
                        case 0://上
                            Grow(x - 1, y, size, lim, sign);
                            break;
                        case 1://左
                            Grow(x, y - 1, size, lim, sign);
                            break;
                        case 2://下
                            Grow(x + 1, y, size, lim, sign);
                            break;
                        case 3://右
                            Grow(x, y + 1, size, lim, sign);
                            break;
                        default:
                            return;

                    }
                }
            }
        }
        static void Grass(int x, int y, int size, double lim)
        {
            if (grasscount <= lim)
            {
                if (x >= size)
                {
                    Grass(x - 1, y, size, lim);
                }
                else if (y >= size)
                {
                    Grass(x, y - 1, size, lim);
                }
                else if (x < 0)
                {
                    Grass(x + 1, y, size, lim);
                }
                else if (y < 0)
                {
                    Grass(x, y + 1, size, lim);
                }
                else
                {
                    if (map[x, y] != 4)
                    {
                        grasscount++;
                        map[x, y] = 4;
                    }
                    int r = Rand(4);
                    switch (r)
                    {
                        case 0://上
                            Grass(x - 1, y, size, lim);
                            break;
                        case 1://左
                            Grass(x, y - 1, size, lim);
                            break;
                        case 2://下
                            Grass(x + 1, y, size, lim);
                            break;
                        case 3://右
                            Grass(x, y + 1, size, lim);
                            break;
                        default:
                            return;

                    }
                }
            }
        }
        static void CreateGrass(int n)
        {
            for (int i = 0; i < n; i++)
            {
                int x, y;
                do
                {
                    x = Rand(50);
                    y = Rand(50);
                }
                while (map[x, y] == 0);
                Grass(x, y, 50, 30);
                grasscount = 0;
            }
        }
        static void Main()
        {
            int size = 50;
            while (Console.Read() != -1)
            {
                CreateMap(size);
                CreateGrass(3);
                for (int i = 0; i < size; i++)
                {
                    for (int j = 0; j < size; j++)
                    {
                        if (map[i, j] != 0 && map[i, j] != 4)
                        {
                            Console.Write(". ");
                        }
                        else if (map[i, j] == 4)
                            Console.Write("::");
                        else Console.Write("0 ");
                        //Console.Write(Convert.ToString(map[i, j]));
                        //Console.Write(' ');
                    }
                    Console.WriteLine();
                }
                Console.WriteLine();
            }
        }
    }
}