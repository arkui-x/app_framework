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


/* [Standardize-exceptions] Use unsafe function: Portability
 * [reason] Use unsafe function to implement security function to maintain platform compatibility.
 *          And sufficient input validation is performed before calling
 */

#include <securec.h>

#ifndef OUTPUT_P_INL_2B263E9C_43D8_44BB_B17A_6D2033DECEE5
#define OUTPUT_P_INL_2B263E9C_43D8_44BB_B17A_6D2033DECEE5

#undef SECUREC_COMPATIBLE_LINUX_FORMAT

#define SECUREC_FLOAT_BUFSIZE (309+40)  /* max float point value */
#define SECUREC_FLOAT_BUFSIZE_LB (4932+40)  /* max long double value */

#define SECUREC_INT_MAX       2147483647

#define SECUREC_MUL10(x) ((((x) << 2) + (x)) << 1)
#define SECUREC_INT_MAX_DIV_TEN       21474836
#define SECUREC_MUL10_ADD_BEYOND_MAX(val) (((val) > SECUREC_INT_MAX_DIV_TEN))

#ifdef SECUREC_STACK_SIZE_LESS_THAN_1K
#define SECUREC_FMT_STR_LEN (8)
#else
#define SECUREC_FMT_STR_LEN (16)
#endif

typedef struct {
    unsigned int flags;
    int fldWidth;
    int precision;
    int bufferIsWide;           /* flag for buffer contains wide chars */
    int dynWidth;               /* %*   1 width from variable parameter ;0 not */
    int dynPrecision;           /* %.*  1 precision from variable parameter ;0 not */
} SecFormatAttr;

typedef union {
    char *str;                  /* not a null terminated  string */
    wchar_t *wStr;
} SecFormatBuf;

typedef union {
    char str[SECUREC_BUFFER_SIZE + 1];
#ifdef SECUREC_FOR_WCHAR
    wchar_t wStr[SECUREC_BUFFER_SIZE + 1];
#endif
} SecBuffer;

static int SecIndirectSprintf(char *strDest, size_t destMax, const char *format, ...)
{
    int ret;                    /* If initialization causes  e838 */
    va_list arglist;

    va_start(arglist, format);
    SECUREC_MASK_MSVC_CRT_WARNING
    ret = vsprintf_s(strDest, destMax, format, arglist);
    SECUREC_END_MASK_MSVC_CRT_WARNING
    va_end(arglist);
    (void)arglist;              /* to clear e438 last value assigned not used , the compiler will optimize this code */

    return ret;
}

#ifdef SECUREC_COMPATIBLE_LINUX_FORMAT
/* to clear e506 warning */
static int SecIsSameSize(size_t sizeA, size_t sizeB)
{
    return sizeA == sizeB;
}
#endif

#define SECUREC_SPECIAL(_val, Base) \
            case Base: \
            do { \
                *--formatBuf.str = digits[_val % Base]; \
            }while ((_val /= Base) != 0)

#define SECUREC_SAFE_WRITE_PREFIX(src, txtLen, _stream, outChars) do { \
            for (ii = 0; ii < txtLen; ++ii) { \
                *((SecChar *)(void *)(_stream->cur)) = *(src); \
                _stream->cur += sizeof(SecChar);              \
                ++(src);                                      \
            } \
            _stream->count -= txtLen * (int)(sizeof(SecChar)); \
            *(outChars) = *(outChars) + (txtLen); \
        } SECUREC_WHILE_ZERO

#define SECUREC_SAFE_WRITE_STR(src, txtLen, _stream, outChars) do { \
            if (txtLen < 12 /* for mobile number length */) { \
                for (ii = 0; ii < txtLen; ++ii) { \
                    *((SecChar *)(void *)(_stream->cur)) = *(src); \
                    _stream->cur += sizeof(SecChar); \
                    ++(src); \
                } \
            } else { \
                (void)memcpy_s(_stream->cur, _stream->count, src, ((size_t)(unsigned int)txtLen * (sizeof(SecChar)))); \
                _stream->cur += (size_t)(unsigned int)txtLen * (sizeof(SecChar)); \
            } \
            _stream->count -= txtLen * (int)(sizeof(SecChar)); \
            *(outChars) = *(outChars) + (txtLen); \
        } SECUREC_WHILE_ZERO

#define SECUREC_SAFE_WRITE_CHAR(_ch, _stream, outChars) do { \
            *((SecChar *)(void *)(_stream->cur)) = (SecChar)_ch; \
            _stream->cur += sizeof(SecChar); \
            _stream->count -= (int)(sizeof(SecChar)); \
            *(outChars) = *(outChars) + 1; \
        } SECUREC_WHILE_ZERO

#define SECUREC_SAFE_PADDING(padChar, padLen, _stream, outChars) do { \
            for (ii = 0; ii < padLen; ++ii) { \
                *((SecChar *)(void *)(_stream->cur)) = (SecChar)padChar; \
                _stream->cur += sizeof(SecChar); \
            } \
            _stream->count -= padLen * (int)(sizeof(SecChar)); \
            *(outChars) = *(outChars) + (padLen); \
        } SECUREC_WHILE_ZERO

/* The count variable can be reduced to 0, and the external function complements the \0 terminator. */
#define SECUREC_IS_REST_BUF_ENOUGH(needLen) ((int)(stream->count - (int)needLen * (int)(sizeof(SecChar)))  >= 0)

#define SECUREC_FMT_STATE_OFFSET  256
#ifdef SECUREC_FOR_WCHAR
#define SECUREC_FMT_TYPE(c, fmtTable)  ((((unsigned int)(int)(c)) <= (unsigned int)(int)SECUREC_CHAR('~')) ? \
                                      (fmtTable[(unsigned char)(c)]) : 0)
#define SECUREC_DECODE_STATE(c, fmtTable, laststate) (SecFmtState)(((fmtTable[(SECUREC_FMT_TYPE(c,fmtTable)) * \
                                                                            ((unsigned char)STAT_INVALID + 1) + \
                                                                            (unsigned char)(laststate) + \
                                                                            SECUREC_FMT_STATE_OFFSET])))
#else
#define SECUREC_DECODE_STATE(c,fmtTable,laststate) (SecFmtState)((fmtTable[(fmtTable[(unsigned char)(c)]) * \
                                                                           ((unsigned char)STAT_INVALID + 1) + \
                                                                           (unsigned char)(laststate) + \
                                                                           SECUREC_FMT_STATE_OFFSET]))
#endif

#define PUBLIC_FLAG_LEN  8
#define PRIVATE_FLAG_LEN 9
#define PUBLIC_FLAG      "{public}"
#define PRIVATE_FLAG     "{private}"

static void SecWritePrivateStr(SecPrintfStream *stream, int *pCharsOut)
{
    int ii = 0;
#define PRIVATE_STR_LEN (9)
#ifndef SECUREC_FOR_WCHAR
    static const char PRIVACY_STRING[] = "<private>";
    const char *pPrivStr = PRIVACY_STRING;

    if (SECUREC_IS_REST_BUF_ENOUGH(PRIVATE_STR_LEN)) {
        SECUREC_SAFE_WRITE_STR(pPrivStr, PRIVATE_STR_LEN, stream, pCharsOut);
    } else {
        SECUREC_WRITE_STRING(pPrivStr, PRIVATE_STR_LEN, stream, pCharsOut);
    }
#else
    static const wchar_t wprivacyString[] = { L'<', L'p', L'r', L'i', L'v', L'a', L't', L'e', L'>', L'\0' };
    const wchar_t *pwPrivStr = wprivacyString;

    if (SECUREC_IS_REST_BUF_ENOUGH(PRIVATE_STR_LEN)) {
        SECUREC_SAFE_WRITE_STR(pwPrivStr, PRIVATE_STR_LEN, stream, pCharsOut);
    } else {
        SECUREC_WRITE_STRING(pwPrivStr, PRIVATE_STR_LEN, stream, pCharsOut);
    }
#endif
}

static void SecDecodeFlags(SecChar ch, SecFormatAttr *attr)
{
    switch (ch) {
    case SECUREC_CHAR(' '):
        attr->flags |= SECUREC_FLAG_SIGN_SPACE;
        break;
    case SECUREC_CHAR('+'):
        attr->flags |= SECUREC_FLAG_SIGN;
        break;
    case SECUREC_CHAR('-'):
        attr->flags |= SECUREC_FLAG_LEFT;
        break;
    case SECUREC_CHAR('0'):
        attr->flags |= SECUREC_FLAG_LEADZERO;   /* add zero th the front */
        break;
    case SECUREC_CHAR('#'):
        attr->flags |= SECUREC_FLAG_ALTERNATE;  /* output %x with 0x */
        break;
    default:
        break;
    }
    return;
}

static int SecDecodeSize(SecChar ch, SecFormatAttr *attr, const SecChar **format)
{
    switch (ch) {
#ifdef SECUREC_COMPATIBLE_LINUX_FORMAT
    case SECUREC_CHAR('j'):
        attr->flags |= SECUREC_FLAG_INTMAX;
        break;
#endif
    case SECUREC_CHAR('q'):  /* fall-through */ /* FALLTHRU */
    case SECUREC_CHAR('L'):
        attr->flags |= SECUREC_FLAG_LONGLONG | SECUREC_FLAG_LONG_DOUBLE;
        break;
    case SECUREC_CHAR('l'):
        if (**format == SECUREC_CHAR('l')) {
            ++(*format);
            attr->flags |= SECUREC_FLAG_LONGLONG;   /* long long */
        } else {
            attr->flags |= SECUREC_FLAG_LONG;   /* long int or wchar_t */
        }
        break;
    case SECUREC_CHAR('t'):
        attr->flags |= SECUREC_FLAG_PTRDIFF;
        break;
#ifdef SECUREC_COMPATIBLE_LINUX_FORMAT
    case SECUREC_CHAR('z'):
        attr->flags |= SECUREC_FLAG_SIZE;
        break;
    case SECUREC_CHAR('Z'):
        attr->flags |= SECUREC_FLAG_SIZE;
        break;
#endif

    case SECUREC_CHAR('I'):
#ifdef SECUREC_ON_64BITS
        attr->flags |= SECUREC_FLAG_I64;    /* %I  to  INT64 */
#endif
        if ((**format == SECUREC_CHAR('6')) && (*((*format) + 1) == SECUREC_CHAR('4'))) {
            (*format) += 2;
            attr->flags |= SECUREC_FLAG_I64;    /* %I64  to  INT64 */
        } else if ((**format == SECUREC_CHAR('3')) && (*((*format) + 1) == SECUREC_CHAR('2'))) {
            (*format) += 2;
            attr->flags &= ~SECUREC_FLAG_I64;   /* %I64  to  INT32 */
        } else if ((**format == SECUREC_CHAR('d')) || (**format == SECUREC_CHAR('i')) ||
                   (**format == SECUREC_CHAR('o')) || (**format == SECUREC_CHAR('u')) ||
                   (**format == SECUREC_CHAR('x')) || (**format == SECUREC_CHAR('X'))) {
            /* do nothing */
        } else {
            /* Compatibility  code for "%I" just print I */
            return -1;
        }
        break;

    case SECUREC_CHAR('h'):
        if (**format == SECUREC_CHAR('h')) {
            attr->flags |= SECUREC_FLAG_CHAR;   /* char */
        } else {
            attr->flags |= SECUREC_FLAG_SHORT;  /* short int */
        }
        break;

    case SECUREC_CHAR('w'):
        attr->flags |= SECUREC_FLAG_WIDECHAR;   /* wide char */
        break;
    default:
        break;

    }

    return 0;
}

static int SecDecodeTypeC(SecFormatAttr *attr, unsigned int cValue, SecFormatBuf *formatBuf, SecBuffer *buffer)
{
    int textLen;
    wchar_t wchar;

#if (defined(SECUREC_COMPATIBLE_LINUX_FORMAT)) && !(defined(__hpux)) && !(defined(SECUREC_ON_SOLARIS))
    attr->flags &= ~SECUREC_FLAG_LEADZERO;
#endif

#ifdef SECUREC_FOR_WCHAR
    attr->bufferIsWide = 1;
    wchar = (wchar_t)cValue;
    if (attr->flags & SECUREC_FLAG_SHORT) {
        /* multibyte character to wide  character */
        char tempchar[2];
        tempchar[0] = (char)(wchar & 0x00ff);
        tempchar[1] = '\0';

        if (mbtowc(buffer->wStr, tempchar, sizeof(tempchar)) < 0) {
            return -1;
        }
    } else {
        buffer->wStr[0] = wchar;
    }
    formatBuf->wStr = buffer->wStr;
    textLen = 1;                /* only 1 wide character */
#else
    attr->bufferIsWide = 0;
    if (attr->flags & (SECUREC_FLAG_LONG | SECUREC_FLAG_WIDECHAR)) {
        wchar = (wchar_t)cValue;
        /* wide  character  to multibyte character */
        SECUREC_MASK_MSVC_CRT_WARNING
        textLen = wctomb(buffer->str, wchar);
        SECUREC_END_MASK_MSVC_CRT_WARNING
        if (textLen < 0) {
            return -1;
        }
    } else {
        /* get  multibyte character from argument */
        unsigned short temp = (unsigned short)cValue;
        buffer->str[0] = (char)temp;
        textLen = 1;
    }
    formatBuf->str = buffer->str;
#endif

    return textLen;
}

static int SecDecodeTypeS(SecFormatAttr *attr, char *argPtr, SecFormatBuf *formatBuf)
{
    /* literal string to print null ptr, define it on stack rather than const text area
       is to avoid gcc warning with pointing const text with variable */
    static char strNullString[8] = "(null)";
    static wchar_t wStrNullString[8] = { L'(', L'n', L'u', L'l', L'l', L')', L'\0', L'\0' };

    int finalPrecision;
    char *strEnd = NULL;
    wchar_t *wStrEnd = NULL;
    int textLen;

#if (defined(SECUREC_COMPATIBLE_LINUX_FORMAT)) && (!defined(SECUREC_ON_UNIX))
    attr->flags &= ~SECUREC_FLAG_LEADZERO;
#endif
    finalPrecision = (attr->precision == -1) ? SECUREC_INT_MAX : attr->precision;
    formatBuf->str = argPtr;

#ifdef SECUREC_FOR_WCHAR
#if defined(SECUREC_COMPATIBLE_LINUX_FORMAT)
    if (!(attr->flags & SECUREC_FLAG_LONG)) {
        attr->flags |= SECUREC_FLAG_SHORT;
    }
#endif
    if (attr->flags & SECUREC_FLAG_SHORT) {
        if (formatBuf->str == NULL) {   /* NULL passed, use special string */
            formatBuf->str = strNullString;
        }
        strEnd = formatBuf->str;
        for (textLen = 0; textLen < finalPrecision && *strEnd; textLen++) {
            ++strEnd;
        }
        /* textLen now contains length in multibyte chars */
    } else {
        if (formatBuf->wStr == NULL) {  /* NULL passed, use special string */
            formatBuf->wStr = wStrNullString;
        }
        attr->bufferIsWide = 1;
        wStrEnd = formatBuf->wStr;
        while (finalPrecision-- && *wStrEnd) {
            ++wStrEnd;
        }
        textLen = (int)(wStrEnd - formatBuf->wStr); /* in wchar_ts */
        /* textLen now contains length in wide chars */
    }
#else /* SECUREC_FOR_WCHAR */
    if (attr->flags & (SECUREC_FLAG_LONG | SECUREC_FLAG_WIDECHAR)) {
        if (formatBuf->wStr == NULL) {  /* NULL passed, use special string */
            formatBuf->wStr = wStrNullString;
        }
        attr->bufferIsWide = 1;
        wStrEnd = formatBuf->wStr;
        while (finalPrecision-- && *wStrEnd) {
            ++wStrEnd;
        }
        textLen = (int)(wStrEnd - formatBuf->wStr);
    } else {
        if (formatBuf->str == NULL) {   /* meet NULL, use special string */
            formatBuf->str = strNullString;
        }

        if (finalPrecision == SECUREC_INT_MAX) {
            /* precision NOT assigned */
            /* The strlen performance is high when the string length is greater than 32 */
            textLen = (int)strlen(formatBuf->str);
        } else {
            /* precision assigned */
            strEnd = formatBuf->str;
            while (finalPrecision-- && *strEnd) {
                ++strEnd;
            }
            textLen = (int)(strEnd - formatBuf->str);   /* length of the string */
        }

    }

#endif /* SECUREC_FOR_WCHAR */
    return textLen;
}

HILOG_LOCAL_API
int SecOutputPS(SecPrintfStream *stream, int priv, const char *cformat, va_list arglist)
{
    const SecChar *format = cformat;

    char *floatBuf = NULL;
    SecFormatBuf formatBuf;
    static const char *itoaUpperDigits = "0123456789ABCDEFX";
    static const char *itoaLowerDigits = "0123456789abcdefx";
    const char *digits = itoaUpperDigits;
    int ii = 0;

    unsigned int radix;
    int charsOut;               /* characters written */

    int prefixLen = 0;
    int padding = 0;

    int textLen;                /* length of the text */
    int bufferSize = 0;         /* size of formatBuf.str */
    int noOutput = 0;

    SecFmtState state;
    SecFmtState laststate;

    SecChar prefix[2] = { 0 };
    SecChar ch;                 /* currently read character */

    static const unsigned char FMT_CHAR_TABLE[337] = {
        /* type 0:    nospecial meaning;
           1:   '%';
           2:    '.'
           3:    '*'
           4:    '0'
           5:    '1' ... '9'
           6:    ' ', '+', '-', '#'
           7:     'h', 'l', 'L', 'F', 'w' , 'N','z','q','t','j'
           8:     'd','o','u','i','x','X','e','f','g'
         */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x06, 0x00, 0x00, 0x06, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x03, 0x06, 0x00, 0x06, 0x02, 0x00,
        0x04, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x08, 0x00, 0x08, 0x08, 0x08, 0x00, 0x07, 0x00, 0x00, 0x07, 0x00, 0x07, 0x00,
        0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x08, 0x08, 0x08, 0x08, 0x08, 0x07, 0x08, 0x07, 0x00, 0x07, 0x00, 0x00, 0x08,
        0x08, 0x07, 0x00, 0x08, 0x07, 0x08, 0x00, 0x07, 0x08, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00,
        /* fill zero  for normal char 128 byte for 0x80 - 0xff */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        /* state  0: normal
           1: percent
           2: flag
           3: width
           4: dot
           5: precis
           6: size
           7: type
           8: invalid
         */
        0x00, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x00, 0x00, 0x01, 0x00, 0x08, 0x08, 0x08, 0x08, 0x08,
        0x01, 0x00, 0x00, 0x04, 0x04, 0x04, 0x08, 0x08, 0x08, 0x00, 0x00, 0x00, 0x03, 0x03, 0x08, 0x05,
        0x08, 0x08, 0x00, 0x00, 0x00, 0x02, 0x02, 0x03, 0x05, 0x05, 0x08, 0x00, 0x00, 0x00, 0x03, 0x03,
        0x03, 0x05, 0x05, 0x08, 0x00, 0x00, 0x00, 0x02, 0x02, 0x02, 0x08, 0x08, 0x08, 0x00, 0x00, 0x00,
        0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x00, 0x00, 0x00, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x00,
        0x00
    };

    SecFormatAttr formatAttr;
    SecBuffer buffer;
    formatAttr.flags = 0;
    formatAttr.bufferIsWide = 0;    /* flag for buffer contains wide chars */
    formatAttr.fldWidth = 0;
    formatAttr.precision = 0;
    formatAttr.dynWidth = 0;
    formatAttr.dynPrecision = 0;

    charsOut = 0;
    textLen = 0;
    state = STAT_NORMAL;        /* starting state */
    formatBuf.str = NULL;

    int isPrivacy = 1; /*whether show private string*/

    /* loop each format character */
    /* remove format != NULL */
    while ((ch = *format++) != SECUREC_CHAR('\0') && charsOut >= 0) {
        laststate = state;
        state = SECUREC_DECODE_STATE(ch, FMT_CHAR_TABLE, laststate);

        switch (state) {
        case STAT_NORMAL:

NORMAL_CHAR:

            /* normal state, write character */
            if (SECUREC_IS_REST_BUF_ENOUGH(1 /* only one char */ )) {
                SECUREC_SAFE_WRITE_CHAR(ch, stream, &charsOut); /* char * cast to wchar * */
            } else {
#ifdef SECUREC_FOR_WCHAR
                SECUREC_WRITE_CHAR(ch, stream, &charsOut);
#else
                /* optimize function call to code */
                charsOut = -1;
                stream->count = -1;
#endif
            }

            continue;

        case STAT_PERCENT:
            /* set default values */
            prefixLen = 0;
            noOutput = 0;
            formatAttr.flags = 0;
            formatAttr.fldWidth = 0;
            formatAttr.precision = -1;
            formatAttr.bufferIsWide = 0;
            if (*format == SECUREC_CHAR('{')) {
                if (strncmp(format, PUBLIC_FLAG, PUBLIC_FLAG_LEN) == 0) {
                    isPrivacy = 0;
                    format += PUBLIC_FLAG_LEN;
                }
                else if (strncmp(format, PRIVATE_FLAG, PRIVATE_FLAG_LEN) == 0) {
                    isPrivacy = 1;
                    format += PRIVATE_FLAG_LEN;
                }
            }
            else {
                isPrivacy = 1;
            }

            if (priv == 0) {
                isPrivacy = 0;
            }

            break;

        case STAT_FLAG:
            /* set flag based on which flag character */
            SecDecodeFlags(ch, &formatAttr);
            break;

        case STAT_WIDTH:
            /* update width value */
            if (ch == SECUREC_CHAR('*')) {
                /* get width */
                formatAttr.fldWidth = (int)va_arg(arglist, int);
                if (formatAttr.fldWidth < 0) {
                    formatAttr.flags |= SECUREC_FLAG_LEFT;
                    formatAttr.fldWidth = -formatAttr.fldWidth;
                }
                formatAttr.dynWidth = 1;
            } else {
                if (laststate != STAT_WIDTH) {
                    formatAttr.fldWidth = 0;
                }
                if (SECUREC_MUL10_ADD_BEYOND_MAX(formatAttr.fldWidth)) {
                    return -1;
                }
                formatAttr.fldWidth = (int)SECUREC_MUL10((unsigned int)formatAttr.fldWidth) + (ch - SECUREC_CHAR('0'));
                formatAttr.dynWidth = 0;
            }
            break;

        case STAT_DOT:
            formatAttr.precision = 0;
            break;

        case STAT_PRECIS:
            /* update precision value */
            if (ch == SECUREC_CHAR('*')) {
                /* get precision from arg list */
                formatAttr.precision = (int)va_arg(arglist, int);
                if (formatAttr.precision < 0) {
                    formatAttr.precision = -1;
                }
                formatAttr.dynPrecision = 1;
            } else {
                /* add digit to current precision */
                if (SECUREC_MUL10_ADD_BEYOND_MAX(formatAttr.precision)) {
                    return -1;
                }
                formatAttr.precision =
                    (int)SECUREC_MUL10((unsigned int)formatAttr.precision) + (ch - SECUREC_CHAR('0'));
                formatAttr.dynPrecision = 0;
            }
            break;

        case STAT_SIZE:
            /* read a size specifier, set the formatAttr.flags based on it */
            if (SecDecodeSize(ch, &formatAttr, &format) != 0) {
                /* Compatibility  code for "%I" just print I */
                state = STAT_NORMAL;
                goto NORMAL_CHAR;
            }
            break;

        case STAT_TYPE:

            switch (ch) {

            case SECUREC_CHAR('C'):
                /* wide char */
                if (!(formatAttr.flags & (SECUREC_FLAG_SHORT | SECUREC_FLAG_LONG | SECUREC_FLAG_WIDECHAR))) {

#ifdef SECUREC_FOR_WCHAR
                    formatAttr.flags |= SECUREC_FLAG_SHORT;
#else
                    formatAttr.flags |= SECUREC_FLAG_WIDECHAR;
#endif
                }
                /* fall-through */
                /* FALLTHRU */
            case SECUREC_CHAR('c'):
                {
                    unsigned int cValue = (unsigned int)va_arg(arglist, int);
                    /*if it's a private arg, just write <priate> to stream*/
                    if (isPrivacy == 1) {
                        break;
                    }
                    textLen = SecDecodeTypeC(&formatAttr, cValue, &formatBuf, &buffer);
                    if (textLen < 0) {
                        noOutput = 1;
                    }
                }
                break;
            case SECUREC_CHAR('S'):    /* wide char string */
#ifndef SECUREC_FOR_WCHAR
                if (!(formatAttr.flags & (SECUREC_FLAG_SHORT | SECUREC_FLAG_LONG | SECUREC_FLAG_WIDECHAR))) {
                    formatAttr.flags |= SECUREC_FLAG_WIDECHAR;
                }
#else
                if (!(formatAttr.flags & (SECUREC_FLAG_SHORT | SECUREC_FLAG_LONG | SECUREC_FLAG_WIDECHAR))) {
                    formatAttr.flags |= SECUREC_FLAG_SHORT;
                }
#endif
                /* fall-through */
                /* FALLTHRU */
            case SECUREC_CHAR('s'):
                {
                    char *argPtr = (char *)va_arg(arglist, char *);
                    /*if it's a private arg, just write <priate> to stream*/
                    if (isPrivacy == 1) {
                        break;
                    }
                    textLen = SecDecodeTypeS(&formatAttr, argPtr, &formatBuf);
                }
                break;

            case SECUREC_CHAR('n'):
                /* higher risk disable it */
                return -1;

            case SECUREC_CHAR('E'):    /* fall-through */ /* FALLTHRU */
            case SECUREC_CHAR('F'):    /* fall-through */ /* FALLTHRU */
            case SECUREC_CHAR('G'):    /* fall-through */ /* FALLTHRU */
            case SECUREC_CHAR('A'):    /* fall-through */ /* FALLTHRU */
                /* convert format char to lower , use Explicit conversion to clean up compilation warning */
                ch = (SecChar)(ch + ((SecChar)(SECUREC_CHAR('a')) - (SECUREC_CHAR('A'))));
                /* fall-through */
                /* FALLTHRU */
            case SECUREC_CHAR('e'):    /* fall-through */ /* FALLTHRU */
            case SECUREC_CHAR('f'):    /* fall-through */ /* FALLTHRU */
            case SECUREC_CHAR('g'):    /* fall-through */ /* FALLTHRU */
            case SECUREC_CHAR('a'):
                {
                    /*if it's a private arg, just write <priate> to stream*/
                    if (isPrivacy == 1) {
#ifdef SECUREC_COMPATIBLE_LINUX_FORMAT
                        if (formatAttr.flags & SECUREC_FLAG_LONG_DOUBLE) {
                            (void)va_arg(arglist, long double);
                        } else
#endif
                        {
                            (void)va_arg(arglist, double);
                        }
                        break;
                    }

                    /* floating point conversion */
                    formatBuf.str = buffer.str; /* output buffer for float string with default size */
                    size_t formatBufLen = sizeof(buffer);

                    /* compute the precision value */
                    if (formatAttr.precision < 0) {
                        formatAttr.precision = 6;
                    } else if (formatAttr.precision == 0 && ch == SECUREC_CHAR('g')) {
                        formatAttr.precision = 1;
                    }

                    /* calc buffer size to store long double value */
                    if (formatAttr.flags & SECUREC_FLAG_LONG_DOUBLE) {
                        if (formatAttr.precision > (SECUREC_INT_MAX - SECUREC_FLOAT_BUFSIZE_LB)) {
                            noOutput = 1;
                            break;
                        }
                        bufferSize = SECUREC_FLOAT_BUFSIZE_LB + formatAttr.precision;
                    } else {
                        if (formatAttr.precision > (SECUREC_INT_MAX - SECUREC_FLOAT_BUFSIZE)) {
                            noOutput = 1;
                            break;
                        }
                        bufferSize = SECUREC_FLOAT_BUFSIZE + formatAttr.precision;
                    }
                    if (formatAttr.fldWidth > bufferSize) {
                        bufferSize = formatAttr.fldWidth;
                    }

                    if (bufferSize >= SECUREC_BUFFER_SIZE) {
                        /* the current value of SECUREC_BUFFER_SIZE could NOT store the formatted float string */
                        formatBufLen = (size_t)(unsigned int)bufferSize + (size_t)2; // size 2: include '+' and '\0'
                        floatBuf = (char *)SECUREC_MALLOC(formatBufLen);
                        if (floatBuf != NULL) {
                            formatBuf.str = floatBuf;
                        } else {
                            noOutput = 1;
                            break;
                        }
                    }

                    {
                        /* add following code to call system sprintf API for float number */
                        const SecChar *pFltFmt = format - 2;    /* point to the position before 'f' or 'g' */
                        int k;
                        int fltFmtStrLen;
                        char fltFmtBuf[SECUREC_FMT_STR_LEN];
                        char *fltFmtStr = fltFmtBuf;
                        char *fltFmtHeap = NULL;    /* to clear warning */

                        /* must meet '%' (normal format) or '}'(with{private} or{public} format)*/
                        while (*pFltFmt != SECUREC_CHAR('%') && *pFltFmt != SECUREC_CHAR('}')) {
                            --pFltFmt;
                        }
                        fltFmtStrLen = (int)((format - pFltFmt) + 1);   /* with ending terminator */
                        if (fltFmtStrLen > SECUREC_FMT_STR_LEN) {
                            /* if SECUREC_FMT_STR_LEN is NOT enough, alloc a new buffer */
                            fltFmtHeap = (char *)SECUREC_MALLOC((size_t)((unsigned int)fltFmtStrLen));
                            if (fltFmtHeap == NULL) {
                                noOutput = 1;
                                break;
                            } else {
                                fltFmtHeap[0] = '%';
                                for (k = 1; k < fltFmtStrLen - 1; ++k) {
                                    /* convert wchar to char */
                                    fltFmtHeap[k] = (char)(pFltFmt[k]); /* copy the format string */
                                }
                                fltFmtHeap[k] = '\0';

                                fltFmtStr = fltFmtHeap;
                            }
                        } else {
                            /* purpose of the repeat code is to solve the tool alarm  Redundant_Null_Check */
                            fltFmtBuf[0] = '%';
                            for (k = 1; k < fltFmtStrLen - 1; ++k) {
                                /* convert wchar to char */
                                fltFmtBuf[k] = (char)(pFltFmt[k]);  /* copy the format string */
                            }
                            fltFmtBuf[k] = '\0';
                        }

#ifdef SECUREC_COMPATIBLE_LINUX_FORMAT
                        if (formatAttr.flags & SECUREC_FLAG_LONG_DOUBLE) {
                            long double tmp = (long double)va_arg(arglist, long double);
                            /* call system sprintf to format float value */
                            if (formatAttr.dynWidth && formatAttr.dynPrecision) {
                                textLen = SecIndirectSprintf(formatBuf.str, formatBufLen, (char *)fltFmtStr,
                                                             formatAttr.fldWidth,formatAttr.precision, tmp);
                            } else if (formatAttr.dynWidth) {
                                textLen = SecIndirectSprintf(formatBuf.str, formatBufLen, (char *)fltFmtStr,
                                                             formatAttr.fldWidth, tmp);
                            } else if (formatAttr.dynPrecision) {
                                textLen = SecIndirectSprintf(formatBuf.str, formatBufLen, (char *)fltFmtStr,
                                                             formatAttr.precision, tmp);
                            } else {
                                textLen = SecIndirectSprintf(formatBuf.str, formatBufLen, (char *)fltFmtStr, tmp);
                            }
                        } else
#endif
                        {
                            double tmp = (double)va_arg(arglist, double);
                            if (formatAttr.dynWidth && formatAttr.dynPrecision) {
                                textLen = SecIndirectSprintf(formatBuf.str, formatBufLen, (char *)fltFmtStr,
                                                             formatAttr.fldWidth, formatAttr.precision, tmp);
                            } else if (formatAttr.dynWidth) {
                                textLen = SecIndirectSprintf(formatBuf.str, formatBufLen, (char *)fltFmtStr,
                                                             formatAttr.fldWidth, tmp);
                            } else if (formatAttr.dynPrecision) {
                                textLen = SecIndirectSprintf(formatBuf.str, formatBufLen, (char *)fltFmtStr,
                                                             formatAttr.precision, tmp);
                            } else {
                                textLen = SecIndirectSprintf(formatBuf.str, formatBufLen, (char *)fltFmtStr, tmp);
                            }
                        }

                        if (fltFmtHeap != NULL) {
                            /* if buffer is allocated on heap, free it */
                            SECUREC_FREE(fltFmtHeap);
                            fltFmtHeap = NULL;
                            /* to clear e438 last value assigned not used , the compiler will optimize this code */
                            (void)fltFmtHeap;
                        }
                        if (textLen < 0) {
                            /* bufferSize is large enough,just validation the return value */
                            noOutput = 1;
                            break;
                        }

                        formatAttr.fldWidth = textLen;  /* no padding ,this variable to calculate amount of padding */
                        prefixLen = 0;  /* no padding ,this variable to  calculate amount of padding */
                        formatAttr.flags = 0;   /* clear all internal formatAttr.flags */
                        break;
                    }
                }

            case SECUREC_CHAR('p'):
                /* print a pointer */
#if defined(SECUREC_COMPATIBLE_WIN_FORMAT)
                formatAttr.flags &= ~SECUREC_FLAG_LEADZERO;
#else
                formatAttr.flags |= SECUREC_FLAG_POINTER;
#endif

#ifdef SECUREC_ON_64BITS
                formatAttr.flags |= SECUREC_FLAG_I64;   /* converting an int64 */
#else
                formatAttr.flags |= SECUREC_FLAG_LONG;  /* converting a long */
#endif

#if (defined(SECUREC_COMPATIBLE_LINUX_FORMAT) || defined(SECUREC_VXWORKS_PLATFORM)) && (!defined(SECUREC_ON_UNIX))

#if defined(SECUREC_VXWORKS_PLATFORM)
                formatAttr.precision = 1;
#else
                formatAttr.precision = 0;
#endif
                formatAttr.flags |= SECUREC_FLAG_ALTERNATE; /* "0x" is not default prefix in UNIX */
                digits = itoaLowerDigits;
                goto OUTPUT_HEX;
#else
/* not linux vxwoks */
#if defined(_AIX) || defined(SECUREC_ON_SOLARIS)
                formatAttr.precision = 1;
#else
                formatAttr.precision = 2 * sizeof(void *);
#endif

#endif

#if defined(SECUREC_ON_UNIX)
                digits = itoaLowerDigits;
                goto OUTPUT_HEX;
#endif

                /* fall-through */
                /* FALLTHRU */
            case SECUREC_CHAR('X'):
                /* unsigned upper hex output */
                digits = itoaUpperDigits;
                goto OUTPUT_HEX;
            case SECUREC_CHAR('x'):
                /* unsigned lower hex output */
                digits = itoaLowerDigits;

OUTPUT_HEX:
                radix = 16;
                if (formatAttr.flags & SECUREC_FLAG_ALTERNATE) {
                    /* alternate form means '0x' prefix */
                    prefix[0] = SECUREC_CHAR('0');
                    prefix[1] = (SecChar)(digits[16]); /* 'x' or 'X' */

#if (defined(SECUREC_COMPATIBLE_LINUX_FORMAT) || defined(SECUREC_VXWORKS_PLATFORM))
                    if (ch == 'p') {
                        prefix[1] = SECUREC_CHAR('x');
                    }
#endif
#if defined(_AIX) || defined(SECUREC_ON_SOLARIS)
                    if (ch == 'p') {
                        prefixLen = 0;
                    } else {
                        prefixLen = 2;
                    }
#else
                    prefixLen = 2;
#endif

                }
                goto OUTPUT_INT;
            case SECUREC_CHAR('i'):    /* fall-through */ /* FALLTHRU */
            case SECUREC_CHAR('d'):    /* fall-through */ /* FALLTHRU */
                /* signed decimal output */
                formatAttr.flags |= SECUREC_FLAG_SIGNED;
                /* fall-through */
                /* FALLTHRU */
            case SECUREC_CHAR('u'):
                radix = 10;
                goto OUTPUT_INT;
            case SECUREC_CHAR('o'):
                /* unsigned octal output */
                radix = 8;
                if (formatAttr.flags & SECUREC_FLAG_ALTERNATE) {
                    /* alternate form means force a leading 0 */
                    formatAttr.flags |= SECUREC_FLAG_FORCE_OCTAL;
                }
OUTPUT_INT:
                {

                    SecUnsignedInt64 number = 0;    /* number to convert */
                    SecInt64 l; /* temp long value */
                    unsigned char tch;
#if defined(SECUREC_VXWORKS_VERSION_5_4) && !defined(SECUREC_ON_64BITS)
                    SecUnsignedInt32 digit = 0; /* ascii value of digit */
                    SecUnsignedInt32 quotientHigh = 0;
                    SecUnsignedInt32 quotientLow = 0;
#endif

                    /* read argument into variable l */
                    if (formatAttr.flags & SECUREC_FLAG_I64) {
                        l = (SecInt64)va_arg(arglist, SecInt64);
                    } else if (formatAttr.flags & SECUREC_FLAG_LONGLONG) {
                        l = (SecInt64)va_arg(arglist, SecInt64);
                    } else
#ifdef SECUREC_ON_64BITS
                    if (formatAttr.flags & SECUREC_FLAG_LONG) {
                        l = (long)va_arg(arglist, long);
                    } else
#endif /* SECUREC_ON_64BITS */
                    if (formatAttr.flags & SECUREC_FLAG_CHAR) {
                        if (formatAttr.flags & SECUREC_FLAG_SIGNED) {
                            l = (char)va_arg(arglist, int); /* sign extend */
                            if (l >= 128) { /* on some platform, char is always unsigned */
                                SecUnsignedInt64 tmpL = (SecUnsignedInt64)l;
                                formatAttr.flags |= SECUREC_FLAG_NEGATIVE;
                                tch = (unsigned char)(~(tmpL));
                                l = tch + 1;
                            }
                        } else {
                            l = (unsigned char)va_arg(arglist, int);    /* zero-extend */
                        }

                    } else if (formatAttr.flags & SECUREC_FLAG_SHORT) {
                        if (formatAttr.flags & SECUREC_FLAG_SIGNED) {
                            l = (short)va_arg(arglist, int);    /* sign extend */
                        } else {
                            l = (unsigned short)va_arg(arglist, int);   /* zero-extend */
                        }

                    }
#ifdef SECUREC_COMPATIBLE_LINUX_FORMAT
                    else if (formatAttr.flags & SECUREC_FLAG_PTRDIFF) {
                        l = (ptrdiff_t)va_arg(arglist, ptrdiff_t);  /* sign extend */
                    } else if (formatAttr.flags & SECUREC_FLAG_SIZE) {
                        if (formatAttr.flags & SECUREC_FLAG_SIGNED) {
                            /* No suitable macros were found to handle the branch */
                            if (SecIsSameSize(sizeof(size_t), sizeof(long))) {
                                l = va_arg(arglist, long);  /* sign extend */
                            } else if (SecIsSameSize(sizeof(size_t), sizeof(long long))) {
                                l = va_arg(arglist, long long); /* sign extend */
                            } else {
                                l = va_arg(arglist, int);   /* sign extend */
                            }
                        } else {
                            l = (SecInt64)(size_t)va_arg(arglist, size_t);  /* sign extend */
                        }
                    } else if (formatAttr.flags & SECUREC_FLAG_INTMAX) {
                        if (formatAttr.flags & SECUREC_FLAG_SIGNED) {
                            l = va_arg(arglist, SecInt64);  /* sign extend */
                        } else {
                            l = (SecInt64)(SecUnsignedInt64)va_arg(arglist, SecUnsignedInt64);  /* sign extend */
                        }
                    }
#endif
                    else {
                        if (formatAttr.flags & SECUREC_FLAG_SIGNED) {
                            l = va_arg(arglist, int);   /* sign extend */
                        } else {
                            l = (unsigned int)va_arg(arglist, int); /* zero-extend */
                        }

                    }
                    /*if it's a private arg, just write <priate> to stream*/
                    if (isPrivacy == 1) {
                        break;
                    }

                    /* check for negative; copy into number */
                    if ((formatAttr.flags & SECUREC_FLAG_SIGNED) && l < 0) {
                        number = (SecUnsignedInt64)(-l);
                        formatAttr.flags |= SECUREC_FLAG_NEGATIVE;
                    } else {
                        number = (SecUnsignedInt64)l;
                    }

                    if (((formatAttr.flags & SECUREC_FLAG_I64) == 0) &&
#ifdef SECUREC_COMPATIBLE_LINUX_FORMAT
                        ((formatAttr.flags & SECUREC_FLAG_INTMAX) == 0) &&
#endif
#ifdef SECUREC_ON_64BITS
                        ((formatAttr.flags & SECUREC_FLAG_PTRDIFF) == 0) &&
                        ((formatAttr.flags & SECUREC_FLAG_SIZE) == 0) &&
#if !defined(SECUREC_COMPATIBLE_WIN_FORMAT)  /* on window 64 system sizeof long is 32bit */
                        ((formatAttr.flags & SECUREC_FLAG_LONG) == 0) &&
#endif
#endif
                        ((formatAttr.flags & SECUREC_FLAG_LONGLONG) == 0)) {

                            number &= 0xffffffff;
                    }

                    /* check precision value for default */
                    if (formatAttr.precision < 0) {
                        formatAttr.precision = 1;   /* default precision */
                    } else {
#if defined(SECUREC_COMPATIBLE_WIN_FORMAT)
                        formatAttr.flags &= ~SECUREC_FLAG_LEADZERO;
#else
                        if (!(formatAttr.flags & SECUREC_FLAG_POINTER)) {
                            formatAttr.flags &= ~SECUREC_FLAG_LEADZERO;
                        }
#endif
                        if (formatAttr.precision > SECUREC_MAX_PRECISION) {
                            formatAttr.precision = SECUREC_MAX_PRECISION;
                        }
                    }

                    /* Check if data is 0; if so, turn off hex prefix,if 'p',add 0x prefix,else not add prefix */
                    if (number == 0) {
#if !(defined(SECUREC_VXWORKS_PLATFORM)||defined(__hpux))
                        prefixLen = 0;
#else
                        if ((ch == 'p') && (formatAttr.flags & SECUREC_FLAG_ALTERNATE))
                            prefixLen = 2;
                        else
                            prefixLen = 0;
#endif
                    }

                    /* Convert data to ASCII */
                    formatBuf.str = &buffer.str[SECUREC_BUFFER_SIZE];

                    if (number > 0) {
#ifdef SECUREC_ON_64BITS
                        switch (radix) {
                            /* the compiler will optimize each one */
                            SECUREC_SPECIAL(number, 10);
                            break;
                            SECUREC_SPECIAL(number, 16);
                            break;
                            SECUREC_SPECIAL(number, 8);
                            break;
                        default:
                            break;
                        }
#else /* for 32 bits system */
                        if (number <= 0xFFFFFFFFUL) {
                            /* in most case, the value to be converted is small value */
                            SecUnsignedInt32 n32Tmp = (SecUnsignedInt32)number;
                            switch (radix) {
                                SECUREC_SPECIAL(n32Tmp, 16);
                                break;
                                SECUREC_SPECIAL(n32Tmp, 8);
                                break;

#ifdef _AIX
                                /* the compiler will optimize div 10 */
                                SECUREC_SPECIAL(n32Tmp, 10);
                                break;
#else
                            case 10:
                                {
                                    /* fast div 10 */
                                    SecUnsignedInt32 q;
                                    SecUnsignedInt32 r;
                                    do {
                                        *--formatBuf.str = digits[n32Tmp % 10];
                                        q = (n32Tmp >> 1) + (n32Tmp >> 2);
                                        q = q + (q >> 4);
                                        q = q + (q >> 8);
                                        q = q + (q >> 16);
                                        q = q >> 3;
                                        r = n32Tmp - (((q << 2) + q) << 1);
                                        n32Tmp = (r > 9) ? (q + 1) : q;
                                    } while (n32Tmp != 0);
                                }
                                break;
#endif
                            default:
                                break;
                            }   /* end switch */
                        } else {
                            /* the value to be converted is greater than 4G */
#if defined(SECUREC_VXWORKS_VERSION_5_4)
                            do {
                                if (0 != SecU64Div32((SecUnsignedInt32)((number >> 16) >> 16),
                                                     (SecUnsignedInt32)number,
                                                     (SecUnsignedInt32)radix, &quotientHigh, &quotientLow, &digit)) {
                                    noOutput = 1;
                                    break;
                                }
                                *--formatBuf.str = digits[digit];

                                number = (SecUnsignedInt64)quotientHigh;
                                number = (number << 32) + quotientLow;
                            } while (number != 0);
#else
                            switch (radix) {
                                /* the compiler will optimize div 10 */
                                SECUREC_SPECIAL(number, 10);
                                break;
                                SECUREC_SPECIAL(number, 16);
                                break;
                                SECUREC_SPECIAL(number, 8);
                                break;
                            default:
                                break;
                            }
#endif
                        }
#endif
                    }           /* END if (number > 0) */
                    /* compute length of number,.if textLen > 0, then formatBuf.str must be in buffer.str */
                    textLen = (int)((char *)&buffer.str[SECUREC_BUFFER_SIZE] - formatBuf.str);
                    if (formatAttr.precision > textLen) {
                        for (ii = 0; ii < formatAttr.precision - textLen; ++ii) {
                            *--formatBuf.str = '0';
                        }
                        textLen = formatAttr.precision;
                    }

                    /* Force a leading zero if FORCEOCTAL flag set */
                    if ((formatAttr.flags & SECUREC_FLAG_FORCE_OCTAL) && (textLen == 0 || formatBuf.str[0] != '0')) {
                        *--formatBuf.str = '0';
                        ++textLen;  /* add a zero */
                    }
                }
                break;
            default:
                break;
            }
            /*if it's a private arg, just write <priate> to stream*/
            if (isPrivacy == 1) {
                SecWritePrivateStr(stream, &charsOut);
                break;
            }

            if (noOutput == 0) {
                if (formatAttr.flags & SECUREC_FLAG_SIGNED) {
                    if (formatAttr.flags & SECUREC_FLAG_NEGATIVE) {
                        /* prefix is a '-' */
                        prefix[0] = SECUREC_CHAR('-');
                        prefixLen = 1;
                    } else if (formatAttr.flags & SECUREC_FLAG_SIGN) {
                        /* prefix is '+' */
                        prefix[0] = SECUREC_CHAR('+');
                        prefixLen = 1;
                    } else if (formatAttr.flags & SECUREC_FLAG_SIGN_SPACE) {
                        /* prefix is ' ' */
                        prefix[0] = SECUREC_CHAR(' ');
                        prefixLen = 1;
                    }
                }

#if defined(SECUREC_COMPATIBLE_LINUX_FORMAT) && (!defined(SECUREC_ON_UNIX))
                if ((formatAttr.flags & SECUREC_FLAG_POINTER) && (0 == textLen)) {
                    formatAttr.flags &= ~SECUREC_FLAG_LEADZERO;
                    formatBuf.str = &buffer.str[SECUREC_BUFFER_SIZE - 1];
                    *formatBuf.str-- = '\0';
                    *formatBuf.str-- = ')';
                    *formatBuf.str-- = 'l';
                    *formatBuf.str-- = 'i';
                    *formatBuf.str-- = 'n';
                    *formatBuf.str = '(';
                    textLen = 5;
                }
#endif

                /* calculate amount of padding */
                padding = (formatAttr.fldWidth - textLen) - prefixLen;

                /* put out the padding, prefix, and text, in the correct order */

                if (!(formatAttr.flags & (SECUREC_FLAG_LEFT | SECUREC_FLAG_LEADZERO)) && padding > 0) {
                    /* pad on left with blanks */
                    if (SECUREC_IS_REST_BUF_ENOUGH(padding)) {
                        /* char * cast to wchar * */
                        SECUREC_SAFE_PADDING(SECUREC_CHAR(' '), padding, stream, &charsOut);
                    } else {
                        SECUREC_WRITE_MULTI_CHAR(SECUREC_CHAR(' '), padding, stream, &charsOut);
                    }
                }

                /* write prefix */
                if (prefixLen > 0) {
                    SecChar *pPrefix = prefix;
                    if (SECUREC_IS_REST_BUF_ENOUGH(prefixLen)) {
                        /* max prefix len is 2, use loop copy */ /* char * cast to wchar * */
                        SECUREC_SAFE_WRITE_PREFIX(pPrefix, prefixLen, stream, &charsOut);
                    } else {
                        SECUREC_WRITE_STRING(prefix, prefixLen, stream, &charsOut);
                    }
                }

                if ((formatAttr.flags & SECUREC_FLAG_LEADZERO) && !(formatAttr.flags & SECUREC_FLAG_LEFT)
                    && padding > 0) {
                    /* write leading zeros */
                    if (SECUREC_IS_REST_BUF_ENOUGH(padding)) {
                        /* char * cast to wchar * */
                        SECUREC_SAFE_PADDING(SECUREC_CHAR('0'), padding, stream, &charsOut);
                    } else {
                        SECUREC_WRITE_MULTI_CHAR(SECUREC_CHAR('0'), padding, stream, &charsOut);
                    }
                }

                /* write text */
#ifndef SECUREC_FOR_WCHAR
                if (formatAttr.bufferIsWide && (textLen > 0)) {
                    wchar_t *p = formatBuf.wStr;
                    int count = textLen;
                    while (count--) {
                        char tmpBuf[SECUREC_MB_LEN + 1];
                        SECUREC_MASK_MSVC_CRT_WARNING
                        int retVal = wctomb(tmpBuf, *p++);
                        SECUREC_END_MASK_MSVC_CRT_WARNING
                        if (retVal <= 0) {
                            charsOut = -1;
                            break;
                        }
                        SECUREC_WRITE_STRING(tmpBuf, retVal, stream, &charsOut);
                    }
                } else {
                    if (SECUREC_IS_REST_BUF_ENOUGH(textLen)) {
                        SECUREC_SAFE_WRITE_STR(formatBuf.str, textLen, stream, &charsOut);
                    } else {
                        SECUREC_WRITE_STRING(formatBuf.str, textLen, stream, &charsOut);
                    }
                }
#else /* SECUREC_FOR_WCHAR */
                if (formatAttr.bufferIsWide == 0 && textLen > 0) {
                    int count = textLen;
                    char *p = formatBuf.str;

                    while (count > 0) {
                        wchar_t wchar = L'\0';
                        int retVal = mbtowc(&wchar, p, (size_t)MB_CUR_MAX);
                        if (retVal <= 0) {
                            charsOut = -1;
                            break;
                        }
                        SECUREC_WRITE_CHAR(wchar, stream, &charsOut);
                        p += retVal;
                        count -= retVal;
                    }
                } else {
                    if (SECUREC_IS_REST_BUF_ENOUGH(textLen)) {
                        SECUREC_SAFE_WRITE_STR(formatBuf.wStr, textLen, stream, &charsOut); /* char * cast to wchar * */
                    } else {
                        SECUREC_WRITE_STRING(formatBuf.wStr, textLen, stream, &charsOut);
                    }
                }
#endif /* SECUREC_FOR_WCHAR */

                if (charsOut >= 0 && (formatAttr.flags & SECUREC_FLAG_LEFT) && padding > 0) {
                    /* pad on right with blanks */
                    if (SECUREC_IS_REST_BUF_ENOUGH(padding)) {
                        /* char * cast to wchar * */
                        SECUREC_SAFE_PADDING(SECUREC_CHAR(' '), padding, stream, &charsOut);
                    } else {
                        SECUREC_WRITE_MULTI_CHAR(SECUREC_CHAR(' '), padding, stream, &charsOut);
                    }
                }

                /* we're done! */
            }
            if (floatBuf != NULL) {
                SECUREC_FREE(floatBuf);
                floatBuf = NULL;
            }
            break;
        case STAT_INVALID:
            return -1;
        default:
            return -1;          /* input format is wrong, directly return */
        }
    }

    if (state != STAT_NORMAL && state != STAT_TYPE) {
        return -1;
    }

    return charsOut;            /* the number of characters written */
}                               /* arglist must not be declare as const */
#endif /* OUTPUT_P_INL_2B263E9C_43D8_44BB_B17A_6D2033DECEE5 */

