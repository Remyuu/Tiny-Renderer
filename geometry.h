#ifndef __GEOMETRY_H__
#define __GEOMETRY_H__

#include <cmath>

/* --向量类定义-- */
// t -> 任意类型的数据，比如说 int float double等等
template <class t> struct Vec2 {
    t x, y;// 创建了两个t类型的数据成员
    Vec2<t>() : x(t()), y(t()) {} // 使用类型t的默认构造函数来初始化x和y。
    Vec2<t>(t _x, t _y) : x(_x), y(_y) {} // 接受两个参数，初始化x,y
//    Vec2<t>(const Vec2<t> &v) : x(t()), y(t()) { *this = v; } // 模板类的拷贝构造函数
    Vec2<t>(const Vec2<t> &v) : x(v.x), y(v.y) {} // 我认为上面的代码不太好，改了一下
    Vec2<t> & operator =(const Vec2<t> &v) { // 重载了等号，改变符号左边对象的数值
        if (this != &v) {
            x = v.x;
            y = v.y;
        }
        return *this;
    }
    Vec2<t> operator +(const Vec2<t> &V) const { return Vec2<t>(x+V.x, y+V.y); }
    Vec2<t> operator -(const Vec2<t> &V) const { return Vec2<t>(x-V.x, y-V.y); }
    Vec2<t> operator *(float f)          const { return Vec2<t>(x*f, y*f); }
    // 重载[]符号，这里官方写错了，if(x<=0)是错误的。
    t& operator[](const int i) { if (i<=0) return x; else return y; }
    // 重载输出流
    template <class > friend std::ostream& operator<<(std::ostream& s, Vec2<t>& v);
};

template <class t> struct Vec3 {
    t x, y, z;
    Vec3<t>() : x(t()), y(t()), z(t()) { }
    Vec3<t>(t _x, t _y, t _z) : x(_x), y(_y), z(_z) {}
    template <class u> Vec3<t>(const Vec3<u> &v);
    Vec3<t>(const Vec3<t> &v) : x(t()), y(t()), z(t()) { *this = v; }
    Vec3<t> & operator =(const Vec3<t> &v) {
        if (this != &v) {
            x = v.x;
            y = v.y;
            z = v.z;
        }
        return *this;
    }
    Vec3<t> operator ^(const Vec3<t> &v) const { return Vec3<t>(y*v.z-z*v.y, z*v.x-x*v.z, x*v.y-y*v.x); }
    Vec3<t> operator +(const Vec3<t> &v) const { return Vec3<t>(x+v.x, y+v.y, z+v.z); }
    Vec3<t> operator -(const Vec3<t> &v) const { return Vec3<t>(x-v.x, y-v.y, z-v.z); }
    Vec3<t> operator *(float f)          const { return Vec3<t>(x*f, y*f, z*f); }
    t       operator *(const Vec3<t> &v) const { return x*v.x + y*v.y + z*v.z; }
    float norm () const { return std::sqrt(x*x+y*y+z*z); }
    Vec3<t> & normalize(t l=1) { *this = (*this)*(l/norm()); return *this; }
    t& operator[](const int i) { if (i<=0) return x; else if (i==1) return y; else return z; }
    template <class > friend std::ostream& operator<<(std::ostream& s, Vec3<t>& v);
};

// 为常用类型提供了类型别名
typedef Vec2<float> Vec2f;
typedef Vec2<int>   Vec2i;
typedef Vec3<float> Vec3f;
typedef Vec3<int>   Vec3i;

/* 特化构造函数 */
// Vec3<int> <- Vec3<float>。浮点数值 -> 整数值
template <> template <> Vec3<int>::Vec3(const Vec3<float> &v);
// Vec3<float> <- Vec3<int>。整数值 -> 浮点数值
template <> template <> Vec3<float>::Vec3(const Vec3<int> &v);

// 使得Vec2对象可以被输出到输出流
template <class t> std::ostream& operator<<(std::ostream& s, Vec2<t>& v) {
    s << "(" << v.x << ", " << v.y << ")\n";
    return s;
}
// 使得Vec3对象可以被输出到输出流
template <class t> std::ostream& operator<<(std::ostream& s, Vec3<t>& v) {
    s << "(" << v.x << ", " << v.y << ", " << v.z << ")\n";
    return s;
}

//////////////////////////////////////////////////////////////////////////////////////////////
/*
 * Matrix类代表了一个浮点数矩阵，其中的数据存储在std::vector。
 *
 */

const int DEFAULT_ALLOC=4;

class Matrix {
    // 这是一个二维vector，它存储了矩阵的所有元素。
    std::vector<std::vector<float> > m;
    int rows, cols;
public:
    // 构造函数 可以接受行数和列数作为参数。如果没有手动提供参数，则会使用DEFAULT_ALLOC（默认值为4）来初始化。
    Matrix(int r=DEFAULT_ALLOC, int c=DEFAULT_ALLOC);
    // 这个inline语法比较适合用于短小且经常被调用的函数。比如 inline int add(int a, int b){return a+b;}
    inline int nrows();
    inline int ncols();

    static Matrix identity(int dimensions); // 返回给定维度的单位矩阵
    std::vector<float>& operator[](const int i); // 这是一个重载的[]操作符，使得你可以使用像mat[i]这样的语法来直接访问矩阵的行。
    Matrix operator*(const Matrix& a); // 重载*操作符，以支持矩阵乘法。
    Matrix transpose(); // 返回矩阵的转置
    Matrix inverse(); // 返回矩阵的逆

    friend std::ostream& operator<<(std::ostream& s, Matrix& m); // 支持流传输
};

/////////////////////////////////////////////////////////////////////////////////////////////


#endif //__GEOMETRY_H__