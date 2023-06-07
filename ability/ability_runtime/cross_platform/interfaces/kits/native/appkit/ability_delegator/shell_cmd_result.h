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

#ifndef OHOS_ABILITY_RUNTIME_SHELL_CMD_RESULT_H
#define OHOS_ABILITY_RUNTIME_SHELL_CMD_RESULT_H

#include <string>

namespace OHOS {
namespace AppExecFwk {
class ShellCmdResult {
public:
    /**
     * Default constructor used to create a ShellCmdResult instance.
     */
    ShellCmdResult() = default;

    /**
     * A constructor used to create a ShellCmdResult instance with the input parameter passed.
     *
     * @param exitCode Indicates the shell cmd exec result.
     * @param stdResult Indicates the cmd standard result.
     */
    ShellCmdResult(const int32_t exitCode, const std::string &stdResult);

    /**
     * Copy constructor used to create a ShellCmdResult instance with the input parameter passed.
     *
     * @param result Indicates the ShellCommandResult object that already exists.
     */
    explicit ShellCmdResult(const std::string &result);

    /**
     * Default deconstructor used to deconstruct.
     */
    ~ShellCmdResult() = default;

    /**
     * Sets exit code.
     *
     * @param exitCode Indicates the exit code.
     */
    void SetExitCode(const int32_t exitCode);

    /**
     * Obtains the exit code.
     *
     * @return the exit code.
     */
    int32_t GetExitCode() const;

    /**
     * Sets standard result.
     *
     * @param stdResult Indicates the standard result.
     */
    void SetStdResult(const std::string &stdResult);

    /**
     * Obtains the standard result.
     *
     * @return the standard result.
     */
    std::string GetStdResult() const;

    /**
     * Returns a string representation of the object.
     *
     * @return a string representation of the object.
     */
    std::string Dump();

private:
    int32_t exitCode_ {-1};
    std::string stdResult_;
};
}  // namespace AppExecFwk
}  // namespace OHOS

#endif  // OHOS_ABILITY_RUNTIME_SHELL_CMD_RESULT_H
