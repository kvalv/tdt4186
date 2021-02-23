#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
void print_heap();
int has_initialized = 0;
char g_block_id= 'a';

// our memory area we can allocate from, here 64 kB
#ifndef MEM_SIZE
#define MEM_SIZE (1024*64)
#endif
uint8_t heap[MEM_SIZE];

// start and end of our own heap memory area
void *managed_memory_start;

// this block is stored at the start of each free and used block
struct mem_control_block {
    int size;
    struct mem_control_block *next;
};

int block_size = sizeof(struct mem_control_block);

// pointer to start of our free list
struct mem_control_block *free_list_start;
typedef int bool ;

void mymalloc_init() {

    // our memory starts at the start of the heap array
    managed_memory_start = &heap;

    // allocate and initialize our memory control block
    // for the first (and at the moment only) free block
    struct mem_control_block *m = (struct mem_control_block *)managed_memory_start;
    m->size = MEM_SIZE;

    // no next free block
    m->next = (struct mem_control_block *)0;

    // initialize the start of the free list
    free_list_start = m;

    // We're initialized and ready to go
    has_initialized = 1;
}

unsigned char* mv_contents(unsigned char *src, size_t nbytes, int offset) {
    unsigned char* dest = src + offset;
    unsigned char copy[nbytes];
    for (int i =0; i < nbytes; i++ ) {
        copy[i] = src[i];
        src[i] = 0;  // zero out.
    }
    for (int i =0; i <  nbytes; i++) {
        dest[i] = copy[i];
    }
    return dest;
}

struct mem_control_block* mv_block(struct mem_control_block* b, int offset) {
    unsigned char* dst = mv_contents(
                             (unsigned char*) b,
                             sizeof(struct mem_control_block),
                             offset);
    return (struct mem_control_block*) dst;
}

void fill(void *loc, unsigned char value, size_t nbytes) {
    for (size_t i =0; i < nbytes; i++) {
        ((unsigned char*)loc)[i] = value;
    }
}

struct mem_control_block* mk_occupied_block(void* loc, size_t size) {
    fill(loc, 0, block_size);
    struct mem_control_block *b = ((struct mem_control_block*)loc);
    b->size = size;
    // Fill the `next` attribute with a magic variable that makes it a bit
    // easier for us to recognize where the occupied blocks are located.
    // This is not necessary for completing the task, but it makes it a bit
    // easier to write some tests and visualise whether the results are good
    // or not.
    long unsigned int id = 0xaaaabbbbccccdd00 | g_block_id;
    g_block_id ++;
    b->next = (struct mem_control_block*) id;
    return b;
}

void *mymalloc(long numbytes) {
    if (has_initialized == 0) {
        mymalloc_init();
    }
    if (free_list_start == NULL || numbytes <= 0) {
        return NULL;
    }

    // round up to nearest block size, for easier alignment if necessary.
    long offset = numbytes;
    if (offset % 8) {
        offset = offset + 8 - (offset % 8);
    }

    // we also would need to store metadata, so add that as well.
    offset += block_size;

    struct mem_control_block *a = NULL;
    struct mem_control_block *b = free_list_start;

    while (b->size < offset) {
        a = b;
        b = b->next;
        if (b == NULL) {
            return NULL;
        }
    }

    // Pointer to the 'malloced' block.
    void *addr = (void*) b;

    int can_split = (b->size - offset) >= block_size;
    if (can_split) {
        struct mem_control_block *x = mv_block(b, offset);
        x->size -= numbytes;
        if (a == NULL) {
            free_list_start = x;
        } else {
            a->next = x;
        }
    } else {
        // no new free block created.
        if (b == free_list_start) {
            free_list_start = NULL;
        }
        if (a != NULL) {
            a->next = b->next;
        }
    }
    mk_occupied_block(addr, offset);
    // struct mem_control_block* x = (struct mem_control_block* ) addr;

    // increment by `block_size` because that's what we actual
    // allow the user to modify.
    return addr + block_size;
}

void wipe(struct mem_control_block* b) {
    fill((void*) b, 0, b->size);
}

void myfree(void *firstbyte) {
    // mymalloc returns the first byte location of data. We want to
    // modify the block, so we have to move one block_size to the left.
    void *loc = firstbyte - block_size;
    struct mem_control_block* b = (struct mem_control_block*) loc;

    // write content with zero's. Again, not necessary but we do it
    // for visualization purposes. Note we should not zero out the
    // mem control block, that's why we use `firstbyte`.
    fill(firstbyte, 0, b->size - block_size);

    if (free_list_start == NULL ) {
        free_list_start = b;
        b->next = NULL;
    } else if (free_list_start > b) {
        struct mem_control_block *copy = free_list_start;
        free_list_start = b;
        if ((void*) b + b->size == copy) { // merge
            b->size += copy->size;
            b->next = copy->next;
            wipe(copy);
        } else {
            b->next = copy;
        }
    } else if (b > free_list_start) {
        struct mem_control_block *a, *c;
        a = free_list_start;
        c = a->next;
        while (c != NULL && c < b) {
            a = a->next;
            c = a->next;
        }
        if ((void*)a + a->size == b) { // merge
            a->size += b->size;
            wipe(b);
        } else if ((void*) b + b->size == c) { // merge
            a->next = b;
            b->next = c->next;
            b->size += c->size;
            wipe(c);
        } else {
            a->next = b;
            b->next = c;
        }
    } else {
        fprintf(stderr, "unknown clause\n");
        exit(1);
    }

}

bool matches_magic_string(uint8_t* ptr) {
    return ptr[1] == 0xdd && ptr[2] == 0xcc && ptr[3] == 0xcc &&
           ptr[4] == 0xbb && ptr[5] == 0xbb && ptr[6] == 0xaa && ptr[7] == 0xaa;
}

void print_heap() {
    unsigned char result [ (MEM_SIZE / 8) + 1];
    result[MEM_SIZE / 8] = 0; // termination
    for (int i = 0; i < MEM_SIZE; i += 8) {
        int v0 = heap[i];
        bool is_data = v0 != 0;
        bool is_zeros = v0 == 0;
        for (int j = 0; j < sizeof(int); j++) {
            if (v0 != heap[i + j] ) {
                is_data = 0;
            }
            if (heap[i + j] != 0) {
                is_zeros = 0;
            }
        }
        if (is_data) {
            result[i / 8] = 'D'; // data
        }
        if (is_zeros) {
            result[i / 8] = 'Z';  // zero data (0's only)
        }
    }
    for (int i = 0; i < MEM_SIZE; i += 8) {
        if (matches_magic_string(heap + i)) {
            result[i/8 - 1] = 'O';   // occupied mem block
            result[i/8 ] = heap[i];  // occupied mem block: id
        }
    }
    struct mem_control_block *b = free_list_start;
    while (b != NULL) {
        int offset = (int) ((void*) b - (void*) heap );
        result[offset / 8] = 'F'; // mem-block: free
        result[offset / 8 + 1] = 'F'; // mem-block: free
        b = b->next;
    }

    for (int i =0; i < MEM_SIZE/8; i++) {
        printf("%c", result[i]);
    }
    printf("\n");

}

bool block_contains_constant_value(struct mem_control_block* b, uint8_t value) {
    // no data indicate it's not a block
    uint8_t* pdata;
    if (b->size == 0) {
        return 0;
    }
    for (int i = block_size; i < b->size; i++) {
        pdata = (void*) b + i;
        if (*pdata != value) {
            return 0;
        }
    }
    return 1;
}

bool malloc_contains_constant_value(void *ptr, uint8_t value) {
    struct mem_control_block* b = (struct mem_control_block*) (ptr - block_size);
    return block_contains_constant_value(b, value);
}

void* block_from_id(char id) {
    for (int i =0; i < MEM_SIZE; i+=8) {
        if (matches_magic_string(heap + i)) {
            if (*((char*)(heap + i)) == id) {
                return heap + i + 8;
            }
        }
    }


    return NULL;
}

void run_test_case(char* actions) {
    char *token = strtok(actions, " ");
    void* p;

    while (token != NULL) {
        long numbytes;
        char block_id;
        void* firstbyte;
        switch (token[0]) {
        case 'M': // malloc
            token++;
            numbytes = atoi(token);
            p = mymalloc(numbytes);
            if (p != NULL) {
                fill(p, 1, numbytes);
            }
            break;
        case 'F': // free
            block_id = token[1];

            firstbyte = block_from_id(block_id);
            if (firstbyte == NULL) {
                fprintf(stderr, "block id '%c' not found.\n", block_id);
                exit(1);
            }
            myfree(firstbyte);
            break;
        }
        token = strtok(NULL, " ");
    }
}

int main(int argc, char **argv) {

    if (argc > 1 && (strcmp(argv[1], "-t") == 0)) {
        run_test_case(argv[2]);
        print_heap();
        return 0;
    }

    printf("For running tests, please see the README.md\n");


    return 0;
}
