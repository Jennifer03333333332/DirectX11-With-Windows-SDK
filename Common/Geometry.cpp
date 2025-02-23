#include "Geometry.h"

namespace Geometry
{
    constexpr float PI = 3.1415926f;
    //
    // 几何体方法的实现
    //

    MeshData CreateSphere(float radius, uint32_t levels, uint32_t slices)
    {
        using namespace DirectX;

        MeshData meshData;
        
        uint32_t vertexCount = 2 + (levels - 1) * (slices + 1);
        uint32_t indexCount = 6 * (levels - 1) * slices;
        meshData.vertices.resize(vertexCount);
        meshData.normals.resize(vertexCount);
        meshData.texcoords.resize(vertexCount);
        meshData.tangents.resize(vertexCount);
        if (indexCount > 65535)
            meshData.indices32.resize(indexCount);
        else
            meshData.indices16.resize(indexCount);

        uint32_t vIndex = 0, iIndex = 0;

        float phi = 0.0f, theta = 0.0f;
        float per_phi = PI / levels;
        float per_theta = 2 * PI / slices;
        float x, y, z;

        // 放入顶端点
        meshData.vertices[vIndex] = XMFLOAT3(0.0f, radius, 0.0f);
        meshData.normals[vIndex] = XMFLOAT3(0.0f, 1.0f, 0.0f);
        meshData.tangents[vIndex] = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
        meshData.texcoords[vIndex++] = XMFLOAT2(0.0f, 0.0f);


        for (uint32_t i = 1; i < levels; ++i)
        {
            phi = per_phi * i;
            // 需要slices + 1个顶点是因为 起点和终点需为同一点，但纹理坐标值不一致
            for (uint32_t j = 0; j <= slices; ++j)
            {
                theta = per_theta * j;
                x = radius * sinf(phi) * cosf(theta);
                y = radius * cosf(phi);
                z = radius * sinf(phi) * sinf(theta);
                // 计算出局部坐标、法向量、Tangent向量和纹理坐标
                XMFLOAT3 pos = XMFLOAT3(x, y, z);

                meshData.vertices[vIndex] = pos;
                XMStoreFloat3(&meshData.normals[vIndex], XMVector3Normalize(XMLoadFloat3(&pos)));
                meshData.tangents[vIndex] = XMFLOAT4(-sinf(theta), 0.0f, cosf(theta), 1.0f);
                meshData.texcoords[vIndex++] = XMFLOAT2(theta / 2 / PI, phi / PI);
            }
        }

        // 放入底端点
        meshData.vertices[vIndex] = XMFLOAT3(0.0f, -radius, 0.0f);
        meshData.normals[vIndex] = XMFLOAT3(0.0f, -1.0f, 0.0f);
        meshData.tangents[vIndex] = XMFLOAT4(-1.0f, 0.0f, 0.0f, 1.0f);
        meshData.texcoords[vIndex++] = XMFLOAT2(0.0f, 1.0f);


        // 放入索引
        if (levels > 1)
        {
            for (uint32_t j = 1; j <= slices; ++j)
            {
                if (indexCount > 65535)
                {
                    meshData.indices32[iIndex++] = 0;
                    meshData.indices32[iIndex++] = j % (slices + 1) + 1;
                    meshData.indices32[iIndex++] = j;
                }
                else
                {
                    meshData.indices16[iIndex++] = 0;
                    meshData.indices16[iIndex++] = j % (slices + 1) + 1;
                    meshData.indices16[iIndex++] = j;
                }
            }
        }


        for (uint32_t i = 1; i < levels - 1; ++i)
        {
            for (uint32_t j = 1; j <= slices; ++j)
            {
                if (indexCount > 65535)
                {
                    meshData.indices32[iIndex++] = (i - 1) * (slices + 1) + j;
                    meshData.indices32[iIndex++] = (i - 1) * (slices + 1) + j % (slices + 1) + 1;
                    meshData.indices32[iIndex++] = i * (slices + 1) + j % (slices + 1) + 1;

                    meshData.indices32[iIndex++] = i * (slices + 1) + j % (slices + 1) + 1;
                    meshData.indices32[iIndex++] = i * (slices + 1) + j;
                    meshData.indices32[iIndex++] = (i - 1) * (slices + 1) + j;
                }
                else
                {
                    meshData.indices16[iIndex++] = (i - 1) * (slices + 1) + j;
                    meshData.indices16[iIndex++] = (i - 1) * (slices + 1) + j % (slices + 1) + 1;
                    meshData.indices16[iIndex++] = i * (slices + 1) + j % (slices + 1) + 1;

                    meshData.indices16[iIndex++] = i * (slices + 1) + j % (slices + 1) + 1;
                    meshData.indices16[iIndex++] = i * (slices + 1) + j;
                    meshData.indices16[iIndex++] = (i - 1) * (slices + 1) + j;
                }
                
            }
        }

        // 逐渐放入索引
        if (levels > 1)
        {
            for (uint32_t j = 1; j <= slices; ++j)
            {
                if (indexCount > 65535)
                {
                    meshData.indices32[iIndex++] = (levels - 2) * (slices + 1) + j;
                    meshData.indices32[iIndex++] = (levels - 2) * (slices + 1) + j % (slices + 1) + 1;
                    meshData.indices32[iIndex++] = (levels - 1) * (slices + 1) + 1;
                }
                else
                {
                    meshData.indices16[iIndex++] = (levels - 2) * (slices + 1) + j;
                    meshData.indices16[iIndex++] = (levels - 2) * (slices + 1) + j % (slices + 1) + 1;
                    meshData.indices16[iIndex++] = (levels - 1) * (slices + 1) + 1;
                }
            }
        }

        return meshData;
    }

    MeshData CreateBox(float width, float height, float depth)
    {
        using namespace DirectX;

        MeshData meshData;

        meshData.vertices.resize(24);
        meshData.normals.resize(24);
        meshData.tangents.resize(24);
        meshData.texcoords.resize(24);

        float w2 = width / 2, h2 = height / 2, d2 = depth / 2;

        // 右面(+X面)
        meshData.vertices[0] = XMFLOAT3(w2, -h2, -d2);
        meshData.vertices[1] = XMFLOAT3(w2, h2, -d2);
        meshData.vertices[2] = XMFLOAT3(w2, h2, d2);
        meshData.vertices[3] = XMFLOAT3(w2, -h2, d2);
        // 左面(-X面)
        meshData.vertices[4] = XMFLOAT3(-w2, -h2, d2);
        meshData.vertices[5] = XMFLOAT3(-w2, h2, d2);
        meshData.vertices[6] = XMFLOAT3(-w2, h2, -d2);
        meshData.vertices[7] = XMFLOAT3(-w2, -h2, -d2);
        // 顶面(+Y面)
        meshData.vertices[8] = XMFLOAT3(-w2, h2, -d2);
        meshData.vertices[9] = XMFLOAT3(-w2, h2, d2);
        meshData.vertices[10] = XMFLOAT3(w2, h2, d2);
        meshData.vertices[11] = XMFLOAT3(w2, h2, -d2);
        // 底面(-Y面)
        meshData.vertices[12] = XMFLOAT3(w2, -h2, -d2);
        meshData.vertices[13] = XMFLOAT3(w2, -h2, d2);
        meshData.vertices[14] = XMFLOAT3(-w2, -h2, d2);
        meshData.vertices[15] = XMFLOAT3(-w2, -h2, -d2);
        // 背面(+Z面)
        meshData.vertices[16] = XMFLOAT3(w2, -h2, d2);
        meshData.vertices[17] = XMFLOAT3(w2, h2, d2);
        meshData.vertices[18] = XMFLOAT3(-w2, h2, d2);
        meshData.vertices[19] = XMFLOAT3(-w2, -h2, d2);
        // 正面(-Z面)
        meshData.vertices[20] = XMFLOAT3(-w2, -h2, -d2);
        meshData.vertices[21] = XMFLOAT3(-w2, h2, -d2);
        meshData.vertices[22] = XMFLOAT3(w2, h2, -d2);
        meshData.vertices[23] = XMFLOAT3(w2, -h2, -d2);

        for (size_t i = 0; i < 4; ++i)
        {
            // 右面(+X面)
            meshData.normals[i] = XMFLOAT3(1.0f, 0.0f, 0.0f);
            meshData.tangents[i] = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
            // 左面(-X面)
            meshData.normals[i + 4] = XMFLOAT3(-1.0f, 0.0f, 0.0f);
            meshData.tangents[i + 4] = XMFLOAT4(0.0f, 0.0f, -1.0f, 1.0f);
            // 顶面(+Y面)
            meshData.normals[i + 8] = XMFLOAT3(0.0f, 1.0f, 0.0f);
            meshData.tangents[i + 8] = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
            // 底面(-Y面)
            meshData.normals[i + 12] = XMFLOAT3(0.0f, -1.0f, 0.0f);
            meshData.tangents[i + 12] = XMFLOAT4(-1.0f, 0.0f, 0.0f, 1.0f);
            // 背面(+Z面)
            meshData.normals[i + 16] = XMFLOAT3(0.0f, 0.0f, 1.0f);
            meshData.tangents[i + 16] = XMFLOAT4(-1.0f, 0.0f, 0.0f, 1.0f);
            // 正面(-Z面)
            meshData.normals[i + 20] = XMFLOAT3(0.0f, 0.0f, -1.0f);
            meshData.tangents[i + 20] = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
        }

        for (size_t i = 0; i < 6; ++i)
        {
            meshData.texcoords[i * 4] = XMFLOAT2(0.0f, 1.0f);
            meshData.texcoords[i * 4 + 1] = XMFLOAT2(0.0f, 0.0f);
            meshData.texcoords[i * 4 + 2] = XMFLOAT2(1.0f, 0.0f);
            meshData.texcoords[i * 4 + 3] = XMFLOAT2(1.0f, 1.0f);
        }

        meshData.indices16.resize(36);
        
        uint16_t indices[] = {
            0, 1, 2, 2, 3, 0,		// 右面(+X面)
            4, 5, 6, 6, 7, 4,		// 左面(-X面)
            8, 9, 10, 10, 11, 8,	// 顶面(+Y面)
            12, 13, 14, 14, 15, 12,	// 底面(-Y面)
            16, 17, 18, 18, 19, 16, // 背面(+Z面)
            20, 21, 22, 22, 23, 20	// 正面(-Z面)
        };
        memcpy_s(meshData.indices16.data(), sizeof indices, indices, sizeof indices);

        return meshData;
    }

    MeshData CreateCylinder(float radius, float height, uint32_t slices, uint32_t stacks, float texU, float texV)
    {
        using namespace DirectX;

        MeshData meshData;
        uint32_t vertexCount = (slices + 1) * (stacks + 3) + 2;
        uint32_t indexCount = 6 * slices * (stacks + 1);

        meshData.vertices.resize(vertexCount);
        meshData.normals.resize(vertexCount);
        meshData.tangents.resize(vertexCount);
        meshData.texcoords.resize(vertexCount);

        if (indexCount > 65535)
            meshData.indices32.resize(indexCount);
        else
            meshData.indices16.resize(indexCount);

        float h2 = height / 2;
        float theta = 0.0f;
        float per_theta = 2 * PI / slices;
        float stackHeight = height / stacks;
        //
        // 侧面部分
        //
        {
            // 自底向上铺设侧面端点
            size_t vIndex = 0;
            for (size_t i = 0; i < stacks + 1; ++i)
            {
                float y = -h2 + i * stackHeight;
                // 当前层顶点
                for (size_t j = 0; j <= slices; ++j)
                {
                    theta = j * per_theta;
                    float u = theta / 2 / PI;
                    float v = 1.0f - (float)i / stacks;

                    meshData.vertices[vIndex] = XMFLOAT3(radius * cosf(theta), y, radius * sinf(theta)), XMFLOAT3(cosf(theta), 0.0f, sinf(theta));
                    meshData.normals[vIndex] = XMFLOAT3(cosf(theta), 0.0f, sinf(theta));
                    meshData.tangents[i] = XMFLOAT4(-sinf(theta), 0.0f, cosf(theta), 1.0f);
                    meshData.texcoords[vIndex++] = XMFLOAT2(u * texU, v * texV);
                }
            }

            // 放入索引
            size_t iIndex = 0;
            for (uint32_t i = 0; i < stacks; ++i)
            {
                for (uint32_t j = 0; j < slices; ++j)
                {
                    if (indexCount > 65535)
                    {
                        meshData.indices32[iIndex++] = i * (slices + 1) + j;
                        meshData.indices32[iIndex++] = (i + 1) * (slices + 1) + j;
                        meshData.indices32[iIndex++] = (i + 1) * (slices + 1) + j + 1;

                        meshData.indices32[iIndex++] = i * (slices + 1) + j;
                        meshData.indices32[iIndex++] = (i + 1) * (slices + 1) + j + 1;
                        meshData.indices32[iIndex++] = i * (slices + 1) + j + 1;
                    }
                    else
                    {
                        meshData.indices16[iIndex++] = i * (slices + 1) + j;
                        meshData.indices16[iIndex++] = (i + 1) * (slices + 1) + j;
                        meshData.indices16[iIndex++] = (i + 1) * (slices + 1) + j + 1;

                        meshData.indices16[iIndex++] = i * (slices + 1) + j;
                        meshData.indices16[iIndex++] = (i + 1) * (slices + 1) + j + 1;
                        meshData.indices16[iIndex++] = i * (slices + 1) + j + 1;
                    }
                }
            }
        }

        //
        // 顶盖底盖部分
        //
        {
            size_t vIndex = (slices + 1) * (stacks + 1), iIndex = 6 * slices * stacks;
            uint32_t offset = static_cast<uint32_t>(vIndex);

            // 放入顶端圆心
            meshData.vertices[vIndex] = XMFLOAT3(0.0f, h2, 0.0f);
            meshData.normals[vIndex] = XMFLOAT3(0.0f, 1.0f, 0.0f);
            meshData.tangents[vIndex] = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
            meshData.texcoords[vIndex++] = XMFLOAT2(0.5f, 0.5f);

            // 放入顶端圆上各点
            for (uint32_t i = 0; i <= slices; ++i)
            {
                theta = i * per_theta;
                float u = cosf(theta) * radius / height + 0.5f;
                float v = sinf(theta) * radius / height + 0.5f;
                meshData.vertices[vIndex] = XMFLOAT3(radius * cosf(theta), h2, radius * sinf(theta));
                meshData.normals[vIndex] = XMFLOAT3(0.0f, 1.0f, 0.0f);
                meshData.tangents[vIndex] = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
                meshData.texcoords[vIndex++] = XMFLOAT2(u, v);
            }

            // 放入底端圆心
            meshData.vertices[vIndex] = XMFLOAT3(0.0f, -h2, 0.0f);
            meshData.normals[vIndex] = XMFLOAT3(0.0f, -1.0f, 0.0f);
            meshData.tangents[vIndex] = XMFLOAT4(-1.0f, 0.0f, 0.0f, 1.0f);
            meshData.texcoords[vIndex++] = XMFLOAT2(0.5f, 0.5f);

            // 放入底部圆上各点
            for (uint32_t i = 0; i <= slices; ++i)
            {
                theta = i * per_theta;
                float u = cosf(theta) * radius / height + 0.5f;
                float v = sinf(theta) * radius / height + 0.5f;
                meshData.vertices[vIndex] = XMFLOAT3(radius * cosf(theta), -h2, radius * sinf(theta));
                meshData.normals[vIndex] = XMFLOAT3(0.0f, -1.0f, 0.0f);
                meshData.tangents[vIndex] = XMFLOAT4(-1.0f, 0.0f, 0.0f, 1.0f);
                meshData.texcoords[vIndex++] = XMFLOAT2(u, v);
            }


            // 放入顶部三角形索引
            for (uint32_t i = 1; i <= slices; ++i)
            {
                if (indexCount > 65535)
                {
                    meshData.indices32[iIndex++] = offset;
                    meshData.indices32[iIndex++] = offset + i % (slices + 1) + 1;
                    meshData.indices32[iIndex++] = offset + i;
                }
                else
                {
                    meshData.indices16[iIndex++] = offset;
                    meshData.indices16[iIndex++] = offset + i % (slices + 1) + 1;
                    meshData.indices16[iIndex++] = offset + i;
                }

            }

            // 放入底部三角形索引
            offset += slices + 2;
            for (uint32_t i = 1; i <= slices; ++i)
            {
                if (indexCount > 65535)
                {
                    meshData.indices32[iIndex++] = offset;
                    meshData.indices32[iIndex++] = offset + i;
                    meshData.indices32[iIndex++] = offset + i % (slices + 1) + 1;
                }
                else
                {
                    meshData.indices16[iIndex++] = offset;
                    meshData.indices16[iIndex++] = offset + i;
                    meshData.indices16[iIndex++] = offset + i % (slices + 1) + 1;
                }
            }
        }
        

        return meshData;
    }

    MeshData CreateCone(float radius, float height, uint32_t slices)
    {
        using namespace DirectX;
        
        MeshData meshData;

        uint32_t vertexCount = 3 * slices + 1;
        uint32_t indexCount = 6 * slices;
        meshData.vertices.resize(vertexCount);
        meshData.normals.resize(vertexCount);
        meshData.tangents.resize(vertexCount);
        meshData.texcoords.resize(vertexCount);

        if (indexCount > 65535)
            meshData.indices32.resize(indexCount);
        else
            meshData.indices16.resize(indexCount);

        float h2 = height / 2;
        float theta = 0.0f;
        float per_theta = 2 * PI / slices;
        float len = sqrtf(height * height + radius * radius);
        
        //
        // 圆锥侧面
        //
        {
            size_t iIndex = 0;
            size_t vIndex = 0;

            // 放入圆锥尖端顶点(每个顶点位置相同，但包含不同的法向量和切线向量)
            for (uint32_t i = 0; i < slices; ++i)
            {
                theta = i * per_theta + per_theta / 2;
                meshData.vertices[vIndex] = XMFLOAT3(0.0f, h2, 0.0f);
                meshData.normals[vIndex] = XMFLOAT3(radius * cosf(theta) / len, height / len, radius * sinf(theta) / len);
                meshData.tangents[vIndex] = XMFLOAT4(-sinf(theta), 0.0f, cosf(theta), 1.0f);
                meshData.texcoords[vIndex++] = XMFLOAT2(0.5f, 0.5f);
            }

            // 放入圆锥侧面底部顶点
            for (uint32_t i = 0; i < slices; ++i)
            {
                theta = i * per_theta;
                meshData.vertices[vIndex] = XMFLOAT3(radius * cosf(theta), -h2, radius * sinf(theta));
                meshData.normals[vIndex] = XMFLOAT3(radius * cosf(theta) / len, height / len, radius * sinf(theta) / len);
                meshData.tangents[vIndex] = XMFLOAT4(-sinf(theta), 0.0f, cosf(theta), 1.0f);
                meshData.texcoords[vIndex++] = XMFLOAT2(cosf(theta) / 2 + 0.5f, sinf(theta) / 2 + 0.5f);
            }

            // 放入索引
            for (uint32_t i = 0; i < slices; ++i)
            {
                if (indexCount > 65535)
                {
                    meshData.indices32[iIndex++] = i;
                    meshData.indices32[iIndex++] = slices + (i + 1) % slices;
                    meshData.indices32[iIndex++] = slices + i % slices;
                }
                else
                {
                    meshData.indices16[iIndex++] = i;
                    meshData.indices16[iIndex++] = slices + (i + 1) % slices;
                    meshData.indices16[iIndex++] = slices + i % slices;
                }
            }
        }
        
        //
        // 圆锥底面
        //
        {
            size_t iIndex = 3 * (size_t)slices;
            size_t vIndex = 2 * (size_t)slices;

            // 放入圆锥底面顶点
            for (uint32_t i = 0; i < slices; ++i)
            {
                theta = i * per_theta;

                meshData.vertices[vIndex] = XMFLOAT3(radius * cosf(theta), -h2, radius * sinf(theta)),
                    meshData.normals[vIndex] = XMFLOAT3(0.0f, -1.0f, 0.0f);
                meshData.tangents[vIndex] = XMFLOAT4(-1.0f, 0.0f, 0.0f, 1.0f);
                meshData.texcoords[vIndex++] = XMFLOAT2(cosf(theta) / 2 + 0.5f, sinf(theta) / 2 + 0.5f);
            }
            // 放入圆锥底面圆心
            meshData.vertices[vIndex] = XMFLOAT3(0.0f, -h2, 0.0f),
                meshData.normals[vIndex] = XMFLOAT3(0.0f, -1.0f, 0.0f);
            meshData.texcoords[vIndex++] = XMFLOAT2(0.5f, 0.5f);

            // 放入索引
            uint32_t offset = 2 * slices;
            for (uint32_t i = 0; i < slices; ++i)
            {
                if (indexCount > 65535)
                {
                    meshData.indices32[iIndex++] = offset + slices;
                    meshData.indices32[iIndex++] = offset + i % slices;
                    meshData.indices32[iIndex++] = offset + (i + 1) % slices;
                }
                else
                {
                    meshData.indices16[iIndex++] = offset + slices;
                    meshData.indices16[iIndex++] = offset + i % slices;
                    meshData.indices16[iIndex++] = offset + (i + 1) % slices;
                }

            }
        }
        

        return meshData;
    }

    MeshData CreatePlane(const DirectX::XMFLOAT2& planeSize, const DirectX::XMFLOAT2& maxTexCoord)
    {
        return CreatePlane(planeSize.x, planeSize.y, maxTexCoord.x, maxTexCoord.y);
    }

    MeshData CreatePlane(float width, float depth, float texU, float texV)
    {
        using namespace DirectX;

        MeshData meshData;
        
        meshData.vertices.resize(4);
        meshData.normals.resize(4);
        meshData.tangents.resize(4);
        meshData.texcoords.resize(4);


        uint32_t vIndex = 0;
        meshData.vertices[vIndex] = XMFLOAT3(-width / 2, 0.0f, -depth / 2);
        meshData.normals[vIndex] = XMFLOAT3(0.0f, 1.0f, 0.0f);
        meshData.tangents[vIndex] = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
        meshData.texcoords[vIndex++] = XMFLOAT2(0.0f, texV);

        meshData.vertices[vIndex] = XMFLOAT3(-width / 2, 0.0f, depth / 2);
        meshData.normals[vIndex] = XMFLOAT3(0.0f, 1.0f, 0.0f);
        meshData.tangents[vIndex] = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
        meshData.texcoords[vIndex++] = XMFLOAT2(0.0f, 0.0f);

        meshData.vertices[vIndex] = XMFLOAT3(width / 2, 0.0f, depth / 2);
        meshData.normals[vIndex] = XMFLOAT3(0.0f, 1.0f, 0.0f);
        meshData.tangents[vIndex] = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
        meshData.texcoords[vIndex++] = XMFLOAT2(texU, 0.0f);

        meshData.vertices[vIndex] = XMFLOAT3(width / 2, 0.0f, -depth / 2);
        meshData.normals[vIndex] = XMFLOAT3(0.0f, 1.0f, 0.0f);
        meshData.tangents[vIndex] = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
        meshData.texcoords[vIndex++] = XMFLOAT2(texU, texV);

        meshData.indices16 = { 0, 1, 2, 2, 3, 0 };

        return meshData;
    }

}
