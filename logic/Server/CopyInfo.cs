using System;
using System.Collections.Generic;
using Communication.Proto;
using GameClass.GameObj;
using Preparation.Utility;

namespace Server
{
    public static class CopyInfo
    {
        public static MessageToClient.Types.GameObjMessage Auto(GameObj gameObj)
        {
            if (gameObj.Type == Preparation.Utility.GameObjType.Character)
                return Player((Character)gameObj);
            else if (gameObj.Type == Preparation.Utility.GameObjType.Bullet)
                return Bullet((Bullet)gameObj);
            else if (gameObj.Type == Preparation.Utility.GameObjType.Prop)
                return Prop((Prop)gameObj);
            else return null;  //先写着防报错
        }
        private static MessageToClient.Types.GameObjMessage Player(Character player)
        {
            MessageToClient.Types.GameObjMessage msg = new MessageToClient.Types.GameObjMessage();
            msg.MessageOfCharacter = new MessageOfCharacter();

            msg.MessageOfCharacter.X = player.Position.x;
            msg.MessageOfCharacter.Y = player.Position.y;
            msg.MessageOfCharacter.AttackRange = player.AttackRange;
            msg.MessageOfCharacter.Buff = 0; //没有buff这个属性，是否要加？
            msg.MessageOfCharacter.BulletNum = player.BulletNum;  
            msg.MessageOfCharacter.CanMove = player.CanMove;
            msg.MessageOfCharacter.CD = player.CD;
            msg.MessageOfCharacter.GemNum = player.GemNum;
            msg.MessageOfCharacter.Guid = player.ID;
            msg.MessageOfCharacter.IsResetting = player.IsResetting;
            msg.MessageOfCharacter.Life = player.HP;
            msg.MessageOfCharacter.LifeNum = player.DeathCount + 1;
            msg.MessageOfCharacter.Radius = player.Radius;
            msg.MessageOfCharacter.Speed = player.MoveSpeed;
            msg.MessageOfCharacter.TimeUntilCommonSkillAvailable = player.TimeUntilCommonSkillAvailable;
            
            //应该要发队伍分数，这里先发个人分数
            msg.MessageOfCharacter.Score = player.Score;

            //这条暂时没啥用
            msg.MessageOfCharacter.TimeUntilUltimateSkillAvailable = 0;
            msg.MessageOfCharacter.Vampire = player.Vampire;

            switch(player.Place)
            {
                case Preparation.Utility.PlaceType.Land:
                    msg.MessageOfCharacter.Place = Communication.Proto.PlaceType.Land;
                    break;
                case Preparation.Utility.PlaceType.Grass1:
                    msg.MessageOfCharacter.Place = Communication.Proto.PlaceType.Grass1;
                    break;
                case Preparation.Utility.PlaceType.Grass2:
                    msg.MessageOfCharacter.Place = Communication.Proto.PlaceType.Grass2;
                    break;
                case Preparation.Utility.PlaceType.Grass3:
                    msg.MessageOfCharacter.Place = Communication.Proto.PlaceType.Grass3;
                    break;
                case Preparation.Utility.PlaceType.Invisible:
                    msg.MessageOfCharacter.Place = Communication.Proto.PlaceType.Invisible;
                    break;
                default:
                    msg.MessageOfCharacter.Place = Communication.Proto.PlaceType.NullPlaceType;
                    break;
            }

           //Character的储存方式可能得改，用enum type存道具和子弹，不应该用对象
           //现在懒得改了，有时间再重整一波
           switch(player.PropInventory)
            {
                //case Preparation.Utility.PropType.addAP:
                //    msg.MessageOfCharacter.Prop = Communication.Proto.PropType.AddAp;
                //    break;
                default:
                    msg.MessageOfCharacter.Prop = Communication.Proto.PropType.NullPropType;
                    break;
            }
            switch(player.PassiveSkillType)
            {
                case Preparation.Utility.PassiveSkillType.RecoverAfterBattle:
                    msg.MessageOfCharacter.PassiveSkillType = Communication.Proto.PassiveSkillType.RecoverAfterBattle;
                    break;
                case Preparation.Utility.PassiveSkillType.SpeedUpWhenLeavingGrass:
                    msg.MessageOfCharacter.PassiveSkillType = Communication.Proto.PassiveSkillType.SpeedUpWhenLeavingGrass;
                    break;
                case Preparation.Utility.PassiveSkillType.Vampire:
                    msg.MessageOfCharacter.PassiveSkillType = Communication.Proto.PassiveSkillType.Vampire;
                    break;
                default:
                    msg.MessageOfCharacter.PassiveSkillType = Communication.Proto.PassiveSkillType.NullPassiveSkillType;
                    break;
            }
            switch(player.CommonSkillType)
            {
                case Preparation.Utility.ActiveSkillType.BecomeAssassin:
                    msg.MessageOfCharacter.ActiveSkillType = Communication.Proto.ActiveSkillType.BecomeAssassin;
                    break;
                case Preparation.Utility.ActiveSkillType.BecomeVampire:
                    msg.MessageOfCharacter.ActiveSkillType = Communication.Proto.ActiveSkillType.BecomeVampire;
                    break;
                case Preparation.Utility.ActiveSkillType.NuclearWeapon:
                    msg.MessageOfCharacter.ActiveSkillType = Communication.Proto.ActiveSkillType.NuclearWeapon;
                    break;
                case Preparation.Utility.ActiveSkillType.SuperFast:
                    msg.MessageOfCharacter.ActiveSkillType = Communication.Proto.ActiveSkillType.SuperFast;
                    break;
                default:
                    msg.MessageOfCharacter.ActiveSkillType = Communication.Proto.ActiveSkillType.NullActiveSkillType;
                    break;
            }

            switch(player.BulletOfPlayer.TypeOfBullet)
            {
                case Preparation.Utility.BulletType.AtomBomb:
                    msg.MessageOfCharacter.BulletType = Communication.Proto.BulletType.AtomBomb;
                    break;
                case Preparation.Utility.BulletType.Bullet0:
                    msg.MessageOfCharacter.BulletType = Communication.Proto.BulletType.CommonBullet1;
                    break;
                default:
                    msg.MessageOfCharacter.BulletType = Communication.Proto.BulletType.NullBulletType;
                    break;
            }

            return msg;
        }

        private static MessageToClient.Types.GameObjMessage Bullet(Bullet bullet)
        {
            MessageToClient.Types.GameObjMessage msg = new MessageToClient.Types.GameObjMessage();
            msg.MessageOfBullet = new MessageOfBullet();
            msg.MessageOfBullet.FacingDirection = bullet.FacingDirection;
            msg.MessageOfBullet.Guid = bullet.ID;
            switch (bullet.TypeOfBullet)
            {
                case Preparation.Utility.BulletType.AtomBomb:
                    msg.MessageOfCharacter.BulletType = Communication.Proto.BulletType.AtomBomb;
                    break;
                case Preparation.Utility.BulletType.Bullet0:
                    msg.MessageOfCharacter.BulletType = Communication.Proto.BulletType.CommonBullet1;
                    break;
                default:
                    msg.MessageOfCharacter.BulletType = Communication.Proto.BulletType.NullBulletType;
                    break;
            }
            msg.MessageOfBullet.X = bullet.Position.x;
            msg.MessageOfBullet.Y = bullet.Position.y;
            msg.MessageOfBullet.ParentID = bullet.Parent.ID;
            return msg;
        }
        private static MessageToClient.Types.GameObjMessage Prop(Prop prop)
        {
            MessageToClient.Types.GameObjMessage msg = new MessageToClient.Types.GameObjMessage();
            msg.MessageOfProp = new MessageOfProp();
            msg.MessageOfBullet.FacingDirection = prop.FacingDirection;
            msg.MessageOfBullet.Guid = prop.ID;
            switch (prop.GetPropType())
            {
                //case Preparation.Utility.PropType.addAP:
                //    msg.MessageOfCharacter.Prop = Communication.Proto.PropType.AddAp;
                //    break;
                default:
                    msg.MessageOfCharacter.Prop = Communication.Proto.PropType.NullPropType;
                    break;
            }
            msg.MessageOfBullet.X = prop.Position.x;
            msg.MessageOfBullet.Y = prop.Position.y;
            //msg.MessageOfBullet.ParentID = bullet.Parent.Id;
            return null;
        }
    }
}
