#include <iostream>

#include "half_edge.h"
int main() {
    // Create a half-edge allocator
    HalfEdgeAllocator allocator;

    // Create a half-edge mesh
    HalfEdgeMesh mesh;

    // Add vertices to the mesh
    Vertex* v1 = mesh.addVertex(0.0, 0.0, 0.0);
    Vertex* v2 = mesh.addVertex(1.0, 1.0, 1.0);
    Vertex* v3 = mesh.addVertex(2.0, 2.0, 2.0);
    Vertex* v4 = mesh.addVertex(0.0, 0.0, 0.0); // Adding duplicate vertex

    // Allocate some half-edges
    HalfEdge* e1 = allocator.allocate();
    HalfEdge* e2 = allocator.allocate();
    HalfEdge* e3 = allocator.allocate();

    // Set up half-edge pointers
    e1->twinEdge = e2;
    e1->nextEdge = e3;
    e1->prevEdge = nullptr;
    e1->vertex = v1;

    e2->twinEdge = e1;
    e2->nextEdge = nullptr;
    e2->prevEdge = nullptr;
    e2->vertex = v2;

    e3->twinEdge = nullptr;
    e3->nextEdge = nullptr;
    e3->prevEdge = e1;
    e3->vertex = v3;

    return 0;
}
