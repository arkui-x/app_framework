/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "want_params.h"
#ifndef WANT_PARAM_USE_LONG
#define WANT_PARAM_USE_LONG
#endif

#include <unistd.h>

#include "array_wrapper.h"
#include "base_interfaces.h"
#include "base_obj.h"
#include "bool_wrapper.h"
#include "byte_wrapper.h"
#include "double_wrapper.h"
#include "float_wrapper.h"
#include "int_wrapper.h"
#include "long_wrapper.h"
#include "securec.h"
#include "short_wrapper.h"
#include "string_wrapper.h"
#include "want_params_wrapper.h"
#include "zchar_wrapper.h"

namespace OHOS {
namespace AAFwk {
const char* FD = "FD";
const char* REMOTE_OBJECT = "RemoteObject";
const char* TYPE_PROPERTY = "type";
const char* VALUE_PROPERTY = "value";
constexpr int32_t MAX_RECURSION_DEPTH = 100;
const std::string JSON_WANTPARAMS_KEY = "key";
const std::string JSON_WANTPARAMS_VALUE = "value";
const std::string JSON_WANTPARAMS_TYPE = "type";
const std::string JSON_WANTPARAMS_PARAM = "params";

std::string WantParams::GetStringByType(const sptr<IInterface> iIt, int typeId)
{
    if (typeId == VALUE_TYPE_BOOLEAN) {
        return static_cast<Boolean *>(IBoolean::Query(iIt))->ToString();
    } else if (typeId == VALUE_TYPE_BYTE) {
        return static_cast<Byte *>(IByte::Query(iIt))->ToString();
    } else if (typeId == VALUE_TYPE_CHAR) {
        return static_cast<Char *>(IChar::Query(iIt))->ToString();
    } else if (typeId == VALUE_TYPE_SHORT) {
        return static_cast<Short *>(IShort::Query(iIt))->ToString();
    } else if (typeId == VALUE_TYPE_INT) {
        return static_cast<Integer *>(IInteger::Query(iIt))->ToString();
    } else if (typeId == VALUE_TYPE_LONG) {
        return static_cast<Long *>(ILong::Query(iIt))->ToString();
    } else if (typeId == VALUE_TYPE_FLOAT) {
        return static_cast<Float *>(IFloat::Query(iIt))->ToString();
    } else if (typeId == VALUE_TYPE_DOUBLE) {
        return static_cast<Double *>(IDouble::Query(iIt))->ToString();
    } else if (typeId == VALUE_TYPE_STRING) {
        return static_cast<String *>(IString::Query(iIt))->ToString();
    } else if (typeId == VALUE_TYPE_ARRAY) {
        return static_cast<Array *>(IArray::Query(iIt))->ToString();
    } else if (typeId == VALUE_TYPE_WANTPARAMS) {
        return static_cast<WantParamWrapper *>(IWantParams::Query(iIt))->ToString();
    } else {
        return "";
    }
    return "";
}
template<typename T1, typename T2, typename T3>
static void SetNewArray(const AAFwk::InterfaceID &id, AAFwk::IArray *orgIArray, sptr<AAFwk::IArray> &ao);
/**
 * @description: A constructor used to create an WantParams instance by using the parameters of an existing
 * WantParams object.
 * @param wantParams  Indicates the existing WantParams object.
 */
WantParams::WantParams(const WantParams &wantParams)
{
    params_.clear();
    NewParams(wantParams, *this);
}

WantParams::WantParams(WantParams &&other) noexcept
{
    *this = std::move(other);
}

// inner use function
bool WantParams::NewFds(const WantParams &source, WantParams &dest)
{
    // Deep copy
    for (auto it : source.fds_) {
        dest.fds_[it.first] = it.second;
    }
    return true;
}  // namespace AAFwk

// inner use function
bool WantParams::NewParams(const WantParams &source, WantParams &dest)
{
    // Deep copy
    for (auto it = source.params_.begin(); it != source.params_.end(); it++) {
        sptr<IInterface> o = it->second;
        if (IString::Query(o) != nullptr) {
            dest.params_[it->first] = String::Box(String::Unbox(IString::Query(o)));
        } else if (IBoolean::Query(o) != nullptr) {
            dest.params_[it->first] = Boolean::Box(Boolean::Unbox(IBoolean::Query(o)));
        } else if (IByte::Query(o) != nullptr) {
            dest.params_[it->first] = Byte::Box(Byte::Unbox(IByte::Query(o)));
        } else if (IChar::Query(o) != nullptr) {
            dest.params_[it->first] = Char::Box(Char::Unbox(IChar::Query(o)));
        } else if (IShort::Query(o) != nullptr) {
            dest.params_[it->first] = Short::Box(Short::Unbox(IShort::Query(o)));
        } else if (IInteger::Query(o) != nullptr) {
            dest.params_[it->first] = Integer::Box(Integer::Unbox(IInteger::Query(o)));
        } else if (ILong::Query(o) != nullptr) {
            dest.params_[it->first] = Long::Box(Long::Unbox(ILong::Query(o)));
        } else if (IFloat::Query(o) != nullptr) {
            dest.params_[it->first] = Float::Box(Float::Unbox(IFloat::Query(o)));
        } else if (IDouble::Query(o) != nullptr) {
            dest.params_[it->first] = Double::Box(Double::Unbox(IDouble::Query(o)));
        } else if (IWantParams::Query(o) != nullptr) {
            dest.params_[it->first] = WantParamWrapper::Box(WantParamWrapper::Unbox(IWantParams::Query(o)));
        } else if (IArray::Query(o) != nullptr) {
            sptr<IArray> destAO = nullptr;
            if (!NewArrayData(IArray::Query(o), destAO)) {
                continue;
            }
            dest.params_[it->first] = destAO;
        }
    }
    return true;
}  // namespace AAFwk
// inner use
bool WantParams::NewArrayData(IArray *source, sptr<IArray> &dest)
{
    if (Array::IsBooleanArray(source)) {
        SetNewArray<bool, AAFwk::Boolean, AAFwk::IBoolean>(AAFwk::g_IID_IBoolean, source, dest);
    } else if (Array::IsCharArray(source)) {
        SetNewArray<char, AAFwk::Char, AAFwk::IChar>(AAFwk::g_IID_IChar, source, dest);
    } else if (Array::IsByteArray(source)) {
        SetNewArray<byte, AAFwk::Byte, AAFwk::IByte>(AAFwk::g_IID_IByte, source, dest);
    } else if (Array::IsShortArray(source)) {
        SetNewArray<short, AAFwk::Short, AAFwk::IShort>(AAFwk::g_IID_IShort, source, dest);
    } else if (Array::IsIntegerArray(source)) {
        SetNewArray<int, AAFwk::Integer, AAFwk::IInteger>(AAFwk::g_IID_IInteger, source, dest);
    } else if (Array::IsLongArray(source)) {
        SetNewArray<long, AAFwk::Long, AAFwk::ILong>(AAFwk::g_IID_ILong, source, dest);
    } else if (Array::IsFloatArray(source)) {
        SetNewArray<float, AAFwk::Float, AAFwk::IFloat>(AAFwk::g_IID_IFloat, source, dest);
    } else if (Array::IsDoubleArray(source)) {
        SetNewArray<double, AAFwk::Double, AAFwk::IDouble>(AAFwk::g_IID_IDouble, source, dest);
    } else if (Array::IsStringArray(source)) {
        SetNewArray<std::string, AAFwk::String, AAFwk::IString>(AAFwk::g_IID_IString, source, dest);
    } else if (Array::IsWantParamsArray(source)) {
        SetNewArray<WantParams, AAFwk::WantParamWrapper, AAFwk::IWantParams>(AAFwk::g_IID_IWantParams, source, dest);
    } else {
        return false;
    }

    if (dest == nullptr) {
        return false;
    }

    return true;
}
/**
 * @description: A WantParams used to
 *
 * @param other  Indicates the existing WantParams object.
 */
WantParams &WantParams::operator=(const WantParams &other)
{
    if (this != &other) {
        params_.clear();
        fds_.clear();
        NewParams(other, *this);
        NewFds(other, *this);
    }
    return *this;
}

WantParams &WantParams::operator=(WantParams &&other) noexcept
{
    if (this != &other) {
        // free existing resources.
        params_.clear();
        params_ = other.params_;
        // free other resources.
        other.params_.clear();
        fds_.clear();
        fds_ = other.fds_;
        other.fds_.clear();
    }
    return *this;
}

bool WantParams::operator==(const WantParams &other)
{
    if (this->params_.size() != other.params_.size()) {
        return false;
    }
    for (auto itthis : this->params_) {
        auto itother = other.params_.find(itthis.first);
        if (itother == other.params_.end()) {
            return false;
        }
        int type1 = WantParams::GetDataType(itthis.second);
        int type2 = WantParams::GetDataType(itother->second);
        if (type1 != type2) {
            return false;
        }
        if (!CompareInterface(itother->second, itthis.second, type1)) {
            return false;
        }
    }
    return true;
}

int WantParams::GetDataType(const sptr<IInterface> iIt)
{
    if (iIt != nullptr && IBoolean::Query(iIt) != nullptr) {
        return VALUE_TYPE_BOOLEAN;
    } else if (iIt != nullptr && IByte::Query(iIt) != nullptr) {
        return VALUE_TYPE_BYTE;
    } else if (iIt != nullptr && IChar::Query(iIt) != nullptr) {
        return VALUE_TYPE_CHAR;
    } else if (iIt != nullptr && IShort::Query(iIt) != nullptr) {
        return VALUE_TYPE_SHORT;
    } else if (iIt != nullptr && IInteger::Query(iIt) != nullptr) {
        return VALUE_TYPE_INT;
    } else if (iIt != nullptr && ILong::Query(iIt) != nullptr) {
        return VALUE_TYPE_LONG;
    } else if (iIt != nullptr && IFloat::Query(iIt) != nullptr) {
        return VALUE_TYPE_FLOAT;
    } else if (iIt != nullptr && IDouble::Query(iIt) != nullptr) {
        return VALUE_TYPE_DOUBLE;
    } else if (iIt != nullptr && IString::Query(iIt) != nullptr) {
        return VALUE_TYPE_STRING;
    } else if (iIt != nullptr && IArray::Query(iIt) != nullptr) {
        return VALUE_TYPE_ARRAY;
    } else if (iIt != nullptr && IWantParams::Query(iIt) != nullptr) {
        return VALUE_TYPE_WANTPARAMS;
    }

    return VALUE_TYPE_NULL;
}

sptr<IInterface> WantParams::GetInterfaceByType(int typeId, const std::string &value)
{
    if (typeId == VALUE_TYPE_BOOLEAN) {
        return Boolean::Parse(value);
    } else if (typeId == VALUE_TYPE_BYTE) {
        return Byte::Parse(value);
    } else if (typeId == VALUE_TYPE_CHAR) {
        return Char::Parse(value);
    } else if (typeId == VALUE_TYPE_SHORT) {
        return Short::Parse(value);
    } else if (typeId == VALUE_TYPE_INT) {
        return Integer::Parse(value);
    } else if (typeId == VALUE_TYPE_LONG) {
        return Long::Parse(value);
    } else if (typeId == VALUE_TYPE_FLOAT) {
        return Float::Parse(value);
    } else if (typeId == VALUE_TYPE_DOUBLE) {
        return Double::Parse(value);
    } else if (typeId == VALUE_TYPE_STRING) {
        return String::Parse(value);
    } else if (typeId == VALUE_TYPE_ARRAY) {
        return Array::Parse(value);
    }

    return nullptr;
}

bool WantParams::CompareInterface(const sptr<IInterface> iIt1, const sptr<IInterface> iIt2, int typeId)
{
    bool flag = true;
    switch (typeId) {
        case VALUE_TYPE_BOOLEAN:
            flag =
                static_cast<Boolean *>(IBoolean::Query(iIt1))->Equals(*(static_cast<Boolean *>(IBoolean::Query(iIt2))));
            break;
        case VALUE_TYPE_BYTE:
            flag = static_cast<Byte *>(IByte::Query(iIt1))->Equals(*(static_cast<Byte *>(IByte::Query(iIt2))));
            break;
        case VALUE_TYPE_CHAR:
            flag = static_cast<Char *>(IChar::Query(iIt1))->Equals(*(static_cast<Char *>(IChar::Query(iIt2))));
            break;
        case VALUE_TYPE_SHORT:
            flag = static_cast<Short *>(IShort::Query(iIt1))->Equals(*(static_cast<Short *>(IShort::Query(iIt2))));
            break;
        case VALUE_TYPE_INT:
            flag =
                static_cast<Integer *>(IInteger::Query(iIt1))->Equals(*(static_cast<Integer *>(IInteger::Query(iIt2))));
            break;
        case VALUE_TYPE_LONG:
            flag = static_cast<Long *>(ILong::Query(iIt1))->Equals(*(static_cast<Long *>(ILong::Query(iIt2))));
            break;
        case VALUE_TYPE_FLOAT:
            flag = static_cast<Float *>(IFloat::Query(iIt1))->Equals(*(static_cast<Float *>(IFloat::Query(iIt2))));
            break;
        case VALUE_TYPE_DOUBLE:
            flag = static_cast<Double *>(IDouble::Query(iIt1))->Equals(*(static_cast<Double *>(IDouble::Query(iIt2))));
            break;
        case VALUE_TYPE_STRING:
            flag = static_cast<String *>(IString::Query(iIt1))->Equals(*(static_cast<String *>(IString::Query(iIt2))));
            break;
        case VALUE_TYPE_ARRAY:
            flag = static_cast<Array *>(IArray::Query(iIt1))->Equals(*(static_cast<Array *>(IArray::Query(iIt2))));
            break;
        case VALUE_TYPE_WANTPARAMS:
            flag = static_cast<WantParamWrapper *>(IWantParams::Query(iIt1))->
                Equals(*(static_cast<WantParamWrapper *>(IWantParams::Query(iIt2))));
            break;
        default:
            break;
    }
    return flag;
}

/**
 * @description: Sets a parameter in key-value pair format.
 * @param key Indicates the key matching the parameter.
 */
void WantParams::SetParam(const std::string &key, IInterface *value)
{
    params_[key] = value;
}

/**
 * @description: Obtains the parameter value based on a given key.
 * @param key Indicates the key matching the parameter.
 * @return Returns the value matching the given key.
 */
sptr<IInterface> WantParams::GetParam(const std::string &key) const
{
    auto it = params_.find(key);
    if (it == params_.cend()) {
        return nullptr;
    }
    return it->second;
}

WantParams WantParams::GetWantParams(const std::string& key) const
{
    auto value = GetParam(key);
    IWantParams *wp = IWantParams::Query(value);
    if (wp != nullptr) {
        return WantParamWrapper::Unbox(wp);
    }
    return WantParams();
}

std::string WantParams::GetStringParam(const std::string& key) const
{
    auto value = GetParam(key);
    IString *ao = IString::Query(value);
    if (ao != nullptr) {
        return String::Unbox(ao);
    }
    return std::string();
}

int WantParams::GetIntParam(const std::string& key, const int defaultValue) const
{
    auto value = GetParam(key);
    IInteger *ao = IInteger::Query(value);
    if (ao != nullptr) {
        return Integer::Unbox(ao);
    }
    return defaultValue;
}

/**
 * @description: Obtains the parameter value based on a given key.
 * @param key Indicates the key matching the parameter.
 * @return Returns the value matching the given key.
 */

const std::map<std::string, sptr<IInterface>> &WantParams::GetParams() const
{
    return params_;
}

/**
 * @description: Obtains a set of the keys of all parameters.
 * @param
 * @return Returns a set of keys.
 */
const std::set<std::string> WantParams::KeySet() const
{
    std::set<std::string> keySet;
    keySet.clear();
    for (auto it : params_) {
        keySet.emplace(it.first);
    }

    return keySet;
}

/**
 * @description: Removes the parameter matching the given key.
 * @param key Indicates the key matching the parameter to be removed.
 */
void WantParams::Remove(const std::string &key)
{
    params_.erase(key);
}

/**
 * @description: Checks whether the Want contains the given key.
 * @param key Indicates the key to check.
 * @return Returns true if the Want contains the key; returns false otherwise.
 */
bool WantParams::HasParam(const std::string &key) const
{
    return (params_.count(key) > 0);
}

/**
 * @description: Obtains the number of parameters contained in this WantParams object.
 * @return Returns the number of parameters.
 */
int WantParams::Size() const
{
    return params_.size();
}

/**
 * @description: Checks whether this WantParams object contains no parameters.
 * @return Returns true if this object does not contain any parameters; returns false otherwise.
 */
bool WantParams::IsEmpty() const
{
    return (params_.size() == 0);
}

template<typename dataType, typename className>
static bool SetArray(const InterfaceID &id, const std::vector<dataType> &value, sptr<IArray> &ao)
{
    typename std::vector<dataType>::size_type size = value.size();
    ao = new (std::nothrow) Array(size, id);
    if (ao != nullptr) {
        for (typename std::vector<dataType>::size_type i = 0; i < size; i++) {
            ao->Set(i, className::Box(value[i]));
        }
        return true;
    }
    return false;
}

template<typename T1, typename T2, typename T3>
static void FillArray(IArray *ao, std::vector<T1> &array)
{
    auto func = [&](IInterface *object) {
        if (object != nullptr) {
            T3 *value = T3::Query(object);
            if (value != nullptr) {
                array.push_back(T2::Unbox(value));
            }
        }
    };
    Array::ForEach(ao, func);
}
// inner use template function
template<typename T1, typename T2, typename T3>
static void SetNewArray(const AAFwk::InterfaceID &id, AAFwk::IArray *orgIArray, sptr<AAFwk::IArray> &ao)
{
    if (orgIArray == nullptr) {
        return;
    }
    std::vector<T1> array;
    auto func = [&](IInterface *object) {
        if (object != nullptr) {
            T3 *value = T3::Query(object);
            if (value != nullptr) {
                array.push_back(T2::Unbox(value));
            }
        }
    };
    Array::ForEach(orgIArray, func);

    typename std::vector<T1>::size_type size = array.size();
    if (size > 0) {
        ao = new (std::nothrow) AAFwk::Array(size, id);
        if (ao != nullptr) {
            for (typename std::vector<T1>::size_type i = 0; i < size; i++) {
                ao->Set(i, T2::Box(array[i]));
            }
        }
    }
}

void WantParams::CloseAllFd()
{
    for (auto it : fds_) {
        if (it.second > 0) {
            close(it.second);
        }
        params_.erase(it.first);
    }
    fds_.clear();
}

void WantParams::RemoveAllFd()
{
    for (auto it : fds_) {
        params_.erase(it.first);
    }
    fds_.clear();
}

void WantParams::DupAllFd()
{
    for (auto it : fds_) {
        if (it.second > 0) {
            int dupFd = dup(it.second);
            if (dupFd > 0) {
                WantParams wp;
                params_.erase(it.first);
                wp.SetParam(TYPE_PROPERTY, String::Box(FD));
                wp.SetParam(VALUE_PROPERTY, Integer::Box(dupFd));
                sptr<AAFwk::IWantParams> pWantParams = AAFwk::WantParamWrapper::Box(wp);
                SetParam(it.first, pWantParams);
                fds_[it.first] = dupFd;
            }
        }
    }
}
}  // namespace AAFwk
}  // namespace OHOS
