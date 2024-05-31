/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "ios_message_loop.h"

#import <Foundation/Foundation.h>

#include "file_descriptor_listener.h"
#include "hilog.h"

namespace {
constexpr double NANOSECONDS_PER_ONE_SECOND = 1000000000;
constexpr CFTimeInterval TIME_INTERVAL = 1.0e10;
}

void IOSMessageLoop::KQCallback(CFFileDescriptorRef kqRef, CFOptionFlags flags, void *info)
{
    HILOG_INFO("KQCallback Start");
    struct kevent evr;
    struct timespec timeout = { 0, 0 };
    int32_t kq = CFFileDescriptorGetNativeDescriptor(kqRef);

    if(kevent(kq, NULL, 0, &evr, 1, &timeout) > 0) {
        if (evr.udata == nullptr) {
            HILOG_ERROR("ev.udata is nullptr");
            return;
        }

        int32_t eventTemp = 0;
        if (evr.filter == EVFILT_READ) {
            eventTemp = OHOS::AppExecFwk::FILE_DESCRIPTOR_INPUT_EVENT;
        }
        if (evr.filter == EVFILT_WRITE) {
            eventTemp = OHOS::AppExecFwk::FILE_DESCRIPTOR_OUTPUT_EVENT;
        }

        auto loop = reinterpret_cast<IOSMessageLoop*>(evr.udata);
        if (loop != nullptr) {
            auto observer = loop->observer_.lock();
            if (observer) {
                observer->OnFileDescriptorEvent(static_cast<int32_t>(evr.ident), eventTemp);
            }
        }
    }

    CFFileDescriptorEnableCallBacks(kqRef, kCFFileDescriptorReadCallBack);
}

void IOSMessageLoop::OnTimerFire(CFRunLoopTimerRef timer, IOSMessageLoop* loop)
{
    @autoreleasepool {
        // RunExpiredTasksNow rearms the timer as appropriate via a call to WakeUp.
        if (loop != nullptr) {
            auto observer = loop->observer_.lock();
            if (observer) {
                observer->OnTimerEvent();
            }
        }
    }
}

void IOSMessageLoop::AttachEventObserver(const std::weak_ptr<IEventObserver>& observer)
{
    observer_ = observer;
}

bool IOSMessageLoop::AddFileDescriptor(int32_t fd, uint32_t events)
{
    CFFileDescriptorContext context = {
        .version = 0,
        .info = this,
        .retain = NULL,
        .release = NULL,
        .copyDescription = NULL
    };

    int32_t kq = kqueue();
    if (kq < 0) {
        HILOG_ERROR("IOSMessageLoop::AddFileDescriptor kq create error");
        return false;
    }

    struct kevent ev[2];
    if ((events &  OHOS::AppExecFwk::FILE_DESCRIPTOR_INPUT_EVENT) != 0) {
        EV_SET(&ev[0], fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, (void*)this); 
    }
    if ((events &  OHOS::AppExecFwk::FILE_DESCRIPTOR_OUTPUT_EVENT) != 0) {
        EV_SET(&ev[1], fd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, (void*)this);
    }

    kevent(kq, ev, 2, NULL, 0, NULL);

    kqref_ = CFFileDescriptorCreate(NULL, kq, true, IOSMessageLoop::KQCallback, &context);

    if (kqref_ == NULL) {
        HILOG_ERROR("kqref_ is nullptr");
        close(kq);
        return false;
    }

    rls_ = CFFileDescriptorCreateRunLoopSource(NULL, kqref_, 0);

    CFRunLoopAddSource(CFRunLoopGetCurrent(), rls_, kCFRunLoopDefaultMode);

    CFFileDescriptorEnableCallBacks(kqref_, kCFFileDescriptorReadCallBack);
    return true;
}

void IOSMessageLoop::Invalidate(int32_t fd)
{
    HILOG_INFO("IOSMessageLoop::Invalidate");
    CFFileDescriptorInvalidate(kqref_);
    CFRunLoopRemoveSource(CFRunLoopGetCurrent(), rls_, kCFRunLoopDefaultMode);
    CFFileDescriptorDisableCallBacks(kqref_, kCFFileDescriptorReadCallBack);
}

bool IOSMessageLoop::AddTimer()
{
    CFRunLoopTimerContext timerContext = {
        .info = this,
    };

    delayed_wake_timer_ = CFRunLoopTimerCreate(kCFAllocatorDefault,
                                               TIME_INTERVAL,
                                               HUGE_VAL,
                                               0,
                                               0,
                                               reinterpret_cast<CFRunLoopTimerCallBack>(&OnTimerFire),
                                               &timerContext);

    if (delayed_wake_timer_ == nullptr) {
        return false;
    }

    CFRunLoopAddTimer(CFRunLoopGetCurrent(), delayed_wake_timer_, kCFRunLoopCommonModes);
    return true;
}

void IOSMessageLoop::WakeUp(int64_t time_point)
{
    // Rearm the timer. The time bases used by CoreFoundation and FXL are
    // different and must be accounted for.
    double targetSeconds = time_point / NANOSECONDS_PER_ONE_SECOND;
    CFRunLoopTimerSetNextFireDate(delayed_wake_timer_, CFAbsoluteTimeGetCurrent() + targetSeconds);
}

void IOSMessageLoop::Terminate()
{
    HILOG_INFO("IOSMessageLoop::Terminate");
    CFRunLoopStop(CFRunLoopGetCurrent());
    CFRunLoopTimerInvalidate(delayed_wake_timer_);
    CFRunLoopRemoveTimer(CFRunLoopGetCurrent(), delayed_wake_timer_, kCFRunLoopCommonModes);
}

bool IOSMessageLoop::IsMainThread()
{
    return [NSThread isMainThread];
}