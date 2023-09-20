//
// Created by remoooo on 2023/9/19.
//

#ifndef MYMATHLIB_GEOMETRY_H
#define MYMATHLIB_GEOMETRY_H

// geometry.h
#pragma once

#include <iostream>

template <typename T, int N>
struct Vec {
    T values[N];

    // 构造函数
    Vec() = default; // 默认构造函数
    template<typename... Args> explicit Vec(Args... args);

    void print() const;
    T& operator[](int index);
    const T& operator[](int index) const;

    Vec<T, N> operator+(const Vec<T, N>& other) const;
    Vec<T, N> operator-(const Vec<T, N>& other) const;
    T dot(const Vec<T, N>& other) const;

    // 叉积仅对3D向量有效
    template<int M = N>
    typename std::enable_if<M == 3, Vec<T, 3>>::type cross(const Vec<T, 3>& other) const;
};

template <typename T, int N>
template<typename... Args>
Vec<T, N>::Vec(Args... args) : values{args...} {
    static_assert(sizeof...(args) == N, "Wrong number of arguments");
}

// 打印Vec
template <typename T, int N>
void Vec<T, N>::print() const {
    std::cout << "(";
    for(int i = 0; i < N; i++) {
        std::cout << values[i] << (i < N - 1 ? ", " : ")\n");
    }
}

// 非常量的Vec访问，可以称为左值
template <typename T, int N>
T& Vec<T, N>::operator[](int index) {
    return values[index];
}

// 常量Vec访问
template <typename T, int N>
const T& Vec<T, N>::operator[](int index) const {
    return values[index];
}

// 实现加法运算
template <typename T, int N>
Vec<T, N> Vec<T, N>::operator+(const Vec<T, N>& other) const {
    Vec<T, N> result;
    for(int i = 0; i < N; i++) {
        result[i] = values[i] + other[i];
    }
    return result;
}

// 实现减法运算
template <typename T, int N>
Vec<T, N> Vec<T, N>::operator-(const Vec<T, N>& other) const {
    Vec<T, N> result;
    for(int i = 0; i < N; i++) {
        result[i] = values[i] - other[i];
    }
    return result;
}

// 实现点积运算
template <typename T, int N>
T Vec<T, N>::dot(const Vec<T, N>& other) const {
    T sum = 0;
    for(int i = 0; i < N; i++) {
        sum += values[i] * other[i];
    }
    return sum;
}

// 实现了Vec3的叉积运算
template <typename T, int N>
template<int M>
typename std::enable_if<M == 3, Vec<T, 3>>::type Vec<T, N>::cross(const Vec<T, 3>& other) const {
    return Vec<T, 3>(
            values[1] * other[2] - values[2] * other[1],
            values[2] * other[0] - values[0] * other[2],
            values[0] * other[1] - values[1] * other[0]
    );
}

// 为 Vec2、Vec3、Vec4 等提供类型别名
using Vec2i = Vec<int, 2>;
using Vec3i = Vec<int, 3>;
using Vec4i = Vec<int, 4>;

using Vec2f = Vec<float, 2>;
using Vec3f = Vec<float, 3>;
using Vec4f = Vec<float, 4>;

using Vec2d = Vec<double, 2>;
using Vec3d = Vec<double, 3>;
using Vec4d = Vec<double, 4>;


#endif //MYMATHLIB_GEOMETRY_H
