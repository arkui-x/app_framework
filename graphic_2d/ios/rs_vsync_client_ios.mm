/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#include "rs_vsync_client_ios.h"

#include <chrono>
#include <Foundation/Foundation.h>
#include <QuartzCore/CADisplayLink.h>
#include <UIKit/UIKit.h>

#include "common/rs_common_def.h"
#include "platform/common/rs_log.h"

/* the coefficient of converting seconds to nanoseconds */
constexpr int64_t SEC_TO_NANOSEC = 1000000000;
constexpr float FRAME_RATE = 60;
@interface RSVsyncIOS : NSObject

- (instancetype)init;
- (void)requestNextVsync;
- (void)setVsyncCallback:(OHOS::Rosen::RSVsyncClient::VsyncCallback) callback;
- (void)invalidate;

@end
namespace OHOS {
namespace Rosen {
std::unique_ptr<RSVsyncClient> RSVsyncClient::Create()
{
    return std::make_unique<RSVsyncClientIOS>();
}

RSVsyncClientIOS::RSVsyncClientIOS()
    : vsyncIOS_([[RSVsyncIOS alloc]init])
{
}

RSVsyncClientIOS::~RSVsyncClientIOS()
{
    NSLog(@"RSVsyncClientIOS::dealloc");
    [vsyncIOS_ invalidate];
    [vsyncIOS_ release];
}

void RSVsyncClientIOS::RequestNextVsync()
{
    [vsyncIOS_ requestNextVsync];
}

void RSVsyncClientIOS::SetVsyncCallback(VsyncCallback callback)
{
    [vsyncIOS_ setVsyncCallback:callback];
}
} // namespace Rosen
} // namespace OHOS

@implementation RSVsyncIOS {
    OHOS::Rosen::RSVsyncClient::VsyncCallback callback_;
    NSLock *lock_;
    CADisplayLink* displayLink_;
}

- (instancetype)init {
    if (self = [super init]) {
        /* not from alloc init, need retain, or displayLink will release */
        displayLink_ = [[CADisplayLink displayLinkWithTarget:self selector:@selector(onDisplayLink:)] retain];
        displayLink_.paused = YES;
        [displayLink_ addToRunLoop:[NSRunLoop mainRunLoop] forMode:NSRunLoopCommonModes];
        float mainMaxFrameRate = [UIScreen mainScreen].maximumFramesPerSecond;
        if (@available(iOS 15.0,*)) {
            float maxFrameRate = fmax(mainMaxFrameRate, FRAME_RATE);
            NSLog(@"RSVsyncIOS::maxFrameRate = %f",maxFrameRate);
            displayLink_.preferredFrameRateRange = CAFrameRateRangeMake(maxFrameRate, maxFrameRate, maxFrameRate);
        } else {
            displayLink_.preferredFramesPerSecond = mainMaxFrameRate;
        }
        lock_ = [[NSLock alloc] init];
    }
    return self;
}

- (void)requestNextVsync {
    displayLink_.paused = NO;
}

- (void)setVsyncCallback:(OHOS::Rosen::RSVsyncClient::VsyncCallback) callback {
    [lock_ lock];
    callback_ = callback;
    [lock_ unlock];
}

- (void)onDisplayLink:(CADisplayLink*)link {
    displayLink_.paused = YES;
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    int64_t now = ts.tv_sec * SEC_TO_NANOSEC + ts.tv_nsec;
    [lock_ lock];
    callback_(now, 0);
    [lock_ unlock];
}

- (void)invalidate {
    [displayLink_ invalidate];
}

- (void)dealloc {
    NSLog(@"RSVsyncIOS::dealloc");
    [displayLink_ release];
    [lock_ release];
    [super dealloc];
}
@end
