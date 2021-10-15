using Preparation.Interface;
using Preparation.Utility;

namespace GameClass.GameObj
{
    /// <summary>
    /// 一切游戏元素的总基类，与THUAI4不同，继承IMoveable接口（出于一切物体其实都是可运动的指导思想）——LHR
    /// </summary>
    public abstract class GameObj : IMoveable
    {   
        protected readonly object gameObjLock = new object();
        /// <summary>
        /// 可移动物体专用锁
        /// </summary>
        public object MoveLock => gameObjLock;

        protected readonly XYPosition birthPos;

        private GameObjType type;
        public GameObjType Type
        {
            get => type;
            set
            {
                lock (gameObjLock)
                {
                    type = value;
                }
            }
        }
        private static long currentMaxID = 0;           //目前游戏对象的最大ID
        public const long invalidID = long.MaxValue;            //无效的ID
        public const long noneID = long.MinValue;
        public long ID { get; }

        private XYPosition position;
        public XYPosition Position
        {
            get => position;
            protected set
            {
                lock (gameObjLock)
                {
                    position = value;
                }
            }
        }
        public abstract bool IsRigid { get; }

        private double facingDirection = 0;
        public double FacingDirection
        {
            get => facingDirection;
            set
            {
                lock (gameObjLock)
                    facingDirection = value;
            }
        }
        public abstract ShapeType Shape { get; }

        private bool canMove;
        public bool CanMove
        {
            get => canMove;
            set
            {
                lock (gameObjLock)
                {
                    canMove = value;
                }
            }
        }

        private bool isMoving;
        public bool IsMoving
        {
            get => isMoving;
            set
            {
                lock (gameObjLock)
                {
                    isMoving = value;
                }
            }
        }

        private bool isResetting;
        public bool IsResetting
        {
            get => isResetting;
            set
            {
                lock (gameObjLock)
                {
                    isResetting = value;
                }
            }
        }
        public bool IsAvailable => !IsMoving && CanMove && !IsResetting;    //是否能接收指令
        public int Radius { get; }

        private PlaceType place;
        public PlaceType Place
        {
            get => place;
            set
            {
                lock (gameObjLock)
                {
                    place = value;
                }
            }
        }
        protected int moveSpeed;
        /// <summary>
        /// 移动速度
        /// </summary>
        public int MoveSpeed
        {
            get => moveSpeed;
            set
            {
                lock (gameObjLock)
                {
                    moveSpeed = value;
                }
            }
        }
        /// <summary>
        /// 原初移动速度，THUAI4在Character类中
        /// </summary>
        private int orgMoveSpeed;
        public int OrgMoveSpeed { get => orgMoveSpeed; protected set { orgMoveSpeed = value; } }
        public virtual bool CanSee(GameObj obj)
        {
            if (obj.Place == PlaceType.Invisible) //先判断是否隐身
                return false;
            if (obj.Place == PlaceType.Land)
                return true;
            if (obj.Place == this.Place)
                return true;
            return false;
        }
        // 移动，改变坐标
        public long Move(Vector moveVec)
        {
            var XYVec = Vector.Vector2XY(moveVec);
            lock (gameObjLock)
            {
                FacingDirection = moveVec.angle;
                this.Position += XYVec;
            }
            return (long)(XYVec.ToVector2() * new Vector2(0, 0));
        }
        /// <summary>
        /// 设置位置
        /// </summary>
        /// <param name="newpos">新位置</param>
        public void SetPosition(XYPosition newpos)
        {
            Position = newpos;
        }
        /// <summary>
        /// 设置移动速度
        /// </summary>
        /// <param name="newMoveSpeed">新速度</param>
        public void SetMoveSpeed(int newMoveSpeed)
        {
            MoveSpeed = newMoveSpeed;
        }
        /// <summary>
        /// 复活时数据重置
        /// </summary>
        public virtual void Reset()
        {
            lock (gameObjLock)
            {

                FacingDirection = 0.0;
                IsMoving = false;
                CanMove = false;
                IsResetting = true;
                this.Position = birthPos;
            }
        }
        /// <summary>
        /// 为了使IgnoreCollide多态化并使GameObj能不报错地继承IMoveable
        /// 在xfgg点播下设计了这个抽象辅助方法，在具体类中实现
        /// </summary>
        /// <returns> 依具体类及该方法参数而定，默认为false </returns> 
        protected virtual bool IgnoreCollideExecutor(IGameObj targetObj) => false;
        bool IMoveable.IgnoreCollide(IGameObj targetObj) => IgnoreCollideExecutor(targetObj);
        public GameObj(XYPosition initPos,int initRadius,PlaceType initPlace)
        {
            this.birthPos = initPos;
            this.Radius = initRadius;
            this.place = initPlace;
        }
    }
}
