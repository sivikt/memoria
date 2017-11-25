
// Copyright 2017 Victor Smirnov
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.


#include <memoria/v1/reactor/reactor.hpp>


#include "linux_timer.hpp"

#include <sys/timerfd.h>


namespace memoria {
namespace v1 {
namespace reactor {


TimerImpl::TimerImpl(Timer::TimeUnit start_after, Timer::TimeUnit repeat_after, uint64_t count, TimerFn timer_fn):
    timer_fd_(timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK)),
    epoll_message_(engine().cpu(), timer_fd_, timer_fn),
    count_(count)
{
    if (timer_fd_ >= 0)
    {
        itimerspec ts = tools::make_zeroed<itimerspec>();

        ts.it_interval.tv_sec   = repeat_after.count() / 1000;
        ts.it_interval.tv_nsec  = (repeat_after.count() % 1000) * 1000000;

        ts.it_value.tv_sec  = start_after.count() / 1000;
        ts.it_value.tv_nsec = (start_after.count() % 1000) * 1000000;

        if (timerfd_settime(timer_fd_, 0, &ts, nullptr) >= 0)
        {
            epoll_event event = tools::make_zeroed<epoll_event>();
            event.events = EPOLLIN ;
            event.data.ptr = &epoll_message_;

            if (epoll_ctl(engine().io_poller().epoll_fd(), EPOLL_CTL_ADD, timer_fd_, &event) < 0)
            {
                ::close(timer_fd_);
                tools::rise_perror(SBuf() << "Can't configure epoller for Timer " << timer_fd_);
            }
        }
        else {
            ::close(timer_fd_);
            tools::rise_perror(SBuf() << "Can't setup timer " << timer_fd_);
        }
    }
    else {
        tools::rise_perror(SBuf() << "Can't create timerfd timer");
    }
}

TimerImpl::~TimerImpl() noexcept
{
    cancel();
}

void TimerImpl::cancel()
{
    if (!stopped_)
    {
        stopped_ = true;
        if (epoll_ctl(engine().io_poller().epoll_fd(), EPOLL_CTL_DEL, timer_fd_, nullptr) >= 0)
        {
            engine().drain_pending_io_events(&epoll_message_);
        }
        else {
            tools::report_perror(SBuf() << "Can't unregister poller for Timer " << timer_fd_);
        }

        ::close(timer_fd_);
    }
}


void IOPoller::sleep_for(const std::chrono::milliseconds& time)
{
    if (time != std::chrono::milliseconds::zero())
    {
        int tfd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
        if (tfd >= 0)
        {
            itimerspec ts = tools::make_zeroed<itimerspec>();

            ts.it_value.tv_sec  = time.count() / 1000;
            ts.it_value.tv_nsec = (time.count() % 1000) * 1000000;

            if (timerfd_settime(tfd, 0, &ts, nullptr) >= 0)
            {
                SocketIOMessage message(cpu_);

                epoll_event event = tools::make_zeroed<epoll_event>();
                event.events = EPOLLIN | EPOLLONESHOT;
                event.data.ptr = &message;

                if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, tfd, &event) >= 0)
                {
                    message.wait_for();

                    epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, tfd, nullptr);

                    close(tfd);
                }
                else {
                    close(tfd);
                    tools::rise_perror(SBuf() << "Can't configure epoller for engine.sleep_for()");
                }
            }
            else {
                ::close(tfd);
                tools::rise_perror(SBuf() << "Can't configure timer for engine.sleep_for()");
            }
        }
        else {
            tools::rise_perror(SBuf() << "Can't create timer for engine.sleep_for()");
        }
    }
}




}}}
