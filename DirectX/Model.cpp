#include "Sample.h"
#include "Model.h"
#include <WICTextureLoader.h>
#include <DDSTextureLoader.h>

using namespace DirectX;

template <class Vertex, class Indice, DXGI_FORMAT format>
HRESULT CMesh::Init(ID3D11Device *d3dDevice,
    const CAtlArray<Vertex> &vertices,
    const CAtlArray<Indice> &indices,
    ID3D11ShaderResourceView * tex)
{
    HRESULT hr = S_OK;

    // 设置顶点缓冲区描述
    D3D11_BUFFER_DESC vbd;
    ZeroMemory(&vbd, sizeof(vbd));
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
    vbd.ByteWidth = sizeof(Vertex) * vertices.GetCount();
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;
    // 新建顶点缓冲区
    D3D11_SUBRESOURCE_DATA initData;
    ZeroMemory(&initData, sizeof(initData));
    initData.pSysMem = vertices.GetData();
    HR_CHECK(d3dDevice->CreateBuffer(&vbd, &initData, &vertexBuffer));

    // 设置索引缓冲区描述
    D3D11_BUFFER_DESC ibd;
    ZeroMemory(&ibd, sizeof(ibd));
    ibd.Usage = D3D11_USAGE_IMMUTABLE;
    ibd.ByteWidth = sizeof(Indice) * indices.GetCount();
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;
    // 新建索引缓冲区
    initData.pSysMem = indices.GetData();
    HR_CHECK(d3dDevice->CreateBuffer(&ibd, &initData, &indexBuffer));

    vertexStride = sizeof(Vertex);
    indexCount = indices.GetCount();
    indexFormat = format;
    texDiffuse.Attach(tex);
exit:
    return hr;
}

void CMesh::Draw(ID3D11DeviceContext *d3dContext)
{
    // Set vertex buffer
    UINT offset = 0;
    d3dContext->IASetVertexBuffers(0, 1, &vertexBuffer.p, &vertexStride, &offset);
    // 输入装配阶段的索引缓冲区设置
    d3dContext->IASetIndexBuffer(indexBuffer, indexFormat, 0);
    // 设置纹理
    d3dContext->PSSetShaderResources(0, 1, &texDiffuse.p);
    // 绘制立方体
    d3dContext->DrawIndexed(indexCount, 0, 0);
}

void CModel::Draw(ID3D11DeviceContext *d3dContext)
{
    for (size_t i = 0; i < meshes.GetCount(); i++) meshes[i]->Draw(d3dContext);
}

void CModel::Load(ID3D11Device *d3dDevice, const aiScene * scene, WCHAR * dir)
{
    CLoad ctx = { g_XMInfinity, g_XMNegInfinity, d3dDevice, scene, dir };
    this->LoadMesh(scene->mRootNode, &ctx);
    // 创建包围盒
    BoundingBox::CreateFromPoints(bound, ctx.vMin, ctx.vMax);

    XMVECTOR dist = g_XMSix / XMVector3Length(XMLoadFloat3(&bound.Extents));
    XMMATRIX trans = XMMatrixTranslationFromVector((ctx.vMin + ctx.vMax) * -0.5f);
    XMStoreFloat4x4(&world, XMMatrixMultiply(trans, XMMatrixScalingFromVector(dist)));
}

void CModel::LoadMesh(aiNode * node, CLoad * ctx)
{
    aiString path;
    size_t len;

    for (UINT n = 0; n < node->mNumMeshes; n++)
    {
        ID3D11ShaderResourceView * tex = NULL;
        CAtlArray<CVertex> vertices;
        CAtlArray<UINT> indices;
        CVertex v;

        aiMesh * mesh = ctx->scene->mMeshes[node->mMeshes[n]];
        aiVector3D * coords = *mesh->mTextureCoords;
        // Walk through each of the mesh's vertices
        for (UINT i = 0; i < mesh->mNumVertices; i++)
        {
            v.Pos.x = mesh->mVertices[i].x;
            v.Pos.y = mesh->mVertices[i].y;
            v.Pos.z = mesh->mVertices[i].z;
            if (coords)
            {
                v.Tex.x = coords[i].x;
                v.Tex.y = coords[i].y;
            }
            XMVECTOR vecPos = XMLoadFloat3(&v.Pos);
            ctx->vMax = XMVectorMax(ctx->vMax, vecPos);
            ctx->vMin = XMVectorMin(ctx->vMin, vecPos);
            vertices.Add(v);
        }

        for (UINT i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace * face = mesh->mFaces + i;
            for (UINT j = 0; j < face->mNumIndices; j++)
                indices.Add(face->mIndices[j]);
        }

        if (mesh->mMaterialIndex >= 0)
        {
            aiColor3D color(0.f, 0.f, 0.f);
            aiMaterial* mat = ctx->scene->mMaterials[mesh->mMaterialIndex];
            // 加载材质
            if (mat->Get(AI_MATKEY_COLOR_DIFFUSE, color) == aiReturn_SUCCESS)
            {

            }
            // 加载纹理
            for (UINT i = 0; i < mat->GetTextureCount(aiTextureType_DIFFUSE); i++)
            {
                if (mat->GetTexture(aiTextureType_DIFFUSE, i, &path) != aiReturn_SUCCESS) continue;
                WCHAR * name = NULL;
                for (int j = 0; ctx->dir[j] != 0; j++) if (ctx->dir[j] == L'\\') name = ctx->dir + j + 1;
                mbstowcs_s(&len, name, MAX_PATH - (name - ctx->dir + 1), path.C_Str(), _TRUNCATE);
                if (FAILED(CreateWICTextureFromFile(ctx->d3dDevice, ctx->dir, NULL, &tex))) continue;
                break;
            }
        }
        CAutoPtr<CMesh> m(new CMesh());
        m->Init(ctx->d3dDevice, vertices, indices, tex);
        meshes.Add(m);
    }
    for (UINT i = 0; i < node->mNumChildren; i++)
    {
        this->LoadMesh(node->mChildren[i], ctx);
    }
}

void CModel::Scale(float x, float y, float z)
{
    XMMATRIX wrp = XMLoadFloat4x4(&world);
    XMMATRIX scale = XMMatrixScaling(x, y, z);
    XMStoreFloat4x4(&world, XMMatrixMultiply(wrp, scale));
}