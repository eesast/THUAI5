#include <iostream>
#include <algorithm>
#include <cmath>
#include <cassert>
using namespace std;

template <typename T>
using mypair = pair<T, T>;
double maxScore = 100;

double WinScore(double delta, double winnerGameScore)  // 根据游戏得分差值，与绝对分数，决定最后的加分
{
    assert(delta > 0);
    double deltaRate = 0.035;
    double scoreRate = 2e-5;
    // 赢者至少加 half of maxScore，平局只加 a quarter of maxScore
    // 差值引入非线性，差值越大非线性贡献的分数越多；绝对分数只用线性
    double score = maxScore / 2 + (deltaRate + log10(delta) / maxScore + winnerGameScore * scoreRate) * delta;
    
    cout << "delta contribution: " << deltaRate + log10(delta) / maxScore << endl;
    cout << "winnerGameScore contribution: " << scoreRate * winnerGameScore << endl;
    
    return score < maxScore ? score : maxScore; 
}

// orgScore 是天梯中两队的分数；competitionScore 是这次游戏两队的得分
mypair<double> cal(mypair<double> orgScore, mypair<double> competitionScore)
{

    // 调整顺序，让第一个元素成为获胜者，便于计算

    bool reverse = false;    // 记录是否需要调整

    if (competitionScore.first < competitionScore.second)
    {
        reverse = true;
    }
    else if (competitionScore.first == competitionScore.second)  // 平局，两边加quarter of maxScore
    {
        orgScore.first += maxScore / 4;
        orgScore.second += maxScore / 4;
        return orgScore;
    }

    if (reverse)   // 如果需要换，换两者的顺序
    {
        swap(competitionScore.first, competitionScore.second);
        swap(orgScore.first, orgScore.second);
    }

    double delta = competitionScore.first - competitionScore.second;
    double addScore = WinScore(delta, competitionScore.first);  
    auto resScore = mypair<double>(orgScore.first + addScore, orgScore.second);
    // 如果换过，再换回来
    if (reverse)
    {
        swap(resScore.first, resScore.second);
    }

    return resScore;
}

void Print(mypair<double> score)
{
    std::cout << " team1: " << score.first << std::endl
        << "team2: " << score.second << std::endl;
}

int main()
{
    int x, y;
    std::cout << "origin score of team 1 and 2: " << std::endl;
    std::cin >> x >> y;
    auto ori = mypair<double>(x, y);
    std::cout << "game score of team 1 and 2: " << std::endl;
    std::cin >> x >> y;
    auto sco = mypair<double>(x, y);
    Print(cal(ori, sco));
}