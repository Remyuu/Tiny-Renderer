//
// Created by 张杰 on 2023/8/31.
//

#ifndef __MODEL_H__
#define __MODEL_H__

#include <vector>
#include "geometry.h"

struct Face {
    std::vector<int> vertexIndices;
    std::vector<int> texcoordIndices;
};

class Model {
private:
    std::vector<Vec3f> verts_;
//    std::vector< std::vector<int> > faces_;
    std::vector<Face> faces_;
    std::vector<Vec2f> texcoords_;
public:
    Model(const char *filename);
    ~Model();
    int nverts();
    int nfaces();
    Vec3f vert(int i);
    Face face(int idx);
    Vec2f& getTexCoord(int index);

    void debugPrint();
};

#endif //__MODEL_H__
