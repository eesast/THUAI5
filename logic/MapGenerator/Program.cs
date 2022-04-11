using System;
using System.IO;

namespace Function
{
   internal class Program
    {
        static int[,] map;
        static int size;
        static readonly double P1 = 0.7;//不建议小于0.2或大于0.9，否则会爆栈。建议加入错误处理
        static readonly double Pturn = 0.33;
        static int count = 0;
        static int grasscount = 0;
        static bool[] connect = new bool[4];
        static bool flag = false;
        static void CreateMap(int size)
        {
            for(int i = 0; i < 4;i++)
            {
                connect[i] = false;
            }
            count = 0;
            flag = false;
            map = new int[size, size];
            Grow(size/5, size/2, size, size*size*P1*Pturn,3);
            Grow(size * 13 / 20, size / 2 - size * 13 / 100, size, 2 * size * size * P1 * Pturn, 1);
            Grow(size * 13 / 20, size / 2+ size * 13 / 100, size, size * size * P1,2);
            for (int i = 0; i < size; i++)
            {
                int j;
                for (j = 0; j < size; j++)
                {
                    if (map[i, j] !=0) break;
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
                        if (map[j, i] !=0) break;
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
            Random r=new();
            return r.Next(0, n);
        }
        static void Grow(int x,int y,int size,double lim,int sign)
        {
            if (count <= lim)
            {
                if (x >= size)
                {
                    Grow(x - 1, y, size,lim,sign);
                }
                else if (y >= size)
                {
                    Grow(x, y - 1, size,lim, sign);
                }
                else if (x < 0)
                {
                    Grow(x + 1, y, size,lim, sign);
                }
                else if (y < 0)
                {
                    Grow(x, y + 1, size,lim, sign);
                }
                else
                {
                    if (map[x, y] !=sign)
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
                            Grow(x - 1, y, size,lim, sign);
                            break;
                        case 1://左
                            Grow(x, y - 1, size,lim, sign);
                            break;
                        case 2://下
                            Grow(x + 1, y, size,lim, sign);
                            break;
                        case 3://右
                            Grow(x, y + 1, size,lim, sign);
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
            for(int i=0; i < n;i++)
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
        static int ThingsAround(int i, int j,int type)
        {
            bool up = (i == 0) ? true : (map[i - 1, j] == type);
            bool down = (i == size - 1) ? true : (map[i + 1, j] == type);
            bool left = (j == 0) ? true : (map[i, j - 1] == type);
            bool right = (j == size - 1) ? true : (map[i, j + 1] == type);
            return ((up?1:0) + (down?1:0) + (left?1:0) + (right?1:0));
        }
        static void Process(int times)//处理
        {
            do
            {
                for (int i = 0; i < size; i++)
                {
                    for (int j = 0; j < size; j++)
                    {
                        if (map[i, j] != 0 && ThingsAround(i, j, 0) > 2)
                        {
                            map[i, j] = 0;
                        }
                        if (ThingsAround(i, j, 0) < 1 || ThingsAround(i, j, 0) > 2)//墙削减
                        {
                            map[i, j] = 1;
                        }
                        if (ThingsAround(i, j, 4) >= 2)//草丛方形化
                        {
                            map[i, j] = 4;
                        }
                        if (i == 0 || i == size - 1 || j == 0 || j == size - 1)//边界墙
                        {
                            map[i, j] = 0;
                        }
                    }
                }
            }
            while (times-->0);
        }
        static void OutPut()
        {
            StreamWriter sw = new StreamWriter("map_array.txt");
            StreamWriter sw1 = new StreamWriter("space_array.txt");
            for (int i = 0; i < size; i++)
            {
                sw.Write("{");
                for (int j = 0; j < size; j++)
                {
                    if (map[i, j] == 4)//草丛
                    {
                        Console.Write("::");
                        sw.Write(j == size - 1 ? "2" : "2,");
                        sw1.Write(j == size - 1 ? "2" : "2 ");
                    }
                    else if(map[i,j]==0)//墙
                    {
                        Console.Write("0 ");
                        sw.Write(j == size - 1 ? "1" : "1,");
                        sw1.Write(j == size - 1 ? "1" : "1 ");
                    }
                    else//空地
                    {
                        Console.Write(". ");
                        sw.Write(j == size - 1 ? "0" : "0,");
                        sw1.Write(j == size - 1 ? "0" : "0 ");
                    }
                    //Console.Write(Convert.ToString(map[i, j]));
                    //Console.Write(' ');//数字显示
                }
                Console.WriteLine();
                sw.Write(i == size - 1 ? "}" : "},\n");
                sw1.Write(i == size - 1 ? "" : "\n");
            }
            Console.WriteLine();
            sw.Close();
            sw1.Close();
        }
        static void Main()
        {
            size = 50;
            while (Console.ReadLine()!="q")//input not q to generate a map
            {
                Console.WriteLine("Begin:"+DateTime.Now.ToString("yyyy-MM-dd HH:mm:ss:fff:ffffff"));
                CreateMap(size);
                CreateGrass(3);
                Console.WriteLine("Done:"+DateTime.Now.ToString("yyyy-MM-dd HH:mm:ss:fff:ffffff"));
                Process(5);
                OutPut();
            }
        }
    }
}
