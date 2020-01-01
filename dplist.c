#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "dplist.h"

/*
 * definition of error codes
 * */
#define DPLIST_NO_ERROR 0
#define DPLIST_MEMORY_ERROR 1  // error due to mem alloc failure
#define DPLIST_INVALID_ERROR 2 //error due to a list operation applied on a NULL list

#ifdef DEBUG
#define DEBUG_PRINTF(...)                                                                    \
    do                                                                                       \
    {                                                                                        \
        fprintf(stderr, "\nIn %s - function %s at line %d: ", __FILE__, __func__, __LINE__); \
        fprintf(stderr, __VA_ARGS__);                                                        \
        fflush(stderr);                                                                      \
    } while (0)
#else
#define DEBUG_PRINTF(...) (void)0
#endif

#define DPLIST_ERR_HANDLER(condition, err_code)   \
    do                                            \
    {                                             \
        if ((condition))                          \
            DEBUG_PRINTF(#condition " failed\n"); \
        assert(!(condition));                     \
    } while (0)

/*
 * The real definition of struct list / struct node
 */

struct dplist_node
{
    dplist_node_t *prev, *next;
    void *element;
};

struct dplist
{
    dplist_node_t *head;
    void *(*element_copy)(void *src_element);
    void (*element_free)(void **element);
    int (*element_compare)(void *x, void *y);
};

dplist_t *dpl_create( // callback functions
    void *(*element_copy)(void *src_element),
    void (*element_free)(void **element),
    int (*element_compare)(void *x, void *y))
{
    dplist_t *list;
    list = malloc(sizeof(struct dplist));
    DPLIST_ERR_HANDLER(list == NULL, DPLIST_MEMORY_ERROR);
    list->head = NULL;
    list->element_copy = element_copy;
    list->element_free = element_free;
    list->element_compare = element_compare;
    return list;
}

void dpl_free(dplist_t **list, bool free_element)
{
    DPLIST_ERR_HANDLER(list == NULL, DPLIST_INVALID_ERROR);
    dplist_t *list2 = *list;
    if (*list == NULL)
        return;
    assert(*list != NULL);
    if (list2->head == NULL)
    {
        free(*list);
        *list = NULL;
        return;
    }

    dplist_node_t *tmp;
    int length = dpl_size(list2);
    for (int i = length - 1; i >= 0; i--)
    {
        tmp = dpl_get_reference_at_index(list2, i);
        if (free_element)
            (*list2->element_free)(&tmp->element);
        free(tmp);
    }
    free(*list);
    *list = NULL;
}

dplist_t *dpl_insert_at_index(dplist_t *list, void *element, int index, bool insert_copy)
{
    DPLIST_ERR_HANDLER(list == NULL, DPLIST_INVALID_ERROR);
    dplist_node_t *newNode, *ref_at_index;

    if (element == NULL)
        return list;
    newNode = malloc(sizeof(dplist_node_t));
    DPLIST_ERR_HANDLER(newNode == NULL, DPLIST_MEMORY_ERROR);
    if (insert_copy)
        newNode->element = (list->element_copy)(element);
    else
    {
        newNode->element = element;
    }

    if (list -> head == NULL)
    {
        newNode->prev = NULL;
        newNode->next = NULL;
        list->head = newNode;
    }
    else if (index <= 0)
    {
        newNode->prev = NULL;
        newNode->next = list->head;
        list->head->prev = newNode;
        list->head = newNode;
    }
    else
    {
        ref_at_index = dpl_get_reference_at_index(list, index);
        assert(ref_at_index != NULL);
        if (index < dpl_size(list))
        {
            newNode->prev = ref_at_index->prev;
            newNode->next = ref_at_index;
            ref_at_index->prev->next = newNode;
            ref_at_index->prev = newNode;
        }
        else
        { // when index is bigger than the number of elements in the list
            assert(ref_at_index->next == NULL);
            newNode->next = NULL;
            newNode->prev = ref_at_index;
            ref_at_index->next = newNode;
        }
    }
    return list;
}

dplist_t *dpl_remove_at_index(dplist_t *list, int index, bool free_element)
{
    DPLIST_ERR_HANDLER(list == NULL, DPLIST_INVALID_ERROR);
    dplist_node_t *dummy = list->head;

    if (dummy == NULL)
        return list;
    dummy = dpl_get_reference_at_index(list, index);
    if (dummy->prev == NULL)
    {
        if (dummy->next != NULL)
        {
            list->head = dummy->next;
            dummy->next->prev = NULL;
        }
        else
        {
            list->head = NULL;
        }  
    }

    else if (dummy->next == NULL)
    {
        dummy->prev->next = NULL;      
    }
    else
    {
        dummy->prev->next = dummy->next;
        dummy->next->prev = dummy->prev;
    }
    if (free_element)
            (*list->element_free)(&dummy->element);
    free(dummy);
    return list;
}

int dpl_size(dplist_t *list)
{
    DPLIST_ERR_HANDLER(list == NULL, DPLIST_INVALID_ERROR);
    dplist_node_t *tmp = list->head;
    if (tmp == NULL)
        return 0;
    int size = 0;
    while (tmp != NULL)
    {
        tmp = tmp->next;
        size++;
    }
    return size;
}

void *dpl_get_element_at_index(dplist_t *list, int index)
{
    DPLIST_ERR_HANDLER(list == NULL, DPLIST_INVALID_ERROR);
    dplist_node_t *tmp = list->head;
    if (list->head == NULL)
        return NULL;
    if (index <= 0)
        return list->head->element;
    while (tmp -> next != NULL && index > 0)
    {
        tmp = tmp->next;
        index--;
    }
    return tmp->element;
}

int dpl_get_index_of_element(dplist_t *list, void *element)
{
    DPLIST_ERR_HANDLER(list == NULL, DPLIST_INVALID_ERROR);
    dplist_node_t *tmp = list->head;
    int index = 0;
    while (tmp != NULL)
    {
        if (!((*list->element_compare)(element, tmp->element)))
            return index;
        else
        {
            index++;
            tmp = tmp->next;
        }
    }
    return -1;
}

dplist_node_t *dpl_get_reference_at_index(dplist_t *list, int index)
{
    DPLIST_ERR_HANDLER(list == NULL, DPLIST_INVALID_ERROR);
    dplist_node_t *tmp = list->head;
    if (list->head == NULL)
        return NULL;
    if (index <= 0)
        return list->head;
    while (tmp->next != NULL && index > 0)
    {
        tmp = tmp->next;
        index--;
    }
    return tmp;
}

void *dpl_get_element_at_reference(dplist_t *list, dplist_node_t *reference)
{
    DPLIST_ERR_HANDLER(list == NULL, DPLIST_INVALID_ERROR);
    dplist_node_t *tmp = list->head;
    if (list->head == NULL)
        return NULL;
    if (reference == NULL)
        return dpl_get_element_at_index(list, dpl_size(list) - 1);
    while (tmp != NULL)
    {
        if (reference == tmp)
            return tmp->element;
        else
        {
            tmp = tmp->next;
        }
    }
    return NULL;
}

dplist_node_t * dpl_get_reference_of_element( dplist_t * list, void * element ) {
    DPLIST_ERR_HANDLER(list == NULL, DPLIST_INVALID_ERROR);
    dplist_node_t *tmp = list->head;
    
    while (tmp != NULL)
    {
        if (!((*list->element_compare)(element, tmp->element)))
            return tmp;
        else
        {        
            tmp = tmp->next;
        }
    }
    return NULL;
}
