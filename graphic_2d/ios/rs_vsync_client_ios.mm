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

#include "rs_vsync_client_ios.h"

#include <chrono>
#include <Foundation/Foundation.h>
#include <QuartzCore/CADisplayLink.h>
#include <UIKit/UIKit.h>

#include "common/rs_common_def.h"
#include "platform/common/rs_log.h"


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
RSVsyncClientIOS::RSVsyncClientIOS():vsyncIOS_([[RSVsyncIOS alloc]init]){}
RSVsyncClientIOS::~RSVsyncClientIOS(){
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
    std::unique_lock lock(mutex_);
    [vsyncIOS_ setVsyncCallback:callback];
}

} // namespace Rosen
} // namespace OHOS

@implementation RSVsyncIOS {
    OHOS::Rosen::RSVsyncClient::VsyncCallback callback_;
    CADisplayLink* displayLink_;
}

- (instancetype)init{
    if (self = [super init]) {
        /* not from alloc init,need retain,or displayLink will release */
        displayLink_ = [[CADisplayLink displayLinkWithTarget:self selector:@selector(onDisplayLink:)] retain];
        displayLink_.paused = YES;
        [displayLink_ addToRunLoop:[NSRunLoop mainRunLoop]
                                            forMode:NSRunLoopCommonModes];
    }
    return self;
}
- (void)requestNextVsync {
    displayLink_.paused = NO;
}
- (void)setVsyncCallback:(OHOS::Rosen::RSVsyncClient::VsyncCallback) callback {
    callback_ = callback;
}
- (void)onDisplayLink:(CADisplayLink*)link {
    displayLink_.paused = YES;
    int64_t now = std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count();
    callback_(now); 
}

- (void)invalidate {
    [displayLink_ invalidate];
}

- (void)dealloc {
    NSLog(@"RSVsyncIOS::dealloc");
    [displayLink_ release];
    [super dealloc];
}
@end
