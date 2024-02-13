#include <iostream>
#include <vector>
#include <deque>
#include <unordered_set>
#include <atomic>
#include <thread>
#include <memory>

#define MAX_HALF_EDGES_PER_BLOCK 1000

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
    HalfEdge() 
    {
      twinEdge = nullptr;
      nextEdge = nullptr;
      prevEdge = nullptr;
      vertex = nullptr;
    }

    HalfEdge* twinEdge;
    HalfEdge* nextEdge;
    HalfEdge* prevEdge;
    Vertex*   vertex;
    // Add more fields as needed
};


struct HalfEdgeFace 
{

  HalfEdgeFace()
  {
    face_head_edge = new HalfEdge();
  }
  
  HalfEdgeFace(HalfEdge* edge)
  {
    delete[] face_head_edge;
  }
  
  HalfEdge* face_head_edge;
  
  void add_next_edge(HalfEdge* & edge)
  { get_edges().back()->nextEdge = edge; }
  
  void add_prev_edge(HalfEdge* & edge)
  { get_edges().back()->prevEdge = edge; }

  void add_twin_edge(HalfEdge* & edge)
  { get_edges().back()->twinEdge = edge; }

  void add_vertex(Vertex* & vertex)
  { get_edges().back()->vertex = vertex; }
 
  HalfEdge* get_next_edge()
  { return get_edges().back()->nextEdge; }

  HalfEdge* get_prev_edge()
  { return get_edges().back()->prevEdge; }

  std::vector<HalfEdge*> get_edges()
  {
    std::vector<HalfEdge*> edges;
    HalfEdge* edge = face_head_edge;
    do
    {
      edges.push_back(edge);
      edge = edge->nextEdge;
    } while (edge != face_head_edge);
    return edges;
  }

  std::vector<Vertex*> get_vertices()
  {
    std::vector<Vertex*> vertices;
    HalfEdge* edge = face_head_edge;
    do
    {
      vertices.push_back(edge->vertex);
      edge = edge->nextEdge;
    } while (edge != face_head_edge);
    return vertices;
  }

};


// Hazard pointer structure
template<typename T>
struct HazardPointer {
    std::atomic<T*> ptr;
    std::atomic_flag write_lock = ATOMIC_FLAG_INIT;
};

class HalfEdgeAllocator {
private:
    std::deque<std::vector<HalfEdge>> memoryPool;
    std::vector<int> freeIndices;
    HazardPointer<HalfEdge>* hazardPointers;

public:
    HalfEdgeAllocator() {
        // Initialize freeIndices with all indices
        freeIndices.resize(MAX_HALF_EDGES_PER_BLOCK);
        for (int i = 0; i < MAX_HALF_EDGES_PER_BLOCK; ++i) {
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
            if (!hp.write_lock.test_and_set()) {
                ptr = hp.ptr.load();
                hp.write_lock.clear();
            }

            if (ptr) {
                // We have a hazard pointer, check if it's safe to use
                if (isValidPointer(ptr)) {
                    return ptr;
                }
            } else {
                if (freeIndices.empty()) {
                    // If memory pool is exhausted, allocate a new vector
                    memoryPool.emplace_back(MAX_HALF_EDGES_PER_BLOCK);
                    for (int i = 0; i < MAX_HALF_EDGES_PER_BLOCK; ++i) {
                        freeIndices.push_back(i + (memoryPool.size() - 1) * MAX_HALF_EDGES_PER_BLOCK);
                    }
                }
                int index = freeIndices.back();
                freeIndices.pop_back();
                int vectorIndex = index / MAX_HALF_EDGES_PER_BLOCK;
                int elementIndex = index % MAX_HALF_EDGES_PER_BLOCK;
                ptr = &memoryPool[vectorIndex][elementIndex];

                // Set hazard pointer
                hp.ptr.store(ptr);
                hp.write_lock.clear();

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


