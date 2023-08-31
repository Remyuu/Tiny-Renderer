//
//  main.h
//  Tiny-Renderer
//
//  Created by remoooo on 2023/8/30.
//

#include <vector>
#include <cmath>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255);
const TGAColor blue = TGAColor(0, 0, 255, 255);

Model *model = NULL;
const int screenWidth  = 800;
const int screenHeight = 800;

void line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color) {
    bool steep = std::abs(x0-x1)<std::abs(y0-y1);
    if (x0>x1 || y0>y1) {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }
    int dx = x1 - x0;
    int dy = y1 - y0;
    int error2 = 0;
    if(steep) { // “陡峭”线
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

int main(int argc, char** argv) {
    TGAImage image(screenWidth, screenHeight, TGAImage::RGB);
    model = new Model("../object/african_head.obj");

    for (int i=0; i<model->nfaces(); i++) {
        std::vector<int> face = model->face(i);
        for (int j=0; j<3; j++) {
            Vec3f v0 = model->vert(face[j]);
            Vec3f v1 = model->vert(face[(j+1)%3]);
            int x0 = (v0.x+1.)*screenWidth/2.;
            int y0 = (v0.y+1.)*screenHeight/2.;
            int x1 = (v1.x+1.)*screenWidth/2.;
            int y1 = (v1.y+1.)*screenHeight/2.;
            line(x0, y0, x1, y1, image, blue);
        }
    }


    image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
    image.write_tga_file("output.tga");
    delete model;
    return 0;
}