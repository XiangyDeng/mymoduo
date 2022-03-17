#pragma once

#include<iostream>
#include<string>

class Timestamp {
 public:
  Timestamp();
  explicit Timestamp(int64_t microSecondsSinceEpoch); // 带参数构造函数最好使用explicit禁止隐式转换
  static Timestamp now();
  std::string toString() const;

 private:
  int64_t microSecondsSinceEpoch_; 

};