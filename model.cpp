#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include "model.h"

Model::Model(const char *filename) : verts_(), faces_(), texcoords_(){
    std::ifstream in;
    in.open (filename, std::ifstream::in);
    if (in.fail()) return;
    std::string line;
    while (!in.eof()) {
        std::getline(in, line);
        std::istringstream iss(line.c_str());
        char trash;
        if (!line.compare(0, 2, "v ")) {
            iss >> trash;
            Vec3f v;
            for (int i=0;i<3;i++) iss >> v[i];
            verts_.push_back(v);
        } else if (!line.compare(0, 3, "vt ")) {
            iss >> trash >> trash;
            Vec2f tc;
            for (int i = 0; i < 2; i++) iss >> tc[i];
            texcoords_.push_back(tc);
        } else if (!line.compare(0, 2, "f ")) {
            Face face;
            int itrash, idx, texIdx;
            iss >> trash;
            while (iss >> idx >> trash >> texIdx >> trash >> itrash) {
                idx--;
                texIdx--;
                face.vertexIndices.push_back(idx);
                face.texcoordIndices.push_back(texIdx);
            }
            faces_.push_back(face);
        }
    }
    std::cerr << "# v# " << verts_.size() << " f# " << faces_.size() << " vt# " << texcoords_.size() << "\n" << std::endl;
}

Model::~Model() {
}

int Model::nverts() {
    return (int)verts_.size();
}

int Model::nfaces() {
    return (int)faces_.size();
}

Face Model::face(int idx) {
    return faces_[idx];
}

Vec3f Model::vert(int i) {
    return verts_[i];
}
Vec2f& Model::getTexCoord(int index) {
    return texcoords_[index];
}