#ifndef MUDUO_UTILS_CURRENT_THREAD_H
#define MUDUO_UTILS_CURRENT_THREAD_H


namespace current_thread
{

extern __thread int t_cached_tid;

void cachedTid();

inline int tid() {
    if (__builtin_expect(t_cached_tid == 0, 0)) {
        cachedTid();
    }
    return t_cached_tid;
}

}

#endif