#include <iostream>

#include <queue>
#include <vector>
#include <algorithm>
#include <numeric>
#include <random>
#include <chrono>

using namespace std;

int main() {
  const int MAX_WINSIZE = 10000;
  queue<float> que_; 
  float sum_ = 0.0, slope_ = 0.0;
  float lastVal = -1.0;

  // 数据产生
  vector<float> vec(MAX_WINSIZE * 10);
  for (int i=0; i < vec.size(); ++i) {
    vec[i] = i + (float)(rand() % 100) / 100;
  }

  // 数据干扰 
  for(int i = 9980; i< 9999;++i)
    vec[i] += 1000;

  // 斜率计算 
  for (auto &curVal : vec) {
    float temp = curVal - lastVal;
    if (abs(temp) >= 100) temp *= 0.5;
    lastVal = curVal;
    que_.emplace(temp);
    sum_ += temp;

    if (que_.size() > MAX_WINSIZE) {
      sum_ -= que_.front();
      que_.pop();
      
      slope_ = sum_ / MAX_WINSIZE;
      cout << "slope_ is :" << slope_ << endl;
    }
  }

  return 0;
}