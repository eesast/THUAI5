#include <random>
#include <cmath>
#include <cstring>
#include "../include/AI.h"

/* 请于 VS2019 项目属性中开启 C++17 标准：/std:c++17 */

// 为假则play()调用期间游戏状态更新阻塞，为真则只保证当前游戏状态不会被状态更新函数与IAPI的方法同时访问
extern const bool asynchronous = false;


// 选手主动技能，选手 !!必须!! 定义此变量来选择主动技能
extern const THUAI5::SoftwareType playerSoftware = THUAI5::SoftwareType::Booster;

// 选手被动技能，选手 !!必须!! 定义此变量来选择被动技能
extern const THUAI5::HardwareType playerHardware = THUAI5::HardwareType::EnergyConvert;

namespace {
    [[maybe_unused]] std::uniform_real_distribution<double> direction(0, 2 * 3.1415926);
    [[maybe_unused]] std::default_random_engine e{ std::random_device{}() };
}

int pre_standing_x, pre_standing_y;

double dist_n2n(double xx1, double yy1, double xx2, double yy2) {  //返回点点距离
    return sqrt((xx1 - xx2) * (xx1 - xx2) + (yy1 - yy2) * (yy1 - yy2));
}

double sqr(double x) {
    return x * x;
}

double dist_n2l(double x, double y, double xx1, double yy1, double xx2, double yy2) { //返回点线距离
    if (sqr(dist_n2n(xx1, yy1, x, y)) + sqr(dist_n2n(xx1, yy1, xx2, yy2)) < sqr(dist_n2n(xx2, yy2, x, y)))
        return dist_n2n(xx1, yy1, x, y);
    else if ((sqr(dist_n2n(xx1, yy1, x, y)) > sqr(dist_n2n(xx1, yy1, xx2, yy2)) + sqr(dist_n2n(xx2, yy2, x, y))))
        return dist_n2n(xx2, yy2, x, y);
    else {
        double A = yy2 - yy1, B = xx1 - xx2, C = xx2 * yy1 - xx1 * yy2;
        return fabs(A * x + B * y + C) / sqrt(A * A + B * B);
    }
}

double calc_angle(double xx1, double yy1, double xx2, double yy2) {  //返回x轴正方向到向量1->2的弧度
    return atan2(yy2 - yy1, xx2 - xx1);
}

THUAI5::PlaceType race_map[52][52];
bool is_strategic_area[52][52];

double find_shortest_path(int now_x, int now_y, int dest_x, int dest_y, double& e) {
    auto wall = THUAI5::PlaceType::Wall;
    const double INF = 1e16;
    const double eps = 1e-6;
    static int location[2000][3];
    int location_number = 0;
    location[++location_number][0] = now_x,
        location[location_number][1] = now_y,
        location[++location_number][0] = dest_x,
        location[location_number][1] = dest_y;
    for (int i = 1; i < 49; i++)
        for (int j = 1; j < 49; j++)
            if (is_strategic_area[i][j] && dist_n2n(i * 1000 + 500, j * 1000 + 500, now_x, now_y) >= 70 && dist_n2n(i * 1000 + 500, j * 1000 + 500, dest_x, dest_y) >= 70)
                location[++location_number][0] = i * 1000 + 500,
                location[location_number][1] = j * 1000 + 500;
    double dist[120][120];
    double lowest_path[120];
    int from[120];
    bool is_visited[120];
	//unsigned int beginning=time(0);
    for (int i = 1; i <= location_number; i++)
        for (int j = 1; j <= location_number; j++)
            if (dist_n2n(location[i][0], location[i][1], location[j][0], location[j][1]) < eps) {
                dist[i][j] = 0;
            }
            else {
                bool b = 1;
                for (int x = 1; x < 49 && b; x++)
                    for (int y = 1; y < 49 && b; y++)
                        if ((race_map[x][y] == wall||race_map[x][y-1]==wall||race_map[x-1][y]==wall||race_map[x-1][y-1]==wall) && dist_n2l(x * 1000, y * 1000, location[i][0], location[i][1], location[j][0], location[j][1]) < 500 - eps) {
                            dist[i][j] = INF, b = 0;
                        }
                if (b) dist[i][j] = dist_n2n(location[i][0], location[i][1], location[j][0], location[j][1]);
            }
	//printf("%u",time(0)-beginning);
    for (int i = 1; i <= location_number; i++)
        from[i] = 2, lowest_path[i] = dist[i][2], is_visited[i] = 0;
    is_visited[2] = 1;
    for (int i = 2; i <= location_number; i++) {
        int t = 1;
        for (int j = 2; j <= location_number; j++) {
            if (!is_visited[j] && lowest_path[j] < lowest_path[t] - eps) t = j;
        }
        is_visited[t] = 1;
        if (t == 1) break;
        for (int j = 1; j <= location_number; j++)
            if (!is_visited[j] && lowest_path[t] + dist[t][j] < lowest_path[j] - eps) {
                lowest_path[j] = lowest_path[t] + dist[t][j];
                from[j] = t;
            }
    }
    e = calc_angle(location[1][0], location[1][1], location[from[1]][0], location[from[1]][1]);
    return lowest_path[1];
}


void AI::play(IAPI& api) {

    const double pi = acos(-1.0);

    std::ios::sync_with_stdio(false);
    auto self = api.GetSelfInfo();
    uint32_t movespeed = 4;

	if(self -> cpuNum > 20)api.UseCPU(self -> cpuNum);
    auto Count = api.GetFrameCount();
    //回合数
    if (Count == 1) {
        auto wall = THUAI5::PlaceType::Wall;
        for (int i = 0; i < 50; i++)
            for (int j = 0; j < 50; j++) {
                race_map[i][j] = api.GetPlaceType(i, j);
                if ((i != self->x / 1000 || j != self->y / 1000) && (race_map[i][j] == THUAI5::PlaceType::BirthPlace1 || race_map[i][j] == THUAI5::PlaceType::BirthPlace2 || race_map[i][j] == THUAI5::PlaceType::BirthPlace3 || race_map[i][j] == THUAI5::PlaceType::BirthPlace4 || race_map[i][j] == THUAI5::PlaceType::BirthPlace5 || race_map[i][j] == THUAI5::PlaceType::BirthPlace6 || race_map[i][j] == THUAI5::PlaceType::BirthPlace7 || race_map[i][j] == THUAI5::PlaceType::BirthPlace8))
                    race_map[i][j] = wall;
        //已经考虑其他人的出生点
            }
        for (int i = 1; i < 49; i++)
            for (int j = 1; j < 49; j++)
                if (race_map[i - 1][j] == wall && race_map[i + 1][j] == wall || race_map[i][j - 1] == wall && race_map[i][j + 1] == wall)
                    race_map[i][j] = wall;
        for (int i = 1; i < 49; i++)
            for (int j = 1; j < 49; j++)
                is_strategic_area[i][j] = 0;
        for (int i = 1; i < 49; i++)
            for (int j = 1; j < 49; j++) {
                if (race_map[i][j] == wall && race_map[i + 1][j] != wall && race_map[i][j + 1] != wall && race_map[i + 1][j + 1] != wall) {
                    is_strategic_area[i + 1][j + 1] = 1;
                }
                if (race_map[i + 1][j] == wall && race_map[i][j] != wall && race_map[i][j + 1] != wall && race_map[i + 1][j + 1] != wall) {
                    is_strategic_area[i][j + 1] = 1;
                }
                if (race_map[i][j + 1] == wall && race_map[i + 1][j] != wall && race_map[i][j] != wall && race_map[i + 1][j + 1] != wall) {
                    is_strategic_area[i + 1][j] = 1;
                }
                if (race_map[i + 1][j + 1] == wall && race_map[i + 1][j] != wall && race_map[i][j + 1] != wall && race_map[i][j] != wall) {
                    is_strategic_area[i][j] = 1;
                }
            }
    }
    auto Props = api.GetProps();
    //判断一号道具是否为cpu
    auto cpu = THUAI5::PropType::CPU;

    double dest_x = 10000, dest_y = 10000, lsp = 1e15,tmp,te;
    double gdest_x = 10000, gdest_y = 10000, glsp = 1e15;
    double e;
    bool icpu = 0;
    for (int i = 0; i < Props.size(); i++) {
        if (Props[i]->type == cpu && lsp > (tmp=find_shortest_path(self->x, self->y, Props[i]->x, Props[i]->y, te))) {
            icpu = 1;
            dest_x = Props[i]->x;
            dest_y = Props[i]->y;
            lsp = tmp;
			e=te;
        }
    }
	
	//新的，发送信息，记得include<cstring>
/*	char Message[100];
	sprintf(Message,"%d %d %d",self->playerID,self->x,self->y);
	
    nt toPlayerID = 0;
    api.Send(toPlayerID, Message);
    nt toPlayerID = 1;
    api.Send(toPlayerID, Message);
    nt toPlayerID = 2;
    api.Send(toPlayerID, Message);
    nt toPlayerID = 3;
    api.Send(toPlayerID, Message);
	
    while (api.MessageAvailable()) {
		int tID,tx,ty;
        auto message = api.TryGetMessage();
		sscanf(message.c_str(),"%d%d%d",&tID,&tx,&ty);
		locat[tID][0]=tx,
		locat[tID][1]=ty;
    }

*/    //if (self->playerID == 0) {
		int i=self->x/1000,j=self->y/1000;
        if (is_strategic_area[i][j] && dist_n2n(self->x, self->y, i * 1000 + 500, j * 1000 + 500) < 25 * movespeed && dist_n2n(self->x, self->y, i * 1000 + 500, j * 1000 + 500) > 3 * movespeed) {
            api.MovePlayer(dist_n2n(self->x, self->y, i * 1000 + 500, j * 1000 + 500) / movespeed, calc_angle(self->x, self->y, i * 1000 + 500, j * 1000 + 500));
        }
        if (icpu) {
            api.MovePlayer(45, e);
            api.Pick(cpu);
        }
        else {
            //去草丛
            if (race_map[self->x / 1000][self->y / 1000] != THUAI5::PlaceType::BlindZone1 && race_map[self->x / 1000][self->y / 1000] != THUAI5::PlaceType::BlindZone2 && race_map[self->x / 1000][self->y / 1000] != THUAI5::PlaceType::BlindZone3) {
                auto BZ1 = THUAI5::PlaceType::BlindZone1,
					BZ2 = THUAI5::PlaceType::BlindZone2,
					BZ3 = THUAI5::PlaceType::BlindZone3;
				for (int i = 1; i < 49; i++)
                    for (int j = 1; j < 49; j++)
                        if (race_map[i][j] == BZ1 || race_map[i][j] == BZ2 || race_map[i][j] == BZ3)
                            if (glsp > dist_n2n(self->x, self->y, i * 1000 + 500, j * 1000 + 500)) {
                                gdest_x = i * 1000 + 500, gdest_y = j * 1000 + 500;
								glsp = dist_n2n(self->x, self->y, i * 1000 + 500, j * 1000 + 500);
                            }
                find_shortest_path(self->x, self->y, gdest_x, gdest_y, e);
                api.MovePlayer(45, e);
            }
        }
    //}


}