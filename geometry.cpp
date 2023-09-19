#include "geometry.h"

// 使用一个vec<3, float>实例来构造一个vec<3, int>实例，四舍五入。下面的同理。
template <>
template <> vec<3,int>  ::vec(const vec<3,float> &v) : x(int(v.x+.5f)),y(int(v.y+.5f)),z(int(v.z+.5f)) {}

template <>
template <> vec<3,float>::vec(const vec<3,int> &v)   : x(v.x),y(v.y),z(v.z) {}

template <>
template <> vec<2,int>  ::vec(const vec<2,float> &v) : x(int(v.x+.5f)),y(int(v.y+.5f)) {}

template <>
template <> vec<2,float>::vec(const vec<2,int> &v)   : x(v.x),y(v.y) {}
