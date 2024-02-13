#include <iostream>
#include <vector>
#include <unordered_set>
#include <atomic>
#include <thread>
#include <memory>

#define MAX_HALF_EDGES 1000

// Define your vertex structure
struct Vertex {
    double x, y, z;
    // Add more fields as needed

    // Define custom hash function for Vertex
    struct Hash {
        size_t operator()(const Vertex& v) const {
            size_t h1 = std::hash<double>{}(v.x);
            size_t h2 = std::hash<double>{}(v.y);
            size_t h3 = std::hash<double>{}(v.z);
            return h1 ^ (h2 << 1) ^ (h3 << 2);
        }
    };

    // Define custom equality function for Vertex
    struct Equal {
        bool operator()(const Vertex& v1, const Vertex& v2) const {
            return v1.x == v2.x && v1.y == v2.y && v1.z == v2.z;
        }
    };
};

// Define your half-edge structure
struct HalfEdge {
    HalfEdge* twinEdge;
    HalfEdge* nextEdge;
    HalfEdge* prevEdge;
    Vertex*   vertex;
    // Add more fields as needed
};

// Hazard pointer structure
struct HazardPointer {
    std::atomic<HalfEdge*> ptr;
    std::atomic_flag locked = ATOMIC_FLAG_INIT;
};

class HalfEdgeAllocator {
private:
    std::vector<HalfEdge> memoryPool;
    std::vector<int> freeIndices;
    int nextFreeIndex;
    HazardPointer* hazardPointers;

public:
    HalfEdgeAllocator() : memoryPool(MAX_HALF_EDGES), nextFreeIndex(0) {
        // Initialize freeIndices with all indices
        freeIndices.reserve(MAX_HALF_EDGES);
        for (int i = 0; i < MAX_HALF_EDGES; ++i) {
            freeIndices.push_back(i);
        }

        // Initialize hazard pointers
        hazardPointers = new HazardPointer[std::thread::hardware_concurrency()];
        for (size_t i = 0; i < std::thread::hardware_concurrency(); ++i) {
            hazardPointers[i].ptr.store(nullptr);
        }
    }

    ~HalfEdgeAllocator() {
        delete[] hazardPointers;
    }

    // Allocate a new half-edge from the memory pool
    HalfEdge* allocate() {
        if (freeIndices.empty()) {
            std::cerr << "Memory pool exhausted!" << std::endl;
            return nullptr; // Or handle error differently
        }
        int index = freeIndices.back();
        freeIndices.pop_back();
        return &memoryPool[index];
    }

    // Deallocate a previously allocated half-edge
    void deallocate(HalfEdge* edge) {
        if (!edge) return;
        int index = edge - &memoryPool[0];
        if (index < 0 || index >= MAX_HALF_EDGES) {
            std::cerr << "Invalid index for deallocation!" << std::endl;
            return;
        }
        freeIndices.push_back(index);
    }

    // Accessor to get a reference to a half-edge by index
    HalfEdge& getHalfEdge(int index) {
        return memoryPool[index];
    }

    // Get a hazard pointer
    HazardPointer& getHazardPointer() {
        return hazardPointers[std::hash<std::thread::id>{}(std::this_thread::get_id()) % std::thread::hardware_concurrency()];
    }
};

class HalfEdgeMesh {
private:
    std::unordered_set<Vertex, Vertex::Hash, Vertex::Equal> vertexPool;

public:
    // Method to add a vertex to the mesh
        Vertex* addVertex(double x, double y, double z) {
        auto v = vertexPool.emplace(Vertex{x, y, z}).first;
        return const_cast<Vertex*>(&(*v));// Required as emplace returns a const iterator
    }

    // Add methods to manipulate the half-edge mesh as needed
};
