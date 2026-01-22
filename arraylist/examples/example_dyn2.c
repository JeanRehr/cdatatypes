/**
 * @file example_fp2.c
 * @brief Example usage on how to use the arraylist.h
 * @details This example will show the function pointer version usage for a pointer type like char *
 */

#include <ctype.h>
#include <stdio.h>
#include <string.h>

// Usually names_arr may collide with your userbase, as it is quite common name
// of course just for showing/testing purposes
#define ARRAYLIST_DYN_USE_PREFIX
#include "arraylist.h"

// Example for an arraylist of strings (char *)
ARRAYLIST_DEF_DYN(char *, names_arr)
ARRAYLIST_DECL_DYN(char *, names_arr)
ARRAYLIST_IMPL_DYN(char *, names_arr)

// Allocates a char * from src, caller must free it
static char *heap_alloc_from_str_lit(const char *src, struct Allocator *alloc) {
    size_t len = strlen(src);
    char *dup = alloc->malloc(len + 1, alloc->ctx);
    if (dup) {
        memcpy(dup, src, len + 1);
    }
    return dup;
}

void char_ptr_deinit(char **p, struct Allocator *alloc) {
    if (p && *p) {
        alloc->free(*p, strlen(*p) + 1, alloc->ctx);
        *p = NULL;
    }
}

bool names_sort(char **a, char **b) {
    return strcmp(*a, *b) < 0;
}

bool names_find_abs(char **a, void *find) {
    if (strcmp(*a, (char *)find) == 0) {
        return true;
    }
    return false;
}

bool names_find_partial(char **a, void *find) {
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

// This will read the lines from stream and insert into the names
// Returns number of lines inserted
size_t names_read_lines(struct arraylist_dyn_names_arr *names_arr, FILE *stream, struct Allocator *alloc) {
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
        *arraylist_dyn_names_arr_emplace_back_slot(names_arr) = line;
        num_lines++;
    }
    printf("\n");
    return num_lines;
}

// Helper function to print contents of the names
void names_print(struct arraylist_dyn_names_arr const *const names) {
    for (size_t i = 0; i < names->size; ++i) {
        printf("String number %zu: %s\n", i, names->data[i]);
    }
}

int main(void) {
    struct Allocator gpa = allocator_get_default();

    struct arraylist_dyn_names_arr names_arr = arraylist_dyn_names_arr_init(&gpa, char_ptr_deinit);

    // Uncomment the following line to read from terminal
    // names_read_lines(&names, stdin, &gpa);

    // Inseting inside the arraylist of strings:

    // The following cannot be done, it needs to be allocated on heap
    // *arraylist_dyn_names_arr_emplace_back_slot(&names) = "Testing";

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
    *arraylist_dyn_names_arr_emplace_back_slot(&names_arr) = str;

    // Using a constructor
    *arraylist_dyn_names_arr_emplace_back_slot(&names_arr) = heap_alloc_from_str_lit("Full Name", &gpa);

    printf("UNSORTED:\n");
    names_print(&names_arr);

    arraylist_dyn_names_arr_qsort(&names_arr, names_sort);

    printf("\n");

    printf("SORTED:\n");
    names_print(&names_arr);

    if (arraylist_dyn_names_arr_contains(&names_arr, names_find_abs, "ABCSD", 0)) {
        printf("NAME <ABCSD> FOUND!!!!!!\n");
    } else {
        printf("NOT FOUND!!!!!\n");
    }

    if (arraylist_dyn_names_arr_contains(&names_arr, names_find_partial, "TEST", 0)) {
        printf("PARTIALLY FOUND <TEST>\n");
    } else {
        printf("NOT FOUND!!!!!\n");
    }

    arraylist_dyn_names_arr_deinit(&names_arr);

    return 0;
}