#include <vector>
#include <cmath>
#include <limits>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"

const int width  = 800;
const int height = 800;
const int depth  = 255;

Model *model = NULL;
int *zBuffer = NULL;
Vec3f light_dir(0,0,-1);
Vec3f camera(0,0,3);

// 4d-->3d
Vec3f m2v(Matrix m) {
    return Vec3f(m[0][0]/m[3][0], m[1][0]/m[3][0], m[2][0]/m[3][0]);
}

// 3d-->4d
Matrix v2m(Vec3f v) {
    Matrix m(4, 1);
    m[0][0] = v.x;
    m[1][0] = v.y;
    m[2][0] = v.z;
    m[3][0] = 1.f;
    return m;
}

// 视角矩阵
Matrix viewport(int x, int y, int w, int h) {
    Matrix m = Matrix::identity(4);
    m[0][3] = x+w/2.f;
    m[1][3] = y+h/2.f;
    m[2][3] = depth/2.f;

    m[0][0] = w/2.f;
    m[1][1] = h/2.f;
    m[2][2] = depth/2.f;
    return m;
}

Vec3f barycentric(Vec3i *pts, Vec3i P) {
    Vec3f u =
            Vec3f(pts[2].x-pts[0].x, pts[1].x-pts[0].x, pts[0].x-P.x)^
            Vec3f(pts[2].y-pts[0].y, pts[1].y-pts[0].y, pts[0].y-P.y)
    ;
    if (std::abs(u.z)<1) return Vec3f(-1,1,1);
    return Vec3f(1.f-(u.x+u.y)/u.z, u.y/u.z, u.x/u.z);
}
void triangle(Vec3i t0, Vec3i t1, Vec3i t2,
              Vec2i uv0, Vec2i uv1, Vec2i uv2,
              TGAImage &image, float intensity, int *zbuffer) {
    // Compute bounding box of the triangle
    Vec3i bboxmin(image.get_width() - 1, image.get_height() - 1, 0);
    Vec3i bboxmax(0, 0, 0);
    Vec3i clamp(image.get_width() - 1, image.get_height() - 1, 0);
    bboxmin.x = std::max(0, std::min(bboxmin.x, std::min(t0.x, std::min(t1.x, t2.x))));
    bboxmin.y = std::max(0, std::min(bboxmin.y, std::min(t0.y, std::min(t1.y, t2.y))));
    bboxmax.x = std::min(clamp.x, std::max(t0.x, std::max(t1.x, t2.x)));
    bboxmax.y = std::min(clamp.y, std::max(t0.y, std::max(t1.y, t2.y)));
    Vec3i pts[3] = {t0, t1, t2};
    Vec3i P;
    for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++) {
        for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++) {
            Vec3f bc_screen = barycentric(pts, P);
            if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0) continue;  // P is outside of triangle
            P.z = bc_screen.x * t0.z + bc_screen.y * t1.z + bc_screen.z * t2.z;
            Vec2i uvP;
            uvP.x = bc_screen.x * uv0.x + bc_screen.y * uv1.x + bc_screen.z * uv2.x;
            uvP.y = bc_screen.x * uv0.y + bc_screen.y * uv1.y + bc_screen.z * uv2.y;
            int idx = P.x + P.y * image.get_width();
            if (zbuffer[idx] < P.z) {
                zbuffer[idx] = P.z;
                TGAColor color = model->diffuse(uvP);
                image.set(P.x, P.y,
                          TGAColor(color.r * intensity, color.g * intensity, color.b * intensity));
            }
        }
    }
}

Matrix lookAt(Vec3f cameraPos, Vec3f lookAt, Vec3f viewUp){
    //计算出z，根据z和up算出x，再算出y
    Vec3f gazeDirection = (cameraPos - lookAt).normalize(); // -z 轴
    Vec3f horizon = (viewUp ^ gazeDirection).normalize(); // 与相机水平的轴
    Vec3f vertical = (gazeDirection ^ horizon).normalize(); //

    Matrix rotation = Matrix::identity(4);
    Matrix translation = Matrix::identity(4);
    for (int i = 0; i < 3; i++) {
        rotation[i][3] = - lookAt[i];
        rotation[0][i] = horizon[i];
        rotation[1][i] = vertical[i];
        rotation[2][i] = gazeDirection[i];
    }
    //这样乘法的效果是先平移物体，再旋转
    Matrix res = rotation*translation;
    return res;
}


int main(int argc, char** argv) {
    model = new Model("../object/african_head.obj");
    zBuffer = new int[width*height];

    for (int i=width*height; i--; zBuffer[i] = -std::numeric_limits<float>::max());

    { // draw the model
        Matrix ModelView  = lookAt(Vec3f(2,1,3), Vec3f(0,0,1), Vec3f(0,1,0));
        Matrix Projection = Matrix::identity(4);
        Matrix ViewPort   = viewport(width/8, height/8, width*3/4, height*3/4);
        Projection[3][2] = -1.f/camera.z;

        TGAImage image(width, height, TGAImage::RGB);
        for (int i=0; i<model->nfaces(); i++) {
            std::vector<int> face = model->face(i);
            Vec3i screen_coords[3];
            Vec3f world_coords[3];
            for (int j=0; j<3; j++) {
                Vec3f v = model->vert(face[j]);
                // 视角矩阵*投影矩阵*坐标
                screen_coords[j] =  m2v(ViewPort*Projection*ModelView*v2m(v));
                world_coords[j]  = v;
            }
            // 计算法向量并且标准化
            Vec3f n = ((world_coords[2]-world_coords[0])^(world_coords[1]-world_coords[0])).normalize();
            // 计算光照
            float intensity = n*light_dir;
            if (intensity>0) {
                Vec2i uv[3];
                for (int k=0; k<3; k++) {
                    uv[k] = model->uv(i, k);
                }
                // 绘制三角形
                triangle(screen_coords[0], screen_coords[1], screen_coords[2],
                         uv[0], uv[1], uv[2], image, intensity, zBuffer);
            }
        }
        image.flip_vertically();
        image.write_tga_file("output.tga");
    }
    { // 输出zbuffer
        TGAImage zbimage(width, height, TGAImage::GRAYSCALE);
        for (int i=0; i<width; i++) {
            for (int j=0; j<height; j++) {
                zbimage.set(i, j, TGAColor(zBuffer[i+j*width], 1));
            }
        }
        zbimage.flip_vertically();
        zbimage.write_tga_file("zbuffer.tga");
    }
    delete model;
    delete [] zBuffer;
    return 0;
}