#include <iostream>
#include <vector>
#include <deque>
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


struct HalfEdgeFace 
{
  std::deque<HalfEdge*> edges;
};


// Hazard pointer structure
template<typename T>
struct HazardPointer {
    std::atomic<T*> ptr;
    std::atomic_flag locked = ATOMIC_FLAG_INIT;
};

class HalfEdgeAllocator {
private:
    std::deque<std::vector<HalfEdge>> memoryPool;
    std::vector<int> freeIndices;
    HazardPointer<HalfEdge>* hazardPointers;

public:
    HalfEdgeAllocator() {
        // Initialize freeIndices with all indices
        freeIndices.reserve(MAX_HALF_EDGES);
        for (int i = 0; i < MAX_HALF_EDGES; ++i) {
            freeIndices.push_back(i);
        }

        // Initialize hazard pointers
        hazardPointers = new HazardPointer<HalfEdge>[std::thread::hardware_concurrency()];
        for (size_t i = 0; i < std::thread::hardware_concurrency(); ++i) {
            hazardPointers[i].ptr.store(nullptr);
        }
    }

    ~HalfEdgeAllocator() {
        delete[] hazardPointers;
    }

    // Allocate a new half-edge from the memory pool
    HalfEdge* allocate() {
        // Get a hazard pointer
        HazardPointer<HalfEdge>& hp = getHazardPointer();

        HalfEdge* hazardPtr = nullptr;
        while (true) {
            HalfEdge* ptr = nullptr;
            if (!hp.locked.test_and_set()) {
                ptr = hp.ptr.load();
                hp.locked.clear();
            }

            if (ptr) {
                // We have a hazard pointer, check if it's safe to use
                if (isValidPointer(ptr)) {
                    return ptr;
                }
            } else {
                if (freeIndices.empty()) {
                    // If memory pool is exhausted, allocate a new vector
                    memoryPool.emplace_back(MAX_HALF_EDGES);
                    for (int i = 0; i < MAX_HALF_EDGES; ++i) {
                        freeIndices.push_back(i + (memoryPool.size() - 1) * MAX_HALF_EDGES);
                    }
                }
                int index = freeIndices.back();
                freeIndices.pop_back();
                int vectorIndex = index / MAX_HALF_EDGES;
                int elementIndex = index % MAX_HALF_EDGES;
                ptr = &memoryPool[vectorIndex][elementIndex];

                // Set hazard pointer
                hp.ptr.store(ptr);
                hp.locked.clear();

                return ptr;
            }
        }
    }

    // Deallocate a previously allocated half-edge
    void deallocate(HalfEdge* edge) {
        // No need to deallocate in hazard pointer model
    }

    // Check if a pointer is valid
    bool isValidPointer(HalfEdge* ptr) {
        // Add your validation logic here
        return true;
    }

    // Get a hazard pointer
    HazardPointer<HalfEdge>& getHazardPointer() {
        return hazardPointers[std::hash<std::thread::id>{}(std::this_thread::get_id()) % std::thread::hardware_concurrency()];
    }
};


