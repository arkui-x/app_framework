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

/**
 * @addtogroup Native_EventHandler
 * @{
 *
 * @brief Provides <b>EventHandler</b>-specific functions, including functions for obtaining
 * <b>EventRunnerNativeImplement</b> instances and adding a file descriptor listener.
 *
 * @since 3
 * @version 2.0
 */

/**
 * @file native_interface_eventhandler.h
 *
 * @brief Declares the <b>EventHandler</b>-specific functions, including functions for obtaining
 * <b>EventRunnerNativeImplement</b> instances and adding a file descriptor listener.
 *
 * @since 3
 * @version 2.0
 */

#ifndef BASE_EVENTHANDLER_INTERFACES_KITS_NATIVE_NATIVE_INTERFACE_EVENTHANDLER_H
#define BASE_EVENTHANDLER_INTERFACES_KITS_NATIVE_NATIVE_INTERFACE_EVENTHANDLER_H

#ifdef __cplusplus
extern "C" {
#endif

namespace OHOS {
namespace AppExecFwk {
struct EventRunnerNativeImplement;
using EventRunnerNativeImplement = struct EventRunnerNativeImplement;

using FileFDCallback = void (*)(int);

/**
 * @brief Defines file descriptor callbacks.
 *
 * You should implement this structure before adding a file descriptor listener.
 *
 * @since 3
 * @version 2.0
 */
struct FileDescriptorCallbacks {
    /** Callback invoked when the file descriptor is readable */
    FileFDCallback readableCallback_;
    /** Callback invoked when the file descriptor is writable */
    FileFDCallback writableCallback_;
    /** Callback invoked when the file descriptor is closed */
    FileFDCallback shutdownCallback_;
    /** Callback invoked when the file descriptor encounters an exception */
    FileFDCallback exceptionCallback_;
};

/**
 * @brief Obtains the {@link EventRunnerNativeImplement} instance of the current thread.
 *
 * You should call this function or {@link CreateEventRunnerNativeObj} to obtain or create an
 * {@link EventRunnerNativeImplement} instance so that the instance will be passed as an input parameter
 * to other <b>EventHandler</b>-specific functions you will call.
 *
 * @return Returns a non-NULL value if the instance is obtained; returns <b>NULL</b> otherwise.
 * @since 3
 * @version 2.0
 */
const EventRunnerNativeImplement* GetEventRunnerNativeObjForThread();

/**
 * @brief Creates an {@link EventRunnerNativeImplement} instance in a new thread.
 *
 * You should call this function or {@link GetEventRunnerNativeObjForThread} to obtain or create an
 * {@link EventRunnerNativeImplement} instance so that the instance will be passed as an input parameter
 * to other <b>EventHandler</b>-specific functions you will call.
 *
 * @return Returns a non-NULL value if the instance is created; returns <b>NULL</b> otherwise.
 * @since 3
 * @version 2.0
 */
const EventRunnerNativeImplement* CreateEventRunnerNativeObj();

/**
 * @brief Starts the {@link EventRunnerNativeImplement} instance.
 *
 * If a file descriptor listener has been added, particular callbacks can be triggered upon corresponding
 * operations only when the {@link EventRunnerNativeImplement} instance is running.
 *
 * @param nativeObj Indicates the pointer to the {@link EventRunnerNativeImplement} instance obtained
 * by calling {@link GetEventRunnerNativeObjForThread} or {@link CreateEventRunnerNativeObj}.
 *
 * @return Returns <b>0</b> if the instance starts running; returns another value otherwise.
 * @see EventRunnerAddFileDescriptorListener
 * @since 3
 * @version 2.0
 */
int EventRunnerRun(const EventRunnerNativeImplement* nativeObj);

/**
 * @brief Stops the {@link EventRunnerNativeImplement} instance.
 *
 * @param nativeObj Indicates the pointer to the {@link EventRunnerNativeImplement} instance obtained
 * by calling {@link GetEventRunnerNativeObjForThread} or {@link CreateEventRunnerNativeObj}.
 *
 * @return Returns <b>0</b> if the instance stops running; returns another value otherwise.
 * @see EventRunnerRun
 * @since 3
 * @version 2.0
 */
int EventRunnerStop(const EventRunnerNativeImplement* nativeObj);

/**
 * @brief Adds a file descriptor listener for the {@link EventRunnerNativeImplement} instance.
 *
 * The file descriptor listener includes callbacks that can be invoked when the file descriptor is
 * readable, writable, or closed, or when it encounters an exception.
 *
 * @param nativeObj Indicates the pointer to the {@link EventRunnerNativeImplement} instance obtained
 * by calling {@link GetEventRunnerNativeObjForThread} or {@link CreateEventRunnerNativeObj}.
 * @param fileDescriptor Indicates the file descriptor for which the listener is to add.
 * @param events Indicates the file descriptor events, including input, output, and error events.
 * @param fileDescriptorCallbacks Indicates the pointer to the {@link FileDescriptorCallbacks} structure
 * containing file descriptor-related callbacks. You must implement this structure.
 *
 * @return Returns <b>0</b> if the file descriptor listener is added; returns another value otherwise.
 * @see EventRunnerRemoveFileDescriptorListener
 * @see FileDescriptorCallbacks
 * @since 3
 * @version 2.0
 */
int EventRunnerAddFileDescriptorListener(const EventRunnerNativeImplement* nativeObj, int fileDescriptor,
    unsigned int events, const FileDescriptorCallbacks* fileDescriptorCallbacks);

/**
 * @brief Removes the listener for a given file descriptor from the {@link EventRunnerNativeImplement} instance.
 *
 * @param nativeObj Indicates the pointer to the {@link EventRunnerNativeImplement} instance obtained by
 * calling {@link GetEventRunnerNativeObjForThread} or {@link CreateEventRunnerNativeObj}.
 * @param fileDescriptor Indicates the file descriptor for which the listener is to remove.
 * @see EventRunnerAddFileDescriptorListener
 * @since 3
 * @version 2.0
 */
void EventRunnerRemoveFileDescriptorListener(const EventRunnerNativeImplement* nativeObj, int fileDescriptor);
} // namespace AppExecFwk
} // namespace OHOS

#ifdef __cplusplus
};
#endif

/** @} */
#endif // BASE_EVENTHANDLER_INTERFACES_KITS_NATIVE_NATIVE_INTERFACE_EVENTHANDLER_H