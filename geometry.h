//
// Created by remoooo on 2023/9/19.
//

#ifndef MYMATHLIB_GEOMETRY_H
#define MYMATHLIB_GEOMETRY_H

// geometry.h
#pragma once

#include <iostream>
#include <cmath>

template <typename T, int N>
struct Vec {
    T values[N];

    // 构造函数
    Vec() = default; // 默认构造函数
    template<typename... Args> explicit Vec(Args... args);

    void print() const;
    T& operator[](int index);
    const T& operator[](int index) const;

    [[nodiscard]] Vec<T, N> operator+(const Vec<T, N>& other) const;// 向量间加法
    [[nodiscard]] Vec<T, N> operator-(const Vec<T, N>& other) const;// 向量间减法
    [[nodiscard]] Vec<T, N> operator*(T scalar) const;// 向量与常数乘法
    [[nodiscard]] Vec<T, N> operator/(T scalar) const;// 向量与常数除法
    [[nodiscard]] T dot(const Vec<T, N>& other) const;

    [[nodiscard]] double magnitude() const;// 向量模长
    [[nodiscard]] Vec<T, N> normalize() const;// 向量单位化

    // 流传输功能
    template <typename U, int M>
    friend std::ostream& operator<<(std::ostream& os, const Vec<U, N>& vec);

    // 叉积仅对3D向量有效
    template<int M = N>
    typename std::enable_if<M == 3, Vec<T, 3>>::type cross(const Vec<T, 3>& other) const;
};

// 流传输功能
template <typename U, int M>
std::ostream& operator<<(std::ostream& os, const Vec<U, M>& vec) {
    os << "(";
    for(int i = 0; i < M; i++) {
        os << vec[i];
        if (i < M - 1) {
            os << ", ";
        }
    }
    os << ")";
    return os;
}

// 折叠表达式构造多传参函数
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

// 实现标量与向量的乘
template <typename T, int N>
Vec<T, N> Vec<T, N>::operator*(T scalar) const {
    Vec<T, N> result;
    for(int i = 0; i < N; i++) {
        result[i] = values[i] * scalar;
    }
    return result;
}

// 实现标量与向量的除法
template <typename T, int N>
Vec<T, N> Vec<T, N>::operator/(T scalar) const {
    Vec<T, N> result;
    for(int i = 0; i < N; i++) {
        result[i] = values[i] / scalar;
    }
    return result;
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

// 实现求模长
template <typename T, int N>
double Vec<T, N>::magnitude() const {
    T sum = 0;
    for(int i = 0; i < N; i++) {
        sum += values[i] * values[i];
    }
    return std::sqrt(sum);
}

// 单位化向量
template <typename T, int N>
Vec<T, N> Vec<T, N>::normalize() const {
    T mag = magnitude();
    if(mag == 0) {
        // 不能单位化一个零向量，此处可以抛出异常或返回原向量
        // 为简化，此处返回原向量
        return *this;
    }

    Vec<T, N> result;
    for(int i = 0; i < N; i++) {
        result[i] = values[i] / mag;
    }
    return result;
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
