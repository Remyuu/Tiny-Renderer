#include <vector>
#include <iostream>

#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#include "our_gl.h"

Model *model     = NULL;
const int width  = 800;
const int height = 800;

Vec3f light_dir(1,1,1);
Vec3f       eye(0,-1,3);
Vec3f    center(0,0,0);
Vec3f        up(0,1,0);

//struct Shader : public IShader {
//    Vec3f          varying_intensity; // written by vertex shader, read by fragment shader
//    mat<2,3,float> varying_uv;        // same as above
//
//    Vec4f vertex(int iface, int nthvert) override {
//        varying_uv.set_col(nthvert, model->uv(iface, nthvert));
//        varying_intensity[nthvert] = std::max(0.f, model->normal(iface, nthvert)*light_dir); // get diffuse lighting intensity
//        Vec4f gl_Vertex = embed<4>(model->vert(iface, nthvert)); // read the vertex from .obj file
//        return Viewport*Projection*ModelView*gl_Vertex; // transform it to screen coordinates
//    }
//
//    bool fragment(Vec3f bar, TGAColor &color) override {
//        float intensity = varying_intensity*bar;   // interpolate intensity for the current pixel
//        Vec2f uv = varying_uv*bar;                 // interpolate uv for the current pixel
//        color = model->diffuse(uv)*intensity;      // well duh
//        return false;                              // no, we do not discard this pixel
//    }
//};

struct Blinn_Phong_Shader : public IShader {
    mat<2,3,float> varying_uv;  // same as above
    mat<4,4,float> uniform_M;   //  Projection*ModelView
    mat<4,4,float> uniform_MIT; // (Projection*ModelView).invert_transpose()

    Vec4f vertex(int iface, int nthvert) override {
        varying_uv.set_col(nthvert, model->uv(iface, nthvert));
        Vec4f gl_Vertex = embed<4>(model->vert(iface, nthvert)); // read the vertex from .obj file
        return Viewport*Projection*ModelView*gl_Vertex; // transform it to screen coordinates
    }
    bool fragment(Vec3f bar, TGAColor &color) override {
        Vec2f uv = varying_uv*bar;
        Vec3f n = proj<3>(uniform_MIT*embed<4>(model->normal(uv))).normalize(); // normal
        Vec3f l = proj<3>(uniform_M  *embed<4>(light_dir        )).normalize(); // light direction
        Vec3f v = Vec3f(0, 0, -1); // simplified view direction
        Vec3f h = (l + v).normalize(); // halfway vector

        float spec = pow(std::max(0.f, n*h), model->specular(uv));
        float diff = std::max(0.f, n*l);
        TGAColor c = model->diffuse(uv);
        color = c;
        for (int i=0; i<3; i++) color[i] = std::min<float>(5 + c[i]*(diff + .6*spec), 255);
        return false;
    }
};

struct GouraudShader : public IShader {
    Vec3f varying_intensity; // written by vertex shader, read by fragment shader

    Vec4f vertex(int iface, int nthvert) override {
        Vec4f gl_Vertex = embed<4>(model->vert(iface, nthvert)); // read the vertex from .obj file
        gl_Vertex = Viewport * Projection * ModelView * gl_Vertex;     // transform it to screen coordinates
        varying_intensity[nthvert] = std::max(0.f, model->normal(iface, nthvert)*light_dir); // get diffuse lighting intensity
        return gl_Vertex;
    }

    bool fragment(Vec3f bar, TGAColor &color) override {
//        float intensity = varying_intensity*bar;   // interpolate intensity for the current pixel
//        color = TGAColor(255, 255, 255)*intensity; // well duh
//        return false;                              // no, we do not discard this pixel

//        float intensity = varying_intensity*bar;
//        if (intensity>.85) intensity = 1;
//        else if (intensity>.60) intensity = .80;
//        else if (intensity>.45) intensity = .60;
//        else if (intensity>.30) intensity = .45;
//        else if (intensity>.15) intensity = .30;
//        else intensity = 0;
//        color = TGAColor(255, 155, 0)*intensity;
//        return false;

//        float t = varying_intensity*bar;
//        color = TGAColor(
//                128 + 127 * std::sin(t),
//                128 + 127 * std::sin(t + 2.f/3.f * 3.14159f),
//                128 + 127 * std::sin(t + 4.f/3.f * 3.14159f)
//        );
//        return false;

//        float t = varying_intensity*bar;
//        float noise = rand() % 100 / 100.0;
//        if (noise > 0.9) {
//            color = TGAColor(255, 255, 255);
//        } else if (noise < 0.1) {
//            color = TGAColor(0, 0, 0);
//        } else {
//            color = TGAColor(155, 155, 155) * t;
//        }
//        return false;


//        float intensity = varying_intensity * bar;
//        color = TGAColor(
//                255 * intensity,
//                (int)(160 * std::sqrt(intensity)),
//                0
//        );
//        return false;
        float noise = rand() % 100 / 100.0;
        if (noise > 0.88) {
            color = TGAColor(255, 255, 255);
        } else {
            color = TGAColor(0, 0, 0);
        }
        return false;
    }
};

int main(int argc, char** argv) {
    model = new Model("../object/african_head/african_head.obj");

    lookat(eye, center, up);
    viewport(width/8, height/8, width*3/4, height*3/4);
    projection(-1.f/(eye-center).norm());
    light_dir.normalize();

    TGAImage image  (width, height, TGAImage::RGB);
    TGAImage zbuffer(width, height, TGAImage::GRAYSCALE);

    Blinn_Phong_Shader shader;
    shader.uniform_M   =  Projection*ModelView;
    shader.uniform_MIT = (Projection*ModelView).invert_transpose();
    for (int i=0; i<model->nfaces(); i++) {
        Vec4f screen_coords[3];
        for (int j=0; j<3; j++) {
            screen_coords[j] = shader.vertex(i, j);
        }
        triangle(screen_coords, shader, image, zbuffer);
    }
//    for (int i=0; i<model->nfaces(); i++) {
//        Vec4f screen_coords[3];
//        for (int j=0; j<3; j++) {
//            screen_coords[j] = shader.vertex(i, j);
//        }
//        triangle(screen_coords, shader, image, zbuffer);
//    }

    image.  flip_vertically(); // to place the origin in the bottom left corner of the image
    zbuffer.flip_vertically();
    image.  write_tga_file("output.tga");
    zbuffer.write_tga_file("zbuffer.tga");

    delete model;
    return 0;
}
