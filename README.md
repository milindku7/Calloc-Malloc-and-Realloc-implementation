# Calloc-Malloc-and-Realloc-implementation
A personalized implementation of Calloc, Malloc and Realloc. Calls sbrk to get the address of a void pointer that provides 4096 bytes of space in the heap. 
It then breaks down the bytes into a free list that contains void pointers containing addresses to different-sized blocks
Based on the number of bytes needed by either of the commands, the code allots the closest 2^n bytes to it. 
