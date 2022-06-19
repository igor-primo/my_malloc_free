// Here I will implement a custom version of
// malloc and free as an exercise.

#include<stdio.h> // print stuff
#include<assert.h> // test stuff and die when depressed
#include<stdbool.h> // to use proper boolean types in the asserts
#include<stdlib.h> // abort

#define HEAP_CAPACITY 64000
#define CHUNK_LIST_CAPACITY 1024

// let us help ourselves
#define UNIMPLEMENTED \
	do { \
		fprintf(stderr, "%s:%d: UNIMPLEMENTED: %s no implemented\n", \
				__FILE__, __LINE__, __func__); \
		abort(); \
	} while(0)

// We need to keep track
// the size of our chunks, besides
// of where they start
typedef struct {
	char *start;
	size_t size;
} chunk_t;

// We will create lists of chunks in order
// to track them in the heap
// we use a struct in order to aggregate
// the list with its size, which is count.
// We will keep the chunks arrays sorted, so we
// avoid fragmentation of our arrays.
// This will make possible for us to iterate over them
// easily, hopefully.
typedef struct {
	size_t count;
	chunk_t chunks[CHUNK_LIST_CAPACITY];
} chunk_list_t;

// Initialize the heap array
// with all zeroes.
// This heap array will be our heap.
char heap[HEAP_CAPACITY] = {0};
//size_t heap_size = 0;

// This array will serve as a map
// for us to access correct pointers in the previous array
chunk_list_t allocced_chunks = {0};

// We also need to keep track
// of free chunks
chunk_list_t freed_chunks = {
	.chunks = {
		[0] = {
		   .start = heap, 
		   .size = sizeof(heap)
		}
   },
	.count = 1,
};

chunk_list_t tmp_chunks = {0};

int chunk_start_compar(const void *a, const void *b){
	const chunk_t *a_c = a;
	const chunk_t *b_c = b;
	return a_c->start - b_c->start;
}

int chunk_list_find(const chunk_list_t *list, void *ptr){
	for(size_t i=0;i<list->count;++i)
		if(list->chunks[i].start == ptr)
			return (int) i;
	return -1;
}

void chunk_list_insert(chunk_list_t *list, void *ptr, size_t size){
	assert(list->count < CHUNK_LIST_CAPACITY);
	// Append a new chunk to this list
	list->chunks[list->count].start = ptr;
	list->chunks[list->count].size = size;
	// Sort the chunks arrays
	for(size_t i=list->count; 
		i > 0 
			&& list->chunks[i].start 
				< list->chunks[i-1].start; --i){
		// Make the swap
		const chunk_t ch = list->chunks[i];
		list->chunks[i] = list->chunks[i-1];
		list->chunks[i-1] = ch;
	}
	list->count++;
}

void chunk_list_remove(chunk_list_t *list, size_t index){
	assert(index < list->count);
	for(size_t i=index;i<list->count-1;++i)
		list->chunks[i] = list->chunks[i+1];
	list->count--;
}

/* function to merge the free chunks for further 
 * allocations
 */
void chunk_list_merge(chunk_list_t *dst, const chunk_list_t *src){
	dst->count = 0;
	for(size_t i=0;i<src->count;++i){
		const chunk_t chunk = src->chunks[i];
		if(dst->count > 0){
			chunk_t * top = &dst->chunks[dst->count-1];
			if(top->start + top->size == chunk.start){
				top->size += chunk.size;
			} else {
				chunk_list_insert(dst, chunk.start, chunk.size);
			}
		} else {
			chunk_list_insert(dst, chunk.start, chunk.size);
		}
	}
}

void *heap_alloc(size_t size){
	if(size > 0) {
		chunk_list_merge(&tmp_chunks, &freed_chunks);
		freed_chunks = tmp_chunks;
		for(size_t i=0;i<freed_chunks.count;++i){
			const chunk_t chunk = freed_chunks.chunks[i];
			if(chunk.size >= size){ // Get a free chunk where my request fits
				chunk_list_remove(&freed_chunks, i); // prepare

				const size_t tail_size = chunk.size - size;
				chunk_list_insert(&allocced_chunks, chunk.start, size); // alloc new chnk
				if(tail_size > 0){ // If we still have space
					// set new start for the free chunk
					chunk_list_insert(&freed_chunks, chunk.start + size, tail_size);
				}
				return chunk.start;;
			}
		}
	}

	return NULL;
}

void chunk_list_dump(const chunk_list_t *list){
	printf("chunks (%zu):\n", list->count);
	for(size_t i = 0; i < list->count; ++i){
		printf("	start: %p, size: %zu\n", 
				list->chunks[i].start,
				list->chunks[i].size);
	}
}

void heap_free(void *ptr){
	if(ptr != NULL){
		// get index of allocated chunk of this pointer
		const int index = chunk_list_find(&allocced_chunks, ptr);
		assert(index >= 0);
		// put it into the free list
		chunk_list_insert(&freed_chunks, 
				allocced_chunks.chunks[index].start,
				allocced_chunks.chunks[index].size);
		chunk_list_remove(&allocced_chunks, (size_t) index);
	}
}

void heap_collect(){
	UNIMPLEMENTED;
}

#define N 10

void *ptrs[N] = {0};

int main(){

	// Let us observe a fragmented heap
	for(int i=0;i<N;i++){
		ptrs[i] = heap_alloc(i);
	}

	for(int i=0;i<N;++i)
		if(i % 2 == 0)
			heap_free(ptrs[i]);

	heap_alloc(10);
	chunk_list_dump(&allocced_chunks);
	chunk_list_dump(&freed_chunks);

	return 0;
}
