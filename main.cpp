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
#include "model.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255);
const TGAColor blue = TGAColor(0, 0, 255, 255);

Model *model = NULL;
const int screenWidth  = 800;
const int screenHeight = 800;

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

// 2.1.3 的方法 - 光栅化三角形
void triangleRaster_2_1_3(Vec2i v0, Vec2i v1, Vec2i v2, TGAImage &image, TGAColor color) {
    if (v0.v > v1.v) std::swap(v0, v1);
    if (v0.v > v2.v) std::swap(v0, v2);
    if (v1.v > v2.v) std::swap(v1, v2);

    // Helper function to compute the intersection of the line and a scanline
    auto interpolate = [](int y, Vec2i v1, Vec2i v2) -> int {
        if (v1.v == v2.v) return v1.u;
        return v1.u + (v2.u - v1.u) * (y - v1.v) / (v2.v - v1.v);
    };

    for (int y = v0.v; y <= v2.v; y++) {
        // Intersect triangle sides with scanline
        int xa = interpolate(y, v0, v2); // Intersection with line v0-v2
        int xb = (y < v1.v) ? interpolate(y, v0, v1) : interpolate(y, v1, v2); // Depending on current half

        if (xa > xb) std::swap(xa, xb);

        // Draw horizontal line
        for (int x = xa; x <= xb; x++) {
            image.set(x, y, color);
        }
    }
}

Vec3f barycentric(Vec2i v0, Vec2i v1, Vec2i v2, Vec2i pixel){
    // v0, v1, v2 correspond to ABC
    Vec3f u = Vec3f(v1.x-v0.x,// AB_x
                    v2.x-v0.x,// AC_x
                    v0.x-pixel.x)// PA_x
              ^
              Vec3f(v1.y-v0.y,
                    v2.y-v0.y,
                    v0.y-pixel.y);
    if (std::abs(u.z)<1) return Vec3f(-1,1,1);
    return Vec3f(1.f-(u.x+u.y)/u.z, u.y/u.z, u.x/u.z);
}
// 重心坐标的方法 - 光栅化三角形
void triangleRaster(Vec2i v0, Vec2i v1, Vec2i v2, TGAImage &image, TGAColor color){
    // Find The Bounding Box
    Vec2i* pts[] = {&v0, &v1, &v2};// Pack
    Vec2i boundingBoxMin(image.get_width()-1,  image.get_height()-1);
    Vec2i boundingBoxMax(0, 0);
    Vec2i clamp(image.get_width()-1, image.get_height()-1);
    for (int i=0; i<3; i++) {
        boundingBoxMin.x = std::max(0, std::min(boundingBoxMin.x, pts[i]->x));
        boundingBoxMin.y = std::max(0, std::min(boundingBoxMin.y, pts[i]->y));

        boundingBoxMax.x = std::min(clamp.x, std::max(boundingBoxMax.x, pts[i]->x));
        boundingBoxMax.y = std::min(clamp.y, std::max(boundingBoxMax.y, pts[i]->y));
    }

    // For Loop To Iterate Over All Pixels Within The Bounding Box
    Vec2i pixel;
    for (pixel.x = boundingBoxMin.x; pixel.x <= boundingBoxMax.x; pixel.x++) {
        for (pixel.y = boundingBoxMin.y; pixel.y <= boundingBoxMax.y; pixel.y++) {
            Vec3f bc = barycentric(v0, v1, v2, pixel);
            if (bc.x<0 || bc.y<0 || bc.z<0 ) continue;
            image.set(pixel.x, pixel.y, color);
        }
    }
}


int main(int argc, char** argv) {
    const float halfWidth = screenWidth / 2.0f;
    const float halfHeight = screenHeight / 2.0f;

    TGAImage image(screenWidth, screenHeight, TGAImage::RGB);
    model = new Model("../object/african_head.obj");

//    for (int i=0; i<model->nfaces(); i++) {
//        std::vector<int> face = model->face(i);
//        Vec2i screen_coords[3];
//        for (int j=0; j<3; j++) {
//            Vec3f world_coords = model->vert(face[j]);
//            screen_coords[j] = Vec2i((world_coords.x+1.)*halfWidth, (world_coords.y+1.)*halfHeight);
//        }
//        triangleRaster(screen_coords[0], screen_coords[1], screen_coords[2], image, TGAColor(rand()%255, rand()%255, rand()%255, 255));
//    }
    Vec3f light_dir(0,0,-1); // define light_dir

    for (int i=0; i<model->nfaces(); i++) {
        std::vector<int> face = model->face(i);
        Vec2i screen_coords[3];
        Vec3f world_coords[3];
        for (int j=0; j<3; j++) {
            Vec3f v = model->vert(face[j]);
            screen_coords[j] = Vec2i((v.x+1.)*halfWidth, (v.y+1.)*halfHeight);
            world_coords[j]  = v;
        }
        Vec3f n = (world_coords[2]-world_coords[0])^(world_coords[1]-world_coords[0]);
        n.normalize();
        float intensity = n*light_dir;
        if (intensity>0) {
            triangleRaster(screen_coords[0], screen_coords[1], screen_coords[2], image, TGAColor(intensity*255, intensity*255, intensity*255, 255));
        }
    }

    image.flip_vertically();
    image.write_tga_file("output.tga");
    delete model;
    return 0;
}