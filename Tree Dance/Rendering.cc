/*
    Copyright (c) 2015 Seth Pendergrass. See LICENSE.
*/

#include "Rendering.h"
#include <vector>
#include <d3dcompiler.h>
using namespace DirectX;
using namespace std;

const D3D11_INPUT_ELEMENT_DESC Renderer::vertexLayout[6]{
    { "POS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
    D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "NORM", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12,
    D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "TAN", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24,
    D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "TEX", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 36,
    D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "BONEIDS", 0, DXGI_FORMAT_R32G32B32A32_SINT, 0, 44,
    D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "WEIGHTS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 60,
    D3D11_INPUT_PER_VERTEX_DATA, 0 },
};

void Renderer::Init(HWND hwnd) {
    RECT window;
    GetClientRect(hwnd, &window);
    height_ = window.bottom - window.top;
    width_ = window.right - window.left;

    /*height_ = 1440;
    width_ = 2560;*/

    DXGI_SWAP_CHAIN_DESC swap_desc{};
    swap_desc.OutputWindow = hwnd;
    swap_desc.SampleDesc.Count = 1;
    swap_desc.Windowed = true; //false;
    swap_desc.BufferCount = 1;
    swap_desc.BufferDesc.Height = height_;
    swap_desc.BufferDesc.Width = width_;
    swap_desc.BufferDesc.RefreshRate.Numerator = 60;
    swap_desc.BufferDesc.RefreshRate.Denominator = 1;
    swap_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swap_desc.BufferUsage = DXGI_USAGE_UNORDERED_ACCESS;
    D3D11CreateDeviceAndSwapChain(0, D3D_DRIVER_TYPE_HARDWARE, 0,
                                  D3D11_CREATE_DEVICE_DEBUG, 0, 0,
                                  D3D11_SDK_VERSION, &swap_desc, &swap_chain_,
                                  &device_, 0, &context_);

    D3D11_VIEWPORT viewport{};
    viewport.Height = static_cast<float>(height_);
    viewport.Width = static_cast<float>(width_);
    viewport.MaxDepth = 1.0f;
    context_->RSSetViewports(1, &viewport);

    ID3D11Texture2D* back_buffer_tex;
    swap_chain_->GetBuffer(0, __uuidof(ID3D11Texture2D),
        reinterpret_cast<void**>(&back_buffer_tex));
    D3D11_UNORDERED_ACCESS_VIEW_DESC uav_desc{};
    uav_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    uav_desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
    device_->CreateUnorderedAccessView(back_buffer_tex, &uav_desc,
        &back_buffer_uav_);
    back_buffer_tex->Release();

    // G Buffer Creation

    D3D11_TEXTURE2D_DESC tex_desc{};
    tex_desc.ArraySize = 1;
    tex_desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    tex_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    tex_desc.Height = height_;
    tex_desc.Width = width_;
    tex_desc.MipLevels = 1;
    tex_desc.SampleDesc.Count = 1;
    tex_desc.Usage = D3D11_USAGE_DEFAULT;
    ID3D11Texture2D* rtv_tex;
    for (int i = 0; i < 2; ++i) {
        device_->CreateTexture2D(&tex_desc, 0, &rtv_tex);
        device_->CreateRenderTargetView(rtv_tex, 0, &gbuffer_rtv_[i]);
        device_->CreateShaderResourceView(rtv_tex, 0, 
                                          &gbuffer_ds_lights_srv_[i]);
        rtv_tex->Release();
    }
    ID3D11Texture2D* dsv_tex;
    tex_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
    tex_desc.Format = DXGI_FORMAT_R24G8_TYPELESS;
    device_->CreateTexture2D(&tex_desc, 0, &dsv_tex);
    D3D11_DEPTH_STENCIL_VIEW_DESC dsv_desc{};
    dsv_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    dsv_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    device_->CreateDepthStencilView(dsv_tex, &dsv_desc, &dsv_);
    D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc{};
    srv_desc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
    srv_desc.Texture2D.MipLevels = 1;
    srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    device_->CreateShaderResourceView(dsv_tex, &srv_desc, 
                                      &gbuffer_ds_lights_srv_[2]);
    dsv_tex->Release();

    ID3DBlob* blob;
    D3DReadFileToBlob(L"../x64/Debug/VertexShader.cso", &blob);
    device_->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(),
        0, &vertex_shader_);
    ID3D11InputLayout* input_layout;
    device_->CreateInputLayout(vertexLayout, 6, blob->GetBufferPointer(),
        blob->GetBufferSize(), &input_layout);
    D3DReadFileToBlob(L"../x64/Debug/HullShader.cso", &blob);
    device_->CreateHullShader(blob->GetBufferPointer(), blob->GetBufferSize(),
        0, &hull_shader_);
    D3DReadFileToBlob(L"../x64/Debug/DomainShader.cso", &blob);
    device_->CreateDomainShader(blob->GetBufferPointer(), blob->GetBufferSize(),
        0, &domain_shader_);
    D3DReadFileToBlob(L"../x64/Debug/PixelShader.cso", &blob);
    device_->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(),
        0, &pixel_shader_);
    D3DReadFileToBlob(L"../x64/Debug/ComputeShader.cso", &blob);
    device_->CreateComputeShader(blob->GetBufferPointer(),
        blob->GetBufferSize(), 0, &compute_shader_);
    blob->Release();

    D3D11_BUFFER_DESC bd{};
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.Usage = D3D11_USAGE_DYNAMIC;
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bd.ByteWidth = sizeof(VSCBuffer);
    device_->CreateBuffer(&bd, 0, &vsbuf_);
    bd.ByteWidth = sizeof(DSCBuffer);
    device_->CreateBuffer(&bd, 0, &dsbuf_);
    bd.ByteWidth = sizeof(CSCBuffer);
    device_->CreateBuffer(&bd, 0, &csbuf_);

    D3D11_SAMPLER_DESC sd{};
    sd.AddressU =
        sd.AddressV =
        sd.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sd.MaxLOD = D3D11_FLOAT32_MAX;
    sd.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sd.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    ID3D11SamplerState* samPoint, *samAniso;
    device_->CreateSamplerState(&sd, &samPoint);
    sd.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    sd.Filter = D3D11_FILTER_ANISOTROPIC;
    sd.MaxAnisotropy = 16;
    device_->CreateSamplerState(&sd, &samAniso);

    D3D11_RASTERIZER_DESC rd{};
    rd.FillMode = D3D11_FILL_SOLID;//WIREFRAME;
    rd.CullMode = D3D11_CULL_BACK;
    rd.DepthClipEnable = true;
    ID3D11RasterizerState* rs;
    device_->CreateRasterizerState(&rd, &rs);

    context_->IASetPrimitiveTopology(
        D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
    context_->IASetInputLayout(input_layout);
    input_layout->Release();
    context_->OMSetRenderTargets(2, gbuffer_rtv_, dsv_);
    
    context_->RSSetState(rs);

    context_->VSSetShader(vertex_shader_, 0, 0);
    context_->HSSetShader(hull_shader_, 0, 0);
    context_->DSSetShader(domain_shader_, 0, 0);
    context_->PSSetShader(pixel_shader_, 0, 0);
    context_->CSSetShader(compute_shader_, 0, 0);
    context_->VSSetConstantBuffers(0, 1, &vsbuf_);
    context_->DSSetConstantBuffers(0, 1, &dsbuf_);
    context_->CSSetConstantBuffers(0, 1, &csbuf_);
    context_->DSSetSamplers(0, 1, &samPoint);
    context_->PSSetSamplers(0, 1, &samAniso);
    context_->CSSetUnorderedAccessViews(0, 1, &back_buffer_uav_, 0);
}

void Renderer::Deinit() {
    swap_chain_->Release();
    device_->Release();
    context_->Release();
    back_buffer_uav_->Release();
    for (int i = 0; i < 2; ++i)
        gbuffer_rtv_[0]->Release();
    //back_buffer_rtv_->Release();
    for (int i = 0; i < 4; ++i)
        gbuffer_ds_lights_srv_[i]->Release();
    dsv_->Release();
    vertex_shader_->Release();
    hull_shader_->Release();
    domain_shader_->Release();
    pixel_shader_->Release();
    compute_shader_->Release();
    vsbuf_->Release();
    dsbuf_->Release();
    csbuf_->Release();
}

void Renderer::Clear() {
    float black[4] = { 0, 0, 0, 0 };
    for (int i = 0; i < 2; ++i) {
        context_->ClearRenderTargetView(gbuffer_rtv_[i], black);
    }
    context_->ClearDepthStencilView(dsv_, D3D11_CLEAR_DEPTH, 1.0f, 0);

    ID3D11ShaderResourceView* nullSRV[4] = { 0, 0, 0, 0 };
    context_->CSSetShaderResources(0, 3, nullSRV);
    context_->OMSetRenderTargets(2, gbuffer_rtv_, dsv_);
}

void Renderer::Present() {
    context_->OMSetRenderTargets(0, nullptr, nullptr);
    context_->CSSetShaderResources(0, 4, gbuffer_ds_lights_srv_);

    MapCBuffer(csbuf_, &cscbuffer_, sizeof(CSCBuffer));

    context_->Dispatch(width_ / 16 + 1, height_ / 16 + 1, 1);
    swap_chain_->Present(0, 0);
}

void Renderer::SetCamera(const Camera& camera) {
    XMMATRIX proj = camera.GetProjection();
    XMMATRIX view = camera.GetView();
    dscbuffer_.ViewProj = XMMatrixTranspose(view * proj);
    cscbuffer_.ViewInv = XMMatrixTranspose(XMMatrixInverse(nullptr, view));
    XMFLOAT4X4 temp;
    XMStoreFloat4x4(&temp, proj);
    cscbuffer_.Proj_11 = temp._11;
    cscbuffer_.Proj_22 = temp._22;
    cscbuffer_.Proj_33 = temp._33;
    cscbuffer_.Proj_43 = temp._43;
    cscbuffer_.Height = height_;
    cscbuffer_.Width = width_;
    vscbuffer_.Eye =
        dscbuffer_.Eye =
        cscbuffer_.Eye = camera.position;
    MapCBuffer(dsbuf_, &dscbuffer_, sizeof(DSCBuffer));
}

void Renderer::SetLights(const vector<Light>& lights) {
    D3D11_BUFFER_DESC bd{};
    D3D11_SUBRESOURCE_DATA srdata{};
    D3D11_SHADER_RESOURCE_VIEW_DESC srvd{};
    ID3D11Buffer* lightBuff;

    int num = static_cast<int>(lights.size());
    int stride = sizeof(Light);

    bd.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    bd.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    bd.ByteWidth = stride * num;
    bd.StructureByteStride = stride;

    srdata.pSysMem = lights.data();

    device_->CreateBuffer(&bd, &srdata, &lightBuff);

    srvd.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    srvd.Buffer.ElementWidth = num;

    device_->CreateShaderResourceView(lightBuff, &srvd, 
                                      &gbuffer_ds_lights_srv_[3]);

    lightBuff->Release();
}

void Renderer::LoadModel(Model* model) {
    D3D11_BUFFER_DESC bd{};
    D3D11_SUBRESOURCE_DATA srd{};

    for (auto& mesh : model->meshes) {
        bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        bd.ByteWidth = static_cast<int>(sizeof(Vertex) * mesh.vertices.size());
        srd.pSysMem = mesh.vertices.data();
        device_->CreateBuffer(&bd, &srd, &mesh.vertex_buffer);

        bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
        bd.ByteWidth = static_cast<int>(sizeof(unsigned int) * 
                                        mesh.indices.size());
        srd.pSysMem = mesh.indices.data();
        device_->CreateBuffer(&bd, &srd, &mesh.index_buffer);
        }

    D3D11_TEXTURE2D_DESC td{};
    ID3D11Texture2D* tex;

    Texture* textures[5] { &model->material.albedo, &model->material.normal,
        &model->material.roughness, &model->material.specular, 
        &model->material.displacement
    };

    for (int i = 0; i < 5; ++i) {
        td.ArraySize = 1;
        td.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        td.MipLevels = 1;
        td.SampleDesc.Count = 1;
        td.Usage = D3D11_USAGE_DEFAULT;
        td.Format = textures[i]->depth == 4 ? DXGI_FORMAT_R8G8B8A8_UNORM :
            DXGI_FORMAT_R8_UNORM;
        td.Height = textures[i]->height;
        td.Width = textures[i]->width;

        srd.SysMemPitch = textures[i]->width * textures[i]->depth;
        srd.pSysMem = textures[i]->data.data();

        device_->CreateTexture2D(&td, &srd, &tex);
        device_->CreateShaderResourceView(tex, 0, &textures[i]->srv);
        tex->Release();
    }
}

void Renderer::Draw(Model* model, TransformSet* transforms) {
    vscbuffer_.InstSize = static_cast<int>(model->skeleton.bones.size());
    MapCBuffer(vsbuf_, &vscbuffer_, sizeof(VSCBuffer));

    D3D11_BUFFER_DESC bd{};
    D3D11_SUBRESOURCE_DATA srd{};
    D3D11_SHADER_RESOURCE_VIEW_DESC srvd{};
    ID3D11Buffer* buf;

    bd.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    srvd.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    srvd.Texture2D.MipLevels = 1;
    srvd.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;

    if (transforms->transforms.size() > 0) {
        bd.ByteWidth = static_cast<UINT>(transforms->transforms.size() * 
                                         sizeof(XMMATRIX));

        srd.pSysMem = transforms->transforms.data();

        device_->CreateBuffer(&bd, &srd, &buf);
        srvd.Buffer.NumElements = 
            static_cast<UINT>(4 * transforms->transforms.size());

        if (transforms->srv_transforms) transforms->srv_transforms->Release();

        device_->CreateShaderResourceView(buf, &srvd,
                                          &transforms->srv_transforms);
        buf->Release();
    }

    bd.ByteWidth = static_cast<UINT>(transforms->world_matrices.size() * 
                                     sizeof(XMMATRIX));

    srd.pSysMem = transforms->world_matrices.data();

    device_->CreateBuffer(&bd, &srd, &buf);

    srvd.Buffer.NumElements = 
        static_cast<UINT>(4 * transforms->world_matrices.size());

    if (transforms->srv_world) transforms->srv_world->Release();
    device_->CreateShaderResourceView(buf, &srvd, &transforms->srv_world);
    buf->Release();

    context_->VSSetShaderResources(0, 1, &transforms->srv_world);
    context_->VSSetShaderResources(1, 1, &transforms->srv_transforms);

    context_->DSSetShaderResources(0, 1, &model->material.displacement.srv);

    context_->PSSetShaderResources(0, 1, &model->material.albedo.srv);
    context_->PSSetShaderResources(1, 1, &model->material.specular.srv);
    context_->PSSetShaderResources(2, 1, &model->material.normal.srv);
    context_->PSSetShaderResources(3, 1, &model->material.roughness.srv);

    for (auto& mesh : model->meshes) {
        context_->IASetIndexBuffer(mesh.index_buffer, DXGI_FORMAT_R32_UINT, 0);
        const UINT vbStride = sizeof(Vertex);
        const UINT vbOffset = 0;
        context_->IASetVertexBuffers(0, 1, &mesh.vertex_buffer, &vbStride, 
                                     &vbOffset);
        context_->DrawIndexedInstanced(static_cast<UINT>(mesh.indices.size()), 
                                       transforms->num_instances, 0, 0, 0);
    }
}

inline void Renderer::MapCBuffer(ID3D11Resource* resource, void* src,
    size_t size) {
    D3D11_MAPPED_SUBRESOURCE msr{};
    context_->Map(resource, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
    memcpy(msr.pData, src, size);
    context_->Unmap(resource, 0);
}
