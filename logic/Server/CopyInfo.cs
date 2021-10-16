using System;
using System.Collections.Generic;
using Communication.Proto;
using GameClass.GameObj;
using Preparation.Utility;

namespace Server
{
    public static class CopyInfo
    {
        public static MessageToRefresh Auto(GameObj gameObj)
        {
            if (gameObj.Type == Preparation.Utility.GameObjType.Character)
                return Player((Character)gameObj);
            else if (gameObj.Type == Preparation.Utility.GameObjType.Bullet)
                return Bullet((Bullet)gameObj);
            //还有道具
            else return null;  //先写着防报错
        }
        private static MessageToRefresh Player(Character player)
        {
            MessageToRefresh msg = new MessageToRefresh();
            msg.GameObjType = Communication.Proto.GameObjType.Character;
            msg.MessageOfCharacter = new Communication.Proto.ChangeableCharacterProperty();

            //类型不匹配？attackRange要double吧
            msg.MessageOfCharacter.AttackRange = (int)player.AttackRange;

            msg.MessageOfCharacter.Buff = 0; //没有buff这个属性，是否要加？
            msg.MessageOfCharacter.BulletNum = player.BulletNum;
            msg.MessageOfCharacter.GemNum = player.GemNum;
            msg.MessageOfCharacter.Life = player.HP;
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

            msg.MessageOfCharacter.Speed = player.MoveSpeed;
            msg.MessageOfCharacter.TimeUntilCommonSkillAvailable = player.TimeUntilCommonSkillAvailable;

            //这条暂时没啥用
            msg.MessageOfCharacter.TimeUntilUltimateSkillAvailable = 0;

            msg.MessageOfCharacter.X = player.Position.x;
            msg.MessageOfCharacter.Y = player.Position.y;

            return msg;
        }

        private static MessageToRefresh Bullet(Bullet bullet)
        {
            //防报错
            return null;
        }
    }
}
