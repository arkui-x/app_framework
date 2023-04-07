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

#ifndef BASE_EVENTHANDLER_FRAMEWORKS_EVENTHANDLER_INCLUDE_IOS_MESSAGE_LOOP_H
#define BASE_EVENTHANDLER_FRAMEWORKS_EVENTHANDLER_INCLUDE_IOS_MESSAGE_LOOP_H

#import <fcntl.h>
#import <sys/event.h>

#include <CoreFoundation/CoreFoundation.h>
#include <functional>
#include <memory>
#include <string>

class IEventObserver {
public:
    virtual ~IEventObserver() {}
    virtual void OnFileDescriptorEvent(int32_t fd, int32_t event) = 0;
    virtual void OnTimerEvent() = 0;
};

class IOSMessageLoop {
public:
    IOSMessageLoop() = default;
    ~IOSMessageLoop() = default;

    void AttachEventObserver(const std::weak_ptr<IEventObserver>& observer);
    bool AddFileDescriptor(int32_t fd, uint32_t events);
    bool AddTimer();

    void Invalidate(int fd);
    void WakeUp(int64_t time_point);
    void Terminate();

    static bool IsMainThread();

private:
    static void KQCallback(CFFileDescriptorRef kqRef, CFOptionFlags flags, void* info);
    static void OnTimerFire(CFRunLoopTimerRef timer, IOSMessageLoop* loop);

private:
    CFFileDescriptorRef kqref_;
    CFRunLoopSourceRef rls_;
    CFRunLoopTimerRef delayed_wake_timer_;
    CFRunLoopRef loop_;
    std::atomic_bool running_;
    std::weak_ptr<IEventObserver> observer_;
};
#endif // BASE_EVENTHANDLER_FRAMEWORKS_EVENTHANDLER_INCLUDE_IOS_MESSAGE_LOOP_H
