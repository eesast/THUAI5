using Preparation.GameData;
using Preparation.Interface;
using Preparation.Utility;

namespace GameClass.GameObj
{
    public partial class Character : GameObj, ICharacter	// 负责人LHR摆烂中...
    {
        public readonly object propLock = new object();
        private object beAttackedLock = new object();
        public object PropLock => propLock;
        #region 角色的基本属性及方法，包括与道具、子弹的交互方法
        /// <summary>
        /// 装弹冷却
        /// </summary>
        protected int cd;
        public int CD
        {
            get => cd;
            private set
            {
                lock (gameObjLock)
                {
                    cd = value;
                    Debugger.Output(this, string.Format("'s CD has been set to: {0}.", value));
                }
            }
        }
        public int OrgCD { get; protected set; }
        protected int maxBulletNum;
        public int MaxBulletNum => maxBulletNum;	// 人物最大子弹数
        protected int bulletNum;	
        public int BulletNum => bulletNum;  // 目前持有的子弹数
        public int MaxHp { get; protected set; }    // 最大血量
        protected int hp;
        public int HP
        {
            get => hp;
            set
            {
                lock (gameObjLock)
                    hp = value;
            }
        }
        private int deathCount = 0;       
        public int DeathCount => deathCount;  // 玩家的死亡次数

        protected int ap;   // 当前攻击力
        public int AP
        {
            get => ap;
            private set
            {
                lock (gameObjLock)
                {
                    ap = value;
                    Debugger.Output(this, "'s AP has been set to: " + value.ToString());
                }
            }
        }
        public int OrgAp { get; protected set; }    // 原初攻击力

        private int score = 0;
        public int Score => score;  // 当前分数

        private double attackRange;
        public double AttackRange => attackRange;

        private double vampire = 0; // 回血率：0-1之间
        public double Vampire
        {
            get => vampire;
            set
            {
                if (value > 1)
                    lock(gameObjLock)
                        vampire = 1;
                else if (value < 0)
                    lock (gameObjLock)
                        vampire = 0;
                else
                    lock (gameObjLock)
                        vampire = value;
            }
        }
        public double oriVampire = 0;

        private int level = 1;
        public int Level
        {
            get => level;
            set
            {
                lock(gameObjLock)
                    level = value;
            }
        }

        //可能要改，改成存type比较好吧? 也不一定，先看看吧（自言自语
        private Bullet bulletOfPlayer;
        public Bullet BulletOfPlayer
        {
            get => bulletOfPlayer;
            set
            {
                lock (gameObjLock)
                    bulletOfPlayer = value;
            }
        }

        private Prop? propInventory;
        public Prop? PropInventory  //持有的道具
        {
            get => propInventory;
            set
            {
                lock (gameObjLock)
                {
                    propInventory = value;
                    Debugger.Output(this, " picked the prop: " + (PropInventory == null ? "null" : PropInventory.ToString()));
                }
            }
        }
        private int gemNum = 0;
        public int GemNum
        {
            get => gemNum;
            set
            {
                lock(gameObjLock)
                {
                    gemNum = value;
                }
            }
        }
        /// <summary>
        /// 使用物品栏中的道具
        /// </summary>
        /// <returns>被使用的道具</returns>
        public Prop? UseProp()
        {
            lock (gameObjLock)
            {
                var oldProp = PropInventory;
                PropInventory = null;
                return oldProp;
            }
        }
        /// <summary>
        /// 是否正在更换道具（包括捡起与抛出）
        /// </summary>
        private bool isModifyingProp = false;
        public bool IsModifyingProp
        {
            get => isModifyingProp;
            set
            {
                lock (gameObjLock)
                {
                    isModifyingProp = value;
                }
            }
        }

        /// <summary>
        /// 进行一次远程攻击
        /// </summary>
        /// <param name="posOffset">子弹初始位置偏差值</param>
        /// <returns>攻击操作发出的子弹</returns>
        public Bullet? RemoteAttack(XYPosition posOffset)
        {
            if (TrySubBulletNum()) return ProduceOneBullet(this.Position + posOffset);
            else return null;
        }
        protected Bullet ProduceOneBullet(XYPosition initPos)
        {
            var newBullet = this.bulletOfPlayer.Clone();
            newBullet.SetPosition(initPos);
            return newBullet;
        }

        /// <summary>
        /// 尝试将子弹数量减1
        /// </summary>
        /// <returns>减操作是否成功</returns>
        private bool TrySubBulletNum()	
        {
            lock (gameObjLock)
            {
                if (bulletNum > 0)
                {
                    --bulletNum;
                    return true;
                }
                return false;
            }
        }
        /// <summary>
        /// 尝试将子弹数量加1
        /// </summary>
        /// <returns>加操作是否成功</returns>
        public bool TryAddBulletNum()
        {
            lock (gameObjLock)
            {
                if (bulletNum < maxBulletNum)
                {
                    ++bulletNum;
                    return true;
                }
                return false;
            }
        }
        /// <summary>
        /// 尝试加血
        /// </summary>
        /// <param name="add">欲加量</param>
        /// <returns>加操作是否成功</returns>
        public bool TryAddHp(int add)
        {
            if(hp < MaxHp)
            {
                lock (gameObjLock)
                    hp = MaxHp > hp + add ? hp + add : MaxHp;
                Debugger.Output(this, " hp has added to: " + hp.ToString());
                return true;
            }
            return false;
        }
        /// <summary>
        /// 尝试减血
        /// </summary>
        /// <param name="sub">减血量</param>
        /// <returns>减操作是否成功</returns>
        public bool TrySubHp(int sub)
        {            
            if (hp > 0)
            {
                lock(gameObjLock)
                    hp = 0 >= hp - sub ? 0 : hp - sub;
                Debugger.Output(this, " hp has subed to: " + hp.ToString());
                return true;
            }
            return false;
        }
        /// <summary>
        /// 增加死亡次数
        /// </summary>
        /// <returns>当前死亡次数</returns>
        private int AddDeathCount()
        {
            lock (gameObjLock)
            {
                ++deathCount;
                return deathCount;
            }
        }
        /// <summary>
        /// 加分
        /// </summary>
        /// <param name="add">增加量</param>
        public void AddScore(int add)
        {
            lock (gameObjLock)
            {
                score += add;
                //Debugger.Output(this, " 's score has been added to: " + score.ToString());
            }
        }
        /// <summary>
        /// 减分
        /// </summary>
        /// <param name="sub">减少量</param>
        public void SubScore(int sub)
        {
            lock (gameObjLock)
            {
                score -= sub;
                //Debugger.Output(this, " 's score has been subed to: " + score.ToString());
            }
        }
        /// <summary>
        /// 遭受攻击
        /// </summary>
        /// <param name="subHP"></param>
        /// <param name="hasSpear"></param>
        /// <param name="attacker">伤害来源</param>
        /// <returns>人物在受到攻击后死了吗</returns>
        public bool BeAttack(Bullet bullet)
        {
            lock (beAttackedLock)
            {
                if (hp <= 0) return false;  //原来已经死了
                if (bullet.Parent.TeamID != this.TeamID)
                {
                    if (HasShield)
                        if (bullet.HasSpear)
                            TrySubHp(bullet.AP);
                        else return false;

                    if (hp <= 0) TryActivatingLIFE();  //如果有复活甲
                }
                return hp <= 0;
            }
        }
        /// <summary>
        /// 攻击被反弹，反弹伤害不会再被反弹
        /// </summary>
        /// <param name="subHP"></param>
        /// <param name="hasSpear"></param>
        /// <param name="bouncer">反弹伤害者</param>
        /// <returns>是否因反弹伤害而死</returns>
        private bool BeBounced(int subHP, bool hasSpear, Character? bouncer)
        {
            lock (beAttackedLock)
            {
                if (hp <= 0) return false;
                if (!(bouncer?.TeamID == this.TeamID))
                {
                    if (hasSpear || !HasShield) TrySubHp(subHP);
                    if (hp <= 0) TryActivatingLIFE();
                }
                return hp <= 0;
            }
        }

        /// <summary>
        /// 角色所属队伍ID
        /// </summary>
        private long teamID = long.MaxValue;
        public long TeamID
        {
            get => teamID;
            set
            {
                lock (gameObjLock)
                {
                    teamID = value;
                    Debugger.Output(this, " joins in the team: " + value.ToString());
                }
            }
        }
        /// <summary>
        /// 角色携带的信息
        /// </summary>
        private string message = "THUAI5";
        public string Message
        {
            get => message;
            set
            {
                lock (gameObjLock)
                {
                    message = value;
                }
            }
        }
        #endregion

        #region 角色拥有的buff相关属性、方法（目前还是完全照搬的）
        public void AddMoveSpeed(double add, int buffTime) => buffManeger.AddMoveSpeed(add, buffTime, newVal => { MoveSpeed = newVal; }, OrgMoveSpeed);

        public void AddAP(double add, int buffTime) => buffManeger.AddAP(add, buffTime, newVal => { AP = newVal; }, OrgAp);

        public void ChangeCD(double discount, int buffTime) => buffManeger.ChangeCD(discount, buffTime, newVal => { CD = newVal; }, OrgCD);

        public void AddShield(int shieldTime) => buffManeger.AddShield(shieldTime);
        public bool HasShield => buffManeger.HasShield;

        public void AddLIFE(int LIFETime) => buffManeger.AddLIFE(LIFETime);
        public bool HasLIFE => buffManeger.HasLIFE;

        public void AddSpear(int spearTime) => buffManeger.AddSpear(spearTime);
        public bool HasSpear => buffManeger.HasSpear;

        private void TryActivatingLIFE()
        {
            if (buffManeger.TryActivatingLIFE())
            {
                hp = MaxHp;
            }
        }
        #endregion
        public override void Reset()
        {
            AddDeathCount();
            base.Reset();
            this.moveSpeed = OrgMoveSpeed;
            hp = MaxHp;
            ap = OrgAp;
            PropInventory = null;
            bulletNum = maxBulletNum / 2;
            buffManeger.ClearAll();
        }
        public override bool IsRigid => true;
        public override ShapeType Shape => ShapeType.Circle;
        protected override bool IgnoreCollideExecutor(IGameObj targetObj)
        {
            if (targetObj is BirthPoint && object.ReferenceEquals(((BirthPoint)targetObj).Parent, this))    // 自己的出生点可以忽略碰撞
            {
                return true;
            }
            else if (targetObj is DebuffMine && ((DebuffMine)targetObj).Parent?.TeamID == TeamID)   // 自己队的地雷忽略碰撞
            {
                return true;
            }
            return false;
        }
    }
}
