#include <unistd.h>

#define _BSD_SOURCE 
#define _SVID_SOURCE
#define _XOPEN_SOURCE 500

struct chunk {
  struct chunk *next, *prev;
  size_t        size;
  long          free;
  void         *data;
};


static inline size_t word_align(size_t n);
void zerofill(void *ptr, size_t len);
void wordcpy(void *dst, void *src, size_t len);
static int extend_heap(struct chunk *last, size_t size);
static struct chunk* find_chunk(size_t size);
static struct chunk* get_chunk(void *p);
void* malloc(size_t size);
void free(void *pt);
void* calloc(size_t size);
void* realloc(void *p, size_t size);

int main()
{
}

// Function to align with the size of size_t
static inline size_t word_align(size_t n)
{
    size_t s = sizeof(size_t);
    if (s >= n) {
        if (s == n)
            return s;
        else {
            size_t b = n^(s - 1); // need find other solution for s - 1 (Bit-wise complement)
            if (b == 0)
                return s;
            s = s - b;
            return s - (n^s);
        }
    } else { // Case when n > sizeof(size_t)
        size_t c = s;
        while ( s < n ) {
            s += c;
        }
        return s;
    }
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
      base = sbrk(sizeof(struct chunk));
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
    last->next = sbrk(sizeof(struct chunk) + size);
    if (last->next == NULL)
        return 0;
    last->next->size = 0;
    last->next->next = NULL;
    last->next->free = 0;
    last->next->data = sbrk(0) - size; // get the data ptr on getting the actual data limit and the size
    
    return 1;  
}

// research a free chunk
static struct chunk* find_chunk(size_t size)
{
    struct chunk *bs = get_base();
    while (bs->next != NULL) {
       if (bs->next->size == size && bs->next->free)
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
    while (ck->next != NULL && &(ck->data) != &p)
        ck = ck->next;
    return ck;
}

void* malloc(size_t size)
{
    struct chunk *ck = find_chunk(0);
    size = word_align(size);
    if (ck->next == NULL) { // extand the head if find chunk cannot find free place
        int i = extend_heap(ck, size);
        if (i == 0)
            return NULL;
    }
    ck->next->free = 0;
    ck->next->size = size;
    return ck->next->data;
}


void pfree(void *pt)
{
   struct chunk *ck = get_chunk(pt);
   ck->free = 1; // change it to a free chunk
}


void* calloc(size_t size)
{
    void* pt = malloc(word_align(size));
    zerofill(pt, size); // fill thr data prt to zero
    return pt;
}

void* realloc(void *p, size_t size)
{
    struct chunk *ck = get_chunk(p);
    if (ck->size > sizeof(ck->data) + size){ // if the size is good enough, change the data ptr 
        ck->data += size; 
    }
    else {
        void* d = p;
        pfree(p);
        p = malloc(size);
        wordcpy(d, p, size); // copy the old data to the new one
    }
    return p;
}
