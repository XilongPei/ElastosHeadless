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

#MODULE_CPPFLAGS := -std=c++11

add_definitions(-std=c++0x -fpermissive -Wno-permissive)


set(src
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
    ElastosRuntime/reflection/pseudo-dlfcn.cpp
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
    )

#add_subdirectory(ElastosRuntime)

MONKEY_PLUGIN(ElastosSuperExe "${src}")
