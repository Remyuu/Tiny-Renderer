//
//  main.h
//  Tiny-Renderer
//
//  Created by remoooo on 2023/8/30.
//

#include <vector>
#include <cmath>
#include "tgaimage.h"
#include "geometry.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255);
const TGAColor blue = TGAColor(0, 0, 255, 255);

const int screenWidth  = 51;
const int screenHeight = 51;

void line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color) {
    int error2 = 0;
    if(std::abs(x0-x1)<std::abs(y0-y1)) { // “陡峭”线
        if (y0>y1) {
            std::swap(x0, x1);
            std::swap(y0, y1);
        }
        int dx = x1 - x0;
        int dy = y1 - y0;
        int dError2 = std::abs(dx) * 2;
        int x = x0;
        for (int y = y0; y <= y1; y++) {
            image.set(x, y, color);
            error2 += dError2;
            if (error2>dy) {
                x += (x1>x0?1:-1);
                error2 -= dy * 2;
            }
        }
    }else { // “平缓”线
        if (x0>x1) {
            std::swap(x0, x1);
            std::swap(y0, y1);
        }
        int dx = x1 - x0;
        int dy = y1 - y0;
        int dError2 = std::abs(dy) * 2;
        int y = y0;
        for (int x = x0; x <= x1; x++) {
            image.set(x, y, color);
            error2 += dError2;
            if (error2>dx) {
                y += (y1>y0?1:-1);
                error2 -= dx*2;
            }
        }
    }
}

void triangleLine(Vec2i v0, Vec2i v1, Vec2i v2, TGAImage &image, TGAColor color){
    line(v0.u, v0.v, v1.u, v1.v, image, color);
    line(v0.u, v0.v, v2.u, v2.v, image, color);
    line(v1.u, v1.v, v2.u, v2.v, image, color);
}

void triangleRaster(Vec2i v0, Vec2i v1, Vec2i v2, TGAImage &image, TGAColor color){
    // TODO: 绘制一个实心的三角形

}

int main(int argc, char** argv) {
    TGAImage image(screenWidth, screenHeight, TGAImage::RGB);

//    triangleLine(Vec2i(0,0),Vec2i(25,25),Vec2i(50,16),image,red);

    triangleRaster(Vec2i(0,0),Vec2i(25,25),Vec2i(50,0),image,red);

    image.flip_vertically();
    image.write_tga_file("output.tga");
    return 0;
}