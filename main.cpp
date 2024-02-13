#include "half_edge.h"

int main() {
    // Create a half-edge allocator
    HalfEdgeAllocator allocator;

    // Allocate some half-edges
    HalfEdge* e1 = allocator.allocate();
    HalfEdge* e2 = allocator.allocate();
    HalfEdge* e3 = allocator.allocate();

    // Deallocate some half-edges
    allocator.deallocate(e1);
    allocator.deallocate(e2);
    allocator.deallocate(e3);

    // Allocate more half-edges
    HalfEdge* e4 = allocator.allocate();
    HalfEdge* e5 = allocator.allocate();

    return 0;
}
