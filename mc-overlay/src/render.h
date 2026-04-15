#pragma once
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include "types.h"
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

class DXRenderer {
public:
    bool Init(HWND hwnd);
    void BeginFrame();
    void DrawESP(const GameData& data);
    void DrawHUD(const GameData& data);
    void EndFrame();
    ~DXRenderer();

private:
    HWND                    m_hwnd      = nullptr;
    ID3D11Device*           m_dev       = nullptr;
    ID3D11DeviceContext*    m_ctx       = nullptr;
    IDXGISwapChain*         m_swap      = nullptr;
    ID3D11RenderTargetView* m_rtv       = nullptr;
    ID3D11BlendState*       m_blendState = nullptr;
    ID3D11Buffer*           m_vb        = nullptr;
    ID3D11VertexShader*     m_vs        = nullptr;
    ID3D11PixelShader*      m_ps        = nullptr;
    ID3D11InputLayout*      m_layout    = nullptr;
    ID3D11Buffer*           m_cbuf      = nullptr;
    int m_width = 0, m_height = 0;

    struct Vertex { float x, y; float r, g, b, a; };

    void DrawRect(float x, float y, float w, float h, float r, float g, float b, float a);
    void DrawLine(float x1, float y1, float x2, float y2, float r, float g, float b, float a);
    void FlushBatch(D3D11_PRIMITIVE_TOPOLOGY topo, Vertex* verts, int count);
    bool WorldToScreen(float wx, float wy, float wz,
                       float lx,  float ly,  float lz,
                       float yaw, float pitch,
                       float& sx, float& sy);
};
