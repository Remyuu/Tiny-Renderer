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
    const float halfWidth = screenWidth / 2.0f;
    const float halfHeight = screenHeight / 2.0f;

    TGAImage image(screenWidth, screenHeight, TGAImage::RGB);
    model = new Model("../object/african_head.obj");

    int nfaces = model->nfaces();
    for (int i = 0; i < nfaces; ++i) {
        const std::vector<int>& face = model->face(i);
        Vec3f verts[3];
        for (int j = 0; j < 3; ++j) {
            verts[j] = model->vert(face[j]);
        }
        for (int j = 0; j < 3; ++j) {
            const Vec3f& v0 = verts[j];
            const Vec3f& v1 = verts[(j + 1) % 3];
            int x0 = (v0.x + 1.0f) * halfWidth;
            int y0 = (v0.y + 1.0f) * halfHeight;
            int x1 = (v1.x + 1.0f) * halfWidth;
            int y1 = (v1.y + 1.0f) * halfHeight;
            line(x0, y0, x1, y1, image, red);
        }
    }

    image.flip_vertically();
    image.write_tga_file("output.tga");
    delete model;
    return 0;
}