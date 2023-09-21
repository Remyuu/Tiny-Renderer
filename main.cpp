#include "geometry.h"
#include <iostream>

int main() {

    Vec3f v_f(1.2f,2.5f,3.8f);
    Vec3i v_f2i(v_f);
    std::cout << v_f2i << std::endl;

    return 0;
}