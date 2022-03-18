#include "CurrentThread.h"

#include <unistd.h>
#include <sys/syscall.h>

namespace CurrentThread {
    __thread int t_cachedTid;

    void cacheTid() {
      if (t_cachedTid == 0) {
        // 通过系统调用获取当前线程的tid
        t_cachedTid = static_cast<pid_t>(::syscall(SYS_gettid));
      }
    } 
}