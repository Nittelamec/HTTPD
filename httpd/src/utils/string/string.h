#ifndef STRING_H
#define STRING_H

#include <ctype.h>
#include <stddef.h>
#include <stdlib.h>

struct string
{
    size_t size;
    char *data;
};

/*
 ** @brief Create new string struct from char * and size
 **        Be careful, the argument str will not be deallocated and thus you
 **        can call this function with a static char *
 **
 ** @param str
 ** @param size
 **
 ** @return the newly allocated string
 */
struct string *string_create(const char *str, size_t size);

struct string *create_string_unknown(char *str, const char *pattern,
                                     size_t plen);

int string_compare_n_str(const struct string *str1, const char *str2, size_t n);

void string_concat_str(struct string *str, const char *to_concat, size_t size);

struct string *string_chr(struct string *str, char c);

struct string *string_tok_pattern(struct string *str, struct string *pattern,
                                  struct string **saveptr);

struct string *string_tok(struct string *str, struct string *delim,
                          struct string **saveptr);

struct string *string_str(struct string *haystack, struct string *needle);

void string_destroy(struct string *str);

char *string_dup(struct string *str);

char *my_strdup(const char *str);

#endif /* ! STRING_H */
