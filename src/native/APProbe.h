#pragma once
#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#ifdef __cplusplus
extern "C" {
#endif
void* ap_malloc(size_t);void* ap_calloc(size_t,size_t);void* ap_realloc(void*,size_t);void ap_free(void*);void ap_enter(LPCRITICAL_SECTION);FILE* ap_fopen(const char*,const char*);size_t ap_fwrite(const void*,size_t,size_t,FILE*);
#ifdef __cplusplus
}
#endif
#ifndef AP_PROBE_IMPL
#define malloc ap_malloc
#define calloc ap_calloc
#define realloc ap_realloc
#define free ap_free
#define EnterCriticalSection ap_enter
#define fopen ap_fopen
#define fwrite ap_fwrite
#endif
