#include "poller.h"
#include "event_loop.h"
#include "channel.h"
#include <stdlib.h>

using namespace muduo;

Poller::Poller(EventLoop* loop)
    : ownerloop_(loop) {}

// void Poller::assertInLoopThread() const {
//     ownerloop_->assertInLoopThread();
// }


bool Poller::hasChannel(Channel* channel) const {
    assertInLoopThread();
    auto it = channels_.find(channel->fd());
    return it != channels_.end() && it->second == channel; 
}
