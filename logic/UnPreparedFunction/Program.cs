using System;

namespace Function
{
     
    internal class Program
    {
        static int[,] map;
        static readonly double P1 = 0.9;
        static readonly double Pturn = 0.35;
        static readonly bool[] rec = new bool[4];
        static int count = 0;
        static bool flag = false;
        static void CreateMap(int size)
        {
            for(int i=0;i<4;i++)
            {
                rec[i] = false;
            }
            count = 0;
            flag = false;
            map = new int[size, size];
            Grow(3, size / 2, size / 2, size, size*size*P1*Pturn);
            for (int i = 0; i < 4; i++)
            {
                if (!rec[i])
                {
                    rec[i] = true;
                    Grow(i, size / 2, size / 2, size, 2 * size * size * P1 * Pturn);
                }
            }
            for (int i = 0; i < 4; i++)
            {
                if (!rec[i])
                {
                    rec[i] = true;
                    Grow(i, size / 2, size / 2, size, size * size * P1);
                }
            }
            for (int i = 0; i < size; i++)
            {
                int j;
                for (j = 0; j < size; j++)
                {
                    if (map[i, j] == 1) break;
                }
                if (j == size) flag = true;
            }
            if(!flag)
            {
                for (int i = 0; i < size; i++)
                {
                    int j;
                    for (j = 0; j < size; j++)
                    {
                        if (map[j, i] == 1) break;
                    }
                    if (j == size) flag = true;
                }
            }
            if (flag)
                CreateMap(size);
        }
        static int Rand(int n)
        {
            Random r=new();
            return r.Next(0, n);
        }
        static void Grow(int dir,int x,int y,int size,double lim)
        {
            if (count <= lim)
            {
                if (count < 2)
                {
                    rec[dir] = true;
                }
                if (x >= size)
                {
                    Grow(3, x - 1, y, size,lim);
                }
                else if (y >= size)
                {
                    Grow(0, x, y - 1, size,lim);
                }
                else if (x < 0)
                {
                    Grow(1, x + 1, y, size,lim);
                }
                else if (y < 0)
                {
                    Grow(2, x, y + 1, size,lim);
                }
                else
                {
                    if (map[x, y] == 0)
                    {
                        count++;
                        map[x, y] = 1;
                    }
                    int r = Rand(4);
                    switch (r)
                    {
                        case 0://上
                            Grow(r, x - 1, y, size,lim);
                            break;
                        case 1://左
                            Grow(r, x, y - 1, size,lim);
                            break;
                        case 2://下
                            Grow(r, x + 1, y, size,lim);
                            break;
                        case 3://右
                            Grow(r, x, y + 1, size,lim);
                            break;
                        default:
                            return;

                    }
                }
            }
        }
    static void Main()
        {
            int size = 50;
            while (Console.ReadLine() =="a")
            {
                CreateMap(size);
                for (int i = 0; i < size; i++)
                {
                    for (int j = 0; j < size; j++)
                    {
                        Console.Write(map[i, j]);
                        Console.Write(" ");
                    }
                    Console.WriteLine();
                }
            }
        }
    }
}
