#include "geometry.h"
#include <iostream>

int main() {
    // 使用整数向量测试
    Vec3f v3(1.0f, 2.0f, 3.0f);
    Vec3f v4(2.0f, 3.0f, 4.0f);
    float scalar = 2;

    Vec3f sumInt = v3 + v4;
    sumInt.print();

    float dotProductInt = v3.dot(v4);
    std::cout << "v3 \\dot v4 = " << dotProductInt << "\n";

    Vec3f crossProduct = v3.cross(v4);
    std::cout << "v3 x v4 = "; crossProduct.print();

    Vec3f scaleVec = v3 * scalar;
    std::cout << "v3 x 2 = "; scaleVec.print();

    std::cout << "||v3|| = " << v3.magnitude() << std::endl;
    std::cout << "|v3| = "; v3.normalize().print();

    std::cout << "(Using OS Test)v3.Normalize = " << v3.normalize() << std::endl;


    return 0;
}