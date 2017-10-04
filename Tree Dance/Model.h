/*
    Copyright (c) 2015 Seth Pendergrass. See LICENSE.
*/

#pragma once
#include <string>
#include <vector>
#include <fstream>
#include "Mesh.h"
#include "Texture.h"
#include "Animation.h"

struct Model {
    std::vector<Mesh> meshes;
    Skeleton skeleton;
    std::vector<Animation> animations;
    Material material;
    void Load(std::string filename) {
        std::ifstream s(filename, std::ios::binary);
        int size;

        s.read((char*)&size, sizeof(size));
        meshes.resize(size);
        
        for (auto& mesh : meshes) {
            s.read((char*)&size, sizeof(size));
            mesh.vertices.resize(size);
            s.read((char*)&size, sizeof(size));
            mesh.indices.resize(size);
            s.read((char*)&mesh.global_transform, 
                   sizeof(mesh.global_transform));
            s.read((char*)mesh.vertices.data(), 
                   sizeof(Vertex) * mesh.vertices.size());
            s.read((char*)mesh.indices.data(), 
                   sizeof(int) * mesh.indices.size());
        }

        s.read((char*)&size, sizeof(size));
        animations.resize(size);

        for (auto& anim : animations) {
            s.read((char*)&size, sizeof(size));
            anim.channels.resize(size);
            s.read((char*)&anim.duration, sizeof(anim.duration));

            for (auto& chan : anim.channels) {
                s.read((char*)&size, sizeof(size));
                chan.resize(size);
                s.read((char*)chan.data(), sizeof(Key) * chan.size());
            }
        }

        s.read((char*)&size, sizeof(size));
        skeleton.bones.resize(size);

        for (auto& bone : skeleton.bones) {
            s.read((char*)&bone.local_bind_pose, sizeof(bone.local_bind_pose));
            s.read((char*)&bone.inverse_global_bind_pose, 
                   sizeof(bone.inverse_global_bind_pose));
            s.read((char*)&bone.parent_index, sizeof(bone.parent_index));
        }

        Texture* textures[5] { &material.albedo, &material.normal,
            &material.roughness, &material.specular, 
            &material.displacement };

        for (int i = 0; i < 5; ++i) {
            Texture& tex = *textures[i];
            s.read((char*)&tex.width, sizeof(tex.width));
            s.read((char*)&tex.height, sizeof(tex.height));
            s.read((char*)&tex.depth, sizeof(tex.depth));
            tex.data.resize(tex.width * tex.height * tex.depth);
            s.read((char*)tex.data.data(), 
                   sizeof(char) * tex.width * tex.height * tex.depth);
        }
    }
};

struct TransformSet {
    std::vector<DirectX::XMMATRIX> world_matrices;
    std::vector<DirectX::XMMATRIX> transforms;
    int num_instances;
    ID3D11ShaderResourceView* srv_world;
    ID3D11ShaderResourceView* srv_transforms;
};