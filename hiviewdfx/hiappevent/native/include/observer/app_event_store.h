/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#ifndef FOUNDATION_APPFRAMEWORK_HIVIEWDFX_HIAPPEVENT_NATIVE_INCLUDE_OBSERVER_APP_EVENT_STORE_H
#define FOUNDATION_APPFRAMEWORK_HIVIEWDFX_HIAPPEVENT_NATIVE_INCLUDE_OBSERVER_APP_EVENT_STORE_H

#include <atomic>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "hiappevent_base.h"
#include "nocopyable.h"
#include "singleton.h"

namespace OHOS {
namespace HiviewDFX {
class AppEventPack;

struct Observer {
    std::string name;
    int64_t hash;
    Observer(const std::string& n, int64_t h) : name(n), hash(h) {}
};

typedef std::pair<int64_t, int64_t> EventObserverPair;

struct CustomEventParamExtended {
    std::string running_id;
    std::string domain;
    std::string name;
    std::string param_key;
    std::string param_value;
    int param_type;
    CustomEventParamExtended(const std::string &rid, const std::string &dom, const std::string &n,
                             const CustomEventParam &param)
        : running_id(rid), domain(dom), name(n), param_key(param.key),
          param_value(param.value), param_type(param.type) {}
};

class AppEventStore : public NoCopyable {
public:
    static AppEventStore& GetInstance();

    int64_t InsertEvent(std::shared_ptr<AppEventPack> event);
    int64_t InsertObserver(const std::string& observer, int64_t hashCode = 0);
    int64_t InsertEventMapping(int64_t eventSeq, int64_t observerSeq);
    int64_t InsertCustomEventParams(const std::shared_ptr<AppEventPack> event);
    int QueryEvents(std::vector<std::shared_ptr<AppEventPack>>& events, int64_t observerSeq, uint32_t eventSize = 0);
    int64_t QueryObserverSeq(const std::string& observer, int64_t hashCode = 0);
    int QueryObserverSeqs(const std::string& observer, int64_t& observerSeq, ObserverType type);
    int QueryCustomParamsAdd2EventPack(std::shared_ptr<AppEventPack> event);
    int DeleteObserver(int64_t observerSeq, ObserverType type);
    int DeleteEventMapping(int64_t observerSeq = 0, const std::vector<int64_t>& eventSeqs = {});
    int DeleteEvent(int64_t eventSeq = 0);
    int DeleteCustomEventParams();
    int DeleteEvent(const std::vector<int64_t>& eventSeqs);
    int DeleteUnusedParamsExceptCurId(const std::string& curRunningId);
    bool DeleteData(int64_t observerSeq, const std::vector<int64_t>& eventSeqs);
    void UpdateCustomEventParam(const std::string& runningId, const std::string& domain, const std::string& name,
        const CustomEventParam& param);
    std::unordered_set<std::string> FindOldParamkeys(
        const std::string& runningId, const std::string& domain, const std::string& name);
    void FindParams(std::unordered_map<std::string, std::string>& params, const std::string& runningId,
        const std::string& domain, const std::string& name);

private:
    AppEventStore() = default;
    ~AppEventStore() = default;

private:
    std::string dirPath_;

    std::map<int64_t, std::shared_ptr<Observer>> appEventObserverMap_;
    std::map<int64_t, std::shared_ptr<AppEventPack>> eventMap_;
    std::map<int64_t, EventObserverPair> appEventObserverMapping_;
    std::map<int64_t, std::shared_ptr<CustomEventParamExtended>> customParamMap_;
    std::map<int64_t, int64_t> seqToHashCodeMap_;

    std::mutex storeMutex_;
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // FOUNDATION_APPFRAMEWORK_HIVIEWDFX_HIAPPEVENT_NATIVE_INCLUDE_OBSERVER_APP_EVENT_STORE_H
