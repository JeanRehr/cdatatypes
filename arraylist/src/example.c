/**
 * @file example.c
 * @brief Example usage on how to use the arraylist.h
 */
#include <ctype.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// If wanted to use asserts:
// #define ARRAYLIST_USE_ASSERT 1
#include "allocator.h"
#include "arraylist.h"

/* === For arraylist of ints start === */

// The following ARRAYLIST_DEF and ARRAYLIST_DECL may be declared on the header
ARRAYLIST_DEF(int, ints)
ARRAYLIST_DECL(int, ints)

// ARRAYLIST_DEF just defined a struct that hold the type int with "ints" appended on the name
// struct arraylist_ints

// This must always be on a .c file
ARRAYLIST_IMPL(int, ints)

// Or with the all in one macro, must be in a .c file
// ARRAYLIST(int, ints)

// If wanted to strip the arraylist_ prefix:
// ARRAYLIST_STRIP_PREFIX(int, ints)
// Creates ints_init(...), etc...
// T and name must match exactly with the ARRAYLIST definitions above
// Can be used with same type and different names:
// ARRAYLIST(int, array)
// ARRAYLIST_STRIP_PREFIX(int, array)
// The same name for different types cannot be used:
// ARRAYLIST(float, ints)
// ARRAYLIST_STRIP_PREFIX(float, ints)

// All three macros must have the same type T and name (int and ints in this case)

// The struct defined may be typedefed if wanted
// typedef struct arraylist_ints arraylist_ints;

// To be used to find an int inside a container of ints
bool find_arraylist_int(int *t, void *find) {
    // Cast void pointer to intptr_t, cast to int
    if (*t == (int)(intptr_t)find) {
        return true;
    }
    return false;
}

// To test the sorting algorithm
bool comp_ascend(int *i, int *j) {
    return *i < *j;
}

// To test the sorting algorithm
bool comp_descend(int *i, int *j) {
    return *i > *j;
}

/* === For arraylist of ints end === */

/* === For arraylist of char * start === */
// Example for an arraylist of strings
ARRAYLIST_DEF(char *, vec_str)
ARRAYLIST_DECL(char *, vec_str)
ARRAYLIST_IMPL(char *, vec_str)
ARRAYLIST_STRIP_PREFIX(char *, vec_str)

// Allocates a char * from src, caller must free it
static char *heap_alloc_from_str_lit(const char *src, Allocator *alloc) {
    size_t len = strlen(src);
    char *dup = alloc->malloc(len+1, alloc->ctx);
    if (dup) memcpy(dup, src, len+1);
    return dup;
}

void vec_str_destructor(char **p, Allocator *alloc) {
    if (p && *p) {
        alloc->free(*p, sizeof(**p), alloc->ctx);
        *p = NULL;
    }
}

bool sort_strings(char **a, char **b) {
    return strcmp(*a, *b) < 0;
}

bool find_abs_string(char **a, void *find) {
    if (strcmp(*a, (char *) find) == 0) {
        return true;
    }
    return false;
}

bool find_partial_string(char **a, void *find) {
    if (strstr(*a, (char *) find) != NULL) {
        return true;
    }
    return false;
}

// Reads a line from a stream into a dynamically allocated string (without \n)
// Will read until EOF
// Returns malloced char* on success which the caller must free, or null on eof or error
char *read_line(FILE *stream, Allocator const * const alloc) {
    size_t bufsize = 2;
    size_t len = 0;
    char *buf = alloc->malloc(bufsize, alloc->ctx);

    if (!buf) {return NULL;}

    int c;
    while ((c = fgetc(stream)) != EOF) {
        if (c == '\n') {
            break;
        }

        if (len + 1 >= bufsize) { // +1 for the \0
            size_t newsize = bufsize * 2;
            if (newsize < bufsize) {
                // size_t overflow
                alloc->free(buf, bufsize, alloc->ctx);
                return NULL;
            }
            char *tmp = alloc->realloc(buf, bufsize, newsize, alloc->ctx);
            if (!tmp) {
                alloc->free(buf, bufsize, alloc->ctx);
                return NULL;
            }
            buf = tmp;
            bufsize = newsize;
        }
        buf[len++] = (char) c;
    }

    // if nothing is read (user pressed enter only)
    if (len == 0 && c == EOF) {
        alloc->free(buf, bufsize, alloc->ctx);
        return NULL;
    }

    buf[len] = '\0';
    // shrink to fit
    char *result = alloc->realloc(buf, bufsize, len+1, alloc->ctx);
    return result ? result : buf;
}

// This will read the lines from stream and insert into the vec_str
// Returns number of lines inserted
size_t vec_str_read_lines(struct arraylist_vec_str *vec_str, FILE *stream, Allocator *alloc) {
    size_t num_lines = 0;
    char *line;
    printf("Enter the strings (press CTRL+D to stop)> ");
    while ((line = read_line(stream, alloc)) != NULL) {
        // To check if it is whitespace only.
        char *p = line;
        while (*p && isspace((unsigned char)*p)) { p++; }
        // If the last whitespace is a null char, skip it and don't insert
        if (*p == '\0') {
            // not insert the empty line, free it and continue
            alloc->free(line, strlen(line) + 1, alloc->ctx);
            printf("Enter the strings> ");
            continue;
        }
        printf("Enter the strings> ");
        // emplace_back or push_back could only fail during reallocation of internal buffer
        // ownership of line allocated by read_line is passed onto the arraylist
        *vec_str_emplace_back_slot(vec_str) = line;
        num_lines++;
    }
    printf("\n");
    return num_lines;
}

// Helper function to print contents of the vec_str
void vec_str_print(struct arraylist_vec_str const * const vec_str) {
    for (size_t i = 0; i < vec_str->size; ++i) {
        printf("String number %zu: %s\n", i, vec_str->data[i]);
    }
}

/* === For arraylist of char * end == */

/* === For arraylist of pointers to a non pod type start == */

struct non_pod {
    int *_number;
    int *add;
    int *sub;
};

ARRAYLIST(struct non_pod*, np)

// Returns a stack allocated struct itself, with allocated members on the heap
struct non_pod non_pod_init(Allocator *alloc, const int n, const int add, const int sub) {
    struct non_pod np = {0};
    np._number = alloc->malloc(sizeof(*np._number), alloc->ctx);
    *np._number = n;

    np.add = alloc->malloc(sizeof(*np.add), alloc->ctx);
    *np.add = add;

    np.sub = alloc->malloc(sizeof(*np.sub), alloc->ctx);
    *np.sub = sub;

    return np;
}

// Returns a heap allocated struct, with heap allocated members
struct non_pod *non_pod_init_alloc(Allocator *alloc, const int n, const int add, const int sub) {
    struct non_pod *np = alloc->malloc(sizeof(struct non_pod), alloc->ctx);
    np->_number = malloc(sizeof(*np->_number));
    *np->_number = n;

    np->add = malloc(sizeof(*np->add));
    *np->add = add;

    np->sub = malloc(sizeof(*np->sub));
    *np->sub = sub;

    return np;
}

// Frees members of the struct
void non_pod_deinit(struct non_pod *self, Allocator *alloc) {
    if (!self)
        return;

    alloc->free(self->_number, sizeof(self->_number), alloc->ctx);
    alloc->free(self->add, sizeof(self->add), alloc->ctx);
    alloc->free(self->sub, sizeof(self->sub), alloc->ctx);
}

// Frees the heap allocated struct itself and the members of the struct
void non_pod_deinit_ptr(struct non_pod **self, Allocator *alloc) {
    if (!self || !*self)
        return;

    non_pod_deinit(*self, alloc);

    alloc->free(*self, sizeof(*self), alloc->ctx);

    *self = NULL;
}

// Example for sorting
bool non_pod_sort(const struct non_pod * const np1, const struct non_pod * const np2) {
    return np1->_number < np2->_number;
}

// Example for finding
bool non_pod_find(const struct non_pod * const self, void *find) {
    if (*self->_number == (int)(intptr_t) find) {
        return true;
    }
    return false;
}

/* === For arraylist of pointers to a non pod type end == */

int main(void) {
    Allocator gpa = allocator_get_default();

    /* == Example with a simple type like int == */
{
    // Creates a zero initialized arraylist that holds ints named int_vec with default allocator and no destructor
    struct arraylist_ints int_vec = arraylist_ints_init(NULL, NULL);
    // May be created with capacity if wanted, but can't be tested for failure
    // if reserving fails due to buffer overflow or allocation failure, it will zero initialize
    // struct arraylist_ints int_vec = arraylist_ints_init_with_capacity(NULL, NULL, 10);

    // Alternatively capacity may be reserved to minimize allocations
    arraylist_ints_reserve(&int_vec, 16);

    // This way, reserve can be tested for allocation failure or buffer overflow if needed
    enum arraylist_error ret = arraylist_ints_reserve(&int_vec, SIZE_MAX - 1); // buffer overflow
    if (ret != ARRAYLIST_OK) {
        printf("buffer overflow, capacity not reserved, ret value: %d\n", ret);
        printf("capacity is still %lu\n", int_vec.capacity);
    }

    /* == INSERTING VALUES == */
    arraylist_ints_push_back(&int_vec, 1);
    arraylist_ints_push_back(&int_vec, 2);
    arraylist_ints_push_back(&int_vec, 3);

    // Prefer using emplace_back_slot to construct the object directly into the arraylist
    // It returns a pointer slot to the end of it, must be dereferenced to assign a value
    *arraylist_ints_emplace_back_slot(&int_vec) = 10;
    *arraylist_ints_emplace_back_slot(&int_vec) = 20;

    // Insert at an index, this will insert 1 to 10 from index 0 to 9
    for (size_t i = 0; i < 10; ++i) {
        arraylist_ints_insert_at(&int_vec, i + 1, i);
    }

    *arraylist_ints_emplace_back_slot(&int_vec) = 20;

    // may use an additional variable for testing before usage
    int *slot = arraylist_ints_emplace_back_slot(&int_vec);
    assert(slot != NULL);

    *slot = 30;

    /* == READING/ACCESSING/ITERATING VALUES == */
    for (size_t i = 0; i < arraylist_ints_size(&int_vec); i++) {
        printf("index %lu value %d\n", i, int_vec.data[i]);
    }

    printf("\n");

    // May access arraylist fields directly
    for (size_t i = 0; i < int_vec.size; i++) {
        printf("index %lu value %d\n", i, int_vec.data[i]);
    }
    
    printf("\n");

    // at() function returns a pointer, so to get the value int, it must be dereferenced
    printf("int_vec value at first position = %d\n", *arraylist_ints_at(&int_vec, 0));
    printf("int_vec value at last position = %d\n", *arraylist_ints_at(&int_vec, int_vec.size - 1));

    // May access the data directly if wanted
    printf("int_vec value at position 2 = %d\n", int_vec.data[2]);
    
    printf("\n");

    // May also use iterators and pointer arithmetic to read values
    for (int *it = arraylist_ints_begin(&int_vec); it < arraylist_ints_end(&int_vec); ++it) {
        printf("value %d\n", *it);
    }

    printf("\n");

    for (size_t i = 0; i < int_vec.size; i++) {
        printf("index %lu value %d\n", i, int_vec.data[i]);
    }

    /* == REMOVING VALUES == */

    // Removes the last element
    arraylist_ints_pop_back(&int_vec);

    // If you want to get the last element before removing:
    int *last_elem = arraylist_ints_back(&int_vec);
    printf("last elem before removing: %d\n", *last_elem);
    arraylist_ints_pop_back(&int_vec);

    // Removes a single value at an index
    arraylist_ints_remove_at(&int_vec, 0);

    // Will Remove values from position 1 to last index, only index 0 will remain
    arraylist_ints_remove_from_to(&int_vec, 1, int_vec.size - 1);
    printf("arraylist size after removing from 1 to (size - 1): %lu\n", int_vec.size);
    printf("last value remaining: %d\n", *arraylist_ints_begin(&int_vec));

    /* == OTHER FUNCTIONS == */
    
    // shrink to fit will reallocate capacity to fit the size, freeing up memory and not removing elements
    printf("Capacity reserved before shrink_to_fit(): %lu\n", arraylist_ints_capacity(&int_vec));
    arraylist_ints_shrink_to_fit(&int_vec);
    printf("Capacity reserved after shrink_to_fit(): %lu\n", int_vec.capacity);

    // Inserting elements again
    for (size_t i = 0; i < 10; ++i) {
        arraylist_ints_insert_at(&int_vec, 0, i);
    }
    // Capacity and size after reinserting some elements:
    printf("Capacity again after reinstering elements: %lu\n", arraylist_ints_capacity(&int_vec));
    printf("Size again after reinstering elements: %lu\n", arraylist_ints_size(&int_vec));

    // shrink_size will shrink size to fit the size passed, removing elements, does not reallocate
    arraylist_ints_shrink_size(&int_vec, 5);
    printf("Capacity after shrink_size(): %lu\n", arraylist_ints_capacity(&int_vec));
    printf("Size after after shrink_size(): %lu\n", arraylist_ints_size(&int_vec));

    // Tests if the arraylist is empty
    if (arraylist_ints_is_empty(&int_vec) == true) {
        printf("is empty\n");
    } else {
        printf("is NOT empty\n");
    }

    // Clear the arraylist of all elements, does not reallocate, only remove elements
    arraylist_ints_clear(&int_vec);
    if (arraylist_ints_is_empty(&int_vec) == true) {
        printf("is empty\n");
    } else {
        printf("is NOT empty\n");
    }

    // Gets the allocator if needed, it will be the default global allocator in this case
    arraylist_ints_get_allocator(&int_vec);

    // Swaps arraylists
    
    struct arraylist_ints other = arraylist_ints_init(NULL, NULL);
    *arraylist_ints_emplace_back_slot(&other) = 1000;
    *arraylist_ints_emplace_back_slot(&other) = 2000;

    // Other size before swap:
    printf("Other arraylist size before swap(): %lu\n", arraylist_ints_size(&other));
    printf("Original arraylist size before swap(): %lu\n", arraylist_ints_size(&int_vec));

    arraylist_ints_swap(&int_vec, &other);

    printf("Other arraylist size after swap(): %lu\n", arraylist_ints_size(&other));
    printf("Original arraylist size after swap(): %lu\n", arraylist_ints_size(&int_vec));

    arraylist_ints_deinit(&other);
    /* == FIND AND CONTAINS == */
    
    // Find returns the reference if found, end if not
    // A function must be provided so that it knows how and what to find
    int *not_foundref = arraylist_ints_find(&int_vec, find_arraylist_int, (void*)10);

    // If value is not found, it will return the end of the arraylist, dereferencing it is UB
    if (not_foundref == arraylist_ints_end(&int_vec)) {
        printf("not found\n");
    }

    int *foundref = arraylist_ints_find(&int_vec, find_arraylist_int, (void*)1000);
    if (foundref != arraylist_ints_end(&int_vec)) {
        printf("found, value is: %d\n", *foundref);
    }

    // Contains returns false or true, and an optional index where it was found
    size_t index = 0;
    bool found = arraylist_ints_contains(&int_vec, find_arraylist_int, (void*)2000, &index);

    if (found) {
        printf("value found at index %lu\n", index);
    }

    /* == SORT FUNCTION == */
    arraylist_ints_clear(&int_vec);
    arraylist_ints_reserve(&int_vec, 5);

    *arraylist_ints_emplace_back_slot(&int_vec) = 3;
    *arraylist_ints_emplace_back_slot(&int_vec) = 5;
    *arraylist_ints_emplace_back_slot(&int_vec) = 1;
    *arraylist_ints_emplace_back_slot(&int_vec) = -2;
    *arraylist_ints_emplace_back_slot(&int_vec) = 6;

    for (size_t i = 0; i < int_vec.size; i++) {
        printf("index %lu value %d\n", i, int_vec.data[i]);
    }

    arraylist_ints_qsort(&int_vec, comp_ascend);

    printf("\n");

    for (size_t i = 0; i < int_vec.size; i++) {
        printf("index %lu value %d\n", i, int_vec.data[i]);
    }

    arraylist_ints_qsort(&int_vec, comp_descend);

    printf("\n");

    for (size_t i = 0; i < int_vec.size; i++) {
        printf("index %lu value %d\n", i, int_vec.data[i]);
    }

    // Must be called when done
    arraylist_ints_deinit(&int_vec);
}

    /* == Example with a vector of strings (char *, not a "type" string) == */
{
    struct arraylist_vec_str names = vec_str_init(&gpa, vec_str_destructor);

    // Uncomment the following line to read from terminal
    // vec_str_read_lines(&names, stdin, &gpa);

    // Inseting inside the arraylist of strings:

    // The following cannot be done, it needs to be allocated on heap
    // *vec_str_emplace_back_slot(&names) = "Testing";

    // manually constructing and allocating a string
    size_t len;
    char *str;
    {
        static const char *s = "TESTING";
        len = strlen(s);
        str = gpa.malloc(sizeof(len + 1), gpa.ctx);
        memcpy(str, s, len + 1);
    }

    // Once str is passed onto the arraylist, if it is provided a destructor, there is no need to worry about freeing the str
    *vec_str_emplace_back_slot(&names) = str;

    // Using a constructor
    *vec_str_emplace_back_slot(&names) = heap_alloc_from_str_lit("Full Name", &gpa);

    printf("UNSORTED:\n");
    vec_str_print(&names);

    vec_str_qsort(&names, sort_strings);
    
    printf("\n");

    printf("SORTED:\n");
    vec_str_print(&names);

    if (vec_str_contains(&names, find_abs_string, "ABCSD", 0)) {
        printf("NAME <ABCSD> FOUND!!!!!!\n");
    } else {
        printf("NOT FOUND!!!!!\n");
    }

    if (vec_str_contains(&names, find_partial_string, "TEST", 0)) {
        printf("PARTIALLY FOUND <TEST>\n");
    } else {
        printf("NOT FOUND!!!!!\n");
    }

    vec_str_deinit(&names);
}

    /* == Example with a vector for a pointer to non pod type == */
{
    struct arraylist_np vec_np = arraylist_np_init(&gpa, non_pod_deinit_ptr);
    arraylist_np_reserve(&vec_np, 40);

    // Inserting into it with emplace back slot one liner
    // Note, slot given back must always be checked in real scenarios
    struct non_pod **slot1 = arraylist_np_emplace_back_slot(&vec_np);
    if (!slot1) {
        fprintf(stderr, "Failed to get the slot\n");
        return -1;
    }
    // A constructor that returns an allocated and initialized struct acts like new
    *slot1 = non_pod_init_alloc(&gpa, 90, 80, 70);
    if (!*slot1) {
        fprintf(stderr, "Allocation failure\n");
        return -1;
    }

    // One liner not testing slot or allocation
    *arraylist_np_emplace_back_slot(&vec_np) = non_pod_init_alloc(&gpa, 940, 820, 710);

    // Inserting into it with emplace back slot with a constructor that does not
    // return an allocated struct itself
    struct non_pod *non_pod1 = gpa.malloc(sizeof(struct non_pod), NULL);
    // Can be tested for allocation failure
    if (!non_pod1) {
        fprintf(stderr, "allocation failure\n");
        return -1;
    }
    
    *non_pod1 = non_pod_init(&gpa, 999, 987, 781);

    // Warning: Not checking slot here
    *arraylist_np_emplace_back_slot(&vec_np) = non_pod1;

    // Inserting into it with push_back one liner, not checking for allocation failure here
    arraylist_np_push_back(&vec_np, non_pod_init_alloc(&gpa, 464, 422, 180));

    // Inserting into it with push_back constructing first and then inserting
    // Warning: Not testing the malloc failure here
    struct non_pod *add1 = gpa.malloc(sizeof(struct non_pod), NULL);
    *add1 = non_pod_init(&gpa, 228, 421, 244);
    // Testing the result of push_back
    if (arraylist_np_push_back(&vec_np, add1) != ARRAYLIST_OK) {
        fprintf(stderr, "error occurred at line %d, file %s\n", __LINE__, __FILE__);
        return -1;
    }
    
    // Inserting into it with insert_at is basically the same as push_back

    // inserting some values
    for (size_t i = 0; i < vec_np.capacity; ++i) {
        *arraylist_np_emplace_back_slot(&vec_np) = non_pod_init_alloc(&gpa, i, i * 3, i / 2);
    }

    // Rest of the functions are essentially the same thing

    arraylist_np_deinit(&vec_np);

}
    return 0;
}
