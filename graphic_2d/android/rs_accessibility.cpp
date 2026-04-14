/*
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
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

#include "foundation/arkui/ace_engine/adapter/android/osal/high_contrast_observer.h"
#include "platform/common/rs_accessibility.h"

namespace OHOS {
namespace Rosen {
class RSAccessibilityAndroid : public RSAccessibility {
public:
    void ListenHighContrastChange(OnHighContrastChange callback) override;

private:
    void ListenHighContrast();
    void OnConfigChanged(bool newHighContrast);
    bool listeningHighContrast_ = false;
    Ace::Platform::ListenHighContrastCallback highContrastCallback_;
    OnHighContrastChange onHighContrastChange_ = nullptr;
};

RSAccessibility &RSAccessibility::GetInstance()
{
    static RSAccessibilityAndroid instance;
    return instance;
}

void RSAccessibilityAndroid::ListenHighContrastChange(OnHighContrastChange callback)
{
    onHighContrastChange_ = callback;
    ListenHighContrast();
}

void RSAccessibilityAndroid::ListenHighContrast()
{
    if (!listeningHighContrast_) {
        highContrastCallback_ = [](bool highContrastEnabled) {
            auto& accessibility = static_cast<RSAccessibilityAndroid&>(RSAccessibility::GetInstance());
            accessibility.OnConfigChanged(highContrastEnabled);
        };
        Ace::Platform::HighContrastObserver::GetInstance().SetListenHighContrastCallback(
            Ace::Platform::ListenHighContrastCallback(highContrastCallback_));
        listeningHighContrast_ = true;
    }
}

void RSAccessibilityAndroid::OnConfigChanged(bool newHighContrast)
{
    if (onHighContrastChange_ != nullptr) {
        onHighContrastChange_(newHighContrast);
    }
}
} // namespace Rosen
} // namespace OHOS
