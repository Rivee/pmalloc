#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

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
void* pmalloc(size_t size);
void pfree(void *pt);
void* pcalloc(size_t size);
void* prealloc(void *p, size_t size);

int main()
{
    char* val = pmalloc(sizeof(char));
    char str[] = "salut ca va oui pas bien et apres je sais pas lol";
    strcpy(val, str);
    printf("%s", val);
    return 0;
}

static inline size_t word_align(size_t n)
{
        
}

void zerofill(void *ptr, size_t len)
{
    for (int i = 0; i < len; i++)
    {
        ptr = 0;
        ptr++;
    }
}

void wordcpy(void *dst, void *src, size_t len)
{
    for(int i = 0; i < len; i++)
    {
        dst = src;
        src++;
    }
}

static struct chunk* get_base(void)
{
  static struct chunk *base = NULL;
  if (base == NULL) 
  {
      base = sbrk(sizeof(struct chunk));
      base->next = NULL;
      base->prev = NULL;
      base->size = 0;
      base->free = 0;
      base->data = NULL;
  }
  return base;
}

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
    last->next->data = sbrk(0) - size;
    
    return 1;  
}

static struct chunk* find_chunk(size_t size)
{
    struct chunk *bs = get_base();
    while (bs->next != NULL)
    {
       if (bs->next->size == size && bs->next->free)
           return bs;
        bs = bs->next;
    }
    return bs;
}

static struct chunk* get_chunk(void *p)
{
    struct chunk *ck = get_base();
    if (p == NULL)
        return NULL;
    while (ck->next != NULL && &(ck->data) != &p)
        ck = ck->next;
    return ck;
}

void* pmalloc(size_t size)
{
    struct chunk *ck = find_chunk(0);
    if (ck->next == NULL)
    {
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
   ck->free = 1;
}


void* pcalloc(size_t size)
{
    void* pt = pmalloc(size);
    zerofill(pt, size);
    return pt;
}

void* prealloc(void *p, size_t size)
{
    struct chunk *ck = get_chunk(p);
    if (ck->size > sizeof(ck->data) + size)
    {
        ck->data += size; 
    }
    else
    {
        void* d = p;
        pfree(p);
        p = pmalloc(size);
        wordcpy(d, p, size);
    }
    return p;
}
