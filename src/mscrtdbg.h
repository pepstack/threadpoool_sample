/***********************************************************************
 * Copyright (c) 2008-2080, (39b63d001409d3682cc901cc838fc1ac40024d40)
 * (cat COPYRIGHT | shasum) ALL RIGHTS RESERVED.
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
 * @filename   mscrtdbg.h
 *     MSVC _CRTDBG_MAP_ALLOC Marco definition.
 *     The first file you should included
 *
 * @author     350137278@qq.com
 * @version    0.0.1
 * @create     2020-06-12 21:12:22
 * @update     2020-07-09 17:38:42
 */
#ifndef MSCRTDBG_H_
#define MSCRTDBG_H_

#if defined(__cplusplus)
extern "C" {
#endif

#if defined(_MSC_VER)
    // warning C4996: 'vsnprintf': This function or variable may be unsafe.
    // Consider using vsnprintf_s instead.
    //  To disable deprecation, use _CRT_SECURE_NO_WARNINGS
    # pragma warning(disable:4996)

    # if defined(_DEBUG)
        /** memory leak auto-detect in MSVC
         * https://blog.csdn.net/lyc201219/article/details/62219503
         */
        # ifndef _CRTDBG_MAP_ALLOC
            # define _CRTDBG_MAP_ALLOC
        # endif

        # include <stdlib.h>
        # include <malloc.h>
        # include <crtdbg.h>

        # ifndef WINDOWS_CRTDBG_ON
            # define WINDOWS_CRTDBG_ON  _CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);
        # endif
    # else
        # ifdef _CRTDBG_MAP_ALLOC
            # undef _CRTDBG_MAP_ALLOC
        # endif

        # include <stdlib.h>
        # include <malloc.h>

        # ifndef WINDOWS_CRTDBG_ON
            # define WINDOWS_CRTDBG_ON
        # endif
    # endif
#endif

#if defined(__cplusplus)
}
#endif

#endif /* MSCRTDBG_H_ */