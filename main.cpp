//
//  main.h
//  Tiny-Renderer
//
//  Created by remoooo on 2023/8/30.
//

#include "tgaimage.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255);
const TGAColor blue = TGAColor(0, 0, 255, 255);

// 第二次优化的代码
void line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color) {
    // TODO: Draw a Line
}


int main(int argc, char** argv) {
    TGAImage image(100, 100, TGAImage::RGB);

    for (int i = 0; i < 1000000; ++i) {
        line(13, 20, 80, 40, image, white);
        line(20, 13, 40, 80, image, red);
        line(80, 40, 13, 20, image, blue);
    }


    image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
    image.write_tga_file("output.tga");
    return 0;
}