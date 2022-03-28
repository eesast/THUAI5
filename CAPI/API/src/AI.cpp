#include <random>
#include "../include/AI.h"

/* 请于 VS2019 项目属性中开启 C++17 标准：/std:c++17 */

// 为假则play()调用期间游戏状态更新阻塞，为真则只保证当前游戏状态不会被状态更新函数与IAPI的方法同时访问
extern const bool asynchronous = false;


// 选手主动技能，选手 !!必须!! 定义此变量来选择主动技能(software type)
extern const THUAI5::SoftwareType playerSoftware = THUAI5::SoftwareType::Booster;

// 选手被动技能，选手 !!必须!! 定义此变量来选择被动技能(hardware type)
extern const THUAI5::HardwareType playerHardware = THUAI5::HardwareType::EnergyConvert;

namespace
{
    [[maybe_unused]] std::uniform_real_distribution<double> direction(0, 2 * 3.1415926);
    [[maybe_unused]] std::default_random_engine e{ std::random_device{}() };
}

int mydirection = 0;

void AI::play(IAPI& api)
{

    auto self = api.GetSelfInfo();
    if (mydirection == 0)
    {
        if (api.GetPlaceType(self->x / 1000 - 1, self->y / 1000) == THUAI5::PlaceType(1))
        {
            mydirection = 1;
        }
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
        auto characters = api.GetRobots();
        api.PrintRobots();
    }

    if (api.GetFrameCount() == 22)
    {
        auto props = api.GetProps();
        api.PrintProps();
    }

    if (api.GetFrameCount() == 23)
    {
        auto bullets = api.GetSignalJammers();
        api.PrintSignalJammers();
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
            api.UseProp();
        }
        else
        {
            api.MoveUp(50);
        }
    }
    if (mydirection == 1)
    {

        if (self->cpuNum != 0)
        {
            api.UseCPU(self->cpuNum);
            // or you can throw it to your teammate:
            // api.ThrowGem(10,1);
        }
    }
    if (mydirection == 2)
    {
        if (api.GetPlaceType(self->x / 1000 + 1, self->y / 1000) == THUAI5::PlaceType(1))
        {
            mydirection = 3;
        }
        else
        {
            api.MoveDown(50);
        }
    }
    if (mydirection == 3)
    {
        if (api.GetPlaceType(self->x / 1000, self->y / 1000 + 1) == THUAI5::PlaceType(1))
        {
            mydirection = 0;
        }
        else
        {
            api.MoveRight(50);
        }
    }
    
    api.Attack(1.0);
}
