/***********************************************************************
 * Copyright (c) 2008-2080 pepstack.com, 350137278@qq.com
 *
 * ALL RIGHTS RESERVED.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 **********************************************************************/

/**
 * @filename   cstrbuf.h
 *  C String Buffer Functions.
 *
 * @author     Liang Zhang <350137278@qq.com>
 * @version    0.0.10
 * @create     2017-08-28 11:12:10
 * @update     2020-01-12 17:20:46
 */
#ifndef _CSTRBUF_H_
#define _CSTRBUF_H_

#if defined(__cplusplus)
extern "C"
{
#endif

#include "memapi.h"
#include "unitypes.h"


#ifndef cstr_allocate_blocksize
# define cstr_allocate_blocksize    16
#endif

#ifndef cstr_length_maximum
# define cstr_length_maximum        536870911
#endif

#define cstr_bool_true   1
#define cstr_bool_false  0

#define cstr_length(str, maxlen)    ((int)((str)==NULL? 0: ((int)(maxlen)==(-1)? strlen(str) : strnlen(str, (int)(maxlen)))))

#define cstr_copybuf(dstbuf, src)   \
    snprintf_chkd_V1(dstbuf, sizeof(dstbuf), "%.*s", (int)strnlen(src, sizeof(dstbuf) - 1), (const char *)(src))

#define cstr_copybuf_len(dstbuf, src, srclen)   \
    snprintf_chkd_V1(dstbuf, sizeof(dstbuf), "%.*s", (int)(((int)(srclen)) < ((int)sizeof(dstbuf))? (srclen) : (sizeof(dstbuf) - 1)), (const char *)(src))


NOWARNING_UNUSED(static)
void cstr_varray_free (char ** varr, int maxnum)
{
    while (varr && maxnum-- > 0) {
        char *p = varr[maxnum];
        if (p) {
            varr[maxnum] = NULL;
            mem_free(p);
        }
    }
}


NOWARNING_UNUSED(static)
char * cstr_toupper (char * s, int num)
{
    char *p = s;

    while (num-- > 0 && *p) {
       *p = toupper(*p);
        p++;
    }

    return s;
}


NOWARNING_UNUSED(static)
char * cstr_tolower (char * s, int num)
{
    char *p = s;

    while (num-- > 0 && *p) {
       *p = tolower(*p);
        p++;
    }

    return s;
}


/**
 * trim specified character in given string
 */
NOWARNING_UNUSED(static)
char * cstr_trim_chr (char * s, char c)
{
    return (*s==0)?s:(((*s!=c)?(((cstr_trim_chr(s+1,c)-1)==s)?s:(*(cstr_trim_chr(s+1,c)-1)=*s,*s=c,cstr_trim_chr(s+1,c))):cstr_trim_chr(s+1,c)));
}


NOWARNING_UNUSED(static)
char * cstr_trim_chr_mul (char * str, const char * chrs, int num)
{
    char *s = str;
    while (num-- > 0) {
        char ch = chrs[num];
        s = cstr_trim_chr(s, ch);
    }
    return s;
}


NOWARNING_UNUSED(static)
char * cstr_Ltrim_chr (char * str, char ch)
{
    char *p = str;
    while (*p && *p++ == ch) {
        str = p;
    }
    return str;
}


NOWARNING_UNUSED(static)
char* cstr_Rtrim_chr (char * str, char ch, int *outlen)
{
    char *p = str;
    char *q = p;

    while (*p) {
        if (*p != ch) {
            q = p;
        }

        p++;
    }

    if (++q <= p) {
        *q = 0;

        if (outlen) {
            *outlen = (int)(q - str);
        }
    } else {
        if (outlen) {
            *outlen = 0;
        }
    }

    return str;
}


#define cstr_LRtrim_chr(str, c, outlen)  cstr_Rtrim_chr(cstr_Ltrim_chr((str), (c)), (c), (outlen))


NOWARNING_UNUSED(static)
char * cstr_Lfind_chr (char * str, int len, char c)
{
    if (! str || len <= 0) {
        return NULL;
    } else {
        char *p = str;
        while ((int)(p - str) < len && *p) {
            if (*p == c) {
                return p;
            }
            p++;
        }
        return NULL;
    }
}


NOWARNING_UNUSED(static)
char * cstr_Rfind_chr (char * str, int len, char c)
{
    if (! str || len <= 0) {
        return NULL;
    } else {
        char *p = &str[len-1];
        while (len-- > 0) {
            if (*p == c) {
                return p;
            }
            p--;
        }
        return NULL;
    }
}


NOWARNING_UNUSED(static)
char * cstr_find_chrs (char * str, int len, const char *chrs, int nch)
{
    if (! str || len <= 0) {
        return NULL;
    } else {
        int i;
        char *p = str;

        while ((int)(p - str) < len && *p) {
            for (i = 0; i < nch; i++) {
                if (chrs[i] == *p) {
                    return p;
                }
            }

            p++;
        }

        return NULL;
    }
}


/**
 * int isspace(char c);
 *   Standard white-space characters are:
 *    ' '   (0x20)	space (SPC) arcii=32
 *    '\t'	(0x09)	horizontal tab (TAB)
 *    '\n'	(0x0a)	newline (LF)
 *    '\v'	(0x0b)	vertical tab (VT)
 *    '\f'	(0x0c)	feed (FF)
 *    '\r'	(0x0d)	carriage return (CR)
 */
NOWARNING_UNUSED(static)
char * cstr_Ltrim_whitespace (char *str)
{
    char *p = str;
    while (p && isspace((int)*p)) {
        p++;
    }
    return p;
}


NOWARNING_UNUSED(static)
int cstr_Rtrim_whitespace (char *str, int len)
{
    while (len-- > 0) {
        if (isspace((int)str[len])) {
            str[len] = 0;
        } else {
            break;
        }
    }
    return len + 1;
}


NOWARNING_UNUSED(static)
char * cstr_LRtrim_whitespace (char *str)
{
    char *p = cstr_Ltrim_whitespace(str);
    cstr_Rtrim_whitespace(p, cstr_length(p, -1));
    return p;
}


NOWARNING_UNUSED(static)
int cstr_shrink_whitespace (const char *str, int *start, int *end)
{
    int s = *start;
    int e = *end;

    for (; s < *end; s++) {
        if (! isspace((int)str[s])) {
            break;
        }
    }
    *start = s;

    for (; e >= *start; e--) {
        if (! isspace((int)str[e])) {
            break;
        }
    }
    *end = e;

    return (*end - *start + 1);
}


NOWARNING_UNUSED(static)
char * cstr_trim_whitespace (char * s)
{
    return (*s==0)?s:((( ! isspace((int)*s) )?(((cstr_trim_whitespace(s+1)-1)==s)? s : (*(cstr_trim_whitespace(s+1)-1)=*s, *s=32 ,cstr_trim_whitespace(s+1))):cstr_trim_whitespace(s+1)));
}


NOWARNING_UNUSED(static)
char * cstr_replace_chr (char * str, char ch, char rpl)
{
    char *p = str;
    while (p && *p) {
        if (*p == ch) {
            *p = rpl;
        }
        p++;
    }
    return str;
}


NOWARNING_UNUSED(static)
int cstr_slpit_chr (const char * str, int len, char delim, char **outstrs, int outstrslen[], int maxoutnum)
{
    char *p;
    const char *s = str;

    int outlen;
    int i = 0;

    int n = 1;
    while (s && (p = (char *)strchr(s, delim)) && (p < str +len)) {
        s = p+1;

        n++;
    }

    if (! outstrs) {
        // only to get count
        return n;
    }

    if (n > 0) {
        char *sb;

        char *s0 = (char*) mem_alloc_unset(len + 1);

        memcpy(s0, str, len);

        s0[len] = 0;

        sb = s0;
        while (sb && (p = strchr(sb, delim))) {
            *p++ = 0;

            if (i < maxoutnum) {
                // remove whitespaces
                outlen = 0;
                outstrs[i] = mem_strdup( cstr_LRtrim_chr(sb, 32, &outlen) );
                if (outstrslen) {
                    outstrslen[i] = outlen;
                }
                i++;
            } else {
                // overflow than maxoutnum
                break;
            }

            sb = (char *) p;
        }

        if (i < maxoutnum) {
            outlen = 0;
            outstrs[i] = mem_strdup( cstr_LRtrim_chr(sb, 32, &outlen) );
            if (outstrslen) {
                outstrslen[i] = outlen;
            }
            i++;
        }

        mem_free(s0);
    }

    return i;
}


NOWARNING_UNUSED(static)
int cstr_replace_new (const char *original, const char *pattern, const char *replacement, char **outresult)
{
    size_t const replen = strlen(replacement);
    size_t const patlen = strlen(pattern);
    size_t const orilen = strlen(original);

    size_t patcnt = 0;

    const char * oriptr;
    const char * patloc;

    *outresult = 0;

    // find how many times the pattern occurs in the original string
    for ((oriptr = original); (patloc = strstr(oriptr, pattern)); (oriptr = patloc + patlen)) {
        patcnt++;
    }

    if (patcnt) {
        // allocate memory for the new string
        size_t len = orilen + patcnt * (replen - patlen);

        char * result = (char *) mem_alloc_unset( sizeof(char) * (len + 1) );

        // copy the original string,
        // replacing all the instances of the pattern
        char * retptr = result;

        for ((oriptr = original); (patloc = strstr(oriptr, pattern)); (oriptr = patloc + patlen)) {
            size_t const skplen = patloc - oriptr;

            // copy the section until the occurence of the pattern
            strncpy(retptr, oriptr, skplen);
            retptr += skplen;

            // copy the replacement
            strncpy(retptr, replacement, replen);
            retptr += replen;
        }

        // copy the rest of the string.
        strcpy(retptr, oriptr);

        *outresult = result;
        return (int) len;
    }

    return 0;
}


/**
 * true:
 *    "8897391"
 *    "   123.456"
 *    "+123456.123"
 *    "-100  "
 * false:
 *    ""
 *    "   "
 *    "189 0931"
 *    "12.56.89"
 *    "+ 1234.556"
 */
NOWARNING_UNUSED(static)
int cstr_isnumeric (char *numeric, int len)
{
    int i = 0;
    int dots = 0;

    char *str = cstr_LRtrim_whitespace(numeric);
    len = cstr_length(str, len);

    if (len < 1 || len > 40) {
        return cstr_bool_false;
    }

    if (str[0] == '+' || str[0] == '-') {
        i = 1;
    }

    for (; i < len; i++) {
        if (! isdigit((int)str[i])) {
            if (str[i] == '.') {
                dots++;
                if (dots > 1) {
                    return cstr_bool_false;
                }
            } else {
                return cstr_bool_false;
            }
        }
    }

    return cstr_bool_true;
}


NOWARNING_UNUSED(static)
int cstr_to_sb8 (int base, const char *str, size_t slen, sb8 *outval)
{
    if (!str || slen == 0) {
        // null string as 0
        *outval = 0;
        return 0;
    } else {
        sb8 val;
        char *endptr;

        /* To distinguish success/failure after call */
        errno = 0;

        if (slen == -1 || str[slen] == '\0') {
            val = strtoll(str, &endptr, base);

            if (val == 0 && endptr == str) {
                // error: no qualified digits were found
                *outval = val;
                return (-1);
            }
        } else {
            char* sbuf = (char *) alloca(slen + 1);
            memcpy(sbuf, str, slen);
            sbuf[slen] = '\0';
            val = strtoll(sbuf, &endptr, base);

            if (val == 0 && endptr == sbuf) {
                // error: no qualified digits were found
                *outval = val;
                return (-1);
            }
        }

        /* Check for various possible errors */
        if (errno) {
            // error occurred
            *outval = val;
            return (-1);
        }

        /* success return */
        *outval = val;
        return 1;
    }
}


NOWARNING_UNUSED(static)
int cstr_to_ub8 (int base, const char *str, size_t slen, ub8 *outval)
{
    if (!str || slen == 0) {
        // null string as 0
        *outval = 0;
        return 0;
    } else {
        ub8 val;
        char *endptr;

        /* To distinguish success/failure after call */
        errno = 0;

        if (slen == -1 || str[slen] == '\0') {
            val = strtoull(str, &endptr, base);

            if (val == 0 && endptr == str) {
                // error: no qualified digits were found
                *outval = val;
                return (-1);
            }
        } else {
            char* sbuf = (char *) alloca(slen + 1);
            memcpy(sbuf, str, slen);
            sbuf[slen] = '\0';
            val = strtoull(sbuf, &endptr, base);

            if (val == 0 && endptr == sbuf) {
                // error: no qualified digits were found
                *outval = val;
                return (-1);
            }
        }

        /* Check for various possible errors */
        if (errno) {
            // error occurred
            *outval = val;
            return (-1);
        }

        /* success return */
        *outval = val;
        return 1;
    }
}


NOWARNING_UNUSED(static)
int cstr_to_dbl (const char *str, size_t slen, double *outval)
{
    if (!str || slen == 0) {
        // null string as 0
        *outval = 0;
        return 0;
    } else {
        double val;
        char *endptr;

        /* To distinguish success/failure after call */
        errno = 0;

        if (slen == -1 || str[slen] == '\0') {
            val = strtod(str, &endptr);

            if (val == 0 && endptr == str) {
                // error: no qualified digits were found
                *outval = val;
                return (-1);
            }
        } else {
            char* sbuf = (char *) alloca(slen + 1);
            memcpy(sbuf, str, slen);
            sbuf[slen] = '\0';
            val = strtod(sbuf, &endptr);

            if (val == 0 && endptr == sbuf) {
                // error: no qualified digits were found
                *outval = val;
                return (-1);
            }
        }

        /* Check for various possible errors */
        if (errno) {
            // error occurred
            *outval = val;
            return (-1);
        }

        /* success return */
        *outval = val;
        return 1;
    }
}


/**
 * cstr_split_substr
 *   split string by separator string (sep) into sub strings
 */
NOWARNING_UNUSED(static)
int cstr_split_substr (char *str, const char *sepstr, int seplen, char **subs, int maxsubs)
{
    int i = 0;

    char *s = str;

    while (s && i < maxsubs) {
        char *p = strstr(s, sepstr);
        if (p) {
            *p = 0;
            p += seplen;
        }

        subs[i++] = s;

        s = p;
    }

    return i;
}


NOWARNING_UNUSED(static)
int cstr_split_multi_chrs (char *str, int slen, const char *sepchrs, int count, char **outsubs, int outsubslen[], int maxsubs)
{
    char *sub;
    int substart, subend, sublen;

    int k;
    int i = 0;
    int len = 0;

    int start = i;
    int end = start;

    for (; i < slen; i++) {
        for (k = 0; k < count; k++) {
            if (str[i] == sepchrs[k]) {
                end = i;
                break;
            }
        }

        if (k < count && end > start && len < maxsubs) {
            substart = start;
            subend = end - 1;

            sublen = cstr_shrink_whitespace(str, &substart, &subend);

            if (sublen > 0) {
                sub = (char *) mem_alloc_unset(sublen + 1);

                memcpy(sub, str + substart, sublen);
                sub[sublen] = 0;

                outsubs[len] = sub;
                outsubslen[len] = sublen;

                if (++len == maxsubs) {
                    return len;
                }
            }

            start = end;
        }
    }

    end = slen;

    if (len < maxsubs && end > start) {
        substart = start;
        subend = end - 1;

        sublen = cstr_shrink_whitespace(str, &substart, &subend);

        if (sublen > 0) {
            sub = (char *) mem_alloc_unset(sublen + 1);

            memcpy(sub, str + substart, sublen);
            sub[sublen] = 0;

            outsubs[len] = sub;
            outsubslen[len] = sublen;

            len++;
        }
    }

    return len;
}


NOWARNING_UNUSED(static)
int cstr_notequal (const char *str1, const char *str2)
{
    if (str1 == str2) {
        return cstr_bool_false;
    }

    if (str1 && str2) {
        // str1 != str2
        return strcmp(str1, str2)? cstr_bool_true : cstr_bool_false;
    }

    // str1 != str2
    return cstr_bool_true;
}


NOWARNING_UNUSED(static)
int cstr_notequal_len (const char *Astr, int Alen, const char *Bstr, int Blen)
{
    if (Alen != Blen) {
        // Astr != Bstr
        return cstr_bool_true;
    }

    if (Astr == Bstr) {
        return cstr_bool_false;
    }

    if (Astr && Bstr) {
        return strncmp(Astr, Bstr, Alen)? cstr_bool_true : cstr_bool_false;
    }

    // not eauql
    return cstr_bool_true;
}


/**
 * cstr_compare_len
 *   Safely compare two strings as strncmp(A, B) do
 *
 * returns:
 *    1: A > B
 *    0: A = B
 *   -1: A < B
 *
 * notes:
 *   1) null string is less than any non-null or empty one.
 *   2) shorter string is less than longer one.
 *   3) two null strings is equal (0 returned).
 */
NOWARNING_UNUSED(static)
int cstr_compare_len (const char *Astr, int Alen, const char *Bstr, int Blen, int caseignore)
{
    if (Astr == Bstr) {
        return 0;
    }

    if (! Astr) {
        // A < B (B is non-null)
        return (-1);
    }

    if (! Bstr) {
        // A > B (B is null)
        return 1;
    }

    if (! Alen && ! Blen) {
        // A and B are all empty
        return 0;
    }

    if (Alen < 0 && Blen < 0) {
        // same as strcmp
        if (caseignore) {
    #ifdef _MSC_VER
        return stricmp(Astr, Bstr);
    #else
        return strcasecmp(Astr, Bstr);
    #endif
        } else {
            return strcmp(Astr, Bstr);
        }
    }

    if (Alen < 0) {
        // get length of Astr
        Alen = cstr_length(Astr, -1);
    }

    if (Blen < 0) {
        // get length of Astr
        Blen = cstr_length(Bstr, -1);
    }

    if (Alen > Blen) {
        return 1;
    }

    if (Alen < Blen) {
        return -1;
    }

    // Alen == Blen
    if (caseignore) {
    #ifdef _MSC_VER
        return strnicmp(Astr, Bstr, Alen);
    #else
        return strncasecmp(Astr, Bstr, Alen);
    #endif
    } else {
        return strncmp(Astr, Bstr, Alen);
    }
}


/**
 * cstr_startwith("HelloWorld", 10, "Hello", 5) == cstr_bool_true
 * cstr_startwith("HelloWorld", 10, "World", 5) == cstr_bool_false
 * cstr_startwith("HelloWorld", 10, "hello", 5) == cstr_bool_false
 */
NOWARNING_UNUSED(static)
int cstr_startwith (const char *str, int count, const char *start, int startlen)
{
    if (str == start) {
        return cstr_bool_true;
    }

    if (str && start && startlen <= count) {
        if (! memcmp(str, start, startlen)) {
            return cstr_bool_true;
        }
    }

    return cstr_bool_false;
}


NOWARNING_UNUSED(static)
int cstr_endwith (const char *str, int count, const char *end, int endlen)
{
    if (str == end) {
        return cstr_bool_true;
    }

    if (str && end && endlen <= count) {
        //   str="aaaaBBBB"
        //   end="aBBBB"
        return ! cstr_notequal_len(&str[count - endlen], endlen, end, endlen);
    }

    return cstr_bool_false;
}


NOWARNING_UNUSED(static)
int cstr_containwith (const char *str, int count, const char *sub, int sublen)
{
    if (str == sub) {
        return 0;
    }

    if (str && sub && sublen <= count) {
        char *at = strstr((char *)str, sub);
        if (! at) {
            return -1;
        }
        return (int)(at - str);
    }

    return -1;
}


NOWARNING_UNUSED(static)
int cstr_startwith_mul (const char *str, int count, const char *starts[], const int *startslen, int startsnum)
{
    while (startsnum-- > 0) {
        const char *s = starts[startsnum];

        if (s) {
            int len = (startslen? startslen[startsnum] : (int) strlen(s));

            if (cstr_startwith(str, count, s, len)) {
                return startsnum;
            }
        }
    }

    return (-1);
}


NOWARNING_UNUSED(static)
int cstr_endwith_mul (const char *str, int count, const char *ends[], const int *endslen, int endsnum)
{
    while (endsnum-- > 0) {
        const char *s = ends[endsnum];

        if (s) {
            int len = (endslen? endslen[endsnum] : (int) strlen(s));

            if (cstr_endwith(str, count, s, len)) {
                return endsnum;
            }
        }
    }

    return (-1);
}


NOWARNING_UNUSED(static)
int cstr_findstr_in (const char *str, int count, const char *dests[], int destsnum, int caseignore)
{
    int i = 0;

    for (; i < destsnum; i++) {
        const char *dest = dests[i];

        if (str == dest) {
            // found
            return i;
        }

        if (dest && str) {
            int len = cstr_length(dest, count + 1);

            if (len == count) {
                if (caseignore) {
    #ifdef _MSC_VER
                    if (! strnicmp(str, dest, count)) {
                        // found
                        return i;
                    }
    #else
                    if (! strncasecmp(str, dest, count)) {
                        // found
                        return i;
                    }
    #endif
                } else {
                    if (! strncmp(str, dest, count)) {
                        // found
                        return i;
                    }
                }
            }
        }
    }

    // not found
    return (-1);
}


NOWARNING_UNUSED(static)
int cstr_isdigit (const char *str, int len)
{
    while(len-- > 0) {
        if (! isdigit((int)str[len])) {
            return cstr_bool_false;
        }
    }

    return cstr_bool_true;
}


NOWARNING_UNUSED(static)
int cstr_safecopy (char *dstbuf, size_t dstbufsize, size_t dstoffset, const char *source, size_t sourcelen)
{
    if (dstoffset + sourcelen < dstbufsize) {
        memcpy(dstbuf + dstoffset, source, sourcelen);
        dstbuf[dstoffset + sourcelen] = '\0';

        /* success full copied */
        return 1;
    }

    if (dstoffset < dstbufsize) {
        size_t cplen = dstbufsize - dstoffset;
        memcpy(dstbuf + dstoffset, source, cplen);

        if (dstbufsize > 3) {
            dstbuf[dstbufsize - 4] = '.';
            dstbuf[dstbufsize - 3] = '.';
            dstbuf[dstbufsize - 2] = '.';
        }
    }

    dstbuf[dstbufsize - 1] = '\0';

    /* error with part copied */
    return 0;
}


NOWARNING_UNUSED(static)
int cstr_readline (FILE *fp, char line[], size_t maxlen, int ignore_whitespace)
{
    int ch, len = 0;

    if (ftell(fp) == 0) {
        int bomhead[3] = {fgetc(fp), fgetc(fp), fgetc(fp)};
        if (bomhead[0] == 0xEF && bomhead[1] == 0xBB && bomhead[2] == 0xBF) {
            fseek(fp, 3, SEEK_SET);
        } else {
            fseek(fp, 0, SEEK_SET);
        }
    }

    while ((ch = fgetc(fp)) != EOF) {
        if ((size_t) len < maxlen) {
            if (ch != '\r' && ch != '\n' && ch != '\\') {
                if (! ignore_whitespace || ! isspace((int)ch)) {
                    line[len++] = ch;
                }
            }
        }

        if (ch == '\n') {
            break;
        }
    }

    if (ch == EOF && len == 0) {
        // end of file
        return -1;
    }

    line[len] = 0;
    return len;
}


#define TM_YEAR_IS_LEAP(year)   ((year) % 4? 0 : ((year) % 100? 1 : ((year) % 400? 0 : 1)))

#define CSTR_DATETIME_SIZE_MAX  30


typedef struct {
    /* date fields: if year = -1, date is invalid */
    int year;
    int month;
    int day;

    /* time fields: if hour = -1, time is invalid */
    int hour;
    int minute;
    int second;

    /* millisecond: if millisecond = -1, millisecond is invalid */
    int millisecond;

    /**
     * -1 : timezone is invalid
     *
     * 800 = +08:00, -800 = W08:00
     */
    int timezone;
} cstr_datetime_t;


NOWARNING_UNUSED(static)
const char * cstr_datetime_print (const cstr_datetime_t *dt, char outfmt[CSTR_DATETIME_SIZE_MAX])
{
    int dlen = 0,
        tlen = 0,
        mlen = 0;

    *outfmt = 0;

    if (dt->year != -1) {
        dlen = snprintf_chkd_V1(outfmt, CSTR_DATETIME_SIZE_MAX, "%04d-%02d-%02d", dt->year, dt->month, dt->day);
    }

    if (dt->hour != -1) {
        if (dlen) {
            outfmt[dlen++] = 32;
        }

        tlen = snprintf_chkd_V1(outfmt + dlen, CSTR_DATETIME_SIZE_MAX - dlen, "%02d:%02d:%02d", dt->hour, dt->minute, dt->second);
    }

    if (dt->millisecond != -1) {
        mlen = snprintf_chkd_V1(outfmt + dlen + tlen, CSTR_DATETIME_SIZE_MAX - dlen - tlen, ".%03d", dt->millisecond);
    }

    if (dt->timezone != -1) {
        if (dt->timezone < 0) {
            snprintf_chkd_V1(outfmt + dlen + tlen + mlen, CSTR_DATETIME_SIZE_MAX - dlen - tlen - mlen, "W%02d:%02d",
                -dt->timezone / 100, -dt->timezone % 100);
        } else {
            snprintf_chkd_V1(outfmt + dlen + tlen + mlen, CSTR_DATETIME_SIZE_MAX - dlen - tlen - mlen, "+%02d:%02d",
                dt->timezone / 100, dt->timezone % 100);
        }
    }

    return outfmt;
}


/**
 * time_is_valid()
 *
 * test time is valid.
 *
 * remark:
 *
 *  The number of seconds(sec) after the minute, normally in the range 0 to 59,
 *   but can be up to 60 to allow for leap.
 *  Deciding when to introduce a leap second is the responsibility of the
 *   international earth rotation and reference systems service.
 *
 * returns:
 *   0: error
 *   1: ok
 */
NOWARNING_UNUSED(static)
int time_is_valid (int year, int mon, int day, int hour, int min, int sec)
{
    if (year < 1900 || year > 9999 || mon <= 0 || mon > 12 || day <= 0 || day > 31 || hour < 0 || hour > 24 || min < 0 || min > 59 || sec < 0 || sec > 60) {
        return 0;
    }

    if (mon == 1 || mon == 3 || mon == 5 || mon == 7 || mon == 8 || mon == 10 || mon == 12) {
        // 31 days ok
        return 1;
    } else if (mon == 4 || mon == 6 || mon == 9 || mon == 11) {
        if (day < 31) {
            return 1;
        }
    } else {
        // mon=2
        return (day > 29? 0 : (day < 29? 1: (TM_YEAR_IS_LEAP(year)? 1 : 0)));
    }
    return 0;
}


NOWARNING_UNUSED(static)
int cstr_parse_timezone (char *tz)
{
    int sign = 0;

    /* +08:00/E0800; W08:00 */
    if (*tz == '+' || *tz == 'E') {
        sign = 1;
    } else if (*tz == '-' || *tz == 'W') {
        sign = -1;
    }

    if (!sign) {
        return -1;
    }

    if (isdigit((int)tz[1]) && isdigit((int)tz[2]) && (tz[3]==':' || tz[3]=='.') &&
        isdigit((int)tz[4]) && isdigit((int)tz[5]) && tz[6]=='\0') {
        char szhour[3] = {tz[1], tz[2], '\0'};
        char szmin[3] = {tz[4], tz[5], '\0'};

        int h = atoi(szhour);
        int m = atoi(szmin);

        if (h > 23) {
            return -1;
        }
        if (m > 59) {
            return -1;
        }
        return (h * 100 + m) * sign;
    }

    return -1;
}


NOWARNING_UNUSED(static)
ub8 cstr_parse_timestamp (char *timestr, cstr_datetime_t *outdt)
{
    int len = 0;
    int YMD = 0, HMS = 0, MMM = 0;

    /**
     * UTC+08:00 = +0800 or E0800
     * UTC-11:00 =        W0800
     *
     * '2019-12-22 12:36:59.065+08:00' or '2019-12-22 12:36:59.065E08:00'
     * '2019-12-22 12:36:59.065'
     * '2019-12-22 12:36:59W08:00' or '2019-12-22 12:36:59-08:00'
     * '2019-12-22 12:36:59'
     * '2019-12-22E08:00' or '2019-12-22+08:00'
     * '2019-12-22'
     */
    char Year[5] = {'0', '0', '0', '0', '\0'};
    char Mon[3] = {'0', '0', '\0'};
    char Day[3] = {'0', '0', '\0'};

    char hour[3] = {'0', '0', '\0'};
    char min[3] = {'0', '0', '\0'};
    char sec[3] = {'0', '0', '\0'};

    char msec[4] = {'0', '0', '0', '\0'};

    int timezone = -1;

    char *str, *a, *b, *hms, *tz;

    tz = strchr(timestr, '+');
    if (!tz) {
        tz = strchr(timestr, 'E');
        if (!tz) {
            tz = strchr(timestr, 'W');
            if (!tz) {
                // -08:00 or -08.00
                tz = strrchr(timestr, '-');
                if (tz) {
                    if (tz[1] && tz[2] && (tz[3] == ':' || tz[3] == '.')) {
                        // has timezone
                        // tz;
                    } else {
                        // no timezone
                        tz = 0;
                    }
                }
            }
        }
    }

    if (tz) {
        timezone = cstr_parse_timezone(tz);
        if (timezone == -1) {
            // error timezone
            return (-1);
        }
        *tz = 0;
    }

    str = cstr_LRtrim_whitespace(timestr);
    a = strchr(str, 39);
    b = strrchr(str, 39);

    if (a && b) {
        *a++ = 0;
        *b-- = 0;
        str = a;
        len = (int)(b - a) + 1;
    } else if (a || b) {
        // error char
        return (-1);
    } else {
        len = cstr_length(str, 30);
    }

    if (len == 10) {
        /* 2019-12-22 */
        YMD = 1;

        a = strchr(str, '-');
        b = strrchr(str, '-');

        if (a && b && a - str == 4 && b-a == 3) {
            *a++ = 0;
            *b++ = 0;

            snprintf_chkd_V1(Year, sizeof(Year), "%.*s", 4, str);
            snprintf_chkd_V1(Mon, sizeof(Mon), "%.*s", 2, a);
            snprintf_chkd_V1(Day, sizeof(Day), "%.*s", 2, b);
        } else {
            // error date format
            return (-1);
        }
    } else if (len == 19) {
        /* 2019-12-22 12:36:59 */
        YMD = 1;
        HMS = 1;

        a = strchr(str, 32);
        if (a && a - str == 10) {
            *a++ = 0;

            hms = a;

            a = strchr(str, '-');
            b = strrchr(str, '-');

            if (a && b && a - str == 4 && b-a == 3) {
                *a++ = 0;
                *b++ = 0;

                snprintf_chkd_V1(Year, sizeof(Year), "%.*s", 4, str);
                snprintf_chkd_V1(Mon, sizeof(Mon), "%.*s", 2, a);
                snprintf_chkd_V1(Day, sizeof(Day), "%.*s", 2, b);
            } else {
                // error date format
                return (-1);
            }

            a = strchr(hms, ':');
            b = strrchr(hms, ':');

            if (a && b && a - hms == 2 && b-a == 3) {
                *a++ = 0;
                *b++ = 0;

                snprintf_chkd_V1(hour, sizeof(hour), "%.*s", 2, hms);
                snprintf_chkd_V1(min, sizeof(min), "%.*s", 2, a);
                snprintf_chkd_V1(sec, sizeof(sec), "%.*s", 2, b);
            } else {
                // error date format
                return (-1);
            }
        } else {
            // error datetime format
            return (-1);
        }
    } else if (len == 23) {
        /* 2019-12-22 12:36:59.065 */
        YMD = 1;
        HMS = 1;
        MMM = 1;

        a = strchr(str, 32);
        if (a && a - str == 10) {
            *a++ = 0;

            hms = a;

            a = strchr(str, '-');
            b = strrchr(str, '-');

            if (a && b && a - str == 4 && b-a == 3) {
                *a++ = 0;
                *b++ = 0;

                snprintf_chkd_V1(Year, sizeof(Year), "%.*s", 4, str);
                snprintf_chkd_V1(Mon, sizeof(Mon), "%.*s", 2, a);
                snprintf_chkd_V1(Day, sizeof(Day), "%.*s", 2, b);
            } else {
                // error date format
                return (-1);
            }

            a = strchr(hms, ':');
            b = strrchr(hms, ':');

            if (a && b && a - hms == 2 && b-a == 3) {
                char *dot = strchr(b, '.');
                if (!dot || dot - b != 3) {
                    // error stamp format
                    return (-1);
                }

                *a++ = 0;
                *b++ = 0;
                *dot++ = 0;

                snprintf_chkd_V1(hour, sizeof(hour), "%.*s", 2, hms);
                snprintf_chkd_V1(min, sizeof(min), "%.*s", 2, a);
                snprintf_chkd_V1(sec, sizeof(sec), "%.*s", 2, b);

                if (*dot) {
                    msec[0] = *dot++;

                    if (*dot) {
                        msec[1] = *dot++;

                        if (*dot) {
                            msec[2] = *dot++;
                        }
                    }
                }
            } else {
                // error date format
                return (-1);
            }
        } else {
            // error datetime format
            return (-1);
        }
    } else {
        // error format
        return (-1);
    }

    if (! YMD) {
        // no year
        return (-1);
    }

    if (cstr_isdigit(Year, 4) &&
        cstr_isdigit(Mon, 2) &&
        cstr_isdigit(Day, 2) &&
        cstr_isdigit(hour, 2) &&
        cstr_isdigit(min, 2) &&
        cstr_isdigit(sec, 2) &&
        cstr_isdigit(msec, 3)) {
        ub8 tsec;
        struct tm t = {0};

        t.tm_year = atoi(Year);
        t.tm_mon = atoi(Mon);
        t.tm_mday = atoi(Day);

        t.tm_hour = atoi(hour);
        t.tm_min = atoi(min);
        t.tm_sec = atoi(sec);

        if (! time_is_valid(t.tm_year, t.tm_mon, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec)) {
            // invalid time
            return (-1);
        }

        t.tm_year -= 1900;
        t.tm_mon  -= 1;

        // since 1970-01-01 UTChh:00:00 (china: hh=8)
        tsec = (ub8) mktime(&t);
        if (tsec == (ub8)(-1)) {
            return (-1);
        }

        if (outdt) {
            outdt->year = atoi(Year);
            outdt->month = atoi(Mon);
            outdt->day = atoi(Day);

            outdt->hour = atoi(hour);
            outdt->minute = atoi(min);
            outdt->second = atoi(sec);
            outdt->millisecond = atoi(msec);

            outdt->timezone = timezone;
        }

        if (! HMS) {
            outdt->hour = -1;
            outdt->minute = 0;
            outdt->second = 0;
        }
        if (! MMM) {
            outdt->millisecond = -1;
        }

        tsec *= 1000;
        return (ub8)(tsec + atoi(msec));
    }

    // error no digit
    return (-1);
}


NOWARNING_UNUSED(static)
const char * cstr_timestamp_to_datetime (char *stampms, int mslen, char timestr[24])
{
    ub8 stamp = 0;
    time_t tsec;
    int msec;
    struct tm t;

    if (mslen == -1) {
        mslen = cstr_length(stampms, 20);
    }

    if (! cstr_to_ub8(10, stampms, mslen, &stamp)) {
        // error stamp
        return NULL;
    }

    tsec = (time_t)(stamp / 1000);
    msec = (int)(stamp % 1000);

#ifdef _WIN32
    if (!localtime_s(&t, (const time_t*) &tsec)) {
        // error localtime
        return NULL;
    }
#else
    if (!localtime_r((const time_t*)&tsec, &t)) {
        // error localtime
        return NULL;
    }
#endif

    t.tm_year += 1900;
    t.tm_mon  += 1;

    if (! time_is_valid(t.tm_year, t.tm_mon, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec)) {
        // invalid time
        return NULL;
    }

    if (msec) {
        /* 2012-12-22 17:45:59.875 */
        snprintf_chkd_V1(timestr, 24, "%04d-%02d-%02d %02d:%02d:%02d.%03d", t.tm_year, t.tm_mon, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec, msec);
    } else {
        snprintf_chkd_V1(timestr, 20, "%04d-%02d-%02d %02d:%02d:%02d", t.tm_year, t.tm_mon, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);
    }

    return timestr;
}


/**********************************************************************
 * cstrbuf
 *   C String Buffer Api
 *********************************************************************/
#define cstrbuf_error_size_len  ((ub4)(-1))
#define cstrbuf_len_max         134217727   /* max size is 128 MB */
#define cstrbuf_size_max        (cstrbuf_len_max + 1)

#define cstrbuf_alignsize(len)  ((ub4) (((len + cstr_allocate_blocksize) / cstr_allocate_blocksize)  * cstr_allocate_blocksize))

#define cstrbufGetLen(s)        ((s)? (int)(s)->len : 0)
#define cstrbufGetMaxsz(s)      ((s)? (int)(s)->maxsz : 0)
#define cstrbufGetStr(s)        ((s)? (s)->str : 0)

#define cstrbufPrint(s)         printf("%.*s", cstrbufGetLen(s), cstrbufGetStr(s))

#define cstrbufCharAt(s, i)     ((char *)(s->str+(i)))


typedef struct _cstrbuf_t
{
    union {
        ub8 __maxszlen;

        struct {
            ub4 maxsz;
            ub4 len;
        };
    };

    char str[0];
} cstrbuf_t, *cstrbuf;


NOWARNING_UNUSED(static)
char * cstrbufCharAtSafe(cstrbuf csb, int at)
{
    int len = cstrbufGetLen(csb);
    if (at >= 0 && at < len) {
        return cstrbufCharAt(csb, at);
    }

    /* invalid cstrbuf or index */
    return NULL;
}


NOWARNING_UNUSED(static)
cstrbuf cstrbufNew (ub4 maxsz, const char *str, ub4 len)
{
    cstrbuf_t *csb;

    if (len == cstrbuf_error_size_len) {
        len = (ub4) cstr_length(str, cstr_length_maximum);
    }

    if (! maxsz) {
        maxsz = len + 1;
    } else if (maxsz == cstrbuf_error_size_len || maxsz <= len) {
        maxsz = cstrbuf_alignsize(len + 1);
    } else {
        /* maxsz > len */
        maxsz = cstrbuf_alignsize(maxsz);
    }

    csb = (cstrbuf_t *) mem_alloc_unset(sizeof(*csb) + maxsz);

    csb->__maxszlen = 0;
    *csb->str = 0;

    if (str) {
        memcpy(csb->str, str, len);
        memset(csb->str + len, 0, maxsz - len);
        csb->len = len;
    } else {
        memset(csb->str, 0, maxsz);
        csb->len = 0;
    }

    csb->maxsz = maxsz;
    return csb;
}


NOWARNING_UNUSED(static)
void cstrbufFree (cstrbuf *csb)
{
    if (csb) {
        cstrbuf s = *csb;
        if (s) {
            *csb = 0;
            mem_free(s);
        }
    }
}


/* NOTE: dstbuf must have enough space for copy to */
NOWARNING_UNUSED(static)
ub4 cstrbufCopyTo (const cstrbuf src, void *dstbuf, ub4 offcb)
{
    if (src && src->len) {
        memcpy((char*)dstbuf + offcb, src->str, src->len);
        return (ub4)(offcb + src->len);
    }
    return (ub4)offcb;
}


NOWARNING_UNUSED(static)
cstrbuf cstrbufCat (cstrbuf dst, const char *fmt, ...)
{
    int vlen, len;

    if (! dst) {
        do {
            va_list args;
            va_start(args, fmt);
            vlen = vsnprintf(0, 0, fmt, args);
            va_end(args);
        } while(0);

        if (vlen < 0) {
            printf("(%s:%d) vsnprintf error: %s\n", __FILE__, __LINE__, strerror(errno));
            return 0;
        }

        dst = cstrbufNew(vlen + 1, 0, 0);

        do {
            va_list args;
            va_start(args, fmt);
            len = vsnprintf(dst->str, dst->maxsz, fmt, args);
            va_end(args);
        } while(0);

        if (len == vlen) {
            dst->len = len;
            dst->str[dst->len] = 0;
            return dst;
        }

        printf("(%s:%d) vsnprintf error: %s\n", __FILE__, __LINE__, strerror(errno));
        cstrbufFree(&dst);
        return 0;
    } else {
        do {
            va_list args;
            va_start(args, fmt);
            vlen = vsnprintf(&dst->str[dst->len], dst->maxsz - dst->len, fmt, args);
            va_end(args);
        } while(0);

        if (vlen < 0) {
            printf("(%s:%d) vsnprintf error: %s\n", __FILE__, __LINE__, strerror(errno));
            dst->str[dst->len] = 0;
            return dst;
        }

        if (dst->len + vlen < dst->maxsz) {
            dst->len += vlen;
            dst->str[dst->len] = 0;
            return dst;
        } else {
            ub4 maxsz = cstrbuf_alignsize(dst->len + vlen + 1);
            cstrbuf_t *newdst = (cstrbuf_t *) mem_realloc(dst, sizeof(*newdst) + maxsz);
            if (! newdst) {
                printf("(%s:%d) Cannot allocate memory.\n", __FILE__, __LINE__);
                exit(EXIT_FAILURE);
            }
            newdst->maxsz = maxsz;

            do {
                va_list args;
                va_start(args, fmt);
                len = vsnprintf(&newdst->str[newdst->len], newdst->maxsz - newdst->len, fmt, args);
                va_end(args);
            } while(0);

            if (len == vlen) {
                newdst->len += len;
            }

            newdst->str[newdst->len] = 0;
            return newdst;
        }
    }
}


NOWARNING_UNUSED(static)
cstrbuf cstrbufTrunc (cstrbuf s, ub4 len)
{
    if (s->len > len) {
        s->len = len;
        s->str[s->len] = 0;
    }
    return s;
}


NOWARNING_UNUSED(static)
cstrbuf cstrbufConcat (const cstrbuf start, ...)
{
    va_list argp;
    cstrbuf src = start;
    va_start(argp, start);
    cstrbuf result = 0;

    do {
        result = cstrbufCat(result, "%.*s", cstrbufGetLen(src), cstrbufGetStr(src));
        src = va_arg(argp, cstrbuf);
    } while(src);

    va_end(argp);
    return result;
}


NOWARNING_UNUSED(static)
cstrbuf cstrbufDup (cstrbuf dst, const char *source, ub4 sourcelen)
{
    if (! dst) {
        return cstrbufNew(0, source, sourcelen);
    }

    if (sourcelen == cstrbuf_error_size_len) {
        sourcelen = (ub4) cstr_length(source, -1);
    }

    if (! sourcelen) {
        dst->len = 0;
        dst->str[dst->len] = 0;
        return dst;
    }

    if (dst->maxsz > sourcelen) {
        memcpy(dst->str, source, sourcelen);

        dst->len = sourcelen;
        dst->str[dst->len] = 0;
        return dst;
    }

    mem_free(dst);
    return cstrbufNew(0, source, sourcelen);
}


NOWARNING_UNUSED(static)
cstrbuf cstrbufSub (const char *source, int startindex, int length, const char *substr, int sublen)
{
    int sourlen = cstr_length(source, -1);

    cstrbuf endstr = cstrbufNew(0, source + startindex + length, sourlen - startindex - length);
    cstrbuf result = cstrbufNew(sourlen - length + sublen + 1, source, startindex);

    memcpy(result->str + startindex, substr, sublen);
    memcpy(result->str + startindex + sublen, endstr->str, endstr->len);

    cstrbufFree(&endstr);

    result->len = sourlen - length + sublen;
    result->str[result->len] = 0;

    return result;
}


NOWARNING_UNUSED(static)
cstrbuf cstrbufPushBack (cstrbuf dst, char ch)
{
    if (dst) {
        if (dst->len + 1 < dst->maxsz) {
            dst->str[dst->len++] = ch;
        } else {
            dst = cstrbufCat(dst, "%c", ch);
        }
    } else {
        dst = cstrbufCat(NULL, "%c", ch);
    }
    return dst;
}

#ifdef __cplusplus
}
#endif
#endif /* _CSTRBUF_H_ */
