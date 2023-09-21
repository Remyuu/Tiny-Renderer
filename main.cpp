#include "geometry.h"
#include <iostream>

int main() {

    Vec3i v_i(1.2,2.5,3.8);
    Vec3f v_f(1.2,2.5,3.8);
    Vec3i v_f2i(v_f);

    v_i.print();
    v_f.print();
    v_f2i.print();

    Vec<int, 3> intVec(1, 2, 3);
    Vec<float, 3> floatVec(intVec);  // 自动转换
    Vec<double, 3> doubleVec(intVec);  // 自动转换

    Vec<double, 3> vvv(1.9, 2, 3);
    Vec<int, 3> finalTest(vvv);

    floatVec.print();
    doubleVec.print();
    finalTest.print();
    return 0;
}