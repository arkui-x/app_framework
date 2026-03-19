/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef __VSNPRINTFP_S_H__
#define __VSNPRINTFP_S_H__

#include <hilog_base.h>

#include <stddef.h>
#include <stdarg.h>
/**
* @Description: The vsnprintfp_s function is equivalent to the vsnprintf function except for the parameter destMax/count
*               and the explicit runtime-constraints violation
* @param strDest -  produce output according to a format ,write to the character string strDest
* @param destMax - The maximum length of destination buffer(including the terminating null  byte ('\0'))
* @param count - do not write more than count bytes to strDest(not including the terminating null  byte ('\0'))
* @param priv_on - if true, any not %{public} prefix formatter arguments will be printed as "<private>"
* @param format - format string
* @param arglist - instead of  a variable  number of arguments
* @return:return the number of characters printed(not including the terminating null byte ('\0')),
*         If an error occurred return -1.Pay special attention to returning -1 when truncation occurs
*/
#ifdef __cplusplus
extern "C" {
#endif
HILOG_LOCAL_API
int vsnprintfp_s(char *strDest, size_t destMax, size_t count, int priv,  const char *format, va_list arglist);

HILOG_LOCAL_API
int snprintfp_s(char *strDest, size_t destMax, size_t count, int priv,  const char *format, ...);
#ifdef __cplusplus
}
#endif
#endif /* __VSNPRINTFP_S_H__ */
