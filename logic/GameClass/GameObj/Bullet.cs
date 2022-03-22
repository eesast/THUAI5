using Preparation.GameData;
using Preparation.Interface;
using Preparation.Utility;
using System;

namespace GameClass.GameObj
{
    public abstract class Bullet : ObjOfCharacter  
    {
        /// <summary>
        /// //攻击力
        /// </summary>
        public abstract int AP { get; }
        public abstract int Speed { get; }

        private readonly bool hasSpear;
        /// <summary>
        /// 是否有矛
        /// </summary>
        public bool HasSpear => hasSpear;

        public abstract double BulletBombRange { get; }
        /// <summary>
        /// 与THUAI4不同的一个攻击判定方案，通过这个函数判断爆炸时能否伤害到target
        /// </summary>
        /// <param name="target">被尝试攻击者</param>
        /// <returns>是否可以攻击到</returns>
        public abstract bool CanAttack(GameObj target);

        protected override bool IgnoreCollideExecutor(IGameObj targetObj)
        {
            if (targetObj is BirthPoint || targetObj.Type == GameObjType.Prop) return true;	// 子弹不会与出生点和道具碰撞
            return false;
        }
        public Bullet(Character player, int radius) : base(player.Position, radius, PlaceType.Null)
        {
            this.CanMove = true;
            this.Type = GameObjType.Bullet;
            this.moveSpeed = this.Speed;
            this.hasSpear = player.HasSpear;
            this.Parent = player;
        }
        public override bool IsRigid => true;	// 默认为true
        public override ShapeType Shape => ShapeType.Circle;	// 默认为圆形
        public abstract BulletType TypeOfBullet { get;}
        
    }

    internal sealed class AtomBomb : Bullet  //3倍爆炸范围，3倍攻击力，1倍速
    {
        public AtomBomb(Character player, int radius = GameData.bulletRadius) : base(player, radius) { }
        public override double BulletBombRange => 3 * GameData.basicBulletBombRange;
        public override int AP => 3 * GameData.basicAp;
        public override int Speed => GameData.basicBulletMoveSpeed;
        public override bool CanAttack(GameObj target)
        {
            // 圆形攻击范围
            return XYPosition.Distance(this.Position, target.Position) <= this.BulletBombRange;
        }

        public override BulletType TypeOfBullet => BulletType.AtomBomb;
    }

    internal sealed class OrdinaryBullet : Bullet //1倍攻击范围，1倍攻击力，一倍速
    {
        public OrdinaryBullet(Character player, int radius = GameData.bulletRadius) : base(player, radius) { }
        public override double BulletBombRange => GameData.basicBulletBombRange;
        public override int AP => GameData.basicAp;
        public override int Speed => GameData.basicBulletMoveSpeed;
        public override bool CanAttack(GameObj target)
        {
            // 圆形攻击范围
            return XYPosition.Distance(this.Position, target.Position) <= this.BulletBombRange;
        }

        public override BulletType TypeOfBullet => BulletType.OrdinaryBullet;
    }

    internal sealed class FastBullet: Bullet //1倍攻击范围，0.2倍攻击力，2倍速
    {
        public FastBullet(Character player, int radius = GameData.bulletRadius) : base(player, radius) { }
        public override double BulletBombRange => GameData.basicBulletBombRange / 4 * 2;
        public override int AP => (int)(0.2 * GameData.basicAp);
        public override int Speed => 2 * GameData.basicBulletMoveSpeed;
            
        public override bool CanAttack(GameObj target)
        {
            // 圆形攻击范围
            return XYPosition.Distance(this.Position, target.Position) <= this.BulletBombRange;
        }

        public override BulletType TypeOfBullet => BulletType.FastBullet;
    }

    internal sealed class LineBullet : Bullet //直线爆炸，宽度1格，长度为2倍攻击范围，1倍攻击力，1倍速
    {
        public LineBullet(Character player, int radius = GameData.bulletRadius) : base(player, radius) { }
        public override double BulletBombRange => 2 * GameData.basicBulletBombRange;
        public override int AP => GameData.basicAp;
        public override int Speed =>  GameData.basicBulletMoveSpeed;

        public override bool CanAttack(GameObj target)
        {
            this.FacingDirection = Preparation.Utility.Tools.CorrectAngle(this.FacingDirection);
            if (this.FacingDirection == Math.PI / 2)
            {
                if (target.Position.y - this.Position.y > BulletBombRange)
                    return false;
                if (target.Position.x < this.Position.x + GameData.numOfPosGridPerCell / 2 && target.Position.x > this.Position.x - GameData.numOfPosGridPerCell / 2)
                    return true;
                return false;
            }
            else if(this.FacingDirection == Math.PI * 3 / 2)
            {
                if (target.Position.y - this.Position.y < -BulletBombRange)
                    return false;
                if (target.Position.x < this.Position.x + GameData.numOfPosGridPerCell / 2 && target.Position.x > this.Position.x - GameData.numOfPosGridPerCell / 2)
                    return true;
                return false;
            }
            double vertical = Math.Tan(this.FacingDirection + Math.PI);
            double dist;
            dist = Math.Abs(vertical * (target.Position.x - this.Position.x) - (target.Position.y - this.Position.y)) / (Math.Sqrt(1 + vertical * vertical));
            if (dist > BulletBombRange)
                return false;
            vertical = Math.Tan(this.FacingDirection);
            dist = Math.Abs(vertical * (target.Position.x - this.Position.x) - (target.Position.y - this.Position.y)) / (Math.Sqrt(1 + vertical * vertical));
            if (dist > GameData.numOfPosGridPerCell / 2)
                return false;
            return true;
        }

        public override BulletType TypeOfBullet => BulletType.LineBullet;
    }
    public static class BulletFactory
    {
        public static Bullet? GetBullet(Character character)
        {
            Bullet? newBullet = null;
            switch (character.BulletOfPlayer)
            {
                case BulletType.AtomBomb:
                    newBullet = new AtomBomb(character);
                    break;
                case BulletType.LineBullet:
                    newBullet = new LineBullet(character);
                    break;
                case BulletType.FastBullet:
                    newBullet = new FastBullet(character);
                    break;
                case BulletType.OrdinaryBullet:
                    newBullet = new OrdinaryBullet(character);
                    break;
                default:
                    break;
            }
            return newBullet;
        }
        public static int BulletRadius(BulletType bulletType)
        {
            switch (bulletType)
            {
                case BulletType.AtomBomb:
                case BulletType.LineBullet:
                case BulletType.FastBullet:
                case BulletType.OrdinaryBullet:
                default:
                    return GameData.bulletRadius;
            }
        }
    }
}
