#include <random>
#include <iostream>
#include <vector>
#include <deque>
#include <map>
#include <chrono>
#include <sstream>
#include "../include/AI.h"
#pragma warning(disable:C26495)
using namespace std;

/* 请于 VS2019 项目属性中开启 C++17 标准：/std:c++17 */

// 为假则play()调用期间游戏状态更新阻塞，为真则只保证当前游戏状态不会被状态更新函数与IAPI的方法同时访问
extern const bool asynchronous = true;


namespace
{
	[[maybe_unused]] std::uniform_real_distribution<double> direction(0, 2 * 3.1415926);
	[[maybe_unused]] std::default_random_engine e{ std::random_device{}() };
}

auto getTime() {
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}


inline std::map<THUAI5::SignalJammerType, int> jammer_damage
{
 { THUAI5::SignalJammerType::NullJammerType ,0},
 { THUAI5::SignalJammerType::LineJammer ,2000},
 { THUAI5::SignalJammerType::CommonJammer ,2500},
 { THUAI5::SignalJammerType::FastJammer ,1500},
 { THUAI5::SignalJammerType::StrongJammer ,7000}
};

inline std::map<THUAI5::SignalJammerType, int> jammer_range
{
 { THUAI5::SignalJammerType::NullJammerType ,0},
 { THUAI5::SignalJammerType::LineJammer ,4000},
 { THUAI5::SignalJammerType::CommonJammer ,2500},
 { THUAI5::SignalJammerType::FastJammer ,1500},
 { THUAI5::SignalJammerType::StrongJammer ,7000}
};

inline std::map<THUAI5::SignalJammerType, int> jammer_move_range
{
 { THUAI5::SignalJammerType::NullJammerType ,0},
 { THUAI5::SignalJammerType::LineJammer ,900},
 { THUAI5::SignalJammerType::CommonJammer ,4500},
 { THUAI5::SignalJammerType::FastJammer ,9000},
 { THUAI5::SignalJammerType::StrongJammer ,7000}
};

inline std::map<THUAI5::SignalJammerType, int> jammer_boom_frame
{
 { THUAI5::SignalJammerType::NullJammerType ,0},
 { THUAI5::SignalJammerType::LineJammer , 18},
 { THUAI5::SignalJammerType::CommonJammer , 36},
 { THUAI5::SignalJammerType::FastJammer , 36},
 { THUAI5::SignalJammerType::StrongJammer , 70}
};
inline std::map<THUAI5::SignalJammerType, long long> jammer_boom_time
{
 { THUAI5::SignalJammerType::NullJammerType ,0},
 { THUAI5::SignalJammerType::LineJammer , 900},
 { THUAI5::SignalJammerType::CommonJammer , 1800},
 { THUAI5::SignalJammerType::FastJammer , 1800},
 { THUAI5::SignalJammerType::StrongJammer , 3500}
};
inline std::map<THUAI5::SignalJammerType, int> jammer_speed
{
 { THUAI5::SignalJammerType::NullJammerType ,0},
 { THUAI5::SignalJammerType::LineJammer , 1000},
 { THUAI5::SignalJammerType::CommonJammer , 2500},
 { THUAI5::SignalJammerType::FastJammer , 5000},
 { THUAI5::SignalJammerType::StrongJammer , 2000}
};

const double PI = 3.1415926535897932384626433832795;

double sqr(double x) { return x * x; }
double getAngle(double x, double y) { return atan2(y, x); }
IAPI* A;

//scouts
// 选手主动技能，选手 !!必须!! 定义此变量来选择主动技能 
extern const THUAI5::SoftwareType playerSoftware = THUAI5::SoftwareType::Amplification;

// 选手被动技能，选手 !!必须!! 定义此变量来选择被动技能
extern const THUAI5::HardwareType playerHardware = THUAI5::HardwareType::PowerBank;

const int maxn = 55;
THUAI5::PlaceType BP, BP1;
bool Cross[maxn][maxn][maxn][maxn], WdCross[maxn][maxn][maxn][maxn], OK[maxn][maxn], ThrowCro[maxn][maxn][maxn][maxn], Throwable[maxn][maxn];
int inpLine[maxn][maxn];
double befacing[maxn][maxn];
THUAI5::PlaceType mAp[55][55];
int blc[55][55];// Which cell to be blocked
int inLine[55][55];
std::shared_ptr<const THUAI5::SignalJammer> inlu[55][55];
void Cover(IAPI& api, std::shared_ptr<const THUAI5::SignalJammer> u) {
	double C = cos(u->facingDirection) * jammer_speed[u->type] * 0.05,
		S = sin(u->facingDirection) * jammer_speed[u->type] * 0.05,
		CC = cos(u->facingDirection),
		SS = sin(u->facingDirection);
	if (u->type == THUAI5::SignalJammerType::LineJammer) {
		for (int i = 0; i <= jammer_boom_frame[u->type]; i++)
			blc[api.GridToCell(int(u->x + C * i - SS * 200))][api.GridToCell(int(u->y + S * i + CC * 200))]++,
			blc[api.GridToCell(int(u->x + C * i + SS * 200))][api.GridToCell(int(u->y + S * i - CC * 200))]++,
			inLine[api.GridToCell(int(u->x + C * i + SS * 200))][api.GridToCell(int(u->y + S * i - CC * 200))]++,
			inlu[api.GridToCell(int(u->x + C * i + SS * 200))][api.GridToCell(int(u->y + S * i - CC * 200))] = u,
			inLine[api.GridToCell(int(u->x + C * i - SS * 200))][api.GridToCell(int(u->y + S * i + CC * 200))]++,
			inlu[api.GridToCell(int(u->x + C * i - SS * 200))][api.GridToCell(int(u->y + S * i + CC * 200))] = u;
		for (int i = 6; i <= 46; i++) // To be improved 
			blc[api.GridToCell(int(u->x + C * i))][api.GridToCell(int(u->y + S * i))]++,
			blc[api.GridToCell(int(u->x + C * i - SS * 500))][api.GridToCell(int(u->y + S * i + CC * 500))]++,
			blc[api.GridToCell(int(u->x + C * i + SS * 500))][api.GridToCell(int(u->y + S * i - CC * 500))]++,
			inLine[api.GridToCell(int(u->x + C * i))][api.GridToCell(int(u->y + S * i))]++,
			inlu[api.GridToCell(int(u->x + C * i))][api.GridToCell(int(u->y + S * i))] = u,
			inLine[api.GridToCell(int(u->x + C * i - SS * 500))][api.GridToCell(int(u->y + S * i + CC * 500))]++,
			inlu[api.GridToCell(int(u->x + C * i - SS * 500))][api.GridToCell(int(u->y + S * i + CC * 500))] = u,
			inLine[api.GridToCell(int(u->x + C * i + SS * 500))][api.GridToCell(int(u->y + S * i - CC * 500))]++,
			inlu[api.GridToCell(int(u->x + C * i + SS * 500))][api.GridToCell(int(u->y + S * i - CC * 500))] = u;
	}
	else {
		int x = int(u->x + jammer_boom_frame[u->type] * C), y = (int)(u->y + jammer_boom_frame[u->type] * S);
		for (int i = 0; i <= jammer_boom_frame[u->type]; i++) {
			blc[api.GridToCell(int(u->x + C * i - SS * 200))][api.GridToCell(int(u->y + S * i + CC * 200))]++,
				blc[api.GridToCell(int(u->x + C * i + SS * 200))][api.GridToCell(int(u->y + S * i - CC * 200))]++;
			inpLine[api.GridToCell(int(u->x + C * i - SS * 200))][api.GridToCell(int(u->y + S * i + CC * 200))] ++;
			inpLine[api.GridToCell(int(u->x + C * i + SS * 200))][api.GridToCell(int(u->y + S * i - CC * 200))] ++;
			befacing[api.GridToCell(int(u->x + C * i - SS * 200))][api.GridToCell(int(u->y + S * i + CC * 200))] = u->facingDirection;
			befacing[api.GridToCell(int(u->x + C * i + SS * 200))][api.GridToCell(int(u->y + S * i - CC * 200))] = u->facingDirection;
			if (mAp[api.GridToCell(int(u->x + C * i))][api.GridToCell(int(u->y + S * i))] == THUAI5::PlaceType::Wall
				|| mAp[api.GridToCell(int(u->x + C * i - SS * 200))][api.GridToCell(int(u->y + S * i + CC * 200))] == THUAI5::PlaceType::Wall
				|| mAp[api.GridToCell(int(u->x + C * i + SS * 200))][api.GridToCell(int(u->y + S * i - CC * 200))] == THUAI5::PlaceType::Wall) {
				x = int(u->x + C * i), y = int(u->y + S * i);
				break;
			}
		}
		int X = api.GridToCell(x), Y = api.GridToCell(y);
		int t = api.GridToCell(jammer_range[u->type] + 1000);

		for (int i = -t; i <= t; i++) for (int j = -t; j <= t; j++)
			if (i + X >= 0 && i + X < 50 && j + Y >= 0 && j + Y < 50)
				if (sqr(api.CellToGrid(X + i) - x) + sqr(api.CellToGrid(Y + j) - y) <= sqr(jammer_range[u->type] + 500))
					blc[i + X][j + Y] ++;
	}
}
void Recover(IAPI& api, std::shared_ptr<const THUAI5::SignalJammer> u) {
	double C = cos(u->facingDirection) * jammer_speed[u->type] * 0.05,
		S = sin(u->facingDirection) * jammer_speed[u->type] * 0.05,
		CC = cos(u->facingDirection),
		SS = sin(u->facingDirection);
	if (u->type == THUAI5::SignalJammerType::LineJammer) {
		for (int i = 0; i <= jammer_boom_frame[u->type]; i++)
			blc[api.GridToCell(int(u->x + C * i - SS * 200))][api.GridToCell(int(u->y + S * i + CC * 200))]--,
			blc[api.GridToCell(int(u->x + C * i + SS * 200))][api.GridToCell(int(u->y + S * i - CC * 200))]--,
			inLine[api.GridToCell(int(u->x + C * i + SS * 200))][api.GridToCell(int(u->y + S * i - CC * 200))]--,
			inLine[api.GridToCell(int(u->x + C * i - SS * 200))][api.GridToCell(int(u->y + S * i + CC * 200))]--;
		for (int i = 6; i <= 46; i++) // To be improved 
			blc[api.GridToCell(int(u->x + C * i))][api.GridToCell(int(u->y + S * i))]--,
			blc[api.GridToCell(int(u->x + C * i - SS * 500))][api.GridToCell(int(u->y + S * i + CC * 500))]--,
			blc[api.GridToCell(int(u->x + C * i + SS * 500))][api.GridToCell(int(u->y + S * i - CC * 500))]--,
			inLine[api.GridToCell(int(u->x + C * i))][api.GridToCell(int(u->y + S * i))]--,
			inLine[api.GridToCell(int(u->x + C * i - SS * 500))][api.GridToCell(int(u->y + S * i + CC * 500))]--,
			inLine[api.GridToCell(int(u->x + C * i + SS * 500))][api.GridToCell(int(u->y + S * i - CC * 500))]--;
	}
	else {
		int x = int(u->x + jammer_boom_frame[u->type] * C), y = (int)(u->y + jammer_boom_frame[u->type] * S);
		for (int i = 0; i <= jammer_boom_frame[u->type]; i++) {
			blc[api.GridToCell(int(u->x + C * i - SS * 200))][api.GridToCell(int(u->y + S * i + CC * 200))]--,
				blc[api.GridToCell(int(u->x + C * i + SS * 200))][api.GridToCell(int(u->y + S * i - CC * 200))]--;

			inpLine[api.GridToCell(int(u->x + C * i - SS * 200))][api.GridToCell(int(u->y + S * i + CC * 200))] --;
			inpLine[api.GridToCell(int(u->x + C * i + SS * 200))][api.GridToCell(int(u->y + S * i - CC * 200))] --;
			if (mAp[api.GridToCell(int(u->x + C * i))][api.GridToCell(int(u->y + S * i))] == THUAI5::PlaceType::Wall
				|| mAp[api.GridToCell(int(u->x + C * i - SS * 200))][api.GridToCell(int(u->y + S * i + CC * 200))] == THUAI5::PlaceType::Wall
				|| mAp[api.GridToCell(int(u->x + C * i + SS * 200))][api.GridToCell(int(u->y + S * i - CC * 200))] == THUAI5::PlaceType::Wall) {
				x = int(u->x + C * i), y = int(u->y + S * i);
				break;
			}
		}
		int X = api.GridToCell(x), Y = api.GridToCell(y);
		int t = api.GridToCell(jammer_range[u->type] + 1000);

		for (int i = -t; i <= t; i++) for (int j = -t; j <= t; j++)
			if (i + X >= 0 && i + X < 50 && j + Y >= 0 && j + Y < 50)
				if (sqr(api.CellToGrid(X + i) - x) + sqr(api.CellToGrid(Y + j) - y) <= sqr(jammer_range[u->type] + 500))
					blc[i + X][j + Y] --;
	}
}

bool NoEnemyNear(int X, int Y) {
	static int tid = A->GetSelfInfo()->teamID;
	auto R = A->GetRobots();
	for (auto u : R) if (!u->isResetting && u->teamID != tid && sqr(u->x - X) + sqr(u->y - Y) <= 3000 * 3000)
		return 0;
	return 1;
}

double Mindis, Mindis2;
pair<int, int>find_Safe(IAPI& api, std::shared_ptr<const THUAI5::Robot>self) {
	pair<int, int>ret = make_pair(-1, -1), ret2 = make_pair(-1, -1);
	Mindis = Mindis2 = 1e15;
	int X = api.GridToCell(self->x), Y = api.GridToCell(self->y);
	for (int u = 0; u <= 10; u++) for (int v = 0; v <= 10; v++) for (int ud = -1; ud <= 1; ud += 2) for (int vd = -1; vd <= 1; vd += 2) {
		int i = u * ud, j = v * vd;
		if (X + i >= 0 && X + i < 50 && Y + j >= 0 && Y + j < 50 && !Cross[X][Y][X + i][Y + j]) {
			double d = sqr(api.CellToGrid(X + i) - self->x) + sqr(api.CellToGrid(Y + j) - self->y);
			if (!blc[X + i][Y + j]) {
				if (d < Mindis) {
					Mindis = d;
					ret = make_pair(api.CellToGrid(X + i), api.CellToGrid(Y + j));
				}
				if (d < Mindis2 && NoEnemyNear(A->CellToGrid(X + i), A->CellToGrid(Y + j))) {
					Mindis2 = d;
					ret2 = make_pair(api.CellToGrid(X + i), api.CellToGrid(Y + j));
				}
			}
		}
	}
	if (Mindis2 <= 6000 && Mindis2 <= Mindis + 2000)
		return ret2;
	return ret;
}

bool inNearAngle(double angle, double beta) {
	int t = floor((angle - beta) / 2 / PI);
	beta += t * 2 * PI;
	return (angle - beta) <= 0.6 * PI || (2 * PI - (angle - beta) <= 0.6 * PI);
}

pair<int, int>find_Safe_a(IAPI& api, std::shared_ptr<const THUAI5::Robot>self, double angle) {
	pair<int, int>ret = make_pair(-1, -1), ret2 = make_pair(-1, -1);
	Mindis = Mindis2 = 1e15;
	int X = api.GridToCell(self->x), Y = api.GridToCell(self->y);
	for (int u = 0; u <= 10; u++) for (int v = 0; v <= 10; v++) for (int ud = -1; ud <= 1; ud += 2) for (int vd = -1; vd <= 1; vd += 2) {
		int i = u * ud, j = v * vd;
		if (X + i >= 0 && X + i < 50 && Y + j >= 0 && Y + j < 50 && !Cross[X][Y][X + i][Y + j] && inNearAngle(angle, atan2(api.CellToGrid(Y + j) - self->y, api.CellToGrid(X + i) - self->x))) {
			double d = sqr(api.CellToGrid(X + i) - self->x) + sqr(api.CellToGrid(Y + j) - self->y);
			if (!blc[X + i][Y + j]) {
				if (d < Mindis) {
					Mindis = d;
					ret = make_pair(api.CellToGrid(X + i), api.CellToGrid(Y + j));
				}
				if (d < Mindis2 && NoEnemyNear(A->CellToGrid(X + i), A->CellToGrid(Y + j))) {
					Mindis2 = d;
					ret2 = make_pair(api.CellToGrid(X + i), api.CellToGrid(Y + j));
				}
			}
		}
	}
	if (Mindis2 <= 6000 && Mindis2 <= Mindis + 2000)
		return ret2;
	return ret;
}
void Move(IAPI& api, std::shared_ptr<const THUAI5::Robot>self, pair<int, int>dst) {
	if (dst.first == self->x && dst.second == self->y)
		return;
	double angle = atan2(dst.second - self->y, dst.first - self->x);
	api.MovePlayer(min(200.0, sqrt(sqr(dst.second - self->y) + sqr(dst.first - self->x)) / self->speed * 1000), angle);
	//cout << '@' << t << endl;
}

//collector

int PropCnt[maxn][maxn];
int Dis[maxn][maxn], vis[maxn][maxn], idx;
int dir[4][2] = { {1,0},{-1,0},{0,1},{0,-1} };
struct node {
	int x, y;
	node(int x = 0, int y = 0) :x(x), y(y) {}
	bool operator == (const node& p)const { return x == p.x && y == p.y; }
	bool operator != (const node& p)const { return !(*this == p); }
	bool operator < (const node& p)const { return Dis[x][y] < Dis[p.x][p.y]; }
}Q[maxn * maxn], pre[maxn][maxn], Beautiful_Place = node(29, 14);
node BFSPoint(node st, node goal) {
	idx++;
	int H = 0, T = 0;
	Q[H = T = 1] = st, Dis[st.x][st.y] = 0;
	while (H <= T) {
		node now = Q[H++];
		int len = Dis[now.x][now.y];
		for (int i = 0; i < 4; i++) {
			int x = now.x + dir[i][0], y = now.y + dir[i][1];
			if (x >= 0 && x < 50 && y >= 0 && y < 50 && vis[x][y] < idx && OK[x][y]) {
				vis[x][y] = idx;
				pre[x][y] = now;
				Dis[x][y] = len + 1;
				Q[++T] = node(x, y);
				if (Q[T] == goal) return Q[T];
			}
		}
	}
	return st;
}
node BFSThrow(node st) {
	idx++;
	int H = 0, T = 0;
	Q[H = T = 1] = st, Dis[st.x][st.y] = 0;
	while (H <= T) {
		node now = Q[H++];
		int len = Dis[now.x][now.y];
		for (int i = 0; i < 4; i++) {
			int x = now.x + dir[i][0], y = now.y + dir[i][1];
			if (x >= 0 && x < 50 && y >= 0 && y < 50 && vis[x][y] < idx && OK[x][y]) {
				vis[x][y] = idx;
				pre[x][y] = now;
				Dis[x][y] = len + 1;
				Q[++T] = node(x, y);
				if (Throwable[x][y]) return Q[T];
			}
		}
	}
	return st;
}
void BFSPath(node st) {
	idx++;
	int H = 0, T = 0;
	Q[H = T = 1] = st, Dis[st.x][st.y] = 0;
	while (H <= T) {
		node now = Q[H++];
		int len = Dis[now.x][now.y];
		for (int i = 0; i < 4; i++) {
			int x = now.x + dir[i][0], y = now.y + dir[i][1];
			if (x >= 0 && x < 50 && y >= 0 && y < 50 && vis[x][y] < idx && OK[x][y]) {
				vis[x][y] = idx;
				pre[x][y] = now;
				Dis[x][y] = len + 1;
				Q[++T] = node(x, y);
			}
		}
	}
}
inline bool Direct(node& a, node& b) {
	return !Cross[a.x][a.y][b.x][b.y];
}
struct Message {
	node pt;
	int type, dis;
	Message() {}
	Message(node pt, int type, int dis) :pt(pt), type(type), dis(dis) {}
	string Encode() {
		return to_string(pt.x) + " " + to_string(pt.y) + " " + to_string(type) + " " + to_string(dis);
	}
};
inline Message Decode(optional<string> s) {
	stringstream trans(*s);
	Message ret;
	trans >> ret.pt.x >> ret.pt.y >> ret.type >> ret.dis;
	return ret;
}
void init_Cross(bool Cro[maxn][maxn][maxn][maxn]) {
	for (int i = 1; i < 49; i++)
		for (int j = 1; j < 49; j++)
			for (int u = i; u < 49; u++)
				for (int v = 1; v < 49; v++) {
					double beta = atan2(v - j, u - i) + PI / 2;
					double S = sin(beta), C = cos(beta);
					bool crossflg = 0;
					if (i < u && j != v) {
						double L = 1.0 * (v - j) / (u - i);
						for (int o = i + 1; o <= u && !crossflg; o++) {
							int x = o, y = j + 0.5 + (o - i - 0.5) * L;
							if (!OK[x - 1][y] || !OK[x][y]) crossflg = 1;
						}
						for (int o = (int)(i + 0.5 + C * 0.5) + 1; o <= (int)(u + 0.5 + C * 0.5) && !crossflg; o++) {
							int x = o, y = j + 0.5 + S * 0.5 + (o - i - 0.5 - C * 0.5) * L;
							if (!OK[x - 1][y] || !OK[x][y]) crossflg = 1;
						}
						for (int o = (int)(i + 0.5 - C * 0.5) + 1; o <= (int)(u + 0.5 - C * 0.5) && !crossflg; o++) {
							int x = o, y = j + 0.5 - S * 0.5 + (o - i - 0.5 + C * 0.5) * L;
							if (!OK[x - 1][y] || !OK[x][y]) crossflg = 1;
						}
						L = 1.0 * (u - i) / (v - j);
						int I = (j < v ? i : u), J = (j < v ? j : v);
						for (int o = min(v, j) + 1, lim = max(v, j); o <= lim && !crossflg; o++) {
							int y = o, x = I + 0.5 + (o - J - 0.5) * L;
							if (!OK[x][y - 1] || !OK[x][y]) crossflg = 1;
						}
						for (int o = (int)(min(v, j) + 0.5 + S * 0.5) + 1; o <= (int)(max(v, j) + 0.5 + S * 0.5) && !crossflg; o++) {
							int y = o, x = I + 0.5 + C * 0.5 + (o - J - 0.5 - S * 0.5) * L;
							if (!OK[x][y - 1] || !OK[x][y]) crossflg = 1;
						}
						for (int o = (int)(min(v, j) + 0.5 - S * 0.5) + 1; o <= (int)(max(v, j) + 0.5 - S * 0.5) && !crossflg; o++) {
							int y = o, x = I + 0.5 - C * 0.5 + (o - J - 0.5 + S * 0.5) * L;
							if (!OK[x][y - 1] || !OK[x][y]) crossflg = 1;
						}
					}
					else if (i < u) {//竖直线特判
						for (int o = i; o <= u && !crossflg; o++)
							if (!OK[o][j]) crossflg = 1;
					}
					else if (j != v) {//水平线特判
						for (int o = min(j, v), lim = max(j, v); o <= lim && !crossflg; o++)
							if (!OK[i][o]) crossflg = 1;
					}
					Cro[i][j][u][v] = Cro[u][v][i][j] = crossflg;
				}
}

/* 对强力干扰弹效用的判断 */
int chk(double angle, int x, int y) {
	double ca = cos(angle), sa = sin(angle);
	int L = 0, R = 7000, mid;
	for (; L < R;) {
		mid = (L + R) >> 1;
		if (Cross[A->GridToCell(x)][A->GridToCell(y)][A->GridToCell(x + ca * mid)][A->GridToCell(y + sa * mid)]) R = mid;
		else L = mid + 1;
	}
	x += ca * (L - 500), y += sa * (L - 500);
	auto Rb = A->GetRobots();
	int cnt = 0;
	for (auto u : Rb)
		if (u->teamID != A->GetSelfInfo()->teamID)
			if (sqr(u->x - x) + sqr(u->y - y) <= 5000 * 5000)
				cnt += 1 + u->cpuNum;
	return cnt;
}

pair<int, int>zjd[55][55];

void AI::play(IAPI& api)
{
	std::ios::sync_with_stdio(false);
	auto self = api.GetSelfInfo();

	bool haveSignal = 1;
	if (self->signalJammerNum == 0) haveSignal = 0;

	static long long tf = getTime();
	auto disToSelf = [self](int x, int y) { return sqrt(sqr(x - self->x) + sqr(y - self->y)); };
	auto getAngleToSelf = [self](int x, int y) { return getAngle(x - self->x, y - self->y); };

	bool final_moment = 0;
	if (getTime() - tf > (9 * 60 + 29) * 1000) api.UseCPU(self->cpuNum), final_moment = 1;
	//只剩30s直接用，没分也不给敌人留

	//预处理：
	static int Prepare = 0;
	static std::map<THUAI5::PropType, int>PT_int;
	static int bx1 = 0, by1 = 0, BX[10], BY[10];
	static node blind[5];

	if (Prepare++ == 0) {
		A = &api;
		unsigned char _tmp = self->teamID * 4 + self->playerID;
		BP1 = self->teamID == 0 ? THUAI5::PlaceType::BirthPlace1 : THUAI5::PlaceType::BirthPlace8;
		if (_tmp == 0) BP = THUAI5::PlaceType::BirthPlace1;
		if (_tmp == 1) BP = THUAI5::PlaceType::BirthPlace2;
		if (_tmp == 2) BP = THUAI5::PlaceType::BirthPlace3;
		if (_tmp == 3) BP = THUAI5::PlaceType::BirthPlace4;
		if (_tmp == 4) BP = THUAI5::PlaceType::BirthPlace5;
		if (_tmp == 5) BP = THUAI5::PlaceType::BirthPlace6;
		if (_tmp == 6) BP = THUAI5::PlaceType::BirthPlace7;
		if (_tmp == 7) BP = THUAI5::PlaceType::BirthPlace8;
		for (int i = 0; i < 50; i++)
			for (int j = 0; j < 50; j++) {
				mAp[i][j] = api.GetPlaceType(i, j);
				if (mAp[i][j] == BP1) {
					bx1 = i, by1 = j;
				}
				OK[i][j] = (
					mAp[i][j] == THUAI5::PlaceType::Land ||
					mAp[i][j] == BP ||
					mAp[i][j] == THUAI5::PlaceType::BlindZone1 ||
					mAp[i][j] == THUAI5::PlaceType::BlindZone2 ||
					mAp[i][j] == THUAI5::PlaceType::BlindZone3 ||
					mAp[i][j] == THUAI5::PlaceType::CPUFactory
					);
				if (mAp[i][j] == THUAI5::PlaceType::BirthPlace1) BX[1] = i, BY[1] = j;
				if (mAp[i][j] == THUAI5::PlaceType::BirthPlace2) BX[2] = i, BY[2] = j;
				if (mAp[i][j] == THUAI5::PlaceType::BirthPlace3) BX[3] = i, BY[3] = j;
				if (mAp[i][j] == THUAI5::PlaceType::BirthPlace4) BX[4] = i, BY[4] = j;
				if (mAp[i][j] == THUAI5::PlaceType::BirthPlace5) BX[5] = i, BY[5] = j;
				if (mAp[i][j] == THUAI5::PlaceType::BirthPlace6) BX[6] = i, BY[6] = j;
				if (mAp[i][j] == THUAI5::PlaceType::BirthPlace7) BX[7] = i, BY[7] = j;
				if (mAp[i][j] == THUAI5::PlaceType::BirthPlace8) BX[8] = i, BY[8] = j;
			}
		Beautiful_Place.y -= self->playerID;

		init_Cross(Cross);//此时的Cross用于开炮，移动判定
		OK[bx1][by1] = 1;
		init_Cross(ThrowCro);//此时的ThrowCro用于扔CPU判定
		OK[bx1][by1] = (BP == BP1);

		for (int i = 1; i < 49; i++)//扩大躲弹判定范围
			for (int j = 1; j < 49; j++)
				if (OK[i][j])
					for (int u = 1; u < 49; u++)
						for (int v = 1; v < 49; v++)
							if (Cross[i][j][u][v]) {
								for (int d = 0; d < 4; d++) {
									WdCross[i + dir[d][0]][j + dir[d][1]][u][v] = 1;
								}
								WdCross[i][j][u][v] = 1;
							}

		//预处理可扔点：
		int goalx = api.CellToGrid(bx1), goaly = api.CellToGrid(by1);
		for (int i = 1; i < 49; i++)
			for (int j = 1; j < 49; j++) {
				int x = api.CellToGrid(i), y = api.CellToGrid(j);
				if (1ll * (x - goalx) * (x - goalx) + 1ll * (y - goaly) * (y - goaly) <= 14000 * 14000ll) {
					bool flg = 0;
					for (int u = -1; u <= 1 && !flg; u++)
						for (int v = -1; v <= 1; v++)
							if (ThrowCro[i + u][j + v][bx1][by1]) {
								flg = 1; break;
							}
					if (!flg) Throwable[i][j] = 1;
				}
			}

		//预处理最近障碍点
		for (int i = 1; i < 49; i++) for (int j = 1; j < 49; j++) if (mAp[i][j] == THUAI5::PlaceType::Wall)
			for (int u = 1; u < 49; u++) for (int v = 1; v < 49; v++)
				if (zjd[u][v] == make_pair(0, 0) || sqr(u - zjd[u][v].first) + sqr(v - zjd[u][v].second) >= sqr(u - i) + sqr(v - j))
					zjd[u][v] = make_pair(i, j);
		// 优先级
		PT_int[THUAI5::PropType::CPU] = 1;
		PT_int[THUAI5::PropType::Shield] = 2;
		PT_int[THUAI5::PropType::Booster] = 3;
		PT_int[THUAI5::PropType::Battery] = 2;
		PT_int[THUAI5::PropType::ShieldBreaker] = 5;
		//PT_int[THUAI5::PropType::NullPropType] = 0;

		//草丛位置
		if (BX[1] == BX[2]) {//第一张图
			blind[1] = node(29, 16);
			blind[2] = node(17, 25);
			blind[3] = node(22, 16);
			blind[4] = node(35, 44);
		}
		else {
			blind[1] = node(25, 9);
			blind[2] = node(25, 41);
			blind[3] = node(18, 25);
			blind[4] = node(32, 25);
		}

	}

	//对道具的处理放到开头，不然会被移动打断

	auto props = api.GetProps();

	int GX = api.GridToCell(self->x), GY = api.GridToCell(self->y);
	bool Accmulate_CPU = (getTime() - tf < 9 * 60 * 1000 || BP != BP1);
	//其他人有就扔，1号在出生点外时扔
	if ((BP != BP1 || (GX != bx1 || GY != by1)) && Throwable[api.GridToCell(self->x)][api.GridToCell(self->y)])
		api.ThrowCPU(disToSelf(api.CellToGrid(bx1), api.CellToGrid(by1)) / 3, getAngleToSelf(api.CellToGrid(bx1), api.CellToGrid(by1)), self->cpuNum);

	vector<pair<int, node> >target;
	//memset(PropCnt, 0, sizeof PropCnt);
	for (auto& P : props) if (!P->isMoving) {
		int x = api.GridToCell(P->x), y = api.GridToCell(P->y);
		if (P->type == THUAI5::PropType::CPU) {
			if (mAp[x][y] != BP1 || !Accmulate_CPU) {
				if (x == GX && y == GY) api.Pick(THUAI5::PropType::CPU);
				else if (OK[x][y]) target.push_back(make_pair(PT_int[P->type], node(x, y)));
			}
		}
		else {
			if (x == GX && y == GY) {
				if (PT_int[self->prop] >= PT_int[P->type]) api.UseProp(), api.Pick(P->type);
				else { api.ThrowProp(1, 0), api.Pick(P->type); }
			}
			else if (OK[x][y]) target.push_back(make_pair(PT_int[P->type], node(x, y)));
		}
	}
	//以上是对地图道具的处理

	auto R = api.GetRobots();

	if (playerSoftware == THUAI5::SoftwareType::Invisible) {
		for (auto u : R) if (!u->isResetting && u->teamID != self->teamID && disToSelf(u->x, u->y) <= 5000) {
			api.UseCommonSkill();
		}
	}
	for (auto u : R)//2000码反向
		if (!u->isResetting && disToSelf(u->x, u->y) <= 2000)
			api.MovePlayer(100, PI + getAngleToSelf(u->x, u->y));
	if (haveSignal) {
		if (playerSoftware == THUAI5::SoftwareType::Amplification &&
			api.GetSelfInfo()->signalJammerNum >= 2 &&
			api.GetSelfInfo()->timeUntilCommonSkillAvailable <= 0 &&
			chk(getAngleToSelf(api.CellToGrid(zjd[api.GridToCell(self->x)][api.GridToCell(self->y)].first), api.CellToGrid(zjd[api.GridToCell(self->x)][api.GridToCell(self->y)].second)), self->x, self->y) >= 3) {
			api.UseCommonSkill();
			api.Attack(getAngleToSelf(api.CellToGrid(zjd[api.GridToCell(self->x)][api.GridToCell(self->y)].first), api.CellToGrid(zjd[api.GridToCell(self->x)][api.GridToCell(self->y)].second)));
			api.Attack(getAngleToSelf(api.CellToGrid(zjd[api.GridToCell(self->x)][api.GridToCell(self->y)].first), api.CellToGrid(zjd[api.GridToCell(self->x)][api.GridToCell(self->y)].second)));
		}
		for (auto u : R) {
			if (!u->isResetting && u->teamID != self->teamID) {
				if (disToSelf(u->x, u->y) <= self->attackRange + max(jammer_range[self->signalJammerType] - 1000, 0) - 700 - 1500 * Cross[api.GridToCell(u->x)][api.GridToCell(u->y)][api.GridToCell(self->x)][api.GridToCell(self->y)]) {
					if (self->prop == THUAI5::PropType::ShieldBreaker)
						api.UseProp();
					if (api.GetSelfInfo()->signalJammerNum >= 2) {
						if (playerSoftware == THUAI5::SoftwareType::PowerEmission || playerSoftware == THUAI5::SoftwareType::Amplification)
							if (chk(getAngleToSelf(u->x, u->y) - 0.4, self->x, self->y) + chk(getAngleToSelf(u->x, u->y) + 0.4, self->x, self->y) >= 3)
								api.UseCommonSkill();
						api.Attack(getAngleToSelf(u->x, u->y) - 0.4);
						api.Attack(getAngleToSelf(u->x, u->y) + 0.4);
					}
					//用于在核弹好的时候留一下子弹
					else if ((playerSoftware != THUAI5::SoftwareType::Amplification || api.GetSelfInfo()->timeUntilCommonSkillAvailable > 6000) && api.GetSelfInfo()->signalJammerNum > 0) {
						api.Attack(getAngleToSelf(u->x, u->y));
					}
				}
			}
		}
	}

	long long t1 = getTime();
	//在 9:00 - 9:30 必须死守出生点，不能被逼到外面去导致还剩十几秒的时候死了，捡不到CPU
	if (t1 - tf > 9 * 60 * 1000 && GX == bx1 && GY == by1 && !final_moment) return;

	auto J = api.GetSignalJammers();
	static vector<pair<long long, std::shared_ptr<const THUAI5::SignalJammer> > >Jammers;
	static int L = 0, isthreatened = 0, readed = 0;
	if (!readed) {
		for (int i = 0; i < 50; i++)
			for (int j = 0; j < 50; j++) {
				mAp[i][j] = api.GetPlaceType(i, j);
				if (mAp[i][j] != THUAI5::PlaceType::Land &&
					mAp[i][j] != THUAI5::PlaceType::BlindZone1 &&
					mAp[i][j] != THUAI5::PlaceType::BlindZone2 &&
					mAp[i][j] != THUAI5::PlaceType::BlindZone3 &&
					mAp[i][j] != THUAI5::PlaceType::CPUFactory)
					blc[i][j]++;
			}
		readed = 1;
	}
	for (; L < Jammers.size() && abs(Jammers[L].first) < getTime();) {
		if (Jammers[L].first > 0)
			Recover(api, Jammers[L].second), isthreatened--;
		L++;
	}
	for (auto u : J) if (disToSelf(u->x, u->y) <= jammer_move_range[u->type] + jammer_range[u->type]
		+ jammer_boom_frame[u->type] * 0.005 * self->speed && u->parentTeamID != self->teamID) {
		bool flg = 0;
		for (int i = L; i < Jammers.size(); i++)
			if (Jammers[i].second->guid == u->guid) {
				flg = 1;
				break;
			}
		if (!flg) {
			Jammers.push_back(make_pair(getTime() + jammer_boom_time[u->type], u));
			if (disToSelf(u->x, u->y) <= self->attackRange + max(jammer_range[self->signalJammerType] - 1000, 0) - 700 - 1500 * Cross[api.GridToCell(u->x)][api.GridToCell(u->y)][api.GridToCell(self->x)][api.GridToCell(self->y)]) {
				if (api.GetSelfInfo()->signalJammerNum >= 3) {
					api.Attack(getAngleToSelf(u->x, u->y) - 0.4);
					api.Attack(getAngleToSelf(u->x, u->y));
					api.Attack(getAngleToSelf(u->x, u->y) + 0.4);
				}
				else
					if ((playerSoftware != THUAI5::SoftwareType::Amplification || api.GetSelfInfo()->timeUntilCommonSkillAvailable > 6000) && api.GetSelfInfo()->signalJammerNum > 0)
						api.Attack(getAngleToSelf(u->x, u->y));
			}
			isthreatened++;
			Cover(api, u);
		}
	}
	else {
		Jammers.push_back(make_pair(-(getTime() + jammer_boom_time[u->type]), u));
	}
	if (isthreatened || !NoEnemyNear(self->x, self->y)) {
		bool fg = 0;
		for (auto u : self->buff)
			if (u == THUAI5::BuffType::AddLIFE || u == THUAI5::BuffType::ShieldBuff) {
				fg = 1; break;
			}
		if (inLine[api.GridToCell(self->x)][api.GridToCell(self->y)]) {
			auto u = inlu[api.GridToCell(self->x)][api.GridToCell(self->y)];
			double C = cos(u->facingDirection), S = sin(u->facingDirection);
			double dis = -S * (u->x - self->x) + C * (u->y - self->y);
			if (abs(dis) <= 200 && !fg) {
				if (self->prop == THUAI5::PropType::Battery || self->prop == THUAI5::PropType::Shield || self->prop == THUAI5::PropType::Booster)
					api.UseProp();
				else if (self->life <= 3000) {
					//wait to fight back
				}
			}
			if (dis < 0) api.MovePlayer(600 * 1000.0 / self->speed, u->facingDirection + PI / 2);
			else api.MovePlayer(600 * 1000.0 / self->speed, u->facingDirection - PI / 2);
		}
		else {
			pair<int, int> u;
			if (inpLine[api.GridToCell(self->x)][api.GridToCell(self->y)])
				u = find_Safe_a(api, self, befacing[api.GridToCell(self->x)][api.GridToCell(self->y)]);
			else
				u = find_Safe(api, self);
			if (u.first == -1) {
				if (!fg) {
					if (self->prop == THUAI5::PropType::Battery || self->prop == THUAI5::PropType::Shield || self->prop == THUAI5::PropType::Booster)
						api.UseProp();
					else if (self->life <= 3000) {
						//wait to fight back
					}
				}
			}
			else {
				static pair<int, int> last_pos = make_pair(-1, -1);
				if (!fg)
					if (Mindis >= 5000 || make_pair(self->x, self->y) == last_pos) // 逃逸距离过于勉强 或 卡墙了
						if (self->prop == THUAI5::PropType::Battery || self->prop == THUAI5::PropType::Shield || self->prop == THUAI5::PropType::Booster)
							api.UseProp();
				last_pos = make_pair(self->x, self->y);
				Move(api, self, u);
			}
		}
	}
	//else {
	if (self->prop == THUAI5::PropType::Booster) api.UseProp();

	int sx = self->x / num_of_grid_per_cell, sy = self->y / num_of_grid_per_cell;
	node st = node(sx, sy), goal = st;
	Message goal_to_send;

	vector<Message>info;
	while (api.MessageAvailable()) {
		info.push_back(Decode(api.TryGetMessage()));
	}
	auto Check = [&info](Message ret) {
		if (ret.type == 2) return true;
		for (auto& I : info)
			if (I.type == 1 && I.pt == ret.pt && I.dis < ret.dis) return false;
		return true;
	};

	/*
	邻近 > 出生点时刻 > 扔CPU. >
	优先级排布：CPU,道具,敌方,草丛
	*/
	auto ManToSelf = [GX, GY](int x, int y) {return abs(x - GX) + abs(y - GY); };
	for (auto& prop : target)
		if (ManToSelf(prop.second.x, prop.second.y) <= 2) {
			goal = BFSPoint(st, prop.second);
			if (Check(goal_to_send = Message(goal, 1, Dis[goal.x][goal.y]))) goto Send;
		}
	if (BP == BP1 && !Accmulate_CPU) {
		goal = BFSPoint(st, node(bx1, by1));
		goal_to_send = Message(goal, 2, Dis[goal.x][goal.y]);
		goto Send;
	}
	if (self->cpuNum > 0) {
		goal = BFSThrow(st);
		goal_to_send = Message(goal, 2, Dis[goal.x][goal.y]);
		goto Send;
	}

	//进行全图路径搜索
	BFSPath(st);

	//将敌方人员加入target. 敌方的优先级为 4，距离大于2500码时追击，最后30s敌方优先级最靠前，CPU越多越靠前
	for (auto& u : R) if (!u->isResetting && u->teamID != self->teamID && disToSelf(u->x, u->y) > 2500) {
		target.push_back(make_pair(!final_moment ? 4 : 0 - u->cpuNum, node(api.GridToCell(u->x), api.GridToCell(u->y))));
	}
	//草丛加入target.
	for (int i = 1; i <= 4; i++) target.push_back(make_pair(6, blind[i]));

	sort(target.begin(), target.end());//按优先级排序，优先级相同按距离排序

	for (auto& loc : target) {//找优先级最靠前的与队友Check过的目标，敌人需要特判共目标
		goal = loc.second;
		if (Check(goal_to_send = Message(goal, loc.first == 4 || loc.first <= 0 ? 2 : 1, Dis[goal.x][goal.y])))
			goto Send;
	}

Send:
	//移动
	node now = goal;
	if (now != st) {
		if (!OK[now.x][now.y]) {//只有敌方在出生点的情况，此时已进行过全图搜索
			for (int i = -1; i <= 1; i++)
				for (int j = -1; j <= 1; j++)
					if (OK[now.x + i][now.y + j]) {
						now.x += i, now.y += j;
						goto final_move;
					}
		}
	final_move:
		bool Around = 0; //判断周围8格是否有墙，有墙就看是否需要调整到中心点。
		for (int i = -1; i <= 1 && !Around; i++)
			for (int j = -1; j <= 1; j++)
				if (!OK[sx + i][sy + j]) {
					Around = 1;
					break;
				}
		while (!Direct(st, now)) now = pre[now.x][now.y];
		//printf("%d %d\n",now.x,now.y);
		if (Around) {
			int mx = self->x % 1000, my = self->y % 1000;
			if (now.x < st.x) {
				int xx = api.GridToCell(self->x - 1000);
				if (my < 500) {
					int yy = api.GridToCell(self->y - 1000);
					if (!OK[xx][yy]) api.MoveRight((500 - my) * 1000.0 / self->speed + 0.9);
				}
				else if (my > 500) {
					int yy = api.GridToCell(self->y + 1000);
					if (!OK[xx][yy]) api.MoveLeft((my - 500) * 1000.0 / self->speed + 0.9);
				}
				//api.MoveUp(200);
			}
			if (now.x > st.x) {
				int xx = api.GridToCell(self->x + 1000);
				if (my < 500) {
					int yy = api.GridToCell(self->y - 1000);
					if (!OK[xx][yy]) api.MoveRight((500 - my) * 1000.0 / self->speed + 0.9);
				}
				else if (my > 500) {
					int yy = api.GridToCell(self->y + 1000);
					if (!OK[xx][yy]) api.MoveLeft((my - 500) * 1000.0 / self->speed + 0.9);
				}
				//api.MoveDown(200);
			}
			if (now.y < st.y) {
				int yy = api.GridToCell(self->y - 1000);
				if (mx < 500) {
					int xx = api.GridToCell(self->x - 1000);
					if (!OK[xx][yy]) api.MoveDown((500 - mx) * 1000.0 / self->speed + 0.9);
				}
				else if (mx > 500) {
					int xx = api.GridToCell(self->x + 1000);
					if (!OK[xx][yy]) api.MoveUp((mx - 500) * 1000.0 / self->speed + 0.9);
				}
				//api.MoveLeft(200);
			}
			if (now.y > st.y) {
				int yy = api.GridToCell(self->y + 1000);
				if (mx < 500) {
					int xx = api.GridToCell(self->x - 1000);
					if (!OK[xx][yy]) api.MoveDown((500 - mx) * 1000.0 / self->speed + 0.9);
				}
				else if (mx > 500) {
					int xx = api.GridToCell(self->x + 1000);
					if (!OK[xx][yy]) api.MoveUp((mx - 500) * 1000.0 / self->speed + 0.9);
				}
				//api.MoveRight(200);
			}
		}
		//if (now == st) puts("???!");
		api.MovePlayer(200, atan2(now.y - st.y, now.x - st.x));
	}
	//发送信息
	string ret = goal_to_send.Encode();
	for (int i = 0; i < 4; i++) if (i != self->playerID) {
		api.Send(i, ret);
	}
	//}
}