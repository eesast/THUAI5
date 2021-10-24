using GameClass.Skill;
using Preparation.Utility;

namespace GameClass.GameObj
{
    public partial class Character
    {
        public object SkillLock => gameObjLock;
        private delegate bool CharacterActiveSkill(Character player); //返回值：是否成功释放了技能
        private delegate void CharacterPassiveSkill(Character player);
        private readonly CharacterActiveSkill commonSkill;
        private readonly ActiveSkillType commonSkillType;
        public ActiveSkillType CommonSkillType => commonSkillType;

        private readonly PassiveSkillType passiveSkillType;
        public PassiveSkillType PassiveSkillType => passiveSkillType;
        public bool UseCommonSkill()
        {
            return commonSkill(this);
        }
        private readonly int timeUntilCommonSkillAvailable = 0; //还剩多少时间可以使用普通技能
        public int TimeUntilCommonSkillAvailable
        {
            get => timeUntilCommonSkillAvailable;
            set
            {
                lock (SkillLock)
                    TimeUntilCommonSkillAvailable = value < 0 ? 0 : value;
            }
        }

        readonly CharacterPassiveSkill passiveSkill;
        public void UsePassiveSkill()
        {
            passiveSkill(this);
            return;
        }
        public Character(XYPosition initPos, int initRadius, PlaceType initPlace, PassiveSkillType passiveSkillType, ActiveSkillType commonSkillType) : base(initPos, initRadius, initPlace)
        {
            this.CanMove = true;
            this.Type = GameObjType.Character;
            this.score = 0;
            this.propInventory = null;
            this.buffManeger = new BuffManeger();
            PassiveSkill pSkill;
            CommonSkill cSkill;
            switch (passiveSkillType)
            {
                case PassiveSkillType.RecoverAfterBattle:
                    pSkill = new RecoverAfterBattle();
                    break;
                case PassiveSkillType.SpeedUpWhenLeavingGrass:
                    pSkill = new SpeedUpWhenLeavingGrass();
                    break;
                case PassiveSkillType.Vampire:
                    pSkill = new Vampire();
                    break;
                default:
                    pSkill = new NoPassiveSkill();
                    break;
            }
            switch (commonSkillType)
            {
                case ActiveSkillType.BecomeAssassin:
                    cSkill = new BecomeAssassin();
                    break;
                case ActiveSkillType.BecomeVampire:
                    cSkill = new BecomeVampire();
                    break;
                case ActiveSkillType.NuclearWeapon:
                    cSkill = new NuclearWeapon();
                    break;
                case ActiveSkillType.SuperFast:
                    cSkill = new SuperFast();
                    break;
                default:
                    cSkill = new NoCommonSkill();
                    break;
            }
            this.attackRange = cSkill.AttackRange;
            this.hp = cSkill.MaxHp;
            this.moveSpeed = cSkill.MoveSpeed;
            this.cd = cSkill.CD;
            this.maxBulletNum = cSkill.MaxBulletNum;
            this.bulletNum = maxBulletNum;
            this.bulletOfPlayer = pSkill.InitBullet;
            this.passiveSkill = pSkill.SkillEffect;
            this.commonSkill = cSkill.SkillEffect;
            this.bulletOfPlayer.Parent = this;
            this.passiveSkillType = passiveSkillType;
            this.commonSkillType = commonSkillType;

            //UsePassiveSkill();  //创建player时开始被动技能，这一过程也可以放到gamestart时进行
            //这可以放在AddPlayer中做

            Debugger.Output(this, "constructed!");
        }
    }
}
