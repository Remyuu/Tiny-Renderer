#include "geometry.h"
#include <iostream>

int main() {
    // 使用整数向量测试
    Vec3i v3(1, 2, 3);
    Vec3i v4(2, 3, 4);

    Vec3i sumInt = v3 + v4;
    sumInt.print();

    int dotProductInt = v3.dot(v4);
    std::cout << "v3 \\dot v4 = " << dotProductInt << "\n";

    Vec3i crossProduct = v3.cross(v4);
    std::cout << "v3 x v4 = "; crossProduct.print();

    return 0;
}