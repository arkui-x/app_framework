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
#ifndef OHOS_UDMF_ADAPTER_WANT_PARAMS_H
#define OHOS_UDMF_ADAPTER_WANT_PARAMS_H

#include <map>
#include <set>
#include <vector>

#include "base_interfaces.h"
#include "refbase.h"
#include "want_params_interface.h"

namespace OHOS {
namespace AAFwk {
extern const char* FD;
extern const char* REMOTE_OBJECT;
extern const char* TYPE_PROPERTY;
extern const char* VALUE_PROPERTY;
extern const std::string JSON_WANTPARAMS_KEY;
extern const std::string JSON_WANTPARAMS_VALUE;
extern const std::string JSON_WANTPARAMS_TYPE;
extern const std::string JSON_WANTPARAMS_PARAM;

enum class WantValueType : int {
    VALUE_TYPE_BOOLEAN = 1,
    VALUE_TYPE_BYTE = 2,
    VALUE_TYPE_CHAR = 3,
    VALUE_TYPE_SHORT = 4,
    VALUE_TYPE_INT = 5,
    VALUE_TYPE_LONG = 6,
    VALUE_TYPE_LONGLONG = 7,
    VALUE_TYPE_FLOAT = 8,
    VALUE_TYPE_DOUBLE = 9,
    VALUE_TYPE_STRING = 10,
    VALUE_TYPE_BOOLEANARRAY = 11,
    VALUE_TYPE_BYTEARRAY = 12,
    VALUE_TYPE_CHARARRAY = 13,
    VALUE_TYPE_SHORTARRAY = 14,
    VALUE_TYPE_INTARRAY = 15,
    VALUE_TYPE_LONGARRAY = 16,
    VALUE_TYPE_FLOATARRAY = 17,
    VALUE_TYPE_DOUBLEARRAY = 18,
    VALUE_TYPE_STRINGARRAY = 19,
    VALUE_TYPE_WANTPARAMSARRAY = 24,
    VALUE_TYPE_WANTPARAMS = 101,
    VALUE_TYPE_ARRAY = 102,
};

enum ScreenMode : int32_t {
    IDLE_SCREEN_MODE = -1,
    JUMP_SCREEN_MODE = 0,
    EMBEDDED_FULL_SCREEN_MODE = 1,
    EMBEDDED_HALF_SCREEN_MODE = 2
};
constexpr const char* SCREEN_MODE_KEY = "ohos.extra.param.key.showMode";

class WantParams : public WantParamsInterface {
public:
    WantParams() = default;
    WantParams(const WantParams &wantParams);
    WantParams(WantParams &&other) noexcept;
    inline ~WantParams()
    {}
    WantParams &operator=(const WantParams &other);
    WantParams &operator=(WantParams &&other) noexcept;
    bool operator==(const WantParams &other);

    static sptr<IInterface> GetInterfaceByType(int typeId, const std::string &value);

    static bool CompareInterface(const sptr<IInterface> iIt1, const sptr<IInterface> iIt2, int typeId);

    static int GetDataType(const sptr<IInterface> iIt);

    static std::string GetStringByType(const sptr<IInterface> iIt, int typeId);

    void SetParam(const std::string &key, IInterface *value);

    sptr<IInterface> GetParam(const std::string &key) const;

    WantParams GetWantParams(const std::string& key) const;

    std::string GetStringParam(const std::string& key) const;

    int GetIntParam(const std::string& key, const int defaultValue) const;

    const std::map<std::string, sptr<IInterface>> &GetParams() const;

    const std::set<std::string> KeySet() const;

    void Remove(const std::string &key);

    bool HasParam(const std::string &key) const;

    int Size() const;

    bool IsEmpty() const;

    void CloseAllFd();

    void RemoveAllFd();

    void DupAllFd();
private:
    enum {
        VALUE_TYPE_NULL = -1,
        VALUE_TYPE_BOOLEAN = 1,
        VALUE_TYPE_BYTE = 2,
        VALUE_TYPE_CHAR = 3,
        VALUE_TYPE_SHORT = 4,
        VALUE_TYPE_INT = 5,
        VALUE_TYPE_LONG = 6,
        VALUE_TYPE_FLOAT = 7,
        VALUE_TYPE_DOUBLE = 8,
        VALUE_TYPE_STRING = 9,
        VALUE_TYPE_CHARSEQUENCE = 10,
        VALUE_TYPE_BOOLEANARRAY = 11,
        VALUE_TYPE_BYTEARRAY = 12,
        VALUE_TYPE_CHARARRAY = 13,
        VALUE_TYPE_SHORTARRAY = 14,
        VALUE_TYPE_INTARRAY = 15,
        VALUE_TYPE_LONGARRAY = 16,
        VALUE_TYPE_FLOATARRAY = 17,
        VALUE_TYPE_DOUBLEARRAY = 18,
        VALUE_TYPE_STRINGARRAY = 19,
        VALUE_TYPE_CHARSEQUENCEARRAY = 20,

        VALUE_TYPE_PARCELABLE = 21,
        VALUE_TYPE_PARCELABLEARRAY = 22,
        VALUE_TYPE_SERIALIZABLE = 23,
        VALUE_TYPE_WANTPARAMSARRAY = 24,
        VALUE_TYPE_LIST = 50,

        VALUE_TYPE_WANTPARAMS = 101,
        VALUE_TYPE_ARRAY = 102,
        VALUE_TYPE_FD = 103,
        VALUE_TYPE_REMOTE_OBJECT = 104,
        VALUE_TYPE_INVALID_FD = 105,
    };

    friend class WantParamWrapper;
    // inner use function
    bool NewArrayData(IArray *source, sptr<IArray> &dest);
    bool NewParams(const WantParams &source, WantParams &dest);
    bool NewFds(const WantParams &source, WantParams &dest);
    std::map<std::string, sptr<IInterface>> params_;
    std::map<std::string, int> fds_;
};
}  // namespace AAFwk
}  // namespace OHOS

#endif  // OHOS_ABILITY_BASE_WANT_PARAMS_H
