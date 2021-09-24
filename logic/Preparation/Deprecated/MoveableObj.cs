
//已弃用：MoveableObj类，包含属性在抽象/具体类实现

/*
using Preparation.Interface;
using Preparation.Utility;

namespace Preparation.GameObj
{
    public abstract class MoveableObj: GameObj,IMoveable
    {
        private int moveSpeed;
        public int MoveSpeed
        {
            get => moveSpeed;
            set 
            {
                lock(gameObjLock)
                {
                    moveSpeed = value;
                }
            }
        }
        protected readonly object moveLock = new object();
        public override ShapeType Shape => ShapeType.Circle; //会移动的一定是圆形
        public MoveableObj(XYPosition initPos, int initRadius, PlaceType initPlace,int initSpeed):base(initPos,initRadius,initPlace)
        {
            moveSpeed = initSpeed;
        }
        public long Move(Vector moveVec)
        {
            var XYVec = Vector.Vector2XY(moveVec);
            lock (moveLock)
            {
                FacingDirection = moveVec.angle;
                this.Position += XYVec;
            }
            return (long)(XYVec.ToVector2()* new Vector2(0, 0));
        }

        public virtual void Reset() //复活时数据重置
        {
            lock (moveLock)
            {
                this.Position = birthPos;
            }
            lock(gameObjLock)
            {
                FacingDirection = 0.0;
                IsMoving = false;
                CanMove = false;
                IsResetting = true;
            }
        }
    }
}
//*/