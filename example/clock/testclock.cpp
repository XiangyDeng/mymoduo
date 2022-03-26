#include <sys/timerfd.h>

#include <typeinfo>
#include <string>
#include <strings.h>
#include <mymuduo/TcpServer.h>
#include <mymuduo/Logger.h>
#include <mymuduo/Timestamp.h>

EventLoop *g_loop;

void onTimeOut (Timestamp timestamp) {
  std::string s = Timestamp::now().toString(); 
  std::cout << "time out at: " << s << std::endl;
  g_loop->quit();
}

// timerfd 触发event
int main() {
  EventLoop loop;
  g_loop = &loop;

  int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
  Channel channel(&loop, timerfd);
  channel.setReadCallback(onTimeOut);
  channel.enableReading();

  struct itimerspec howlong;
  bzero(&howlong, sizeof howlong);
  howlong.it_value.tv_sec = 1;
  ::timerfd_settime(timerfd, 0, &howlong, NULL);

  loop.loop();

  ::close(timerfd);

  return 0;
}