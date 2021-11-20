using Preparation.GameData;
using Preparation.Interface;
using Preparation.Utility;

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
        public Bullet(XYPosition initPos, int radius, bool hasSpear) : base(initPos, radius, PlaceType.Null)
        {
            this.CanMove = true;
            this.Type = GameObjType.Bullet;
            this.moveSpeed = this.Speed;
            this.hasSpear = hasSpear;
        }
        public Bullet(Character player, int radius) : base(player.Position, radius, PlaceType.Null)
        {
            this.CanMove = true;
            this.Type = GameObjType.Bullet;
            this.moveSpeed = this.Speed;
            this.hasSpear = player.HasSpear;
        }
        public Bullet(XYPosition initPos, Bullet bullet) : base(initPos, bullet.Radius, PlaceType.Null) { }
        public override bool IsRigid => true;	// 默认为true
        public override ShapeType Shape => ShapeType.Circle;	// 默认为圆形
        public abstract BulletType TypeOfBullet { get; }
        public abstract Bullet Clone(Character parent);  //深复制子弹
    }

    internal sealed class AtomBomb : Bullet  //3倍爆炸范围，3倍攻击力，1倍速
    {
        public AtomBomb(XYPosition initPos, int radius = GameData.bulletRadius, bool hasSpear = false) : base(initPos, radius, hasSpear) { }
        public AtomBomb(Character player, int radius = GameData.bulletRadius) : base(player, radius) { }
        public override double BulletBombRange => 3 * GameData.basicBulletBombRange;
        public override int AP => 3 * GameData.basicAp;
        public override int Speed => GameData.basicBulletMoveSpeed;
        public override bool CanAttack(GameObj target)
        {
            // 圆形攻击范围
            return XYPosition.Distance(this.Position, target.Position) <= this.BulletBombRange;
        }
        public override AtomBomb Clone(Character parent)
        {
            AtomBomb a = new AtomBomb(this.Position, this.Radius, this.HasSpear);
            a.Parent = parent;
            return a;
        }
        public override BulletType TypeOfBullet => BulletType.AtomBomb;
    }

    internal sealed class OrdinaryBullet : Bullet //1倍攻击范围，1倍攻击力，一倍速
    {
        public OrdinaryBullet(XYPosition initPos, int radius = GameData.bulletRadius, bool hasSpear = false) : base(initPos, radius, hasSpear) { }
        public OrdinaryBullet(Character player, int radius = GameData.bulletRadius) : base(player, radius) { }
        public override double BulletBombRange => GameData.basicBulletBombRange;
        public override int AP => GameData.basicAp;
        public override int Speed => GameData.basicBulletMoveSpeed;
        public override bool CanAttack(GameObj target)
        {
            // 圆形攻击范围
            return XYPosition.Distance(this.Position, target.Position) <= this.BulletBombRange;
        }
        public override OrdinaryBullet Clone(Character parent)
        {
            OrdinaryBullet a= new OrdinaryBullet(this.Position, this.Radius, this.HasSpear);
            a.Parent = parent;
            return a;
        }
        public override BulletType TypeOfBullet => BulletType.OrdinaryBullet;
    }

    internal sealed class FastBullet: Bullet //1倍攻击范围，0.5倍攻击力，2倍速
    {
        public FastBullet(XYPosition initPos, int radius = GameData.bulletRadius, bool hasSpear = false) : base(initPos, radius, hasSpear) { }
        public FastBullet(Character player, int initSpeed, int radius = GameData.bulletRadius) : base(player, radius) { }
        public override double BulletBombRange => GameData.basicBulletBombRange;
        public override int AP => (int)(0.5 * GameData.basicAp);
        public override int Speed => 2 * GameData.basicBulletMoveSpeed;
            
        public override bool CanAttack(GameObj target)
        {
            // 圆形攻击范围
            return XYPosition.Distance(this.Position, target.Position) <= this.BulletBombRange;
        }
        public override FastBullet Clone(Character parent)
        {
<<<<<<< HEAD
            Bullet0 a = new Bullet0(this.Position, this.Radius, this.MoveSpeed, this.AP, this.HasSpear);
=======
            FastBullet a = new FastBullet(this.Position, this.Radius, this.HasSpear);
>>>>>>> 86b0e2d2ed915257b2b73fcf19ffe3e0dc3438fa
            a.Parent = parent;
            return a;
        }
        public override BulletType TypeOfBullet => BulletType.FastBullet;
    }
    
    //我还想搞个不是圆形攻击范围的...
}
