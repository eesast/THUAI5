using System.Windows.Controls;
using System.Windows.Media;
using Communication.Proto;
using System;
namespace Client
{
    internal class StatusBar
    {
        public StatusBar(Grid parent,int margin1,int margin2,int margin3,int margin4)
        {
            backGround = new();
            star = new();
            progressBar = new();
            status = new();
            scores = new();
            serial = new();
            parent.Children.Add(backGround);
            backGround.Background = Brushes.White;
            backGround.Margin = new(margin1, margin2, margin3, margin4);
            progressBar.Height = 15;
            progressBar.Width = 65;
            progressBar.Value = 50;
            progressBar.Background = Brushes.White;
            backGround.Children.Add(progressBar);
            Canvas.SetTop(progressBar, 150);

            star.Height = 15;
            star.Text = "⭐：";
            star.TextWrapping = System.Windows.TextWrapping.Wrap;
            star.Width = 65;
            star.FontSize = 12;
            star.BorderBrush = Brushes.White;
            star.RenderTransformOrigin = new(0.478, 0.159);
            star.IsReadOnly = true;
            backGround.Children.Add(star);
            Canvas.SetTop(star, 130);

            status.Height = 63;
            status.Text = "🗡：\n🏹：\n🏃：\n♥：";
            status.TextWrapping = System.Windows.TextWrapping.Wrap;
            status.Width = 65;
            status.FontSize = 12;
            status.BorderBrush = Brushes.White;
            status.IsReadOnly = true;
            backGround.Children.Add(status);
            Canvas.SetTop(status, 47);

            scores.Height = 15;
            scores.Text = "Scores：";
            scores.TextWrapping = System.Windows.TextWrapping.Wrap;
            scores.Width = 65;
            scores.FontSize = 12;
            scores.BorderBrush = Brushes.White;
            scores.IsReadOnly = true;
            backGround.Children.Add(scores);
            Canvas.SetTop(scores, 115);

            serial.Height = 42;
            serial.Text = "👥null🧓null\n职业：";
            serial.TextWrapping = System.Windows.TextWrapping.Wrap;
            serial.Width = 65;
            serial.FontSize = 12;
            serial.BorderBrush = Brushes.White;
            serial.RenderTransformOrigin = new(0.478, 0.159);
            serial.IsReadOnly = true;
            backGround.Children.Add(serial);
            initialized = false;
        }
        private void SetStaticValue(MessageOfCharacter obj)
        {
            switch(obj.ActiveSkillType)
            {
                case ActiveSkillType.BecomeVampire:
                    coolTime = 30000;
                    serial.Text = "👥" + Convert.ToString(obj.TeamID) + "🧓" + Convert.ToString(obj.PlayerID) + "\n职业：Vampaire";
                    break;
                case ActiveSkillType.SuperFast:
                    coolTime = 30000;
                    serial.Text = "👥" + Convert.ToString(obj.TeamID) + "🧓" + Convert.ToString(obj.PlayerID) + "\n职业：SuperFast";
                    break;
                case ActiveSkillType.NuclearWeapon:
                    coolTime = 30000;
                    serial.Text = "👥" + Convert.ToString(obj.TeamID) + "🧓" + Convert.ToString(obj.PlayerID) + "\n职业：Nuclear";
                    break;
                case ActiveSkillType.BecomeAssassin:
                    coolTime = 30000;
                    serial.Text = "👥" + Convert.ToString(obj.TeamID) + "🧓" + Convert.ToString(obj.PlayerID) + "\n职业：Assassin";
                    break;
                case ActiveSkillType.NullActiveSkillType:
                    coolTime = 30000;
                    serial.Text = "👥" + Convert.ToString(obj.TeamID) + "🧓" + Convert.ToString(obj.PlayerID) + "\n职业：Null";
                    break;
            }
            initialized = true;
        }
        private void SetDynamicValue(MessageOfCharacter obj)
        {
            progressBar.Value = obj.TimeUntilCommonSkillAvailable / coolTime * 100;
            star.Text = "⭐：" + Convert.ToString(obj.GemNum);
            status.Text = "🗡："+Convert.ToString(obj.AttackRange)+"\n🏹："+Convert.ToString(obj.BulletNum) +"\n🏃："+Convert.ToString(obj.Speed)+"\n♥："+Convert.ToString(obj.Life);
            scores.Text="Scores:"+Convert.ToString(obj.Score);
        }
        public void Test(Int64 i)
        {
            progressBar.Value = i%100;
        }
        public void SetValue(MessageOfCharacter obj)
        {
            if(!initialized)SetStaticValue(obj);
            SetDynamicValue(obj);
        }
        private Canvas backGround;
        private ProgressBar progressBar;
        private TextBox star;
        private TextBox status;
        private TextBox scores;
        private TextBox serial;
        private int coolTime;
        private bool initialized;
    }  
}
