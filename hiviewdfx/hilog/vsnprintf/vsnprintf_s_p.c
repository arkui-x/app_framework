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

#include "vsnprintf_s_p.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Define the max length of the string */
#ifndef SECUREC_STRING_MAX_LEN
#define SECUREC_STRING_MAX_LEN 0x7fffffffUL
#endif

#if SECUREC_STRING_MAX_LEN > 0x7fffffffUL
#error "max string is 2G"
#endif

#if defined(_WIN64) || defined(WIN64) || defined(__LP64__) || defined(_LP64)
#define SECUREC_ON_64BITS
#endif

#if defined(_DEBUG) || defined(DEBUG)
    #if defined(SECUREC_ERROR_HANDLER_BY_ASSERT)
        #define SECUREC_ERROR_INVALID_PARAMTER(msg) assert(msg "invalid argument" == NULL)
        #define SECUREC_ERROR_INVALID_RANGE(msg)    assert(msg "invalid dest buffer size" == NULL)
    #elif defined(SECUREC_ERROR_HANDLER_BY_PRINTF)
        #if SECUREC_IN_KERNEL
            #define SECUREC_ERROR_INVALID_PARAMTER(msg) printk("%s invalid argument\n", msg)
            #define SECUREC_ERROR_INVALID_RANGE(msg)    printk("%s invalid dest buffer size\n", msg)
        #else
            #define SECUREC_ERROR_INVALID_PARAMTER(msg) printf("%s invalid argument\n", msg)
            #define SECUREC_ERROR_INVALID_RANGE(msg)    printf("%s invalid dest buffer size\n", msg)
        #endif
    #elif defined(SECUREC_ERROR_HANDLER_BY_FILE_LOG)
        #define SECUREC_ERROR_INVALID_PARAMTER(msg) LogSecureCRuntimeError(msg " EINVAL\n")
        #define SECUREC_ERROR_INVALID_RANGE(msg)    LogSecureCRuntimeError(msg " ERANGE\n")
    #else
        #define SECUREC_ERROR_INVALID_PARAMTER(msg) ((void)0)
        #define SECUREC_ERROR_INVALID_RANGE(msg)    ((void)0)
    #endif

#else
    #define SECUREC_ERROR_INVALID_PARAMTER(msg) ((void)0)
    #define SECUREC_ERROR_INVALID_RANGE(msg)    ((void)0)
    #define SECUREC_ERROR_BUFFER_OVERLAP(msg)   ((void)0)
#endif

#define SECUREC_PRINTF_TRUNCATE (-2)
typedef struct {
    int count;
    char *cur;
} SecPrintfStream;

#ifdef SECUREC_STACK_SIZE_LESS_THAN_1K
/* SECUREC_BUFFER_SIZE Can not be less than 23 ,
 *the length of the octal representation of 64-bit integers with zero lead
 */
#define SECUREC_BUFFER_SIZE    256
#else
#define SECUREC_BUFFER_SIZE    512
#endif
#define SECUREC_MAX_PRECISION  SECUREC_BUFFER_SIZE
/* max. # bytes in multibyte char  ,see MB_LEN_MAX */
#define SECUREC_MB_LEN 16

#if (defined(_MSC_VER)) && (_MSC_VER >= 1400)
#define SECUREC_MASK_MSVC_CRT_WARNING __pragma(warning(push)) \
    __pragma(warning(disable:4996 4127))
#define SECUREC_END_MASK_MSVC_CRT_WARNING  __pragma(warning(pop))
#else
#define SECUREC_MASK_MSVC_CRT_WARNING
#define SECUREC_END_MASK_MSVC_CRT_WARNING
#endif

#define SECUREC_WHILE_ZERO SECUREC_MASK_MSVC_CRT_WARNING while (0) SECUREC_END_MASK_MSVC_CRT_WARNING

/* flag definitions */
/* Using macros instead of enumerations is because some of the enumerated types under the compiler are 16bit. */
#define SECUREC_FLAG_SIGN           0x00001U
#define SECUREC_FLAG_SIGN_SPACE     0x00002U
#define SECUREC_FLAG_LEFT           0x00004U
#define SECUREC_FLAG_LEADZERO       0x00008U
#define SECUREC_FLAG_LONG           0x00010U
#define SECUREC_FLAG_SHORT          0x00020U
#define SECUREC_FLAG_SIGNED         0x00040U
#define SECUREC_FLAG_ALTERNATE      0x00080U
#define SECUREC_FLAG_NEGATIVE       0x00100U
#define SECUREC_FLAG_FORCE_OCTAL    0x00200U
#define SECUREC_FLAG_LONG_DOUBLE    0x00400U
#define SECUREC_FLAG_WIDECHAR       0x00800U
#define SECUREC_FLAG_LONGLONG       0x01000U
#define SECUREC_FLAG_CHAR           0x02000U
#define SECUREC_FLAG_POINTER        0x04000U
#define SECUREC_FLAG_I64            0x08000U
#define SECUREC_FLAG_PTRDIFF        0x10000U
#define SECUREC_FLAG_SIZE           0x20000U
#ifdef  SECUREC_COMPATIBLE_LINUX_FORMAT
#define SECUREC_FLAG_INTMAX         0x40000U
#endif

/* put a char to output */
#define SECUREC_PUTC(_c, _stream)    ((--(_stream)->count >= 0) ? ((*(_stream)->cur++ = (char)(_c)) & 0xff) : EOF)
/* to clear e835 */
#define SECUREC_PUTC_ZERO(_stream)    ((--(_stream)->count >= 0) ? ((*(_stream)->cur++ = (char)('\0'))) : EOF)

/* state definitions */
typedef enum {
    STAT_NORMAL,
    STAT_PERCENT,
    STAT_FLAG,
    STAT_WIDTH,
    STAT_DOT,
    STAT_PRECIS,
    STAT_SIZE,
    STAT_TYPE,
    STAT_INVALID
} SecFmtState;

#ifndef HILOG_PROHIBIT_ALLOCATION
#ifndef SECUREC_MALLOC
#define SECUREC_MALLOC(x) malloc((size_t)(x))
#endif

#ifndef SECUREC_FREE
#define SECUREC_FREE(x)   free((void *)(x))
#endif

#else
#define SECUREC_MALLOC(x) (NULL)
#define SECUREC_FREE(x) { printf("Malloc is not allowed, so free should not be possible to execute!"); \
    exit(EXIT_FAILURE); }
#endif

#if (defined(_WIN32) || defined(_WIN64) || defined(_MSC_VER)) || defined(__ARMCC_VERSION)
typedef __int64 SecInt64;
typedef unsigned __int64 SecUnsignedInt64;
#if defined(__ARMCC_VERSION)
typedef int SecInt32;
typedef unsigned int SecUnsignedInt32;
#else
typedef __int32 SecInt32;
typedef unsigned __int32 SecUnsignedInt32;
#endif
#else
typedef int SecInt32;
typedef unsigned int SecUnsignedInt32;
typedef long long SecInt64;
typedef unsigned long long SecUnsignedInt64;
#endif

static inline void SecWriteString(const char *string, int len, SecPrintfStream *f, int *pnumwritten)
{
    const char *str = string;
    int count = len;
    while (count-- > 0) {
        if (SECUREC_PUTC(*str, f) == EOF) {
            *pnumwritten = -1;
            break;
        } else {
            ++(*pnumwritten);
            ++str;
        }
    }
}

static inline void SecWriteMultiChar(char ch, int num, SecPrintfStream *f, int *pnumwritten)
{
    int count = num;
    while (count-- > 0) {
        if (SECUREC_PUTC(ch, f) == EOF) {
            *pnumwritten = -1;
            break;
        } else {
            ++(*pnumwritten);
        }
    }
}

static inline int SecVsnprintfPImpl(char *string, size_t count, int priv, const char *format, va_list arglist);

/*******************************************************************************
 * <FUNCTION DESCRIPTION>
 *    The vsnprintf_s function is equivalent to the vsnprintf function
 *     except for the parameter destMax/count and the explicit runtime-constraints violation
 *    The vsnprintf_s function takes a pointer to an argument list, then formats
 *    and writes up to count characters of the given data to the memory pointed
 *    to by strDest and appends a terminating null.
 *
 * <INPUT PARAMETERS>
 *    strDest                  Storage location for the output.
 *    destMax                The size of the strDest for output.
 *    count                    Maximum number of character to write(not including
 *                                the terminating NULL)
 *    priv_on               whether print <private> for not-public args
 *    format                   Format-control string.
 *    arglist                     pointer to list of arguments.
 *
 * <OUTPUT PARAMETERS>
 *    strDest                is updated
 *
 * <RETURN VALUE>
 *    return  the number of characters written, not including the terminating null
 *    return -1 if an  error occurs.
 *    return -1 if count < destMax and the output string  has been truncated
 *
 * If there is a runtime-constraint violation, strDest[0] will be set to the '\0' when strDest and destMax valid
 *******************************************************************************
 */
HILOG_LOCAL_API
int vsnprintfp_s(char *strDest, size_t destMax, size_t count, int priv,  const char *format, va_list arglist)
{
    int retVal;

    if (format == NULL || strDest == NULL || destMax == 0 || destMax > SECUREC_STRING_MAX_LEN ||
        (count > (SECUREC_STRING_MAX_LEN - 1) && count != (size_t)(-1))) {
        if (strDest != NULL && destMax > 0) {
            strDest[0] = '\0';
        }
        SECUREC_ERROR_INVALID_PARAMTER("vsnprintfp_s");
        return -1;
    }

    if (destMax > count) {
        retVal = SecVsnprintfPImpl(strDest, count + 1, priv, format, arglist);
        if (retVal == SECUREC_PRINTF_TRUNCATE) {  /* lsd add to keep dest buffer not destroyed 2014.2.18 */
            /* the string has been truncated, return  -1 */
            return -1;          /* to skip error handler,  return strlen(strDest) or -1 */
        }
    } else {                    /* destMax <= count */
        retVal = SecVsnprintfPImpl(strDest, destMax, priv, format, arglist);
#ifdef SECUREC_COMPATIBLE_WIN_FORMAT
        if (retVal == SECUREC_PRINTF_TRUNCATE && count == (size_t)-1) {
            return -1;
        }
#endif
    }

    if (retVal < 0) {
        strDest[0] = '\0';      /* empty the dest strDest */

        if (retVal == SECUREC_PRINTF_TRUNCATE) {
            /* Buffer too small */
            SECUREC_ERROR_INVALID_RANGE("vsnprintfp_s");
        }

        SECUREC_ERROR_INVALID_PARAMTER("vsnprintfp_s");
        return -1;
    }

    return retVal;
}

HILOG_LOCAL_API
int snprintfp_s(char *strDest, size_t destMax, size_t count, int priv,  const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    int ret = vsnprintfp_s(strDest, destMax, count, priv, format, ap);
    va_end(ap);
    return ret;
}

#ifdef SECUREC_FOR_WCHAR
#undef SECUREC_FOR_WCHAR
#endif

typedef char SecChar;
#define SECUREC_CHAR(x) x

#define SECUREC_WRITE_MULTI_CHAR  SecWriteMultiChar
#define SECUREC_WRITE_STRING      SecWriteString
#include "output_p.inl"

static inline int SecVsnprintfPImpl(char *string, size_t count, int priv, const char *format, va_list arglist)
{
    SecPrintfStream str;
    int retVal;

    str.count = (int)(count);     /* this count include \0 character */
    str.cur = string;

    retVal = SecOutputPS(&str, priv, format, arglist);
    if ((retVal >= 0) && (SECUREC_PUTC_ZERO(&str) != EOF)) {
        return (retVal);
    } else if (str.count < 0) {
        /* the buffer was too small; we return truncation */
        string[count - 1] = 0;
        return SECUREC_PRINTF_TRUNCATE;
    }

    return -1;
}
