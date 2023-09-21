/**
 * @file geometry.h
 * @author remoooo
 * @date 2023/9/19.
 * @brief 提供用于表示和操作矩阵(Matrix)和向量(Vec)的模板类。
 */

#ifndef GEOMETRY_H
#define GEOMETRY_H

// geometry.h
#pragma once

#include <iostream>
#include <cmath>

/**
 *  ========================
 *         Vec Part
 *  ========================
 *
 * @brief 表示一个N维向量的模板类。
 *
 * @tparam T 向量中元素的类型。
 * @tparam N 向量的维度。
 */
template <typename T, int N>
struct Vec {
    T values[N]; ///< 存储向量的数组。

    /**
     * @brief 默认构造函数。
     * 使用默认的成员初始化。
     */
    Vec() = default;

    /**
     * @brief 变参构造函数。
     * 允许通过提供具体的值来初始化向量。
     *
     * @tparam Args 变参类型。
     * @param args 用于初始化向量的值。
     */
    template<typename... Args> explicit Vec(Args... args);

    /**
     * @brief 打印向量的内容到标准输出。
     */
    void print() const;

    /**
     * @brief 用于访问向量元素的下标操作符。
     *
     * @param index 要访问的元素的索引。
     * @return 向量元素的引用。
     */
    T& operator[](int index);

    /**
     * @brief 访问向量元素的常量版本的下标操作符。
     *
     * @param index 要访问的元素的索引。
     * @return 向量元素的常量引用。
     */
    const T& operator[](int index) const;

    /**
     * @brief 向量加法操作符。
     *
     * @param other 要与当前向量相加的向量。
     * @return 相加后的新向量。
     */
    [[nodiscard]] Vec<T, N> operator+(const Vec<T, N>& other) const;

    /**
     * @brief 向量减法操作符。
     *
     * @param other 要从当前向量中减去的向量。
     * @return 相减后的新向量。
     */
    [[nodiscard]] Vec<T, N> operator-(const Vec<T, N>& other) const;

    /**
     * @brief 向量与标量的乘法操作符。
     *
     * @param scalar 与向量相乘的标量。
     * @return 标量乘法后的新向量。
     */
    [[nodiscard]] Vec<T, N> operator*(T scalar) const;

    /**
     * @brief 向量与标量的除法操作符。
     *
     * @param scalar 作为除数的标量。
     * @return 标量除法后的新向量。
     */
    [[nodiscard]] Vec<T, N> operator/(T scalar) const;

    /**
     * @brief 计算向量的点积。
     *
     * @param other 与当前向量计算点积的另一个向量。
     * @return 两向量的点积。
     */
    [[nodiscard]] T dot(const Vec<T, N>& other) const;

    /**
     * @brief 计算向量的模长。
     *
     * @return 向量的模长。
     */
    [[nodiscard]] double magnitude() const;

    /**
     * @brief 计算向量的单位化版本。
     *
     * @return 单位化后的向量。
     */
    [[nodiscard]] Vec<T, N> normalize() const;

    /**
     * @brief 为向量定义的流输出操作符。
     *
     * @param os 输出流。
     * @param vec 要输出的向量。
     * @return 引用输出流，允许链式调用。
     */
    template <typename U, int M>
    friend std::ostream& operator<<(std::ostream& os, const Vec<U, N>& vec);

    /**
     * @brief 计算两个3D向量的叉积。
     * 只对3D向量有效。
     *
     * @param other 与当前向量计算叉积的另一个向量。
     * @return 两向量的叉积。
     */
    template<int M = N>
    typename std::enable_if<M == 3, Vec<T, 3>>::type cross(const Vec<T, 3>& other) const;

    /**
     * @brief 可以将已有的 `Vec_U` 对象构造 `Vec_T`。
     * 四舍五入的。
     *
     * @param Vec<U, N> other 待转换的向量
     * @return 新类型的向量。
     */
    template <typename U>
    explicit Vec<T, N>(const Vec<U, N>& other);
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
//template <typename T, int N>
//template<typename... Args>
//Vec<T, N>::Vec(Args... args) : values{args...} {
//    static_assert(sizeof...(args) == N, "Wrong number of arguments");
//}

// 更通用的构造函数 比如你可以这样构造函数 Vec2f v(1,2);
template <typename T, int N>
template <typename... Args>
Vec<T, N>::Vec(Args... args) : values{static_cast<T>(args)...} {
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

// 将已有的 `Vec_U` 对象构造 `Vec_T`
template <typename T, int N>
template <typename U>
Vec<T, N>::Vec(const Vec<U, N>& other) {
    for(int i = 0; i < N; ++i) {
        values[i] = static_cast<T>(std::round(other[i]));
    }
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

/**
 *  ========================
 *        Matrix Part
 *  ========================
 *
 * @brief 表示一个矩阵的模板类。
 *
 * @tparam T 矩阵中元素的类型。
 * @tparam Rows 矩阵的行数。
 * @tparam Cols 矩阵的列数。
 */

template <typename T, int Rows, int Cols>
struct Matrix {
    T values[Rows][Cols]; ///< 存储矩阵的2D数组。

    /**
     * @brief 默认构造函数。
     * 使用默认的成员初始化。
     */
    Matrix() = default;

    /**
     * @brief 列表初始化构造函数。
     *
     * @param list 用于初始化矩阵的值的2D初始化列表。
     */
    Matrix(const std::initializer_list< std::initializer_list<T> >& list);

    /**
     * @brief 打印矩阵的内容到标准输出。
     */
    void print() const;

    /**
     * @brief 用于访问矩阵元素的函数调用操作符。
     *
     * @param row 要访问的元素的行。
     * @param col 要访问的元素的列。
     * @return 矩阵元素的引用。
     */
    T& operator()(int row, int col);

    /**
     * @brief 访问矩阵元素的常量版本的函数调用操作符。
     *
     * @param row 要访问的元素的行。
     * @param col 要访问的元素的列。
     * @return 矩阵元素的常量引用。
     */
    const T& operator()(int row, int col) const;

    /**
     * @brief 矩阵加法操作符。
     *
     * @param other 要与当前矩阵相加的矩阵。
     * @return 相加后的新矩阵。
     */
    Matrix operator+(const Matrix& other) const;

    /**
     * @brief 矩阵与标量的乘法操作符。
     *
     * @param scalar 与矩阵相乘的标量。
     * @return 标量乘法后的新矩阵。
     */
    Matrix operator*(T scalar) const;

    /**
     * @brief 矩阵与矩阵的乘法操作符。
     *
     * @tparam NewCols 结果矩阵的列数。
     * @param other 与当前矩阵相乘的矩阵。
     * @return 矩阵乘法的结果。
     */
    template<int NewCols>
    Matrix<T, Rows, NewCols> operator*(const Matrix<T, Cols, NewCols>& other) const;
};


// 使用初始化列表{}构造矩阵
template<typename T, int Rows, int Cols>
Matrix<T, Rows, Cols>::Matrix(const std::initializer_list< std::initializer_list<T> >& list) {
    T* target = &values[0][0];
    for (const auto& rowList : list) {
        target = std::copy(rowList.begin(), rowList.end(), target);
    }
}

// 打印矩阵函数
template<typename T, int Rows, int Cols>
void Matrix<T, Rows, Cols>::print() const {
    for (int i = 0; i < Rows; i++) {
        for (int j = 0; j < Cols; j++) {
            std::cout << values[i][j];
            if (j < Cols - 1) {
                std::cout << "\t";  // 在列之间添加制表符，以美化输出
            }
        }
        std::cout << std::endl;  // 打印换行，进入下一行
    }
}

// 访问矩阵元素
template<typename T, int Rows, int Cols>
T& Matrix<T, Rows, Cols>::operator()(int row, int col) {
    return values[row][col];
}
template<typename T, int Rows, int Cols>
const T& Matrix<T, Rows, Cols>::operator()(int row, int col) const {
    return values[row][col];
}

// 矩阵加法
template <typename T, int Rows, int Cols>
Matrix<T, Rows, Cols> Matrix<T, Rows, Cols>::operator+(const Matrix& other) const {
    Matrix result;
    for (int i = 0; i < Rows; i++) {
        for (int j = 0; j < Cols; j++) {
            result(i, j) = values[i][j] + other(i, j);
        }
    }
    return result;
}

// 矩阵与标量的乘法
template <typename T, int Rows, int Cols>
Matrix<T, Rows, Cols> Matrix<T, Rows, Cols>::operator*(T scalar) const {
    Matrix result;
    for (int i = 0; i < Rows; i++) {
        for (int j = 0; j < Cols; j++) {
            result(i, j) = values[i][j] * scalar;
        }
    }
    return result;
}

// 矩阵与矩阵的乘法
template <typename T, int Rows, int Cols> template<int NewCols>
Matrix<T, Rows, NewCols> Matrix<T, Rows, Cols>::operator*(const Matrix<T, Cols, NewCols>& other) const {
    Matrix<T, Rows, NewCols> result;
    for (int i = 0; i < Rows; i++) {
        for (int j = 0; j < NewCols; j++) {
            T sum = 0;
            for (int k = 0; k < Cols; k++) {
                sum += values[i][k] * other(k, j);
            }
            result(i, j) = sum;
        }
    }
    return result;
}

// 流传输运算符重载
template<typename T, int Rows, int Cols>
std::ostream& operator<<(std::ostream& os, const Matrix<T, Rows, Cols>& matrix) {
    for (int i = 0; i < Rows; i++) {
        for (int j = 0; j < Cols; j++) {
            os << matrix(i, j);
            if (j < Cols - 1) os << "\t";
        }
        if (i < Rows - 1) os << "\n";
    }
    return os;
}

// 为 Mat2、Mat3、Mat4 等提供类型别名
using Matrix2i = Matrix<int   , 2, 2>;
using Matrix3i = Matrix<int   , 3, 3>;
using Matrix4i = Matrix<int   , 4, 4>;

using Matrix2f = Matrix<float , 2, 2>;
using Matrix3f = Matrix<float , 3, 3>;
using Matrix4f = Matrix<float , 4, 4>;

using Matrix2d = Matrix<double, 2, 2>;
using Matrix3d = Matrix<double, 3, 3>;
using Matrix4d = Matrix<double, 4, 4>;

#endif GEOMETRY_H
