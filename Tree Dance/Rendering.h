/*
    Copyright (c) 2015 Seth Pendergrass. See LICENSE.
*/

#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <vector>
#include "Camera.h"
#include "Light.h"
#include "Model.h"

class Renderer {
 public:
    void Init(HWND hwnd);
    void Deinit();
    void Clear();
    void Present();

    void SetCamera(const Camera& camera);
    void SetLights(const std::vector<Light>& lights);
    void LoadModel(Model* model);

    void Draw(Model* model, TransformSet* transforms);

 private:
    UINT width_, height_;
    IDXGISwapChain* swap_chain_;
    ID3D11Device* device_;
    ID3D11DeviceContext* context_;
    ID3D11UnorderedAccessView* back_buffer_uav_;
    ID3D11RenderTargetView* gbuffer_rtv_[2], *back_buffer_rtv_;
    ID3D11ShaderResourceView* gbuffer_ds_lights_srv_[4];
    ID3D11DepthStencilView* dsv_;
    ID3D11VertexShader* vertex_shader_;
    ID3D11HullShader* hull_shader_;
    ID3D11DomainShader* domain_shader_;
    ID3D11PixelShader* pixel_shader_;
    ID3D11ComputeShader* compute_shader_;
    static const D3D11_INPUT_ELEMENT_DESC vertexLayout[6];
    __declspec(align(16))
    struct VSCBuffer {
        DirectX::XMFLOAT3 Eye;
        float TesDisMax = 5.0f, TesDisMin = 10.0f, TesFacMax = 3.0f, 
            TesFacMin = 1.0f;
        unsigned int InstSize;
    } vscbuffer_;
    __declspec(align(16))
    struct DSCBuffer {
        DirectX::XMMATRIX ViewProj;
        DirectX::XMFLOAT3 Eye;
    } dscbuffer_;
    __declspec(align(16))
    struct CSCBuffer {
        DirectX::XMMATRIX ViewInv;
        DirectX::XMFLOAT3 Eye;
        float Proj_11, Proj_22, Proj_33, Proj_43;
        unsigned int Height, Width;
    } cscbuffer_;
    ID3D11Buffer* vsbuf_, *dsbuf_, *csbuf_;

    inline void MapCBuffer(ID3D11Resource* resource, void* src, size_t size);
};

