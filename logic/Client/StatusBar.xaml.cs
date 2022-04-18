using System;
using System.Windows.Controls;
using System.Windows.Media;
using Communication.Proto;


namespace Client
{
    /// <summary>
    /// status.xaml 的交互逻辑
    /// </summary>
    public partial class StatusBar : UserControl
    {
        public StatusBar(Grid parent, int Row,int Column)
        {
            InitializeComponent();
            parent.Children.Add(this);
            Grid.SetColumn(this, Column);
            Grid.SetRow(this, Row);
            initialized = false;
        }
        public void SetFontSize(double fontsize)
        {
            serial.FontSize = scores.FontSize = star.FontSize = status.FontSize = prop.FontSize = fontsize;
        }
        private void SetStaticValue(MessageOfCharacter obj)
        {
            switch (obj.ActiveSkillType)
            {
                case ActiveSkillType.BecomeVampire:
                    coolTime = 30000;
                    serial.Text = "👥" + Convert.ToString(obj.TeamID) + "🧓"
                        + Convert.ToString(obj.PlayerID) + "\n软件：Emission";
                    break;
                case ActiveSkillType.SuperFast:
                    coolTime = 30000;
                    serial.Text = "👥" + Convert.ToString(obj.TeamID) + "🧓"
                        + Convert.ToString(obj.PlayerID) + "\n软件：Booster";
                    break;
                case ActiveSkillType.NuclearWeapon:
                    coolTime = 30000;
                    serial.Text = "👥" + Convert.ToString(obj.TeamID) + "🧓"
                        + Convert.ToString(obj.PlayerID) + "\n软件：Amplifier";
                    break;
                case ActiveSkillType.BecomeAssassin:
                    coolTime = 30000;
                    serial.Text = "👥" + Convert.ToString(obj.TeamID) + "🧓"
                        + Convert.ToString(obj.PlayerID) + "\n软件：Invisible";
                    break;
                case ActiveSkillType.NullActiveSkillType:
                    coolTime = 30000;
                    serial.Text = "👥" + Convert.ToString(obj.TeamID) + "🧓"
                        + Convert.ToString(obj.PlayerID) + "\n软件：Null";
                    break;
            }
            initialized = true;
        }
        private void SetDynamicValue(MessageOfCharacter obj)
        {
            skillprogress.Value = obj.TimeUntilCommonSkillAvailable / coolTime * 100;
            if (obj.IsResetting) skillprogress.Background = Brushes.Gray;
            else skillprogress.Background = Brushes.White;
            Func<MessageOfCharacter,int> life=
            (obj) =>
            {
                if (obj.IsResetting)
                    return 0;
                else return obj.Life;
            };
            star.Text = "⭐：" + Convert.ToString(obj.GemNum);
            status.Text = "🗡："
                + Convert.ToString(obj.AttackRange)
                + "\n🏹："
                + Convert.ToString(obj.BulletNum)
                + "\n🏃：" + Convert.ToString(obj.Speed)
                + "\n♥：" + Convert.ToString(life(obj));
            scores.Text = "Scores:" + Convert.ToString(obj.Score);
            switch (obj.Prop)
            {
                case PropType.Gem:
                    prop.Text = "📱";
                    break;
                case PropType.Shield:
                    prop.Text = "🛡";
                    break;
                case PropType.Spear:
                    prop.Text = "🗡";
                    break;
                case PropType.AddSpeed:
                    prop.Text = "⛸";
                    break;
                case PropType.AddLife:
                    prop.Text = "♥";
                    break;
                default:
                    prop.Text = "  ";
                    break;
            }
        }
        public void SetValue(MessageOfCharacter obj)
        {
            if (!initialized) SetStaticValue(obj);
            SetDynamicValue(obj);
        }
        private int coolTime;
        private bool initialized;
    }
}
