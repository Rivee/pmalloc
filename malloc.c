#define _DEFAULT_SOURCE 
#define _XOPEN_SOURCE 500

#include <unistd.h>

#undef malloc
#undef realloc
#undef calloc
#undef free

struct chunk {
  struct chunk *next, *prev;
  size_t        size;
  long          free;
  void         *data;
};

// Function to align with the size of size_t
static inline size_t word_align(size_t n)
{
    return ((n + sizeof(size_t) - 1) & -sizeof(size_t)); 
}

// Set to zero ptr with the length
void zerofill(void *ptr, size_t len)
{
    for (size_t i = 0; i < len; i++) {
        ptr = 0;
        ptr++;
    }
}

// Copy scr to dst
void wordcpy(void *dst, void *src, size_t len)
{
    for(size_t i = 0; i < len; i++) {
        *(char *)dst++ = *(char *)src++;
    }

}

// Return the base chunk if it was not created, else return new one
static struct chunk* get_base(void)
{
  static struct chunk *base = NULL;
  if (base == NULL) { // Check if base hasn't been created
      base = (struct chunk*)sbrk(0);
      if (sbrk(sizeof(struct chunk)) == (void*) -1)
          return NULL;
      base->next = NULL;
      base->prev = NULL;
      base->size = 0;
      base->free = 0;
      base->data = NULL;
  }
  return base;
}

// extend the head with the next size
static int extend_heap(struct chunk *last, size_t size)
{
    if (last->next != NULL)
        return 0;
    last->next = sbrk(0);
    if (sbrk(sizeof(struct chunk) + size) == (void*) -1)
        return 0;
    last->next->size = size;
    last->next->next = NULL;
    last->next->free = 0;
    last->next->data = (void*)sbrk(0) - size; // get the data ptr on getting the actual data limit and the size
    return 1;  
}

// research a free chunk
static struct chunk* find_chunk(size_t size)
{
    struct chunk *bs = get_base();
    while (bs->next) {
       if (bs->next->size >= size && bs->next->free)
           return bs;
        bs = bs->next;
    }
    return bs;
}

// get the chunk associate with p
static struct chunk* get_chunk(void *p)
{
    struct chunk *ck = get_base();
    if (p == NULL)
        return NULL;
    while (ck->next < (struct chunk*)sbrk(0) && ck->data != p)
        ck = ck->next;
    return ck;
}

void* malloc(size_t size)
{
    size = word_align(size);
    struct chunk *ck = find_chunk(size);
    if (ck->next == NULL) { // extand the head if find chunk cannot find free place
        int i = extend_heap(ck, size);
        if (i == 0)
            return NULL;
    }
    ck->next->free = 0;
    ck->next->size = size;
    return ck->next->data;
}


void free(void *pt)
{
    if (pt) {
        struct chunk *ck = get_chunk(pt);
        ck->free = 1; // change it to a free chunk
    }
}


void* calloc(size_t nb, size_t size)
{
    void* pt = malloc(word_align(size));
    zerofill(pt, size * nb); // fill thr data prt to zero
    return pt;
}

void* realloc(void *p, size_t size)
{
    if (p == NULL)
        return malloc(size);
    else if (size == 0){
        free(p);
        return NULL;
    }
    struct chunk *ck = get_chunk(p);
    if (ck->size > sizeof(ck->data) + size){ // if the size is good enough, change the data ptr 
        ck->data += size; 
    }
    else {
        void* d = p;
        free(p);
        p = malloc(size);
        wordcpy(d, p, size); // copy the old data to the new one
    }
    return p;
}

/*int main()
{
    int *c = malloc(sizeof(int));
    *c = 1;
    realloc(c, sizeof(int) * 30);
    free(c);
    int *b = malloc(sizeof(int));
    *b = 2;
    free(b);
    int *a = malloc(sizeof(int));
    *a = 3;
    free(a);
    return 0;
}*/
