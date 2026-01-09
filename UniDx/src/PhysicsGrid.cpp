#include "pch.h"
#include <PhysicsGrid.h>

#include <algorithm>


namespace UniDx
{
    using namespace std;

    PhysicsGrid::PhysicsGrid(
        CheckBoundFunc checkBoundFunc)
        : gridNodeSize(0), checkBoundF(checkBoundFunc)
    {
        // GridNodesはできるだけ再利用し、最初にある程度の数を予約
        gridNodes.resize(64);
        traverseAncestorShapes.reserve(4);
        traverseNeighborGrids.reserve(28);
    }

    // グリッドに登録するShapeを更新
    void PhysicsGrid::update(std::span<PhysicsShape> shapes)
    {
        gridNodeSize = 0;

        // ルートグリッド作成
        GridNode* rootGrid = addGridNode();
        if (shapes.size() == 0) return;

        Vector3 shapeMin = Vector3::positiveInfinity;
        Vector3 shapeAve;
        for (auto& shape : shapes)
        {
            rootGrid->bounds.Encapsulate(shape.moveBounds);
            Vector3 s = shape.moveBounds.size();
            shapeMin = Min(s, shapeMin);
            shapeAve += s;
        }
        shapeAve /= float(shapes.size());
        Vector3 cellMin = Max(shapeAve, shapeMin * 2);
        // 最小セルは平均Shapeサイズにする。ただし最小コライダーがすれ違えない大きさより小さくしない

        rootGrid->setLeafCellSize(nodeDivide, cellMin);
        subdivide(rootGrid, shapeMin);

        // グリッドに登録
        for (auto& shape : shapes)
        {
            insertShape(rootGrid, &shape, shapeMin);
        }

    }

    // 衝突する可能性のあるペアを集める
    void PhysicsGrid::gatherPairs()
    {
        traverseAncestorShapes.clear();
        traverseNeighborGrids.clear();
        traverseNode(&gridNodes[0], traverseAncestorShapes, traverseNeighborGrids);
    }

    // 新しいグリッドノードを作成する（内部では再利用する）
    PhysicsGrid::GridNode* PhysicsGrid::addGridNode()
    {
        if (gridNodeSize == gridNodes.size())
        {
            gridNodes.emplace_back();
        }
        return &gridNodes[gridNodeSize++].initialize();
    }

    // 子ノードを作ってセルを分割
    void PhysicsGrid::subdivide(GridNode* node, Vector3 cellMin)
    {
        node->makeChildren(); // 子ノードのポインタ配列のみ作成

        // 子の中で小さいものを振り分ける
        for(auto it = node->shapes.begin(); it != node->shapes.end(); )
        {
            if(node->isTooBig((*it)->moveBounds.size()))
            {
                ++it;
            }
            else
            {
                insertShapeToChild(node, *it, cellMin);
                it = node->shapes.erase(it);
            }
        }
    }

    // shapeの位置に該当する子グリッドに挿入
    void PhysicsGrid::insertShapeToChild(GridNode* node, PhysicsShape* shape, Vector3 cellMin)
    {
        auto offset = shape->moveBounds.Center - node->bounds.min();
        auto indexVec = offset / node->stride;
 
        // 子のインデクス
        int ix = std::clamp(int(indexVec.x), 0, node->childX - 1);
        int iy = std::clamp(int(indexVec.y), 0, node->childY - 1);
        int iz = std::clamp(int(indexVec.z), 0, node->childZ - 1);
        auto* childNode = node->getChild(ix, iy, iz);
        if (childNode == nullptr)
        {
            // 子ノードを作って境界を計算
            childNode = addGridNode();
            auto boundsMin = node->bounds.min() + node->stride * Vector3(float(ix), float(iy), float(iz));
            auto boundsMax = boundsMin + node->stride;
            childNode->bounds.SetMinMax(boundsMin, boundsMax);
            childNode->setLeafCellSize(nodeDivide, cellMin);
            node->setChild(ix, iy, iz, childNode);
        }

        // グリッドに含まれるshape全体の境界を拡張
        childNode->shapeBounds.Encapsulate(shape->moveBounds);

        // Shapeを挿入（呼び出し元の再帰）
        insertShape(childNode, shape, cellMin);
    }

    // グリッドにshapeを挿入
    void PhysicsGrid::insertShape(GridNode* node, PhysicsShape* shape, Vector3 cellMin)
    {
        // この階層のセルサイズと比べて大きすぎるかどうか
        // セルサイズの２つ隣まで及ぶ可能性があるもの
        const bool tooBig = node->isTooBig(shape->moveBounds.size());
        if (tooBig)
        {
            node->shapes.push_back(shape);
            return; // 小さなセルには置かない
        }

        if (node->isLeaf())
        {
            // まだ分割されていない葉であれば、この階層に置く
            node->shapes.push_back(shape);
            node->smallShapeSize++;

            // 数が多すぎ＆まだ十分セルを小さくできるなら subdivide
            if (node->smallShapeSize > maxPerCell && node->childX * node->childY * node->childZ >= 8)
            {
                subdivide(node, cellMin);
            }
        }
        else
        {
            // 子にShapeを挿入
            insertShapeToChild(node, shape, cellMin);
        }
    }

    // 指定したshapeとノードに登録されたshapeの境界を調べて衝突可能性ペアを作る
    void PhysicsGrid::checkBounds(GridNode* node, PhysicsShape* shape)
    {
        // グリッドに直接登録されているものをチェック
        for (auto& nodeShape : node->shapes)
        {
            checkBoundF(shape, nodeShape);
        }

        if (node->isLeaf()) return;

        auto offsetMin = shape->moveBounds.min() - node->bounds.min();
        auto offsetMax = shape->moveBounds.max() - node->bounds.min();
        auto indexVecMin = offsetMin / node->stride;
        auto indexVecMax = offsetMax / node->stride;

        // 相手の最大半サイズ広げてチェックする
        auto extend = node->leafCellSize * 0.5f;
        int sx = std::max(int(indexVecMin.x - extend.x), 0);
        int sy = std::max(int(indexVecMin.y - extend.y), 0);
        int sz = std::max(int(indexVecMin.z - extend.z), 0);
        int ex = std::min(int(indexVecMax.x + extend.x) + 1, node->childX);
        int ey = std::min(int(indexVecMax.y + extend.y) + 1, node->childY);
        int ez = std::min(int(indexVecMax.z + extend.z) + 1, node->childZ);
        for (int z = sz; z < ez; ++z)
        {
            for (int y = sy; y < ey; ++y)
            {
                for (int x = sx; x < ex; ++x)
                {
                    auto* childGrid = node->getChild(x, y, z);
                    if (childGrid != nullptr && childGrid->shapeBounds.Intersects(shape->moveBounds))
                    {
                        checkBounds(childGrid, shape);
                    }
                }
            }
        }
    }

    // 指定したノードを巡って、衝突可能性のあるペアを作る
    void PhysicsGrid::traverseNode(GridNode* node, std::vector<std::span<PhysicsShape*>>& ancestorShapes, std::vector<GridNode*>& neighbor)
    {
        // 上の階層に属するShapeとこの階層のShape
        for (auto& shapes : ancestorShapes)
        {
            for (auto a : shapes)
            {
                for (auto* s : node->shapes)
                {
                    checkBoundF(a, s);
                }
            }
        }

        // この階層同士のShape
        for (int i = 0; i < node->shapes.size(); ++i)
        {
            for (int j = i + 1; j < node->shapes.size(); ++j)
            {
                checkBoundF(node->shapes[i], node->shapes[j]);
            }
        }

        // この階層と近隣グリッド
        for (auto* n : neighbor)
        {
            if (n == nullptr) continue;
            for (auto* s : node->shapes)
            {
                checkBounds(n, s);
            }
        }

        if (node->isLeaf())
        {
            return;
        }

        // この階層を積む
        ancestorShapes.push_back(node->shapes);

        for (int z = 0; z < node->childZ; ++z)
        {
            for (int y = 0; y < node->childY; ++y)
            {
                for (int x = 0; x < node->childX; ++x)
                {
                    auto c = node->getChild(x, y, z);
                    if (c == nullptr) continue;
                    neighbor.push_back(node->getChild(x-1, y-1, z-1));
                    neighbor.push_back(node->getChild(x-1, y-1, z));
                    neighbor.push_back(node->getChild(x-1, y, z-1));
                    neighbor.push_back(node->getChild(x-1, y, z));
                    neighbor.push_back(node->getChild(x, y-1, z-1));
                    neighbor.push_back(node->getChild(x, y-1, z));
                    neighbor.push_back(node->getChild(x, y, z-1));
                    traverseNode(c, ancestorShapes, neighbor);
                    neighbor.resize(neighbor.size()-7);
                }
            }
        }

        // この階層を戻す
        ancestorShapes.pop_back();
    }

} // UniDx
