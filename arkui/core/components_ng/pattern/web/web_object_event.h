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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_WEB_CORS_WEB_OBJECT_EVENT_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_WEB_CORS_WEB_OBJECT_EVENT_H

#include <string>
#include <unordered_map>
#include <map>
#include <vector>

#include "base/json/json_util.h"
#include "base/log/log.h"
#include "base/memory/referenced.h"
#include "base/utils/macros.h"
#include "base/utils/noncopyable.h"
#include "base/utils/singleton.h"
#include "core/components/web/web_event.h"

namespace OHOS::Ace {
class WebResourceRequestObject : public Referenced {
public:
    virtual std::map<std::string, std::string> GetRequestHeader(void* object) = 0;
    virtual std::string GetRequestUrl(void* object) = 0;
    virtual std::string GetMethod(void* object) = 0;
    virtual bool IsRequestGesture(void* object) = 0;
    virtual bool IsMainFrame(void* object) = 0;
    virtual bool IsRedirect(void* object) = 0;
};

class WebScrollObject : public Referenced {
public:
    virtual float GetX(void* object) = 0;
    virtual float GetY(void* object) = 0;
    virtual float GetContentWidth(void* object) = 0;
    virtual float GetContentHeight(void* object) = 0;
    virtual float GetFrameWidth(void* object) = 0;
    virtual float GetFrameHeight(void* object) = 0;
};

class WebScaleChangeObject : public Referenced {
public:
    virtual float GetNewScale(void* object) = 0;
    virtual float GetOldScale(void* object) = 0;
};

class WebResourceResponseObject : public Referenced {
public:
    virtual std::map<std::string, std::string> GetResponseHeader(void* object) = 0;
    virtual std::string GetResponseData(void* object) = 0;
    virtual std::string GetEncoding(void* object) = 0;
    virtual std::string GetMimeType(void* object) = 0;
    virtual std::string GetReason(void* object) = 0;
    virtual int GetStatusCode(void* object) = 0;
};

class WebConsoleMessageObject : public Referenced {
public:
    virtual std::string GetMessage(void* object) = 0;
    virtual int GetMessageLevel(void* object) = 0;
    virtual std::string GetSourceId(void* object) = 0;
    virtual int GetLineNumber(void* object) = 0;
};

class WebCommonDialogObject : public Referenced {
public:
    virtual std::string GetUrl(void* object) = 0;
    virtual std::string GetMessage(void* object) = 0;
    virtual std::string GetValue(void* object) = 0;
    virtual void Confirm(void* object, const std::string& promptResult, int index) {};
    virtual void Confirm(void* object, int index) {};
    virtual void Cancel(void* object, int index) {};
    virtual int AddObject(void* object)
    {
        return 0;
    };
    virtual void DelObject(int index) {};
};

class WebPermissionRequestObject : public Referenced {
public:
    virtual std::string GetOrigin(void* object) = 0;
    virtual int GetResourcesId(void* object) = 0;
    virtual void Grant(void* object, const int resourcesId, int index) {};
    virtual void Deny(void* object, int index) {};
    virtual int AddObject(void* object)
    {
        return 0;
    };
    virtual void DelObject(int index) {};
};

class WebHttpAuthRequestObject : public Referenced {
public:
    virtual std::string GetHost(void* object) = 0;
    virtual std::string GetRealm(void* object) = 0;
    virtual int AddObject(void* object)
    {
        return 0;
    };
    virtual void DelObject(int index) {};
    virtual bool Confirm(void* object, std::string& userName, std::string& pwd, int index)
    {
        return false;
    };
    virtual bool IsHttpAuthInfoSaved(void* object, int index)
    {
        return false;
    };
    virtual void Cancel(void* object, int index) {};
};

class WebDownloadResponseObject : public Referenced {
public:
    virtual std::string GetUrl(void* object) = 0;
    virtual std::string GetMimetype(void* object) = 0;
    virtual long GetContentLength(void* object) = 0;
    virtual std::string GetContentDisposition(void* object) = 0;
    virtual std::string GetUserAgent(void* object) = 0;
};

class WebRefreshAccessedHistoryObject : public Referenced {
public:
    virtual std::string GetUrl(void* object) = 0;
    virtual bool GetIsRefreshed(void* object) = 0;
};

class WebFullScreenEnterObject : public Referenced {
public:
    virtual int GetWidths(void* object) = 0;
    virtual int GetHeights(void* object) = 0;
    virtual void ExitFullScreen(void* object, int index) {};
    virtual int AddObject(void* object)
    {
        return 0;
    };
    virtual void DelObject(int index) {};
};

class WebFullScreenExitObject : public Referenced {
};

class WebFileChooserObject : public Referenced {
public:
    virtual std::string GetTitle(void* object) = 0;
    virtual int GetMode(void* object) = 0;
    virtual std::vector<std::string> GetAcceptType(void* object) = 0;
    virtual bool IsCapture(void* object) = 0;
    virtual int AddObject(void* object)
    {
        return 0;
    };
    virtual void DelObject(int index) {};
    virtual void HandleFileList(void* object, std::vector<std::string>& result, int index) = 0;
};

class WebGeolocationObject : public Referenced {
public:
    virtual std::string GetOrigin(void* object) = 0;
    virtual int AddObject(void* object)
    {
        return 0;
    };
    virtual void DelObject(int index) {};
    virtual void Invoke(int index, const std::string& origin, const bool& allow, const bool& retain) = 0;
};

class WebResourceErrorObject : public Referenced {
public:
    virtual std::string GetErrorInfo(void* object) = 0;
    virtual int GetErrorCode(void* object) = 0;
};

class WebOnSslErrorEventReceiveEventObject : public Referenced {
public:
    virtual int GetError(void* object) = 0;
    virtual std::vector<std::string> GetCertChainData(void* object) = 0;
    virtual int AddObject(void* object)
    {
        return 0;
    };
    virtual void DelObject(int index) {};
    virtual void Confirm(void* object, int index) {};
    virtual void Cancel(void* object, int index) {};
};

class WebSslErrorEventObject : public Referenced {
public:
    virtual int GetError(void* object) = 0;
    virtual std::string GetUrl(void* object) = 0;
    virtual std::string GetOriginalUrl(void* object) = 0;
    virtual std::string GetReferrer(void* object) = 0;
    virtual bool IsFatalError(void* object) = 0;
    virtual bool IsMainFrame(void* object) = 0;
    virtual int AddObject(void* object)
    {
        return 0;
    };
    virtual void DelObject(int index) {};
    virtual std::vector<std::string> GetCertificateChain(void* object) = 0;
    virtual void Confirm(void* object, int index) {};
    virtual void Cancel(void* object, bool abortLoading, int index) {};
};

class WebOnClientAuthenticationEventObject : public Referenced {
public:
    virtual std::string GetHost(void* object) = 0;
    virtual int GetPort(void* object) = 0;
    virtual std::vector<std::string> GetKeyTypes(void* object) = 0;
    virtual std::vector<std::string> GetIssuers(void* object) = 0;
    virtual void Confirm(void* object, const std::string& privateKeyFile, const std::string& certChainFile) {};
    virtual void Cancel(void* object) {};
    virtual void Ignore(void* object) {};
};
class WebObjectEventManager : public Singleton<WebObjectEventManager> {
    DECLARE_SINGLETON(WebObjectEventManager)
public:
    using EventObJectCallback = std::function<void(const std::string&, void *object)>;
    using EventObjectWithBoolReturnCallback = std::function<bool(const std::string&, void *object)>;
    using EventObjectWithResponseReturnCallback = std::function<RefPtr<WebResponse>(const std::string&, void *object)>;

    void RegisterObjectEvent(const std::string& eventId, const EventObJectCallback&& eventCallback)
    {
        eventObjectMap_[eventId] = std::move(eventCallback);
    }

    void RegisterObjectEventWithBoolReturn(
		const std::string& eventId, const EventObjectWithBoolReturnCallback&& eventCallback)
    {
        TAG_LOGI(AceLogTag::ACE_WEB, "RegisterObjectEventWithBoolReturn %{public}s", eventId.c_str());
        eventObjectWithBoolReturnMap_[eventId] = std::move(eventCallback);
    }

    void RegisterObjectEventWithResponseReturn(
        const std::string& eventId, const EventObjectWithResponseReturnCallback&& eventCallback)
    {
        eventObjectWithResponseReturnMap_[eventId] = std::move(eventCallback);
    }

    void UnRegisterObjectEvent(const std::string& eventId)
    {
        eventObjectMap_.erase(eventId);
    }

    void UnRegisterObjectEventWithBoolReturn(const std::string& eventId)
    {
        eventObjectWithBoolReturnMap_.erase(eventId);
    }

    void UnRegisterObjectEventWithResponseReturn(const std::string& eventId)
    {
        eventObjectWithResponseReturnMap_.erase(eventId);
    }

    void OnObjectEvent(const std::string& eventId, const std::string& param, void *jObject)
    {
        auto event = eventObjectMap_.find(eventId);
        if (event != eventObjectMap_.end() && event->second) {
            event->second(param, jObject);
        } else {
            TAG_LOGW(AceLogTag::ACE_WEB, "failed to find object eventId = %{public}s", eventId.c_str());
        }
    }

    bool OnObjectEventWithBoolReturn(const std::string& eventId, const std::string& param, void *jObject)
    {
        TAG_LOGI(AceLogTag::ACE_WEB, "OnObjectEventWithBoolReturn %{public}s", eventId.c_str());
        auto event = eventObjectWithBoolReturnMap_.find(eventId);
        if (event != eventObjectWithBoolReturnMap_.end() && event->second) {
            return event->second(param, jObject);
        } else {
            LOGW("failed to find object eventIdWithBoolReturn = %{public}s", eventId.c_str());
        }
        return false;
    }

    RefPtr<WebResponse> OnObjectEventWithResponseReturn(
        const std::string& eventId, const std::string& param, void* jObject)
    {
        auto event = eventObjectWithResponseReturnMap_.find(eventId);
        if (event != eventObjectWithResponseReturnMap_.end() && event->second) {
            return event->second(param, jObject);
        } else {
            LOGW("failed to find object eventIdWithResponseReturn = %{public}s", eventId.c_str());
        }
        return nullptr;
    }
    const RefPtr<WebResourceRequestObject>& GetResourceRequestObject()
    {
        return resourceRequestObject_;
    }

    void SetResourceRequestObject(const RefPtr<WebResourceRequestObject>& object)
    {
        resourceRequestObject_ = object;
    }

    const RefPtr<WebResourceErrorObject>& GetResourceErrorObject()
    {
        return resourceErrorObject_;
    }

    void SetResourceErrorObject(const RefPtr<WebResourceErrorObject>& object)
    {
        resourceErrorObject_ = object;
    }

    const RefPtr<WebScrollObject>& GetScrollObject()
    {
        return scrollObject_;
    }

    void SetScrollObject(const RefPtr<WebScrollObject>& object)
    {
        scrollObject_ = object;
    }

    const RefPtr<WebScaleChangeObject>& GetScaleChangeObject()
    {
        return scaleChangeObject_;
    }

    void SetScaleChangeObject(const RefPtr<WebScaleChangeObject>& object)
    {
        scaleChangeObject_ = object;
    }

    const RefPtr<WebResourceResponseObject>& GetResourceResponseObject()
    {
        return resourceResponseObject_;
    }
	
    const RefPtr<WebRefreshAccessedHistoryObject>& GetRefreshAccessedHistoryObject()
    {
        return refreshAccessedHistoryObject_;
    }

    void SetRefreshAccessedHistoryObject(const RefPtr<WebRefreshAccessedHistoryObject>& object)
    {
        refreshAccessedHistoryObject_ = object;
    }

    const RefPtr<WebFullScreenEnterObject>& GetFullScreenEnterObject()
    {
        return fullScreenEnterObject_;
    }

    void SetFullScreenEnterObject(const RefPtr<WebFullScreenEnterObject>& object)
    {
        fullScreenEnterObject_ = object;
    }

    const RefPtr<WebFullScreenExitObject>& GetFullScreenExitObject()
    {
        return fullScreenExitObject_;
    }

    void SetFullScreenExitObject(const RefPtr<WebFullScreenExitObject>& object)
    {
        fullScreenExitObject_ = object;
    }

    void SetResourceResponseObject(const RefPtr<WebResourceResponseObject>& object)
    {
        resourceResponseObject_ = object;
    }

    const RefPtr<WebConsoleMessageObject>& GetConsoleMessageObject()
    {
        return consoleMessageObject_;
    }

    void SetConsoleMessageObject(const RefPtr<WebConsoleMessageObject>& object)
    {
        consoleMessageObject_ = object;
    }

    const RefPtr<WebCommonDialogObject>& GetCommonDialogObject()
    {
        return commonDialogObject_;
    }

    void SetCommonDialogObject(const RefPtr<WebCommonDialogObject>& object)
    {
        commonDialogObject_ = object;
    }

    const RefPtr<WebPermissionRequestObject>& GetPermissionRequestObject()
    {
        return permissionRequestObject_;
    }

    void SetPermissionRequestObject(const RefPtr<WebPermissionRequestObject>& object)
    {
        permissionRequestObject_ = object;
    }

    const RefPtr<WebHttpAuthRequestObject>& GetHttpAuthRequestObject()
    {
        return httpAuthRequestObject_;
    }

    void SetHttpAuthRequestObject(const RefPtr<WebHttpAuthRequestObject>& object)
    {
        httpAuthRequestObject_ = object;
    }

    const RefPtr<WebDownloadResponseObject>& GetDownloadResponseObject()
    {
        return downloadResponseObject_;
    }

    void SetDownloadResponseObject(const RefPtr<WebDownloadResponseObject>& object)
    {
        downloadResponseObject_ = object;
    }

    const RefPtr<WebFileChooserObject>& GetFileChooserObject()
    {
        return fileChooserObject_;
    }

    void SetFileChooserObject(const RefPtr<WebFileChooserObject>& object)
    {
        fileChooserObject_ = object;
    }

    const RefPtr<WebGeolocationObject>& GetGeolocationObject()
    {
        return GeolocationObject_;
    }

    void SetGeolocationObject(const RefPtr<WebGeolocationObject>& object)
    {
        GeolocationObject_ = object;
    }

    const RefPtr<WebOnSslErrorEventReceiveEventObject>& GetOnSslErrorEventReceiveEventObject()
    {
        return onSslErrorEventReceiveEventObject_;
    }

    void SetOnSslErrorEventReceiveEventObject(const RefPtr<WebOnSslErrorEventReceiveEventObject>& object)
    {
        onSslErrorEventReceiveEventObject_ = object;
    }

    const RefPtr<WebSslErrorEventObject>& GetSslErrorEventObject()
    {
        return sslErrorEventObject_;
    }

    void SetSslErrorEventObject(const RefPtr<WebSslErrorEventObject>& object)
    {
        sslErrorEventObject_ = object;
    }

    const RefPtr<WebOnClientAuthenticationEventObject>& GetOnClientAuthenticationEventObject()
    {
        return onClientAuthenticationEventObject_;
    }

    void SetOnClientAuthenticationEventObject(const RefPtr<WebOnClientAuthenticationEventObject>& object)
    {
        onClientAuthenticationEventObject_ = object;
    }

private:
    RefPtr<WebResourceRequestObject> resourceRequestObject_;
    RefPtr<WebScrollObject> scrollObject_;
    RefPtr<WebScaleChangeObject> scaleChangeObject_;
    RefPtr<WebResourceErrorObject> resourceErrorObject_;
    RefPtr<WebResourceResponseObject> resourceResponseObject_;
    RefPtr<WebConsoleMessageObject> consoleMessageObject_;
    RefPtr<WebCommonDialogObject> commonDialogObject_;
    RefPtr<WebPermissionRequestObject> permissionRequestObject_;
    RefPtr<WebHttpAuthRequestObject> httpAuthRequestObject_;
    RefPtr<WebDownloadResponseObject> downloadResponseObject_;
    RefPtr<WebFileChooserObject> fileChooserObject_;
    RefPtr<WebGeolocationObject> GeolocationObject_;
    std::unordered_map<std::string, EventObJectCallback> eventObjectMap_;
    RefPtr<WebRefreshAccessedHistoryObject> refreshAccessedHistoryObject_;
    RefPtr<WebFullScreenEnterObject> fullScreenEnterObject_;
    RefPtr<WebFullScreenExitObject> fullScreenExitObject_;
    std::unordered_map<std::string, EventObjectWithBoolReturnCallback> eventObjectWithBoolReturnMap_;
    std::unordered_map<std::string, EventObjectWithResponseReturnCallback> eventObjectWithResponseReturnMap_;
    RefPtr<WebOnSslErrorEventReceiveEventObject> onSslErrorEventReceiveEventObject_;
    RefPtr<WebSslErrorEventObject> sslErrorEventObject_;
    RefPtr<WebOnClientAuthenticationEventObject> onClientAuthenticationEventObject_;
};
inline WebObjectEventManager::WebObjectEventManager() = default;
inline WebObjectEventManager::~WebObjectEventManager() = default;
} // namespace OHOS::Ace
#endif