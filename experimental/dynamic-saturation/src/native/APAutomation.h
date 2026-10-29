#pragma once
#include <cmath>
#include <cstdint>
struct APPoint {int offset;double value;};
struct APPlan {
 static constexpr int MaxFrames=16384,MaxPoints=4096;
 APPoint points[4][MaxPoints]{};int count[4]{};double value[4][MaxFrames]{};double final[4]{};
 bool build(int frames,const double initial[4]){
 if(frames<0||frames>MaxFrames)return false;
 for(int id=0;id<4;++id){int x=-1;double y=initial[id];if(!std::isfinite(y)||y<0||y>1)return false;
 if(count[id]<0||count[id]>MaxPoints)return false;
 for(int j=0;j<count[id];++j){auto q=points[id][j];if(q.offset<0||q.offset<(j?points[id][j-1].offset:0)||q.offset>=(frames?frames:1)||!std::isfinite(q.value)||q.value<0||q.value>1)return false;
 // VST3 (ivstparameterchanges.h): each point defines a straight line from the previous point.
 // Continuous parameters (ANGER, COLOR, GAIN) interpolate linearly per sample; BYPASS (id 3) is
 // discrete and holds its value. The mapping to plain units is affine, so this is linear in both domains.
 for(int s=x+1;s<q.offset&&s<frames;++s)value[id][s]=id==3?y:y+(q.value-y)*double(s-x)/double(q.offset-x);
 if(frames)value[id][q.offset]=q.value;
 // Same-offset duplicate: last transmitted point wins at that sample.
 if(q.offset==x&&frames)value[id][x]=q.value;
 x=q.offset;y=q.value;
 }
 for(int s=x+1;s<frames;++s)value[id][s]=y;
 final[id]=y;
 }
 return true;}
};
