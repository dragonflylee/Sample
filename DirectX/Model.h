#pragma once

class CMesh
{
public:
    template <class Vertex, class Indice, DXGI_FORMAT format = DXGI_FORMAT_R32_UINT>
    HRESULT Init(ID3D11Device *d3dDevice,
        const CAtlArray<Vertex> &vertices,
        const CAtlArray<Indice> &indices,
        ID3D11ShaderResourceView* tex = NULL);
    // 绘制多边形
    void Draw(ID3D11DeviceContext *d3dContext);

    typedef CAutoPtrArray<CMesh> Collection;

private:
    UINT indexCount;
    UINT vertexStride;
    DXGI_FORMAT indexFormat;
    CComPtr<ID3D11Buffer> indexBuffer;            // 索引缓冲区
    CComPtr<ID3D11Buffer> vertexBuffer;           // 顶点缓冲区
    CComPtr<ID3D11ShaderResourceView> texDiffuse;
};

#include <assimp/scene.h>

class CModel
{
public:

    struct CVertex
    {
        DirectX::XMFLOAT3 Pos;
        DirectX::XMFLOAT3 Tex;
    };

    void Load(ID3D11Device *d3dDevice, const aiScene * scene, WCHAR * dir);

    // 绘制模型
    void Draw(ID3D11DeviceContext *d3dContext);

    void Scale(float x, float y, float z);

    void XM_CALLCONV SetWorld(DirectX::CXMMATRIX w)
    {
        DirectX::XMStoreFloat4x4(&world, w);
    }

    DirectX::XMMATRIX XM_CALLCONV GetWorld()
    {
        return DirectX::XMLoadFloat4x4(&world);
    }

    typedef CAutoPtrArray<CModel> Collection;

private:
    struct CLoad
    {
        DirectX::XMVECTOR vMin, vMax;           // AABB盒双顶点
        ID3D11Device *d3dDevice;
        const aiScene * scene;
        WCHAR * dir;
    };

    // 加载模型
    void LoadMesh(aiNode * node, CLoad * ctx);

private:
    CMesh::Collection meshes;
    DirectX::BoundingBox bound;
    DirectX::XMFLOAT4X4 world;
};

