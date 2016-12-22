#
#ref a makefile for Android: ElastosRuntime/reflection/rules.mk
#

include_directories(Runtime/Core/inc)
include_directories(Runtime/Library/inc/eltypes)
include_directories(Runtime/Library/inc/car)
include_directories(Runtime/Library/inc/elasys)
include_directories(Runtime/Library/inc/clsmodule)
include_directories(Runtime/Library/syscar)
include_directories(rdk/inc)
include_directories(rdk/PortingLayer)
include_directories(ElRuntimeAPI)

#MODULE_CPPFLAGS := -std=c++11

set(src
    reflection/CClsModule.cpp
    reflection/CObjInfoList.cpp
    reflection/CEntryList.cpp
    reflection/refutil.cpp
    reflection/reflection.cpp
    reflection/CArgumentList.cpp
    #reflection/CCallbackArgumentList.cpp
    #reflection/CCallbackMethodInfo.cpp
    reflection/CCarArrayInfo.cpp
    reflection/CVariableOfCarArray.cpp
    reflection/CClassInfo.cpp
    reflection/CConstantInfo.cpp
    reflection/CConstructorInfo.cpp
    reflection/CCppVectorInfo.cpp
    #reflection/CDelegateProxy.cpp
    reflection/CEnumInfo.cpp
    reflection/CEnumItemInfo.cpp
    reflection/CFieldInfo.cpp
    reflection/CInterfaceInfo.cpp
    reflection/CIntrinsicInfo.cpp
    reflection/CLocalPtrInfo.cpp
    reflection/CLocalTypeInfo.cpp
    reflection/CMethodInfo.cpp
    reflection/CModuleInfo.cpp
    reflection/CParamInfo.cpp
    reflection/CStructInfo.cpp
    reflection/CVariableOfStruct.cpp
    reflection/CVariableOfCppVector.cpp
    reflection/CTypeAliasInfo.cpp
    reflection/invoke_gnuc.S
    reflection/manifest.c
    reflection/ref_app.cpp
    reflection/pseudo-dlfcn.cpp
    reflection/pseudo-misc.cpp
    Runtime/Library/elasys/sysiids.cpp
    Runtime/Library/eltypes/elstring/elstring.cpp
    Runtime/Library/eltypes/elstring/elsharedbuf.cpp
    Runtime/Library/eltypes/elstringapi.cpp
    Runtime/Library/elasys/elaatomics.cpp
    Runtime/Library/eltypes/elquintet.cpp

    ElRuntimeAPI/xni.cpp
    ElRuntimeAPI/implClassFactory.cpp

    #Runtime/Library/clsmodule/reloc.cpp
    #Runtime/Library/clsmodule/clserr.cpp
    #Runtime/Library/clsmodule/comprcls.cpp
    #Runtime/Library/clsmodule/datatype.cpp
)

#add_subdirectory(ElastosRuntime)

MONKEY_PLUGIN(ElastosSuperExe "${src}")
