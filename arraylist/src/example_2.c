/**
 * @file example2.c
 * @brief Example usage on how to use the arraylist.h
 * @details This example will show the function pointer version usage for a pointer type like char *
 */

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "arraylist.h"

// Example for an arraylist of strings (char *)
ARRAYLIST_DEF(char *, vec_str)
ARRAYLIST_DECL(char *, vec_str)

// Destructors

void char_ptr_deinit(char **p, Allocator *alloc) {
    if (p && *p) {
        alloc->free(*p, strlen(*p) + 1, alloc->ctx);
        *p = NULL;
    }
}

#define char_ptr_deinit_macro_unsafe(char_double_ptr, allocptr) \
do { \
    if (char_double_ptr && *char_double_ptr) { \
        (allocptr)->free(*(char_double_ptr), strlen(*(char_double_ptr)) + 1, (allocptr)->ctx); \
        *(char_double_ptr) = NULL; \
    } \
} while(0)

// C23 typeof could also be used to prevent misuses of this macro, example:
#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 202311L)

    #define check_same_types(T, U) \
        { static_assert(_Generic((T), typeof(U): true, default: false), "Parameters must have the same type"); }

#else

    #define check_same_types(T, U) (void)0 // Do nothing

#endif

// now this macro, on C23, will check that char_double_ptr is indeed a pointer to char pointer
// As this is a macro just to be used inside the arraylist, there is no way I can think of that can
// be misused as the arraylist is typesafe, unless used outside of the arraylist
#define char_ptr_deinit_macro(char_double_ptr, allocptr) \
do { \
    check_same_types(char_double_ptr, char**); \
    check_same_types(struct Allocator, allocptr); \
    if (char_double_ptr && *char_double_ptr) { \
        (allocptr)->free(*(char_double_ptr), strlen(*(char_double_ptr)) + 1, (allocptr)->ctx); \
        *(char_double_ptr) = NULL; \
    } \
} while(0)

ARRAYLIST_IMPL(char *, vec_str, char_ptr_deinit_macro)

// Could also use the function destructor instead of macro:
// ARRAYLIST_IMPL(char *, vec_str, char_ptr_deinit)

ARRAYLIST_STRIP_PREFIX(char *, vec_str)

// Constructor, allocates a char * from src, caller must free it
static char *heap_alloc_from_str_lit(const char *src, Allocator *alloc) {
    size_t len = strlen(src);
    char *dup = alloc->malloc(len + 1, alloc->ctx);
    if (dup) {
        memcpy(dup, src, len + 1);
    }
    return dup;
}

bool vec_str_sort(char **a, char **b) {
    return strcmp(*a, *b) < 0;
}

bool vec_str_find_abs(char **a, void *find) {
    if (strcmp(*a, (char *)find) == 0) {
        return true;
    }
    return false;
}

bool vec_str_find_partial(char **a, void *find) {
    if (strstr(*a, (char *)find) != NULL) {
        return true;
    }
    return false;
}

// Reads a line from a stream into a dynamically allocated string (without \n)
// Will read until EOF
// Returns malloced char* on success which the caller must free, or null on eof or error
char *read_line(FILE *stream, Allocator const *const alloc) {
    size_t bufsize = 2;
    size_t len = 0;
    char *buf = alloc->malloc(bufsize, alloc->ctx);

    if (!buf) {
        return NULL;
    }

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
        buf[len++] = (char)c;
    }

    // if nothing is read (user pressed enter only)
    if (len == 0 && c == EOF) {
        alloc->free(buf, bufsize, alloc->ctx);
        return NULL;
    }

    buf[len] = '\0';
    // shrink to fit
    char *result = alloc->realloc(buf, bufsize, len + 1, alloc->ctx);
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
        while (*p && isspace((unsigned char)*p)) {
            p++;
        }
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
void vec_str_print(struct arraylist_vec_str const *const vec_str) {
    for (size_t i = 0; i < vec_str->size; ++i) {
        printf("String number %zu: %s\n", i, vec_str->data[i]);
    }
}

int main(void) {
    Allocator gpa = allocator_get_default();

    struct arraylist_vec_str names = vec_str_init(&gpa);

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
        str = gpa.malloc(len + 1, gpa.ctx);
        memcpy(str, s, len + 1);
    }

    // Once str is passed onto the arraylist, if it is provided a destructor, there is no need to worry about freeing the str
    *vec_str_emplace_back_slot(&names) = str;

    // Using a constructor
    *vec_str_emplace_back_slot(&names) = heap_alloc_from_str_lit("Full Name", &gpa);

    printf("UNSORTED:\n");
    vec_str_print(&names);

    vec_str_qsort(&names, vec_str_sort);

    printf("\n");

    printf("SORTED:\n");
    vec_str_print(&names);

    if (vec_str_contains(&names, vec_str_find_abs, "ABCSD", 0)) {
        printf("NAME <ABCSD> FOUND!!!!!!\n");
    } else {
        printf("NOT FOUND!!!!!\n");
    }

    if (vec_str_contains(&names, vec_str_find_partial, "TEST", 0)) {
        printf("PARTIALLY FOUND <TEST>\n");
    } else {
        printf("NOT FOUND!!!!!\n");
    }

    vec_str_deinit(&names);

    return 0;
}