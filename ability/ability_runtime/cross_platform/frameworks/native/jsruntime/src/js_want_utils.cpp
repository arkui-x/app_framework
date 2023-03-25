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

#include "js_want_utils.h"

#include "hilog.h"

namespace OHOS {
namespace AbilityRuntime {
NativeValue* CreateJsWant(NativeEngine& engine, const AAFwk::Want& want)
{
    HILOG_INFO("CreateJsWant is called");
    NativeValue* objValue = engine.CreateObject();
    NativeObject* object = ConvertNativeValueTo<NativeObject>(objValue);
    object->SetProperty("moduleName", CreateJsValue(engine, want.GetModuleName()));
    object->SetProperty("bundleName", CreateJsValue(engine, want.GetBundleName()));
    object->SetProperty("abilityName", CreateJsValue(engine, want.GetAbilityName()));
    return objValue;
}

bool UnwrapJsWant(NativeEngine& engine, NativeValue* param, AAFwk::Want& want)
{
    HILOG_INFO("UnwrapWant is called");
    if (param->TypeOf() != NativeValueType::NATIVE_OBJECT) {
        HILOG_ERROR("param type mismatch!");
        return false;
    }

    NativeObject* objectParam = ConvertNativeValueTo<NativeObject>(param);
    std::string bundleName = "";
    if (objectParam->HasProperty("bundleName")) {
        NativeValue* jsBundleName = objectParam->GetProperty("bundleName");
        if (!ConvertFromJsValue(engine, jsBundleName, bundleName)) {
            HILOG_ERROR("UnwrapWant convert bundleName error!");
            return false;
        }
    }

    std::string moduleName = "";
    if (objectParam->HasProperty("moduleName")) {
        NativeValue* jsModuleName = objectParam->GetProperty("moduleName");
        if (!ConvertFromJsValue(engine, jsModuleName, moduleName)) {
            HILOG_ERROR("UnwrapWant convert moduleName error!");
            return false;
        }
    }

    std::string abilityName = "";
    if (objectParam->HasProperty("abilityName")) {
        NativeValue* jsAbilityName = objectParam->GetProperty("abilityName");
        if (!ConvertFromJsValue(engine, jsAbilityName, abilityName)) {
            HILOG_ERROR("UnwrapWant convert abilityName error!");
            return false;
        }
    }

    want.SetBundleName(bundleName);
    want.SetModuleName(moduleName);
    want.SetAbilityName(abilityName);
    return true;
}
} // namespace AbilityRuntime
} // namespace OHOS
