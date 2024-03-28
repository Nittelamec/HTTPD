#include "string.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "../variables/variables.h"

/*
 * Create a new struct string
 */
struct string *string_create(const char *str, size_t size)
{
    if (!size)
        return NULL;

    struct string *sstr = malloc(sizeof(struct string));
    if (!sstr)
        return NULL;

    char *data = malloc(size + 1);
    if (!data)
    {
        free(sstr);
        return NULL;
    }

    size_t pos_data = 0;
    size_t pos_str = 0;
    while (pos_str < size)
    {
        if (str[pos_str] == '\0')
            pos_str++;
        else
        {
            data[pos_data] = str[pos_str];
            pos_data++;
            pos_str++;
        }
    }
    data[pos_data] = '\0';

    sstr->size = pos_data;
    sstr->data = (pos_data == size) ? data : realloc(data, pos_data);

    return sstr;
}

/*
 * My own version of memmem
 *
 * @param haystack: a pointer to a string or whatever
 * @param hlen: the size in bytes of the memory we try to read (arbitrary)
 * @param needle: the pattern we are looking for
 */
static void *my_memmem(void *haystack, size_t hlen, struct string *needle)
{
    size_t i = 0;
    char *h = haystack;
    ssize_t j;
    while (i < hlen)
    {
        j = needle->size - 1;
        while (j >= 0 && *(h + i + j) == needle->data[j])
            j--;
        if (j < 0)
            return h + i + needle->size - 1;
        else
        {
            ssize_t k = needle->size - 1;
            while (k >= 0 && *(h + i + needle->size - 1) != needle->data[k])
                k--;
            if (k < 0) // the letter does not belong to needle
                i += needle->size;
            else
                i += k - 1; // letter belongs, k = distance from the last letter
        }
    }
    return NULL;
}

/*
 * Allows us to create a struct string without knowing its size, but the
 * string must contain the "pattern" at least once
 *
 * @param str: the string to convert
 * @param pattern: the mandatory pattern
 * @param plen: the pattern length
 */
struct string *create_string_unknown(char *str, const char *pattern,
                                     size_t plen)
{
    struct string *needle = string_create(pattern, plen);

    void *vstr = str;
    char *end_of_str = my_memmem(vstr, BUFFERSIZE, needle);

    size_t stlen = 0;
    char *tmp = str;
    while (tmp != end_of_str)
    {
        stlen++;
        tmp++;
    }

    stlen++;
    string_destroy(needle);

    return string_create(str, stlen);
}

/*
 * adapted version of strncmp()
 */
int string_compare_n_str(const struct string *str1, const char *str2, size_t n)
{
    if (!n)
        return 0;
    for (size_t i = 0; i < n; i++)
    {
        if (str1->data[i] != str2[i])
            return str1->data[i] - str2[i];
    }
    return 0;
}

/*
 * my version of strdup
 */
char *my_strdup(const char *str)
{
    size_t len = 0;
    while (str[len] != '\0')
        len++;
    char *string = malloc(len + 1);
    if (string)
    {
        size_t i = 0;
        while (i < len)
        {
            string[i] = str[i];
            i++;
        }
        string[len] = '\0';
    }
    return string;
}

/*
 * Adapted version of strdup
 */
char *string_dup(struct string *str)
{
    char *string = malloc(str->size + 1);
    if (string)
    {
        size_t i = 0;
        while (i < str->size)
        {
            string[i] = str->data[i];
            i++;
        }
        string[str->size] = '\0';
    }
    return string;
}

/*
 * adapted version of strconcat()
 */
void string_concat_str(struct string *str, const char *to_concat, size_t size)
{
    if (size)
    {
        str->data = realloc(str->data, str->size + size);
        for (size_t i = str->size; i < str->size + size; i++)
            str->data[i] = to_concat[i - str->size];
        str->size += size;
    }
}

/*
 * adapted version of strchr
 */
struct string *string_chr(struct string *str, char c)
{
    size_t i = 0;
    while (i < str->size)
    {
        if (str->data[i] == c)
            return string_create(str->data + i, str->size - i);
    }
    return NULL;
}

/*
 * Adapted version of strstr
 */
struct string *string_str(struct string *haystack, struct string *needle)
{
    size_t i = 0;
    ssize_t j;
    while (i < haystack->size)
    {
        j = needle->size - 1;
        while (j >= 0 && haystack->data[i + j] == needle->data[j])
            j--;
        if (j < 0)
            return string_create(haystack->data + i, haystack->size - i);
        else
        {
            ssize_t k = needle->size - 1;
            while (k >= 0
                   && haystack->data[i + needle->size - 1] != needle->data[k])
                k--;
            if (k < 0) // the letter does not belong to needle
                i += needle->size;
            else
                i += k - 1; // letter belongs, k = distance from the last letter
        }
    }
    return NULL;
}

/*
 * adapted version of strtok_r() but it looks for a pattern
 */
static int is_pattern(struct string *str, struct string *pattern, size_t offset)
{
    if ((str->size - offset) < pattern->size)
        return 0;
    size_t i = 0;
    while (i < pattern->size)
    {
        if (str->data[offset] != pattern->data[i])
            return 0;
        i++;
        offset++;
    }
    return 1;
}
struct string *string_tok_pattern(struct string *str, struct string *pattern,
                                  struct string **saveptr)
{
    int to_free = 0;
    if (!str)
    {
        if (!saveptr || !(*saveptr))
            return NULL;
        str = string_create((*saveptr)->data, (*saveptr)->size);
        to_free = 1;
    }

    size_t offset = 0; // in case of patterniters at the beginning
    while (offset < str->size && is_pattern(str, pattern, offset))
        offset += pattern->size;
    if (offset == str->size)
    {
        if (to_free)
            string_destroy(str);
        return NULL;
    }

    char *start = str->data + offset;
    size_t i = offset;
    while (i < str->size && !is_pattern(str, pattern, i))
        i++;
    if (i == str->size)
    {
        if (to_free)
            string_destroy(str);
        return NULL;
    }


    struct string *res = string_create(start, i - offset);

    while (i < str->size && is_pattern(str, pattern, i))
        i += pattern->size;

    string_destroy(*saveptr);
    *saveptr = string_create(start + i - offset, str->size - i);

    if (to_free)
        string_destroy(str);

    return res;
}

/*
 * Adapted version of strtok()
 */
static int is_delim(char c, struct string *delim)
{
    size_t i = 0;
    while (i < delim->size)
    {
        if (c == delim->data[i])
            return 1;
        i++;
    }
    return 0;
}
struct string *string_tok(struct string *str, struct string *delim,
                          struct string **saveptr)
{
    int to_free = 0;
    if (!str)
    {
        if (!saveptr || !(*saveptr))
            return NULL;
        str = string_create((*saveptr)->data, (*saveptr)->size);
        to_free = 1;
    }

    size_t offset = 0; // in case of delimiters at the beginning
    while (offset < str->size && is_delim(*(str->data + offset), delim))
        offset++;
    if (offset == str->size)
    {
        string_destroy(str);
        return NULL;
    }

    char *start = str->data + offset;
    size_t i = offset;
    while (i < str->size && !is_delim(*(str->data + i), delim))
        i++;

    struct string *res = string_create(start, i - offset);

    while (i < str->size && is_delim(*(str->data + i), delim))
        i++;

    string_destroy(*saveptr);
    *saveptr = string_create(start + i - offset, str->size - i);

    if (to_free)
        string_destroy(str);

    return res;
}

/*
 * Free the structure
 */
void string_destroy(struct string *str)
{
    if (str)
    {
        free(str->data);
        free(str);
    }
}
