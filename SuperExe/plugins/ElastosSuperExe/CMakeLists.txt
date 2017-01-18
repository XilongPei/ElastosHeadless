#
#ref a makefile for Android: ElastosRuntime/reflection/rules.mk
#

include_directories(ElastosRuntime/Runtime/Core/inc)
include_directories(ElastosRuntime/Runtime/Library/inc/eltypes)
include_directories(ElastosRuntime/Runtime/Library/inc/car)
include_directories(ElastosRuntime/Runtime/Library/inc/elasys)
include_directories(ElastosRuntime/Runtime/Library/inc/clsmodule)
include_directories(ElastosRuntime/Runtime/Library/syscar)
include_directories(ElastosRuntime/rdk/inc)
include_directories(ElastosRuntime/rdk/PortingLayer)
include_directories(ElastosRuntime/ElRuntimeAPI)

include_directories(linker/bionic/libc/include)
include_directories(linker/bionic/libc/arch-common)
include_directories(linker/bionic/libc/arch-x86/include)
include_directories(linker/bionic/libc/bionic)
include_directories(linker/bionic/libc/private)
include_directories(linker/bionic/libc/kernel/uapi)
include_directories(linker/bionic/libc)
include_directories(linker/include)
include_directories(linker/bionic/libc/kernel/uapi/asm-x86)
include_directories(linker/bionic/libc/kernel/uapi)
#include_directories(linker/bionic/libc/kernel/uapi/asm-generic)

#MODULE_CPPFLAGS := -std=c++11

add_definitions(-std=c++0x -fpermissive -Wno-permissive)

add_definitions(-DANDROID_SMP=1)

#set_property(SOURCE ElastosRuntime/reflection/invoke_gnuc.S PROPERTY LANGUAGE C)
set_property(SOURCE linker/bionic/libc/arch-x86/syscalls/__set_thread_area.S PROPERTY LANGUAGE C)
set_property(SOURCE linker/bionic/libc/arch-x86/syscalls/__set_tid_address.S PROPERTY LANGUAGE C)

set(src
    SuperExeStaticMain.cpp
    SuperExeEntry.c

    ElastosRuntime/reflection/CClsModule.cpp
    ElastosRuntime/reflection/CObjInfoList.cpp
    ElastosRuntime/reflection/CEntryList.cpp
    ElastosRuntime/reflection/refutil.cpp
    ElastosRuntime/reflection/reflection.cpp
    ElastosRuntime/reflection/CArgumentList.cpp
    #ElastosRuntime/reflection/CCallbackArgumentList.cpp
    #ElastosRuntime/reflection/CCallbackMethodInfo.cpp
    ElastosRuntime/reflection/CCarArrayInfo.cpp
    ElastosRuntime/reflection/CVariableOfCarArray.cpp
    ElastosRuntime/reflection/CClassInfo.cpp
    ElastosRuntime/reflection/CConstantInfo.cpp
    ElastosRuntime/reflection/CConstructorInfo.cpp
    ElastosRuntime/reflection/CCppVectorInfo.cpp
    #ElastosRuntime/reflection/CDelegateProxy.cpp
    ElastosRuntime/reflection/CEnumInfo.cpp
    ElastosRuntime/reflection/CEnumItemInfo.cpp
    ElastosRuntime/reflection/CFieldInfo.cpp
    ElastosRuntime/reflection/CInterfaceInfo.cpp
    ElastosRuntime/reflection/CIntrinsicInfo.cpp
    ElastosRuntime/reflection/CLocalPtrInfo.cpp
    ElastosRuntime/reflection/CLocalTypeInfo.cpp
    ElastosRuntime/reflection/CMethodInfo.cpp
    ElastosRuntime/reflection/CModuleInfo.cpp
    ElastosRuntime/reflection/CParamInfo.cpp
    ElastosRuntime/reflection/CStructInfo.cpp
    ElastosRuntime/reflection/CVariableOfStruct.cpp
    ElastosRuntime/reflection/CVariableOfCppVector.cpp
    ElastosRuntime/reflection/CTypeAliasInfo.cpp
    ElastosRuntime/reflection/invoke_gnuc.S
#    ElastosRuntime/reflection/pseudo-dlfcn.cpp
    ElastosRuntime/reflection/pseudo-misc.cpp
    ElastosRuntime/Runtime/Library/elasys/sysiids.cpp
    ElastosRuntime/Runtime/Library/eltypes/elstring/elstring.cpp
    ElastosRuntime/Runtime/Library/eltypes/elstring/elsharedbuf.cpp
    ElastosRuntime/Runtime/Library/eltypes/elstringapi.cpp
    ElastosRuntime/Runtime/Library/elasys/elaatomics.cpp
    ElastosRuntime/Runtime/Library/eltypes/elquintet.cpp

    ElastosRuntime/ElRuntimeAPI/xni.cpp
    ElastosRuntime/ElRuntimeAPI/implClassFactory.cpp

    #ElastosRuntime/Runtime/Library/clsmodule/reloc.cpp
    #ElastosRuntime/Runtime/Library/clsmodule/clserr.cpp
    #ElastosRuntime/Runtime/Library/clsmodule/comprcls.cpp
    #ElastosRuntime/Runtime/Library/clsmodule/datatype.cpp

    linker/debugger.cpp
    linker/dlfcnCAR.cpp
    linker/linker.cpp
    linker/linker_allocator.cpp
    linker/linker_environ.cpp
    linker/linker_phdr.cpp
    linker/rt.cpp
    linker/libc_init_common.cpp

    linker/bionic/libc/bionic/thread_private.cpp

    linker/bionic/libc/bionic/libc_logging.cpp
#    linker/bionic/libc/bionic/libc_init_common.cpp

    linker/bionic/libc/bionic/pthread_atfork.cpp
    linker/bionic/libc/bionic/pthread_attr.cpp
    linker/bionic/libc/bionic/pthread_cond.cpp
    linker/bionic/libc/bionic/pthread_create.cpp
    linker/bionic/libc/bionic/pthread_detach.cpp
    linker/bionic/libc/bionic/pthread_equal.cpp
    linker/bionic/libc/bionic/pthread_exit.cpp
    linker/bionic/libc/bionic/pthread_getcpuclockid.cpp
    linker/bionic/libc/bionic/pthread_getschedparam.cpp
    linker/bionic/libc/bionic/pthread_gettid_np.cpp
    linker/bionic/libc/bionic/pthread_internals.cpp
    linker/bionic/libc/bionic/pthread_join.cpp
    linker/bionic/libc/bionic/pthread_key.cpp
    linker/bionic/libc/bionic/pthread_kill.cpp
    linker/bionic/libc/bionic/pthread_mutex.cpp
    linker/bionic/libc/bionic/pthread_once.cpp
    linker/bionic/libc/bionic/pthread_rwlock.cpp
    linker/bionic/libc/bionic/pthread_self.cpp
    linker/bionic/libc/bionic/pthread_setname_np.cpp
    linker/bionic/libc/bionic/pthread_setschedparam.cpp
    linker/bionic/libc/bionic/pthread_sigmask.cpp

    linker/bionic/libc/bionic/bionic_time_conversions.cpp
    linker/bionic/libc/bionic/getauxval.cpp
    linker/bionic/libc/bionic/__set_errno.cpp
    linker/bionic/libc/bionic/__errno.cpp

    linker/bionic/libc/arch-x86/bionic/__set_tls.c
    linker/bionic/libc/arch-x86/syscalls/__set_thread_area.S
    linker/bionic/libc/arch-x86/syscalls/__set_tid_address.S

    linker/bionic/libc/upstream-openbsd/lib/libc/stdlib/exit.c

    #linker has not its own version of crtbegin, but it has to be initialized
    linker/arch/x86/begin.c
    linker/atexit.c
    )

add_subdirectory(conf)

MONKEY_PLUGIN(ElastosSuperExe "${src}")

