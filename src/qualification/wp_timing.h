#pragma once
#ifndef AP_TIMING
#define AP_TIMING 0
#endif
#if AP_TIMING
#ifdef __cplusplus
extern "C" {
#endif
void wp_begin(int stage);void wp_end(int stage);void wp_count(int field,int value);
#ifdef __cplusplus
}
#endif
#else
#define wp_begin(x) ((void)0)
#define wp_end(x) ((void)0)
#define wp_count(x,y) ((void)0)
#endif
