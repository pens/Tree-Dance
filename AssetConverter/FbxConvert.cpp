#include "FbxConvert.h"
#include <stack>
#include <unordered_map>
#include <fbxsdk.h>
using namespace std;

struct CPWeight {
    CPWeight() {
        for (auto i = 0; i < 4; ++i) {
            boneIdxs[i] = -1;
            weights[i] = 0;
        }
    }
    int boneIdxs[4];
    float weights[4];
};

template<class T>
T GetMeshElements(int ctrl_pt_id, int vert_id, 
                  FbxLayerElementTemplate<T>* element);

void BuildSkeleton(FbxNode* root, unordered_map<FbxNode*, int>& bone_to_idx, 
                   Skeleton& skeleton);

void CalculateWeights(FbxMesh* fbx_mesh, 
                      unordered_map<FbxNode*, int>& bone_to_idx, 
                      Skeleton& skeleton, vector<CPWeight>& weights);

void ReadMesh(FbxMesh* fbx_mesh, unordered_map<FbxNode*, int>& bone_to_idx,
              Skeleton& skeleton, Mesh& mesh);

void EvaluateAnimation(FbxAnimStack* stack, Skeleton& skeleton,
                       Animation& animation);

inline FbxAMatrix GetGeoMat(FbxNode* node) {
    return FbxAMatrix(node->GetGeometricTranslation(FbxNode::eSourcePivot),
        node->GetGeometricRotation(FbxNode::eSourcePivot),
        node->GetGeometricScaling(FbxNode::eSourcePivot));
}

inline FbxAMatrix ToLeftHand(FbxAMatrix mat) {
    for (int i = 0; i < 4; ++i)
        swap(mat[1][i], mat[2][i]);
    for (int i = 0; i < 4; ++i)
        swap(mat[i][1], mat[i][2]);
    return mat;
}

void ConvertFBX(string filename, Model* model) {
    FbxManager* sdk_manager = FbxManager::Create();
    FbxIOSettings* io_settings = FbxIOSettings::Create(sdk_manager, IOSROOT);
    FbxImporter* importer = FbxImporter::Create(sdk_manager, "");
    importer->Initialize(filename.c_str(), -1, sdk_manager->GetIOSettings());

    FbxScene* scene = FbxScene::Create(sdk_manager, "scene");
    importer->Import(scene);
    importer->Destroy();
    FbxGeometryConverter converter(sdk_manager);
    if (!converter.Triangulate(scene, true))
        return;

    //Skeleton
    unordered_map<FbxNode*, int> bone_to_idx;
    Skeleton skeleton;
    BuildSkeleton(scene->GetRootNode(), bone_to_idx, skeleton);

    //Meshes
    vector<Mesh> meshes(scene->GetMemberCount<FbxMesh>());
    for (int i = 0; i < scene->GetMemberCount<FbxMesh>(); ++i) {
        FbxMesh* mesh = scene->GetMember<FbxMesh>(i);
        ReadMesh(mesh, bone_to_idx, skeleton, meshes[i]);
    }

    //Animations
    vector<Animation> anims(scene->GetMemberCount<FbxAnimStack>());
    for (int i = 0; i < scene->GetMemberCount<FbxAnimStack>(); ++i) {
        FbxAnimStack* stack = scene->GetMember<FbxAnimStack>(i);
        scene->SetCurrentAnimationStack(stack);
        EvaluateAnimation(stack, skeleton, anims[i]);
    }

    model->animations = anims;
    model->skeleton = skeleton;
    model->meshes = meshes;
}

template<class T>
T GetMeshElements(int ctrl_pt_id, int vert_id, 
                  FbxLayerElementTemplate<T>* element) {
    int index;
    switch (element->GetMappingMode()) {
    case FbxGeometryElement::eByControlPoint:
        index = ctrl_pt_id;
        break;
    case FbxGeometryElement::eByPolygonVertex:
        index = vert_id; 
        break;
    default:
        throw exception("Unsupported vertex element mapping mode.");
    }
    if (element->GetReferenceMode() == FbxGeometryElement::eIndexToDirect)
        index = element->GetIndexArray().GetAt(index);
    else if (element->GetReferenceMode() != FbxGeometryElement::eDirect)
        throw exception("Unsupported vertex elemenet reference mode.");
    return element->GetDirectArray().GetAt(index);
}

void BuildSkeleton(FbxNode* root, unordered_map<FbxNode*, int>& bone_to_idx, 
                   Skeleton& skeleton) {
    stack<pair<FbxNode*, int>> children{};
    children.push({ root, -1 });

    while (!children.empty()) {
        FbxNode* child = children.top().first;
        int parent = children.top().second;
        children.pop();
        
        FbxNodeAttribute* attr = child->GetNodeAttribute();
        if (attr != nullptr && attr->GetAttributeType() == 
            FbxNodeAttribute::eSkeleton) {
            skeleton.bones.push_back({ {}, {}, parent, child });
            bone_to_idx[child] = skeleton.bones.size() - 1;
        }

        for (auto i = 0; i < child->GetChildCount(); ++i) {
            children.push({ child->GetChild(i), skeleton.bones.size() - 1 });
        }
    }

    //Blender assigns armature name to parent node of skeleton root
    //if (!skeleton.bones.empty())
    //    skeleton.name = skeleton.bones[0].node->GetParent()->GetName();
}

void CalculateWeights(FbxMesh* fbx_mesh, 
                      unordered_map<FbxNode*, int>& bone_to_idx, 
                      Skeleton& skeleton, vector<CPWeight>& weights) {
    for (auto i = 0; i < fbx_mesh->GetDeformerCount(FbxDeformer::eSkin); ++i) {
        FbxSkin* skin = (FbxSkin*)fbx_mesh->GetDeformer(i, FbxDeformer::eSkin);

        for (auto j = 0; j < skin->GetClusterCount(); ++j) {
            FbxCluster* cluster = skin->GetCluster(j);

            Bone& bone = skeleton.bones[bone_to_idx.at(cluster->GetLink())];
            FbxAMatrix link;
            FbxAMatrix transform;
            cluster->GetTransformLinkMatrix(link);
            cluster->GetTransformMatrix(transform);

            bone.inverse_global_bind_pose = 
                ToLeftHand(link.Inverse() * transform);


            for (auto k = 0; k < cluster->GetControlPointIndicesCount(); ++k) {
                int cpIndex = cluster->GetControlPointIndices()[k];
                float cpWeight = cluster->GetControlPointWeights()[k];

                int minIdx = -1;
                int min = 0;
                for (auto l = 0; l < 4; ++l) {
                    int boneIdx = weights[cpIndex].boneIdxs[l];
                    float boneWeight = weights[cpIndex].weights[l];
                    if (boneIdx == -1) {
                        minIdx = l;
                        break;
                    }
                    else if (boneWeight < cpWeight && 
                             (minIdx == -1 || boneWeight < min)) {
                        minIdx = l;
                        min = boneWeight;
                    }
                }
                if (minIdx != -1) {
                    weights[cpIndex].boneIdxs[minIdx] = 
                        bone_to_idx.at(cluster->GetLink());
                    weights[cpIndex].weights[minIdx] = cpWeight;
                }
            }
        }
    }
}

void ReadMesh(FbxMesh* fbx_mesh, unordered_map<FbxNode*, int>& bone_to_idx,
              Skeleton& skeleton, Mesh& mesh) {
    if (!fbx_mesh->GenerateTangentsDataForAllUVSets())
        throw exception("Vertex tangent data is not available.");
    if (fbx_mesh->GetElementUV() == nullptr)
        throw exception("Vertex UV data is not available.");
    if (!GetGeoMat(fbx_mesh->GetNode()).IsIdentity())
        throw new exception("Mesh geometry transformation it not identity.");

    //mesh.name = fbx_mesh->GetName();
    mesh.global_transform = 
        ToLeftHand(fbx_mesh->GetNode()->EvaluateGlobalTransform().Inverse());

    vector<CPWeight> weights(fbx_mesh->GetControlPointsCount());
    CalculateWeights(fbx_mesh, bone_to_idx, skeleton, weights);

    for (auto i = 0; i < fbx_mesh->GetPolygonCount(); ++i) {

        auto polySize = fbx_mesh->GetPolygonSize(i);
        for (auto j = 0; j < polySize; ++j) {
            int ctrlPtIdx = fbx_mesh->GetPolygonVertex(i, j);
            int vertIdx = polySize * i + j;

            Vertex vert{};
            FbxVector4 pos = fbx_mesh->GetControlPointAt(ctrlPtIdx);
            FbxVector4 norm = GetMeshElements<FbxVector4>(ctrlPtIdx, vertIdx,
                                                  fbx_mesh->GetElementNormal());
            FbxVector4 tan = GetMeshElements<FbxVector4>(ctrlPtIdx, vertIdx, 
                                                 fbx_mesh->GetElementTangent());
            for (int i = 0; i < 3; ++i) {
                vert.position[i] = pos[i];
                vert.normal[i] = norm[i];
                vert.tangent[i] = tan[i];
            }

            swap(vert.position[1], vert.position[2]);
            swap(vert.normal[1], vert.normal[2]);
            swap(vert.tangent[1], vert.tangent[2]);

            FbxVector2 uv = GetMeshElements<FbxVector2>(ctrlPtIdx, vertIdx, 
                                                  fbx_mesh->GetElementUV());
            vert.uv[0] = uv[0];
            vert.uv[1] = 1 - uv[1];

            float totalWeight = 0;
            for (auto k = 0; k < 4; ++k) {
                totalWeight += weights[ctrlPtIdx].weights[k];
                vert.bones[k] = weights[ctrlPtIdx].boneIdxs[k];
            }
            if (totalWeight != 0) {
                for (auto k = 0; k < 4; ++k) {
                    vert.weights[k] = 
                        weights[ctrlPtIdx].weights[k] / totalWeight;
                }
            }

            bool present = false;
            for (auto k = 0; k < mesh.vertices.size(); ++k) {
                if (vert == mesh.vertices[k]) {
                    present = true;
                    mesh.indices.push_back(k);
                    break;
                }
            }
            if (!present) {
                mesh.vertices.push_back(vert);
                mesh.indices.push_back(mesh.vertices.size() - 1);
            }
        }
    }

    for (int i = 0; i < mesh.indices.size(); i += 3) {
        swap(mesh.indices[i], mesh.indices[i + 2]);
    }

    for (auto& bone : skeleton.bones) {
        if (bone.parent_index > -1) {
            bone.local_bind_pose =
                skeleton.bones[bone.parent_index].inverse_global_bind_pose *
                bone.inverse_global_bind_pose.Inverse();
        }
        else {
            //skeleton base transform * bone bind pose
            //TODO may be the cause of bad transforms
            bone.local_bind_pose = 
                ToLeftHand(bone.node->GetParent()->EvaluateGlobalTransform()) *
                bone.inverse_global_bind_pose.Inverse();
        }

    }
}

void EvaluateAnimation(FbxAnimStack* stack, Skeleton& skeleton,
                       Animation& animation) {
    //animation.name = stack->GetName();
    animation.duration = stack->LocalStop.Get().GetSecondDouble() - 
                         stack->LocalStart.Get().GetSecondDouble();
    animation.channels.resize(skeleton.bones.size());
    
    auto start = 
        stack->GetLocalTimeSpan().GetStart().GetFrameCount(FbxTime::eFrames24);
    auto stop = 
        stack->GetLocalTimeSpan().GetStop().GetFrameCount(FbxTime::eFrames24);

    for (auto i = 0; i < skeleton.bones.size(); ++i) {
        auto& chan = animation.channels[i];
        chan.resize(stop - start + 1);

        for (auto j = start; j <= stop; ++j) {
            FbxTime time;
            time.SetFrame(j, FbxTime::eFrames24);
            //TODO add data for frame rate?
            //TODO check for transformation modes
            FbxAMatrix srt;
            if (skeleton.bones[i].parent_index == -1)
                srt = ToLeftHand(
                    skeleton.bones[i].node->EvaluateGlobalTransform(time));
            else
                srt = ToLeftHand(
                    skeleton.bones[i].node->EvaluateLocalTransform(time));

            chan[j - start].rotation[0] = srt.GetQ()[0];
            chan[j - start].rotation[1] = srt.GetQ()[1];
            chan[j - start].rotation[2] = srt.GetQ()[2];
            chan[j - start].rotation[3] = srt.GetQ()[3];

            chan[j - start].translation[0] = srt.GetT()[0];
            chan[j - start].translation[1] = srt.GetT()[1];
            chan[j - start].translation[2] = srt.GetT()[2];
            
            chan[j - start].scale = srt.GetS()[0];
            float sclY = srt.GetS()[1];
            float sclZ = srt.GetS()[2];
            //TODO check for nonuniform scale
        }
    }
}