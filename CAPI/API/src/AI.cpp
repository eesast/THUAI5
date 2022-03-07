#include <random>
#include "../include/AI.h"

/* 请于 VS2019 项目属性中开启 C++17 标准：/std:c++17 */

// 为假则play()调用期间游戏状态更新阻塞，为真则只保证当前游戏状态不会被状态更新函数与IAPI的方法同时访问
extern const bool asynchronous = false;

// 选手主动技能，选手 !!必须!! 定义此变量来选择主动技能
extern const THUAI5::ActiveSkillType playerActiveSkill = THUAI5::ActiveSkillType::SuperFast;

// 选手被动技能，选手 !!必须!! 定义此变量来选择被动技能
extern const THUAI5::PassiveSkillType playerPassiveSkill = THUAI5::PassiveSkillType::SpeedUpWhenLeavingGrass;

namespace
{
    [[maybe_unused]] std::uniform_real_distribution<double> direction(0, 2 * 3.1415926);
    [[maybe_unused]] std::default_random_engine e{std::random_device{}()};
}

void AI::play(IAPI &api)
{
    //!注：此处的GetFrameCount()仅供通信组和逻辑组debug使用，实际编写时大可以去掉
    auto selfinfo = api.GetSelfInfo();

    // how to know the game frame
    std::cout << api.GetFrameCount() << std::endl;

    // how to get player GUIDS
    if (api.GetFrameCount() == 18)
    {
        auto guids = api.GetPlayerGUIDs();
        for (auto it = guids.begin(); it != guids.end(); it++)
        {
            std::cout << *it << ' ';
        }
        std::cout << std::endl;
    }

    // how to get team score
    if (api.GetFrameCount() == 19)
    {
        std::cout << api.GetTeamScore() << std::endl;
    }

    // how to get the game information & how to view the game information on terminal
    if (api.GetFrameCount() == 20)
    {
        auto selfinfo = api.GetSelfInfo(); // store the value
        api.PrintSelfInfo();               // print the value in a format style
    }

    if (api.GetFrameCount() == 21)
    {
        auto characters = api.GetCharacters();
        api.PrintCharacters();
    }

    if (api.GetFrameCount() == 22)
    {
        auto props = api.GetProps();
        api.PrintProps();
    }

    if (api.GetFrameCount() == 23)
    {
        auto bullets = api.GetBullets();
        api.PrintBullets();
    }

    if (api.GetFrameCount() == 24)
    {
        auto PlaceType = api.GetPlaceType(25, 25); //! use cell instead of grid!
        std::cout << THUAI5::place_dict[PlaceType] << std::endl;
    }

    // how to execute the player
    if (api.GetFrameCount() == 25)
    {
        api.Attack(1.0);
        api.MoveDown(10);
        api.MoveLeft(10);
        api.MoveRight(10);
        api.MoveUp(10);
        api.MovePlayer(10, 1.0);
    }

    // how to convert the two types of position(cell/grid)
    if (api.GetFrameCount() == 26)
    {
        uint32_t gridnumbers = api.CellToGrid(5);
        std::cout << "cell to grid: " << gridnumbers << std::endl;
        std::cout << "grid to cell: " << api.GridToCell(gridnumbers) << std::endl;
    }

    // how to use props
    if (api.GetFrameCount() == 27)
    {
        auto props = api.GetProps();
        if (props.size() != 0)
        {
            api.Pick(props[0]->type);
            api.UseProp();
            // or you can throw it to your teammate:
            // api.ThrowProp(10,1);

            // some examples for get the thuai5 type [code]:
            // auto atombomb=THUAI5::BulletType::AtomBomb;
            // auto circle=THUAI5::ShapeType::Circle;
            // auto superFast=THUAI5::ActiveSkillType::SuperFast;
        }
    }

    // how to use gems
    if (api.GetFrameCount() == 28)
    {
        if (selfinfo->gemNum != 0)
        {
            api.UseGem(selfinfo->gemNum);
            // or you can throw it to your teammate:
            // api.ThrowGem(10,1);
        }
    }

    // how to use skill
    if (api.GetFrameCount() == 29)
    {
        api.UseCommonSkill();
    }

    // how to receive a message
    if (api.MessageAvailable())
    {
        auto message = api.TryGetMessage();
        std::cout << "receive message:" << message.value() << std::endl;
    }

    // how to send a message
    api.Send(1, "this is an example");

    // // ! only for debug!
    if (api.GetFrameCount() == 30)
    {
        exit(0);
    }
}
