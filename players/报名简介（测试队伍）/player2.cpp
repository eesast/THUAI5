#include<random>
#include<cmath>
#include"../include/AI.h"
#include<queue>
#include<cstdlib>
#include<thread>
#include<algorithm>   

/* 请于 VS2019 项目属性中开启 C++17 标准：/std:c++17 */

// 为假则play()调用期间游戏状态更新阻塞，为真则只保证当前游戏状态不会被状态更新函数与IAPI的方法同时访问
extern const bool asynchronous = false;


// 选手主动技能，选手 !!必须!! 定义此变量来选择主动技能
extern const THUAI5::SoftwareType playerSoftware = THUAI5::SoftwareType::Invisible;

// 选手被动技能，选手 !!必须!! 定义此变量来选择被动技能
extern const THUAI5::HardwareType playerHardware = THUAI5::HardwareType::EmissionAccessory;

namespace
{
    [[maybe_unused]] std::uniform_real_distribution<double> direction(0, 2 * 3.1415926);
    [[maybe_unused]] std::default_random_engine e{ std::random_device{}() };
}
#define PATHLEN 201

int birth = 5;

struct XYPosition
{
    int x, y;
};
class Utils
{
public:
    static int Map[50][50];

    static void Print(double x)
    {
        std::cout << x << std::endl;
    }

    static void delay(int milliseconds)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
    }

    static double Distance(int x1, int y1, int x2, int y2)
    {
        return sqrt(long(x2 - x1) * (x2 - x1) + long(y2 - y1) * (y2 - y1));
    }

    static int PlaceToInt(THUAI5::PlaceType type)
    {
        switch (type)
        {
        case THUAI5::PlaceType::Land:
            return 0;
        case THUAI5::PlaceType::Wall:
            return 1;
        case THUAI5::PlaceType::BlindZone1:
            return 2;
        case THUAI5::PlaceType::BlindZone2:
            return 3;
        case THUAI5::PlaceType::BlindZone3:
            return 4;
        case THUAI5::PlaceType::BirthPlace1:
            return 5;
        case THUAI5::PlaceType::BirthPlace2:
            return 6;
        case THUAI5::PlaceType::BirthPlace3:
            return 7;
        case THUAI5::PlaceType::BirthPlace4:
            return 8;
        case THUAI5::PlaceType::BirthPlace5:
            return 9;
        case THUAI5::PlaceType::BirthPlace6:
            return 10;
        case THUAI5::PlaceType::BirthPlace7:
            return 11;
        case THUAI5::PlaceType::BirthPlace8:
            return 12;
        case THUAI5::PlaceType::CPUFactory:
            return 13;
        default:
            return 0;
        }
    }

    static bool IsWall(int cellx, int celly)
    {
        if (cellx < 0 || cellx >= 50 || celly < 0 || celly >= 50)
            return false;
        return Map[cellx][celly] == 1 || (Map[cellx][celly] >= 5 && Map[cellx][celly] <= 12 && Map[cellx][celly] != birth);
    }

    static bool IsInAttackRange(std::shared_ptr<const THUAI5::Robot> self, std::shared_ptr<const THUAI5::Robot> enemy)
    {
        return Distance(self->x, self->y, enemy->x, enemy->y) <= 6500;
    }

    static constexpr double PI = 3.141592653589793;
};
int Utils::Map[50][50] = { 0 };
XYPosition path[PATHLEN];  //理论上最大长度不会超过 2*50
XYPosition LastPos[50][50];
bool HasPassed[50][50];
class MoveEngine
{
public:
    MoveEngine(IAPI& api, std::shared_ptr<const THUAI5::Robot> self) :api(api), self(self)
    {
        birth = self->teamID * 4 + self->playerID;
    }

    void AutoMoveTo(int targetx, int targety, XYPosition last_last_pos)
    {
        if (self->x == last_last_pos.x && self->y == last_last_pos.y)
        {
            AdjustPos();
        }
        MoveOneStepTo(targetx, targety);
    }

    void MoveOneStepTo(int x, int y)
    {
        char direction;
        if ((direction = BFS(x, y)) != 0)
        {
            switch (direction)
            {
            case 'u':
                api.MoveUp(moveTime);
                break;
            case 'l':
                api.MoveLeft(moveTime);
                break;
            case 'r':
                api.MoveRight(moveTime);
                break;
            case 'd':
                api.MoveDown(moveTime);
                break;
            }
        }
    }

    void AdjustPos()
    {
        int selfx = self->x / 1000;
        int selfy = self->y / 1000;
        int adjx = 1000 * selfx + 500;
        int adjy = 1000 * selfy + 500;
        int deltax = adjx - self->x;
        int deltay = adjy - self->y;
        int firstdelaytime = 75;
        int secondelaytime = 75;
        Utils::delay(firstdelaytime);
        if (deltax < 0)
            api.MoveUp(moveTime);
        else
            api.MoveDown(moveTime);
        Utils::delay(secondelaytime);
        if (deltay < 0)
            api.MoveLeft(moveTime);
        else
            api.MoveRight(moveTime);
    }

protected:
    char BFS(int x, int y)
    {
        x /= 1000;
        y /= 1000;
        if (Utils::IsWall(x, y))
            return 0;
        memset(path, 0, PATHLEN * sizeof(XYPosition));
        memset(LastPos, 0, 2500 * sizeof(XYPosition));
        memset(HasPassed, 0, 2500 * sizeof(bool));
        int selfx = self->x / 1000;
        int selfy = self->y / 1000;
        std::queue<XYPosition> queue;
        queue.push(XYPosition{ selfx, selfy });
        HasPassed[selfx][selfy] = true;
        while (!queue.empty())
        {
            auto pos = queue.front();
            queue.pop();
            if (pos.x == x && pos.y == y)
                break;
            if (pos.x > 0)
            {
                if (!Utils::IsWall(pos.x - 1, pos.y) && !HasPassed[pos.x - 1][pos.y])
                {
                    LastPos[pos.x - 1][pos.y].x = pos.x;
                    LastPos[pos.x - 1][pos.y].y = pos.y;
                    queue.push(XYPosition{ pos.x - 1, pos.y });
                    HasPassed[pos.x - 1][pos.y] = true;
                }
            }
            if (pos.x <= 48)
            {
                if (!Utils::IsWall(pos.x + 1, pos.y) && !HasPassed[pos.x + 1][pos.y])
                {
                    LastPos[pos.x + 1][pos.y].x = pos.x;
                    LastPos[pos.x + 1][pos.y].y = pos.y;
                    queue.push(XYPosition{ pos.x + 1, pos.y });
                    HasPassed[pos.x + 1][pos.y] = true;
                }
            }
            if (pos.y > 0)
            {
                if (!Utils::IsWall(pos.x, pos.y - 1) && !HasPassed[pos.x][pos.y - 1])
                {
                    LastPos[pos.x][pos.y - 1].x = pos.x;
                    LastPos[pos.x][pos.y - 1].y = pos.y;
                    queue.push(XYPosition{ pos.x, pos.y - 1 });
                    HasPassed[pos.x][pos.y - 1] = true;
                }
            }
            if (pos.y <= 48)
            {
                if (!Utils::IsWall(pos.x, pos.y + 1) && !HasPassed[pos.x][pos.y + 1])
                {
                    LastPos[pos.x][pos.y + 1].x = pos.x;
                    LastPos[pos.x][pos.y + 1].y = pos.y;
                    queue.push(XYPosition{ pos.x, pos.y + 1 });
                    HasPassed[pos.x][pos.y + 1] = true;
                }
            }
        }
        path[0] = XYPosition{ x, y };
        int itr = 0;
        while (path[itr].x != selfx || path[itr].y != selfy)
        {
            path[itr + 1] = LastPos[path[itr].x][path[itr].y];
            itr++;
        }
        if (itr == 0)
            return 0;
        else
        {
            itr--;
            if (path[itr].x > path[itr + 1].x)
            {
                return('d');
            }
            else if (path[itr].x < path[itr + 1].x)
            {
                return('u');
            }
            else if (path[itr].y > path[itr + 1].y)
            {
                return('r');
            }
            else if (path[itr].y < path[itr + 1].y)
            {
                return('l');
            }
        }
        return 0;
    }

private:
    IAPI& api;
    std::shared_ptr<const THUAI5::Robot> self;
    const int moveTime = 75;
};
bool start = true;

void AI::play(IAPI& api)
{
    std::ios::sync_with_stdio(false);
    if (start)
    {
        start = false;
        Utils::delay(50);
        for (int i = 0; i < 50; i++)
        {
            for (int j = 0; j < 50; j++)
            {
                Utils::Map[i][j] = Utils::PlaceToInt(api.GetPlaceType(i, j));
            }
        }
    }
    auto self = api.GetSelfInfo();
    static XYPosition last_pos = XYPosition{ -1, -1 };
    static XYPosition last_last_pos = XYPosition{ -2, -2 };
    MoveEngine moveEngine(api, self);

    auto Robots = api.GetRobots();
    double moveTime = 75;
    std::shared_ptr <const THUAI5::Robot> robot;
    std::vector<std::shared_ptr <const THUAI5::Robot>> enemies;
    for (auto& p : Robots)
    {
        if (p->teamID != self->teamID)
        {
            enemies.push_back(p);
        }
    }
    if (!enemies.empty())
    {
        auto itr = std::min_element(enemies.begin(), enemies.end(),
            [self](std::shared_ptr <const THUAI5::Robot> a, std::shared_ptr <const THUAI5::Robot> b)
            {
                return Utils::Distance(a->x, a->y, self->x, self->y) < Utils::Distance(b->x, b->y, self->x, self->y);
            });
        robot = *itr;
        moveEngine.AutoMoveTo(robot->x, robot->y, last_last_pos);
        if (Utils::IsInAttackRange(self, robot))
        {
            api.UseCommonSkill();
            auto theta = atan2((double)robot->y - self->y, (double)robot->x - self->x);
            api.Attack(theta);
        }
    }

    self = api.GetSelfInfo();
    last_last_pos = last_pos;
    last_pos.x = (int)self->x;
    last_pos.y = (int)self->y;
}
