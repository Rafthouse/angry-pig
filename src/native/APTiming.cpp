#include "APTiming.h"
#if AP_TIMING
thread_local WPFrame* wp_frame=nullptr;
extern "C" void wp_begin(int s){if(wp_frame)wp_frame->begin[s-1]=wp_now();}
extern "C" void wp_end(int s){if(wp_frame){auto&t=wp_frame->begin[s-1];if(t){wp_frame->r.s[s-1]+=wp_now()-t;t=0;}}}
extern "C" void wp_count(int field,int value){if(!wp_frame)return;if(field==0)wp_frame->r.queues+=value;if(field==1)wp_frame->r.calls+=value;if(field==2)wp_frame->r.reset=value;}
#endif
