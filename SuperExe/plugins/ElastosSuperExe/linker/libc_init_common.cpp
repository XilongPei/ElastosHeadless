/*
 * Copyright (C) 2008 The Android Open Source Project
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

#include "bionic/libc_init_common.h"

#include <elf.h>
#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/auxv.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>

#include "private/bionic_auxv.h"
#include "private/bionic_ssp.h"
#include "private/bionic_tls.h"
#include "private/KernelArgumentBlock.h"
#include "bionic/pthread_internal.h"
#include "asm-generic/mman-common.h"
#include "sys/mman.h"

extern abort_msg_t** __abort_message_ptr;
//extern "C" int __system_properties_init(void);
extern "C" int __set_tls(void* ptr);
extern "C" int __set_tid_address(int* tid_address);

#if 0
// This code is used both by each new pthread and the code that initializes the main thread.
void __init_tls(pthread_internal_t* thread) {
  if (thread->user_allocated_stack()) {
    // We don't know where the user got their stack, so assume the worst and zero the TLS area.
    memset(&thread->tls[0], 0, BIONIC_TLS_SLOTS * sizeof(void*));
  }

  // Slot 0 must point to itself. The x86 Linux kernel reads the TLS from %fs:0.
  thread->tls[TLS_SLOT_SELF] = thread->tls;
  thread->tls[TLS_SLOT_THREAD_ID] = thread;
  // GCC looks in the TLS for the stack guard on x86, so copy it there from our global.
  thread->tls[TLS_SLOT_STACK_GUARD] = (void*) __stack_chk_guard;
}

int __init_thread(pthread_internal_t* thread, bool add_to_thread_list) {
  int error = 0;

  // Set the scheduling policy/priority of the thread.
  if (thread->attr.sched_policy != SCHED_NORMAL) {
    sched_param param;
    param.sched_priority = thread->attr.sched_priority;
    if (sched_setscheduler(thread->tid, thread->attr.sched_policy, &param) == -1) {
#if __LP64__
      // For backwards compatibility reasons, we only report failures on 64-bit devices.
      error = errno;
#endif
//      __libc_format_log(ANDROID_LOG_WARN, "libc",
//                        "pthread_create sched_setscheduler call failed: %s", strerror(errno));
    }
  }

  thread->cleanup_stack = NULL;

  if (add_to_thread_list) {
    _pthread_internal_add(thread);
  }

  return error;
}

ElfW(auxv_t)* __libc_auxv = NULL;
#endif

void __libc_init_vdso();

// Not public, but well-known in the BSDs.
const char* __progname;

// Declared in <unistd.h>.
char** environ;

// Declared in "private/bionic_ssp.h".
uintptr_t __stack_chk_guard = 0;

/* Init TLS for the initial thread. Called by the linker _before_ libc is mapped
 * in memory. Beware: all writes to libc globals from this function will
 * apply to linker-private copies and will not be visible from libc later on.
 *
 * Note: this function creates a pthread_internal_t for the initial thread and
 * stores the pointer in TLS, but does not add it to pthread's thread list. This
 * has to be done later from libc itself (see __libc_init_common).
 *
 * This function also stores a pointer to the kernel argument block in a TLS slot to be
 * picked up by the libc constructor.
 */
void __libc_init_tls(KernelArgumentBlock& args) {
  __libc_auxv = args.auxv;

  static void* tls[BIONIC_TLS_SLOTS];
  static pthread_internal_t main_thread;
  main_thread.tls = tls;

  // Tell the kernel to clear our tid field when we exit, so we're like any other pthread.
  // As a side-effect, this tells us our pid (which is the same as the main thread's tid).
  main_thread.tid = __set_tid_address(&main_thread.tid);
//  main_thread.set_cached_pid(main_thread.tid);

  // We don't want to free the main thread's stack even when the main thread exits
  // because things like environment variables with global scope live on it.
  // We also can't free the pthread_internal_t itself, since that lives on the main
  // thread's stack rather than on the heap.
  pthread_attr_init(&main_thread.attr);
  main_thread.attr.flags = PTHREAD_ATTR_FLAG_USER_ALLOCATED_STACK | PTHREAD_ATTR_FLAG_MAIN_THREAD;
  main_thread.attr.guard_size = 0; // The main thread has no guard page.
  main_thread.attr.stack_size = 0; // User code should never see this; we'll compute it when asked.
  // TODO: the main thread's sched_policy and sched_priority need to be queried.

  __init_thread(&main_thread, false);
  __init_tls(&main_thread);
//  __set_tls(main_thread.tls);
  tls[TLS_SLOT_BIONIC_PREINIT] = &args;

  __init_alternate_signal_stack(&main_thread);
}

#if 0
void __init_alternate_signal_stack(pthread_internal_t* thread) {
  // Create and set an alternate signal stack.
  stack_t ss;
  ss.ss_sp = mmap(NULL, SIGSTKSZ, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
  if (ss.ss_sp != MAP_FAILED) {
    ss.ss_size = SIGSTKSZ;
    ss.ss_flags = 0;
    sigaltstack(&ss, NULL);
    thread->alternate_signal_stack = ss.ss_sp;
  }
}
#endif

void __libc_init_common(KernelArgumentBlock& args) {
  // Initialize various globals.
  environ = args.envp;
  errno = 0;
  __libc_auxv = args.auxv;
  __progname = args.argv[0] ? args.argv[0] : "<unknown>";
  __abort_message_ptr = args.abort_message_ptr;

  // AT_RANDOM is a pointer to 16 bytes of randomness on the stack.
  __stack_chk_guard = *reinterpret_cast<uintptr_t*>(getauxval(AT_RANDOM));

  // Get the main thread from TLS and add it to the thread list.
  pthread_internal_t* main_thread = __get_thread();
  _pthread_internal_add(main_thread);

  //__system_properties_init(); // Requires 'environ'.

  __libc_init_vdso();
}

/* This function will be called during normal program termination
 * to run the destructors that are listed in the .fini_array section
 * of the executable, if any.
 *
 * 'fini_array' points to a list of function addresses. The first
 * entry in the list has value -1, the last one has value 0.
 */
void __libc_fini(void* array) {
  void** fini_array = reinterpret_cast<void**>(array);
  const size_t minus1 = ~(size_t)0; /* ensure proper sign extension */

  /* Sanity check - first entry must be -1 */
  if (array == NULL || (size_t)fini_array[0] != minus1) {
    return;
  }

  /* skip over it */
  fini_array += 1;

  /* Count the number of destructors. */
  int count = 0;
  while (fini_array[count] != NULL) {
    ++count;
  }

  /* Now call each destructor in reverse order. */
  while (count > 0) {
    void (*func)() = (void (*)()) fini_array[--count];

    /* Sanity check, any -1 in the list is ignored */
    if ((size_t)func == minus1) {
      continue;
    }

    func();
  }

#ifndef LIBC_STATIC
  {
    extern void __libc_postfini(void) __attribute__((weak));
    if (__libc_postfini) {
      __libc_postfini();
    }
  }
#endif
}
