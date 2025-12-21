#include "pch.h"
#include <UniDx/Mesh.h>

#include <UniDx/D3DManager.h>

namespace UniDx{


void SubMesh::createVertexBuffer(void* data)
{
    // 事前に設定されたstrideと位置の数でデータサイズを計算
    UINT byteSize = static_cast<UINT>(stride * positions.size());

    // 作成するバッファの仕様を決める
    D3D11_BUFFER_DESC vbDesc = {};
    vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;	// デバイスにバインドするときの種類(頂点バッファ)
    vbDesc.ByteWidth = byteSize;				    // 作成するバッファのバイトサイズ
    vbDesc.MiscFlags = 0;							// その他のフラグ
    vbDesc.StructureByteStride = 0;					// 構造化バッファの場合、その構造体のサイズ
    vbDesc.Usage = D3D11_USAGE_DEFAULT;				// 作成するバッファの使用法
    vbDesc.CPUAccessFlags = 0;

    // 上の仕様を渡して頂点バッファを作ってもらう
    D3D11_SUBRESOURCE_DATA initData = { data, byteSize, 0 };	// 書き込むデータ

    // 頂点バッファの作成
    D3DManager::getInstance()->GetDevice()->CreateBuffer(&vbDesc, &initData, &vertexBuffer);
}


void SubMesh::createIndexBuffer()
{
    // データサイズを計算
    UINT byteSize = static_cast<UINT>(indices.size() * sizeof(uint32_t));

    // 作成するバッファの仕様を決める
    D3D11_BUFFER_DESC vbDesc = {};
    vbDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;	    // デバイスにバインドするときの種類(インデックスバッファ)
    vbDesc.ByteWidth = byteSize;				    // 作成するバッファのバイトサイズ
    vbDesc.MiscFlags = 0;							// その他のフラグ
    vbDesc.StructureByteStride = 0;					// 構造化バッファの場合、その構造体のサイズ
    vbDesc.Usage = D3D11_USAGE_DEFAULT;				// 作成するバッファの使用法
    vbDesc.CPUAccessFlags = 0;

    // 上の仕様を渡して頂点バッファを作ってもらう
    D3D11_SUBRESOURCE_DATA initData = { &indices.front(), byteSize, 0};	// 書き込むデータ

    // 頂点バッファの作成
    D3DManager::getInstance()->GetDevice()->CreateBuffer(&vbDesc, &initData, &indexBuffer);
}


void SubMesh::render() const
{
    // 頂点バッファを描画で使えるようにセットする
    UINT offset = 0;
    D3DManager::getInstance()->GetContext()->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);

    // プロミティブ・トポロジーをセット
    D3DManager::getInstance()->GetContext()->IASetPrimitiveTopology(topology);

    // GPUへの描画命令発行
    if (indices.size() > 0 && indexBuffer)
    {
        // インデックスバッファを使う場合
        D3DManager::getInstance()->GetContext()->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
        D3DManager::getInstance()->GetContext()->DrawIndexed(static_cast<UINT>(indices.size()), 0, 0);
    }
    else
    {
        // 頂点データのみ場合
        D3DManager::getInstance()->GetContext()->Draw(static_cast<UINT>(positions.size()), 0);
    }
}


}
