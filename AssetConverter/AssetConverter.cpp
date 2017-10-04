/*
    Copyright (c) 2015 Seth Pendergrass. See LICENSE.
*/

#include <fstream>
#include <string>
#include <vector>
#include "FbxConvert.h"
#include "ImageConvert.h"
using namespace std;

void WriteModel(string filename, Model& model);

int main(int argc, char** argv) {
    string name = argv[1];

    Model model;
    ConvertFBX(name + ".fbx", &model);

    ConvertImage(name + ".png", &model.material.albedo);
    ConvertImage(name + "_n.png", &model.material.normal);
    ConvertImage(name + "_s.png", &model.material.specular, true);
    ConvertImage(name + "_r.png", &model.material.roughness, true);
    ConvertImage(name + "_d.png", &model.material.displacement, true);

    WriteModel(name + ".model", model);

    return 0;
}

void WriteModel(string filename, Model& model) {
    ofstream of(filename, ios::binary);

    int size = model.meshes.size();
    of.write((char*)&size, sizeof(size));

    for (auto& mesh : model.meshes) {
        size = mesh.vertices.size();
        of.write((char*)&size, sizeof(size));
        size = mesh.indices.size();
        of.write((char*)&size, sizeof(size));
        float temp[16];
        for (int i = 0; i < 16; ++i)
            temp[i] = mesh.global_transform[i / 4][i % 4];
        of.write((char*)&temp, sizeof(temp));
        of.write((char*)mesh.vertices.data(), 
                 sizeof(Vertex) * mesh.vertices.size());
        of.write((char*)mesh.indices.data(), sizeof(int) * mesh.indices.size());
    }
    
    size = model.animations.size();
    of.write((char*)&size, sizeof(size));

    for (auto& anim : model.animations) {
        size = anim.channels.size();
        of.write((char*)&size, sizeof(size));
        of.write((char*)&anim.duration, sizeof(anim.duration));

        for (auto& chan : anim.channels) {
            size = chan.size();
            of.write((char*)&size, sizeof(size));
            of.write((char*)chan.data(), sizeof(Key) * chan.size());
        }
    }

    size = model.skeleton.bones.size();
    of.write((char*)&size, sizeof(size));
    
    for (auto& bone : model.skeleton.bones) {
        float temp[16];
        for (int i = 0; i < 16; ++i)
            temp[i] = bone.local_bind_pose[i / 4][i % 4];
        of.write((char*)&temp, sizeof(temp));
        for (int i = 0; i < 16; ++i)
            temp[i] = bone.inverse_global_bind_pose[i / 4][i % 4];
        of.write((char*)&temp, sizeof(temp));
        of.write((char*)&bone.parent_index, sizeof(bone.parent_index));
    }

    Texture* textures[5] { &model.material.albedo, &model.material.normal,
        &model.material.roughness, &model.material.specular, 
        &model.material.displacement };

    for (int i = 0; i < 5; ++i) {
        Texture& tex = *textures[i];
        of.write((char*)&tex.width, sizeof(tex.width));
        of.write((char*)&tex.height, sizeof(tex.height));
        of.write((char*)&tex.depth, sizeof(tex.depth));
        of.write((char*)tex.data.data(), 
                 sizeof(char) * tex.width * tex.height * tex.depth);
    }
}