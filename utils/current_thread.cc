#include "current_thread.h"
#include <unistd.h>
#include <sys/syscall.h>

namespace current_thread
{

__thread int t_cached_tid = 0;

void cachedTid() {
    if (t_cached_tid == 0) {
        t_cached_tid = static_cast<pid_t>(::syscall(SYS_gettid));
    }
}

}
