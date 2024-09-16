#include <string.h>
#include <stdio.h>
#include <unistd.h>

void *malloc(size_t size);
void free(void *ptr);
void *calloc(size_t nmemb, size_t size);
void *realloc(void *ptr, size_t size);
void print_the_list();

typedef struct List_Node
{
    size_t size;
    struct List_Node *next;
}List_Node;

static List_Node **free_list = NULL;


#define CHUNK_SIZE (1<<12)

void list_init()
{
    void *p  = sbrk(CHUNK_SIZE);
    free_list = (List_Node **)p;
}


extern void *bulk_alloc(size_t size);


extern void bulk_free(void *ptr, size_t size);


static inline __attribute__((unused)) int block_index(size_t x) {
    if (x <= 8) {
        return 5;
    } else {
        return 32 - __builtin_clz((unsigned int)x + 7);
    }
}


void *malloc(size_t size) {
    //fprintf(stderr,"%zu Malloc beginning is working \n", size);
    if (free_list ==NULL)
        {
            list_init();
        }
    if (size > 4088)
        {
            void *ret_ptr =  bulk_alloc(size+8);
            *(size_t *)ret_ptr = size;
            ret_ptr += 8;
            return ret_ptr;
        }
    int index = block_index(size);
    List_Node *block = free_list[index];
    
    //fprintf(stderr,"%d  index returned \n", index);
    if (free_list[index] == NULL)
        {
            
            void *p = sbrk(CHUNK_SIZE);
            size_t block_size = 1 << index;
            List_Node *first_block = (List_Node *)p;
            first_block->size = block_size;
            first_block->next = NULL;
            p += block_size;
            List_Node *populate;
            for (int i = 0; i < (4096 - block_size); i = i + block_size)
                {
                    populate = (List_Node *)p;
                    populate->size = block_size;
                    populate->next = first_block;
                    p += block_size;
                    first_block = populate;
                }

            
            free_list[index] = first_block;
            List_Node *ptr = free_list[index];
            if (index != 12){
                ptr->size++;
            }
            free_list[index] = ptr->next;
            //fprintf(stderr,"size of pointer being returned: %zu\n",ptr->size);
            //fprintf(stderr,"pointer of pointer being returned: %p\n\n",ptr);
            void *ret_val = (void *)ptr;
            ret_val += 8;
            //fprintf(stderr,"pointer of pointer being returned: %p\n\n",ret_val);
            //print_the_list();
            return ret_val;
                
        }
    else
        {
            
            //fprintf(stderr,"%p\n",block->next);
            if (index != 12){
                block->size++;
            }
            
            //fprintf(stderr,"is it here?\n");
            free_list[index] = block->next;
            
            //fprintf(stderr,"size of pointer being returned: %zu\n",block->size);
            void *ret_val = (void *)block;
            
            ret_val += 8;
            
            //fprintf(stderr," pointer being returned: %p\n\n",ret_val);
            //fprintf(stderr,"\n\n     size = %zu      \n\n",block->next->size);
            //fprintf(stderr,"\n\n     pointer address = %p      \n\n",block->next);
            //print_the_list();
            
            return ret_val;
        }
    
}


void *calloc(size_t nmemb, size_t size) {
    //fprintf(stderr,"calloc started size requested: %zu\n\n",nmemb * size);
    //fprintf(stderr,"size requested: %zu\n", nmemb * size);
    //int block_size = 1 << block_index(nmemb * size);
    void *ptr = malloc(nmemb * size);
    memset(ptr,0,nmemb*size);
    return ptr;
}


void *realloc(void *ptr, size_t size) {
    //fprintf(stderr,"%zu realloc reuested memory      \n", size);
    //fprintf(stderr,"\n\n\n %p pointer address to be reallocated \n\n\n", ptr);
    if (ptr == NULL)
        {
            void *ret_val = malloc(size);
            return ret_val;
        }
    void *ptr_beginning = ptr - 8;
    //fprintf(stderr,"Hi\n");
    size_t ptr_size = *(size_t *)ptr_beginning - 1;
    if (ptr_size == 4095)
        {
            ptr_size++;
        }
    //fprintf(stderr,"%zu size of available pointer \n\n",ptr_size);
    
    if ((block_index(ptr_size) - 1) == block_index(size))
        {
            return ptr;
        }
    else if ((block_index(ptr_size) - 1) > block_index(size))
        {
            return ptr;
        }
    else
        {
            /*if (size > 4088)
                {
                    void *ret_bulk_ptr = bulk_alloc(size+8);
                    *(size_t *)ret_bulk_ptr = size;
                    ret_bulk_ptr += 8;
                    memcpy(ret_bulk_ptr,ptr,ptr_size);
                    free(ptr);
                    return ret_bulk_ptr;
                }*/
            
            void *ret_ptr = malloc(size);
            
            memcpy(ret_ptr,ptr,ptr_size-8);
            
           
            free(ptr);
            return ret_ptr;
        }
}


void free(void *ptr) {
    //fprintf(stderr," Free called pointer = %p\n\n",ptr);
    if (ptr == NULL)
        {
            return;
        }
    void *ptr_beginning = ptr - 8;
    size_t ptr_size = *(size_t *)ptr_beginning;
    //fprintf(stderr,"%zu free number\n",ptr_size);
    if (ptr_size > 4096)
        {
            bulk_free(ptr_beginning,ptr_size+8);
            return;
        }
    ptr_size--;
    int index = block_index(ptr_size) - 1;
    List_Node *ptr_list_type = (List_Node *)ptr_beginning;
    ptr_list_type->size = ptr_size;
    ptr_list_type->next = free_list[index];
    free_list[index] = ptr_list_type;
    //fprintf(stderr," pointer to head pointer = %p\n",free_list[index]);    
    //fprintf(stderr," pointer to freed pointer = %p\n",free_list[index]);
}

void print_the_list(void *ret_val)
{
    void * p = ret_val;
    size_t size = *(size_t *)p;
    fprintf(stderr,"\n\nsize of block is: %zu\n\n", size);
}
