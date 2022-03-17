#pragma once

/**
 * @brief noncopyable类被继承以后，派生类对象可以正常地构造与析构，但是派生类对象不可
 *        进行拷贝构造与赋值操作
 */
class noncopyable {
 public:
  noncopyable(const noncopyable& lhs) = delete;
  noncopyable& operator = (const noncopyable& lhs) = delete;

 protected:
  noncopyable() = default;
  ~noncopyable() = default;

};