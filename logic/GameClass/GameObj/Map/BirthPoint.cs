﻿using Preparation.Interface;
using Preparation.Utility;
using Preparation.GameData;

namespace GameClass.GameObj
{
    /// <summary>
    /// 出生点
    /// </summary>
    public class BirthPoint : ObjOfCharacter
    {
        public BirthPoint(XYPosition initPos) : base(initPos, GameData.numOfPosGridPerCell, PlaceType.Land)
        {
            this.CanMove = false;
            this.Type = GameObjType.BirthPoint;
        }
        // 修改建议：需要避免非自己的玩家进入出生点，否则会重叠
        public override bool IsRigid => true;
        protected override bool IgnoreCollideExecutor(IGameObj targetObj)
        {
            if (targetObj.Type != GameObjType.Character)
                return true;    // 非玩家不碰撞
            else if (targetObj.Type == GameObjType.Character && targetObj.ID == this.Parent.ID)
                return true;    // 出生点所属的玩家不碰撞
            return false;
        }
        public override ShapeType Shape => ShapeType.Square; // 与THUAI4不同，改成了方形
    }
}