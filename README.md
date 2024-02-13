# Concurrent_Half_Edge_Mesh
A C++ Half Edge data structure based on hazard pointer technique, std::list and a custom memory allocator
## Idea
Pointer Based Half Edge Data structures are very flexible and efficient compared to indexed Based HE data structure when it comes to insertions.
But they have very bad cache locality, very low thread safety since multiple threads can remove a node at the same time while one of the thread is reading its value.

So , the idea is to use hazard pointers which offer thread safety to point, create and perform operations on the nodes.
Also having a custom memory allocator to maintain cache locality will give us the same performance as using an array kernel (index based data structures).   
