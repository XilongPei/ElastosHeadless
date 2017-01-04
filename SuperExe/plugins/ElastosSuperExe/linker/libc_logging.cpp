/*
 * Copyright (C) 2010 The Android Open Source Project
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef _LIBC_LOGGING_H
#define _LIBC_LOGGING_H

#include <sys/cdefs.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

__BEGIN_DECLS

#include "private/libc_events.h"
#include "private/libc_logging.h"

//
// Formats a message to the log (priority 'fatal'), then aborts.
//

void __libc_fatal(const char* format, ...) {
  int ret;
  va_list ap;

  va_start(ap, format);
  ret = vfprintf(stdout, format, ap);
  va_end(ap);
//  return (ret);
}

//
// Formats a message to the log (priority 'fatal'), but doesn't abort.
// Used by the malloc implementation to ensure that debuggerd dumps memory
// around the bad address.
//

void __libc_fatal_no_abort(const char* format, ...) {
  int ret;
  va_list ap;

  va_start(ap, format);
  ret = vfprintf(stdout, format, ap);
  va_end(ap);
//  return (ret);
}

//
// Formatting routines for the C library's internal debugging.
// Unlike the usual alternatives, these don't allocate, and they don't drag in all of stdio.
//

int __libc_format_buffer(char* buffer, size_t buffer_size, const char* format, ...) {
  int ret;
  va_list ap;

  va_start(ap, format);
  ret = snprintf(buffer, buffer_size, format, ap);
  va_end(ap);
  return (ret);
}

int __libc_format_fd(int fd, const char* format, ...) {
  int ret;
  va_list ap;

  va_start(ap, format);
  ret = printf(format, ap);
  va_end(ap);
  return (ret);
}

int __libc_format_log(int priority, const char* tag, const char* format, ...) {
  int ret;
  va_list ap;

  va_start(ap, format);
  printf("TAG:%s", tag);
  ret = printf(format, ap);
  va_end(ap);
  return (ret);
}

__END_DECLS

#endif
