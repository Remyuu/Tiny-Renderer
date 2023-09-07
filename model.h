#ifndef __MODEL_H__
#define __MODEL_H__

#include <vector>
#include "geometry.h"
#include "tgaimage.h"

class Model {
private:
    std::vector<Vec3f> verts_; // 模型的顶点
    std::vector<std::vector<Vec3i> > faces_; // this Vec3i means vertex/uv/normal
    std::vector<Vec3f> norms_; // 存储模型的法线
    std::vector<Vec2f> uv_; // 存储模型的 UV 纹理坐标
    TGAImage diffusemap_; // 模型的漫反射纹理图像
    // load_texture() 在加载模型的时候会用到，用于加载纹理。
    void load_texture(std::string filename, const char *suffix, TGAImage &img);
public:
    Model(const char *filename); // 构造函数，从给定文件名加载模型
    ~Model(); // 析构函数，用于释放模型所占用的资源
    int nverts(); // 返回模型的顶点数量
    int nfaces(); // 返回模型的面数量
    Vec3f vert(int i); // 返回指定索引的顶点
    Vec2i uv(int iface, int nvert); // 返回指定面和指定顶点的 UV 坐标
    TGAColor diffuse(Vec2i uv); // 根据给定的 UV 坐标，从纹理图中获取颜色
    std::vector<int> face(int idx); // 返回指定索引的面信息（可能是顶点/纹理坐标/法线的索引）
};

#endif //__MODEL_H__