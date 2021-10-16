using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Threading;
using System.Windows.Controls;
using System.Windows.Data;
using System.Diagnostics;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.IO;

namespace Client
{
    /// <summary>
    /// MainWindow.xaml 的交互逻辑
    /// </summary>
    public partial class MainWindow : Window
    {
        public MainWindow()
        {
            textBox = new TextBox[52, 52];
            gameObject = new TextBox();
            timer = new DispatcherTimer();
            timer.Interval = new TimeSpan(1000000/60);
            timer.Tick += new EventHandler(Refresh);    //定时器初始化
            InitializeComponent();
            timer.Start();
            WindowStartupLocation = WindowStartupLocation.CenterScreen;
            //绘制地图
            try
            {
                for (int x = 0; x < 52; x++)
                {
                    for (int y = 0; y < 52; y++)
                    {
                        textBox[x, y] = new TextBox();
                        textBox[x, y].Text = "";
                        if (x==51||y==51||x==0||y==0)
                        {
                            textBox[x, y].Background = Brushes.Gray;
                            textBox[x, y].BorderBrush = Brushes.Gray;
                        }//不然就什么也不做，节省时间。如果有密集恐惧症患者，加一句边框赋值白色。
                        textBox[x, y].Height = 13;
                        textBox[x, y].Width = 13;
                        textBox[x, y].HorizontalAlignment = HorizontalAlignment.Left;
                        textBox[x, y].VerticalAlignment = VerticalAlignment.Top;
                        textBox[x, y].Margin = new Thickness(13 * x, 13 * y, 0, 0);
                        textBox[x, y].IsReadOnly = true;
                        //textBox[x, y].IsEnabled = true;
                        BottomLayerOfMap.Children.Add(textBox[x, y]);
                    }
                }           //设想用配置文件加载地图。但要做好配置文件加密。。。
                            //注：队伍用边框区分，人物编号以背景颜色区分
                            //角色死亡则对应信息框变灰
                            //被动技能和buff在人物编号后用彩色文字注明
                            //饼：允许玩家自定义人物名称
            }
            catch (Exception exc)
            {
                ErrorDisplayer error = new ErrorDisplayer("发生错误。以下是系统报告\n" + exc.Message);
                error.Show();
            }
            gameObject.Height = 13;
            gameObject.Width = 13;
            gameObject.BorderBrush = Brushes.Orange;
            gameObject.Background = Brushes.Orange;
            gameObject.HorizontalAlignment = HorizontalAlignment.Left;
            gameObject.VerticalAlignment = VerticalAlignment.Top;
            gameObject.IsReadOnly = true;
            gameObject.IsEnabled = true;
            UpperLayerOfMap.Children.Add(gameObject);
        }

        //基础窗口函数
        private void ClickToClose(object sender, RoutedEventArgs e)
        {
            Application.Current.Shutdown();
        }

        private void ClickToMinimize(object sender, RoutedEventArgs e)
        {
            WindowState = WindowState.Minimized;
        }
        private void DragWindow(object sender, RoutedEventArgs e)
        {
            DragMove();
        }

        //Client控制函数
        private void ClickToBegin(object sender, RoutedEventArgs e)
        {

        }

        private void ClickToPause(object sender, RoutedEventArgs e)
        {

        }

        private void ClickToSetMode(object sender, RoutedEventArgs e)
        {

        }

        //其他比赛信息

        //可能需要通信协助
        private void ClickToSetConnect(object sender, RoutedEventArgs e)
        {

        }

        //以下两个函数可能需要网站协助
        private void ClickToCheckLadder(object sender, RoutedEventArgs e)
        {

        }

        private void ClickForUpdate(object sender, RoutedEventArgs e)
        {

        }

        //杂项功能
        private void ClickToEnterVS(object sender, RoutedEventArgs e)
        {
            try
            {
                FileStream route = new FileStream("VSRoute.txt", FileMode.OpenOrCreate, FileAccess.ReadWrite);//创建路径文件 
                StreamReader Route = new StreamReader(route);
                string s = Route.ReadLine();
                Process.Start(s);
            }
            catch (Exception exc)
            {
                ErrorDisplayer error = new ErrorDisplayer("发生错误。以下是系统报告:\n" + exc.Message);
                error.Show();
            }
        }

        private void ClickToVisitEESAST(object sender, RoutedEventArgs e)
        {
            try
            {
                Process.Start("C:\\Program Files (x86)\\Microsoft\\Edge\\Application\\msedge.exe", "https://eesast.com");
            }
            catch (Exception exc)
            {
                ErrorDisplayer error = new ErrorDisplayer("发生错误。以下是系统报告\n" + exc.Message);
                error.Show();
            }
        }

        //定时器事件，刷新地图
        private void Refresh(object sender, EventArgs e)
        {
            //for debug
            gameObject.Margin = new Thickness(300 + 50 * Math.Cos(i * 3.14 / 180), 300 + 50 * Math.Sin(i * 3.14 / 180),0,0);
            i++;
        }
        //以下为Mainwindow自定义属性
        private TextBox[,] textBox;
        private DispatcherTimer timer;//定时器
        private static int i;//for debug
        private TextBox gameObject;
    }
}
