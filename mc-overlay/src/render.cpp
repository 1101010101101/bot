#include "render.h"
#include <cmath>
#include <cstring>

static const char* g_shader = R"(
struct VS_IN  { float2 pos : POSITION; float4 col : COLOR; };
struct PS_IN  { float4 pos : SV_POSITION; float4 col : COLOR; };
cbuffer CB : register(b0) { float4x4 proj; };
PS_IN VS(VS_IN v) {
    PS_IN o;
    o.pos = mul(float4(v.pos, 0, 1), proj);
    o.col = v.col;
    return o;
}
float4 PS(PS_IN p) : SV_TARGET { return p.col; }
)";

bool DXRenderer::Init(HWND hwnd) {
    m_hwnd = hwnd;
    RECT rc; GetClientRect(hwnd, &rc);
    m_width  = rc.right  - rc.left;
    m_height = rc.bottom - rc.top;

    DXGI_SWAP_CHAIN_DESC scd = {};
    scd.BufferCount          = 1;
    scd.BufferDesc.Width     = (UINT)m_width;
    scd.BufferDesc.Height    = (UINT)m_height;
    scd.BufferDesc.Format    = DXGI_FORMAT_R8G8B8A8_UNORM;
    scd.BufferUsage          = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scd.OutputWindow         = hwnd;
    scd.SampleDesc.Count     = 1;
    scd.Windowed             = TRUE;
    scd.SwapEffect           = DXGI_SWAP_EFFECT_DISCARD;

    D3D_FEATURE_LEVEL fl;
    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
        0, nullptr, 0, D3D11_SDK_VERSION,
        &scd, &m_swap, &m_dev, &fl, &m_ctx);
    if (FAILED(hr)) return false;

    ID3D11Texture2D* bb = nullptr;
    m_swap->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&bb);
    m_dev->CreateRenderTargetView(bb, nullptr, &m_rtv);
    bb->Release();

    D3D11_BLEND_DESC bd = {};
    bd.RenderTarget[0].BlendEnable            = TRUE;
    bd.RenderTarget[0].SrcBlend              = D3D11_BLEND_SRC_ALPHA;
    bd.RenderTarget[0].DestBlend             = D3D11_BLEND_INV_SRC_ALPHA;
    bd.RenderTarget[0].BlendOp               = D3D11_BLEND_OP_ADD;
    bd.RenderTarget[0].SrcBlendAlpha         = D3D11_BLEND_ONE;
    bd.RenderTarget[0].DestBlendAlpha        = D3D11_BLEND_ZERO;
    bd.RenderTarget[0].BlendOpAlpha          = D3D11_BLEND_OP_ADD;
    bd.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    m_dev->CreateBlendState(&bd, &m_blendState);

    ID3DBlob *vsBlob = nullptr, *psBlob = nullptr, *err = nullptr;
    D3DCompile(g_shader, strlen(g_shader), nullptr, nullptr, nullptr,
               "VS", "vs_4_0", 0, 0, &vsBlob, &err);
    if (err) { err->Release(); err = nullptr; }
    D3DCompile(g_shader, strlen(g_shader), nullptr, nullptr, nullptr,
               "PS", "ps_4_0", 0, 0, &psBlob, &err);
    if (err) { err->Release(); }

    m_dev->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &m_vs);
    m_dev->CreatePixelShader (psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &m_ps);

    D3D11_INPUT_ELEMENT_DESC ied[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT,       0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0,  8, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    m_dev->CreateInputLayout(ied, 2,
        vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &m_layout);
    vsBlob->Release(); psBlob->Release();

    D3D11_BUFFER_DESC vbd = {};
    vbd.Usage          = D3D11_USAGE_DYNAMIC;
    vbd.ByteWidth      = sizeof(Vertex) * 4096;
    vbd.BindFlags      = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    m_dev->CreateBuffer(&vbd, nullptr, &m_vb);

    D3D11_BUFFER_DESC cbd = {};
    cbd.Usage          = D3D11_USAGE_DYNAMIC;
    cbd.ByteWidth      = sizeof(DirectX::XMFLOAT4X4);
    cbd.BindFlags      = D3D11_BIND_CONSTANT_BUFFER;
    cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    m_dev->CreateBuffer(&cbd, nullptr, &m_cbuf);

    DirectX::XMFLOAT4X4 proj;
    DirectX::XMStoreFloat4x4(&proj,
        DirectX::XMMatrixOrthographicOffCenterLH(
            0, (float)m_width, (float)m_height, 0, 0, 1));
    D3D11_MAPPED_SUBRESOURCE ms;
    m_ctx->Map(m_cbuf, 0, D3D11_MAP_WRITE_DISCARD, 0, &ms);
    memcpy(ms.pData, &proj, sizeof(proj));
    m_ctx->Unmap(m_cbuf, 0);
    return true;
}

void DXRenderer::BeginFrame() {
    float clear[4] = { 0.f, 0.f, 0.f, 0.f };
    m_ctx->ClearRenderTargetView(m_rtv, clear);
    D3D11_VIEWPORT vp = { 0, 0, (float)m_width, (float)m_height, 0, 1 };
    m_ctx->RSSetViewports(1, &vp);
    m_ctx->OMSetRenderTargets(1, &m_rtv, nullptr);
    float bf[4] = {};
    m_ctx->OMSetBlendState(m_blendState, bf, 0xFFFFFFFF);
    m_ctx->IASetInputLayout(m_layout);
    m_ctx->VSSetShader(m_vs, nullptr, 0);
    m_ctx->PSSetShader(m_ps, nullptr, 0);
    m_ctx->VSSetConstantBuffers(0, 1, &m_cbuf);
    UINT stride = sizeof(Vertex), offset = 0;
    m_ctx->IASetVertexBuffers(0, 1, &m_vb, &stride, &offset);
}

void DXRenderer::FlushBatch(D3D11_PRIMITIVE_TOPOLOGY topo, Vertex* verts, int count) {
    D3D11_MAPPED_SUBRESOURCE ms;
    m_ctx->Map(m_vb, 0, D3D11_MAP_WRITE_DISCARD, 0, &ms);
    memcpy(ms.pData, verts, sizeof(Vertex) * count);
    m_ctx->Unmap(m_vb, 0);
    m_ctx->IASetPrimitiveTopology(topo);
    m_ctx->Draw((UINT)count, 0);
}

void DXRenderer::DrawLine(float x1, float y1, float x2, float y2,
                           float r,  float g,  float b,  float a) {
    Vertex v[2] = { {x1,y1,r,g,b,a}, {x2,y2,r,g,b,a} };
    FlushBatch(D3D11_PRIMITIVE_TOPOLOGY_LINELIST, v, 2);
}

void DXRenderer::DrawRect(float x, float y, float w, float h,
                           float r, float g, float b, float a) {
    Vertex v[8] = {
        {x,   y,   r,g,b,a}, {x+w, y,   r,g,b,a},
        {x+w, y,   r,g,b,a}, {x+w, y+h, r,g,b,a},
        {x+w, y+h, r,g,b,a}, {x,   y+h, r,g,b,a},
        {x,   y+h, r,g,b,a}, {x,   y,   r,g,b,a},
    };
    FlushBatch(D3D11_PRIMITIVE_TOPOLOGY_LINELIST, v, 8);
}

bool DXRenderer::WorldToScreen(
    float wx, float wy, float wz,
    float lx, float ly, float lz,
    float yaw, float pitch,
    float& sx, float& sy)
{
    const float PI = 3.14159265f;
    float dx = wx - lx, dy = wy - ly, dz = wz - lz;
    float yawR   = yaw   * PI / 180.f;
    float pitchR = pitch * PI / 180.f;

    float rx  =  dx * cosf(yawR) + dz * sinf(yawR);
    float ry  =  dy;
    float rz  = -dx * sinf(yawR) + dz * cosf(yawR);

    float ry2 = ry * cosf(pitchR) - rz * sinf(pitchR);
    float rz2 = ry * sinf(pitchR) + rz * cosf(pitchR);

    if (rz2 <= 0.1f) return false;

    float fov    = 70.f * PI / 180.f;
    float aspect = (float)m_width / (float)m_height;
    float tanFov = tanf(fov / 2.f);

    sx = (m_width  / 2.f) * (1.f + rx  / (rz2 * tanFov * aspect));
    sy = (m_height / 2.f) * (1.f - ry2 / (rz2 * tanFov));
    return true;
}

void DXRenderer::DrawESP(const GameData& data) {
    for (int i = 0; i < data.entityCount; i++) {
        const auto& e = data.entities[i];
        float hx, hy, fx, fy;
        bool headOk = WorldToScreen(e.x, e.y + 1.8f, e.z,
            data.localX, data.localY + 1.62f, data.localZ,
            data.yaw, data.pitch, hx, hy);
        bool feetOk = WorldToScreen(e.x, e.y, e.z,
            data.localX, data.localY + 1.62f, data.localZ,
            data.yaw, data.pitch, fx, fy);
        if (!headOk || !feetOk) continue;

        float boxH = fy - hy;
        float boxW = boxH / 2.f;
        float r = e.isPlayer ? 1.f : 1.f;
        float g = e.isPlayer ? 0.f : 1.f;
        DrawRect(hx - boxW / 2.f, hy, boxW, boxH, r, g, 0.f, 1.f);
        DrawLine((float)m_width / 2.f, (float)m_height, hx, hy, r, g, 0.f, 0.5f);
    }
}

void DXRenderer::DrawHUD(const GameData& data) {
    float hpRatio = data.health / 20.f;
    if (hpRatio < 0.f) hpRatio = 0.f;
    if (hpRatio > 1.f) hpRatio = 1.f;
    float barW = 200.f, barH = 12.f;
    float bx = 20.f, by = (float)m_height - 40.f;
    DrawRect(bx, by, barW,          barH, 0.2f, 0.2f, 0.2f, 0.8f);
    DrawRect(bx, by, barW * hpRatio, barH, 0.f,  1.f,  0.f,  1.f);
}

void DXRenderer::EndFrame() {
    m_swap->Present(0, 0);
}

DXRenderer::~DXRenderer() {
    if (m_blendState) m_blendState->Release();
    if (m_cbuf)       m_cbuf->Release();
    if (m_vb)         m_vb->Release();
    if (m_layout)     m_layout->Release();
    if (m_vs)         m_vs->Release();
    if (m_ps)         m_ps->Release();
    if (m_rtv)        m_rtv->Release();
    if (m_swap)       m_swap->Release();
    if (m_ctx)        m_ctx->Release();
    if (m_dev)        m_dev->Release();
}
