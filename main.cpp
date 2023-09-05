//
//  main.h
//  Tiny-Renderer
//
//  Created by remoooo on 2023/8/30.
//

#include <vector>
#include <cmath>
#include <iostream>
#include "tgaimage.h"
#include "geometry.h"
#include "model.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255);
const TGAColor blue = TGAColor(0, 0, 255, 255);

Model *model = NULL;

const int screenWidth  = 500;
const int screenHeight = 500;

Vec3f barycentric(Vec3f A, Vec3f B, Vec3f C, Vec3f P) {
    Vec3f s[2];
    for (int i=2; i--; ) {
        s[i][0] = C[i]-A[i];
        s[i][1] = B[i]-A[i];
        s[i][2] = A[i]-P[i];
    }
    Vec3f u = cross(s[0], s[1]);
    if (std::abs(u[2])>1e-2) // dont forget that u[2] is integer. If it is zero then triangle ABC is degenerate
        return Vec3f(1.f-(u.x+u.y)/u.z, u.y/u.z, u.x/u.z);
    return Vec3f(-1,1,1); // in this case generate negative coordinates, it will be thrown away by the rasterizator
}

// 重心坐标的方法 - 光栅化三角形
void triangleRaster(Vec3f v0, Vec3f v1, Vec3f v2, float *zBuffer, TGAImage &image, TGAColor color){
    Vec3f* pts[] = {&v0, &v1, &v2};// Pack
    // Find The Bounding Box
    Vec2f boundingBoxMin( std::numeric_limits<float>::max(),  std::numeric_limits<float>::max());
    Vec2f boundingBoxMax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
    Vec2f clamp(image.get_width()-1, image.get_height()-1);
    for (int i=0; i<3; i++) {
        boundingBoxMin.x = std::max(0.f, std::min(boundingBoxMin.x, pts[i]->x));
        boundingBoxMin.y = std::max(0.f, std::min(boundingBoxMin.y, pts[i]->y));
        boundingBoxMax.x = std::min(clamp.x, std::max(boundingBoxMax.x, pts[i]->x));
        boundingBoxMax.y = std::min(clamp.y, std::max(boundingBoxMax.y, pts[i]->y));
    }

    // For Loop To Iterate Over All Pixels Within The Bounding Box
    Vec3f pixel;// 将深度值打包到pixel的z分量上
    for (pixel.x = boundingBoxMin.x; pixel.x <= boundingBoxMax.x; pixel.x++) {
        for (pixel.y = boundingBoxMin.y; pixel.y <= boundingBoxMax.y; pixel.y++) {
            Vec3f bc = barycentric(v0, v1, v2, pixel);// Screen Space
            if (bc.x<0 || bc.y<0 || bc.z<0 ) continue;
            // HIGHLIGHT: Finished The Z-Buffer
            pixel.z = 0;
            pixel.z = bc.x*v0.z+bc.y+v1.z+bc.z+v2.z;// 通过重心坐标插值计算当前Shading Point的深度值
            if(zBuffer[int(pixel.x+pixel.y*screenWidth)]<pixel.z) {
                zBuffer[int(pixel.x + pixel.y * screenWidth)] = pixel.z;
                image.set(pixel.x, pixel.y,color);
            }
        }
    }
}

TGAColor getTextureColor(TGAImage &texture, float u, float v) {
    // 纹理坐标限制在(0, 1)
    u = std::max(0.0f, std::min(1.0f, u));
    v = std::max(0.0f, std::min(1.0f, v));
    // 将u, v坐标乘以纹理的宽度和高度，以获取纹理中的像素位置
    int x = u * texture.get_width();
    int y = v * texture.get_height();
    // 从纹理中获取颜色
    TGAColor color = texture.get(x, y);
    // tga使用的是BGRA通道
    return TGAColor(color[2],color[1],color[0], 255);
}

// 带贴图 - 光栅化三角形
void triangleRasterWithTexture(Vec3f v0, Vec3f v1, Vec3f v2,
                               Vec2f vt0, Vec2f vt1, Vec2f vt2,// 纹理贴图
                               float *zBuffer, TGAImage &image,
                               TGAImage &texture){
    Vec3f* pts[] = {&v0, &v1, &v2};// Pack
    // Find The Bounding Box
    Vec2f boundingBoxMin( std::numeric_limits<float>::max(),  std::numeric_limits<float>::max());
    Vec2f boundingBoxMax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
    Vec2f clamp(image.get_width()-1, image.get_height()-1);
    for (int i=0; i<3; i++) {
        boundingBoxMin.x = std::max(0.f, std::min(boundingBoxMin.x, pts[i]->x));
        boundingBoxMin.y = std::max(0.f, std::min(boundingBoxMin.y, pts[i]->y));
        boundingBoxMax.x = std::min(clamp.x, std::max(boundingBoxMax.x, pts[i]->x));
        boundingBoxMax.y = std::min(clamp.y, std::max(boundingBoxMax.y, pts[i]->y));
    }
    // For Loop To Iterate Over All Pixels Within The Bounding Box
    Vec3f pixel;// 将深度值打包到pixel的z分量上
    for (pixel.x = boundingBoxMin.x; pixel.x <= boundingBoxMax.x; pixel.x++) {
        for (pixel.y = boundingBoxMin.y; pixel.y <= boundingBoxMax.y; pixel.y++) {
            Vec3f bc = barycentric(v0, v1, v2, pixel);// Screen Space
            if (bc.x<0 || bc.y<0 || bc.z<0 ) continue;
            pixel.z = 0;
            pixel.z  = bc.x*v0.z+bc.y*v1.z+bc.z*v2.z;// 通过重心坐标插值计算当前Shading Point的深度值
            Vec2f uv = bc.x*vt0 +bc.y*vt1 +bc.z*vt2;
            if(zBuffer[int(pixel.x+pixel.y*screenWidth)]<pixel.z) {
                zBuffer[int(pixel.x + pixel.y * screenWidth)] = pixel.z;
                image.set(pixel.x, pixel.y,getTextureColor(texture, uv.x, 1-uv.y));
            }
        }
    }
}

Vec3f world2screen(Vec3f v) {
    return Vec3f(int((v.x+1.)*screenWidth/2.+.5), int((v.y+1.)*screenHeight/2.+.5), v.z);
}

int main(int argc, char** argv) {
    TGAImage image(screenWidth, screenHeight, TGAImage::RGB);
    model = new Model("../object/african_head.obj");
    TGAImage texture;
    if(texture.read_tga_file("../object/african_head_diffuse.tga")){
        std::cout << "Image successfully loaded!" << std::endl;
        // 可以做一些图像处理
    } else {
        std::cerr << "Error loading the image." << std::endl;
    }

    float *zBuffer = new float[screenWidth*screenHeight];
    for (int i=screenWidth*screenHeight; i--; zBuffer[i] = -std::numeric_limits<float>::max());

    for (int i=0; i<model->nfaces(); i++) {
        Face face = model->face(i);
        Vec3f screen_coords[3], world_coords[3];
        Vec2f tex_coords[3];
        for (int j=0; j<3; j++) {
            world_coords[j]  = model->vert(face.vertexIndices[j]);
            screen_coords[j] = world2screen(world_coords[j]);
            tex_coords[j] = model->getTexCoord(face.texcoordIndices[j]);
        }
        triangleRasterWithTexture(screen_coords[0], screen_coords[1], screen_coords[2],
                                  tex_coords[0],tex_coords[1],tex_coords[2],
                                  zBuffer, image, texture);
    }
    image.flip_vertically();
    image.write_tga_file("output.tga");
    delete model;
    delete[] zBuffer;
    return 0;
}