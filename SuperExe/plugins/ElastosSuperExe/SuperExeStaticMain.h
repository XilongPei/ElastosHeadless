#ifndef __SuperExeStaticMain_h__
#define __SuperExeStaticMain_h__

#ifdef __cplusplus
extern "C" {
#endif

int ElastosMain(char *car_name, char *query_string);

char *reflectCAR(char *moduleName);
char *freeReflectMem(char *reflect_info);

#ifdef __cplusplus
}
#endif

#endif
