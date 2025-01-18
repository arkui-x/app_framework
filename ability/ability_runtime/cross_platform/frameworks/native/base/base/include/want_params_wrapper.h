/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
#ifndef OHOS_ABILITY_BASE_WANT_PARAMS_WRAPPER_H
#define OHOS_ABILITY_BASE_WANT_PARAMS_WRAPPER_H
#include "base_def.h"
#include "base_obj.h"
#include "nlohmann/json.hpp"
#include "want_params.h"
namespace OHOS {
namespace AAFwk {
class WantParams;
INTERFACE(IWantParams, a75b9db6 - 9813 - 4371 - 8848 - d2966ce6ec68)
{
    inline static IWantParams *Query(IInterface * object)
    {
        if (object == nullptr) {
            return nullptr;
        }
        return static_cast<IWantParams *>(object->Query(g_IID_IWantParams));
    }

    virtual ErrCode GetValue(WantParams & value) = 0;
};
class WantParamWrapper final : public Object, public IWantParams {
public:
    inline WantParamWrapper(const WantParams &value) : wantParams_(value)
    {}
    /**
     * @brief Constructor for move semantics
     */
    inline WantParamWrapper(WantParams &&value) : wantParams_(std::move(value))
    {}

    inline ~WantParamWrapper()
    {}

    IINTERFACE_DECL();

    ErrCode GetValue(WantParams &value) override;

    bool Equals(IObject &other) override;

    std::string ToString() override;

    static sptr<IWantParams> Box(const WantParams &value);
    /**
     * @brief Wrap a wantParam into a WantParamWrapper using move semantics and return the wrapper.
     *
     * @param value
     * @return sptr<IWantParams>
     */
    static sptr<IWantParams> Box(WantParams &&value);

    static WantParams Unbox(IWantParams *object);

    static bool ValidateStr(const std::string &str);

    static sptr<IWantParams> Parse(const std::string &str);

    static WantParams ParseWantParams(const std::string &str);

    static WantParams ParseWantParamsWithBrackets(const std::string &str);

    static sptr<IWantParams> Parse(const nlohmann::json& wantJson);

    static void ParseWantParams(
        const nlohmann::json& wantJson, OHOS::AAFwk::WantParams& wantParams);

    static constexpr char SIGNATURE = 'W';

private:
    WantParams wantParams_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_BASE_WANT_PARAMS_WRAPPER_H