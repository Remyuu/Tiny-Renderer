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

    Matrix() = default;

    Matrix(const std::initializer_list< std::initializer_list<T> >& list);

    void print() const;

    T& operator()(int row, int col);

    const T& operator()(int row, int col) const;

    Matrix operator+(const Matrix& other) const;

    Matrix operator*(T scalar) const;

    template<int NewCols>
    Matrix<T, Rows, NewCols> operator*(const Matrix<T, Cols, NewCols>& other) const;

    void set_col(size_t idx, const Vec<T, Rows>& v);

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














//#ifndef __GEOMETRY_H__
//#define __GEOMETRY_H__
//#include <cmath>
//#include <vector>
//#include <cassert>
//#include <iostream>
//
//template<size_t DimCols,size_t DimRows,typename T> class mat;
//
//template <size_t DIM, typename T> struct vec {
//    vec() { for (size_t i=DIM; i--; data_[i] = T()); }
//    T& operator[](const size_t i)       { assert(i<DIM); return data_[i]; }
//    const T& operator[](const size_t i) const { assert(i<DIM); return data_[i]; }
//private:
//    T data_[DIM];
//};
//
///////////////////////////////////////////////////////////////////////////////////
//
//template <typename T> struct vec<2,T> {
//    vec() : x(T()), y(T()) {}
//    vec(T X, T Y) : x(X), y(Y) {}
//    template <class U> vec<2,T>(const vec<2,U> &v);
//    T& operator[](const size_t i)       { assert(i<2); return i<=0 ? x : y; }
//    const T& operator[](const size_t i) const { assert(i<2); return i<=0 ? x : y; }
//
//    T x,y;
//};
//
///////////////////////////////////////////////////////////////////////////////////
//
//template <typename T> struct vec<3,T> {
//    vec() : x(T()), y(T()), z(T()) {}
//    vec(T X, T Y, T Z) : x(X), y(Y), z(Z) {}
//    template <class U> vec<3,T>(const vec<3,U> &v);
//    T& operator[](const size_t i)       { assert(i<3); return i<=0 ? x : (1==i ? y : z); }
//    const T& operator[](const size_t i) const { assert(i<3); return i<=0 ? x : (1==i ? y : z); }
//    float norm() { return std::sqrt(x*x+y*y+z*z); }
//    vec<3,T> & normalize(T l=1) { *this = (*this)*(l/norm()); return *this; }
//
//    T x,y,z;
//};
//
///////////////////////////////////////////////////////////////////////////////////
//
//template<size_t DIM,typename T> T operator*(const vec<DIM,T>& lhs, const vec<DIM,T>& rhs) {
//    T ret = T();
//    for (size_t i=DIM; i--; ret+=lhs[i]*rhs[i]);
//    return ret;
//}
//
//
//template<size_t DIM,typename T>vec<DIM,T> operator+(vec<DIM,T> lhs, const vec<DIM,T>& rhs) {
//    for (size_t i=DIM; i--; lhs[i]+=rhs[i]);
//    return lhs;
//}
//
//template<size_t DIM,typename T>vec<DIM,T> operator-(vec<DIM,T> lhs, const vec<DIM,T>& rhs) {
//    for (size_t i=DIM; i--; lhs[i]-=rhs[i]);
//    return lhs;
//}
//
//template<size_t DIM,typename T,typename U> vec<DIM,T> operator*(vec<DIM,T> lhs, const U& rhs) {
//    for (size_t i=DIM; i--; lhs[i]*=rhs);
//    return lhs;
//}
//
//template<size_t DIM,typename T,typename U> vec<DIM,T> operator/(vec<DIM,T> lhs, const U& rhs) {
//    for (size_t i=DIM; i--; lhs[i]/=rhs);
//    return lhs;
//}
//
//template<size_t LEN,size_t DIM,typename T> vec<LEN,T> embed(const vec<DIM,T> &v, T fill=1) {
//    vec<LEN,T> ret;
//    for (size_t i=LEN; i--; ret[i]=(i<DIM?v[i]:fill));
//    return ret;
//}
//
//template<size_t LEN,size_t DIM, typename T> vec<LEN,T> proj(const vec<DIM,T> &v) {
//    vec<LEN,T> ret;
//    for (size_t i=LEN; i--; ret[i]=v[i]);
//    return ret;
//}
//
//template <typename T> vec<3,T> cross(vec<3,T> v1, vec<3,T> v2) {
//    return vec<3,T>(v1.y*v2.z - v1.z*v2.y, v1.z*v2.x - v1.x*v2.z, v1.x*v2.y - v1.y*v2.x);
//}
//
//template <size_t DIM, typename T> std::ostream& operator<<(std::ostream& out, vec<DIM,T>& v) {
//    for(unsigned int i=0; i<DIM; i++) {
//        out << v[i] << " " ;
//    }
//    return out ;
//}
//
///////////////////////////////////////////////////////////////////////////////////
//
//template<size_t DIM,typename T> struct dt {
//    static T det(const mat<DIM,DIM,T>& src) {
//        T ret=0;
//        for (size_t i=DIM; i--; ret += src[0][i]*src.cofactor(0,i));
//        return ret;
//    }
//};
//
//template<typename T> struct dt<1,T> {
//    static T det(const mat<1,1,T>& src) {
//        return src[0][0];
//    }
//};
//
///////////////////////////////////////////////////////////////////////////////////
//
//template<size_t DimRows,size_t DimCols,typename T> class mat {
//    vec<DimCols,T> rows[DimRows];
//public:
//    mat() {}
//
//    vec<DimCols,T>& operator[] (const size_t idx) {
//        assert(idx<DimRows);
//        return rows[idx];
//    }
//
//    const vec<DimCols,T>& operator[] (const size_t idx) const {
//        assert(idx<DimRows);
//        return rows[idx];
//    }
//
//    vec<DimRows,T> col(const size_t idx) const {
//        assert(idx<DimCols);
//        vec<DimRows,T> ret;
//        for (size_t i=DimRows; i--; ret[i]=rows[i][idx]);
//        return ret;
//    }
//
//    void set_col(size_t idx, vec<DimRows,T> v) {
//        assert(idx<DimCols);
//        for (size_t i=DimRows; i--; rows[i][idx]=v[i]);
//    }
//
//    static mat<DimRows,DimCols,T> identity() {
//        mat<DimRows,DimCols,T> ret;
//        for (size_t i=DimRows; i--; )
//            for (size_t j=DimCols;j--; ret[i][j]=(i==j));
//        return ret;
//    }
//
//    T det() const {
//        return dt<DimCols,T>::det(*this);
//    }
//
//    mat<DimRows-1,DimCols-1,T> get_minor(size_t row, size_t col) const {
//        mat<DimRows-1,DimCols-1,T> ret;
//        for (size_t i=DimRows-1; i--; )
//            for (size_t j=DimCols-1;j--; ret[i][j]=rows[i<row?i:i+1][j<col?j:j+1]);
//        return ret;
//    }
//
//    T cofactor(size_t row, size_t col) const {
//        return get_minor(row,col).det()*((row+col)%2 ? -1 : 1);
//    }
//
//    mat<DimRows,DimCols,T> adjugate() const {
//        mat<DimRows,DimCols,T> ret;
//        for (size_t i=DimRows; i--; )
//            for (size_t j=DimCols; j--; ret[i][j]=cofactor(i,j));
//        return ret;
//    }
//
//    mat<DimRows,DimCols,T> invert_transpose() {
//        mat<DimRows,DimCols,T> ret = adjugate();
//        T tmp = ret[0]*rows[0];
//        return ret/tmp;
//    }
//
//    mat<DimCols,DimRows,T> invert() {
//        return invert_transpose().transpose();
//    }
//
//    mat<DimCols,DimRows,T> transpose() {
//        mat<DimCols,DimRows,T> ret;
//        for (size_t i=DimRows; i--; ret[i]=this->col(i));
//        return ret;
//    }
//};
//
///////////////////////////////////////////////////////////////////////////////////
//
//template<size_t DimRows,size_t DimCols,typename T> vec<DimRows,T> operator*(const mat<DimRows,DimCols,T>& lhs, const vec<DimCols,T>& rhs) {
//    vec<DimRows,T> ret;
//    for (size_t i=DimRows; i--; ret[i]=lhs[i]*rhs);
//    return ret;
//}
//
//template<size_t R1,size_t C1,size_t C2,typename T>mat<R1,C2,T> operator*(const mat<R1,C1,T>& lhs, const mat<C1,C2,T>& rhs) {
//    mat<R1,C2,T> result;
//    for (size_t i=R1; i--; )
//        for (size_t j=C2; j--; result[i][j]=lhs[i]*rhs.col(j));
//    return result;
//}
//
//template<size_t DimRows,size_t DimCols,typename T>mat<DimCols,DimRows,T> operator/(mat<DimRows,DimCols,T> lhs, const T& rhs) {
//    for (size_t i=DimRows; i--; lhs[i]=lhs[i]/rhs);
//    return lhs;
//}
//
//template <size_t DimRows,size_t DimCols,class T> std::ostream& operator<<(std::ostream& out, mat<DimRows,DimCols,T>& m) {
//    for (size_t i=0; i<DimRows; i++) out << m[i] << std::endl;
//    return out;
//}
//
///////////////////////////////////////////////////////////////////////////////////
//
//typedef vec<2,  float> Vec2f;
//typedef vec<2,  int>   Vec2i;
//typedef vec<3,  float> Vec3f;
//typedef vec<3,  int>   Vec3i;
//typedef vec<4,  float> Vec4f;
//typedef mat<4,4,float> Matrix;
//#endif //__GEOMETRY_H__