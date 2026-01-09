#pragma once

#include <vector>
#include <deque>
#include <span>
#include <array>
#include <map>


namespace UniDx
{

class Collider;
class Rigidbody;
class PhysicsShape;


// --------------------
// PhysicsGrid
// --------------------
class PhysicsGrid
{
public:
    typedef std::pair<PhysicsShape*, PhysicsShape*> PotentialPair;
    typedef MemberAction<Physics, PhysicsShape*, PhysicsShape*> CheckBoundFunc;

    PhysicsGrid(CheckBoundFunc checkBoundFunc);

    int nodeDivide = 8;
    int maxPerCell = 16;

    // グリッドに登録するShapeを更新
    void update(std::span<PhysicsShape> shapes);

    // 衝突する可能性のあるペアを集める
    void gatherPairs();

private:
    struct GridNode
    {
        Bounds bounds; // グリッドの境界
        Bounds shapeBounds; // グリッドに含まれるシェープを含む境界。隣のグリッドまで及ぶ可能性がある
        std::vector<PhysicsShape*> shapes; // このノードに属するコライダー
        std::vector<GridNode*> children; // 子グリッド３次元配列
        Vector3 leafCellSize;
        int smallShapeSize;
        int childX, childY, childZ;
        Vector3 stride;

        GridNode& initialize()
        {
            shapeBounds.extents = Vector3::negativeInfinity;
            shapes.clear();
            children.clear();
            smallShapeSize = 0;
            return *this;
        }
        GridNode* getChild(int x, int y, int z)
        {
            if (x < 0 || childX <= x || y < 0 || childY <= y || z < 0 || childZ < z) return nullptr;
            return children[x + childX * y + childX * childY * z];
        }
        void setChild(int x, int y, int z, GridNode* child) { children[x + childX * y + childX * childY * z] = child; }
        bool isLeaf() const { return children.size() == 0; }
        void makeChildren() { children.resize(childX * childY * childZ); }
        void setLeafCellSize(int divide, Vector3 cellMin)
        {
            leafCellSize = Max(bounds.size() / float(divide), cellMin);
            childX = int(std::ceil(bounds.size().x / leafCellSize.x));
            childY = int(std::ceil(bounds.size().y / leafCellSize.y));
            childZ = int(std::ceil(bounds.size().z / leafCellSize.z));
            stride = bounds.size() / Vector3(float(childX), float(childY), float(childZ));
        }
        bool isTooBig(Vector3 size) const
        { return size.x >= leafCellSize.x || size.y >= leafCellSize.y || size.z >= leafCellSize.z; }
    };
    CheckBoundFunc checkBoundF;
    std::deque<GridNode> gridNodes;
    int gridNodeSize;
    std::vector<std::span<PhysicsShape*>> traverseAncestorShapes;
    std::vector<GridNode*> traverseNeighborGrids;

    GridNode* addGridNode();
    void subdivide(GridNode* node, Vector3 cellMin);
    void insertShape(GridNode* node, PhysicsShape* shape, Vector3 cellMin);
    void insertShapeToChild(GridNode* node, PhysicsShape* shape, Vector3 cellMin);
    void traverseNode(GridNode* node, std::vector<std::span<PhysicsShape*>>& ancestorShapes, std::vector<GridNode*>& neighbor);
    void checkBounds(GridNode* node, PhysicsShape* shape);
};

}
