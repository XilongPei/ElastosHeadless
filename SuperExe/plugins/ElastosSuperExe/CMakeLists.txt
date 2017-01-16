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

#MODULE_CPPFLAGS := -std=c++11

add_definitions(-std=c++0x -fpermissive -Wno-permissive)

add_definitions(-DANDROID_SMP=1)

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
    linker/libc_logging.cpp
    linker/pthread_internals.cpp
    linker/thread_private.cpp
    linker/libc_init_common.cpp
    linker/findfp.c
    linker/fflush.c
    linker/stdio.c

    #linker has not its own version of crtbegin, but it has to be initialized
    linker/arch/x86/begin.c
    linker/atexit.c
    )

add_subdirectory(conf)

MONKEY_PLUGIN(ElastosSuperExe "${src}")

