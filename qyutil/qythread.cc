/*
 *  Copyright (c) 2011 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "qythread.h"

#if defined(_WIN32)
#include "qythread_win.h"
#else
#include "qythread_posix.h"
#endif

DEFINE_NAMESPACE(qy)

QyThread* QyThread::createThread(QyThreadRunFunction func,
                                 QyThreadObj obj, eThreadPriority prio,
                                 const char* thread_name) {
#if defined(_WIN32)
    return new QyThreadWindow(func, obj, prio, thread_name);
#else
    return QyThreadPosix::create(func, obj, prio, thread_name);
#endif
}

END_NAMESPACE(qy)