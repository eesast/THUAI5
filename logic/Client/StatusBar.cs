using System.Windows.Controls;
using System.Windows.Media;
using Communication.Proto;
using System;

namespace Client
{
    internal class StatusBar
    {
        const int Width = 65;
        const int Height = 15;
        const int FontSize = 12;
        public StatusBar(Grid parent,int margin1,int margin2,int margin3,int margin4)
        {
            backGround = new();
            star = new();
            progressBar = new();
            status = new();
            scores = new();
            serial = new();
            icon = new();
            parent.Children.Add(backGround);
            backGround.Background = Brushes.White;
            backGround.Margin = new(margin1, margin2, margin3, margin4);
            progressBar.Height = Height;
            progressBar.Width = Width-20;
            progressBar.Value = 50;
            progressBar.Background = Brushes.White;
            backGround.Children.Add(progressBar);
            Canvas.SetTop(progressBar, 150);
            Canvas.SetLeft(progressBar, Height+2);

            icon.Height = Height;
            icon.Width = Height;
            icon.IsReadOnly = true;
            icon.FontSize = 12;
            icon.TextWrapping = System.Windows.TextWrapping.Wrap;
            icon.BorderBrush = Brushes.White;
            icon.Background = Brushes.White;
            backGround.Children.Add(icon);
            Canvas.SetTop(icon, 150);
            
            star.Height = Height;
            star.Text = "⭐：";
            star.TextWrapping = System.Windows.TextWrapping.Wrap;
            star.Width = Width;
            star.FontSize = 12;
            star.BorderBrush = Brushes.White;
            star.RenderTransformOrigin = new(0.478, 0.159);
            star.IsReadOnly = true;
            backGround.Children.Add(star);
            Canvas.SetTop(star, 130);

            status.Height = 63;
            status.Text = "🗡：\n🏹：\n🏃：\n♥：";
            status.TextWrapping = System.Windows.TextWrapping.Wrap;
            status.Width = Width;
            status.FontSize = 12;
            status.BorderBrush = Brushes.White;
            status.IsReadOnly = true;
            backGround.Children.Add(status);
            Canvas.SetTop(status, 51);

            scores.Height = Height;
            scores.Text = "Scores：";
            scores.TextWrapping = System.Windows.TextWrapping.Wrap;
            scores.Width = Width;
            scores.FontSize = 12;
            scores.BorderBrush = Brushes.White;
            scores.IsReadOnly = true;
            backGround.Children.Add(scores);
            Canvas.SetTop(scores, 115);

            serial.Height = 46;
            serial.Text = "👥null🧓null\n职业：";
            serial.TextWrapping = System.Windows.TextWrapping.Wrap;
            serial.Width = Width;
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
                    serial.Text = "👥" + Convert.ToString(obj.TeamID) + "🧓"
                        + Convert.ToString(obj.PlayerID) + "\n职业：Vampaire";
                    break;
                case ActiveSkillType.SuperFast:
                    coolTime = 30000;
                    serial.Text = "👥" + Convert.ToString(obj.TeamID) + "🧓" 
                        + Convert.ToString(obj.PlayerID) + "\n职业：SuperFast";
                    break;
                case ActiveSkillType.NuclearWeapon:
                    coolTime = 30000;
                    serial.Text = "👥" + Convert.ToString(obj.TeamID) + "🧓" 
                        + Convert.ToString(obj.PlayerID) + "\n职业：Nuclear";
                    break;
                case ActiveSkillType.BecomeAssassin:
                    coolTime = 30000;
                    serial.Text = "👥" + Convert.ToString(obj.TeamID) + "🧓" 
                        + Convert.ToString(obj.PlayerID) + "\n职业：Assassin";
                    break;
                case ActiveSkillType.NullActiveSkillType:
                    coolTime = 30000;
                    serial.Text = "👥" + Convert.ToString(obj.TeamID) + "🧓" 
                        + Convert.ToString(obj.PlayerID) + "\n职业：Null";
                    break;
            }
            initialized = true;
        }
        private void SetDynamicValue(MessageOfCharacter obj)
        {
            progressBar.Value = obj.TimeUntilCommonSkillAvailable / coolTime * 100;
            star.Text = "⭐：" + Convert.ToString(obj.GemNum);
            status.Text = "🗡："
                + Convert.ToString(obj.AttackRange)
                + "\n🏹："
                + Convert.ToString(obj.BulletNum)
                + "\n🏃：" + Convert.ToString(obj.Speed)
                + "\n♥：" + Convert.ToString(obj.Life);
            scores.Text="Scores:"+Convert.ToString(obj.Score);
            switch(obj.Prop)
            {
                case PropType.Gem:
                    icon.Text= "💎";
                    break;
                case PropType.Shield:
                    icon.Text = "🛡";
                    break;
                case PropType.Spear:
                    icon.Text = "🗡";
                    break;
                case PropType.AddSpeed:
                    icon.Text = "⛸";
                    break;
                default:
                    icon.Text = "♨"; 
                    break;
            }
        }
        public void SetValue(MessageOfCharacter obj)
        {
            if(!initialized)SetStaticValue(obj);
            SetDynamicValue(obj);
        }
        private readonly Canvas backGround;
        private readonly TextBox icon;
        private readonly ProgressBar progressBar;
        private readonly TextBox star;
        private readonly TextBox status;
        private readonly TextBox scores;
        private readonly TextBox serial;
        private int coolTime;
        private bool initialized;
    }  
}