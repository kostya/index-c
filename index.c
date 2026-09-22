// # C Stress Test (https://github.com/kostya/index-c)

// A single-file, self-contained stress test for C compilers and optimizers.

// This is an amalgamated/simplified version of the LangArena benchmark
// (https://github.com/kostya/LangArena) for the C language.

// LangArena: A collection of 50 tasks across 24 languages - complex,
// non-synthetic, and inspired by real-world problems (JSON, Base64, CSV,
// neural networks, compression, maze A*, graph algorithms, sorting, hashing,
// interpreters, parallel matmul, and more).

// Fully "all in" - it has no external dependencies beyond the standard
// libraries. That is, this file is completely self-contained and can be used
// as a stress test for C compilers on its own, as well as for the optimizer.

// Build:

//     gcc -Wno-format index.c -O3 -lm -o ./index
//     ./index

// It includes the following third-party libraries:

// * https://github.com/DaveGamble/cJSON
//   MIT License (Copyright (c) 2009-2017 Dave Gamble and cJSON contributors)

// * https://github.com/troydhanson/uthash
//   Copyright (c) 2005-2026, Troy D. Hanson
//   https://troydhanson.github.io/uthash/

// * https://github.com/kokke/tiny-regex-c
//   All material in this repository is in the public domain.

// * https://github.com/wareya/Remimu/
//   Creative Commons Legal Code

// It also includes the config directly in this file.

// All parameters have been tuned so that each test runs for roughly 1 second
// on my machine. That is, the total time is approximately 50 seconds (but
// results may differ on other hardware).

// MIT License Makarchev Konstantin

#include <pthread.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <limits.h>
#include <math.h>
#include <stdbool.h>
#include <time.h>
#include <float.h>

#ifndef __DBL_EPSILON__
#define __DBL_EPSILON__ 2.2204460492503131e-16
#endif

// https://github.com/DaveGamble/cJSON/blob/master/cJSON.h
// ------------------------------------------------------------------------------------------
/*
  Copyright (c) 2009-2017 Dave Gamble and cJSON contributors

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

#ifndef cJSON__h
#define cJSON__h

#ifdef __cplusplus
extern "C"
{
#endif

#if !defined(__WINDOWS__) && (defined(WIN32) || defined(WIN64) || defined(_MSC_VER) || defined(_WIN32))
#define __WINDOWS__
#endif

#ifdef __WINDOWS__

/* When compiling for windows, we specify a specific calling convention to avoid issues where we are being called from a project with a different default calling convention.  For windows you have 3 define options:

CJSON_HIDE_SYMBOLS - Define this in the case where you don't want to ever dllexport symbols
CJSON_EXPORT_SYMBOLS - Define this on library build when you want to dllexport symbols (default)
CJSON_IMPORT_SYMBOLS - Define this if you want to dllimport symbol

For *nix builds that support visibility attribute, you can define similar behavior by

setting default visibility to hidden by adding
-fvisibility=hidden (for gcc)
or
-xldscope=hidden (for sun cc)
to CFLAGS

then using the CJSON_API_VISIBILITY flag to "export" the same symbols the way CJSON_EXPORT_SYMBOLS does

*/

#define CJSON_CDECL __cdecl
#define CJSON_STDCALL __stdcall

/* export symbols by default, this is necessary for copy pasting the C and header file */
#if !defined(CJSON_HIDE_SYMBOLS) && !defined(CJSON_IMPORT_SYMBOLS) && !defined(CJSON_EXPORT_SYMBOLS)
#define CJSON_EXPORT_SYMBOLS
#endif

#if defined(CJSON_HIDE_SYMBOLS)
#define CJSON_PUBLIC(type)   type CJSON_STDCALL
#elif defined(CJSON_EXPORT_SYMBOLS)
#define CJSON_PUBLIC(type)   __declspec(dllexport) type CJSON_STDCALL
#elif defined(CJSON_IMPORT_SYMBOLS)
#define CJSON_PUBLIC(type)   __declspec(dllimport) type CJSON_STDCALL
#endif
#else /* !__WINDOWS__ */
#define CJSON_CDECL
#define CJSON_STDCALL

#if (defined(__GNUC__) || defined(__SUNPRO_CC) || defined (__SUNPRO_C)) && defined(CJSON_API_VISIBILITY)
#define CJSON_PUBLIC(type)   __attribute__((visibility("default"))) type
#else
#define CJSON_PUBLIC(type) type
#endif
#endif

/* project version */
#define CJSON_VERSION_MAJOR 1
#define CJSON_VERSION_MINOR 7
#define CJSON_VERSION_PATCH 19

#include <stddef.h>

/* cJSON Types: */
#define cJSON_Invalid (0)
#define cJSON_False  (1 << 0)
#define cJSON_True   (1 << 1)
#define cJSON_NULL   (1 << 2)
#define cJSON_Number (1 << 3)
#define cJSON_String (1 << 4)
#define cJSON_Array  (1 << 5)
#define cJSON_Object (1 << 6)
#define cJSON_Raw    (1 << 7) /* raw json */

#define cJSON_IsReference 256
#define cJSON_StringIsConst 512

/* The cJSON structure: */
typedef struct cJSON
{
    /* next/prev allow you to walk array/object chains. Alternatively, use GetArraySize/GetArrayItem/GetObjectItem */
    struct cJSON *next;
    struct cJSON *prev;
    /* An array or object item will have a child pointer pointing to a chain of the items in the array/object. */
    struct cJSON *child;

    /* The type of the item, as above. */
    int type;

    /* The item's string, if type==cJSON_String  and type == cJSON_Raw */
    char *valuestring;
    /* writing to valueint is DEPRECATED, use cJSON_SetNumberValue instead */
    int valueint;
    /* The item's number, if type==cJSON_Number */
    double valuedouble;

    /* The item's name string, if this item is the child of, or is in the list of subitems of an object. */
    char *string;
} cJSON;

typedef struct cJSON_Hooks
{
      /* malloc/free are CDECL on Windows regardless of the default calling convention of the compiler, so ensure the hooks allow passing those functions directly. */
      void *(CJSON_CDECL *malloc_fn)(size_t sz);
      void (CJSON_CDECL *free_fn)(void *ptr);
} cJSON_Hooks;

typedef int cJSON_bool;

/* Limits how deeply nested arrays/objects can be before cJSON rejects to parse them.
 * This is to prevent stack overflows. */
#ifndef CJSON_NESTING_LIMIT
#define CJSON_NESTING_LIMIT 1000
#endif

/* Limits the length of circular references can be before cJSON rejects to parse them.
 * This is to prevent stack overflows. */
#ifndef CJSON_CIRCULAR_LIMIT
#define CJSON_CIRCULAR_LIMIT 10000
#endif

/* returns the version of cJSON as a string */
CJSON_PUBLIC(const char*) cJSON_Version(void);

/* Supply malloc, realloc and free functions to cJSON */
CJSON_PUBLIC(void) cJSON_InitHooks(cJSON_Hooks* hooks);

/* Memory Management: the caller is always responsible to free the results from all variants of cJSON_Parse (with cJSON_Delete) and cJSON_Print (with stdlib free, cJSON_Hooks.free_fn, or cJSON_free as appropriate). The exception is cJSON_PrintPreallocated, where the caller has full responsibility of the buffer. */
/* Supply a block of JSON, and this returns a cJSON object you can interrogate. */
CJSON_PUBLIC(cJSON *) cJSON_Parse(const char *value);
CJSON_PUBLIC(cJSON *) cJSON_ParseWithLength(const char *value, size_t buffer_length);
/* ParseWithOpts allows you to require (and check) that the JSON is null terminated, and to retrieve the pointer to the final byte parsed. */
/* If you supply a ptr in return_parse_end and parsing fails, then return_parse_end will contain a pointer to the error so will match cJSON_GetErrorPtr(). */
CJSON_PUBLIC(cJSON *) cJSON_ParseWithOpts(const char *value, const char **return_parse_end, cJSON_bool require_null_terminated);
CJSON_PUBLIC(cJSON *) cJSON_ParseWithLengthOpts(const char *value, size_t buffer_length, const char **return_parse_end, cJSON_bool require_null_terminated);

/* Render a cJSON entity to text for transfer/storage. */
CJSON_PUBLIC(char *) cJSON_Print(const cJSON *item);
/* Render a cJSON entity to text for transfer/storage without any formatting. */
CJSON_PUBLIC(char *) cJSON_PrintUnformatted(const cJSON *item);
/* Render a cJSON entity to text using a buffered strategy. prebuffer is a guess at the final size. guessing well reduces reallocation. fmt=0 gives unformatted, =1 gives formatted */
CJSON_PUBLIC(char *) cJSON_PrintBuffered(const cJSON *item, int prebuffer, cJSON_bool fmt);
/* Render a cJSON entity to text using a buffer already allocated in memory with given length. Returns 1 on success and 0 on failure. */
/* NOTE: cJSON is not always 100% accurate in estimating how much memory it will use, so to be safe allocate 5 bytes more than you actually need */
CJSON_PUBLIC(cJSON_bool) cJSON_PrintPreallocated(cJSON *item, char *buffer, const int length, const cJSON_bool format);
/* Delete a cJSON entity and all subentities. */
CJSON_PUBLIC(void) cJSON_Delete(cJSON *item);

/* Returns the number of items in an array (or object). */
CJSON_PUBLIC(int) cJSON_GetArraySize(const cJSON *array);
/* Retrieve item number "index" from array "array". Returns NULL if unsuccessful. */
CJSON_PUBLIC(cJSON *) cJSON_GetArrayItem(const cJSON *array, int index);
/* Get item "string" from object. Case insensitive. */
CJSON_PUBLIC(cJSON *) cJSON_GetObjectItem(const cJSON * const object, const char * const string);
CJSON_PUBLIC(cJSON *) cJSON_GetObjectItemCaseSensitive(const cJSON * const object, const char * const string);
CJSON_PUBLIC(cJSON_bool) cJSON_HasObjectItem(const cJSON *object, const char *string);
/* For analysing failed parses. This returns a pointer to the parse error. You'll probably need to look a few chars back to make sense of it. Defined when cJSON_Parse() returns 0. 0 when cJSON_Parse() succeeds. */
CJSON_PUBLIC(const char *) cJSON_GetErrorPtr(void);

/* Check item type and return its value */
CJSON_PUBLIC(char *) cJSON_GetStringValue(const cJSON * const item);
CJSON_PUBLIC(double) cJSON_GetNumberValue(const cJSON * const item);

/* These functions check the type of an item */
CJSON_PUBLIC(cJSON_bool) cJSON_IsInvalid(const cJSON * const item);
CJSON_PUBLIC(cJSON_bool) cJSON_IsFalse(const cJSON * const item);
CJSON_PUBLIC(cJSON_bool) cJSON_IsTrue(const cJSON * const item);
CJSON_PUBLIC(cJSON_bool) cJSON_IsBool(const cJSON * const item);
CJSON_PUBLIC(cJSON_bool) cJSON_IsNull(const cJSON * const item);
CJSON_PUBLIC(cJSON_bool) cJSON_IsNumber(const cJSON * const item);
CJSON_PUBLIC(cJSON_bool) cJSON_IsString(const cJSON * const item);
CJSON_PUBLIC(cJSON_bool) cJSON_IsArray(const cJSON * const item);
CJSON_PUBLIC(cJSON_bool) cJSON_IsObject(const cJSON * const item);
CJSON_PUBLIC(cJSON_bool) cJSON_IsRaw(const cJSON * const item);

/* These calls create a cJSON item of the appropriate type. */
CJSON_PUBLIC(cJSON *) cJSON_CreateNull(void);
CJSON_PUBLIC(cJSON *) cJSON_CreateTrue(void);
CJSON_PUBLIC(cJSON *) cJSON_CreateFalse(void);
CJSON_PUBLIC(cJSON *) cJSON_CreateBool(cJSON_bool boolean);
CJSON_PUBLIC(cJSON *) cJSON_CreateNumber(double num);
CJSON_PUBLIC(cJSON *) cJSON_CreateString(const char *string);
/* raw json */
CJSON_PUBLIC(cJSON *) cJSON_CreateRaw(const char *raw);
CJSON_PUBLIC(cJSON *) cJSON_CreateArray(void);
CJSON_PUBLIC(cJSON *) cJSON_CreateObject(void);

/* Create a string where valuestring references a string so
 * it will not be freed by cJSON_Delete */
CJSON_PUBLIC(cJSON *) cJSON_CreateStringReference(const char *string);
/* Create an object/array that only references it's elements so
 * they will not be freed by cJSON_Delete */
CJSON_PUBLIC(cJSON *) cJSON_CreateObjectReference(const cJSON *child);
CJSON_PUBLIC(cJSON *) cJSON_CreateArrayReference(const cJSON *child);

/* These utilities create an Array of count items.
 * The parameter count cannot be greater than the number of elements in the number array, otherwise array access will be out of bounds.*/
CJSON_PUBLIC(cJSON *) cJSON_CreateIntArray(const int *numbers, int count);
CJSON_PUBLIC(cJSON *) cJSON_CreateFloatArray(const float *numbers, int count);
CJSON_PUBLIC(cJSON *) cJSON_CreateDoubleArray(const double *numbers, int count);
CJSON_PUBLIC(cJSON *) cJSON_CreateStringArray(const char *const *strings, int count);

/* Append item to the specified array/object. */
CJSON_PUBLIC(cJSON_bool) cJSON_AddItemToArray(cJSON *array, cJSON *item);
CJSON_PUBLIC(cJSON_bool) cJSON_AddItemToObject(cJSON *object, const char *string, cJSON *item);
/* Use this when string is definitely const (i.e. a literal, or as good as), and will definitely survive the cJSON object.
 * WARNING: When this function was used, make sure to always check that (item->type & cJSON_StringIsConst) is zero before
 * writing to `item->string` */
CJSON_PUBLIC(cJSON_bool) cJSON_AddItemToObjectCS(cJSON *object, const char *string, cJSON *item);
/* Append reference to item to the specified array/object. Use this when you want to add an existing cJSON to a new cJSON, but don't want to corrupt your existing cJSON. */
CJSON_PUBLIC(cJSON_bool) cJSON_AddItemReferenceToArray(cJSON *array, cJSON *item);
CJSON_PUBLIC(cJSON_bool) cJSON_AddItemReferenceToObject(cJSON *object, const char *string, cJSON *item);

/* Remove/Detach items from Arrays/Objects. */
CJSON_PUBLIC(cJSON *) cJSON_DetachItemViaPointer(cJSON *parent, cJSON * const item);
CJSON_PUBLIC(cJSON *) cJSON_DetachItemFromArray(cJSON *array, int which);
CJSON_PUBLIC(void) cJSON_DeleteItemFromArray(cJSON *array, int which);
CJSON_PUBLIC(cJSON *) cJSON_DetachItemFromObject(cJSON *object, const char *string);
CJSON_PUBLIC(cJSON *) cJSON_DetachItemFromObjectCaseSensitive(cJSON *object, const char *string);
CJSON_PUBLIC(void) cJSON_DeleteItemFromObject(cJSON *object, const char *string);
CJSON_PUBLIC(void) cJSON_DeleteItemFromObjectCaseSensitive(cJSON *object, const char *string);

/* Update array items. */
CJSON_PUBLIC(cJSON_bool) cJSON_InsertItemInArray(cJSON *array, int which, cJSON *newitem); /* Shifts pre-existing items to the right. */
CJSON_PUBLIC(cJSON_bool) cJSON_ReplaceItemViaPointer(cJSON * const parent, cJSON * const item, cJSON * replacement);
CJSON_PUBLIC(cJSON_bool) cJSON_ReplaceItemInArray(cJSON *array, int which, cJSON *newitem);
CJSON_PUBLIC(cJSON_bool) cJSON_ReplaceItemInObject(cJSON *object,const char *string,cJSON *newitem);
CJSON_PUBLIC(cJSON_bool) cJSON_ReplaceItemInObjectCaseSensitive(cJSON *object,const char *string,cJSON *newitem);

/* Duplicate a cJSON item */
CJSON_PUBLIC(cJSON *) cJSON_Duplicate(const cJSON *item, cJSON_bool recurse);
/* Duplicate will create a new, identical cJSON item to the one you pass, in new memory that will
 * need to be released. With recurse!=0, it will duplicate any children connected to the item.
 * The item->next and ->prev pointers are always zero on return from Duplicate. */
/* Recursively compare two cJSON items for equality. If either a or b is NULL or invalid, they will be considered unequal.
 * case_sensitive determines if object keys are treated case sensitive (1) or case insensitive (0) */
CJSON_PUBLIC(cJSON_bool) cJSON_Compare(const cJSON * const a, const cJSON * const b, const cJSON_bool case_sensitive);

/* Minify a strings, remove blank characters(such as ' ', '\t', '\r', '\n') from strings.
 * The input pointer json cannot point to a read-only address area, such as a string constant, 
 * but should point to a readable and writable address area. */
CJSON_PUBLIC(void) cJSON_Minify(char *json);

/* Helper functions for creating and adding items to an object at the same time.
 * They return the added item or NULL on failure. */
CJSON_PUBLIC(cJSON*) cJSON_AddNullToObject(cJSON * const object, const char * const name);
CJSON_PUBLIC(cJSON*) cJSON_AddTrueToObject(cJSON * const object, const char * const name);
CJSON_PUBLIC(cJSON*) cJSON_AddFalseToObject(cJSON * const object, const char * const name);
CJSON_PUBLIC(cJSON*) cJSON_AddBoolToObject(cJSON * const object, const char * const name, const cJSON_bool boolean);
CJSON_PUBLIC(cJSON*) cJSON_AddNumberToObject(cJSON * const object, const char * const name, const double number);
CJSON_PUBLIC(cJSON*) cJSON_AddStringToObject(cJSON * const object, const char * const name, const char * const string);
CJSON_PUBLIC(cJSON*) cJSON_AddRawToObject(cJSON * const object, const char * const name, const char * const raw);
CJSON_PUBLIC(cJSON*) cJSON_AddObjectToObject(cJSON * const object, const char * const name);
CJSON_PUBLIC(cJSON*) cJSON_AddArrayToObject(cJSON * const object, const char * const name);

/* When assigning an integer value, it needs to be propagated to valuedouble too. */
#define cJSON_SetIntValue(object, number) ((object) ? (object)->valueint = (object)->valuedouble = (number) : (number))
/* helper for the cJSON_SetNumberValue macro */
CJSON_PUBLIC(double) cJSON_SetNumberHelper(cJSON *object, double number);
#define cJSON_SetNumberValue(object, number) ((object != NULL) ? cJSON_SetNumberHelper(object, (double)number) : (number))
/* Change the valuestring of a cJSON_String object, only takes effect when type of object is cJSON_String */
CJSON_PUBLIC(char*) cJSON_SetValuestring(cJSON *object, const char *valuestring);

/* If the object is not a boolean type this does nothing and returns cJSON_Invalid else it returns the new type*/
#define cJSON_SetBoolValue(object, boolValue) ( \
    (object != NULL && ((object)->type & (cJSON_False|cJSON_True))) ? \
    (object)->type=((object)->type &(~(cJSON_False|cJSON_True)))|((boolValue)?cJSON_True:cJSON_False) : \
    cJSON_Invalid\
)

/* Macro for iterating over an array or object */
#define cJSON_ArrayForEach(element, array) for(element = (array != NULL) ? (array)->child : NULL; element != NULL; element = element->next)

/* malloc/free objects using the malloc/free functions that have been set with cJSON_InitHooks */
CJSON_PUBLIC(void *) cJSON_malloc(size_t size);
CJSON_PUBLIC(void) cJSON_free(void *object);

#ifdef __cplusplus
}
#endif

#endif
// ------------------------------------------------------------------------------------------


// https://github.com/DaveGamble/cJSON/blob/master/cJSON.c
// ------------------------------------------------------------------------------------------
/*
  Copyright (c) 2009-2017 Dave Gamble and cJSON contributors

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

/* cJSON */
/* JSON parser in C. */

/* disable warnings about old C89 functions in MSVC */
#if !defined(_CRT_SECURE_NO_DEPRECATE) && defined(_MSC_VER)
#define _CRT_SECURE_NO_DEPRECATE
#endif

#ifdef __GNUC__
#pragma GCC visibility push(default)
#endif
#if defined(_MSC_VER)
#pragma warning (push)
/* disable warning about single line comments in system headers */
#pragma warning (disable : 4001)
#endif

#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <limits.h>
#include <ctype.h>
#include <float.h>

#ifdef ENABLE_LOCALES
#include <locale.h>
#endif

#if defined(_MSC_VER)
#pragma warning (pop)
#endif
#ifdef __GNUC__
#pragma GCC visibility pop
#endif

/* define our own boolean type */
#ifdef true
#undef true
#endif
#define true ((cJSON_bool)1)

#ifdef false
#undef false
#endif
#define false ((cJSON_bool)0)

/* define isnan and isinf for ANSI C, if in C99 or above, isnan and isinf has been defined in math.h */
#ifndef isinf
#define isinf(d) (isnan((d - d)) && !isnan(d))
#endif
#ifndef isnan
#define isnan(d) (d != d)
#endif

#ifndef NAN
#ifdef _WIN32
#define NAN sqrt(-1.0)
#else
#define NAN 0.0/0.0
#endif
#endif

typedef struct {
    const unsigned char *json;
    size_t position;
} error;
static error global_error = { NULL, 0 };

CJSON_PUBLIC(const char *) cJSON_GetErrorPtr(void)
{
    return (const char*) (global_error.json + global_error.position);
}

CJSON_PUBLIC(char *) cJSON_GetStringValue(const cJSON * const item)
{
    if (!cJSON_IsString(item))
    {
        return NULL;
    }

    return item->valuestring;
}

CJSON_PUBLIC(double) cJSON_GetNumberValue(const cJSON * const item)
{
    if (!cJSON_IsNumber(item))
    {
        return (double) NAN;
    }

    return item->valuedouble;
}

/* This is a safeguard to prevent copy-pasters from using incompatible C and header files */
#if (CJSON_VERSION_MAJOR != 1) || (CJSON_VERSION_MINOR != 7) || (CJSON_VERSION_PATCH != 19)
    #error cJSON.h and cJSON.c have different versions. Make sure that both have the same.
#endif

CJSON_PUBLIC(const char*) cJSON_Version(void)
{
    static char version[15];
    sprintf(version, "%i.%i.%i", CJSON_VERSION_MAJOR, CJSON_VERSION_MINOR, CJSON_VERSION_PATCH);

    return version;
}

/* Case insensitive string comparison, doesn't consider two NULL pointers equal though */
static int case_insensitive_strcmp(const unsigned char *string1, const unsigned char *string2)
{
    if ((string1 == NULL) || (string2 == NULL))
    {
        return 1;
    }

    if (string1 == string2)
    {
        return 0;
    }

    for(; tolower(*string1) == tolower(*string2); (void)string1++, string2++)
    {
        if (*string1 == '\0')
        {
            return 0;
        }
    }

    return tolower(*string1) - tolower(*string2);
}

typedef struct internal_hooks
{
    void *(CJSON_CDECL *allocate)(size_t size);
    void (CJSON_CDECL *deallocate)(void *pointer);
    void *(CJSON_CDECL *reallocate)(void *pointer, size_t size);
} internal_hooks;

#if defined(_MSC_VER)
/* work around MSVC error C2322: '...' address of dllimport '...' is not static */
static void * CJSON_CDECL internal_malloc(size_t size)
{
    return malloc(size);
}
static void CJSON_CDECL internal_free(void *pointer)
{
    free(pointer);
}
static void * CJSON_CDECL internal_realloc(void *pointer, size_t size)
{
    return realloc(pointer, size);
}
#else
#define internal_malloc malloc
#define internal_free free
#define internal_realloc realloc
#endif

/* strlen of character literals resolved at compile time */
#define static_strlen(string_literal) (sizeof(string_literal) - sizeof(""))

static internal_hooks global_hooks = { internal_malloc, internal_free, internal_realloc };

static unsigned char* cJSON_strdup(const unsigned char* string, const internal_hooks * const hooks)
{
    size_t length = 0;
    unsigned char *copy = NULL;

    if (string == NULL)
    {
        return NULL;
    }

    length = strlen((const char*)string) + sizeof("");
    copy = (unsigned char*)hooks->allocate(length);
    if (copy == NULL)
    {
        return NULL;
    }
    memcpy(copy, string, length);

    return copy;
}

CJSON_PUBLIC(void) cJSON_InitHooks(cJSON_Hooks* hooks)
{
    if (hooks == NULL)
    {
        /* Reset hooks */
        global_hooks.allocate = malloc;
        global_hooks.deallocate = free;
        global_hooks.reallocate = realloc;
        return;
    }

    global_hooks.allocate = malloc;
    if (hooks->malloc_fn != NULL)
    {
        global_hooks.allocate = hooks->malloc_fn;
    }

    global_hooks.deallocate = free;
    if (hooks->free_fn != NULL)
    {
        global_hooks.deallocate = hooks->free_fn;
    }

    /* use realloc only if both free and malloc are used */
    global_hooks.reallocate = NULL;
    if ((global_hooks.allocate == malloc) && (global_hooks.deallocate == free))
    {
        global_hooks.reallocate = realloc;
    }
}

/* Internal constructor. */
static cJSON *cJSON_New_Item(const internal_hooks * const hooks)
{
    cJSON* node = (cJSON*)hooks->allocate(sizeof(cJSON));
    if (node)
    {
        memset(node, '\0', sizeof(cJSON));
    }

    return node;
}

/* Delete a cJSON structure. */
CJSON_PUBLIC(void) cJSON_Delete(cJSON *item)
{
    cJSON *next = NULL;
    while (item != NULL)
    {
        next = item->next;
        if (!(item->type & cJSON_IsReference) && (item->child != NULL))
        {
            cJSON_Delete(item->child);
        }
        if (!(item->type & cJSON_IsReference) && (item->valuestring != NULL))
        {
            global_hooks.deallocate(item->valuestring);
            item->valuestring = NULL;
        }
        if (!(item->type & cJSON_StringIsConst) && (item->string != NULL))
        {
            global_hooks.deallocate(item->string);
            item->string = NULL;
        }
        global_hooks.deallocate(item);
        item = next;
    }
}

/* get the decimal point character of the current locale */
static unsigned char get_decimal_point(void)
{
#ifdef ENABLE_LOCALES
    struct lconv *lconv = localeconv();
    return (unsigned char) lconv->decimal_point[0];
#else
    return '.';
#endif
}

typedef struct
{
    const unsigned char *content;
    size_t length;
    size_t offset;
    size_t depth; /* How deeply nested (in arrays/objects) is the input at the current offset. */
    internal_hooks hooks;
} parse_buffer;

/* check if the given size is left to read in a given parse buffer (starting with 1) */
#define can_read(buffer, size) ((buffer != NULL) && (((buffer)->offset + size) <= (buffer)->length))
/* check if the buffer can be accessed at the given index (starting with 0) */
#define can_access_at_index(buffer, index) ((buffer != NULL) && (((buffer)->offset + index) < (buffer)->length))
#define cannot_access_at_index(buffer, index) (!can_access_at_index(buffer, index))
/* get a pointer to the buffer at the position */
#define buffer_at_offset(buffer) ((buffer)->content + (buffer)->offset)

/* Parse the input text to generate a number, and populate the result into item. */
static cJSON_bool parse_number(cJSON * const item, parse_buffer * const input_buffer)
{
    double number = 0;
    unsigned char *after_end = NULL;
    unsigned char *number_c_string;
    unsigned char decimal_point = get_decimal_point();
    size_t i = 0;
    size_t number_string_length = 0;
    cJSON_bool has_decimal_point = false;

    if ((input_buffer == NULL) || (input_buffer->content == NULL))
    {
        return false;
    }

    /* copy the number into a temporary buffer and replace '.' with the decimal point
     * of the current locale (for strtod)
     * This also takes care of '\0' not necessarily being available for marking the end of the input */
    for (i = 0; can_access_at_index(input_buffer, i); i++)
    {
        switch (buffer_at_offset(input_buffer)[i])
        {
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
            case '+':
            case '-':
            case 'e':
            case 'E':
                number_string_length++;
                break;

            case '.':
                number_string_length++;
                has_decimal_point = true;
                break;

            default:
                goto loop_end;
        }
    }
loop_end:
    /* malloc for temporary buffer, add 1 for '\0' */
    number_c_string = (unsigned char *) input_buffer->hooks.allocate(number_string_length + 1);
    if (number_c_string == NULL)
    {
        return false; /* allocation failure */
    }

    memcpy(number_c_string, buffer_at_offset(input_buffer), number_string_length);
    number_c_string[number_string_length] = '\0';

    if (has_decimal_point)
    {
        for (i = 0; i < number_string_length; i++)
        {
            if (number_c_string[i] == '.')
            {
                /* replace '.' with the decimal point of the current locale (for strtod) */
                number_c_string[i] = decimal_point;
            }
        }
    }

    number = strtod((const char*)number_c_string, (char**)&after_end);
    if (number_c_string == after_end)
    {
        /* free the temporary buffer */
        input_buffer->hooks.deallocate(number_c_string);
        return false; /* parse_error */
    }

    item->valuedouble = number;

    /* use saturation in case of overflow */
    if (number >= INT_MAX)
    {
        item->valueint = INT_MAX;
    }
    else if (number <= (double)INT_MIN)
    {
        item->valueint = INT_MIN;
    }
    else
    {
        item->valueint = (int)number;
    }

    item->type = cJSON_Number;

    input_buffer->offset += (size_t)(after_end - number_c_string);
    /* free the temporary buffer */
    input_buffer->hooks.deallocate(number_c_string);
    return true;
}

/* don't ask me, but the original cJSON_SetNumberValue returns an integer or double */
CJSON_PUBLIC(double) cJSON_SetNumberHelper(cJSON *object, double number)
{
    if (object == NULL)
    {
        return (double)NAN;
    }

    if (number >= INT_MAX)
    {
        object->valueint = INT_MAX;
    }
    else if (number <= (double)INT_MIN)
    {
        object->valueint = INT_MIN;
    }
    else
    {
        object->valueint = (int)number;
    }

    return object->valuedouble = number;
}

/* Note: when passing a NULL valuestring, cJSON_SetValuestring treats this as an error and return NULL */
CJSON_PUBLIC(char*) cJSON_SetValuestring(cJSON *object, const char *valuestring)
{
    char *copy = NULL;
    size_t v1_len;
    size_t v2_len;
    /* if object's type is not cJSON_String or is cJSON_IsReference, it should not set valuestring */
    if ((object == NULL) || !(object->type & cJSON_String) || (object->type & cJSON_IsReference))
    {
        return NULL;
    }
    /* return NULL if the object is corrupted or valuestring is NULL */
    if (object->valuestring == NULL || valuestring == NULL)
    {
        return NULL;
    }

    v1_len = strlen(valuestring);
    v2_len = strlen(object->valuestring);

    if (v1_len <= v2_len)
    {
        /* strcpy does not handle overlapping string: [X1, X2] [Y1, Y2] => X2 < Y1 or Y2 < X1 */
        if (!( valuestring + v1_len < object->valuestring || object->valuestring + v2_len < valuestring ))
        {
            return NULL;
        }
        strcpy(object->valuestring, valuestring);
        return object->valuestring;
    }
    copy = (char*) cJSON_strdup((const unsigned char*)valuestring, &global_hooks);
    if (copy == NULL)
    {
        return NULL;
    }
    if (object->valuestring != NULL)
    {
        cJSON_free(object->valuestring);
    }
    object->valuestring = copy;

    return copy;
}

typedef struct
{
    unsigned char *buffer;
    size_t length;
    size_t offset;
    size_t depth; /* current nesting depth (for formatted printing) */
    cJSON_bool noalloc;
    cJSON_bool format; /* is this print a formatted print */
    internal_hooks hooks;
} printbuffer;

/* realloc printbuffer if necessary to have at least "needed" bytes more */
static unsigned char* ensure(printbuffer * const p, size_t needed)
{
    unsigned char *newbuffer = NULL;
    size_t newsize = 0;

    if ((p == NULL) || (p->buffer == NULL))
    {
        return NULL;
    }

    if ((p->length > 0) && (p->offset >= p->length))
    {
        /* make sure that offset is valid */
        return NULL;
    }

    if (needed > INT_MAX)
    {
        /* sizes bigger than INT_MAX are currently not supported */
        return NULL;
    }

    needed += p->offset + 1;
    if (needed <= p->length)
    {
        return p->buffer + p->offset;
    }

    if (p->noalloc) {
        return NULL;
    }

    /* calculate new buffer size */
    if (needed > (INT_MAX / 2))
    {
        /* overflow of int, use INT_MAX if possible */
        if (needed <= INT_MAX)
        {
            newsize = INT_MAX;
        }
        else
        {
            return NULL;
        }
    }
    else
    {
        newsize = needed * 2;
    }

    if (p->hooks.reallocate != NULL)
    {
        /* reallocate with realloc if available */
        newbuffer = (unsigned char*)p->hooks.reallocate(p->buffer, newsize);
        if (newbuffer == NULL)
        {
            p->hooks.deallocate(p->buffer);
            p->length = 0;
            p->buffer = NULL;

            return NULL;
        }
    }
    else
    {
        /* otherwise reallocate manually */
        newbuffer = (unsigned char*)p->hooks.allocate(newsize);
        if (!newbuffer)
        {
            p->hooks.deallocate(p->buffer);
            p->length = 0;
            p->buffer = NULL;

            return NULL;
        }

        memcpy(newbuffer, p->buffer, p->offset + 1);
        p->hooks.deallocate(p->buffer);
    }
    p->length = newsize;
    p->buffer = newbuffer;

    return newbuffer + p->offset;
}

/* calculate the new length of the string in a printbuffer and update the offset */
static void update_offset(printbuffer * const buffer)
{
    const unsigned char *buffer_pointer = NULL;
    if ((buffer == NULL) || (buffer->buffer == NULL))
    {
        return;
    }
    buffer_pointer = buffer->buffer + buffer->offset;

    buffer->offset += strlen((const char*)buffer_pointer);
}

/* securely comparison of floating-point variables */
static cJSON_bool compare_double(double a, double b)
{
    double maxVal = fabs(a) > fabs(b) ? fabs(a) : fabs(b);
    return (fabs(a - b) <= maxVal * DBL_EPSILON);
}

/* Render the number nicely from the given item into a string. */
static cJSON_bool print_number(const cJSON * const item, printbuffer * const output_buffer)
{
    unsigned char *output_pointer = NULL;
    double d = item->valuedouble;
    int length = 0;
    size_t i = 0;
    unsigned char number_buffer[26] = {0}; /* temporary buffer to print the number into */
    unsigned char decimal_point = get_decimal_point();
    double test = 0.0;

    if (output_buffer == NULL)
    {
        return false;
    }

    /* This checks for NaN and Infinity */
    if (isnan(d) || isinf(d))
    {
        length = sprintf((char*)number_buffer, "null");
    }
    else if(d == (double)item->valueint)
    {
        length = sprintf((char*)number_buffer, "%d", item->valueint);
    }
    else
    {
        /* Try 15 decimal places of precision to avoid nonsignificant nonzero digits */
        length = sprintf((char*)number_buffer, "%1.15g", d);

        /* Check whether the original double can be recovered */
        if ((sscanf((char*)number_buffer, "%lg", &test) != 1) || !compare_double((double)test, d))
        {
            /* If not, print with 17 decimal places of precision */
            length = sprintf((char*)number_buffer, "%1.17g", d);
        }
    }

    /* sprintf failed or buffer overrun occurred */
    if ((length < 0) || (length > (int)(sizeof(number_buffer) - 1)))
    {
        return false;
    }

    /* reserve appropriate space in the output */
    output_pointer = ensure(output_buffer, (size_t)length + sizeof(""));
    if (output_pointer == NULL)
    {
        return false;
    }

    /* copy the printed number to the output and replace locale
     * dependent decimal point with '.' */
    for (i = 0; i < ((size_t)length); i++)
    {
        if (number_buffer[i] == decimal_point)
        {
            output_pointer[i] = '.';
            continue;
        }

        output_pointer[i] = number_buffer[i];
    }
    output_pointer[i] = '\0';

    output_buffer->offset += (size_t)length;

    return true;
}

/* parse 4 digit hexadecimal number */
static unsigned parse_hex4(const unsigned char * const input)
{
    unsigned int h = 0;
    size_t i = 0;

    for (i = 0; i < 4; i++)
    {
        /* parse digit */
        if ((input[i] >= '0') && (input[i] <= '9'))
        {
            h += (unsigned int) input[i] - '0';
        }
        else if ((input[i] >= 'A') && (input[i] <= 'F'))
        {
            h += (unsigned int) 10 + input[i] - 'A';
        }
        else if ((input[i] >= 'a') && (input[i] <= 'f'))
        {
            h += (unsigned int) 10 + input[i] - 'a';
        }
        else /* invalid */
        {
            return 0;
        }

        if (i < 3)
        {
            /* shift left to make place for the next nibble */
            h = h << 4;
        }
    }

    return h;
}

/* converts a UTF-16 literal to UTF-8
 * A literal can be one or two sequences of the form \uXXXX */
static unsigned char utf16_literal_to_utf8(const unsigned char * const input_pointer, const unsigned char * const input_end, unsigned char **output_pointer)
{
    long unsigned int codepoint = 0;
    unsigned int first_code = 0;
    const unsigned char *first_sequence = input_pointer;
    unsigned char utf8_length = 0;
    unsigned char utf8_position = 0;
    unsigned char sequence_length = 0;
    unsigned char first_byte_mark = 0;

    if ((input_end - first_sequence) < 6)
    {
        /* input ends unexpectedly */
        goto fail;
    }

    /* get the first utf16 sequence */
    first_code = parse_hex4(first_sequence + 2);

    /* check that the code is valid */
    if (((first_code >= 0xDC00) && (first_code <= 0xDFFF)))
    {
        goto fail;
    }

    /* UTF16 surrogate pair */
    if ((first_code >= 0xD800) && (first_code <= 0xDBFF))
    {
        const unsigned char *second_sequence = first_sequence + 6;
        unsigned int second_code = 0;
        sequence_length = 12; /* \uXXXX\uXXXX */

        if ((input_end - second_sequence) < 6)
        {
            /* input ends unexpectedly */
            goto fail;
        }

        if ((second_sequence[0] != '\\') || (second_sequence[1] != 'u'))
        {
            /* missing second half of the surrogate pair */
            goto fail;
        }

        /* get the second utf16 sequence */
        second_code = parse_hex4(second_sequence + 2);
        /* check that the code is valid */
        if ((second_code < 0xDC00) || (second_code > 0xDFFF))
        {
            /* invalid second half of the surrogate pair */
            goto fail;
        }


        /* calculate the unicode codepoint from the surrogate pair */
        codepoint = 0x10000 + (((first_code & 0x3FF) << 10) | (second_code & 0x3FF));
    }
    else
    {
        sequence_length = 6; /* \uXXXX */
        codepoint = first_code;
    }

    /* encode as UTF-8
     * takes at maximum 4 bytes to encode:
     * 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx */
    if (codepoint < 0x80)
    {
        /* normal ascii, encoding 0xxxxxxx */
        utf8_length = 1;
    }
    else if (codepoint < 0x800)
    {
        /* two bytes, encoding 110xxxxx 10xxxxxx */
        utf8_length = 2;
        first_byte_mark = 0xC0; /* 11000000 */
    }
    else if (codepoint < 0x10000)
    {
        /* three bytes, encoding 1110xxxx 10xxxxxx 10xxxxxx */
        utf8_length = 3;
        first_byte_mark = 0xE0; /* 11100000 */
    }
    else if (codepoint <= 0x10FFFF)
    {
        /* four bytes, encoding 1110xxxx 10xxxxxx 10xxxxxx 10xxxxxx */
        utf8_length = 4;
        first_byte_mark = 0xF0; /* 11110000 */
    }
    else
    {
        /* invalid unicode codepoint */
        goto fail;
    }

    /* encode as utf8 */
    for (utf8_position = (unsigned char)(utf8_length - 1); utf8_position > 0; utf8_position--)
    {
        /* 10xxxxxx */
        (*output_pointer)[utf8_position] = (unsigned char)((codepoint | 0x80) & 0xBF);
        codepoint >>= 6;
    }
    /* encode first byte */
    if (utf8_length > 1)
    {
        (*output_pointer)[0] = (unsigned char)((codepoint | first_byte_mark) & 0xFF);
    }
    else
    {
        (*output_pointer)[0] = (unsigned char)(codepoint & 0x7F);
    }

    *output_pointer += utf8_length;

    return sequence_length;

fail:
    return 0;
}

/* Parse the input text into an unescaped cinput, and populate item. */
static cJSON_bool parse_string(cJSON * const item, parse_buffer * const input_buffer)
{
    const unsigned char *input_pointer = buffer_at_offset(input_buffer) + 1;
    const unsigned char *input_end = buffer_at_offset(input_buffer) + 1;
    unsigned char *output_pointer = NULL;
    unsigned char *output = NULL;

    /* not a string */
    if (buffer_at_offset(input_buffer)[0] != '\"')
    {
        goto fail;
    }

    {
        /* calculate approximate size of the output (overestimate) */
        size_t allocation_length = 0;
        size_t skipped_bytes = 0;
        while (((size_t)(input_end - input_buffer->content) < input_buffer->length) && (*input_end != '\"'))
        {
            /* is escape sequence */
            if (input_end[0] == '\\')
            {
                if ((size_t)(input_end + 1 - input_buffer->content) >= input_buffer->length)
                {
                    /* prevent buffer overflow when last input character is a backslash */
                    goto fail;
                }
                skipped_bytes++;
                input_end++;
            }
            input_end++;
        }
        if (((size_t)(input_end - input_buffer->content) >= input_buffer->length) || (*input_end != '\"'))
        {
            goto fail; /* string ended unexpectedly */
        }

        /* This is at most how much we need for the output */
        allocation_length = (size_t) (input_end - buffer_at_offset(input_buffer)) - skipped_bytes;
        output = (unsigned char*)input_buffer->hooks.allocate(allocation_length + sizeof(""));
        if (output == NULL)
        {
            goto fail; /* allocation failure */
        }
    }

    output_pointer = output;
    /* loop through the string literal */
    while (input_pointer < input_end)
    {
        if (*input_pointer != '\\')
        {
            *output_pointer++ = *input_pointer++;
        }
        /* escape sequence */
        else
        {
            unsigned char sequence_length = 2;
            if ((input_end - input_pointer) < 1)
            {
                goto fail;
            }

            switch (input_pointer[1])
            {
                case 'b':
                    *output_pointer++ = '\b';
                    break;
                case 'f':
                    *output_pointer++ = '\f';
                    break;
                case 'n':
                    *output_pointer++ = '\n';
                    break;
                case 'r':
                    *output_pointer++ = '\r';
                    break;
                case 't':
                    *output_pointer++ = '\t';
                    break;
                case '\"':
                case '\\':
                case '/':
                    *output_pointer++ = input_pointer[1];
                    break;

                /* UTF-16 literal */
                case 'u':
                    sequence_length = utf16_literal_to_utf8(input_pointer, input_end, &output_pointer);
                    if (sequence_length == 0)
                    {
                        /* failed to convert UTF16-literal to UTF-8 */
                        goto fail;
                    }
                    break;

                default:
                    goto fail;
            }
            input_pointer += sequence_length;
        }
    }

    /* zero terminate the output */
    *output_pointer = '\0';

    item->type = cJSON_String;
    item->valuestring = (char*)output;

    input_buffer->offset = (size_t) (input_end - input_buffer->content);
    input_buffer->offset++;

    return true;

fail:
    if (output != NULL)
    {
        input_buffer->hooks.deallocate(output);
        output = NULL;
    }

    if (input_pointer != NULL)
    {
        input_buffer->offset = (size_t)(input_pointer - input_buffer->content);
    }

    return false;
}

/* Render the cstring provided to an escaped version that can be printed. */
static cJSON_bool print_string_ptr(const unsigned char * const input, printbuffer * const output_buffer)
{
    const unsigned char *input_pointer = NULL;
    unsigned char *output = NULL;
    unsigned char *output_pointer = NULL;
    size_t output_length = 0;
    /* numbers of additional characters needed for escaping */
    size_t escape_characters = 0;

    if (output_buffer == NULL)
    {
        return false;
    }

    /* empty string */
    if (input == NULL)
    {
        output = ensure(output_buffer, sizeof("\"\""));
        if (output == NULL)
        {
            return false;
        }
        strcpy((char*)output, "\"\"");

        return true;
    }

    /* set "flag" to 1 if something needs to be escaped */
    for (input_pointer = input; *input_pointer; input_pointer++)
    {
        switch (*input_pointer)
        {
            case '\"':
            case '\\':
            case '\b':
            case '\f':
            case '\n':
            case '\r':
            case '\t':
                /* one character escape sequence */
                escape_characters++;
                break;
            default:
                if (*input_pointer < 32)
                {
                    /* UTF-16 escape sequence uXXXX */
                    escape_characters += 5;
                }
                break;
        }
    }
    output_length = (size_t)(input_pointer - input) + escape_characters;

    output = ensure(output_buffer, output_length + sizeof("\"\""));
    if (output == NULL)
    {
        return false;
    }

    /* no characters have to be escaped */
    if (escape_characters == 0)
    {
        output[0] = '\"';
        memcpy(output + 1, input, output_length);
        output[output_length + 1] = '\"';
        output[output_length + 2] = '\0';

        return true;
    }

    output[0] = '\"';
    output_pointer = output + 1;
    /* copy the string */
    for (input_pointer = input; *input_pointer != '\0'; (void)input_pointer++, output_pointer++)
    {
        if ((*input_pointer > 31) && (*input_pointer != '\"') && (*input_pointer != '\\'))
        {
            /* normal character, copy */
            *output_pointer = *input_pointer;
        }
        else
        {
            /* character needs to be escaped */
            *output_pointer++ = '\\';
            switch (*input_pointer)
            {
                case '\\':
                    *output_pointer = '\\';
                    break;
                case '\"':
                    *output_pointer = '\"';
                    break;
                case '\b':
                    *output_pointer = 'b';
                    break;
                case '\f':
                    *output_pointer = 'f';
                    break;
                case '\n':
                    *output_pointer = 'n';
                    break;
                case '\r':
                    *output_pointer = 'r';
                    break;
                case '\t':
                    *output_pointer = 't';
                    break;
                default:
                    /* escape and print as unicode codepoint */
                    sprintf((char*)output_pointer, "u%04x", *input_pointer);
                    output_pointer += 4;
                    break;
            }
        }
    }
    output[output_length + 1] = '\"';
    output[output_length + 2] = '\0';

    return true;
}

/* Invoke print_string_ptr (which is useful) on an item. */
static cJSON_bool print_string(const cJSON * const item, printbuffer * const p)
{
    return print_string_ptr((unsigned char*)item->valuestring, p);
}

/* Predeclare these prototypes. */
static cJSON_bool parse_value(cJSON * const item, parse_buffer * const input_buffer);
static cJSON_bool print_value(const cJSON * const item, printbuffer * const output_buffer);
static cJSON_bool parse_array(cJSON * const item, parse_buffer * const input_buffer);
static cJSON_bool print_array(const cJSON * const item, printbuffer * const output_buffer);
static cJSON_bool parse_object(cJSON * const item, parse_buffer * const input_buffer);
static cJSON_bool print_object(const cJSON * const item, printbuffer * const output_buffer);

/* Utility to jump whitespace and cr/lf */
static parse_buffer *buffer_skip_whitespace(parse_buffer * const buffer)
{
    if ((buffer == NULL) || (buffer->content == NULL))
    {
        return NULL;
    }

    if (cannot_access_at_index(buffer, 0))
    {
        return buffer;
    }

    while (can_access_at_index(buffer, 0) && (buffer_at_offset(buffer)[0] <= 32))
    {
       buffer->offset++;
    }

    if (buffer->offset == buffer->length)
    {
        buffer->offset--;
    }

    return buffer;
}

/* skip the UTF-8 BOM (byte order mark) if it is at the beginning of a buffer */
static parse_buffer *skip_utf8_bom(parse_buffer * const buffer)
{
    if ((buffer == NULL) || (buffer->content == NULL) || (buffer->offset != 0))
    {
        return NULL;
    }

    if (can_access_at_index(buffer, 4) && (strncmp((const char*)buffer_at_offset(buffer), "\xEF\xBB\xBF", 3) == 0))
    {
        buffer->offset += 3;
    }

    return buffer;
}

CJSON_PUBLIC(cJSON *) cJSON_ParseWithOpts(const char *value, const char **return_parse_end, cJSON_bool require_null_terminated)
{
    size_t buffer_length;

    if (NULL == value)
    {
        return NULL;
    }

    /* Adding null character size due to require_null_terminated. */
    buffer_length = strlen(value) + sizeof("");

    return cJSON_ParseWithLengthOpts(value, buffer_length, return_parse_end, require_null_terminated);
}

/* Parse an object - create a new root, and populate. */
CJSON_PUBLIC(cJSON *) cJSON_ParseWithLengthOpts(const char *value, size_t buffer_length, const char **return_parse_end, cJSON_bool require_null_terminated)
{
    parse_buffer buffer = { 0, 0, 0, 0, { 0, 0, 0 } };
    cJSON *item = NULL;

    /* reset error position */
    global_error.json = NULL;
    global_error.position = 0;

    if (value == NULL || 0 == buffer_length)
    {
        goto fail;
    }

    buffer.content = (const unsigned char*)value;
    buffer.length = buffer_length;
    buffer.offset = 0;
    buffer.hooks = global_hooks;

    item = cJSON_New_Item(&global_hooks);
    if (item == NULL) /* memory fail */
    {
        goto fail;
    }

    if (!parse_value(item, buffer_skip_whitespace(skip_utf8_bom(&buffer))))
    {
        /* parse failure. ep is set. */
        goto fail;
    }

    /* if we require null-terminated JSON without appended garbage, skip and then check for a null terminator */
    if (require_null_terminated)
    {
        buffer_skip_whitespace(&buffer);
        if ((buffer.offset >= buffer.length) || buffer_at_offset(&buffer)[0] != '\0')
        {
            goto fail;
        }
    }
    if (return_parse_end)
    {
        *return_parse_end = (const char*)buffer_at_offset(&buffer);
    }

    return item;

fail:
    if (item != NULL)
    {
        cJSON_Delete(item);
    }

    if (value != NULL)
    {
        error local_error;
        local_error.json = (const unsigned char*)value;
        local_error.position = 0;

        if (buffer.offset < buffer.length)
        {
            local_error.position = buffer.offset;
        }
        else if (buffer.length > 0)
        {
            local_error.position = buffer.length - 1;
        }

        if (return_parse_end != NULL)
        {
            *return_parse_end = (const char*)local_error.json + local_error.position;
        }

        global_error = local_error;
    }

    return NULL;
}

/* Default options for cJSON_Parse */
CJSON_PUBLIC(cJSON *) cJSON_Parse(const char *value)
{
    return cJSON_ParseWithOpts(value, 0, 0);
}

CJSON_PUBLIC(cJSON *) cJSON_ParseWithLength(const char *value, size_t buffer_length)
{
    return cJSON_ParseWithLengthOpts(value, buffer_length, 0, 0);
}

#define cjson_min(a, b) (((a) < (b)) ? (a) : (b))

static unsigned char *print(const cJSON * const item, cJSON_bool format, const internal_hooks * const hooks)
{
    static const size_t default_buffer_size = 256;
    printbuffer buffer[1];
    unsigned char *printed = NULL;

    memset(buffer, 0, sizeof(buffer));

    /* create buffer */
    buffer->buffer = (unsigned char*) hooks->allocate(default_buffer_size);
    buffer->length = default_buffer_size;
    buffer->format = format;
    buffer->hooks = *hooks;
    if (buffer->buffer == NULL)
    {
        goto fail;
    }

    /* print the value */
    if (!print_value(item, buffer))
    {
        goto fail;
    }
    update_offset(buffer);

    /* check if reallocate is available */
    if (hooks->reallocate != NULL)
    {
        printed = (unsigned char*) hooks->reallocate(buffer->buffer, buffer->offset + 1);
        if (printed == NULL) {
            goto fail;
        }
        buffer->buffer = NULL;
    }
    else /* otherwise copy the JSON over to a new buffer */
    {
        printed = (unsigned char*) hooks->allocate(buffer->offset + 1);
        if (printed == NULL)
        {
            goto fail;
        }
        memcpy(printed, buffer->buffer, cjson_min(buffer->length, buffer->offset + 1));
        printed[buffer->offset] = '\0'; /* just to be sure */

        /* free the buffer */
        hooks->deallocate(buffer->buffer);
        buffer->buffer = NULL;
    }

    return printed;

fail:
    if (buffer->buffer != NULL)
    {
        hooks->deallocate(buffer->buffer);
        buffer->buffer = NULL;
    }

    if (printed != NULL)
    {
        hooks->deallocate(printed);
        printed = NULL;
    }

    return NULL;
}

/* Render a cJSON item/entity/structure to text. */
CJSON_PUBLIC(char *) cJSON_Print(const cJSON *item)
{
    return (char*)print(item, true, &global_hooks);
}

CJSON_PUBLIC(char *) cJSON_PrintUnformatted(const cJSON *item)
{
    return (char*)print(item, false, &global_hooks);
}

CJSON_PUBLIC(char *) cJSON_PrintBuffered(const cJSON *item, int prebuffer, cJSON_bool fmt)
{
    printbuffer p = { 0, 0, 0, 0, 0, 0, { 0, 0, 0 } };

    if (prebuffer < 0)
    {
        return NULL;
    }

    p.buffer = (unsigned char*)global_hooks.allocate((size_t)prebuffer);
    if (!p.buffer)
    {
        return NULL;
    }

    p.length = (size_t)prebuffer;
    p.offset = 0;
    p.noalloc = false;
    p.format = fmt;
    p.hooks = global_hooks;

    if (!print_value(item, &p))
    {
        global_hooks.deallocate(p.buffer);
        p.buffer = NULL;
        return NULL;
    }

    return (char*)p.buffer;
}

CJSON_PUBLIC(cJSON_bool) cJSON_PrintPreallocated(cJSON *item, char *buffer, const int length, const cJSON_bool format)
{
    printbuffer p = { 0, 0, 0, 0, 0, 0, { 0, 0, 0 } };

    if ((length < 0) || (buffer == NULL))
    {
        return false;
    }

    p.buffer = (unsigned char*)buffer;
    p.length = (size_t)length;
    p.offset = 0;
    p.noalloc = true;
    p.format = format;
    p.hooks = global_hooks;

    return print_value(item, &p);
}

/* Parser core - when encountering text, process appropriately. */
static cJSON_bool parse_value(cJSON * const item, parse_buffer * const input_buffer)
{
    if ((input_buffer == NULL) || (input_buffer->content == NULL))
    {
        return false; /* no input */
    }

    /* parse the different types of values */
    /* null */
    if (can_read(input_buffer, 4) && (strncmp((const char*)buffer_at_offset(input_buffer), "null", 4) == 0))
    {
        item->type = cJSON_NULL;
        input_buffer->offset += 4;
        return true;
    }
    /* false */
    if (can_read(input_buffer, 5) && (strncmp((const char*)buffer_at_offset(input_buffer), "false", 5) == 0))
    {
        item->type = cJSON_False;
        input_buffer->offset += 5;
        return true;
    }
    /* true */
    if (can_read(input_buffer, 4) && (strncmp((const char*)buffer_at_offset(input_buffer), "true", 4) == 0))
    {
        item->type = cJSON_True;
        item->valueint = 1;
        input_buffer->offset += 4;
        return true;
    }
    /* string */
    if (can_access_at_index(input_buffer, 0) && (buffer_at_offset(input_buffer)[0] == '\"'))
    {
        return parse_string(item, input_buffer);
    }
    /* number */
    if (can_access_at_index(input_buffer, 0) && ((buffer_at_offset(input_buffer)[0] == '-') || ((buffer_at_offset(input_buffer)[0] >= '0') && (buffer_at_offset(input_buffer)[0] <= '9'))))
    {
        return parse_number(item, input_buffer);
    }
    /* array */
    if (can_access_at_index(input_buffer, 0) && (buffer_at_offset(input_buffer)[0] == '['))
    {
        return parse_array(item, input_buffer);
    }
    /* object */
    if (can_access_at_index(input_buffer, 0) && (buffer_at_offset(input_buffer)[0] == '{'))
    {
        return parse_object(item, input_buffer);
    }

    return false;
}

/* Render a value to text. */
static cJSON_bool print_value(const cJSON * const item, printbuffer * const output_buffer)
{
    unsigned char *output = NULL;

    if ((item == NULL) || (output_buffer == NULL))
    {
        return false;
    }

    switch ((item->type) & 0xFF)
    {
        case cJSON_NULL:
            output = ensure(output_buffer, 5);
            if (output == NULL)
            {
                return false;
            }
            strcpy((char*)output, "null");
            return true;

        case cJSON_False:
            output = ensure(output_buffer, 6);
            if (output == NULL)
            {
                return false;
            }
            strcpy((char*)output, "false");
            return true;

        case cJSON_True:
            output = ensure(output_buffer, 5);
            if (output == NULL)
            {
                return false;
            }
            strcpy((char*)output, "true");
            return true;

        case cJSON_Number:
            return print_number(item, output_buffer);

        case cJSON_Raw:
        {
            size_t raw_length = 0;
            if (item->valuestring == NULL)
            {
                return false;
            }

            raw_length = strlen(item->valuestring) + sizeof("");
            output = ensure(output_buffer, raw_length);
            if (output == NULL)
            {
                return false;
            }
            memcpy(output, item->valuestring, raw_length);
            return true;
        }

        case cJSON_String:
            return print_string(item, output_buffer);

        case cJSON_Array:
            return print_array(item, output_buffer);

        case cJSON_Object:
            return print_object(item, output_buffer);

        default:
            return false;
    }
}

/* Build an array from input text. */
static cJSON_bool parse_array(cJSON * const item, parse_buffer * const input_buffer)
{
    cJSON *head = NULL; /* head of the linked list */
    cJSON *current_item = NULL;

    if (input_buffer->depth >= CJSON_NESTING_LIMIT)
    {
        return false; /* to deeply nested */
    }
    input_buffer->depth++;

    if (buffer_at_offset(input_buffer)[0] != '[')
    {
        /* not an array */
        goto fail;
    }

    input_buffer->offset++;
    buffer_skip_whitespace(input_buffer);
    if (can_access_at_index(input_buffer, 0) && (buffer_at_offset(input_buffer)[0] == ']'))
    {
        /* empty array */
        goto success;
    }

    /* check if we skipped to the end of the buffer */
    if (cannot_access_at_index(input_buffer, 0))
    {
        input_buffer->offset--;
        goto fail;
    }

    /* step back to character in front of the first element */
    input_buffer->offset--;
    /* loop through the comma separated array elements */
    do
    {
        /* allocate next item */
        cJSON *new_item = cJSON_New_Item(&(input_buffer->hooks));
        if (new_item == NULL)
        {
            goto fail; /* allocation failure */
        }

        /* attach next item to list */
        if (head == NULL)
        {
            /* start the linked list */
            current_item = head = new_item;
        }
        else
        {
            /* add to the end and advance */
            current_item->next = new_item;
            new_item->prev = current_item;
            current_item = new_item;
        }

        /* parse next value */
        input_buffer->offset++;
        buffer_skip_whitespace(input_buffer);
        if (!parse_value(current_item, input_buffer))
        {
            goto fail; /* failed to parse value */
        }
        buffer_skip_whitespace(input_buffer);
    }
    while (can_access_at_index(input_buffer, 0) && (buffer_at_offset(input_buffer)[0] == ','));

    if (cannot_access_at_index(input_buffer, 0) || buffer_at_offset(input_buffer)[0] != ']')
    {
        goto fail; /* expected end of array */
    }

success:
    input_buffer->depth--;

    if (head != NULL) {
        head->prev = current_item;
    }

    item->type = cJSON_Array;
    item->child = head;

    input_buffer->offset++;

    return true;

fail:
    if (head != NULL)
    {
        cJSON_Delete(head);
    }

    return false;
}

/* Render an array to text */
static cJSON_bool print_array(const cJSON * const item, printbuffer * const output_buffer)
{
    unsigned char *output_pointer = NULL;
    size_t length = 0;
    cJSON *current_element = item->child;

    if (output_buffer == NULL)
    {
        return false;
    }

    if (output_buffer->depth >= CJSON_NESTING_LIMIT)
    {
        return false; /* nesting is too deep */
    }

    /* Compose the output array. */
    /* opening square bracket */
    output_pointer = ensure(output_buffer, 1);
    if (output_pointer == NULL)
    {
        return false;
    }

    *output_pointer = '[';
    output_buffer->offset++;
    output_buffer->depth++;

    while (current_element != NULL)
    {
        if (!print_value(current_element, output_buffer))
        {
            return false;
        }
        update_offset(output_buffer);
        if (current_element->next)
        {
            length = (size_t) (output_buffer->format ? 2 : 1);
            output_pointer = ensure(output_buffer, length + 1);
            if (output_pointer == NULL)
            {
                return false;
            }
            *output_pointer++ = ',';
            if(output_buffer->format)
            {
                *output_pointer++ = ' ';
            }
            *output_pointer = '\0';
            output_buffer->offset += length;
        }
        current_element = current_element->next;
    }

    output_pointer = ensure(output_buffer, 2);
    if (output_pointer == NULL)
    {
        return false;
    }
    *output_pointer++ = ']';
    *output_pointer = '\0';
    output_buffer->depth--;

    return true;
}

/* Build an object from the text. */
static cJSON_bool parse_object(cJSON * const item, parse_buffer * const input_buffer)
{
    cJSON *head = NULL; /* linked list head */
    cJSON *current_item = NULL;

    if (input_buffer->depth >= CJSON_NESTING_LIMIT)
    {
        return false; /* to deeply nested */
    }
    input_buffer->depth++;

    if (cannot_access_at_index(input_buffer, 0) || (buffer_at_offset(input_buffer)[0] != '{'))
    {
        goto fail; /* not an object */
    }

    input_buffer->offset++;
    buffer_skip_whitespace(input_buffer);
    if (can_access_at_index(input_buffer, 0) && (buffer_at_offset(input_buffer)[0] == '}'))
    {
        goto success; /* empty object */
    }

    /* check if we skipped to the end of the buffer */
    if (cannot_access_at_index(input_buffer, 0))
    {
        input_buffer->offset--;
        goto fail;
    }

    /* step back to character in front of the first element */
    input_buffer->offset--;
    /* loop through the comma separated array elements */
    do
    {
        /* allocate next item */
        cJSON *new_item = cJSON_New_Item(&(input_buffer->hooks));
        if (new_item == NULL)
        {
            goto fail; /* allocation failure */
        }

        /* attach next item to list */
        if (head == NULL)
        {
            /* start the linked list */
            current_item = head = new_item;
        }
        else
        {
            /* add to the end and advance */
            current_item->next = new_item;
            new_item->prev = current_item;
            current_item = new_item;
        }

        if (cannot_access_at_index(input_buffer, 1))
        {
            goto fail; /* nothing comes after the comma */
        }

        /* parse the name of the child */
        input_buffer->offset++;
        buffer_skip_whitespace(input_buffer);
        if (!parse_string(current_item, input_buffer))
        {
            goto fail; /* failed to parse name */
        }
        buffer_skip_whitespace(input_buffer);

        /* swap valuestring and string, because we parsed the name */
        current_item->string = current_item->valuestring;
        current_item->valuestring = NULL;

        if (cannot_access_at_index(input_buffer, 0) || (buffer_at_offset(input_buffer)[0] != ':'))
        {
            goto fail; /* invalid object */
        }

        /* parse the value */
        input_buffer->offset++;
        buffer_skip_whitespace(input_buffer);
        if (!parse_value(current_item, input_buffer))
        {
            goto fail; /* failed to parse value */
        }
        buffer_skip_whitespace(input_buffer);
    }
    while (can_access_at_index(input_buffer, 0) && (buffer_at_offset(input_buffer)[0] == ','));

    if (cannot_access_at_index(input_buffer, 0) || (buffer_at_offset(input_buffer)[0] != '}'))
    {
        goto fail; /* expected end of object */
    }

success:
    input_buffer->depth--;

    if (head != NULL) {
        head->prev = current_item;
    }

    item->type = cJSON_Object;
    item->child = head;

    input_buffer->offset++;
    return true;

fail:
    if (head != NULL)
    {
        cJSON_Delete(head);
    }

    return false;
}

/* Render an object to text. */
static cJSON_bool print_object(const cJSON * const item, printbuffer * const output_buffer)
{
    unsigned char *output_pointer = NULL;
    size_t length = 0;
    cJSON *current_item = item->child;

    if (output_buffer == NULL)
    {
        return false;
    }

    if (output_buffer->depth >= CJSON_NESTING_LIMIT)
    {
        return false; /* nesting is too deep */
    }

    /* Compose the output: */
    length = (size_t) (output_buffer->format ? 2 : 1); /* fmt: {\n */
    output_pointer = ensure(output_buffer, length + 1);
    if (output_pointer == NULL)
    {
        return false;
    }

    *output_pointer++ = '{';
    output_buffer->depth++;
    if (output_buffer->format)
    {
        *output_pointer++ = '\n';
    }
    output_buffer->offset += length;

    while (current_item)
    {
        if (output_buffer->format)
        {
            size_t i;
            output_pointer = ensure(output_buffer, output_buffer->depth);
            if (output_pointer == NULL)
            {
                return false;
            }
            for (i = 0; i < output_buffer->depth; i++)
            {
                *output_pointer++ = '\t';
            }
            output_buffer->offset += output_buffer->depth;
        }

        /* print key */
        if (!print_string_ptr((unsigned char*)current_item->string, output_buffer))
        {
            return false;
        }
        update_offset(output_buffer);

        length = (size_t) (output_buffer->format ? 2 : 1);
        output_pointer = ensure(output_buffer, length);
        if (output_pointer == NULL)
        {
            return false;
        }
        *output_pointer++ = ':';
        if (output_buffer->format)
        {
            *output_pointer++ = '\t';
        }
        output_buffer->offset += length;

        /* print value */
        if (!print_value(current_item, output_buffer))
        {
            return false;
        }
        update_offset(output_buffer);

        /* print comma if not last */
        length = ((size_t)(output_buffer->format ? 1 : 0) + (size_t)(current_item->next ? 1 : 0));
        output_pointer = ensure(output_buffer, length + 1);
        if (output_pointer == NULL)
        {
            return false;
        }
        if (current_item->next)
        {
            *output_pointer++ = ',';
        }

        if (output_buffer->format)
        {
            *output_pointer++ = '\n';
        }
        *output_pointer = '\0';
        output_buffer->offset += length;

        current_item = current_item->next;
    }

    output_pointer = ensure(output_buffer, output_buffer->format ? (output_buffer->depth + 1) : 2);
    if (output_pointer == NULL)
    {
        return false;
    }
    if (output_buffer->format)
    {
        size_t i;
        for (i = 0; i < (output_buffer->depth - 1); i++)
        {
            *output_pointer++ = '\t';
        }
    }
    *output_pointer++ = '}';
    *output_pointer = '\0';
    output_buffer->depth--;

    return true;
}

/* Get Array size/item / object item. */
CJSON_PUBLIC(int) cJSON_GetArraySize(const cJSON *array)
{
    cJSON *child = NULL;
    size_t size = 0;

    if (array == NULL)
    {
        return 0;
    }

    child = array->child;

    while(child != NULL)
    {
        size++;
        child = child->next;
    }

    /* FIXME: Can overflow here. Cannot be fixed without breaking the API */

    return (int)size;
}

static cJSON* get_array_item(const cJSON *array, size_t index)
{
    cJSON *current_child = NULL;

    if (array == NULL)
    {
        return NULL;
    }

    current_child = array->child;
    while ((current_child != NULL) && (index > 0))
    {
        index--;
        current_child = current_child->next;
    }

    return current_child;
}

CJSON_PUBLIC(cJSON *) cJSON_GetArrayItem(const cJSON *array, int index)
{
    if (index < 0)
    {
        return NULL;
    }

    return get_array_item(array, (size_t)index);
}

static cJSON *get_object_item(const cJSON * const object, const char * const name, const cJSON_bool case_sensitive)
{
    cJSON *current_element = NULL;

    if ((object == NULL) || (name == NULL))
    {
        return NULL;
    }

    current_element = object->child;
    if (case_sensitive)
    {
        while ((current_element != NULL) && (current_element->string != NULL) && (strcmp(name, current_element->string) != 0))
        {
            current_element = current_element->next;
        }
    }
    else
    {
        while ((current_element != NULL) && (case_insensitive_strcmp((const unsigned char*)name, (const unsigned char*)(current_element->string)) != 0))
        {
            current_element = current_element->next;
        }
    }

    if ((current_element == NULL) || (current_element->string == NULL)) {
        return NULL;
    }

    return current_element;
}

CJSON_PUBLIC(cJSON *) cJSON_GetObjectItem(const cJSON * const object, const char * const string)
{
    return get_object_item(object, string, false);
}

CJSON_PUBLIC(cJSON *) cJSON_GetObjectItemCaseSensitive(const cJSON * const object, const char * const string)
{
    return get_object_item(object, string, true);
}

CJSON_PUBLIC(cJSON_bool) cJSON_HasObjectItem(const cJSON *object, const char *string)
{
    return cJSON_GetObjectItem(object, string) ? 1 : 0;
}

/* Utility for array list handling. */
static void suffix_object(cJSON *prev, cJSON *item)
{
    prev->next = item;
    item->prev = prev;
}

/* Utility for handling references. */
static cJSON *create_reference(const cJSON *item, const internal_hooks * const hooks)
{
    cJSON *reference = NULL;
    if (item == NULL)
    {
        return NULL;
    }

    reference = cJSON_New_Item(hooks);
    if (reference == NULL)
    {
        return NULL;
    }

    memcpy(reference, item, sizeof(cJSON));
    reference->string = NULL;
    reference->type |= cJSON_IsReference;
    reference->next = reference->prev = NULL;
    return reference;
}

static cJSON_bool add_item_to_array(cJSON *array, cJSON *item)
{
    cJSON *child = NULL;

    if ((item == NULL) || (array == NULL) || (array == item))
    {
        return false;
    }

    child = array->child;
    /*
     * To find the last item in array quickly, we use prev in array
     */
    if (child == NULL)
    {
        /* list is empty, start new one */
        array->child = item;
        item->prev = item;
        item->next = NULL;
    }
    else
    {
        /* append to the end */
        if (child->prev)
        {
            suffix_object(child->prev, item);
            array->child->prev = item;
        }
    }

    return true;
}

/* Add item to array/object. */
CJSON_PUBLIC(cJSON_bool) cJSON_AddItemToArray(cJSON *array, cJSON *item)
{
    return add_item_to_array(array, item);
}

#if defined(__clang__) || (defined(__GNUC__)  && ((__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ > 5))))
    #pragma GCC diagnostic push
#endif
#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wcast-qual"
#endif
/* helper function to cast away const */
static void* cast_away_const(const void* string)
{
    return (void*)string;
}
#if defined(__clang__) || (defined(__GNUC__)  && ((__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ > 5))))
    #pragma GCC diagnostic pop
#endif


static cJSON_bool add_item_to_object(cJSON * const object, const char * const string, cJSON * const item, const internal_hooks * const hooks, const cJSON_bool constant_key)
{
    char *new_key = NULL;
    int new_type = cJSON_Invalid;

    if ((object == NULL) || (string == NULL) || (item == NULL) || (object == item))
    {
        return false;
    }

    if (constant_key)
    {
        new_key = (char*)cast_away_const(string);
        new_type = item->type | cJSON_StringIsConst;
    }
    else
    {
        new_key = (char*)cJSON_strdup((const unsigned char*)string, hooks);
        if (new_key == NULL)
        {
            return false;
        }

        new_type = item->type & ~cJSON_StringIsConst;
    }

    if (!(item->type & cJSON_StringIsConst) && (item->string != NULL))
    {
        hooks->deallocate(item->string);
    }

    item->string = new_key;
    item->type = new_type;

    return add_item_to_array(object, item);
}

CJSON_PUBLIC(cJSON_bool) cJSON_AddItemToObject(cJSON *object, const char *string, cJSON *item)
{
    return add_item_to_object(object, string, item, &global_hooks, false);
}

/* Add an item to an object with constant string as key */
CJSON_PUBLIC(cJSON_bool) cJSON_AddItemToObjectCS(cJSON *object, const char *string, cJSON *item)
{
    return add_item_to_object(object, string, item, &global_hooks, true);
}

CJSON_PUBLIC(cJSON_bool) cJSON_AddItemReferenceToArray(cJSON *array, cJSON *item)
{
    if (array == NULL)
    {
        return false;
    }

    return add_item_to_array(array, create_reference(item, &global_hooks));
}

CJSON_PUBLIC(cJSON_bool) cJSON_AddItemReferenceToObject(cJSON *object, const char *string, cJSON *item)
{
    if ((object == NULL) || (string == NULL))
    {
        return false;
    }

    return add_item_to_object(object, string, create_reference(item, &global_hooks), &global_hooks, false);
}

CJSON_PUBLIC(cJSON*) cJSON_AddNullToObject(cJSON * const object, const char * const name)
{
    cJSON *null = cJSON_CreateNull();
    if (add_item_to_object(object, name, null, &global_hooks, false))
    {
        return null;
    }

    cJSON_Delete(null);
    return NULL;
}

CJSON_PUBLIC(cJSON*) cJSON_AddTrueToObject(cJSON * const object, const char * const name)
{
    cJSON *true_item = cJSON_CreateTrue();
    if (add_item_to_object(object, name, true_item, &global_hooks, false))
    {
        return true_item;
    }

    cJSON_Delete(true_item);
    return NULL;
}

CJSON_PUBLIC(cJSON*) cJSON_AddFalseToObject(cJSON * const object, const char * const name)
{
    cJSON *false_item = cJSON_CreateFalse();
    if (add_item_to_object(object, name, false_item, &global_hooks, false))
    {
        return false_item;
    }

    cJSON_Delete(false_item);
    return NULL;
}

CJSON_PUBLIC(cJSON*) cJSON_AddBoolToObject(cJSON * const object, const char * const name, const cJSON_bool boolean)
{
    cJSON *bool_item = cJSON_CreateBool(boolean);
    if (add_item_to_object(object, name, bool_item, &global_hooks, false))
    {
        return bool_item;
    }

    cJSON_Delete(bool_item);
    return NULL;
}

CJSON_PUBLIC(cJSON*) cJSON_AddNumberToObject(cJSON * const object, const char * const name, const double number)
{
    cJSON *number_item = cJSON_CreateNumber(number);
    if (add_item_to_object(object, name, number_item, &global_hooks, false))
    {
        return number_item;
    }

    cJSON_Delete(number_item);
    return NULL;
}

CJSON_PUBLIC(cJSON*) cJSON_AddStringToObject(cJSON * const object, const char * const name, const char * const string)
{
    cJSON *string_item = cJSON_CreateString(string);
    if (add_item_to_object(object, name, string_item, &global_hooks, false))
    {
        return string_item;
    }

    cJSON_Delete(string_item);
    return NULL;
}

CJSON_PUBLIC(cJSON*) cJSON_AddRawToObject(cJSON * const object, const char * const name, const char * const raw)
{
    cJSON *raw_item = cJSON_CreateRaw(raw);
    if (add_item_to_object(object, name, raw_item, &global_hooks, false))
    {
        return raw_item;
    }

    cJSON_Delete(raw_item);
    return NULL;
}

CJSON_PUBLIC(cJSON*) cJSON_AddObjectToObject(cJSON * const object, const char * const name)
{
    cJSON *object_item = cJSON_CreateObject();
    if (add_item_to_object(object, name, object_item, &global_hooks, false))
    {
        return object_item;
    }

    cJSON_Delete(object_item);
    return NULL;
}

CJSON_PUBLIC(cJSON*) cJSON_AddArrayToObject(cJSON * const object, const char * const name)
{
    cJSON *array = cJSON_CreateArray();
    if (add_item_to_object(object, name, array, &global_hooks, false))
    {
        return array;
    }

    cJSON_Delete(array);
    return NULL;
}

CJSON_PUBLIC(cJSON *) cJSON_DetachItemViaPointer(cJSON *parent, cJSON * const item)
{
    if ((parent == NULL) || (item == NULL) || (item != parent->child && item->prev == NULL))
    {
        return NULL;
    }

    if (item != parent->child)
    {
        /* not the first element */
        item->prev->next = item->next;
    }
    if (item->next != NULL)
    {
        /* not the last element */
        item->next->prev = item->prev;
    }

    if (item == parent->child)
    {
        /* first element */
        parent->child = item->next;
    }
    else if (item->next == NULL)
    {
        /* last element */
        parent->child->prev = item->prev;
    }

    /* make sure the detached item doesn't point anywhere anymore */
    item->prev = NULL;
    item->next = NULL;

    return item;
}

CJSON_PUBLIC(cJSON *) cJSON_DetachItemFromArray(cJSON *array, int which)
{
    if (which < 0)
    {
        return NULL;
    }

    return cJSON_DetachItemViaPointer(array, get_array_item(array, (size_t)which));
}

CJSON_PUBLIC(void) cJSON_DeleteItemFromArray(cJSON *array, int which)
{
    cJSON_Delete(cJSON_DetachItemFromArray(array, which));
}

CJSON_PUBLIC(cJSON *) cJSON_DetachItemFromObject(cJSON *object, const char *string)
{
    cJSON *to_detach = cJSON_GetObjectItem(object, string);

    return cJSON_DetachItemViaPointer(object, to_detach);
}

CJSON_PUBLIC(cJSON *) cJSON_DetachItemFromObjectCaseSensitive(cJSON *object, const char *string)
{
    cJSON *to_detach = cJSON_GetObjectItemCaseSensitive(object, string);

    return cJSON_DetachItemViaPointer(object, to_detach);
}

CJSON_PUBLIC(void) cJSON_DeleteItemFromObject(cJSON *object, const char *string)
{
    cJSON_Delete(cJSON_DetachItemFromObject(object, string));
}

CJSON_PUBLIC(void) cJSON_DeleteItemFromObjectCaseSensitive(cJSON *object, const char *string)
{
    cJSON_Delete(cJSON_DetachItemFromObjectCaseSensitive(object, string));
}

/* Replace array/object items with new ones. */
CJSON_PUBLIC(cJSON_bool) cJSON_InsertItemInArray(cJSON *array, int which, cJSON *newitem)
{
    cJSON *after_inserted = NULL;

    if (which < 0 || newitem == NULL)
    {
        return false;
    }

    after_inserted = get_array_item(array, (size_t)which);
    if (after_inserted == NULL)
    {
        return add_item_to_array(array, newitem);
    }

    if (after_inserted != array->child && after_inserted->prev == NULL) {
        /* return false if after_inserted is a corrupted array item */
        return false;
    }

    newitem->next = after_inserted;
    newitem->prev = after_inserted->prev;
    after_inserted->prev = newitem;
    if (after_inserted == array->child)
    {
        array->child = newitem;
    }
    else
    {
        newitem->prev->next = newitem;
    }
    return true;
}

CJSON_PUBLIC(cJSON_bool) cJSON_ReplaceItemViaPointer(cJSON * const parent, cJSON * const item, cJSON * replacement)
{
    if ((parent == NULL) || (parent->child == NULL) || (replacement == NULL) || (item == NULL))
    {
        return false;
    }

    if (replacement == item)
    {
        return true;
    }

    replacement->next = item->next;
    replacement->prev = item->prev;

    if (replacement->next != NULL)
    {
        replacement->next->prev = replacement;
    }
    if (parent->child == item)
    {
        if (parent->child->prev == parent->child)
        {
            replacement->prev = replacement;
        }
        parent->child = replacement;
    }
    else
    {   /*
         * To find the last item in array quickly, we use prev in array.
         * We can't modify the last item's next pointer where this item was the parent's child
         */
        if (replacement->prev != NULL)
        {
            replacement->prev->next = replacement;
        }
        if (replacement->next == NULL)
        {
            parent->child->prev = replacement;
        }
    }

    item->next = NULL;
    item->prev = NULL;
    cJSON_Delete(item);

    return true;
}

CJSON_PUBLIC(cJSON_bool) cJSON_ReplaceItemInArray(cJSON *array, int which, cJSON *newitem)
{
    if (which < 0)
    {
        return false;
    }

    return cJSON_ReplaceItemViaPointer(array, get_array_item(array, (size_t)which), newitem);
}

static cJSON_bool replace_item_in_object(cJSON *object, const char *string, cJSON *replacement, cJSON_bool case_sensitive)
{
    if ((replacement == NULL) || (string == NULL))
    {
        return false;
    }

    /* replace the name in the replacement */
    if (!(replacement->type & cJSON_StringIsConst) && (replacement->string != NULL))
    {
        cJSON_free(replacement->string);
    }
    replacement->string = (char*)cJSON_strdup((const unsigned char*)string, &global_hooks);
    if (replacement->string == NULL)
    {
        return false;
    }

    replacement->type &= ~cJSON_StringIsConst;

    return cJSON_ReplaceItemViaPointer(object, get_object_item(object, string, case_sensitive), replacement);
}

CJSON_PUBLIC(cJSON_bool) cJSON_ReplaceItemInObject(cJSON *object, const char *string, cJSON *newitem)
{
    return replace_item_in_object(object, string, newitem, false);
}

CJSON_PUBLIC(cJSON_bool) cJSON_ReplaceItemInObjectCaseSensitive(cJSON *object, const char *string, cJSON *newitem)
{
    return replace_item_in_object(object, string, newitem, true);
}

/* Create basic types: */
CJSON_PUBLIC(cJSON *) cJSON_CreateNull(void)
{
    cJSON *item = cJSON_New_Item(&global_hooks);
    if(item)
    {
        item->type = cJSON_NULL;
    }

    return item;
}

CJSON_PUBLIC(cJSON *) cJSON_CreateTrue(void)
{
    cJSON *item = cJSON_New_Item(&global_hooks);
    if(item)
    {
        item->type = cJSON_True;
    }

    return item;
}

CJSON_PUBLIC(cJSON *) cJSON_CreateFalse(void)
{
    cJSON *item = cJSON_New_Item(&global_hooks);
    if(item)
    {
        item->type = cJSON_False;
    }

    return item;
}

CJSON_PUBLIC(cJSON *) cJSON_CreateBool(cJSON_bool boolean)
{
    cJSON *item = cJSON_New_Item(&global_hooks);
    if(item)
    {
        item->type = boolean ? cJSON_True : cJSON_False;
    }

    return item;
}

CJSON_PUBLIC(cJSON *) cJSON_CreateNumber(double num)
{
    cJSON *item = cJSON_New_Item(&global_hooks);
    if(item)
    {
        item->type = cJSON_Number;
        item->valuedouble = num;

        /* use saturation in case of overflow */
        if (num >= INT_MAX)
        {
            item->valueint = INT_MAX;
        }
        else if (num <= (double)INT_MIN)
        {
            item->valueint = INT_MIN;
        }
        else
        {
            item->valueint = (int)num;
        }
    }

    return item;
}

CJSON_PUBLIC(cJSON *) cJSON_CreateString(const char *string)
{
    cJSON *item = cJSON_New_Item(&global_hooks);
    if(item)
    {
        item->type = cJSON_String;
        item->valuestring = (char*)cJSON_strdup((const unsigned char*)string, &global_hooks);
        if(!item->valuestring)
        {
            cJSON_Delete(item);
            return NULL;
        }
    }

    return item;
}

CJSON_PUBLIC(cJSON *) cJSON_CreateStringReference(const char *string)
{
    cJSON *item = cJSON_New_Item(&global_hooks);
    if (item != NULL)
    {
        item->type = cJSON_String | cJSON_IsReference;
        item->valuestring = (char*)cast_away_const(string);
    }

    return item;
}

CJSON_PUBLIC(cJSON *) cJSON_CreateObjectReference(const cJSON *child)
{
    cJSON *item = cJSON_New_Item(&global_hooks);
    if (item != NULL) {
        item->type = cJSON_Object | cJSON_IsReference;
        item->child = (cJSON*)cast_away_const(child);
    }

    return item;
}

CJSON_PUBLIC(cJSON *) cJSON_CreateArrayReference(const cJSON *child) {
    cJSON *item = cJSON_New_Item(&global_hooks);
    if (item != NULL) {
        item->type = cJSON_Array | cJSON_IsReference;
        item->child = (cJSON*)cast_away_const(child);
    }

    return item;
}

CJSON_PUBLIC(cJSON *) cJSON_CreateRaw(const char *raw)
{
    cJSON *item = cJSON_New_Item(&global_hooks);
    if(item)
    {
        item->type = cJSON_Raw;
        item->valuestring = (char*)cJSON_strdup((const unsigned char*)raw, &global_hooks);
        if(!item->valuestring)
        {
            cJSON_Delete(item);
            return NULL;
        }
    }

    return item;
}

CJSON_PUBLIC(cJSON *) cJSON_CreateArray(void)
{
    cJSON *item = cJSON_New_Item(&global_hooks);
    if(item)
    {
        item->type=cJSON_Array;
    }

    return item;
}

CJSON_PUBLIC(cJSON *) cJSON_CreateObject(void)
{
    cJSON *item = cJSON_New_Item(&global_hooks);
    if (item)
    {
        item->type = cJSON_Object;
    }

    return item;
}

/* Create Arrays: */
CJSON_PUBLIC(cJSON *) cJSON_CreateIntArray(const int *numbers, int count)
{
    size_t i = 0;
    cJSON *n = NULL;
    cJSON *p = NULL;
    cJSON *a = NULL;

    if ((count < 0) || (numbers == NULL))
    {
        return NULL;
    }

    a = cJSON_CreateArray();

    for(i = 0; a && (i < (size_t)count); i++)
    {
        n = cJSON_CreateNumber(numbers[i]);
        if (!n)
        {
            cJSON_Delete(a);
            return NULL;
        }
        if(!i)
        {
            a->child = n;
        }
        else
        {
            suffix_object(p, n);
        }
        p = n;
    }

    if (a && a->child) {
        a->child->prev = n;
    }

    return a;
}

CJSON_PUBLIC(cJSON *) cJSON_CreateFloatArray(const float *numbers, int count)
{
    size_t i = 0;
    cJSON *n = NULL;
    cJSON *p = NULL;
    cJSON *a = NULL;

    if ((count < 0) || (numbers == NULL))
    {
        return NULL;
    }

    a = cJSON_CreateArray();

    for(i = 0; a && (i < (size_t)count); i++)
    {
        n = cJSON_CreateNumber((double)numbers[i]);
        if(!n)
        {
            cJSON_Delete(a);
            return NULL;
        }
        if(!i)
        {
            a->child = n;
        }
        else
        {
            suffix_object(p, n);
        }
        p = n;
    }

    if (a && a->child) {
        a->child->prev = n;
    }

    return a;
}

CJSON_PUBLIC(cJSON *) cJSON_CreateDoubleArray(const double *numbers, int count)
{
    size_t i = 0;
    cJSON *n = NULL;
    cJSON *p = NULL;
    cJSON *a = NULL;

    if ((count < 0) || (numbers == NULL))
    {
        return NULL;
    }

    a = cJSON_CreateArray();

    for(i = 0; a && (i < (size_t)count); i++)
    {
        n = cJSON_CreateNumber(numbers[i]);
        if(!n)
        {
            cJSON_Delete(a);
            return NULL;
        }
        if(!i)
        {
            a->child = n;
        }
        else
        {
            suffix_object(p, n);
        }
        p = n;
    }

    if (a && a->child) {
        a->child->prev = n;
    }

    return a;
}

CJSON_PUBLIC(cJSON *) cJSON_CreateStringArray(const char *const *strings, int count)
{
    size_t i = 0;
    cJSON *n = NULL;
    cJSON *p = NULL;
    cJSON *a = NULL;

    if ((count < 0) || (strings == NULL))
    {
        return NULL;
    }

    a = cJSON_CreateArray();

    for (i = 0; a && (i < (size_t)count); i++)
    {
        n = cJSON_CreateString(strings[i]);
        if(!n)
        {
            cJSON_Delete(a);
            return NULL;
        }
        if(!i)
        {
            a->child = n;
        }
        else
        {
            suffix_object(p,n);
        }
        p = n;
    }

    if (a && a->child) {
        a->child->prev = n;
    }

    return a;
}

/* Duplication */
cJSON * cJSON_Duplicate_rec(const cJSON *item, size_t depth, cJSON_bool recurse);

CJSON_PUBLIC(cJSON *) cJSON_Duplicate(const cJSON *item, cJSON_bool recurse)
{
    return cJSON_Duplicate_rec(item, 0, recurse );
}

cJSON * cJSON_Duplicate_rec(const cJSON *item, size_t depth, cJSON_bool recurse)
{
    cJSON *newitem = NULL;
    cJSON *child = NULL;
    cJSON *next = NULL;
    cJSON *newchild = NULL;

    /* Bail on bad ptr */
    if (!item)
    {
        goto fail;
    }
    /* Create new item */
    newitem = cJSON_New_Item(&global_hooks);
    if (!newitem)
    {
        goto fail;
    }
    /* Copy over all vars */
    newitem->type = item->type & (~cJSON_IsReference);
    newitem->valueint = item->valueint;
    newitem->valuedouble = item->valuedouble;
    if (item->valuestring)
    {
        newitem->valuestring = (char*)cJSON_strdup((unsigned char*)item->valuestring, &global_hooks);
        if (!newitem->valuestring)
        {
            goto fail;
        }
    }
    if (item->string)
    {
        newitem->string = (item->type&cJSON_StringIsConst) ? item->string : (char*)cJSON_strdup((unsigned char*)item->string, &global_hooks);
        if (!newitem->string)
        {
            goto fail;
        }
    }
    /* If non-recursive, then we're done! */
    if (!recurse)
    {
        return newitem;
    }
    /* Walk the ->next chain for the child. */
    child = item->child;
    while (child != NULL)
    {
        if(depth >= CJSON_CIRCULAR_LIMIT) {
            goto fail;
        }
        newchild = cJSON_Duplicate_rec(child, depth + 1, true); /* Duplicate (with recurse) each item in the ->next chain */
        if (!newchild)
        {
            goto fail;
        }
        if (next != NULL)
        {
            /* If newitem->child already set, then crosswire ->prev and ->next and move on */
            next->next = newchild;
            newchild->prev = next;
            next = newchild;
        }
        else
        {
            /* Set newitem->child and move to it */
            newitem->child = newchild;
            next = newchild;
        }
        child = child->next;
    }
    if (newitem && newitem->child)
    {
        newitem->child->prev = newchild;
    }

    return newitem;

fail:
    if (newitem != NULL)
    {
        cJSON_Delete(newitem);
    }

    return NULL;
}

static void skip_oneline_comment(char **input)
{
    *input += static_strlen("//");

    for (; (*input)[0] != '\0'; ++(*input))
    {
        if ((*input)[0] == '\n') {
            *input += static_strlen("\n");
            return;
        }
    }
}

static void skip_multiline_comment(char **input)
{
    *input += static_strlen("/*");

    for (; (*input)[0] != '\0'; ++(*input))
    {
        if (((*input)[0] == '*') && ((*input)[1] == '/'))
        {
            *input += static_strlen("*/");
            return;
        }
    }
}

static void minify_string(char **input, char **output) {
    (*output)[0] = (*input)[0];
    *input += static_strlen("\"");
    *output += static_strlen("\"");


    for (; (*input)[0] != '\0'; (void)++(*input), ++(*output)) {
        (*output)[0] = (*input)[0];

        if ((*input)[0] == '\"') {
            (*output)[0] = '\"';
            *input += static_strlen("\"");
            *output += static_strlen("\"");
            return;
        } else if (((*input)[0] == '\\') && ((*input)[1] == '\"')) {
            (*output)[1] = (*input)[1];
            *input += static_strlen("\"");
            *output += static_strlen("\"");
        }
    }
}

CJSON_PUBLIC(void) cJSON_Minify(char *json)
{
    char *into = json;

    if (json == NULL)
    {
        return;
    }

    while (json[0] != '\0')
    {
        switch (json[0])
        {
            case ' ':
            case '\t':
            case '\r':
            case '\n':
                json++;
                break;

            case '/':
                if (json[1] == '/')
                {
                    skip_oneline_comment(&json);
                }
                else if (json[1] == '*')
                {
                    skip_multiline_comment(&json);
                } else {
                    json++;
                }
                break;

            case '\"':
                minify_string(&json, (char**)&into);
                break;

            default:
                into[0] = json[0];
                json++;
                into++;
        }
    }

    /* and null-terminate. */
    *into = '\0';
}

CJSON_PUBLIC(cJSON_bool) cJSON_IsInvalid(const cJSON * const item)
{
    if (item == NULL)
    {
        return false;
    }

    return (item->type & 0xFF) == cJSON_Invalid;
}

CJSON_PUBLIC(cJSON_bool) cJSON_IsFalse(const cJSON * const item)
{
    if (item == NULL)
    {
        return false;
    }

    return (item->type & 0xFF) == cJSON_False;
}

CJSON_PUBLIC(cJSON_bool) cJSON_IsTrue(const cJSON * const item)
{
    if (item == NULL)
    {
        return false;
    }

    return (item->type & 0xff) == cJSON_True;
}


CJSON_PUBLIC(cJSON_bool) cJSON_IsBool(const cJSON * const item)
{
    if (item == NULL)
    {
        return false;
    }

    return (item->type & (cJSON_True | cJSON_False)) != 0;
}
CJSON_PUBLIC(cJSON_bool) cJSON_IsNull(const cJSON * const item)
{
    if (item == NULL)
    {
        return false;
    }

    return (item->type & 0xFF) == cJSON_NULL;
}

CJSON_PUBLIC(cJSON_bool) cJSON_IsNumber(const cJSON * const item)
{
    if (item == NULL)
    {
        return false;
    }

    return (item->type & 0xFF) == cJSON_Number;
}

CJSON_PUBLIC(cJSON_bool) cJSON_IsString(const cJSON * const item)
{
    if (item == NULL)
    {
        return false;
    }

    return (item->type & 0xFF) == cJSON_String;
}

CJSON_PUBLIC(cJSON_bool) cJSON_IsArray(const cJSON * const item)
{
    if (item == NULL)
    {
        return false;
    }

    return (item->type & 0xFF) == cJSON_Array;
}

CJSON_PUBLIC(cJSON_bool) cJSON_IsObject(const cJSON * const item)
{
    if (item == NULL)
    {
        return false;
    }

    return (item->type & 0xFF) == cJSON_Object;
}

CJSON_PUBLIC(cJSON_bool) cJSON_IsRaw(const cJSON * const item)
{
    if (item == NULL)
    {
        return false;
    }

    return (item->type & 0xFF) == cJSON_Raw;
}

CJSON_PUBLIC(cJSON_bool) cJSON_Compare(const cJSON * const a, const cJSON * const b, const cJSON_bool case_sensitive)
{
    if ((a == NULL) || (b == NULL) || ((a->type & 0xFF) != (b->type & 0xFF)))
    {
        return false;
    }

    /* check if type is valid */
    switch (a->type & 0xFF)
    {
        case cJSON_False:
        case cJSON_True:
        case cJSON_NULL:
        case cJSON_Number:
        case cJSON_String:
        case cJSON_Raw:
        case cJSON_Array:
        case cJSON_Object:
            break;

        default:
            return false;
    }

    /* identical objects are equal */
    if (a == b)
    {
        return true;
    }

    switch (a->type & 0xFF)
    {
        /* in these cases and equal type is enough */
        case cJSON_False:
        case cJSON_True:
        case cJSON_NULL:
            return true;

        case cJSON_Number:
            if (compare_double(a->valuedouble, b->valuedouble))
            {
                return true;
            }
            return false;

        case cJSON_String:
        case cJSON_Raw:
            if ((a->valuestring == NULL) || (b->valuestring == NULL))
            {
                return false;
            }
            if (strcmp(a->valuestring, b->valuestring) == 0)
            {
                return true;
            }

            return false;

        case cJSON_Array:
        {
            cJSON *a_element = a->child;
            cJSON *b_element = b->child;

            for (; (a_element != NULL) && (b_element != NULL);)
            {
                if (!cJSON_Compare(a_element, b_element, case_sensitive))
                {
                    return false;
                }

                a_element = a_element->next;
                b_element = b_element->next;
            }

            /* one of the arrays is longer than the other */
            if (a_element != b_element) {
                return false;
            }

            return true;
        }

        case cJSON_Object:
        {
            cJSON *a_element = NULL;
            cJSON *b_element = NULL;
            cJSON_ArrayForEach(a_element, a)
            {
                /* TODO This has O(n^2) runtime, which is horrible! */
                b_element = get_object_item(b, a_element->string, case_sensitive);
                if (b_element == NULL)
                {
                    return false;
                }

                if (!cJSON_Compare(a_element, b_element, case_sensitive))
                {
                    return false;
                }
            }

            /* doing this twice, once on a and b to prevent true comparison if a subset of b
             * TODO: Do this the proper way, this is just a fix for now */
            cJSON_ArrayForEach(b_element, b)
            {
                a_element = get_object_item(a, b_element->string, case_sensitive);
                if (a_element == NULL)
                {
                    return false;
                }

                if (!cJSON_Compare(b_element, a_element, case_sensitive))
                {
                    return false;
                }
            }

            return true;
        }

        default:
            return false;
    }
}

CJSON_PUBLIC(void *) cJSON_malloc(size_t size)
{
    return global_hooks.allocate(size);
}

CJSON_PUBLIC(void) cJSON_free(void *object)
{
    global_hooks.deallocate(object);
    object = NULL;
}
// ------------------------------------------------------------------------------------------


// https://github.com/troydhanson/uthash/blob/master/src/uthash.h
// ------------------------------------------------------------------------------------------
/*
Copyright (c) 2003-2026, Troy D. Hanson  https://troydhanson.github.io/uthash/
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef UTHASH_H
#define UTHASH_H

#define UTHASH_VERSION 2.4.0

#include <string.h>   /* memcmp, memset, strlen */
#include <stddef.h>   /* ptrdiff_t */
#include <stdlib.h>   /* exit */

#if defined(HASH_NO_STDINT) && HASH_NO_STDINT
/* The user doesn't have <stdint.h>, and must figure out their own way
   to provide definitions for uint8_t and uint32_t. */
#else
#include <stdint.h>   /* uint8_t, uint32_t */
#endif

/* These macros use decltype or the earlier __typeof GNU extension.
   As decltype is only available in newer compilers (VS2010 or gcc 4.3+
   when compiling c++ source) this code uses whatever method is needed
   or, for VS2008 where neither is available, uses casting workarounds. */
#if !defined(DECLTYPE) && !defined(NO_DECLTYPE)
#if defined(_MSC_VER)   /* MS compiler */
#if _MSC_VER >= 1600 && defined(__cplusplus)  /* VS2010 or newer in C++ mode */
#define DECLTYPE(x) (decltype(x))
#else                   /* VS2008 or older (or VS2010 in C mode) */
#define NO_DECLTYPE
#endif
#elif defined(__MCST__)  /* Elbrus C Compiler */
#define DECLTYPE(x) (__typeof(x))
#elif defined(__BORLANDC__) || defined(__ICCARM__) || defined(__LCC__) || defined(__WATCOMC__)
#define NO_DECLTYPE
#else                   /* GNU, Sun and other compilers */
#define DECLTYPE(x) (__typeof(x))
#endif
#endif

#ifdef NO_DECLTYPE
#define DECLTYPE(x)
#define DECLTYPE_ASSIGN(dst,src)                                                 \
do {                                                                             \
  char **_da_dst = (char**)(&(dst));                                             \
  *_da_dst = (char*)(src);                                                       \
} while (0)
#else
#define DECLTYPE_ASSIGN(dst,src)                                                 \
do {                                                                             \
  (dst) = DECLTYPE(dst)(src);                                                    \
} while (0)
#endif

#ifndef uthash_malloc
#define uthash_malloc(sz) malloc(sz)      /* malloc fcn                      */
#endif
#ifndef uthash_free
#define uthash_free(ptr,sz) free(ptr)     /* free fcn                        */
#endif
#ifndef uthash_bzero
#define uthash_bzero(a,n) memset(a,'\0',n)
#endif
#ifndef uthash_strlen
#define uthash_strlen(s) strlen(s)
#endif

#ifndef HASH_FUNCTION
#define HASH_FUNCTION(keyptr,keylen,hashv) HASH_JEN(keyptr, keylen, hashv)
#endif

#ifndef HASH_KEYCMP
#define HASH_KEYCMP(a,b,n) memcmp(a,b,n)
#endif

#ifndef uthash_noexpand_fyi
#define uthash_noexpand_fyi(tbl)          /* can be defined to log noexpand  */
#endif
#ifndef uthash_expand_fyi
#define uthash_expand_fyi(tbl)            /* can be defined to log expands   */
#endif

#ifndef HASH_NONFATAL_OOM
#define HASH_NONFATAL_OOM 0
#endif

#if HASH_NONFATAL_OOM
/* malloc failures can be recovered from */

#ifndef uthash_nonfatal_oom
#define uthash_nonfatal_oom(obj) do {} while (0)    /* non-fatal OOM error */
#endif

#define HASH_RECORD_OOM(oomed) do { (oomed) = 1; } while (0)
#define IF_HASH_NONFATAL_OOM(x) x

#else
/* malloc failures result in lost memory, hash tables are unusable */

#ifndef uthash_fatal
#define uthash_fatal(msg) exit(-1)        /* fatal OOM error */
#endif

#define HASH_RECORD_OOM(oomed) uthash_fatal("out of memory")
#define IF_HASH_NONFATAL_OOM(x)

#endif

/* initial number of buckets */
#define HASH_INITIAL_NUM_BUCKETS 32U     /* initial number of buckets        */
#define HASH_INITIAL_NUM_BUCKETS_LOG2 5U /* lg2 of initial number of buckets */
#define HASH_BKT_CAPACITY_THRESH 10U     /* expand when bucket count reaches */

/* calculate the element whose hash handle address is hhp */
#define ELMT_FROM_HH(tbl,hhp) ((void*)(((char*)(hhp)) - ((tbl)->hho)))
/* calculate the hash handle from element address elp */
#define HH_FROM_ELMT(tbl,elp) ((UT_hash_handle*)(void*)(((char*)(elp)) + ((tbl)->hho)))

#define HASH_ROLLBACK_BKT(hh, head, itemptrhh)                                   \
do {                                                                             \
  struct UT_hash_handle *_hd_hh_item = (itemptrhh);                              \
  unsigned _hd_bkt;                                                              \
  HASH_TO_BKT(_hd_hh_item->hashv, (head)->hh.tbl->num_buckets, _hd_bkt);         \
  (head)->hh.tbl->buckets[_hd_bkt].count++;                                      \
  _hd_hh_item->hh_next = NULL;                                                   \
  _hd_hh_item->hh_prev = NULL;                                                   \
} while (0)

#define HASH_VALUE(keyptr,keylen,hashv)                                          \
do {                                                                             \
  HASH_FUNCTION(keyptr, keylen, hashv);                                          \
} while (0)

#define HASH_FIND_BYHASHVALUE(hh,head,keyptr,keylen,hashval,out)                 \
do {                                                                             \
  (out) = NULL;                                                                  \
  if (head) {                                                                    \
    unsigned _hf_bkt;                                                            \
    HASH_TO_BKT(hashval, (head)->hh.tbl->num_buckets, _hf_bkt);                  \
    if (HASH_BLOOM_TEST((head)->hh.tbl, hashval)) {                              \
      HASH_FIND_IN_BKT((head)->hh.tbl, hh, (head)->hh.tbl->buckets[ _hf_bkt ], keyptr, keylen, hashval, out); \
    }                                                                            \
  }                                                                              \
} while (0)

#define HASH_FIND(hh,head,keyptr,keylen,out)                                     \
do {                                                                             \
  (out) = NULL;                                                                  \
  if (head) {                                                                    \
    unsigned _hf_hashv;                                                          \
    HASH_VALUE(keyptr, keylen, _hf_hashv);                                       \
    HASH_FIND_BYHASHVALUE(hh, head, keyptr, keylen, _hf_hashv, out);             \
  }                                                                              \
} while (0)

#ifdef HASH_BLOOM
#define HASH_BLOOM_BITLEN (1UL << HASH_BLOOM)
#define HASH_BLOOM_BYTELEN (HASH_BLOOM_BITLEN/8UL) + (((HASH_BLOOM_BITLEN%8UL)!=0UL) ? 1UL : 0UL)
#define HASH_BLOOM_MAKE(tbl,oomed)                                               \
do {                                                                             \
  (tbl)->bloom_nbits = HASH_BLOOM;                                               \
  (tbl)->bloom_bv = (uint8_t*)uthash_malloc(HASH_BLOOM_BYTELEN);                 \
  if (!(tbl)->bloom_bv) {                                                        \
    HASH_RECORD_OOM(oomed);                                                      \
  } else {                                                                       \
    uthash_bzero((tbl)->bloom_bv, HASH_BLOOM_BYTELEN);                           \
    (tbl)->bloom_sig = HASH_BLOOM_SIGNATURE;                                     \
  }                                                                              \
} while (0)

#define HASH_BLOOM_FREE(tbl)                                                     \
do {                                                                             \
  uthash_free((tbl)->bloom_bv, HASH_BLOOM_BYTELEN);                              \
} while (0)

#define HASH_BLOOM_BITSET(bv,idx) (bv[(idx)/8U] |= (1U << ((idx)%8U)))
#define HASH_BLOOM_BITTEST(bv,idx) ((bv[(idx)/8U] & (1U << ((idx)%8U))) != 0)

#define HASH_BLOOM_ADD(tbl,hashv)                                                \
  HASH_BLOOM_BITSET((tbl)->bloom_bv, ((hashv) & (uint32_t)((1UL << (tbl)->bloom_nbits) - 1U)))

#define HASH_BLOOM_TEST(tbl,hashv)                                               \
  HASH_BLOOM_BITTEST((tbl)->bloom_bv, ((hashv) & (uint32_t)((1UL << (tbl)->bloom_nbits) - 1U)))

#else
#define HASH_BLOOM_MAKE(tbl,oomed)
#define HASH_BLOOM_FREE(tbl)
#define HASH_BLOOM_ADD(tbl,hashv)
#define HASH_BLOOM_TEST(tbl,hashv) 1
#define HASH_BLOOM_BYTELEN 0U
#endif

#define HASH_MAKE_TABLE(hh,head,oomed)                                           \
do {                                                                             \
  (head)->hh.tbl = (UT_hash_table*)uthash_malloc(sizeof(UT_hash_table));         \
  if (!(head)->hh.tbl) {                                                         \
    HASH_RECORD_OOM(oomed);                                                      \
  } else {                                                                       \
    uthash_bzero((head)->hh.tbl, sizeof(UT_hash_table));                         \
    (head)->hh.tbl->tail = &((head)->hh);                                        \
    (head)->hh.tbl->num_buckets = HASH_INITIAL_NUM_BUCKETS;                      \
    (head)->hh.tbl->log2_num_buckets = HASH_INITIAL_NUM_BUCKETS_LOG2;            \
    (head)->hh.tbl->hho = (char*)(&(head)->hh) - (char*)(head);                  \
    (head)->hh.tbl->buckets = (UT_hash_bucket*)uthash_malloc(                    \
        HASH_INITIAL_NUM_BUCKETS * sizeof(struct UT_hash_bucket));               \
    (head)->hh.tbl->signature = HASH_SIGNATURE;                                  \
    if (!(head)->hh.tbl->buckets) {                                              \
      HASH_RECORD_OOM(oomed);                                                    \
      uthash_free((head)->hh.tbl, sizeof(UT_hash_table));                        \
    } else {                                                                     \
      uthash_bzero((head)->hh.tbl->buckets,                                      \
          HASH_INITIAL_NUM_BUCKETS * sizeof(struct UT_hash_bucket));             \
      HASH_BLOOM_MAKE((head)->hh.tbl, oomed);                                    \
      IF_HASH_NONFATAL_OOM(                                                      \
        if (oomed) {                                                             \
          uthash_free((head)->hh.tbl->buckets,                                   \
              HASH_INITIAL_NUM_BUCKETS*sizeof(struct UT_hash_bucket));           \
          uthash_free((head)->hh.tbl, sizeof(UT_hash_table));                    \
        }                                                                        \
      )                                                                          \
    }                                                                            \
  }                                                                              \
} while (0)

#define HASH_REPLACE_BYHASHVALUE_INORDER(hh,head,fieldname,keylen_in,hashval,add,replaced,cmpfcn) \
do {                                                                             \
  (replaced) = NULL;                                                             \
  HASH_FIND_BYHASHVALUE(hh, head, &((add)->fieldname), keylen_in, hashval, replaced); \
  if (replaced) {                                                                \
    HASH_DELETE(hh, head, replaced);                                             \
  }                                                                              \
  HASH_ADD_KEYPTR_BYHASHVALUE_INORDER(hh, head, &((add)->fieldname), keylen_in, hashval, add, cmpfcn); \
} while (0)

#define HASH_REPLACE_BYHASHVALUE(hh,head,fieldname,keylen_in,hashval,add,replaced) \
do {                                                                             \
  (replaced) = NULL;                                                             \
  HASH_FIND_BYHASHVALUE(hh, head, &((add)->fieldname), keylen_in, hashval, replaced); \
  if (replaced) {                                                                \
    HASH_DELETE(hh, head, replaced);                                             \
  }                                                                              \
  HASH_ADD_KEYPTR_BYHASHVALUE(hh, head, &((add)->fieldname), keylen_in, hashval, add); \
} while (0)

#define HASH_REPLACE(hh,head,fieldname,keylen_in,add,replaced)                   \
do {                                                                             \
  unsigned _hr_hashv;                                                            \
  HASH_VALUE(&((add)->fieldname), keylen_in, _hr_hashv);                         \
  HASH_REPLACE_BYHASHVALUE(hh, head, fieldname, keylen_in, _hr_hashv, add, replaced); \
} while (0)

#define HASH_REPLACE_INORDER(hh,head,fieldname,keylen_in,add,replaced,cmpfcn)    \
do {                                                                             \
  unsigned _hr_hashv;                                                            \
  HASH_VALUE(&((add)->fieldname), keylen_in, _hr_hashv);                         \
  HASH_REPLACE_BYHASHVALUE_INORDER(hh, head, fieldname, keylen_in, _hr_hashv, add, replaced, cmpfcn); \
} while (0)

#define HASH_APPEND_LIST(hh, head, add)                                          \
do {                                                                             \
  (add)->hh.next = NULL;                                                         \
  (add)->hh.prev = ELMT_FROM_HH((head)->hh.tbl, (head)->hh.tbl->tail);           \
  (head)->hh.tbl->tail->next = (add);                                            \
  (head)->hh.tbl->tail = &((add)->hh);                                           \
} while (0)

#define HASH_AKBI_INNER_LOOP(hh,head,add,cmpfcn)                                 \
do {                                                                             \
  do {                                                                           \
    if (cmpfcn(DECLTYPE(head)(_hs_iter), add) > 0) {                             \
      break;                                                                     \
    }                                                                            \
  } while ((_hs_iter = HH_FROM_ELMT((head)->hh.tbl, _hs_iter)->next));           \
} while (0)

#ifdef NO_DECLTYPE
#undef HASH_AKBI_INNER_LOOP
#define HASH_AKBI_INNER_LOOP(hh,head,add,cmpfcn)                                 \
do {                                                                             \
  char *_hs_saved_head = (char*)(head);                                          \
  do {                                                                           \
    DECLTYPE_ASSIGN(head, _hs_iter);                                             \
    if (cmpfcn(head, add) > 0) {                                                 \
      DECLTYPE_ASSIGN(head, _hs_saved_head);                                     \
      break;                                                                     \
    }                                                                            \
    DECLTYPE_ASSIGN(head, _hs_saved_head);                                       \
  } while ((_hs_iter = HH_FROM_ELMT((head)->hh.tbl, _hs_iter)->next));           \
} while (0)
#endif

#if HASH_NONFATAL_OOM

#define HASH_ADD_TO_TABLE(hh,head,keyptr,keylen_in,hashval,add,oomed)            \
do {                                                                             \
  if (!(oomed)) {                                                                \
    unsigned _ha_bkt;                                                            \
    (head)->hh.tbl->num_items++;                                                 \
    HASH_TO_BKT(hashval, (head)->hh.tbl->num_buckets, _ha_bkt);                  \
    HASH_ADD_TO_BKT((head)->hh.tbl->buckets[_ha_bkt], hh, &(add)->hh, oomed);    \
    if (oomed) {                                                                 \
      HASH_ROLLBACK_BKT(hh, head, &(add)->hh);                                   \
      HASH_DELETE_HH(hh, head, &(add)->hh);                                      \
      (add)->hh.tbl = NULL;                                                      \
      uthash_nonfatal_oom(add);                                                  \
    } else {                                                                     \
      HASH_BLOOM_ADD((head)->hh.tbl, hashval);                                   \
      HASH_EMIT_KEY(hh, head, keyptr, keylen_in);                                \
    }                                                                            \
  } else {                                                                       \
    (add)->hh.tbl = NULL;                                                        \
    uthash_nonfatal_oom(add);                                                    \
  }                                                                              \
} while (0)

#else

#define HASH_ADD_TO_TABLE(hh,head,keyptr,keylen_in,hashval,add,oomed)            \
do {                                                                             \
  unsigned _ha_bkt;                                                              \
  (head)->hh.tbl->num_items++;                                                   \
  HASH_TO_BKT(hashval, (head)->hh.tbl->num_buckets, _ha_bkt);                    \
  HASH_ADD_TO_BKT((head)->hh.tbl->buckets[_ha_bkt], hh, &(add)->hh, oomed);      \
  HASH_BLOOM_ADD((head)->hh.tbl, hashval);                                       \
  HASH_EMIT_KEY(hh, head, keyptr, keylen_in);                                    \
} while (0)

#endif


#define HASH_ADD_KEYPTR_BYHASHVALUE_INORDER(hh,head,keyptr,keylen_in,hashval,add,cmpfcn) \
do {                                                                             \
  IF_HASH_NONFATAL_OOM( int _ha_oomed = 0; )                                     \
  (add)->hh.hashv = (hashval);                                                   \
  (add)->hh.key = (char*) (keyptr);                                              \
  (add)->hh.keylen = (unsigned) (keylen_in);                                     \
  if (!(head)) {                                                                 \
    (add)->hh.next = NULL;                                                       \
    (add)->hh.prev = NULL;                                                       \
    HASH_MAKE_TABLE(hh, add, _ha_oomed);                                         \
    IF_HASH_NONFATAL_OOM( if (!_ha_oomed) { )                                    \
      (head) = (add);                                                            \
    IF_HASH_NONFATAL_OOM( } )                                                    \
  } else {                                                                       \
    void *_hs_iter = (head);                                                     \
    (add)->hh.tbl = (head)->hh.tbl;                                              \
    HASH_AKBI_INNER_LOOP(hh, head, add, cmpfcn);                                 \
    if (_hs_iter) {                                                              \
      (add)->hh.next = _hs_iter;                                                 \
      if (((add)->hh.prev = HH_FROM_ELMT((head)->hh.tbl, _hs_iter)->prev)) {     \
        HH_FROM_ELMT((head)->hh.tbl, (add)->hh.prev)->next = (add);              \
      } else {                                                                   \
        (head) = (add);                                                          \
      }                                                                          \
      HH_FROM_ELMT((head)->hh.tbl, _hs_iter)->prev = (add);                      \
    } else {                                                                     \
      HASH_APPEND_LIST(hh, head, add);                                           \
    }                                                                            \
  }                                                                              \
  HASH_ADD_TO_TABLE(hh, head, keyptr, keylen_in, hashval, add, _ha_oomed);       \
  HASH_FSCK(hh, head, "HASH_ADD_KEYPTR_BYHASHVALUE_INORDER");                    \
} while (0)

#define HASH_ADD_KEYPTR_INORDER(hh,head,keyptr,keylen_in,add,cmpfcn)             \
do {                                                                             \
  unsigned _hs_hashv;                                                            \
  HASH_VALUE(keyptr, keylen_in, _hs_hashv);                                      \
  HASH_ADD_KEYPTR_BYHASHVALUE_INORDER(hh, head, keyptr, keylen_in, _hs_hashv, add, cmpfcn); \
} while (0)

#define HASH_ADD_BYHASHVALUE_INORDER(hh,head,fieldname,keylen_in,hashval,add,cmpfcn) \
  HASH_ADD_KEYPTR_BYHASHVALUE_INORDER(hh, head, &((add)->fieldname), keylen_in, hashval, add, cmpfcn)

#define HASH_ADD_INORDER(hh,head,fieldname,keylen_in,add,cmpfcn)                 \
  HASH_ADD_KEYPTR_INORDER(hh, head, &((add)->fieldname), keylen_in, add, cmpfcn)

#define HASH_ADD_KEYPTR_BYHASHVALUE(hh,head,keyptr,keylen_in,hashval,add)        \
do {                                                                             \
  IF_HASH_NONFATAL_OOM( int _ha_oomed = 0; )                                     \
  (add)->hh.hashv = (hashval);                                                   \
  (add)->hh.key = (const void*) (keyptr);                                        \
  (add)->hh.keylen = (unsigned) (keylen_in);                                     \
  if (!(head)) {                                                                 \
    (add)->hh.next = NULL;                                                       \
    (add)->hh.prev = NULL;                                                       \
    HASH_MAKE_TABLE(hh, add, _ha_oomed);                                         \
    IF_HASH_NONFATAL_OOM( if (!_ha_oomed) { )                                    \
      (head) = (add);                                                            \
    IF_HASH_NONFATAL_OOM( } )                                                    \
  } else {                                                                       \
    (add)->hh.tbl = (head)->hh.tbl;                                              \
    HASH_APPEND_LIST(hh, head, add);                                             \
  }                                                                              \
  HASH_ADD_TO_TABLE(hh, head, keyptr, keylen_in, hashval, add, _ha_oomed);       \
  HASH_FSCK(hh, head, "HASH_ADD_KEYPTR_BYHASHVALUE");                            \
} while (0)

#define HASH_ADD_KEYPTR(hh,head,keyptr,keylen_in,add)                            \
do {                                                                             \
  unsigned _ha_hashv;                                                            \
  HASH_VALUE(keyptr, keylen_in, _ha_hashv);                                      \
  HASH_ADD_KEYPTR_BYHASHVALUE(hh, head, keyptr, keylen_in, _ha_hashv, add);      \
} while (0)

#define HASH_ADD_BYHASHVALUE(hh,head,fieldname,keylen_in,hashval,add)            \
  HASH_ADD_KEYPTR_BYHASHVALUE(hh, head, &((add)->fieldname), keylen_in, hashval, add)

#define HASH_ADD(hh,head,fieldname,keylen_in,add)                                \
  HASH_ADD_KEYPTR(hh, head, &((add)->fieldname), keylen_in, add)

#define HASH_TO_BKT(hashv,num_bkts,bkt)                                          \
do {                                                                             \
  bkt = ((hashv) & ((num_bkts) - 1U));                                           \
} while (0)

/* delete "delptr" from the hash table.
 * "the usual" patch-up process for the app-order doubly-linked-list.
 * The use of _hd_hh_del below deserves special explanation.
 * These used to be expressed using (delptr) but that led to a bug
 * if someone used the same symbol for the head and deletee, like
 *  HASH_DELETE(hh,users,users);
 * We want that to work, but by changing the head (users) below
 * we were forfeiting our ability to further refer to the deletee (users)
 * in the patch-up process. Solution: use scratch space to
 * copy the deletee pointer, then the latter references are via that
 * scratch pointer rather than through the repointed (users) symbol.
 */
#define HASH_DELETE(hh,head,delptr)                                              \
    HASH_DELETE_HH(hh, head, &(delptr)->hh)

#define HASH_DELETE_HH(hh,head,delptrhh)                                         \
do {                                                                             \
  const struct UT_hash_handle *_hd_hh_del = (delptrhh);                          \
  if ((_hd_hh_del->prev == NULL) && (_hd_hh_del->next == NULL)) {                \
    HASH_BLOOM_FREE((head)->hh.tbl);                                             \
    uthash_free((head)->hh.tbl->buckets,                                         \
                (head)->hh.tbl->num_buckets * sizeof(struct UT_hash_bucket));    \
    uthash_free((head)->hh.tbl, sizeof(UT_hash_table));                          \
    (head) = NULL;                                                               \
  } else {                                                                       \
    unsigned _hd_bkt;                                                            \
    if (_hd_hh_del == (head)->hh.tbl->tail) {                                    \
      (head)->hh.tbl->tail = HH_FROM_ELMT((head)->hh.tbl, _hd_hh_del->prev);     \
    }                                                                            \
    if (_hd_hh_del->prev != NULL) {                                              \
      HH_FROM_ELMT((head)->hh.tbl, _hd_hh_del->prev)->next = _hd_hh_del->next;   \
    } else {                                                                     \
      DECLTYPE_ASSIGN(head, _hd_hh_del->next);                                   \
    }                                                                            \
    if (_hd_hh_del->next != NULL) {                                              \
      HH_FROM_ELMT((head)->hh.tbl, _hd_hh_del->next)->prev = _hd_hh_del->prev;   \
    }                                                                            \
    HASH_TO_BKT(_hd_hh_del->hashv, (head)->hh.tbl->num_buckets, _hd_bkt);        \
    HASH_DEL_IN_BKT((head)->hh.tbl->buckets[_hd_bkt], _hd_hh_del);               \
    (head)->hh.tbl->num_items--;                                                 \
  }                                                                              \
  HASH_FSCK(hh, head, "HASH_DELETE_HH");                                         \
} while (0)

/* convenience forms of HASH_FIND/HASH_ADD/HASH_DEL */
#define HASH_FIND_STR(head,findstr,out)                                          \
do {                                                                             \
    unsigned _uthash_hfstr_keylen = (unsigned)uthash_strlen(findstr);            \
    HASH_FIND(hh, head, findstr, _uthash_hfstr_keylen, out);                     \
} while (0)
#define HASH_ADD_STR(head,strfield,add)                                          \
do {                                                                             \
    unsigned _uthash_hastr_keylen = (unsigned)uthash_strlen((add)->strfield);    \
    HASH_ADD(hh, head, strfield[0], _uthash_hastr_keylen, add);                  \
} while (0)
#define HASH_REPLACE_STR(head,strfield,add,replaced)                             \
do {                                                                             \
    unsigned _uthash_hrstr_keylen = (unsigned)uthash_strlen((add)->strfield);    \
    HASH_REPLACE(hh, head, strfield[0], _uthash_hrstr_keylen, add, replaced);    \
} while (0)
#define HASH_FIND_INT(head,findint,out)                                          \
    HASH_FIND(hh,head,findint,sizeof(int),out)
#define HASH_ADD_INT(head,intfield,add)                                          \
    HASH_ADD(hh,head,intfield,sizeof(int),add)
#define HASH_REPLACE_INT(head,intfield,add,replaced)                             \
    HASH_REPLACE(hh,head,intfield,sizeof(int),add,replaced)
#define HASH_FIND_PTR(head,findptr,out)                                          \
    HASH_FIND(hh,head,findptr,sizeof(void *),out)
#define HASH_ADD_PTR(head,ptrfield,add)                                          \
    HASH_ADD(hh,head,ptrfield,sizeof(void *),add)
#define HASH_REPLACE_PTR(head,ptrfield,add,replaced)                             \
    HASH_REPLACE(hh,head,ptrfield,sizeof(void *),add,replaced)
#define HASH_DEL(head,delptr)                                                    \
    HASH_DELETE(hh,head,delptr)

/* HASH_FSCK checks hash integrity on every add/delete when HASH_DEBUG is defined.
 * This is for uthash developer only; it compiles away if HASH_DEBUG isn't defined.
 */
#ifdef HASH_DEBUG
#include <stdio.h>   /* fprintf, stderr */
#define HASH_OOPS(...) do { fprintf(stderr, __VA_ARGS__); exit(-1); } while (0)
#define HASH_FSCK(hh,head,where)                                                 \
do {                                                                             \
  struct UT_hash_handle *_thh;                                                   \
  if (head) {                                                                    \
    unsigned _bkt_i;                                                             \
    unsigned _count = 0;                                                         \
    char *_prev;                                                                 \
    for (_bkt_i = 0; _bkt_i < (head)->hh.tbl->num_buckets; ++_bkt_i) {           \
      unsigned _bkt_count = 0;                                                   \
      _thh = (head)->hh.tbl->buckets[_bkt_i].hh_head;                            \
      _prev = NULL;                                                              \
      while (_thh) {                                                             \
        if (_prev != (char*)(_thh->hh_prev)) {                                   \
          HASH_OOPS("%s: invalid hh_prev %p, actual %p\n",                       \
              (where), (void*)_thh->hh_prev, (void*)_prev);                      \
        }                                                                        \
        _bkt_count++;                                                            \
        _prev = (char*)(_thh);                                                   \
        _thh = _thh->hh_next;                                                    \
      }                                                                          \
      _count += _bkt_count;                                                      \
      if ((head)->hh.tbl->buckets[_bkt_i].count !=  _bkt_count) {                \
        HASH_OOPS("%s: invalid bucket count %u, actual %u\n",                    \
            (where), (head)->hh.tbl->buckets[_bkt_i].count, _bkt_count);         \
      }                                                                          \
    }                                                                            \
    if (_count != (head)->hh.tbl->num_items) {                                   \
      HASH_OOPS("%s: invalid hh item count %u, actual %u\n",                     \
          (where), (head)->hh.tbl->num_items, _count);                           \
    }                                                                            \
    _count = 0;                                                                  \
    _prev = NULL;                                                                \
    _thh =  &(head)->hh;                                                         \
    while (_thh) {                                                               \
      _count++;                                                                  \
      if (_prev != (char*)_thh->prev) {                                          \
        HASH_OOPS("%s: invalid prev %p, actual %p\n",                            \
            (where), (void*)_thh->prev, (void*)_prev);                           \
      }                                                                          \
      _prev = (char*)ELMT_FROM_HH((head)->hh.tbl, _thh);                         \
      _thh = (_thh->next ? HH_FROM_ELMT((head)->hh.tbl, _thh->next) : NULL);     \
    }                                                                            \
    if (_count != (head)->hh.tbl->num_items) {                                   \
      HASH_OOPS("%s: invalid app item count %u, actual %u\n",                    \
          (where), (head)->hh.tbl->num_items, _count);                           \
    }                                                                            \
  }                                                                              \
} while (0)
#else
#define HASH_FSCK(hh,head,where)
#endif

/* When compiled with -DHASH_EMIT_KEYS, length-prefixed keys are emitted to
 * the descriptor to which this macro is defined for tuning the hash function.
 * The app can #include <unistd.h> to get the prototype for write(2). */
#ifdef HASH_EMIT_KEYS
#define HASH_EMIT_KEY(hh,head,keyptr,fieldlen)                                   \
do {                                                                             \
  unsigned _klen = fieldlen;                                                     \
  write(HASH_EMIT_KEYS, &_klen, sizeof(_klen));                                  \
  write(HASH_EMIT_KEYS, keyptr, (unsigned long)fieldlen);                        \
} while (0)
#else
#define HASH_EMIT_KEY(hh,head,keyptr,fieldlen)
#endif

/* The Bernstein hash function, used in Perl prior to v5.6. Note (x<<5+x)=x*33. */
#define HASH_BER(key,keylen,hashv)                                               \
do {                                                                             \
  unsigned _hb_keylen = (unsigned)keylen;                                        \
  const unsigned char *_hb_key = (const unsigned char*)(key);                    \
  (hashv) = 0;                                                                   \
  while (_hb_keylen-- != 0U) {                                                   \
    (hashv) = (((hashv) << 5) + (hashv)) + *_hb_key++;                           \
  }                                                                              \
} while (0)


/* SAX/FNV/OAT/JEN hash functions are macro variants of those listed at
 * http://eternallyconfuzzled.com/tuts/algorithms/jsw_tut_hashing.aspx
 * (archive link: https://archive.is/Ivcan )
 */
#define HASH_SAX(key,keylen,hashv)                                               \
do {                                                                             \
  unsigned _sx_i;                                                                \
  const unsigned char *_hs_key = (const unsigned char*)(key);                    \
  hashv = 0;                                                                     \
  for (_sx_i=0; _sx_i < keylen; _sx_i++) {                                       \
    hashv ^= (hashv << 5) + (hashv >> 2) + _hs_key[_sx_i];                       \
  }                                                                              \
} while (0)
/* FNV-1a variation */
#define HASH_FNV(key,keylen,hashv)                                               \
do {                                                                             \
  unsigned _fn_i;                                                                \
  const unsigned char *_hf_key = (const unsigned char*)(key);                    \
  (hashv) = 2166136261U;                                                         \
  for (_fn_i=0; _fn_i < keylen; _fn_i++) {                                       \
    hashv = hashv ^ _hf_key[_fn_i];                                              \
    hashv = hashv * 16777619U;                                                   \
  }                                                                              \
} while (0)

#define HASH_OAT(key,keylen,hashv)                                               \
do {                                                                             \
  unsigned _ho_i;                                                                \
  const unsigned char *_ho_key=(const unsigned char*)(key);                      \
  hashv = 0;                                                                     \
  for(_ho_i=0; _ho_i < keylen; _ho_i++) {                                        \
      hashv += _ho_key[_ho_i];                                                   \
      hashv += (hashv << 10);                                                    \
      hashv ^= (hashv >> 6);                                                     \
  }                                                                              \
  hashv += (hashv << 3);                                                         \
  hashv ^= (hashv >> 11);                                                        \
  hashv += (hashv << 15);                                                        \
} while (0)

#define HASH_JEN_MIX(a,b,c)                                                      \
do {                                                                             \
  a -= b; a -= c; a ^= ( c >> 13 );                                              \
  b -= c; b -= a; b ^= ( a << 8 );                                               \
  c -= a; c -= b; c ^= ( b >> 13 );                                              \
  a -= b; a -= c; a ^= ( c >> 12 );                                              \
  b -= c; b -= a; b ^= ( a << 16 );                                              \
  c -= a; c -= b; c ^= ( b >> 5 );                                               \
  a -= b; a -= c; a ^= ( c >> 3 );                                               \
  b -= c; b -= a; b ^= ( a << 10 );                                              \
  c -= a; c -= b; c ^= ( b >> 15 );                                              \
} while (0)

#define HASH_JEN(key,keylen,hashv)                                               \
do {                                                                             \
  unsigned _hj_i,_hj_j,_hj_k;                                                    \
  unsigned const char *_hj_key=(unsigned const char*)(key);                      \
  hashv = 0xfeedbeefu;                                                           \
  _hj_i = _hj_j = 0x9e3779b9u;                                                   \
  _hj_k = (unsigned)(keylen);                                                    \
  while (_hj_k >= 12U) {                                                         \
    _hj_i +=    (_hj_key[0] + ( (unsigned)_hj_key[1] << 8 )                      \
        + ( (unsigned)_hj_key[2] << 16 )                                         \
        + ( (unsigned)_hj_key[3] << 24 ) );                                      \
    _hj_j +=    (_hj_key[4] + ( (unsigned)_hj_key[5] << 8 )                      \
        + ( (unsigned)_hj_key[6] << 16 )                                         \
        + ( (unsigned)_hj_key[7] << 24 ) );                                      \
    hashv += (_hj_key[8] + ( (unsigned)_hj_key[9] << 8 )                         \
        + ( (unsigned)_hj_key[10] << 16 )                                        \
        + ( (unsigned)_hj_key[11] << 24 ) );                                     \
                                                                                 \
     HASH_JEN_MIX(_hj_i, _hj_j, hashv);                                          \
                                                                                 \
     _hj_key += 12;                                                              \
     _hj_k -= 12U;                                                               \
  }                                                                              \
  hashv += (unsigned)(keylen);                                                   \
  switch ( _hj_k ) {                                                             \
    case 11: hashv += ( (unsigned)_hj_key[10] << 24 ); /* FALLTHROUGH */         \
    case 10: hashv += ( (unsigned)_hj_key[9] << 16 );  /* FALLTHROUGH */         \
    case 9:  hashv += ( (unsigned)_hj_key[8] << 8 );   /* FALLTHROUGH */         \
    case 8:  _hj_j += ( (unsigned)_hj_key[7] << 24 );  /* FALLTHROUGH */         \
    case 7:  _hj_j += ( (unsigned)_hj_key[6] << 16 );  /* FALLTHROUGH */         \
    case 6:  _hj_j += ( (unsigned)_hj_key[5] << 8 );   /* FALLTHROUGH */         \
    case 5:  _hj_j += _hj_key[4];                      /* FALLTHROUGH */         \
    case 4:  _hj_i += ( (unsigned)_hj_key[3] << 24 );  /* FALLTHROUGH */         \
    case 3:  _hj_i += ( (unsigned)_hj_key[2] << 16 );  /* FALLTHROUGH */         \
    case 2:  _hj_i += ( (unsigned)_hj_key[1] << 8 );   /* FALLTHROUGH */         \
    case 1:  _hj_i += _hj_key[0];                      /* FALLTHROUGH */         \
    default: ;                                                                   \
  }                                                                              \
  HASH_JEN_MIX(_hj_i, _hj_j, hashv);                                             \
} while (0)

/* The Paul Hsieh hash function */
#undef get16bits
#if (defined(__GNUC__) && defined(__i386__)) || defined(__WATCOMC__)             \
  || defined(_MSC_VER) || defined (__BORLANDC__) || defined (__TURBOC__)
#define get16bits(d) (*((const uint16_t *) (d)))
#endif

#if !defined (get16bits)
#define get16bits(d) ((((uint32_t)(((const uint8_t *)(d))[1])) << 8)             \
                       +(uint32_t)(((const uint8_t *)(d))[0]) )
#endif
#define HASH_SFH(key,keylen,hashv)                                               \
do {                                                                             \
  unsigned const char *_sfh_key=(unsigned const char*)(key);                     \
  uint32_t _sfh_tmp, _sfh_len = (uint32_t)keylen;                                \
                                                                                 \
  unsigned _sfh_rem = _sfh_len & 3U;                                             \
  _sfh_len >>= 2;                                                                \
  hashv = 0xcafebabeu;                                                           \
                                                                                 \
  /* Main loop */                                                                \
  for (;_sfh_len > 0U; _sfh_len--) {                                             \
    hashv    += get16bits (_sfh_key);                                            \
    _sfh_tmp  = ((uint32_t)(get16bits (_sfh_key+2)) << 11) ^ hashv;              \
    hashv     = (hashv << 16) ^ _sfh_tmp;                                        \
    _sfh_key += 2U*sizeof (uint16_t);                                            \
    hashv    += hashv >> 11;                                                     \
  }                                                                              \
                                                                                 \
  /* Handle end cases */                                                         \
  switch (_sfh_rem) {                                                            \
    case 3: hashv += get16bits (_sfh_key);                                       \
            hashv ^= hashv << 16;                                                \
            hashv ^= (uint32_t)(_sfh_key[sizeof (uint16_t)]) << 18;              \
            hashv += hashv >> 11;                                                \
            break;                                                               \
    case 2: hashv += get16bits (_sfh_key);                                       \
            hashv ^= hashv << 11;                                                \
            hashv += hashv >> 17;                                                \
            break;                                                               \
    case 1: hashv += *_sfh_key;                                                  \
            hashv ^= hashv << 10;                                                \
            hashv += hashv >> 1;                                                 \
            break;                                                               \
    default: ;                                                                   \
  }                                                                              \
                                                                                 \
  /* Force "avalanching" of final 127 bits */                                    \
  hashv ^= hashv << 3;                                                           \
  hashv += hashv >> 5;                                                           \
  hashv ^= hashv << 4;                                                           \
  hashv += hashv >> 17;                                                          \
  hashv ^= hashv << 25;                                                          \
  hashv += hashv >> 6;                                                           \
} while (0)

/* iterate over items in a known bucket to find desired item */
#define HASH_FIND_IN_BKT(tbl,hh,head,keyptr,keylen_in,hashval,out)               \
do {                                                                             \
  if ((head).hh_head != NULL) {                                                  \
    DECLTYPE_ASSIGN(out, ELMT_FROM_HH(tbl, (head).hh_head));                     \
  } else {                                                                       \
    (out) = NULL;                                                                \
  }                                                                              \
  while ((out) != NULL) {                                                        \
    if ((out)->hh.hashv == (hashval) && (out)->hh.keylen == (keylen_in)) {       \
      if (HASH_KEYCMP((out)->hh.key, keyptr, keylen_in) == 0) {                  \
        break;                                                                   \
      }                                                                          \
    }                                                                            \
    if ((out)->hh.hh_next != NULL) {                                             \
      DECLTYPE_ASSIGN(out, ELMT_FROM_HH(tbl, (out)->hh.hh_next));                \
    } else {                                                                     \
      (out) = NULL;                                                              \
    }                                                                            \
  }                                                                              \
} while (0)

/* add an item to a bucket  */
#define HASH_ADD_TO_BKT(head,hh,addhh,oomed)                                     \
do {                                                                             \
  UT_hash_bucket *_ha_head = &(head);                                            \
  _ha_head->count++;                                                             \
  (addhh)->hh_next = _ha_head->hh_head;                                          \
  (addhh)->hh_prev = NULL;                                                       \
  if (_ha_head->hh_head != NULL) {                                               \
    _ha_head->hh_head->hh_prev = (addhh);                                        \
  }                                                                              \
  _ha_head->hh_head = (addhh);                                                   \
  if ((_ha_head->count >= ((_ha_head->expand_mult + 1U) * HASH_BKT_CAPACITY_THRESH)) \
      && !(addhh)->tbl->noexpand) {                                              \
    HASH_EXPAND_BUCKETS(addhh,(addhh)->tbl, oomed);                              \
    IF_HASH_NONFATAL_OOM(                                                        \
      if (oomed) {                                                               \
        HASH_DEL_IN_BKT(head,addhh);                                             \
      }                                                                          \
    )                                                                            \
  }                                                                              \
} while (0)

/* remove an item from a given bucket */
#define HASH_DEL_IN_BKT(head,delhh)                                              \
do {                                                                             \
  UT_hash_bucket *_hd_head = &(head);                                            \
  _hd_head->count--;                                                             \
  if (_hd_head->hh_head == (delhh)) {                                            \
    _hd_head->hh_head = (delhh)->hh_next;                                        \
  }                                                                              \
  if ((delhh)->hh_prev) {                                                        \
    (delhh)->hh_prev->hh_next = (delhh)->hh_next;                                \
  }                                                                              \
  if ((delhh)->hh_next) {                                                        \
    (delhh)->hh_next->hh_prev = (delhh)->hh_prev;                                \
  }                                                                              \
} while (0)

/* Bucket expansion has the effect of doubling the number of buckets
 * and redistributing the items into the new buckets. Ideally the
 * items will distribute more or less evenly into the new buckets
 * (the extent to which this is true is a measure of the quality of
 * the hash function as it applies to the key domain).
 *
 * With the items distributed into more buckets, the chain length
 * (item count) in each bucket is reduced. Thus by expanding buckets
 * the hash keeps a bound on the chain length. This bounded chain
 * length is the essence of how a hash provides constant time lookup.
 *
 * The calculation of tbl->ideal_chain_maxlen below deserves some
 * explanation. First, keep in mind that we're calculating the ideal
 * maximum chain length based on the *new* (doubled) bucket count.
 * In fractions this is just n/b (n=number of items,b=new num buckets).
 * Since the ideal chain length is an integer, we want to calculate
 * ceil(n/b). We don't depend on floating point arithmetic in this
 * hash, so to calculate ceil(n/b) with integers we could write
 *
 *      ceil(n/b) = (n/b) + ((n%b)?1:0)
 *
 * and in fact a previous version of this hash did just that.
 * But now we have improved things a bit by recognizing that b is
 * always a power of two. We keep its base 2 log handy (call it lb),
 * so now we can write this with a bit shift and logical AND:
 *
 *      ceil(n/b) = (n>>lb) + ( (n & (b-1)) ? 1:0)
 *
 */
#define HASH_EXPAND_BUCKETS(hh,tbl,oomed)                                        \
do {                                                                             \
  unsigned _he_bkt;                                                              \
  unsigned _he_bkt_i;                                                            \
  struct UT_hash_handle *_he_thh, *_he_hh_nxt;                                   \
  UT_hash_bucket *_he_new_buckets, *_he_newbkt;                                  \
  _he_new_buckets = (UT_hash_bucket*)uthash_malloc(                              \
           sizeof(struct UT_hash_bucket) * (tbl)->num_buckets * 2U);             \
  if (!_he_new_buckets) {                                                        \
    HASH_RECORD_OOM(oomed);                                                      \
  } else {                                                                       \
    uthash_bzero(_he_new_buckets,                                                \
        sizeof(struct UT_hash_bucket) * (tbl)->num_buckets * 2U);                \
    (tbl)->ideal_chain_maxlen =                                                  \
       ((tbl)->num_items >> ((tbl)->log2_num_buckets+1U)) +                      \
       ((((tbl)->num_items & (((tbl)->num_buckets*2U)-1U)) != 0U) ? 1U : 0U);    \
    (tbl)->nonideal_items = 0;                                                   \
    for (_he_bkt_i = 0; _he_bkt_i < (tbl)->num_buckets; _he_bkt_i++) {           \
      _he_thh = (tbl)->buckets[ _he_bkt_i ].hh_head;                             \
      while (_he_thh != NULL) {                                                  \
        _he_hh_nxt = _he_thh->hh_next;                                           \
        HASH_TO_BKT(_he_thh->hashv, (tbl)->num_buckets * 2U, _he_bkt);           \
        _he_newbkt = &(_he_new_buckets[_he_bkt]);                                \
        if (++(_he_newbkt->count) > (tbl)->ideal_chain_maxlen) {                 \
          (tbl)->nonideal_items++;                                               \
          if (_he_newbkt->count > _he_newbkt->expand_mult * (tbl)->ideal_chain_maxlen) { \
            _he_newbkt->expand_mult++;                                           \
          }                                                                      \
        }                                                                        \
        _he_thh->hh_prev = NULL;                                                 \
        _he_thh->hh_next = _he_newbkt->hh_head;                                  \
        if (_he_newbkt->hh_head != NULL) {                                       \
          _he_newbkt->hh_head->hh_prev = _he_thh;                                \
        }                                                                        \
        _he_newbkt->hh_head = _he_thh;                                           \
        _he_thh = _he_hh_nxt;                                                    \
      }                                                                          \
    }                                                                            \
    uthash_free((tbl)->buckets, (tbl)->num_buckets * sizeof(struct UT_hash_bucket)); \
    (tbl)->num_buckets *= 2U;                                                    \
    (tbl)->log2_num_buckets++;                                                   \
    (tbl)->buckets = _he_new_buckets;                                            \
    (tbl)->ineff_expands = ((tbl)->nonideal_items > ((tbl)->num_items >> 1)) ?   \
        ((tbl)->ineff_expands+1U) : 0U;                                          \
    if ((tbl)->ineff_expands > 1U) {                                             \
      (tbl)->noexpand = 1;                                                       \
      uthash_noexpand_fyi(tbl);                                                  \
    }                                                                            \
    uthash_expand_fyi(tbl);                                                      \
  }                                                                              \
} while (0)


/* This is an adaptation of Simon Tatham's O(n log(n)) mergesort */
/* Note that HASH_SORT assumes the hash handle name to be hh.
 * HASH_SRT was added to allow the hash handle name to be passed in. */
#define HASH_SORT(head,cmpfcn) HASH_SRT(hh,head,cmpfcn)
#define HASH_SRT(hh,head,cmpfcn)                                                 \
do {                                                                             \
  unsigned _hs_i;                                                                \
  unsigned _hs_looping,_hs_nmerges,_hs_insize,_hs_psize,_hs_qsize;               \
  struct UT_hash_handle *_hs_p, *_hs_q, *_hs_e, *_hs_list, *_hs_tail;            \
  if (head != NULL) {                                                            \
    _hs_insize = 1;                                                              \
    _hs_looping = 1;                                                             \
    _hs_list = &((head)->hh);                                                    \
    while (_hs_looping != 0U) {                                                  \
      _hs_p = _hs_list;                                                          \
      _hs_list = NULL;                                                           \
      _hs_tail = NULL;                                                           \
      _hs_nmerges = 0;                                                           \
      while (_hs_p != NULL) {                                                    \
        _hs_nmerges++;                                                           \
        _hs_q = _hs_p;                                                           \
        _hs_psize = 0;                                                           \
        for (_hs_i = 0; _hs_i < _hs_insize; ++_hs_i) {                           \
          _hs_psize++;                                                           \
          _hs_q = ((_hs_q->next != NULL) ?                                       \
            HH_FROM_ELMT((head)->hh.tbl, _hs_q->next) : NULL);                   \
          if (_hs_q == NULL) {                                                   \
            break;                                                               \
          }                                                                      \
        }                                                                        \
        _hs_qsize = _hs_insize;                                                  \
        while ((_hs_psize != 0U) || ((_hs_qsize != 0U) && (_hs_q != NULL))) {    \
          if (_hs_psize == 0U) {                                                 \
            _hs_e = _hs_q;                                                       \
            _hs_q = ((_hs_q->next != NULL) ?                                     \
              HH_FROM_ELMT((head)->hh.tbl, _hs_q->next) : NULL);                 \
            _hs_qsize--;                                                         \
          } else if ((_hs_qsize == 0U) || (_hs_q == NULL)) {                     \
            _hs_e = _hs_p;                                                       \
            if (_hs_p != NULL) {                                                 \
              _hs_p = ((_hs_p->next != NULL) ?                                   \
                HH_FROM_ELMT((head)->hh.tbl, _hs_p->next) : NULL);               \
            }                                                                    \
            _hs_psize--;                                                         \
          } else if ((cmpfcn(                                                    \
                DECLTYPE(head)(ELMT_FROM_HH((head)->hh.tbl, _hs_p)),             \
                DECLTYPE(head)(ELMT_FROM_HH((head)->hh.tbl, _hs_q))              \
                )) <= 0) {                                                       \
            _hs_e = _hs_p;                                                       \
            if (_hs_p != NULL) {                                                 \
              _hs_p = ((_hs_p->next != NULL) ?                                   \
                HH_FROM_ELMT((head)->hh.tbl, _hs_p->next) : NULL);               \
            }                                                                    \
            _hs_psize--;                                                         \
          } else {                                                               \
            _hs_e = _hs_q;                                                       \
            _hs_q = ((_hs_q->next != NULL) ?                                     \
              HH_FROM_ELMT((head)->hh.tbl, _hs_q->next) : NULL);                 \
            _hs_qsize--;                                                         \
          }                                                                      \
          if ( _hs_tail != NULL ) {                                              \
            _hs_tail->next = ((_hs_e != NULL) ?                                  \
              ELMT_FROM_HH((head)->hh.tbl, _hs_e) : NULL);                       \
          } else {                                                               \
            _hs_list = _hs_e;                                                    \
          }                                                                      \
          if (_hs_e != NULL) {                                                   \
            _hs_e->prev = ((_hs_tail != NULL) ?                                  \
              ELMT_FROM_HH((head)->hh.tbl, _hs_tail) : NULL);                    \
          }                                                                      \
          _hs_tail = _hs_e;                                                      \
        }                                                                        \
        _hs_p = _hs_q;                                                           \
      }                                                                          \
      if (_hs_tail != NULL) {                                                    \
        _hs_tail->next = NULL;                                                   \
      }                                                                          \
      if (_hs_nmerges <= 1U) {                                                   \
        _hs_looping = 0;                                                         \
        (head)->hh.tbl->tail = _hs_tail;                                         \
        DECLTYPE_ASSIGN(head, ELMT_FROM_HH((head)->hh.tbl, _hs_list));           \
      }                                                                          \
      _hs_insize *= 2U;                                                          \
    }                                                                            \
    HASH_FSCK(hh, head, "HASH_SRT");                                             \
  }                                                                              \
} while (0)

/* This function selects items from one hash into another hash.
 * The end result is that the selected items have dual presence
 * in both hashes. There is no copy of the items made; rather
 * they are added into the new hash through a secondary hash
 * hash handle that must be present in the structure. */
#define HASH_SELECT(hh_dst, dst, hh_src, src, cond)                              \
do {                                                                             \
  unsigned _src_bkt, _dst_bkt;                                                   \
  void *_last_elt = NULL, *_elt;                                                 \
  UT_hash_handle *_src_hh, *_dst_hh, *_last_elt_hh=NULL;                         \
  ptrdiff_t _dst_hho = ((char*)(&(dst)->hh_dst) - (char*)(dst));                 \
  if ((src) != NULL) {                                                           \
    for (_src_bkt=0; _src_bkt < (src)->hh_src.tbl->num_buckets; _src_bkt++) {    \
      for (_src_hh = (src)->hh_src.tbl->buckets[_src_bkt].hh_head;               \
        _src_hh != NULL;                                                         \
        _src_hh = _src_hh->hh_next) {                                            \
        _elt = ELMT_FROM_HH((src)->hh_src.tbl, _src_hh);                         \
        if (cond(_elt)) {                                                        \
          IF_HASH_NONFATAL_OOM( int _hs_oomed = 0; )                             \
          _dst_hh = (UT_hash_handle*)(void*)(((char*)_elt) + _dst_hho);          \
          _dst_hh->key = _src_hh->key;                                           \
          _dst_hh->keylen = _src_hh->keylen;                                     \
          _dst_hh->hashv = _src_hh->hashv;                                       \
          _dst_hh->prev = _last_elt;                                             \
          _dst_hh->next = NULL;                                                  \
          if (_last_elt_hh != NULL) {                                            \
            _last_elt_hh->next = _elt;                                           \
          }                                                                      \
          if ((dst) == NULL) {                                                   \
            DECLTYPE_ASSIGN(dst, _elt);                                          \
            HASH_MAKE_TABLE(hh_dst, dst, _hs_oomed);                             \
            IF_HASH_NONFATAL_OOM(                                                \
              if (_hs_oomed) {                                                   \
                uthash_nonfatal_oom(_elt);                                       \
                (dst) = NULL;                                                    \
                continue;                                                        \
              }                                                                  \
            )                                                                    \
          } else {                                                               \
            _dst_hh->tbl = (dst)->hh_dst.tbl;                                    \
          }                                                                      \
          HASH_TO_BKT(_dst_hh->hashv, _dst_hh->tbl->num_buckets, _dst_bkt);      \
          HASH_ADD_TO_BKT(_dst_hh->tbl->buckets[_dst_bkt], hh_dst, _dst_hh, _hs_oomed); \
          (dst)->hh_dst.tbl->num_items++;                                        \
          IF_HASH_NONFATAL_OOM(                                                  \
            if (_hs_oomed) {                                                     \
              HASH_ROLLBACK_BKT(hh_dst, dst, _dst_hh);                           \
              HASH_DELETE_HH(hh_dst, dst, _dst_hh);                              \
              _dst_hh->tbl = NULL;                                               \
              uthash_nonfatal_oom(_elt);                                         \
              continue;                                                          \
            }                                                                    \
          )                                                                      \
          HASH_BLOOM_ADD(_dst_hh->tbl, _dst_hh->hashv);                          \
          _last_elt = _elt;                                                      \
          _last_elt_hh = _dst_hh;                                                \
        }                                                                        \
      }                                                                          \
    }                                                                            \
  }                                                                              \
  HASH_FSCK(hh_dst, dst, "HASH_SELECT");                                         \
} while (0)

#define HASH_CLEAR(hh,head)                                                      \
do {                                                                             \
  if ((head) != NULL) {                                                          \
    HASH_BLOOM_FREE((head)->hh.tbl);                                             \
    uthash_free((head)->hh.tbl->buckets,                                         \
                (head)->hh.tbl->num_buckets*sizeof(struct UT_hash_bucket));      \
    uthash_free((head)->hh.tbl, sizeof(UT_hash_table));                          \
    (head) = NULL;                                                               \
  }                                                                              \
} while (0)

#define HASH_OVERHEAD(hh,head)                                                   \
 (((head) != NULL) ? (                                                           \
 (size_t)(((head)->hh.tbl->num_items   * sizeof(UT_hash_handle))   +             \
          ((head)->hh.tbl->num_buckets * sizeof(UT_hash_bucket))   +             \
           sizeof(UT_hash_table)                                   +             \
           (HASH_BLOOM_BYTELEN))) : 0U)

#ifdef NO_DECLTYPE
#define HASH_ITER(hh,head,el,tmp)                                                \
for(((el)=(head)), ((*(char**)(&(tmp)))=(char*)((head!=NULL)?(head)->hh.next:NULL)); \
  (el) != NULL; ((el)=(tmp)), ((*(char**)(&(tmp)))=(char*)((tmp!=NULL)?(tmp)->hh.next:NULL)))
#else
#define HASH_ITER(hh,head,el,tmp)                                                \
for(((el)=(head)), ((tmp)=DECLTYPE(el)((head!=NULL)?(head)->hh.next:NULL));      \
  (el) != NULL; ((el)=(tmp)), ((tmp)=DECLTYPE(el)((tmp!=NULL)?(tmp)->hh.next:NULL)))
#endif

/* obtain a count of items in the hash */
#define HASH_COUNT(head) HASH_CNT(hh,head)
#define HASH_CNT(hh,head) ((head != NULL)?((head)->hh.tbl->num_items):0U)

typedef struct UT_hash_bucket {
   struct UT_hash_handle *hh_head;
   unsigned count;

   /* expand_mult is normally set to 0. In this situation, the max chain length
    * threshold is enforced at its default value, HASH_BKT_CAPACITY_THRESH. (If
    * the bucket's chain exceeds this length, bucket expansion is triggered).
    * However, setting expand_mult to a non-zero value delays bucket expansion
    * (that would be triggered by additions to this particular bucket)
    * until its chain length reaches a *multiple* of HASH_BKT_CAPACITY_THRESH.
    * (The multiplier is simply expand_mult+1). The whole idea of this
    * multiplier is to reduce bucket expansions, since they are expensive, in
    * situations where we know that a particular bucket tends to be overused.
    * It is better to let its chain length grow to a longer yet-still-bounded
    * value, than to do an O(n) bucket expansion too often.
    */
   unsigned expand_mult;

} UT_hash_bucket;

/* random signature used only to find hash tables in external analysis */
#define HASH_SIGNATURE 0xa0111fe1u
#define HASH_BLOOM_SIGNATURE 0xb12220f2u

typedef struct UT_hash_table {
   UT_hash_bucket *buckets;
   unsigned num_buckets, log2_num_buckets;
   unsigned num_items;
   struct UT_hash_handle *tail; /* tail hh in app order, for fast append    */
   ptrdiff_t hho; /* hash handle offset (byte pos of hash handle in element */

   /* in an ideal situation (all buckets used equally), no bucket would have
    * more than ceil(#items/#buckets) items. that's the ideal chain length. */
   unsigned ideal_chain_maxlen;

   /* nonideal_items is the number of items in the hash whose chain position
    * exceeds the ideal chain maxlen. these items pay the penalty for an uneven
    * hash distribution; reaching them in a chain traversal takes >ideal steps */
   unsigned nonideal_items;

   /* ineffective expands occur when a bucket doubling was performed, but
    * afterward, more than half the items in the hash had nonideal chain
    * positions. If this happens on two consecutive expansions we inhibit any
    * further expansion, as it's not helping; this happens when the hash
    * function isn't a good fit for the key domain. When expansion is inhibited
    * the hash will still work, albeit no longer in constant time. */
   unsigned ineff_expands, noexpand;

   uint32_t signature; /* used only to find hash tables in external analysis */
#ifdef HASH_BLOOM
   uint32_t bloom_sig; /* used only to test bloom exists in external analysis */
   uint8_t *bloom_bv;
   uint8_t bloom_nbits;
#endif

} UT_hash_table;

typedef struct UT_hash_handle {
   struct UT_hash_table *tbl;
   void *prev;                       /* prev element in app order      */
   void *next;                       /* next element in app order      */
   struct UT_hash_handle *hh_prev;   /* previous hh in bucket order    */
   struct UT_hash_handle *hh_next;   /* next hh in bucket order        */
   const void *key;                  /* ptr to enclosing struct's key  */
   unsigned keylen;                  /* enclosing struct's key len     */
   unsigned hashv;                   /* result of hash-fcn(key)        */
} UT_hash_handle;

#endif /* UTHASH_H */
// ------------------------------------------------------------------------------------------

// https://github.com/kokke/tiny-regex-c/blob/master/re.h
// ------------------------------------------------------------------------------------------
// This is free and unencumbered software released into the public domain.

// Anyone is free to copy, modify, publish, use, compile, sell, or
// distribute this software, either in source code form or as a compiled
// binary, for any purpose, commercial or non-commercial, and by any
// means.

// In jurisdictions that recognize copyright laws, the author or authors
// of this software dedicate any and all copyright interest in the
// software to the public domain. We make this dedication for the benefit
// of the public at large and to the detriment of our heirs and
// successors. We intend this dedication to be an overt act of
// relinquishment in perpetuity of all present and future rights to this
// software under copyright law.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
// OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.

// For more information, please refer to <http://unlicense.org>

/*
 *
 * Mini regex-module inspired by Rob Pike's regex code described in:
 *
 * http://www.cs.princeton.edu/courses/archive/spr09/cos333/beautiful.html
 *
 *
 *
 * Supports:
 * ---------
 *   '.'        Dot, matches any character
 *   '^'        Start anchor, matches beginning of string
 *   '$'        End anchor, matches end of string
 *   '*'        Asterisk, match zero or more (greedy)
 *   '+'        Plus, match one or more (greedy)
 *   '?'        Question, match zero or one (non-greedy)
 *   '[abc]'    Character class, match if one of {'a', 'b', 'c'}
 *   '[^abc]'   Inverted class, match if NOT one of {'a', 'b', 'c'} -- NOTE: feature is currently broken!
 *   '[a-zA-Z]' Character ranges, the character set of the ranges { a-z | A-Z }
 *   '\s'       Whitespace, \t \f \r \n \v and spaces
 *   '\S'       Non-whitespace
 *   '\w'       Alphanumeric, [a-zA-Z0-9_]
 *   '\W'       Non-alphanumeric
 *   '\d'       Digits, [0-9]
 *   '\D'       Non-digits
 *
 *
 */

#ifndef _TINY_REGEX_C
#define _TINY_REGEX_C


#ifndef RE_DOT_MATCHES_NEWLINE
/* Define to 0 if you DON'T want '.' to match '\r' + '\n' */
#define RE_DOT_MATCHES_NEWLINE 1
#endif

#ifdef __cplusplus
extern "C"{
#endif



/* Typedef'd pointer to get abstract datatype. */
typedef struct regex_t* re_t;


/* Compile regex string pattern to a regex_t-array. */
re_t re_compile(const char* pattern);


/* Find matches of the compiled pattern inside text. */
int re_matchp(re_t pattern, const char* text, int* matchlength);


/* Find matches of the txt pattern inside text (will compile automatically first). */
int re_match(const char* pattern, const char* text, int* matchlength);


#ifdef __cplusplus
}
#endif

#endif /* ifndef _TINY_REGEX_C */

// ------------------------------------------------------------------------------------------

// https://github.com/kokke/tiny-regex-c/blob/master/re.c
// ------------------------------------------------------------------------------------------
// This is free and unencumbered software released into the public domain.

// Anyone is free to copy, modify, publish, use, compile, sell, or
// distribute this software, either in source code form or as a compiled
// binary, for any purpose, commercial or non-commercial, and by any
// means.

// In jurisdictions that recognize copyright laws, the author or authors
// of this software dedicate any and all copyright interest in the
// software to the public domain. We make this dedication for the benefit
// of the public at large and to the detriment of our heirs and
// successors. We intend this dedication to be an overt act of
// relinquishment in perpetuity of all present and future rights to this
// software under copyright law.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
// OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.

// For more information, please refer to <http://unlicense.org>

/*
 *
 * Mini regex-module inspired by Rob Pike's regex code described in:
 *
 * http://www.cs.princeton.edu/courses/archive/spr09/cos333/beautiful.html
 *
 *
 *
 * Supports:
 * ---------
 *   '.'        Dot, matches any character
 *   '^'        Start anchor, matches beginning of string
 *   '$'        End anchor, matches end of string
 *   '*'        Asterisk, match zero or more (greedy)
 *   '+'        Plus, match one or more (greedy)
 *   '?'        Question, match zero or one (non-greedy)
 *   '[abc]'    Character class, match if one of {'a', 'b', 'c'}
 *   '[^abc]'   Inverted class, match if NOT one of {'a', 'b', 'c'} -- NOTE: feature is currently broken!
 *   '[a-zA-Z]' Character ranges, the character set of the ranges { a-z | A-Z }
 *   '\s'       Whitespace, \t \f \r \n \v and spaces
 *   '\S'       Non-whitespace
 *   '\w'       Alphanumeric, [a-zA-Z0-9_]
 *   '\W'       Non-alphanumeric
 *   '\d'       Digits, [0-9]
 *   '\D'       Non-digits
 *
 *
 */


/* Definitions: */

#define MAX_REGEXP_OBJECTS      30    /* Max number of regex symbols in expression. */
#define MAX_CHAR_CLASS_LEN      40    /* Max length of character-class buffer in.   */


enum { UNUSED, DOT, BEGIN, END, QUESTIONMARK, STAR, PLUS, CHAR, CHAR_CLASS, INV_CHAR_CLASS, DIGIT, NOT_DIGIT, ALPHA, NOT_ALPHA, WHITESPACE, NOT_WHITESPACE, /* BRANCH */ };

typedef struct regex_t
{
  unsigned char  type;   /* CHAR, STAR, etc.                      */
  union
  {
    unsigned char  ch;   /*      the character itself             */
    unsigned char* ccl;  /*  OR  a pointer to characters in class */
  } u;
} regex_t;



/* Private function declarations: */
static int matchpattern(regex_t* pattern, const char* text, int* matchlength);
static int matchcharclass(char c, const char* str);
static int matchstar(regex_t p, regex_t* pattern, const char* text, int* matchlength);
static int matchplus(regex_t p, regex_t* pattern, const char* text, int* matchlength);
static int matchone(regex_t p, char c);
static int matchdigit(char c);
static int matchalpha(char c);
static int matchwhitespace(char c);
static int matchmetachar(char c, const char* str);
static int matchrange(char c, const char* str);
static int matchdot(char c);
static int ismetachar(char c);



/* Public functions: */
int re_match(const char* pattern, const char* text, int* matchlength)
{
  return re_matchp(re_compile(pattern), text, matchlength);
}

int re_matchp(re_t pattern, const char* text, int* matchlength)
{
  *matchlength = 0;
  if (pattern != 0)
  {
    if (pattern[0].type == BEGIN)
    {
      return ((matchpattern(&pattern[1], text, matchlength)) ? 0 : -1);
    }
    else
    {
      int idx = -1;

      do
      {
        idx += 1;

        if (matchpattern(pattern, text, matchlength))
        {
          if (text[0] == '\0')
            return -1;

          return idx;
        }
      }
      while (*text++ != '\0');
    }
  }
  return -1;
}

re_t re_compile(const char* pattern)
{
  /* The sizes of the two static arrays below substantiates the static RAM usage of this module.
     MAX_REGEXP_OBJECTS is the max number of symbols in the expression.
     MAX_CHAR_CLASS_LEN determines the size of buffer for chars in all char-classes in the expression. */
  static regex_t re_compiled[MAX_REGEXP_OBJECTS];
  static unsigned char ccl_buf[MAX_CHAR_CLASS_LEN];
  int ccl_bufidx = 1;

  char c;     /* current char in pattern   */
  int i = 0;  /* index into pattern        */
  int j = 0;  /* index into re_compiled    */

  while (pattern[i] != '\0' && (j+1 < MAX_REGEXP_OBJECTS))
  {
    c = pattern[i];

    switch (c)
    {
      /* Meta-characters: */
      case '^': {    re_compiled[j].type = BEGIN;           } break;
      case '$': {    re_compiled[j].type = END;             } break;
      case '.': {    re_compiled[j].type = DOT;             } break;
      case '*': {    re_compiled[j].type = STAR;            } break;
      case '+': {    re_compiled[j].type = PLUS;            } break;
      case '?': {    re_compiled[j].type = QUESTIONMARK;    } break;
/*    case '|': {    re_compiled[j].type = BRANCH;          } break; <-- not working properly */

      /* Escaped character-classes (\s \w ...): */
      case '\\':
      {
        if (pattern[i+1] != '\0')
        {
          /* Skip the escape-char '\\' */
          i += 1;
          /* ... and check the next */
          switch (pattern[i])
          {
            /* Meta-character: */
            case 'd': {    re_compiled[j].type = DIGIT;            } break;
            case 'D': {    re_compiled[j].type = NOT_DIGIT;        } break;
            case 'w': {    re_compiled[j].type = ALPHA;            } break;
            case 'W': {    re_compiled[j].type = NOT_ALPHA;        } break;
            case 's': {    re_compiled[j].type = WHITESPACE;       } break;
            case 'S': {    re_compiled[j].type = NOT_WHITESPACE;   } break;

            /* Escaped character, e.g. '.' or '$' */
            default:
            {
              re_compiled[j].type = CHAR;
              re_compiled[j].u.ch = pattern[i];
            } break;
          }
        }
        /* '\\' as last char in pattern -> invalid regular expression. */
/*
        else
        {
          re_compiled[j].type = CHAR;
          re_compiled[j].ch = pattern[i];
        }
*/
      } break;

      /* Character class: */
      case '[':
      {
        /* Remember where the char-buffer starts. */
        int buf_begin = ccl_bufidx;

        /* Look-ahead to determine if negated */
        if (pattern[i+1] == '^')
        {
          re_compiled[j].type = INV_CHAR_CLASS;
          i += 1; /* Increment i to avoid including '^' in the char-buffer */
          if (pattern[i+1] == 0) /* incomplete pattern, missing non-zero char after '^' */
          {
            return 0;
          }
        }
        else
        {
          re_compiled[j].type = CHAR_CLASS;
        }

        /* Copy characters inside [..] to buffer */
        while (    (pattern[++i] != ']')
                && (pattern[i]   != '\0')) /* Missing ] */
        {
          if (pattern[i] == '\\')
          {
            if (ccl_bufidx >= MAX_CHAR_CLASS_LEN - 1)
            {
              //fputs("exceeded internal buffer!\n", stderr);
              return 0;
            }
            if (pattern[i+1] == 0) /* incomplete pattern, missing non-zero char after '\\' */
            {
              return 0;
            }
            ccl_buf[ccl_bufidx++] = pattern[i++];
          }
          else if (ccl_bufidx >= MAX_CHAR_CLASS_LEN)
          {
              //fputs("exceeded internal buffer!\n", stderr);
              return 0;
          }
          ccl_buf[ccl_bufidx++] = pattern[i];
        }
        if (ccl_bufidx >= MAX_CHAR_CLASS_LEN)
        {
            /* Catches cases such as [00000000000000000000000000000000000000][ */
            //fputs("exceeded internal buffer!\n", stderr);
            return 0;
        }
        /* Null-terminate string end */
        ccl_buf[ccl_bufidx++] = 0;
        re_compiled[j].u.ccl = &ccl_buf[buf_begin];
      } break;

      /* Other characters: */
      default:
      {
        re_compiled[j].type = CHAR;
        re_compiled[j].u.ch = c;
      } break;
    }
    /* no buffer-out-of-bounds access on invalid patterns - see https://github.com/kokke/tiny-regex-c/commit/1a279e04014b70b0695fba559a7c05d55e6ee90b */
    if (pattern[i] == 0)
    {
      return 0;
    }

    i += 1;
    j += 1;
  }
  /* 'UNUSED' is a sentinel used to indicate end-of-pattern */
  re_compiled[j].type = UNUSED;

  return (re_t) re_compiled;
}

void re_print(regex_t* pattern)
{
  const char* types[] = { "UNUSED", "DOT", "BEGIN", "END", "QUESTIONMARK", "STAR", "PLUS", "CHAR", "CHAR_CLASS", "INV_CHAR_CLASS", "DIGIT", "NOT_DIGIT", "ALPHA", "NOT_ALPHA", "WHITESPACE", "NOT_WHITESPACE", "BRANCH" };

  int i;
  int j;
  char c;
  for (i = 0; i < MAX_REGEXP_OBJECTS; ++i)
  {
    if (pattern[i].type == UNUSED)
    {
      break;
    }

    printf("type: %s", types[pattern[i].type]);
    if (pattern[i].type == CHAR_CLASS || pattern[i].type == INV_CHAR_CLASS)
    {
      printf(" [");
      for (j = 0; j < MAX_CHAR_CLASS_LEN; ++j)
      {
        c = pattern[i].u.ccl[j];
        if ((c == '\0') || (c == ']'))
        {
          break;
        }
        printf("%c", c);
      }
      printf("]");
    }
    else if (pattern[i].type == CHAR)
    {
      printf(" '%c'", pattern[i].u.ch);
    }
    printf("\n");
  }
}



/* Private functions: */
static int matchdigit(char c)
{
  return isdigit(c);
}
static int matchalpha(char c)
{
  return isalpha(c);
}
static int matchwhitespace(char c)
{
  return isspace(c);
}
static int matchalphanum(char c)
{
  return ((c == '_') || matchalpha(c) || matchdigit(c));
}
static int matchrange(char c, const char* str)
{
  return (    (c != '-')
           && (str[0] != '\0')
           && (str[0] != '-')
           && (str[1] == '-')
           && (str[2] != '\0')
           && (    (c >= str[0])
                && (c <= str[2])));
}
static int matchdot(char c)
{
#if defined(RE_DOT_MATCHES_NEWLINE) && (RE_DOT_MATCHES_NEWLINE == 1)
  (void)c;
  return 1;
#else
  return c != '\n' && c != '\r';
#endif
}
static int ismetachar(char c)
{
  return ((c == 's') || (c == 'S') || (c == 'w') || (c == 'W') || (c == 'd') || (c == 'D'));
}

static int matchmetachar(char c, const char* str)
{
  switch (str[0])
  {
    case 'd': return  matchdigit(c);
    case 'D': return !matchdigit(c);
    case 'w': return  matchalphanum(c);
    case 'W': return !matchalphanum(c);
    case 's': return  matchwhitespace(c);
    case 'S': return !matchwhitespace(c);
    default:  return (c == str[0]);
  }
}

static int matchcharclass(char c, const char* str)
{
  do
  {
    if (matchrange(c, str))
    {
      return 1;
    }
    else if (str[0] == '\\')
    {
      /* Escape-char: increment str-ptr and match on next char */
      str += 1;
      if (matchmetachar(c, str))
      {
        return 1;
      }
      else if ((c == str[0]) && !ismetachar(c))
      {
        return 1;
      }
    }
    else if (c == str[0])
    {
      if (c == '-')
      {
        return ((str[-1] == '\0') || (str[1] == '\0'));
      }
      else
      {
        return 1;
      }
    }
  }
  while (*str++ != '\0');

  return 0;
}

static int matchone(regex_t p, char c)
{
  switch (p.type)
  {
    case DOT:            return matchdot(c);
    case CHAR_CLASS:     return  matchcharclass(c, (const char*)p.u.ccl);
    case INV_CHAR_CLASS: return !matchcharclass(c, (const char*)p.u.ccl);
    case DIGIT:          return  matchdigit(c);
    case NOT_DIGIT:      return !matchdigit(c);
    case ALPHA:          return  matchalphanum(c);
    case NOT_ALPHA:      return !matchalphanum(c);
    case WHITESPACE:     return  matchwhitespace(c);
    case NOT_WHITESPACE: return !matchwhitespace(c);
    default:             return  (p.u.ch == c);
  }
}

static int matchstar(regex_t p, regex_t* pattern, const char* text, int* matchlength)
{
  int prelen = *matchlength;
  const char* prepoint = text;
  while ((text[0] != '\0') && matchone(p, *text))
  {
    text++;
    (*matchlength)++;
  }
  while (text >= prepoint)
  {
    if (matchpattern(pattern, text--, matchlength))
      return 1;
    (*matchlength)--;
  }

  *matchlength = prelen;
  return 0;
}

static int matchplus(regex_t p, regex_t* pattern, const char* text, int* matchlength)
{
  const char* prepoint = text;
  while ((text[0] != '\0') && matchone(p, *text))
  {
    text++;
    (*matchlength)++;
  }
  while (text > prepoint)
  {
    if (matchpattern(pattern, text--, matchlength))
      return 1;
    (*matchlength)--;
  }

  return 0;
}

static int matchquestion(regex_t p, regex_t* pattern, const char* text, int* matchlength)
{
  if (p.type == UNUSED)
    return 1;
  if (matchpattern(pattern, text, matchlength))
      return 1;
  if (*text && matchone(p, *text++))
  {
    if (matchpattern(pattern, text, matchlength))
    {
      (*matchlength)++;
      return 1;
    }
  }
  return 0;
}


#if 0

/* Recursive matching */
static int matchpattern(regex_t* pattern, const char* text, int *matchlength)
{
  int pre = *matchlength;
  if ((pattern[0].type == UNUSED) || (pattern[1].type == QUESTIONMARK))
  {
    return matchquestion(pattern[1], &pattern[2], text, matchlength);
  }
  else if (pattern[1].type == STAR)
  {
    return matchstar(pattern[0], &pattern[2], text, matchlength);
  }
  else if (pattern[1].type == PLUS)
  {
    return matchplus(pattern[0], &pattern[2], text, matchlength);
  }
  else if ((pattern[0].type == END) && pattern[1].type == UNUSED)
  {
    return text[0] == '\0';
  }
  else if ((text[0] != '\0') && matchone(pattern[0], text[0]))
  {
    (*matchlength)++;
    return matchpattern(&pattern[1], text+1);
  }
  else
  {
    *matchlength = pre;
    return 0;
  }
}

#else

/* Iterative matching */
static int matchpattern(regex_t* pattern, const char* text, int* matchlength)
{
  int pre = *matchlength;
  do
  {
    if ((pattern[0].type == UNUSED) || (pattern[1].type == QUESTIONMARK))
    {
      return matchquestion(pattern[0], &pattern[2], text, matchlength);
    }
    else if (pattern[1].type == STAR)
    {
      return matchstar(pattern[0], &pattern[2], text, matchlength);
    }
    else if (pattern[1].type == PLUS)
    {
      return matchplus(pattern[0], &pattern[2], text, matchlength);
    }
    else if ((pattern[0].type == END) && pattern[1].type == UNUSED)
    {
      return (text[0] == '\0');
    }
/*  Branching is not working properly
    else if (pattern[1].type == BRANCH)
    {
      return (matchpattern(pattern, text) || matchpattern(&pattern[2], text));
    }
*/
  (*matchlength)++;
  }
  while ((text[0] != '\0') && matchone(*pattern++, *text++));

  *matchlength = pre;
  return 0;
}

#endif

// ------------------------------------------------------------------------------------------


// https://github.com/wareya/Remimu/blob/main/remimu.h
// Creative Commons Legal Code

// CC0 1.0 Universal

//     CREATIVE COMMONS CORPORATION IS NOT A LAW FIRM AND DOES NOT PROVIDE
//     LEGAL SERVICES. DISTRIBUTION OF THIS DOCUMENT DOES NOT CREATE AN
//     ATTORNEY-CLIENT RELATIONSHIP. CREATIVE COMMONS PROVIDES THIS
//     INFORMATION ON AN "AS-IS" BASIS. CREATIVE COMMONS MAKES NO WARRANTIES
//     REGARDING THE USE OF THIS DOCUMENT OR THE INFORMATION OR WORKS
//     PROVIDED HEREUNDER, AND DISCLAIMS LIABILITY FOR DAMAGES RESULTING FROM
//     THE USE OF THIS DOCUMENT OR THE INFORMATION OR WORKS PROVIDED
//     HEREUNDER.

// Statement of Purpose

// The laws of most jurisdictions throughout the world automatically confer
// exclusive Copyright and Related Rights (defined below) upon the creator
// and subsequent owner(s) (each and all, an "owner") of an original work of
// authorship and/or a database (each, a "Work").

// Certain owners wish to permanently relinquish those rights to a Work for
// the purpose of contributing to a commons of creative, cultural and
// scientific works ("Commons") that the public can reliably and without fear
// of later claims of infringement build upon, modify, incorporate in other
// works, reuse and redistribute as freely as possible in any form whatsoever
// and for any purposes, including without limitation commercial purposes.
// These owners may contribute to the Commons to promote the ideal of a free
// culture and the further production of creative, cultural and scientific
// works, or to gain reputation or greater distribution for their Work in
// part through the use and efforts of others.

// For these and/or other purposes and motivations, and without any
// expectation of additional consideration or compensation, the person
// associating CC0 with a Work (the "Affirmer"), to the extent that he or she
// is an owner of Copyright and Related Rights in the Work, voluntarily
// elects to apply CC0 to the Work and publicly distribute the Work under its
// terms, with knowledge of his or her Copyright and Related Rights in the
// Work and the meaning and intended legal effect of CC0 on those rights.

// 1. Copyright and Related Rights. A Work made available under CC0 may be
// protected by copyright and related or neighboring rights ("Copyright and
// Related Rights"). Copyright and Related Rights include, but are not
// limited to, the following:

//   i. the right to reproduce, adapt, distribute, perform, display,
//      communicate, and translate a Work;
//  ii. moral rights retained by the original author(s) and/or performer(s);
// iii. publicity and privacy rights pertaining to a person's image or
//      likeness depicted in a Work;
//  iv. rights protecting against unfair competition in regards to a Work,
//      subject to the limitations in paragraph 4(a), below;
//   v. rights protecting the extraction, dissemination, use and reuse of data
//      in a Work;
//  vi. database rights (such as those arising under Directive 96/9/EC of the
//      European Parliament and of the Council of 11 March 1996 on the legal
//      protection of databases, and under any national implementation
//      thereof, including any amended or successor version of such
//      directive); and
// vii. other similar, equivalent or corresponding rights throughout the
//      world based on applicable law or treaty, and any national
//      implementations thereof.

// 2. Waiver. To the greatest extent permitted by, but not in contravention
// of, applicable law, Affirmer hereby overtly, fully, permanently,
// irrevocably and unconditionally waives, abandons, and surrenders all of
// Affirmer's Copyright and Related Rights and associated claims and causes
// of action, whether now known or unknown (including existing as well as
// future claims and causes of action), in the Work (i) in all territories
// worldwide, (ii) for the maximum duration provided by applicable law or
// treaty (including future time extensions), (iii) in any current or future
// medium and for any number of copies, and (iv) for any purpose whatsoever,
// including without limitation commercial, advertising or promotional
// purposes (the "Waiver"). Affirmer makes the Waiver for the benefit of each
// member of the public at large and to the detriment of Affirmer's heirs and
// successors, fully intending that such Waiver shall not be subject to
// revocation, rescission, cancellation, termination, or any other legal or
// equitable action to disrupt the quiet enjoyment of the Work by the public
// as contemplated by Affirmer's express Statement of Purpose.

// 3. Public License Fallback. Should any part of the Waiver for any reason
// be judged legally invalid or ineffective under applicable law, then the
// Waiver shall be preserved to the maximum extent permitted taking into
// account Affirmer's express Statement of Purpose. In addition, to the
// extent the Waiver is so judged Affirmer hereby grants to each affected
// person a royalty-free, non transferable, non sublicensable, non exclusive,
// irrevocable and unconditional license to exercise Affirmer's Copyright and
// Related Rights in the Work (i) in all territories worldwide, (ii) for the
// maximum duration provided by applicable law or treaty (including future
// time extensions), (iii) in any current or future medium and for any number
// of copies, and (iv) for any purpose whatsoever, including without
// limitation commercial, advertising or promotional purposes (the
// "License"). The License shall be deemed effective as of the date CC0 was
// applied by Affirmer to the Work. Should any part of the License for any
// reason be judged legally invalid or ineffective under applicable law, such
// partial invalidity or ineffectiveness shall not invalidate the remainder
// of the License, and in such case Affirmer hereby affirms that he or she
// will not (i) exercise any of his or her remaining Copyright and Related
// Rights in the Work or (ii) assert any associated claims and causes of
// action with respect to the Work, in either case contrary to Affirmer's
// express Statement of Purpose.

// 4. Limitations and Disclaimers.

//  a. No trademark or patent rights held by Affirmer are waived, abandoned,
//     surrendered, licensed or otherwise affected by this document.
//  b. Affirmer offers the Work as-is and makes no representations or
//     warranties of any kind concerning the Work, express, implied,
//     statutory or otherwise, including without limitation warranties of
//     title, merchantability, fitness for a particular purpose, non
//     infringement, or the absence of latent or other defects, accuracy, or
//     the present or absence of errors, whether or not discoverable, all to
//     the greatest extent permissible under applicable law.
//  c. Affirmer disclaims responsibility for clearing rights of other persons
//     that may apply to the Work or any use thereof, including without
//     limitation any person's Copyright and Related Rights in the Work.
//     Further, Affirmer disclaims responsibility for obtaining any necessary
//     consents, permissions or other rights required for any use of the
//     Work.
//  d. Affirmer understands and acknowledges that Creative Commons is not a
//     party to this document and has no duty or obligation with respect to
//     this CC0 or use of the Work.
// ------------------------------------------------------------------------------------------
#ifndef INCLUDE_REMIMU
#define INCLUDE_REMIMU 1

#ifndef REMIMU_FUNC_VISIBILITY
#define REMIMU_FUNC_VISIBILITY static inline
#endif

#ifndef REMIMU_CONST_VISIBILITY
#define REMIMU_CONST_VISIBILITY static const
#endif

#ifndef REMIMU_LOG_ERROR
#define REMIMU_LOG_ERROR puts
#endif

#ifndef REMIMU_ITERATION_LIMIT
#define REMIMU_ITERATION_LIMIT 0 // Set to non-zero to enable an interation limit
#endif

#ifndef REMIMU_ASSERT
#define REMIMU_ASSERT(x) assert(x)
#endif

/************

    REMIMU: SINGLE HEADER C/C++ REGEX LIBRARY

    Compatible with C99 and C++11 and later standards. Uses backtracking and relatively standard regex syntax.

    #include "remimu.h"

FUNCTIONS

    // Returns 0 on success, or -1 on invalid or unsupported regex, or -2 on not enough tokens given to parse regex.
    int regex_parse(
        const char * pattern,       // Regex pattern to parse.
        RegexToken * tokens,        // Output buffer of token_count regex tokens.
        int16_t * token_count,      // Maximum allowed number of tokens to write
        int32_t flags               // Optional bitflags.
    )

    // Returns match length, or -1 on no match, or -2 on out of memory, or -3 if the regex is invalid.
    int64_t regex_match(
        const RegexToken * tokens,  // Parsed regex to match against text.
        const char * text,          // Text to match against tokens.
        size_t start_i,             // index value to match at.
        uint16_t cap_slots,         // Number of allowed capture info output slots.
        int64_t * cap_pos,          // Capture position info output buffer.
        int64_t * cap_span          // Capture length info output buffer.
    )

    void print_regex_tokens(
        RegexToken * tokens     // Regex tokens to spew to stdout, for debugging.
    )

PERFORMANCE

    On simple cases, Remimu's match speed is similar to PCRE2. Regex parsing/compilation is also much faster (around 4x to 10x), so single-shot regexes are often faster than PCRE2.

    HOWEVER: Remimu is a pure backtracking engine, and has `O(2^x)` complexity on regexes with catastrophic backtracking. It can be much, much, MUCH slower than PCRE2. Beware!

    Remimu uses length-checked fixed memory buffers with no recursion, so memory usage is statically known.

FEATURES

    - Lowest-common-denominator common regex syntax
    - Based on backtracking (slow in the worst case, but fast in the best case)
    - 8-bit only, no utf-16 or utf-32
    - Statically known memory usage (no heap allocation or recursion)
    - Groups with or without capture, and with or without quantifiers
    - Supported escapes:
    - - 2-digit hex: e.g. \x00, \xFF, or lowercase, or mixed case
    - - \r, \n, \t, \v, \f (whitespace characters)
    - - \d, \s, \w, \D, \S, \W (digit, space, and word character classes)
    - - \b, \B word boundary and non-word-boundary anchors (not fully supported in zero-size quantified groups, but even then, usually supported)
    - - Escaped literal characters: {}[]-()|^$*+?:./\
    - - - Escapes work in character classes, except for 'b'
    - Character classes, including disjoint ranges, proper handling of bare [ and trailing -, etc
    - - Dot (.) matches all characters, including newlines, unless REMIMU_FLAG_DOT_NO_NEWLINES is passed as a flag to regex_parse
    - - Dot (.) only matches at most one byte at a time, so matching \r\n requires two dots (and not using REMIMU_FLAG_DOT_NO_NEWLINES)
    - Anchors (^ and $)
    - - Same support caveats as \b, \B apply
    - Basic quantifiers (*, +, ?)
    - - Quantifiers are greedy by default.
    - Explicit quantifiers ({2}, {5}, {5,}, {5,7})
    - Alternation e.g. (asdf|foo)
    - Lazy quantifiers e.g. (asdf)*? or \w+?
    - Possessive greedy quantifiers e.g. (asdf)*+ or \w++
    - - NOTE: Capture groups for and inside of possessive groups return no capture information.
    - Atomic groups e.g. (?>(asdf))
    - - NOTE: Capture groups inside of atomic groups return no capture information.

NOT SUPPORTED

    - Strings with non-terminal null characters
    - Unicode character classes (matching single utf-8 characters works regardless)
    - Exact POSIX regex semantics (posix-style greediness etc)
    - Backreferences
    - Lookbehind/Lookahead
    - Named groups
    - Most other weird flavor-specific regex stuff
    - Capture of or inside of possessive-quantified groups (still take up a capture slot, but no data is returned)

USAGE

    // minimal:

    RegexToken tokens[1024];
    int16_t token_count = 1024;
    int e = regex_parse("[0-9]+\\.[0-9]+", tokens, &token_count, 0);
    assert(!e);

    int64_t match_len = regex_match(tokens, "23.53) ", 0, 0, 0, 0);
    printf("########### return: %zd\n", match_len);

    // with captures:

    RegexToken tokens[256];
    int16_t token_count = sizeof(tokens)/sizeof(tokens[0]);
    int e = regex_parse("((a)|(b))++", tokens, &token_count, 0);
    assert(!e);

    int64_t cap_pos[5];
    int64_t cap_span[5];
    memset(cap_pos, 0xFF, sizeof(cap_pos));
    memset(cap_span, 0xFF, sizeof(cap_span));

    int64_t matchlen = regex_match(tokens, "aaaaaabbbabaqa", 0, 5, cap_pos, cap_span);
    printf("Match length: %zd\n", matchlen);
    for (int i = 0; i < 5; i++)
        printf("Capture %d: %zd plus %zd\n", i, cap_pos[i], cap_span[i]);

    // for debugging
    print_regex_tokens(tokens);

LICENSE

    Creative Commons Zero, public domain.

*/

#include <stdint.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>

REMIMU_CONST_VISIBILITY int REMIMU_FLAG_DOT_NO_NEWLINES = 1;

REMIMU_CONST_VISIBILITY uint8_t REMIMU_KIND_NORMAL      = 0;
REMIMU_CONST_VISIBILITY uint8_t REMIMU_KIND_OPEN        = 1;
REMIMU_CONST_VISIBILITY uint8_t REMIMU_KIND_NCOPEN      = 2;
REMIMU_CONST_VISIBILITY uint8_t REMIMU_KIND_CLOSE       = 3;
REMIMU_CONST_VISIBILITY uint8_t REMIMU_KIND_OR          = 4;
REMIMU_CONST_VISIBILITY uint8_t REMIMU_KIND_CARET       = 5;
REMIMU_CONST_VISIBILITY uint8_t REMIMU_KIND_DOLLAR      = 6;
REMIMU_CONST_VISIBILITY uint8_t REMIMU_KIND_BOUND       = 7;
REMIMU_CONST_VISIBILITY uint8_t REMIMU_KIND_NBOUND      = 8;
REMIMU_CONST_VISIBILITY uint8_t REMIMU_KIND_END         = 9;

REMIMU_CONST_VISIBILITY uint8_t REMIMU_MODE_POSSESSIVE  = 1;
REMIMU_CONST_VISIBILITY uint8_t REMIMU_MODE_LAZY        = 2;
REMIMU_CONST_VISIBILITY uint8_t REMIMU_MODE_INVERTED    = 128; // temporary; gets cleared later

typedef struct _RegexToken {
    uint8_t kind;
    uint8_t mode;
    uint16_t count_lo;
    uint16_t count_hi; // 0 means no limit
    uint16_t mask[16]; // for groups: mask 0 stores group-with-quantifier number (quantifiers are +, *, ?, {n}, {n,}, or {n,m})
    int16_t pair_offset; // from ( or ), offset in token list to matching paren. TODO: move into mask maybe
} RegexToken;

static int remimu_nibble_hex_to_bin(char hex, uint8_t *bin)
{
    if (hex >= '0' && hex <= '9')
    {
        *bin = hex - '0';
        return 0;
    }
    if (hex >= 'A' && hex <= 'F')
    {
        *bin = hex - 'A' + 10;
        return 0;
    }
    if (hex >= 'a' && hex <= 'f')
    {
        *bin = hex - 'a' + 10;
        return 0;
    }
    return -1; // invalid hex digit
}

/// Returns a negative number on failure:
/// -1: Regex string is invalid or using unsupported features or too long.
/// -2: Provided buffer not long enough. Give up, or reallocate with more length and retry.
/// Returns 0 on success.
/// On call, token_count pointer must point to the number of tokens that can be written to the tokens buffer.
/// On successful return, the number of actually used tokens is written to token_count.
/// Sets token_count to zero if a regex is not created but no error happened (e.g. empty pattern).
/// Flags: Not yet used.
/// SAFETY: Pattern must be null-terminated.
/// SAFETY: tokens buffer must have at least the input token_count number of RegexToken objects. They are allowed to be uninitialized.
REMIMU_FUNC_VISIBILITY int regex_parse(const char * pattern, RegexToken * tokens, int16_t * token_count, int32_t flags)
{
    int64_t tokens_len = *token_count;
    uint64_t pattern_len = strlen(pattern);
    if (token_count == 0)
        return -2;

    // 0: normal
    // 1: just saw a backslash
    int esc_state = 0;

    // 0: init
    // 1: normal
    // 2: in char class, initial state
    // 3: in char class, but possibly looking for a range marker
    // 4: in char class, but just saw a range marker
    // 5: immediately after quantifiable token
    // 6: immediately after quantifier

    const int STATE_NORMAL    = 1;
    const int STATE_QUANT     = 2;
    const int STATE_MODE      = 3;
    const int STATE_CC_INIT   = 4;
    const int STATE_CC_NORMAL = 5;
    const int STATE_CC_RANGE  = 6;
    int state = STATE_NORMAL;

    int char_class_mem = -1;

    RegexToken token;

    #define _REGEX_CLEAR_TOKEN() do { \
        memset(&token, 0, sizeof(RegexToken)); \
        token.count_lo = 1; \
        token.count_hi = 2; \
    } while(0)

    _REGEX_CLEAR_TOKEN();

    #define _REGEX_DO_INVERT() do { \
        for (int n = 0; n < 16; n++) \
            token.mask[n] = ~token.mask[n]; \
        token.mode &= ~REMIMU_MODE_INVERTED; \
    } while (0)

    int16_t k = 0;

    #define _REGEX_PUSH_TOKEN() do { \
        if (k == 0 || tokens[k-1].kind != token.kind || (token.kind != REMIMU_KIND_BOUND && token.kind != REMIMU_KIND_NBOUND)) \
        { \
            if (token.mode & REMIMU_MODE_INVERTED) _REGEX_DO_INVERT(); \
            if (k >= tokens_len) \
            { \
                REMIMU_LOG_ERROR("buffer overflow"); \
                return -2; \
            } \
            tokens[k++] = token; \
            _REGEX_CLEAR_TOKEN(); \
        } \
    } while (0)

    #define _REGEX_SET_MASK(byte) do { token.mask[((uint8_t)(byte))>>4] |= 1 << ((uint8_t)(byte) & 0xF); } while (0)
    #define _REGEX_SET_MASK_ALL() do { \
        for (int n = 0; n < 16; n++) \
            token.mask[n] = 0xFFFF; \
    } while (0)

    // start with an invisible group specifier
    // (this allows the matcher to not need to have a special root-level alternation operator case)
    token.kind = REMIMU_KIND_OPEN;
    token.count_lo = 0;
    token.count_hi = 0;

    int paren_count = 0;

    for (uint64_t i = 0; i < pattern_len; i++)
    {
        char c = pattern[i];
        if (state == STATE_QUANT)
        {
            state = STATE_MODE;
            if (c == '?')
            {
                token.count_lo = 0;
                token.count_hi = 2; // first non-allowed amount
                continue;
            }
            else if (c == '+')
            {
                token.count_lo = 1;
                token.count_hi = 0; // unlimited
                continue;
            }
            else if (c == '*')
            {
                token.count_lo = 0;
                token.count_hi = 0; // unlimited
                continue;
            }
            else if (c == '{')
            {
                if (pattern[i+1] == 0 || pattern[i+1] < '0' || pattern[i+1] > '9')
                    state = STATE_NORMAL;
                else
                {
                    i += 1;
                    uint32_t val = 0;
                    while (pattern[i] >= '0' && pattern[i] <= '9')
                    {
                        val *= 10;
                        val += (uint32_t)(pattern[i] - '0');
                        if (val > 0xFFFF)
                        {
                            REMIMU_LOG_ERROR("quantifier range too long");
                            return -1; // unsupported length
                        }
                        i += 1;
                    }
                    token.count_lo = val;
                    token.count_hi = val + 1;
                    if (pattern[i] == ',')
                    {
                        token.count_hi = 0; // unlimited
                        i += 1;

                        if (pattern[i] >= '0' && pattern[i] <= '9')
                        {
                            uint32_t val2 = 0;
                            while (pattern[i] >= '0' && pattern[i] <= '9')
                            {
                                val2 *= 10;
                                val2 += (uint32_t)(pattern[i] - '0');
                                if (val2 > 0xFFFF)
                                {
                                    REMIMU_LOG_ERROR("quantifier range too long");
                                    return -1; // unsupported length
                                }
                                i += 1;
                            }
                            if (val2 < val)
                            {
                                REMIMU_LOG_ERROR("quantifier range is backwards");
                                return -1; // unsupported length
                            }
                            token.count_hi = val2 + 1;
                        }
                    }

                    if (pattern[i] == '}')
                    {
                        // quantifier range parsed successfully
                        continue;
                    }
                    else
                    {
                        REMIMU_LOG_ERROR("quantifier range syntax broken (no terminator)");
                        return -1;
                    }
                }
            }
        }

        if (state == STATE_MODE)
        {
            state = STATE_NORMAL;
            if (c == '?')
            {
                token.mode |= REMIMU_MODE_LAZY;
                continue;
            }
            else if (c == '+')
            {
                token.mode |= REMIMU_MODE_POSSESSIVE;
                continue;
            }
        }

        if (state == STATE_NORMAL)
        {
            if (esc_state == 1)
            {
                esc_state = 0;
                if (c == 'n')
                    _REGEX_SET_MASK('\n');
                else if (c == 'r')
                    _REGEX_SET_MASK('\r');
                else if (c == 't')
                    _REGEX_SET_MASK('\t');
                else if (c == 'v')
                    _REGEX_SET_MASK('\v');
                else if (c == 'f')
                    _REGEX_SET_MASK('\f');
                else if (c == 'x')
                {
                    if (pattern[i+1] == 0 || pattern[i+2] == 0)
                        return -1; // too-short hex pattern
                    uint8_t n0, n1;
                    if (remimu_nibble_hex_to_bin(pattern[i+1], &n0))
                        return -1; // invalid hex
                    if (remimu_nibble_hex_to_bin(pattern[i+2], &n1))
                        return -1; // invalid hex
                    _REGEX_SET_MASK((n0 << 4) | n1);
                    i += 2;
                    state = STATE_QUANT;
                }
                else if (c == '{' || c == '}' ||
                         c == '[' || c == ']' || c == '-' ||
                         c == '(' || c == ')' ||
                         c == '|' || c == '^' || c == '$' ||
                         c == '*' || c == '+' || c == '?' || c == ':' ||
                         c == '.' || c == '/' || c == '\\')
                {
                    _REGEX_SET_MASK(c);
                    state = STATE_QUANT;
                }
                else if (c == 'd' || c == 's' || c == 'w' ||
                         c == 'D' || c == 'S' || c == 'W')
                {
                    uint8_t is_upper = c <= 'Z';

                    uint16_t m[16];
                    memset(m, 0, sizeof(m));

                    if (is_upper)
                        c += 0x20;
                    if (c == 'd' || c == 'w')
                        m[3] |= 0x03FF; // 0~7
                    if (c == 's')
                    {
                        m[0] |= 0x3E00; // \t-\r (includes \n, \v, and \f in the middle. 5 enabled bits.)
                        m[2] |= 1; // ' '
                    }
                    if (c == 'w')
                    {
                        m[4] |= 0xFFFE; // A-O
                        m[5] |= 0x87FF; // P-Z_
                        m[6] |= 0xFFFE; // a-o
                        m[7] |= 0x07FF; // p-z
                    }

                    for (int j = 0; j < 16; j++)
                        token.mask[j] |= is_upper ? ~m[j] : m[j];

                    token.kind = REMIMU_KIND_NORMAL;
                    state = STATE_QUANT;
                }
                else if (c == 'b')
                {
                    token.kind = REMIMU_KIND_BOUND;
                    state = STATE_NORMAL;
                }
                else if (c == 'B')
                {
                    token.kind = REMIMU_KIND_NBOUND;
                    state = STATE_NORMAL;
                }
                else
                {
                    REMIMU_LOG_ERROR("unsupported escape sequence");
                    return -1; // unknown/unsupported escape sequence
                }
            }
            else
            {
                _REGEX_PUSH_TOKEN();
                if (c == '\\')
                {
                    esc_state = 1;
                }
                else if (c == '[')
                {
                    state = STATE_CC_INIT;
                    char_class_mem = -1;
                    token.kind = REMIMU_KIND_NORMAL;
                    if (pattern[i + 1] == '^')
                    {
                        token.mode |= REMIMU_MODE_INVERTED;
                        i += 1;
                    }
                }
                else if (c == '(')
                {
                    paren_count += 1;
                    state = STATE_NORMAL;
                    token.kind = REMIMU_KIND_OPEN;
                    token.count_lo = 0;
                    token.count_hi = 1;
                    if (pattern[i + 1] == '?' && pattern[i + 2] == ':')
                    {
                        token.kind = REMIMU_KIND_NCOPEN;
                        i += 2;
                    }
                    else if (pattern[i + 1] == '?' && pattern[i + 2] == '>')
                    {
                        token.kind = REMIMU_KIND_NCOPEN;
                        _REGEX_PUSH_TOKEN();

                        state = STATE_NORMAL;
                        token.kind = REMIMU_KIND_NCOPEN;
                        token.mode = REMIMU_MODE_POSSESSIVE;
                        token.count_lo = 1;
                        token.count_hi = 2;

                        i += 2;
                    }
                }
                else if (c == ')')
                {
                    paren_count -= 1;
                    if (paren_count < 0 || k == 0)
                        return -1; // unbalanced parens
                    token.kind = REMIMU_KIND_CLOSE;
                    state = STATE_QUANT;

                    int balance = 0;
                    ptrdiff_t found = -1;
                    for (ptrdiff_t l = k - 1; l >= 0; l--)
                    {
                        if (tokens[l].kind == REMIMU_KIND_NCOPEN || tokens[l].kind == REMIMU_KIND_OPEN)
                        {
                            if (balance == 0)
                            {
                                found = l;
                                break;
                            }
                            else
                                balance -= 1;
                        }
                        else if (tokens[l].kind == REMIMU_KIND_CLOSE)
                            balance += 1;
                    }
                    if (found == -1)
                        return -1; // unbalanced parens
                    ptrdiff_t diff = k - found;
                    if (diff > 32767)
                        return -1; // too long
                    token.pair_offset = -diff;
                    tokens[found].pair_offset = diff;
                    // phantom group for atomic group emulation
                    if (tokens[found].mode == REMIMU_MODE_POSSESSIVE)
                    {
                        _REGEX_PUSH_TOKEN();
                        token.kind = REMIMU_KIND_CLOSE;
                        token.mode = REMIMU_MODE_POSSESSIVE;
                        token.pair_offset = -diff - 2;
                        tokens[found - 1].pair_offset = diff + 2;
                    }
                }
                else if (c == '?' || c == '+' || c == '*' || c == '{')
                {
                    REMIMU_LOG_ERROR("quantifier in non-quantifier context");
                    return -1; // quantifier in non-quantifier context
                }
                else if (c == '.')
                {
                    //puts("setting ALL of mask...");
                    _REGEX_SET_MASK_ALL();
                    if (flags & REMIMU_FLAG_DOT_NO_NEWLINES)
                    {
                        token.mask[1] ^= 0x04; // \n
                        token.mask[1] ^= 0x20; // \r
                    }
                    state = STATE_QUANT;
                }
                else if (c == '^')
                {
                    token.kind = REMIMU_KIND_CARET;
                    state = STATE_NORMAL;
                }
                else if (c == '$')
                {
                    token.kind = REMIMU_KIND_DOLLAR;
                    state = STATE_NORMAL;
                }
                else if (c == '|')
                {
                    token.kind = REMIMU_KIND_OR;
                    state = STATE_NORMAL;
                }
                else
                {
                    _REGEX_SET_MASK(c);
                    state = STATE_QUANT;
                }
            }
        }
        else if (state == STATE_CC_INIT || state == STATE_CC_NORMAL || state == STATE_CC_RANGE)
        {
            if (c == '\\' && esc_state == 0)
            {
                esc_state = 1;
                continue;
            }
            uint8_t esc_c = 0;
            if (esc_state == 1)
            {
                esc_state = 0;
                if (c == 'n')
                    esc_c = '\n';
                else if (c == 'r')
                    esc_c = '\r';
                else if (c == 't')
                    esc_c = '\t';
                else if (c == 'v')
                    esc_c = '\v';
                else if (c == 'f')
                    esc_c = '\f';
                else if (c == 'x')
                {
                    if (pattern[i+1] == 0 || pattern[i+2] == 0)
                        return -1; // too-short hex pattern
                    uint8_t n0, n1;
                    if (remimu_nibble_hex_to_bin(pattern[i+1], &n0))
                        return -1; // invalid hex
                    if (remimu_nibble_hex_to_bin(pattern[i+2], &n1))
                        return -1; // invalid hex
                    esc_c = (n0 << 4) | n1;
                    i += 2;
                }
                else if (c == '{' || c == '}' ||
                         c == '[' || c == ']' || c == '-' ||
                         c == '(' || c == ')' ||
                         c == '|' || c == '^' || c == '$' ||
                         c == '*' || c == '+' || c == '?' || c == ':' ||
                         c == '.' || c == '/' || c == '\\')
                {
                    esc_c = c;
                }
                else if (c == 'd' || c == 's' || c == 'w' ||
                         c == 'D' || c == 'S' || c == 'W')
                {
                    if (state == STATE_CC_RANGE)
                    {
                        REMIMU_LOG_ERROR("tried to use a shorthand as part of a range");
                        return -1; // range shorthands can't be part of a range
                    }
                    uint8_t is_upper = c <= 'Z';

                    uint16_t m[16];
                    memset(m, 0, sizeof(m));

                    if (is_upper)
                        c += 0x20;
                    if (c == 'd' || c == 'w')
                        m[3] |= 0x03FF; // 0~7
                    if (c == 's')
                    {
                        m[0] |= 0x3E00; // \t-\r (includes \n, \v, and \f in the middle. 5 enabled bits.)
                        m[2] |= 1; // ' '
                    }
                    if (c == 'w')
                    {
                        m[4] |= 0xFFFE; // A-O
                        m[5] |= 0x87FF; // P-Z_
                        m[6] |= 0xFFFE; // a-o
                        m[7] |= 0x07FF; // p-z
                    }

                    for (int j = 0; j < 16; j++)
                        token.mask[j] |= is_upper ? ~m[j] : m[j];

                    char_class_mem = -1; // range shorthands can't be part of a range
                    continue;
                }
                else
                {
                    printf("unknown/unsupported escape sequence in character class (\\%c)\n", c);
                    return -1; // unknown/unsupported escape sequence
                }
            }
            if (state == STATE_CC_INIT)
            {
                uint8_t val = esc_c ? esc_c : (uint8_t)c;
                char_class_mem = val;
                _REGEX_SET_MASK(val);
                state = STATE_CC_NORMAL;
            }
            else if (state == STATE_CC_NORMAL)
            {
                if (c == ']' && esc_c == 0)
                {
                    char_class_mem = -1;
                    state = STATE_QUANT;
                    continue;
                }
                else if (c == '-' && esc_c == 0 && char_class_mem >= 0)
                {
                    state = STATE_CC_RANGE;
                    continue;
                }
                else
                {
                    uint8_t val = esc_c ? esc_c : (uint8_t)c;
                    char_class_mem = val;
                    _REGEX_SET_MASK(val);
                    state = STATE_CC_NORMAL;
                }
            }
            else if (state == STATE_CC_RANGE)
            {
                if (c == ']' && esc_c == 0)
                {
                    char_class_mem = -1;
                    _REGEX_SET_MASK('-');
                    state = STATE_QUANT;
                    continue;
                }
                else
                {
                    if (char_class_mem == -1)
                    {
                        REMIMU_LOG_ERROR("character class range is broken");
                        return -1; // probably tried to use a character class shorthand as part of a range
                    }
                    uint8_t rhs = esc_c ? esc_c : (uint8_t)c;
                    if (rhs < (uint8_t)char_class_mem)
                    {
                        REMIMU_LOG_ERROR("character class range is misordered");
                        return -1; // range is in wrong order
                    }
                    //printf("enabling char class from %d to %d...\n", char_class_mem, c);
                    for (uint8_t j = rhs; j > (uint8_t)char_class_mem; j--)
                        _REGEX_SET_MASK(j);
                    state = STATE_CC_NORMAL;
                    char_class_mem = -1;
                }
            }
        }
        else
            REMIMU_ASSERT(0);
    }
    if (paren_count > 0)
    {
        REMIMU_LOG_ERROR("(paren_count > 0)");
        return -1; // unbalanced parens
    }
    if (esc_state != 0)
    {
        REMIMU_LOG_ERROR("(esc_state != 0)");
        return -1; // open escape sequence
    }
    if (state >= STATE_CC_INIT)
    {
        REMIMU_LOG_ERROR("(state >= STATE_CC_INIT)");
        return -1; // open character class
    }

    _REGEX_PUSH_TOKEN();

    // add invisible non-capturing group specifier
    token.kind = REMIMU_KIND_CLOSE;
    token.count_lo = 1;
    token.count_hi = 2;
    _REGEX_PUSH_TOKEN();

    // add end token (tells matcher that it's done)
    token.kind = REMIMU_KIND_END;
    _REGEX_PUSH_TOKEN();

    tokens[0].pair_offset = k - 2;
    tokens[k-2].pair_offset = -(k - 2);

    *token_count = k;

    // copy quantifiers from )s to (s (so (s know whether they're optional)
    // also take the opportunity to smuggle "quantified group index" into the mask field for the )
    uint64_t n = 0;
    for (int16_t k2 = 0; k2 < k; k2++)
    {
        if (tokens[k2].kind == REMIMU_KIND_CLOSE)
        {
            tokens[k2].mask[0] = n++;

            int16_t k3 = k2 + tokens[k2].pair_offset;
            tokens[k3].count_lo = tokens[k2].count_lo;
            tokens[k3].count_hi = tokens[k2].count_hi;
            tokens[k3].mask[0] = n++;
            tokens[k3].mode = tokens[k2].mode;

            //if (n > 65535)
            if (n > 1024)
                return -1; // too many quantified groups
        }
        else if (tokens[k2].kind == REMIMU_KIND_OR || tokens[k2].kind == REMIMU_KIND_OPEN || tokens[k2].kind == REMIMU_KIND_NCOPEN)
        {
            // find next | or ) and how far away it is. store in token
            int balance = 0;
            ptrdiff_t found = -1;
            for (ptrdiff_t l = k2 + 1; l < k; l++)
            {
                if (tokens[l].kind == REMIMU_KIND_OR && balance == 0)
                {
                    found = l;
                    break;
                }
                else if (tokens[l].kind == REMIMU_KIND_CLOSE)
                {
                    if (balance == 0)
                    {
                        found = l;
                        break;
                    }
                    else
                        balance -= 1;
                }
                else if (tokens[l].kind == REMIMU_KIND_NCOPEN || tokens[l].kind == REMIMU_KIND_OPEN)
                    balance += 1;
            }
            if (found == -1)
            {
                REMIMU_LOG_ERROR("unbalanced parens...");
                return -1; // unbalanced parens
            }
            ptrdiff_t diff = found - k2;
            if (diff > 32767)
            {
                REMIMU_LOG_ERROR("too long...");
                return -1; // too long
            }

            if (tokens[k2].kind == REMIMU_KIND_OR)
                tokens[k2].pair_offset = diff;
            else
                tokens[k2].mask[15] = diff;
        }
    }

    #undef _REGEX_PUSH_TOKEN
    #undef _REGEX_SET_MASK
    #undef _REGEX_CLEAR_TOKEN

    return 0;
}

typedef struct _RegexMatcherState {
    uint32_t k;
    uint32_t group_state; // quantified group temp state (e.g. number of repetitions)
    uint32_t prev; // for )s, stack index of corresponding previous quantified state
#ifdef REGEX_STACK_SMOL
    uint32_t i;
    uint32_t range_min;
    uint32_t range_max;
#else
    uint64_t i;
    uint64_t range_min;
    uint64_t range_max;
#endif
} RegexMatcherState;

// NOTE: undef'd later
#define _REGEX_CHECK_MASK(K, byte) (!!(tokens[K].mask[((uint8_t)byte)>>4] & (1 << ((uint8_t)byte & 0xF))))

// Returns match length if text starts with a regex match.
// Returns -1 if the text doesn't start with a regex match.
// Returns -2 if the matcher ran out of memory or the regex is too complex.
// Returns -3 if the regex is somehow invalid.
// The first cap_slots capture positions and spans (lengths) will be written to cap_pos and cap_span. If zero, will not be written to.
// SAFETY: The text variable must be null-terminated, and start_i must be the index of a character within the string or its null terminator.
// SAFETY: Tokens array must be terminated by a REMIMU_KIND_END token (done by default by regex_parse).
// SAFETY: Partial capture data may be written even if the match fails.
REMIMU_FUNC_VISIBILITY int64_t regex_match(const RegexToken * tokens, const char * text, size_t start_i, uint16_t cap_slots, int64_t * cap_pos, int64_t * cap_span)
{
    (void)text;

#ifdef REGEX_VERBOSE
    const uint8_t verbose = 1;
#else
    const uint8_t verbose = 0;
#endif

#define IF_VERBOSE(X) { if (verbose) { X } }

#ifdef REGEX_STACK_SMOL
    const uint16_t stack_size_max = 256;
#else
    const uint16_t stack_size_max = 1024;
#endif
    const uint16_t aux_stats_size = 1024;
    if (cap_slots > aux_stats_size)
        cap_slots = aux_stats_size;

    // quantified group state
    uint8_t q_group_accepts_zero[aux_stats_size];
    uint32_t q_group_state[aux_stats_size]; // number of repetitions
    uint32_t q_group_stack[aux_stats_size]; // location of most recent corresponding ) on stack. 0 means nowhere

    uint16_t q_group_cap_index[aux_stats_size];
    memset(q_group_cap_index, 0xFF, sizeof(q_group_cap_index));

    uint64_t tokens_len = 0;
    uint32_t k = 0;
    uint16_t caps = 0;

    while (tokens[k].kind != REMIMU_KIND_END)
    {
        if (tokens[k].kind == REMIMU_KIND_OPEN && caps < cap_slots)
        {
            q_group_cap_index[tokens[k].mask[0]] = caps;
            q_group_cap_index[tokens[k + tokens[k].pair_offset].mask[0]] = caps;
            cap_pos[caps] = -1;
            cap_span[caps] = -1;
            caps += 1;
        }
        k += 1;
        if (tokens[k].kind == REMIMU_KIND_CLOSE || tokens[k].kind == REMIMU_KIND_OPEN || tokens[k].kind == REMIMU_KIND_NCOPEN)
        {
            if (tokens[k].mask[0] >= aux_stats_size)
            {
                REMIMU_LOG_ERROR("too many qualified groups. returning");
                return -2; // OOM: too many quantified groups
            }

            q_group_state[tokens[k].mask[0]] = 0;
            q_group_stack[tokens[k].mask[0]] = 0;
            q_group_accepts_zero[tokens[k].mask[0]] = 0;
        }
    }

    tokens_len = k;

    RegexMatcherState rewind_stack[stack_size_max];
    uint16_t stack_n = 0;

    uint64_t i = start_i;

    uint64_t range_min = 0;
    uint64_t range_max = 0;
    uint8_t just_rewinded = 0;

    #define _P_TEXT_HIGHLIGHTED() do { \
        IF_VERBOSE(printf("\033[91m"); \
        for (uint64_t q = 0; q < i; q++) printf("%c", text[q]); \
        printf("\033[0m"); \
        for (uint64_t q = i; text[q] != 0; q++) printf("%c", text[q]); \
        printf("\n");) \
    } while (0)

    #define _REWIND_DO_SAVE_RAW(K, ISDUMMY) do { \
        if (stack_n >= stack_size_max) \
        { \
            REMIMU_LOG_ERROR("out of backtracking room. returning"); \
            return -2; \
        } \
        RegexMatcherState s; \
        memset(&s, 0, sizeof(RegexMatcherState)); \
        s.i = i; \
        s.k = (K); \
        s.range_min = range_min; \
        s.range_max = range_max; \
        s.prev = 0; \
        if (ISDUMMY) s.prev = 0xFAC7; \
        else if (tokens[s.k].kind == REMIMU_KIND_CLOSE) \
        { \
            s.group_state = q_group_state[tokens[s.k].mask[0]]; \
            s.prev = q_group_stack[tokens[s.k].mask[0]]; \
            q_group_stack[tokens[s.k].mask[0]] = stack_n; \
        } \
        rewind_stack[stack_n++] = s; \
        _P_TEXT_HIGHLIGHTED(); \
        IF_VERBOSE(printf("-- saving rewind state k %u i %zd rmin %zu rmax %zd (line %d) (depth %d prev %d)\n", s.k, i, range_min, range_max, __LINE__, stack_n, s.prev);) \
    } while (0)
    #define _REWIND_DO_SAVE_DUMMY(K) _REWIND_DO_SAVE_RAW(K, 1)
    #define _REWIND_DO_SAVE(K) _REWIND_DO_SAVE_RAW(K, 0)

    #define _REWIND_OR_ABORT() do { \
        if (stack_n == 0) \
            return -1; \
        stack_n -= 1; \
        while (stack_n > 0 && rewind_stack[stack_n].prev == 0xFAC7) stack_n -= 1; \
        just_rewinded = 1; \
        range_min = rewind_stack[stack_n].range_min; \
        range_max = rewind_stack[stack_n].range_max; \
        REMIMU_ASSERT(rewind_stack[stack_n].i <= i); \
        i = rewind_stack[stack_n].i; \
        k = rewind_stack[stack_n].k; \
        if (tokens[k].kind == REMIMU_KIND_CLOSE) \
        { \
            q_group_state[tokens[k].mask[0]] = rewind_stack[stack_n].group_state; \
            q_group_stack[tokens[k].mask[0]] = rewind_stack[stack_n].prev; \
        } \
        _P_TEXT_HIGHLIGHTED(); \
        IF_VERBOSE(printf("-- rewound to k %u i %zd rmin %zu rmax %zd (kind %d prev %d)\n", k, i, range_min, range_max, tokens[k].kind, rewind_stack[stack_n].prev);) \
        k -= 1; \
    } while (0)
    // the -= 1 is because of the k++ in the for loop

    // used in boundary anchor checker
    uint64_t w_mask[16];
    memset(w_mask, 0, sizeof(w_mask));
    w_mask[3] = 0x03FF;
    w_mask[4] = 0xFFFE;
    w_mask[5] = 0x87FF;
    w_mask[6] = 0xFFFE;
    w_mask[7] = 0x07FF;
    #define _REGEX_CHECK_IS_W(byte) (!!(w_mask[((uint8_t)byte)>>4] & (1 << ((uint8_t)byte & 0xF))))

    int limit = REMIMU_ITERATION_LIMIT;
    for (k = 0; k < tokens_len; k++)
    {
        if (REMIMU_ITERATION_LIMIT)
        {
            if (limit-- == 0)
            {
                REMIMU_LOG_ERROR("iteration limit exceeded. returning");
                return -2;
            }
        }
        IF_VERBOSE(printf("k: %u\ti: %zu\tl: %zu\tstack_n: %d\n", k, i, limit, stack_n);)
        _P_TEXT_HIGHLIGHTED();
        if (tokens[k].kind == REMIMU_KIND_CARET)
        {
            if (i != 0)
                _REWIND_OR_ABORT();
            continue;
        }
        else if (tokens[k].kind == REMIMU_KIND_DOLLAR)
        {
            if (text[i] != 0)
                _REWIND_OR_ABORT();
            continue;
        }
        else if (tokens[k].kind == REMIMU_KIND_BOUND)
        {
            if (i == 0 && !_REGEX_CHECK_IS_W(text[i]))
                _REWIND_OR_ABORT();
            else if (i != 0 && text[i] == 0 && !_REGEX_CHECK_IS_W(text[i-1]))
                _REWIND_OR_ABORT();
            else if (i != 0 && text[i] != 0 && _REGEX_CHECK_IS_W(text[i-1]) == _REGEX_CHECK_IS_W(text[i]))
                _REWIND_OR_ABORT();
        }
        else if (tokens[k].kind == REMIMU_KIND_NBOUND)
        {
            if (i == 0 && _REGEX_CHECK_IS_W(text[i]))
                _REWIND_OR_ABORT();
            else if (i != 0 && text[i] == 0 && _REGEX_CHECK_IS_W(text[i-1]))
                _REWIND_OR_ABORT();
            else if (i != 0 && text[i] != 0 && _REGEX_CHECK_IS_W(text[i-1]) != _REGEX_CHECK_IS_W(text[i]))
                _REWIND_OR_ABORT();
        }
        else
        {
            // deliberately unmatchable token (e.g. a{0}, a{0,0})
            if (tokens[k].count_hi == 1)
            {
                if (tokens[k].kind == REMIMU_KIND_OPEN || tokens[k].kind == REMIMU_KIND_NCOPEN)
                    k += tokens[k].pair_offset;
                else
                    k += 1;
                continue;
            }

            if (tokens[k].kind == REMIMU_KIND_OPEN || tokens[k].kind == REMIMU_KIND_NCOPEN)
            {
                if (!just_rewinded)
                {
                    IF_VERBOSE(printf("hit OPEN. i is %zd, depth is %d\n", i, stack_n);)
                    // need this to be able to detect and reject zero-size matches
                    //q_group_state[tokens[k].mask[0]] = i;

                    // if we're lazy and the min length is 0, we need to try the non-group case first
                    if ((tokens[k].mode & REMIMU_MODE_LAZY) && (tokens[k].count_lo == 0 || q_group_accepts_zero[tokens[k + tokens[k].pair_offset].mask[0]]))
                    {
                        IF_VERBOSE(puts("trying non-group case first.....");)
                        range_min = 0;
                        range_max = 0;
                        _REWIND_DO_SAVE(k);
                        k += tokens[k].pair_offset; // automatic += 1 will put us past the matching )
                    }
                    else
                    {
                        range_min = 1;
                        range_max = 0;
                        _REWIND_DO_SAVE(k);
                    }
                }
                else
                {
                    IF_VERBOSE(printf("rewinded into OPEN. i is %zd, depth is %d\n", i, stack_n);)
                    just_rewinded = 0;

                    uint64_t orig_k = k;

                    IF_VERBOSE(printf("--- trying to try another alternation, start k is %d, rmin is %zu\n", k, range_min);)

                    if (range_min != 0)
                    {
                        IF_VERBOSE(puts("rangemin is not zero. checking...");)
                        k += range_min;
                        IF_VERBOSE(printf("start kind: %d\n", tokens[k].kind);)
                        IF_VERBOSE(printf("before start kind: %d\n", tokens[k-1].kind);)
                        if (tokens[k-1].kind == REMIMU_KIND_OR)
                            k += tokens[k-1].pair_offset - 1;
                        else if (tokens[k-1].kind == REMIMU_KIND_OPEN || tokens[k-1].kind == REMIMU_KIND_NCOPEN)
                            k += tokens[k-1].mask[15] - 1;

                        IF_VERBOSE(printf("kamakama %d %d\n", k, tokens[k].kind);)

                        if (tokens[k].kind == REMIMU_KIND_END) // unbalanced parens
                            return -3;

                        IF_VERBOSE(printf("---?!?!   %d, %d\n", k, q_group_state[tokens[k].mask[0]]);)
                        if (tokens[k].kind == REMIMU_KIND_CLOSE)
                        {
                            IF_VERBOSE(puts("!!~!~!~~~~!!~~!~   hit CLOSE. rewinding");)
                            // do nothing and continue on if we don't need this group
                            if (tokens[k].count_lo == 0 || q_group_accepts_zero[tokens[k].mask[0]])
                            {
                                IF_VERBOSE(puts("continuing because we don't need this group");)
                                q_group_state[tokens[k].mask[0]] = 0;

                                if (!(tokens[k].mode & REMIMU_MODE_LAZY))
                                    q_group_stack[tokens[k].mask[0]] = 0;

                                continue;
                            }
                            // otherwise go to the last point before the group
                            else
                            {
                                IF_VERBOSE(puts("going to last point before this group");)
                                _REWIND_OR_ABORT();
                                continue;
                            }
                        }

                        REMIMU_ASSERT(tokens[k].kind == REMIMU_KIND_OR);
                    }

                    IF_VERBOSE(printf("--- FOUND ALTERNATION for paren at k %zd at k %d\n", orig_k, k);)

                    ptrdiff_t k_diff = k - orig_k;
                    range_min = k_diff + 1;

                    IF_VERBOSE(puts("(saving in paren after rewinding and looking for next regex token to check)");)
                    IF_VERBOSE(printf("%zd\n", range_min);)
                    _REWIND_DO_SAVE(k - k_diff);
                }
            }
            else if (tokens[k].kind == REMIMU_KIND_CLOSE)
            {
                // unquantified
                if (tokens[k].count_lo == 1 && tokens[k].count_hi == 2)
                {
                    // for captures
                    uint16_t cap_index = q_group_cap_index[tokens[k].mask[0]];
                    if (cap_index != 0xFFFF)
                        _REWIND_DO_SAVE_DUMMY(k);
                }
                // quantified
                else
                {
                    IF_VERBOSE(puts("closer test.....");)
                    if (!just_rewinded)
                    {
                        uint32_t prev = q_group_stack[tokens[k].mask[0]];

                        IF_VERBOSE(printf("qrqrqrqrqrqrqrq-------      k %d, gs %d, gaz %d, i %zd, tklo %d, rmin %zd, tkhi %d, rmax %zd, prev %d, sn %d\n", k, q_group_state[tokens[k].mask[0]], q_group_accepts_zero[tokens[k].mask[0]], i, tokens[k].count_lo, range_min, tokens[k].count_hi, range_max, prev, stack_n);)

                        range_max = tokens[k].count_hi;
                        range_max -= 1;
                        range_min = q_group_accepts_zero[tokens[k].mask[0]] ? 0 : tokens[k].count_lo;
                        //REMIMU_ASSERT(q_group_state[tokens[k + tokens[k].pair_offset].mask[0]] <= i);
                        //if (prev) REMIMU_ASSERT(rewind_stack[prev].i <= i);
                        IF_VERBOSE(printf("qzqzqzqzqzqzqzq-------      rmin %zd, rmax %zd\n", range_min, range_max);)

                        // minimum requirement not yet met
                        if (q_group_state[tokens[k].mask[0]] + 1 < range_min)
                        {
                            IF_VERBOSE(puts("continuing minimum matches for a quantified group");)
                            q_group_state[tokens[k].mask[0]] += 1;
                            _REWIND_DO_SAVE(k);

                            k += tokens[k].pair_offset; // back to start of group
                            k -= 1; // ensure we actually hit the group node next and not the node after it
                            continue;
                        }
                        // maximum allowance exceeded
                        else if (tokens[k].count_hi != 0 && q_group_state[tokens[k].mask[0]] + 1 > range_max)
                        {
                            IF_VERBOSE(printf("hit maximum allowed instances of a quantified group %d %zd\n", q_group_state[tokens[k].mask[0]], range_max);)
                            range_max -= 1;
                            _REWIND_OR_ABORT();
                            continue;
                        }

                        // fallback case to detect zero-length matches when we backtracked into the inside of this group
                        // after an attempted parse of a second copy of itself
                        uint8_t force_zero = 0;
                        if (prev != 0 && rewind_stack[prev].i > i)
                        {
                            // find matching open paren
                            size_t n = stack_n - 1;
                            while (n > 0 && rewind_stack[n].k != k + tokens[k].pair_offset)
                                n -= 1;
                            REMIMU_ASSERT(n > 0);
                            if (rewind_stack[n].i == i)
                                force_zero = 1;
                        }

                        // reject zero-length matches
                        if ((force_zero || (prev != 0 && rewind_stack[prev].i == i))) //  && q_group_state[tokens[k].mask[0]] > 0
                        {
                            IF_VERBOSE(printf("rejecting zero-length match..... %d %zd %zd\n", force_zero, rewind_stack[prev].i, i);)
                            IF_VERBOSE(printf("%d (k: %d)\n", q_group_state[tokens[k].mask[0]], k);)

                            q_group_accepts_zero[tokens[k].mask[0]] = 1;
                            _REWIND_OR_ABORT();
                            //range_max = q_group_state[tokens[k].mask[0]];
                            //range_min = 0;
                        }
                        else if (tokens[k].mode & REMIMU_MODE_LAZY) // lazy
                        {
                            IF_VERBOSE(printf("nidnfasidfnidfndifn-------      %d, %d, %zd\n", q_group_state[tokens[k].mask[0]], tokens[k].count_lo, range_min);)
                            if (prev)
                                IF_VERBOSE(printf("lazy doesn't think it's zero-length. prev i %zd vs i %zd (depth %d)\n", rewind_stack[prev].i, i, stack_n);)
                            // continue on to past the group; group retry is in rewind state
                            q_group_state[tokens[k].mask[0]] += 1;
                            _REWIND_DO_SAVE(k);
                            q_group_state[tokens[k].mask[0]] = 0;
                        }
                        else // greedy
                        {
                            IF_VERBOSE(puts("wahiwahi");)
                            // clear unwanted memory if possessive
                            if ((tokens[k].mode & REMIMU_MODE_POSSESSIVE))
                            {
                                uint32_t k2 = k;

                                // special case for first, only rewind to (, not to )
                                if (q_group_state[tokens[k].mask[0]] == 0)
                                    k2 = k + tokens[k].pair_offset;

                                if (stack_n == 0)
                                    return -1;
                                stack_n -= 1;

                                while (stack_n > 0 && rewind_stack[stack_n].k != k2)
                                    stack_n -= 1;

                                if (stack_n == 0)
                                    return -1;
                            }
                            // continue to next match if sane
                            if ((uint32_t)q_group_state[tokens[k + tokens[k].pair_offset].mask[0]] < (uint32_t)i)
                            {
                                IF_VERBOSE(puts("REWINDING FROM GREEDY NON-REWIND CLOSER");)
                                q_group_state[tokens[k].mask[0]] += 1;
                                _REWIND_DO_SAVE(k);
                                k += tokens[k].pair_offset; // back to start of group
                                k -= 1; // ensure we actually hit the group node next and not the node after it
                            }
                            else
                                IF_VERBOSE(puts("CONTINUING FROM GREEDY NON-REWIND CLOSER");)
                        }
                    }
                    else
                    {
                        IF_VERBOSE(puts("IN CLOSER REWIND!!!");)
                        just_rewinded = 0;

                        if (tokens[k].mode & REMIMU_MODE_LAZY)
                        {
                            // lazy rewind: need to try matching the group again
                            _REWIND_DO_SAVE_DUMMY(k);
                            q_group_stack[tokens[k].mask[0]] = stack_n;
                            k += tokens[k].pair_offset; // back to start of group
                            k -= 1; // ensure we actually hit the group node next and not the node after it
                        }
                        else
                        {
                            // greedy. if we're going to go outside the acceptable range, rewind
                            IF_VERBOSE(printf("kufukufu %d %zd\n", tokens[k].count_lo, range_min);)
                            //uint64_t old_i = i;
                            if (q_group_state[tokens[k].mask[0]] < range_min && !q_group_accepts_zero[tokens[k].mask[0]])
                            {
                                IF_VERBOSE(printf("rewinding from greedy group because we're going to go out of range (%d vs %zd)\n", q_group_state[tokens[k].mask[0]], range_min);)
                                //i = old_i;
                                _REWIND_OR_ABORT();
                            }
                            // otherwise continue on to past the group
                            else
                            {
                                IF_VERBOSE(puts("continuing past greedy group");)
                                q_group_state[tokens[k].mask[0]] = 0;

                                // for captures
                                uint16_t cap_index = q_group_cap_index[tokens[k].mask[0]];
                                if (cap_index != 0xFFFF)
                                    _REWIND_DO_SAVE_DUMMY(k);
                            }
                        }
                    }
                }
            }
            else if (tokens[k].kind == REMIMU_KIND_OR)
            {
                IF_VERBOSE(printf("hit OR at %d. adding %d\n", k, tokens[k].pair_offset);)
                k += tokens[k].pair_offset;
                k -= 1;
            }
            else if (tokens[k].kind == REMIMU_KIND_NORMAL)
            {
                if (!just_rewinded)
                {
                    uint64_t n = 0;
                    // do whatever the obligatory minimum amount of matching is
                    uint64_t old_i = i;
                    while (n < tokens[k].count_lo && text[i] != 0 && _REGEX_CHECK_MASK(k, text[i]))
                    {
                        i += 1;
                        n += 1;
                    }
                    if (n < tokens[k].count_lo)
                    {
                        IF_VERBOSE(printf("non-match A. rewinding (token %d)\n", k);)
                        i = old_i;
                        _REWIND_OR_ABORT();
                        continue;
                    }

                    if (tokens[k].mode & REMIMU_MODE_LAZY)
                    {
                        range_min = n;
                        range_max = tokens[k].count_hi - 1;
                        _REWIND_DO_SAVE(k);
                    }
                    else
                    {
                        uint64_t ilimit = tokens[k].count_hi;
                        if (ilimit == 0)
                            ilimit = ~ilimit;
                        range_min = n;
                        while (text[i] != 0 && _REGEX_CHECK_MASK(k, text[i]) && n + 1 < ilimit)
                        {
                            IF_VERBOSE(printf("match!! (%c)\n", text[i]);)
                            i += 1;
                            n += 1;
                        }
                        range_max = n;
                        IF_VERBOSE(printf("set rmin to %zd and rmax to %zd on entry into normal greedy token with k %d\n", range_min, range_max, k);)
                        if (!(tokens[k].mode & REMIMU_MODE_POSSESSIVE))
                            _REWIND_DO_SAVE(k);
                    }
                }
                else
                {
                    just_rewinded = 0;

                    if (tokens[k].mode & REMIMU_MODE_LAZY)
                    {
                        uint64_t ilimit = range_max;
                        if (ilimit == 0)
                            ilimit = ~ilimit;

                        if (_REGEX_CHECK_MASK(k, text[i]) && text[i] != 0 && range_min < ilimit)
                        {
                            IF_VERBOSE(printf("match2!! (%c) (k: %d)\n", text[i], k);)
                            i += 1;
                            range_min += 1;
                            _REWIND_DO_SAVE(k);
                        }
                        else
                        {
                            IF_VERBOSE(printf("core rewind lazy (k: %d)\n", k);)
                            _REWIND_OR_ABORT();
                        }
                    }
                    else
                    {
                        //IF_VERBOSE(printf("comparing rmin %zd and rmax %zd token with k %d\n", range_min, range_max, k);)
                        if (range_max > range_min)
                        {
                            IF_VERBOSE(printf("greedy normal going back (k: %d)\n", k);)
                            i -= 1;
                            range_max -= 1;
                            _REWIND_DO_SAVE(k);
                        }
                        else
                        {
                            IF_VERBOSE(printf("core rewind greedy (k: %d)\n", k);)
                            _REWIND_OR_ABORT();
                        }
                    }
                }
            }
            else
            {
                fprintf(stderr, "unimplemented token kind %d\n", tokens[k].kind);
                REMIMU_ASSERT(0);
            }
        }
        //printf("k... %d\n", k);
    }

    if (caps != 0)
    {
        //printf("stack_n: %d\n", stack_n);
        fflush(stdout);
        for (size_t n = 0; n < stack_n; n++)
        {
            RegexMatcherState s = rewind_stack[n];
            int kind = tokens[s.k].kind;
            if (kind == REMIMU_KIND_OPEN || kind == REMIMU_KIND_CLOSE)
            {
                uint16_t cap_index = q_group_cap_index[tokens[s.k].mask[0]];
                if (cap_index == 0xFFFF)
                    continue;
                if (tokens[s.k].kind == REMIMU_KIND_OPEN)
                    cap_pos[cap_index] = s.i;
                else if (cap_pos[cap_index] >= 0)
                    cap_span[cap_index] = s.i - cap_pos[cap_index];
            }
        }
        // re-deinitialize capture positions that have no associated capture span
        for (size_t n = 0; n < caps; n++)
        {
            if (cap_span[n] == -1)
                cap_pos[n] = -1;
        }
    }

    #undef _REWIND_DO_SAVE
    #undef _REWIND_OR_ABORT
    #undef _REGEX_CHECK_IS_W
    #undef _P_TEXT_HIGHLIGHTED
    #undef IF_VERBOSE

    return i;
}

REMIMU_FUNC_VISIBILITY void print_regex_tokens(RegexToken * tokens)
{
    const char * kind_to_str[] = {
        "NORMAL",
        "OPEN",
        "NCOPEN",
        "CLOSE",
        "OR",
        "CARET",
        "DOLLAR",
        "BOUND",
        "NBOUND",
        "END",
    };
    const char * mode_to_str[] = {
        "GREEDY",
        "POSSESS",
        "LAZY",
    };
    for (int k = 0;; k++)
    {
        printf("%s\t%s\t", kind_to_str[tokens[k].kind], mode_to_str[tokens[k].mode]);

        int c_old = -1;
        for (int c = 0; c < (tokens[k].kind ? 0 : 256); c++)
        {
            #define _PRINT_C_SMART(c) { \
                if (c >= 0x20 && c <= 0x7E) \
                    printf("%c", c); \
                else \
                    printf("\\x%02x", c); \
            }

            if (_REGEX_CHECK_MASK(k, c))
            {
                if (c_old == -1)
                    c_old = c;
            }
            else if (c_old != -1)
            {
                if (c - 1 == c_old)
                {
                    _PRINT_C_SMART(c_old)
                    c_old = -1;
                }
                else if (c - 2 == c_old)
                {
                    _PRINT_C_SMART(c_old)
                    _PRINT_C_SMART(c_old + 1)
                    c_old = -1;
                }
                else
                {
                    _PRINT_C_SMART(c_old)
                    printf("-");
                    _PRINT_C_SMART(c - 1)
                    c_old = -1;
                }
            }
        }

        /*
        printf("\t");
        for (int i = 0; i < 16; i++)
            printf("%04x", tokens[k].mask[i]);
        */

        printf("\t{%d,%d}\t(%d)\n", tokens[k].count_lo, tokens[k].count_hi - 1, tokens[k].pair_offset);

        if (tokens[k].kind == REMIMU_KIND_END)
            break;
    }
}

#undef _REGEX_CHECK_MASK

#endif //INCLUDE_REMIMU
// ------------------------------------------------------------------------------------------

#define IM 139968
#define IA 3877
#define IC 29573
#define INIT 42

extern char **global_order;
extern size_t global_order_count;

void Helper_reset(void);
uint32_t Helper_next_int(uint32_t max);
uint32_t Helper_next_int_range(uint32_t from, uint32_t to);
double Helper_next_float(double max);

uint32_t Helper_checksum_string(const char *v);
uint32_t Helper_checksum_bytes(const uint8_t *data, size_t length);
uint32_t Helper_checksum_f64(double v);

void Helper_load_config(const char *filename);
void Helper_free_config(void);
int64_t Helper_config_i64(const char *class_name, const char *field_name);
const char *Helper_config_s(const char *class_name, const char *field_name);

typedef struct Benchmark Benchmark;

struct Benchmark {
  const char *name;
  uint32_t checksum_val;
  int64_t iterations_val;

  void (*prepare)(Benchmark *self);
  void (*run)(Benchmark *self, int iteration_id);
  void (*run_all)(Benchmark *self);
  void (*warmup)(Benchmark *self);
  uint32_t (*checksum)(Benchmark *self);
  int64_t (*iterations)(Benchmark *self);
  int64_t (*expected_checksum)(Benchmark *self);
  void (*cleanup)(Benchmark *self);

  void *data;
};

void Benchmark_default_prepare(Benchmark *self);
void Benchmark_default_warmup(Benchmark *self);
void Benchmark_default_run_all(Benchmark *self);
uint32_t Benchmark_default_checksum(Benchmark *self);
int64_t Benchmark_default_iterations(Benchmark *self);
int64_t Benchmark_default_expected_checksum(Benchmark *self);
void Benchmark_default_cleanup(Benchmark *self);
Benchmark *Benchmark_create(const char *name);

typedef struct {
  char name[100];
  Benchmark *(*create)(void);
} BenchmarkFactory;

extern BenchmarkFactory *benchmark_factories;
extern size_t benchmark_factories_count;
extern size_t benchmark_factories_capacity;

void Benchmark_register(const char *name, Benchmark *(*factory)(void));
void Benchmark_all(const char *single_bench);


void Benchmark_default_prepare(Benchmark *self) { (void)self; }

void Benchmark_default_warmup(Benchmark *self) {
  int64_t warmup_iters = Helper_config_i64(self->name, "warmup_iterations");
  if (warmup_iters == 0) {
    warmup_iters = self->iterations(self);
    warmup_iters = (int64_t)(warmup_iters * 0.2);
    if (warmup_iters < 1)
      warmup_iters = 1;
  }

  for (int64_t i = 0; i < warmup_iters; i++) {
    self->run(self, i);
  }
}

void Benchmark_default_run_all(Benchmark *self) {
  int64_t iters = self->iterations(self);
  for (int64_t i = 0; i < iters; i++) {
    self->run(self, i);
  }
}

uint32_t Benchmark_default_checksum(Benchmark *self) {
  return self->checksum_val;
}

int64_t Benchmark_default_iterations(Benchmark *self) {
  if (self->iterations_val > 0) {
    return self->iterations_val;
  }
  self->iterations_val = Helper_config_i64(self->name, "iterations");
  return self->iterations_val;
}

int64_t Benchmark_default_expected_checksum(Benchmark *self) {
  return Helper_config_i64(self->name, "checksum");
}

void Benchmark_default_cleanup(Benchmark *self) { (void)self; }

Benchmark *Benchmark_create(const char *name) {
  Benchmark *bench = malloc(sizeof(Benchmark));
  bench->name = name;
  bench->checksum_val = 0;
  bench->iterations_val = 0;
  bench->data = NULL;

  bench->prepare = Benchmark_default_prepare;
  bench->warmup = Benchmark_default_warmup;
  bench->run_all = Benchmark_default_run_all;
  bench->checksum = Benchmark_default_checksum;
  bench->iterations = Benchmark_default_iterations;
  bench->expected_checksum = Benchmark_default_expected_checksum;
  bench->cleanup = Benchmark_default_cleanup;

  return bench;
}

BenchmarkFactory *benchmark_factories = NULL;
size_t benchmark_factories_count = 0;
size_t benchmark_factories_capacity = 0;

void Benchmark_register(const char *name, Benchmark *(*factory)(void)) {
  if (benchmark_factories_count >= benchmark_factories_capacity) {
    benchmark_factories_capacity =
        benchmark_factories_capacity ? benchmark_factories_capacity * 2 : 16;
    benchmark_factories =
        realloc(benchmark_factories,
                sizeof(BenchmarkFactory) * benchmark_factories_capacity);
  }

  strncpy(benchmark_factories[benchmark_factories_count].name, name,
          sizeof(benchmark_factories[benchmark_factories_count].name) - 1);
  benchmark_factories[benchmark_factories_count]
      .name[sizeof(benchmark_factories[benchmark_factories_count].name) - 1] =
      '\0';
  benchmark_factories[benchmark_factories_count].create = factory;
  benchmark_factories_count++;
}

void Benchmark_all(const char *single_bench) {
  struct timespec start, end;
  double summary_time = 0.0;
  int ok = 0;
  int fails = 0;

  for (size_t i = 0; i < global_order_count; i++) {
    const char *bench_name = global_order[i];

    if (single_bench && strlen(single_bench) > 0) {
      const char *haystack = bench_name;
      const char *needle = single_bench;

      char haystack_lower[100];
      char needle_lower[100];
      strncpy(haystack_lower, haystack, sizeof(haystack_lower) - 1);
      haystack_lower[sizeof(haystack_lower) - 1] = '\0';
      strncpy(needle_lower, needle, sizeof(needle_lower) - 1);
      needle_lower[sizeof(needle_lower) - 1] = '\0';

      for (char *p = haystack_lower; *p; p++)
        *p = tolower(*p);
      for (char *p = needle_lower; *p; p++)
        *p = tolower(*p);

      if (strstr(haystack_lower, needle_lower) == NULL) {
        continue;
      }
    }

    BenchmarkFactory *factory = NULL;
    for (size_t j = 0; j < benchmark_factories_count; j++) {
      if (strcmp(benchmark_factories[j].name, bench_name) == 0) {
        factory = &benchmark_factories[j];
        break;
      }
    }

    if (!factory) {
      printf(
          "Warning: Benchmark '%s' defined in config but not found in code\n",
          bench_name);
      continue;
    }

    printf("%s: ", bench_name);

    Benchmark *bench = factory->create();

    Helper_reset();

    bench->prepare(bench);

    bench->warmup(bench);

    Helper_reset();
    clock_gettime(CLOCK_MONOTONIC, &start);

    bench->run_all(bench);

    clock_gettime(CLOCK_MONOTONIC, &end);
    double duration =
        (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) * 1e-9;

    summary_time += duration;

    uint32_t actual_checksum = bench->checksum(bench);
    uint32_t expected_checksum = (uint32_t)bench->expected_checksum(bench);

    if (actual_checksum == expected_checksum) {
      printf("OK ");
      ok++;
    } else {
      printf("ERR[actual=%u, expected=%u] ", actual_checksum,
             (unsigned int)expected_checksum);
      fails++;
    }

    printf("in %.3fs\n", duration);

    bench->cleanup(bench);
    free(bench->data);
    free(bench);

    usleep(1000);
  }

  if (ok + fails > 0) {
    printf("Summary: %.4fs, %d, %d, %d\n", summary_time, ok + fails, ok, fails);
  }

  if (fails > 0) {
    exit(1);
  }
}


static const char b64_alphabet[] = 
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static size_t b64_encode(char *dst, const char *src, size_t src_size) {
    size_t i, j;
    uint32_t octet1, octet2, octet3;
    
    for (i = 0, j = 0; i < src_size; i += 3) {
        octet1 = (uint8_t)src[i];
        octet2 = (i + 1 < src_size) ? (uint8_t)src[i + 1] : 0;
        octet3 = (i + 2 < src_size) ? (uint8_t)src[i + 2] : 0;
        
        dst[j++] = b64_alphabet[octet1 >> 2];
        dst[j++] = b64_alphabet[((octet1 & 0x03) << 4) | (octet2 >> 4)];
        dst[j++] = (i + 1 < src_size) ? 
            b64_alphabet[((octet2 & 0x0F) << 2) | (octet3 >> 6)] : '=';
        dst[j++] = (i + 2 < src_size) ? 
            b64_alphabet[octet3 & 0x3F] : '=';
    }
    
    return j;
}

static const uint8_t b64_decode_table[256] = {
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 62,  255, 62,  255, 63,
    52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  255, 255, 255, 255, 255, 255,
    255, 0,   1,   2,   3,   4,   5,   6,   7,   8,   9,   10,  11,  12,  13,  14,
    15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  255, 255, 255, 255, 63,
    255, 26,  27,  28,  29,  30,  31,  32,  33,  34,  35,  36,  37,  38,  39,  40,
    41,  42,  43,  44,  45,  46,  47,  48,  49,  50,  51,  255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255
};

static size_t b64_decode(char *dst, const char *src, size_t src_size) {
    size_t i = 0, j = 0;
    while (src_size > 0 && src[src_size - 1] == '=') {
        src_size--;
    }
    size_t full = src_size & ~(size_t)3;

    for (; i < full; i += 4) {
        uint32_t a = b64_decode_table[(uint8_t)src[i]];
        uint32_t b = b64_decode_table[(uint8_t)src[i + 1]];
        uint32_t c = b64_decode_table[(uint8_t)src[i + 2]];
        uint32_t d = b64_decode_table[(uint8_t)src[i + 3]];

        if ((a | b | c | d) == 255) {
            return 0;
        }

        uint32_t v = (a << 18) | (b << 12) | (c << 6) | d;

        dst[j++] = (uint8_t)(v >> 16);
        dst[j++] = (uint8_t)(v >> 8);
        dst[j++] = (uint8_t)v;
    }

    if (i >= src_size) {
        return j;
    }

    uint32_t v = 0;
    int nbytes = 0;

    for (; i < src_size; i++) {
        uint8_t ch = (uint8_t)src[i];
        if (ch == '=') {
            break;
        }
        uint32_t val = b64_decode_table[ch];
        if (val == 255) {
            return 0;
        }
        v = (v << 6) | val;
        nbytes++;
    }

    if (nbytes < 2) {
        return 0;
    }

    v <<= (4 - nbytes) * 6;

    dst[j++] = (uint8_t)(v >> 16);
    if (nbytes >= 3) {
        dst[j++] = (uint8_t)(v >> 8);
    }
    if (nbytes >= 4) {
        dst[j++] = (uint8_t)v;
    }

    return j;
}

static size_t b64_encode_size(size_t size) {
    return (size_t)(size * 4 / 3.0) + 6;
}

static size_t b64_decode_size(size_t size) {
    return (size_t)(size * 3 / 4.0) + 6;
}

typedef struct {
    char *input_str;
    size_t input_len;
    char *encoded_str;
    size_t encoded_len;
    uint32_t result_val;
} Base64EncodeData;

void Base64Encode_prepare(Benchmark *self) {
    Base64EncodeData *data = (Base64EncodeData *)self->data;

    int64_t n = Helper_config_i64(self->name, "size");
    if (n <= 0) {
        n = 100;
    }

    data->input_len = (size_t)n;
    data->input_str = malloc(data->input_len + 1);
    memset(data->input_str, 'a', data->input_len);
    data->input_str[data->input_len] = '\0';

    data->encoded_len = b64_encode_size(data->input_len);
    data->encoded_str = malloc(data->encoded_len);
    data->encoded_len = b64_encode(data->encoded_str, data->input_str, data->input_len);

    data->result_val = 0;
}

void Base64Encode_run(Benchmark *self, int iteration_id) {
    (void)iteration_id;
    Base64EncodeData *data = (Base64EncodeData *)self->data;

    size_t encoded_size = b64_encode_size(data->input_len);
    char *encoded_buf = (char *)malloc(encoded_size);

    size_t actual_len = b64_encode(encoded_buf, data->input_str, data->input_len);

    data->result_val += (uint32_t)actual_len;
    free(encoded_buf);
}

uint32_t Base64Encode_checksum(Benchmark *self) {
    Base64EncodeData *data = (Base64EncodeData *)self->data;

    char result_str[256];

    char input_preview[32];
    if (data->input_len > 4) {
        strncpy(input_preview, data->input_str, 4);
        input_preview[4] = '.';
        input_preview[5] = '.';
        input_preview[6] = '.';
        input_preview[7] = '\0';
    } else {
        strncpy(input_preview, data->input_str, data->input_len);
        input_preview[data->input_len] = '\0';
    }

    char encoded_preview[32];
    if (data->encoded_len > 4) {
        strncpy(encoded_preview, data->encoded_str, 4);
        encoded_preview[4] = '.';
        encoded_preview[5] = '.';
        encoded_preview[6] = '.';
        encoded_preview[7] = '\0';
    } else {
        strncpy(encoded_preview, data->encoded_str, data->encoded_len);
        encoded_preview[data->encoded_len] = '\0';
    }

    snprintf(result_str, sizeof(result_str), "encode %s to %s: %u", input_preview,
             encoded_preview, data->result_val);

    return Helper_checksum_string(result_str);
}

void Base64Encode_cleanup(Benchmark *self) {
    Base64EncodeData *data = (Base64EncodeData *)self->data;
    if (data->input_str)
        free(data->input_str);
    if (data->encoded_str)
        free(data->encoded_str);
}

Benchmark *Base64Encode_create(void) {
    Benchmark *bench = Benchmark_create("Base64::Encode");

    Base64EncodeData *data = malloc(sizeof(Base64EncodeData));
    memset(data, 0, sizeof(Base64EncodeData));

    bench->data = data;

    bench->prepare = Base64Encode_prepare;
    bench->run = Base64Encode_run;
    bench->checksum = Base64Encode_checksum;
    bench->cleanup = Base64Encode_cleanup;

    return bench;
}

typedef struct {
    char *encoded_str;
    size_t encoded_len;
    char *decoded_str;
    size_t decoded_len;
    uint32_t result_val;
    size_t total_decoded_size;
} Base64DecodeData;

void Base64Decode_prepare(Benchmark *self) {
    Base64DecodeData *data = (Base64DecodeData *)self->data;

    int64_t n = Helper_config_i64(self->name, "size");
    if (n <= 0) {
        n = 100;
    }

    size_t input_len = (size_t)n;
    char *input_str = malloc(input_len + 1);
    memset(input_str, 'a', input_len);
    input_str[input_len] = '\0';

    size_t encoded_size = b64_encode_size(input_len);
    data->encoded_str = malloc(encoded_size);
    size_t actual_encoded = b64_encode(data->encoded_str, input_str, input_len);
    data->encoded_len = actual_encoded;

    size_t decoded_size = b64_decode_size(data->encoded_len);
    data->decoded_str = malloc(decoded_size);
    data->decoded_len = b64_decode(data->decoded_str, data->encoded_str, data->encoded_len);

    data->result_val = 0;

    free(input_str);
}

void Base64Decode_run(Benchmark *self, int iteration_id) {
    (void)iteration_id;
    Base64DecodeData *data = (Base64DecodeData *)self->data;

    size_t decoded_size = b64_decode_size(data->encoded_len);
    char *decoded_buf = (char *)malloc(decoded_size);

    size_t actual_len = b64_decode(decoded_buf, data->encoded_str, data->encoded_len);

    data->result_val += (uint32_t)actual_len;
    free(decoded_buf);
}

uint32_t Base64Decode_checksum(Benchmark *self) {
    Base64DecodeData *data = (Base64DecodeData *)self->data;

    char result_str[256];

    char encoded_preview[32];
    if (data->encoded_len > 4) {
        strncpy(encoded_preview, data->encoded_str, 4);
        encoded_preview[4] = '.';
        encoded_preview[5] = '.';
        encoded_preview[6] = '.';
        encoded_preview[7] = '\0';
    } else {
        strncpy(encoded_preview, data->encoded_str, data->encoded_len);
        encoded_preview[data->encoded_len] = '\0';
    }

    char decoded_preview[32];
    if (data->decoded_len > 4) {
        strncpy(decoded_preview, data->decoded_str, 4);
        decoded_preview[4] = '.';
        decoded_preview[5] = '.';
        decoded_preview[6] = '.';
        decoded_preview[7] = '\0';
    } else {
        strncpy(decoded_preview, data->decoded_str, data->decoded_len);
        decoded_preview[data->decoded_len] = '\0';
    }

    snprintf(result_str, sizeof(result_str), "decode %s to %s: %u",
             encoded_preview, decoded_preview, data->result_val);

    return Helper_checksum_string(result_str);
}

void Base64Decode_cleanup(Benchmark *self) {
    Base64DecodeData *data = (Base64DecodeData *)self->data;
    if (data->encoded_str)
        free(data->encoded_str);
    if (data->decoded_str)
        free(data->decoded_str);
}

Benchmark *Base64Decode_create(void) {
    Benchmark *bench = Benchmark_create("Base64::Decode");

    Base64DecodeData *data = malloc(sizeof(Base64DecodeData));
    memset(data, 0, sizeof(Base64DecodeData));

    bench->data = data;

    bench->prepare = Base64Decode_prepare;
    bench->run = Base64Decode_run;
    bench->checksum = Base64Decode_checksum;
    bench->cleanup = Base64Decode_cleanup;

    return bench;
}


typedef struct BinarytreesObj_TreeNode {
  struct BinarytreesObj_TreeNode *left;
  struct BinarytreesObj_TreeNode *right;
  int32_t item;
} BinarytreesObj_TreeNode;

static BinarytreesObj_TreeNode *BinarytreesObj_TreeNode_new(int32_t item,
                                                            int32_t depth) {
  BinarytreesObj_TreeNode *node = malloc(sizeof(BinarytreesObj_TreeNode));
  node->item = item;
  node->left = NULL;
  node->right = NULL;

  if (depth > 0) {
    int32_t shift = 1 << (depth - 1);
    node->left = BinarytreesObj_TreeNode_new(item - shift, depth - 1);
    node->right = BinarytreesObj_TreeNode_new(item + shift, depth - 1);
  }

  return node;
}

static void BinarytreesObj_TreeNode_free(BinarytreesObj_TreeNode *node) {
  if (!node)
    return;
  BinarytreesObj_TreeNode_free(node->left);
  BinarytreesObj_TreeNode_free(node->right);
  free(node);
}

static uint32_t BinarytreesObj_TreeNode_sum(BinarytreesObj_TreeNode *node) {
  uint32_t total = (uint32_t)node->item + 1;
  if (node->left)
    total += BinarytreesObj_TreeNode_sum(node->left);
  if (node->right)
    total += BinarytreesObj_TreeNode_sum(node->right);
  return total;
}

typedef struct {
  int64_t n;
  uint32_t result_val;
} BinarytreesObjData;

void BinarytreesObj_prepare(Benchmark *self) {
  BinarytreesObjData *data = (BinarytreesObjData *)self->data;
  data->n = Helper_config_i64(self->name, "depth");
  data->result_val = 0;
}

void BinarytreesObj_run(Benchmark *self, int iteration_id) {
  (void)iteration_id;
  BinarytreesObjData *data = (BinarytreesObjData *)self->data;

  BinarytreesObj_TreeNode *root =
      BinarytreesObj_TreeNode_new(0, (int32_t)data->n);
  data->result_val += BinarytreesObj_TreeNode_sum(root);
  BinarytreesObj_TreeNode_free(root);
}

uint32_t BinarytreesObj_checksum(Benchmark *self) {
  BinarytreesObjData *data = (BinarytreesObjData *)self->data;
  return data->result_val;
}

void BinarytreesObj_cleanup(Benchmark *self) {
  if (self && self->data) {
    BinarytreesObjData *data = (BinarytreesObjData *)self->data;

    free(data);
    self->data = NULL;
  }
}

Benchmark *BinarytreesObj_create(void) {
  Benchmark *bench = Benchmark_create("Binarytrees::Obj");
  if (!bench)
    return NULL;

  BinarytreesObjData *data = malloc(sizeof(BinarytreesObjData));
  if (!data) {
    free(bench);
    return NULL;
  }

  data->n = 0;
  data->result_val = 0;

  bench->data = data;
  bench->prepare = BinarytreesObj_prepare;
  bench->run = BinarytreesObj_run;
  bench->checksum = BinarytreesObj_checksum;
  bench->cleanup = BinarytreesObj_cleanup;

  return bench;
}

typedef struct {
  int32_t item;
  int32_t left;
  int32_t right;
} BinarytreesArena_Node;

typedef struct {
  BinarytreesArena_Node *nodes;
  size_t capacity;
  size_t size;
} BinarytreesArena_Arena;

static BinarytreesArena_Arena *BinarytreesArena_Arena_new(void) {
  BinarytreesArena_Arena *arena = malloc(sizeof(BinarytreesArena_Arena));
  if (!arena)
    return NULL;

  arena->nodes = NULL;
  arena->capacity = 0;
  arena->size = 0;
  return arena;
}

static void BinarytreesArena_Arena_free(BinarytreesArena_Arena *arena) {
  if (arena) {
    if (arena->nodes) {
      free(arena->nodes);
      arena->nodes = NULL;
    }
    arena->capacity = 0;
    arena->size = 0;
    free(arena);
  }
}

static int32_t BinarytreesArena_Arena_add(BinarytreesArena_Arena *arena,
                                          int32_t item) {
  if (!arena)
    return -1;

  if (arena->size >= arena->capacity) {
    size_t new_capacity = arena->capacity == 0 ? 1024 : arena->capacity * 2;
    BinarytreesArena_Node *new_nodes =
        realloc(arena->nodes, new_capacity * sizeof(BinarytreesArena_Node));
    if (!new_nodes)
      return -1;

    arena->nodes = new_nodes;
    arena->capacity = new_capacity;
  }

  int32_t idx = (int32_t)arena->size;
  BinarytreesArena_Node *node = &arena->nodes[idx];
  node->item = item;
  node->left = -1;
  node->right = -1;
  arena->size++;

  return idx;
}

static int32_t BinarytreesArena_build_tree(BinarytreesArena_Arena *arena,
                                           int32_t item, int32_t depth) {
  if (!arena)
    return -1;

  int32_t idx = BinarytreesArena_Arena_add(arena, item);
  if (idx < 0)
    return -1;

  if (depth > 0) {
    int32_t shift = 1 << (depth - 1);
    int32_t left_idx =
        BinarytreesArena_build_tree(arena, item - shift, depth - 1);
    int32_t right_idx =
        BinarytreesArena_build_tree(arena, item + shift, depth - 1);

    if (left_idx >= 0 && right_idx >= 0) {
      BinarytreesArena_Node *node = &arena->nodes[idx];
      node->left = left_idx;
      node->right = right_idx;
    }
  }

  return idx;
}

static uint32_t BinarytreesArena_sum(BinarytreesArena_Arena *arena,
                                     int32_t idx) {
  if (!arena || idx < 0 || (size_t)idx >= arena->size)
    return 0;

  BinarytreesArena_Node *node = &arena->nodes[idx];
  uint32_t total = (uint32_t)node->item + 1;

  if (node->left >= 0)
    total += BinarytreesArena_sum(arena, node->left);
  if (node->right >= 0)
    total += BinarytreesArena_sum(arena, node->right);

  return total;
}

typedef struct {
  int64_t n;
  uint32_t result_val;
  BinarytreesArena_Arena *arena;
} BinarytreesArenaData;

void BinarytreesArena_prepare(Benchmark *self) {
  if (!self || !self->data)
    return;

  BinarytreesArenaData *data = (BinarytreesArenaData *)self->data;
  data->n = Helper_config_i64(self->name, "depth");
  data->result_val = 0;
}

void BinarytreesArena_run(Benchmark *self, int iteration_id) {
  (void)iteration_id;
  if (!self || !self->data)
    return;

  BinarytreesArenaData *data = (BinarytreesArenaData *)self->data;

  data->arena = BinarytreesArena_Arena_new();

  int32_t root_idx =
      BinarytreesArena_build_tree(data->arena, 0, (int32_t)data->n);
  if (root_idx >= 0) {
    data->result_val += BinarytreesArena_sum(data->arena, root_idx);
  }

  BinarytreesArena_Arena_free(data->arena);
  data->arena = NULL;
}

uint32_t BinarytreesArena_checksum(Benchmark *self) {
  if (!self || !self->data)
    return 0;
  BinarytreesArenaData *data = (BinarytreesArenaData *)self->data;
  return data->result_val;
}

void BinarytreesArena_cleanup(Benchmark *self) {
  if (self && self->data) {
    BinarytreesArenaData *data = (BinarytreesArenaData *)self->data;

    if (data->arena) {
      BinarytreesArena_Arena_free(data->arena);
      data->arena = NULL;
    }

    free(data);
    self->data = NULL;
  }
}

Benchmark *BinarytreesArena_create(void) {
  Benchmark *bench = Benchmark_create("Binarytrees::Arena");
  if (!bench)
    return NULL;

  BinarytreesArenaData *data = malloc(sizeof(BinarytreesArenaData));
  if (!data) {
    free(bench);
    return NULL;
  }

  data->n = 0;
  data->result_val = 0;
  data->arena = NULL;

  bench->data = data;
  bench->prepare = BinarytreesArena_prepare;
  bench->run = BinarytreesArena_run;
  bench->checksum = BinarytreesArena_checksum;
  bench->cleanup = BinarytreesArena_cleanup;

  return bench;
}


typedef struct {
  uint8_t *tape;
  size_t tape_size;
  size_t pos;
} BrainfuckArray_Tape;

typedef struct {
  char *program;
  char *warmup_program;
  int32_t program_length;
  int32_t warmup_length;
  int32_t *jumps;
  int32_t *warmup_jumps;
  uint32_t result_val;
} BrainfuckArrayData;

static void BrainfuckArray_Tape_init(BrainfuckArray_Tape *tape) {
  tape->tape_size = 30000;
  tape->tape = calloc(tape->tape_size, sizeof(uint8_t));
  tape->pos = 0;
}

static void BrainfuckArray_Tape_destroy(BrainfuckArray_Tape *tape) {
  if (tape && tape->tape) {
    free(tape->tape);
    tape->tape = NULL;
    tape->tape_size = 0;
    tape->pos = 0;
  }
}

static uint8_t BrainfuckArray_Tape_get(const BrainfuckArray_Tape *tape) {
  return tape->tape[tape->pos];
}

static void BrainfuckArray_Tape_inc(BrainfuckArray_Tape *tape) {
  tape->tape[tape->pos]++;
}

static void BrainfuckArray_Tape_dec(BrainfuckArray_Tape *tape) {
  tape->tape[tape->pos]--;
}

static void BrainfuckArray_Tape_advance(BrainfuckArray_Tape *tape) {
  tape->pos++;
  if (tape->pos >= tape->tape_size) {

    size_t new_size = tape->tape_size + 1;
    uint8_t *new_tape = realloc(tape->tape, new_size);
    if (new_tape) {
      tape->tape = new_tape;
      tape->tape[tape->tape_size] = 0;
      tape->tape_size = new_size;
    }
  }
}

static void BrainfuckArray_Tape_devance(BrainfuckArray_Tape *tape) {
  if (tape->pos > 0) {
    tape->pos--;
  }
}

static char *BrainfuckArray_filter_commands(const char *input,
                                            int32_t *out_length) {
  if (!input) {
    *out_length = 0;
    return NULL;
  }

  size_t input_len = strlen(input);
  char *program = malloc(input_len + 1);
  if (!program) {
    *out_length = 0;
    return NULL;
  }

  int32_t program_pos = 0;
  for (size_t i = 0; i < input_len; i++) {
    char c = input[i];
    if (strchr("[]<>+-,.", c)) {
      program[program_pos++] = c;
    }
  }
  program[program_pos] = '\0';

  *out_length = program_pos;
  return program;
}

static int32_t *BrainfuckArray_build_jumps(const char *program,
                                           int32_t program_length) {
  if (!program || program_length == 0)
    return NULL;

  int32_t *jumps = calloc(program_length, sizeof(int32_t));
  if (!jumps)
    return NULL;

  int32_t *stack = malloc(sizeof(int32_t) * (program_length / 2 + 1));
  if (!stack) {
    free(jumps);
    return NULL;
  }

  int32_t stack_top = -1;

  for (int32_t pc = 0; pc < program_length; pc++) {
    char c = program[pc];
    if (c == '[') {
      stack[++stack_top] = pc;
    } else if (c == ']') {
      if (stack_top >= 0) {
        int32_t left = stack[stack_top--];
        jumps[left] = pc;
        jumps[pc] = left;
      }
    }
  }

  free(stack);
  return jumps;
}

static uint32_t BrainfuckArray_execute_program(const char *program,
                                               int32_t program_length,
                                               const int32_t *jumps) {
  if (!program || program_length == 0 || !jumps) {
    return 0;
  }

  BrainfuckArray_Tape tape;
  BrainfuckArray_Tape_init(&tape);

  int32_t pc = 0;
  uint32_t result = 0;

  while (pc < program_length) {
    char c = program[pc];
    switch (c) {
    case '+':
      BrainfuckArray_Tape_inc(&tape);
      break;

    case '-':
      BrainfuckArray_Tape_dec(&tape);
      break;

    case '>':
      BrainfuckArray_Tape_advance(&tape);
      break;

    case '<':
      BrainfuckArray_Tape_devance(&tape);
      break;

    case '[':
      if (BrainfuckArray_Tape_get(&tape) == 0) {
        pc = jumps[pc];
      }
      break;

    case ']':
      if (BrainfuckArray_Tape_get(&tape) != 0) {
        pc = jumps[pc];
      }
      break;

    case '.':
      result = (result << 2) + BrainfuckArray_Tape_get(&tape);
      break;

    default:
      break;
    }
    pc++;
  }

  BrainfuckArray_Tape_destroy(&tape);
  return result;
}

void BrainfuckArray_prepare(Benchmark *self) {
  BrainfuckArrayData *data = (BrainfuckArrayData *)self->data;

  const char *program_text = Helper_config_s(self->name, "program");
  const char *warmup_text = Helper_config_s(self->name, "warmup_program");

  data->program =
      BrainfuckArray_filter_commands(program_text, &data->program_length);
  data->jumps = BrainfuckArray_build_jumps(data->program, data->program_length);

  data->warmup_program =
      BrainfuckArray_filter_commands(warmup_text, &data->warmup_length);
  data->warmup_jumps =
      BrainfuckArray_build_jumps(data->warmup_program, data->warmup_length);

  data->result_val = 0;
}

void BrainfuckArray_warmup(Benchmark *self) {
  BrainfuckArrayData *data = (BrainfuckArrayData *)self->data;

  if (!data->warmup_program || data->warmup_length == 0)
    return;

  int64_t warmup_iters = Helper_config_i64(self->name, "warmup_iterations");
  if (warmup_iters == 0) {
    warmup_iters = self->iterations(self) / 5;
    if (warmup_iters < 1)
      warmup_iters = 1;
  }

  for (int64_t i = 0; i < warmup_iters; i++) {
    BrainfuckArray_execute_program(data->warmup_program, data->warmup_length,
                                   data->warmup_jumps);
  }
}

void BrainfuckArray_run(Benchmark *self, int iteration_id) {
  BrainfuckArrayData *data = (BrainfuckArrayData *)self->data;

  if (iteration_id == 0) {
    data->result_val = 0;
  }

  if (data->program && data->program_length > 0 && data->jumps) {
    uint32_t run_result = BrainfuckArray_execute_program(
        data->program, data->program_length, data->jumps);
    data->result_val += run_result;
  }
}

uint32_t BrainfuckArray_checksum(Benchmark *self) {
  BrainfuckArrayData *data = (BrainfuckArrayData *)self->data;
  return data->result_val;
}

void BrainfuckArray_cleanup(Benchmark *self) {
  if (!self || !self->data)
    return;

  BrainfuckArrayData *data = (BrainfuckArrayData *)self->data;

  free(data->program);
  free(data->jumps);
  free(data->warmup_program);
  free(data->warmup_jumps);

  free(data);
  self->data = NULL;
}

Benchmark *BrainfuckArray_create(void) {
  Benchmark *bench = Benchmark_create("Brainfuck::Array");
  if (!bench)
    return NULL;

  BrainfuckArrayData *data = calloc(1, sizeof(BrainfuckArrayData));
  if (!data) {
    free(bench);
    return NULL;
  }

  bench->data = data;
  bench->prepare = BrainfuckArray_prepare;
  bench->warmup = BrainfuckArray_warmup;
  bench->run = BrainfuckArray_run;
  bench->checksum = BrainfuckArray_checksum;
  bench->cleanup = BrainfuckArray_cleanup;

  return bench;
}


typedef enum {
  BrainfuckRecursion_OP_INC,
  BrainfuckRecursion_OP_DEC,
  BrainfuckRecursion_OP_RIGHT,
  BrainfuckRecursion_OP_LEFT,
  BrainfuckRecursion_OP_PRINT,
  BrainfuckRecursion_OP_LOOP
} BrainfuckRecursion_OpType;

typedef struct BrainfuckRecursion_Op BrainfuckRecursion_Op;

struct BrainfuckRecursion_Op {
  BrainfuckRecursion_OpType type;
  BrainfuckRecursion_Op *loop_ops;
  int32_t loop_size;
};

typedef struct BrainfuckRecursion_Tape {
  uint8_t *tape;
  int32_t size;
  int32_t pos;
} BrainfuckRecursion_Tape;

static BrainfuckRecursion_Tape *BrainfuckRecursion_Tape_new(void) {
  BrainfuckRecursion_Tape *self = malloc(sizeof(BrainfuckRecursion_Tape));
  self->size = 30000;
  self->tape = calloc(self->size, sizeof(uint8_t));
  self->pos = 0;
  return self;
}

static uint8_t BrainfuckRecursion_Tape_get(BrainfuckRecursion_Tape *self) {
  return self->tape[self->pos];
}

static void BrainfuckRecursion_Tape_inc(BrainfuckRecursion_Tape *self) {
  self->tape[self->pos]++;
}

static void BrainfuckRecursion_Tape_dec(BrainfuckRecursion_Tape *self) {
  self->tape[self->pos]--;
}

static void BrainfuckRecursion_Tape_right(BrainfuckRecursion_Tape *self) {
  self->pos++;
  if (self->pos >= self->size) {
    self->size++;
    self->tape = realloc(self->tape, self->size);
    self->tape[self->size - 1] = 0;
  }
}

static void BrainfuckRecursion_Tape_left(BrainfuckRecursion_Tape *self) {
  if (self->pos > 0) {
    self->pos--;
  }
}

static void BrainfuckRecursion_Tape_free(BrainfuckRecursion_Tape *self) {
  free(self->tape);
  free(self);
}

static BrainfuckRecursion_Op *BrainfuckRecursion_parse_ops(const char **code,
                                                           int32_t *ops_count) {
  int32_t capacity = 16;
  BrainfuckRecursion_Op *ops = malloc(sizeof(BrainfuckRecursion_Op) * capacity);
  int32_t count = 0;

  while (**code) {
    if (count >= capacity) {
      capacity *= 2;
      ops = realloc(ops, sizeof(BrainfuckRecursion_Op) * capacity);
    }

    switch (**code) {
    case '+':
      ops[count].type = BrainfuckRecursion_OP_INC;
      ops[count].loop_ops = NULL;
      ops[count].loop_size = 0;
      count++;
      break;
    case '-':
      ops[count].type = BrainfuckRecursion_OP_DEC;
      ops[count].loop_ops = NULL;
      ops[count].loop_size = 0;
      count++;
      break;
    case '>':
      ops[count].type = BrainfuckRecursion_OP_RIGHT;
      ops[count].loop_ops = NULL;
      ops[count].loop_size = 0;
      count++;
      break;
    case '<':
      ops[count].type = BrainfuckRecursion_OP_LEFT;
      ops[count].loop_ops = NULL;
      ops[count].loop_size = 0;
      count++;
      break;
    case '.':
      ops[count].type = BrainfuckRecursion_OP_PRINT;
      ops[count].loop_ops = NULL;
      ops[count].loop_size = 0;
      count++;
      break;
    case '[':
      (*code)++;
      ops[count].type = BrainfuckRecursion_OP_LOOP;
      ops[count].loop_ops = NULL;
      ops[count].loop_size = 0;

      int32_t loop_ops_count = 0;
      ops[count].loop_ops = BrainfuckRecursion_parse_ops(code, &loop_ops_count);
      ops[count].loop_size = loop_ops_count;

      count++;
      continue;
    case ']':
      *ops_count = count;
      (*code)++;
      return ops;
    default:
      break;
    }
    (*code)++;
  }

  *ops_count = count;
  return ops;
}

static void BrainfuckRecursion_free_ops(BrainfuckRecursion_Op *ops,
                                        int32_t ops_size) {
  if (!ops)
    return;

  for (int32_t i = 0; i < ops_size; i++) {
    if (ops[i].type == BrainfuckRecursion_OP_LOOP && ops[i].loop_ops) {
      BrainfuckRecursion_free_ops(ops[i].loop_ops, ops[i].loop_size);
    }
  }
  free(ops);
}

static void BrainfuckRecursion_run_ops(BrainfuckRecursion_Op *ops,
                                       int32_t ops_size,
                                       BrainfuckRecursion_Tape *tape,
                                       uint32_t *result) {
  for (int32_t i = 0; i < ops_size; i++) {
    BrainfuckRecursion_Op *op = &ops[i];
    switch (op->type) {
    case BrainfuckRecursion_OP_INC:
      BrainfuckRecursion_Tape_inc(tape);
      break;
    case BrainfuckRecursion_OP_DEC:
      BrainfuckRecursion_Tape_dec(tape);
      break;
    case BrainfuckRecursion_OP_RIGHT:
      BrainfuckRecursion_Tape_right(tape);
      break;
    case BrainfuckRecursion_OP_LEFT:
      BrainfuckRecursion_Tape_left(tape);
      break;
    case BrainfuckRecursion_OP_PRINT: {
      uint8_t value = BrainfuckRecursion_Tape_get(tape);
      *result = (*result << 2) + value;
      break;
    }
    case BrainfuckRecursion_OP_LOOP:
      while (BrainfuckRecursion_Tape_get(tape) != 0) {
        BrainfuckRecursion_run_ops(op->loop_ops, op->loop_size, tape, result);
      }
      break;
    }
  }
}

static uint32_t BrainfuckRecursion_run_program(const char *code) {
  if (!code || !code[0]) {
    return 0;
  }

  const char *code_ptr = code;
  int32_t ops_count = 0;
  BrainfuckRecursion_Op *ops =
      BrainfuckRecursion_parse_ops(&code_ptr, &ops_count);

  if (!ops) {
    return 0;
  }

  BrainfuckRecursion_Tape *tape = BrainfuckRecursion_Tape_new();
  uint32_t result = 0;
  BrainfuckRecursion_run_ops(ops, ops_count, tape, &result);

  BrainfuckRecursion_Tape_free(tape);
  BrainfuckRecursion_free_ops(ops, ops_count);

  return result;
}

typedef struct {
  const char *program;
  const char *warmup_program;
  uint32_t result_val;
} BrainfuckRecursionData;

void BrainfuckRecursion_prepare(Benchmark *self) {
  BrainfuckRecursionData *data = (BrainfuckRecursionData *)self->data;
  data->program = Helper_config_s(self->name, "program");
  data->warmup_program = Helper_config_s(self->name, "warmup_program");
  data->result_val = 0;
}

void BrainfuckRecursion_warmup(Benchmark *self) {
  BrainfuckRecursionData *data = (BrainfuckRecursionData *)self->data;
  int64_t warmup_iters = Helper_config_i64(self->name, "warmup_iterations");

  if (warmup_iters == 0) {
    warmup_iters = self->iterations(self);
    warmup_iters = (int64_t)(warmup_iters * 0.2);
    if (warmup_iters < 1)
      warmup_iters = 1;
  }

  for (int64_t i = 0; i < warmup_iters; i++) {
    BrainfuckRecursion_run_program(data->warmup_program);
  }
}

void BrainfuckRecursion_run(Benchmark *self, int iteration_id) {
  BrainfuckRecursionData *data = (BrainfuckRecursionData *)self->data;

  if (iteration_id == 0) {
    data->result_val = 0;
  }

  uint32_t run_result = BrainfuckRecursion_run_program(data->program);
  data->result_val += run_result;
}

uint32_t BrainfuckRecursion_checksum(Benchmark *self) {
  BrainfuckRecursionData *data = (BrainfuckRecursionData *)self->data;
  return data->result_val;
}

void BrainfuckRecursion_cleanup(Benchmark *self) { (void)self; }

Benchmark *BrainfuckRecursion_create(void) {
  Benchmark *bench = Benchmark_create("Brainfuck::Recursion");

  BrainfuckRecursionData *data = malloc(sizeof(BrainfuckRecursionData));
  data->program = NULL;
  data->warmup_program = NULL;
  data->result_val = 0;

  bench->data = data;
  bench->prepare = BrainfuckRecursion_prepare;
  bench->warmup = BrainfuckRecursion_warmup;
  bench->run = BrainfuckRecursion_run;
  bench->checksum = BrainfuckRecursion_checksum;
  bench->cleanup = BrainfuckRecursion_cleanup;

  return bench;
}


typedef struct {
  uint8_t *data;
  int64_t size_val;
  uint32_t result_val;
} BufferHashBaseData;

static void buffer_hash_base_prepare(Benchmark *self, const char *bench_name,
                                     BufferHashBaseData *data) {
  (void)self;
  data->size_val = Helper_config_i64(bench_name, "size");
  if (data->size_val <= 0) {
    data->size_val = 1000000;
  }

  data->data = malloc(data->size_val * sizeof(uint8_t));

  for (int64_t i = 0; i < data->size_val; i++) {
    data->data[i] = (uint8_t)Helper_next_int(256);
  }

  data->result_val = 0;
}

static void buffer_hash_base_cleanup(BufferHashBaseData *data) {
  if (data->data) {
    free(data->data);
    data->data = NULL;
  }
}

typedef struct {
  BufferHashBaseData base;
} BufferHashSHA256Data;

static uint32_t buffer_hash_sha256_digest(uint8_t *data, int64_t size) {
  uint32_t hashes[8] = {0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
                        0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};

  for (int64_t i = 0; i < size; i++) {
    uint32_t hash_idx = (uint32_t)(i & 7);
    uint32_t *hash = &hashes[hash_idx];

    uint32_t temp = (*hash << 5) + *hash;
    temp = temp + data[i];
    *hash = temp;

    temp = *hash + (*hash << 10);
    *hash = temp ^ (*hash >> 6);
  }

  uint8_t result[32];
  for (int i = 0; i < 8; i++) {
    result[i * 4] = (hashes[i] >> 24) & 0xFF;
    result[i * 4 + 1] = (hashes[i] >> 16) & 0xFF;
    result[i * 4 + 2] = (hashes[i] >> 8) & 0xFF;
    result[i * 4 + 3] = hashes[i] & 0xFF;
  }

  return (uint32_t)result[0] | ((uint32_t)result[1] << 8) |
         ((uint32_t)result[2] << 16) | ((uint32_t)result[3] << 24);
}

void BufferHashSHA256_prepare(Benchmark *self) {
  BufferHashSHA256Data *data = (BufferHashSHA256Data *)self->data;
  buffer_hash_base_prepare(self, "Hash::SHA256", &data->base);
}

void BufferHashSHA256_run(Benchmark *self, int iteration_id) {
  (void)iteration_id;
  BufferHashSHA256Data *data = (BufferHashSHA256Data *)self->data;

  uint32_t hash_result =
      buffer_hash_sha256_digest(data->base.data, data->base.size_val);

  data->base.result_val = (data->base.result_val + hash_result) & 0xFFFFFFFFu;
}

uint32_t BufferHashSHA256_checksum(Benchmark *self) {
  BufferHashSHA256Data *data = (BufferHashSHA256Data *)self->data;
  return data->base.result_val;
}

void BufferHashSHA256_cleanup(Benchmark *self) {
  BufferHashSHA256Data *data = (BufferHashSHA256Data *)self->data;
  buffer_hash_base_cleanup(&data->base);
}

Benchmark *BufferHashSHA256_create(void) {
  Benchmark *bench = Benchmark_create("Hash::SHA256");

  BufferHashSHA256Data *data = malloc(sizeof(BufferHashSHA256Data));
  memset(data, 0, sizeof(BufferHashSHA256Data));

  bench->data = data;

  bench->prepare = BufferHashSHA256_prepare;
  bench->run = BufferHashSHA256_run;
  bench->checksum = BufferHashSHA256_checksum;
  bench->cleanup = BufferHashSHA256_cleanup;

  return bench;
}

typedef struct {
  BufferHashBaseData base;
} BufferHashCRC32Data;

static uint32_t buffer_hash_crc32_digest(uint8_t *data, int64_t size) {
  uint32_t crc = 0xFFFFFFFFu;

  for (int64_t i = 0; i < size; i++) {
    crc = crc ^ data[i];
    for (int j = 0; j < 8; j++) {
      if (crc & 1) {
        crc = (crc >> 1) ^ 0xEDB88320u;
      } else {
        crc = crc >> 1;
      }
    }
  }
  return crc ^ 0xFFFFFFFFu;
}

void BufferHashCRC32_prepare(Benchmark *self) {
  BufferHashCRC32Data *data = (BufferHashCRC32Data *)self->data;
  buffer_hash_base_prepare(self, "Hash::CRC32", &data->base);
}

void BufferHashCRC32_run(Benchmark *self, int iteration_id) {
  (void)iteration_id;
  BufferHashCRC32Data *data = (BufferHashCRC32Data *)self->data;

  uint32_t crc_result =
      buffer_hash_crc32_digest(data->base.data, data->base.size_val);

  data->base.result_val += crc_result;
}

uint32_t BufferHashCRC32_checksum(Benchmark *self) {
  BufferHashCRC32Data *data = (BufferHashCRC32Data *)self->data;
  return data->base.result_val;
}

void BufferHashCRC32_cleanup(Benchmark *self) {
  BufferHashCRC32Data *data = (BufferHashCRC32Data *)self->data;
  buffer_hash_base_cleanup(&data->base);
}

Benchmark *BufferHashCRC32_create(void) {
  Benchmark *bench = Benchmark_create("Hash::CRC32");

  BufferHashCRC32Data *data = malloc(sizeof(BufferHashCRC32Data));
  memset(data, 0, sizeof(BufferHashCRC32Data));

  bench->data = data;

  bench->prepare = BufferHashCRC32_prepare;
  bench->run = BufferHashCRC32_run;
  bench->checksum = BufferHashCRC32_checksum;
  bench->cleanup = BufferHashCRC32_cleanup;

  return bench;
}


typedef struct CacheNode {
  char *key;
  char *value;
  int64_t timestamp;
  struct CacheNode *prev;
  struct CacheNode *next;
  UT_hash_handle hh;
} CacheNode;

typedef struct {
  CacheNode *head;
  CacheNode *tail;
  CacheNode *hash_table;
  size_t capacity;
  size_t size;
  int64_t time;
} CacheSimulationCache;

typedef struct {
  uint32_t result_val;
  int64_t values_size;
  int64_t cache_size;
  int64_t hits;
  int64_t misses;
  CacheSimulationCache *cache;
} CacheSimulationData;

static CacheSimulationCache *cache_simulation_cache_new(size_t capacity) {
  CacheSimulationCache *cache = malloc(sizeof(CacheSimulationCache));
  cache->head = NULL;
  cache->tail = NULL;
  cache->hash_table = NULL;
  cache->capacity = capacity;
  cache->size = 0;
  cache->time = 0;
  return cache;
}

static void cache_simulation_cache_move_to_front(CacheSimulationCache *cache,
                                                 CacheNode *node) {
  if (node == cache->head)
    return;

  if (node->prev)
    node->prev->next = node->next;
  if (node->next)
    node->next->prev = node->prev;

  if (node == cache->tail)
    cache->tail = node->prev;

  node->prev = NULL;
  node->next = cache->head;
  if (cache->head)
    cache->head->prev = node;
  cache->head = node;
  if (!cache->tail)
    cache->tail = node;
}

static CacheNode *cache_simulation_cache_get(CacheSimulationCache *cache,
                                             const char *key) {
  CacheNode *node = NULL;
  HASH_FIND_STR(cache->hash_table, key, node);

  if (node) {
    cache_simulation_cache_move_to_front(cache, node);
    node->timestamp = ++cache->time;
    return node;
  }
  return NULL;
}

static void cache_simulation_cache_remove_oldest(CacheSimulationCache *cache) {
  if (!cache->tail)
    return;

  CacheNode *oldest = cache->tail;

  HASH_DEL(cache->hash_table, oldest);

  if (oldest->prev)
    oldest->prev->next = oldest->next;
  if (oldest->next)
    oldest->next->prev = oldest->prev;

  if (cache->head == oldest)
    cache->head = oldest->next;
  if (cache->tail == oldest)
    cache->tail = oldest->prev;

  free(oldest->key);
  free(oldest->value);
  free(oldest);

  cache->size--;
}

static void cache_simulation_cache_put(CacheSimulationCache *cache,
                                       const char *key, const char *value) {
  CacheNode *node = NULL;
  HASH_FIND_STR(cache->hash_table, key, node);

  if (node) {

    free(node->value);
    node->value = strdup(value);
    cache_simulation_cache_move_to_front(cache, node);
    node->timestamp = ++cache->time;
    return;
  }

  if (cache->size >= cache->capacity) {
    cache_simulation_cache_remove_oldest(cache);
  }

  node = malloc(sizeof(CacheNode));
  node->key = strdup(key);
  node->value = strdup(value);
  node->timestamp = ++cache->time;
  node->prev = NULL;
  node->next = NULL;

  HASH_ADD_STR(cache->hash_table, key, node);

  node->next = cache->head;
  if (cache->head)
    cache->head->prev = node;
  cache->head = node;
  if (!cache->tail)
    cache->tail = node;

  cache->size++;
}

static void cache_simulation_cache_free(CacheSimulationCache *cache) {
  CacheNode *node, *tmp;
  HASH_ITER(hh, cache->hash_table, node, tmp) {
    HASH_DEL(cache->hash_table, node);
    free(node->key);
    free(node->value);
    free(node);
  }
  free(cache);
}

void CacheSimulation_prepare(Benchmark *self) {
  CacheSimulationData *data = (CacheSimulationData *)self->data;

  data->values_size = Helper_config_i64(self->name, "values");
  if (data->values_size <= 0) {
    data->values_size = 2000;
  }

  data->cache_size = Helper_config_i64(self->name, "size");
  if (data->cache_size <= 0) {
    data->cache_size = 1000;
  }

  data->result_val = 5432;
  data->hits = 0;
  data->misses = 0;
  data->cache = cache_simulation_cache_new((size_t)data->cache_size);
}

void CacheSimulation_run(Benchmark *self, int iteration_id) {
  CacheSimulationData *data = (CacheSimulationData *)self->data;

  for (int i = 0; i < 1000; i++) {
    char key_buf[32];
    snprintf(key_buf, sizeof(key_buf), "item_%u",
             Helper_next_int((uint32_t)data->values_size));

    if (cache_simulation_cache_get(data->cache, key_buf)) {
      data->hits++;
      char value_buf[32];
      snprintf(value_buf, sizeof(value_buf), "updated_%d", iteration_id);
      cache_simulation_cache_put(data->cache, key_buf, value_buf);
    } else {
      data->misses++;
      char value_buf[32];
      snprintf(value_buf, sizeof(value_buf), "new_%d", iteration_id);
      cache_simulation_cache_put(data->cache, key_buf, value_buf);
    }
  }
}

uint32_t CacheSimulation_checksum(Benchmark *self) {
  CacheSimulationData *data = (CacheSimulationData *)self->data;

  uint32_t final_result = data->result_val;
  final_result = ((int64_t)final_result << 5) + (uint32_t)data->hits;
  final_result = ((int64_t)final_result << 5) + (uint32_t)data->misses;
  final_result = ((int64_t)final_result << 5) + (uint32_t)data->cache->size;

  return final_result;
}

void CacheSimulation_cleanup(Benchmark *self) {
  CacheSimulationData *data = (CacheSimulationData *)self->data;

  if (data->cache) {
    cache_simulation_cache_free(data->cache);
    data->cache = NULL;
  }
}

Benchmark *CacheSimulation_create(void) {
  Benchmark *bench = Benchmark_create("Etc::CacheSimulation");

  CacheSimulationData *data = malloc(sizeof(CacheSimulationData));
  memset(data, 0, sizeof(CacheSimulationData));

  bench->data = data;

  bench->prepare = CacheSimulation_prepare;
  bench->run = CacheSimulation_run;
  bench->checksum = CacheSimulation_checksum;
  bench->cleanup = CacheSimulation_cleanup;

  return bench;
}


#define CHAR_EOF '\0'
#define CHAR_PLUS '+'
#define CHAR_MINUS '-'
#define CHAR_STAR '*'
#define CHAR_SLASH '/'
#define CHAR_PERCENT '%'
#define CHAR_LPAREN '('
#define CHAR_RPAREN ')'
#define CHAR_EQUALS '='
#define CHAR_ZERO '0'
#define CHAR_NINE '9'
#define CHAR_A_LOWER 'a'
#define CHAR_Z_LOWER 'z'
#define CHAR_A_UPPER 'A'
#define CHAR_Z_UPPER 'Z'
#define CHAR_SPACE ' '
#define CHAR_TAB '\t'
#define CHAR_NEWLINE '\n'
#define CHAR_CR '\r'

typedef enum {
  AST_NUMBER,
  AST_VARIABLE,
  AST_BINARY_OP,
  AST_ASSIGNMENT
} AST_NodeType;

typedef struct AST_Node AST_Node;
typedef struct AST_BinaryOp AST_BinaryOp;
typedef struct AST_Assignment AST_Assignment;

struct AST_BinaryOp {
  char op;
  AST_Node *left;
  AST_Node *right;
};

struct AST_Assignment {
  char *var_name;
  AST_Node *expr;
};

struct AST_Node {
  AST_NodeType type;
  union {
    int64_t number_value;
    char *variable_name;
    AST_BinaryOp *binary_op;
    AST_Assignment *assignment;
  } data;
};

typedef struct {
  AST_Node **expressions;
  int64_t expressions_count;
  int64_t expressions_capacity;
  uint32_t result_val;
  char *text;
  int64_t operations;
} CalculatorAstData;

static AST_Node *ast_node_new_number(int64_t value) {
  AST_Node *node = malloc(sizeof(AST_Node));
  node->type = AST_NUMBER;
  node->data.number_value = value;
  return node;
}

static AST_Node *ast_node_new_variable(const char *name) {
  AST_Node *node = malloc(sizeof(AST_Node));
  node->type = AST_VARIABLE;
  node->data.variable_name = strdup(name);
  return node;
}

static AST_Node *ast_node_new_binary_op(char op, AST_Node *left,
                                        AST_Node *right) {
  AST_BinaryOp *binary_op = malloc(sizeof(AST_BinaryOp));
  binary_op->op = op;
  binary_op->left = left;
  binary_op->right = right;

  AST_Node *node = malloc(sizeof(AST_Node));
  node->type = AST_BINARY_OP;
  node->data.binary_op = binary_op;
  return node;
}

static AST_Node *ast_node_new_assignment(const char *var_name, AST_Node *expr) {
  AST_Assignment *assignment = malloc(sizeof(AST_Assignment));
  assignment->var_name = strdup(var_name);
  assignment->expr = expr;

  AST_Node *node = malloc(sizeof(AST_Node));
  node->type = AST_ASSIGNMENT;
  node->data.assignment = assignment;
  return node;
}

static void ast_node_free(AST_Node *node) {
  if (!node)
    return;

  switch (node->type) {
  case AST_VARIABLE:
    free(node->data.variable_name);
    break;
  case AST_BINARY_OP:
    if (node->data.binary_op) {
      ast_node_free(node->data.binary_op->left);
      ast_node_free(node->data.binary_op->right);
      free(node->data.binary_op);
    }
    break;
  case AST_ASSIGNMENT:
    if (node->data.assignment) {
      free(node->data.assignment->var_name);
      ast_node_free(node->data.assignment->expr);
      free(node->data.assignment);
    }
    break;
  default:
    break;
  }
  free(node);
}

typedef struct {
  const char *input;
  size_t pos;
  size_t len;
  char current_char;
} CalculatorAstParser;

static AST_Node *
calculator_ast_parser_parse_expression(CalculatorAstParser *parser);
static AST_Node *calculator_ast_parser_parse_term(CalculatorAstParser *parser);
static AST_Node *
calculator_ast_parser_parse_factor(CalculatorAstParser *parser);

static bool is_digit(char c) { return c >= CHAR_ZERO && c <= CHAR_NINE; }

static bool is_letter(char c) {
  return (c >= CHAR_A_LOWER && c <= CHAR_Z_LOWER) ||
         (c >= CHAR_A_UPPER && c <= CHAR_Z_UPPER);
}

static bool is_whitespace(char c) {
  return c == CHAR_SPACE || c == CHAR_TAB || c == CHAR_NEWLINE || c == CHAR_CR;
}

static void calculator_ast_parser_init(CalculatorAstParser *parser,
                                       const char *input) {
  parser->input = input;
  parser->pos = 0;
  parser->len = strlen(input);
  parser->current_char = parser->len > 0 ? input[0] : CHAR_EOF;
}

static void calculator_ast_parser_advance(CalculatorAstParser *parser) {
  parser->pos++;
  if (parser->pos >= parser->len) {
    parser->current_char = CHAR_EOF;
  } else {
    parser->current_char = parser->input[parser->pos];
  }
}

static void calculator_ast_parser_skip_whitespace(CalculatorAstParser *parser) {
  while (is_whitespace(parser->current_char)) {
    calculator_ast_parser_advance(parser);
  }
}

static AST_Node *
calculator_ast_parser_parse_number(CalculatorAstParser *parser) {
  int64_t value = 0;
  while (is_digit(parser->current_char)) {
    value = value * 10 + (parser->current_char - CHAR_ZERO);
    calculator_ast_parser_advance(parser);
  }
  return ast_node_new_number(value);
}

static AST_Node *
calculator_ast_parser_parse_variable(CalculatorAstParser *parser) {
  size_t start = parser->pos;
  while (is_letter(parser->current_char) || is_digit(parser->current_char)) {
    calculator_ast_parser_advance(parser);
  }

  size_t len = parser->pos - start;
  char *var_name = malloc(len + 1);
  strncpy(var_name, parser->input + start, len);
  var_name[len] = '\0';

  calculator_ast_parser_skip_whitespace(parser);

  if (parser->current_char == CHAR_EQUALS) {
    calculator_ast_parser_advance(parser);
    AST_Node *expr = calculator_ast_parser_parse_expression(parser);
    AST_Node *node = ast_node_new_assignment(var_name, expr);
    free(var_name);
    return node;
  }

  AST_Node *node = ast_node_new_variable(var_name);
  free(var_name);
  return node;
}

static AST_Node *
calculator_ast_parser_parse_factor(CalculatorAstParser *parser) {
  calculator_ast_parser_skip_whitespace(parser);

  if (is_digit(parser->current_char)) {
    return calculator_ast_parser_parse_number(parser);
  }

  if (is_letter(parser->current_char)) {
    return calculator_ast_parser_parse_variable(parser);
  }

  if (parser->current_char == CHAR_LPAREN) {
    calculator_ast_parser_advance(parser);
    AST_Node *node = calculator_ast_parser_parse_expression(parser);
    calculator_ast_parser_skip_whitespace(parser);
    if (parser->current_char == CHAR_RPAREN) {
      calculator_ast_parser_advance(parser);
    }
    return node;
  }

  calculator_ast_parser_advance(parser);
  return ast_node_new_number(0);
}

static AST_Node *calculator_ast_parser_parse_term(CalculatorAstParser *parser) {
  AST_Node *node = calculator_ast_parser_parse_factor(parser);

  while (true) {
    calculator_ast_parser_skip_whitespace(parser);

    if (parser->current_char == CHAR_STAR ||
        parser->current_char == CHAR_SLASH ||
        parser->current_char == CHAR_PERCENT) {
      char op = parser->current_char;
      calculator_ast_parser_advance(parser);
      AST_Node *right = calculator_ast_parser_parse_factor(parser);
      node = ast_node_new_binary_op(op, node, right);
    } else {
      break;
    }
  }

  return node;
}

static AST_Node *
calculator_ast_parser_parse_expression(CalculatorAstParser *parser) {
  AST_Node *node = calculator_ast_parser_parse_term(parser);

  while (true) {
    calculator_ast_parser_skip_whitespace(parser);

    if (parser->current_char == CHAR_PLUS ||
        parser->current_char == CHAR_MINUS) {
      char op = parser->current_char;
      calculator_ast_parser_advance(parser);
      AST_Node *right = calculator_ast_parser_parse_term(parser);
      node = ast_node_new_binary_op(op, node, right);
    } else {
      break;
    }
  }

  return node;
}

static void calculator_ast_parser_parse_all(CalculatorAstParser *parser,
                                            CalculatorAstData *data) {
  data->expressions_count = 0;

  while (parser->current_char != CHAR_EOF) {
    calculator_ast_parser_skip_whitespace(parser);
    if (parser->current_char == CHAR_EOF)
      break;

    if (data->expressions_count >= data->expressions_capacity) {
      data->expressions_capacity =
          data->expressions_capacity ? data->expressions_capacity * 2 : 16;
      data->expressions = realloc(
          data->expressions, data->expressions_capacity * sizeof(AST_Node *));
    }

    data->expressions[data->expressions_count++] =
        calculator_ast_parser_parse_expression(parser);

    calculator_ast_parser_skip_whitespace(parser);
    while (parser->current_char == CHAR_NEWLINE) {
      calculator_ast_parser_advance(parser);
      calculator_ast_parser_skip_whitespace(parser);
    }
  }
}

static char *calculator_ast_generate_random_program(int64_t operations) {
  size_t capacity = operations * 100;
  char *result = malloc(capacity);
  size_t len = 0;

  len += snprintf(result + len, capacity - len, "v0 = 1\n");
  for (int i = 0; i < 10; i++) {
    len += snprintf(result + len, capacity - len, "v%d = v%d + %d\n", i + 1, i,
                    i + 1);
  }

  for (int64_t i = 0; i < operations; i++) {
    int v = (int)(i + 10);

    len += snprintf(result + len, capacity - len, "v%d = v%d + ", v, v - 1);

    switch (Helper_next_int(10)) {
    case 0:
      len += snprintf(result + len, capacity - len,
                      "(v%d / 3) * 4 - %ld / (3 + (18 - v%d)) %% v%d + 2 * ((9 "
                      "- v%d) * (v%d + 7))",
                      v - 1, (long)i, v - 2, v - 3, v - 6, v - 5);
      break;
    case 1:
      len += snprintf(result + len, capacity - len,
                      "v%d + (v%d + v%d) * v%d - (v%d / v%d)", v - 1, v - 2,
                      v - 3, v - 4, v - 5, v - 6);
      break;
    case 2:
      len += snprintf(result + len, capacity - len, "(3789 - (((v%d)))) + 1",
                      v - 7);
      break;
    case 3:
      len += snprintf(result + len, capacity - len, "4/2 * (1-3) + v%d/v%d",
                      v - 9, v - 5);
      break;
    case 4:
      len += snprintf(result + len, capacity - len, "1+2+3+4+5+6+v%d", v - 1);
      break;
    case 5:
      len += snprintf(result + len, capacity - len, "(99999 / v%d)", v - 3);
      break;
    case 6:
      len += snprintf(result + len, capacity - len, "0 + 0 - v%d", v - 8);
      break;
    case 7:
      len += snprintf(result + len, capacity - len,
                      "((((((((((v%d)))))))))) * 2", v - 6);
      break;
    case 8:
      len += snprintf(result + len, capacity - len, "%ld * (v%d%%6)%%7",
                      (long)i, v - 1);
      break;
    case 9:
      len += snprintf(result + len, capacity - len, "(1)/(0-v%d) + (v%d)",
                      v - 5, v - 7);
      break;
    }
    len += snprintf(result + len, capacity - len, "\n");
  }

  result[len] = '\0';
  return result;
}

void CalculatorAst_prepare(Benchmark *self) {
  CalculatorAstData *data = (CalculatorAstData *)self->data;

  data->operations = Helper_config_i64(self->name, "operations");
  if (data->operations <= 0) {
    data->operations = 1000;
  }

  data->text = calculator_ast_generate_random_program(data->operations);
  data->expressions = NULL;
  data->expressions_count = 0;
  data->expressions_capacity = 0;
  data->result_val = 0;
}

void CalculatorAst_run(Benchmark *self, int iteration_id) {
  (void)iteration_id;
  CalculatorAstData *data = (CalculatorAstData *)self->data;

  if (data->expressions) {
    for (int64_t i = 0; i < data->expressions_count; i++) {
      ast_node_free(data->expressions[i]);
    }
    free(data->expressions);
    data->expressions = NULL;
    data->expressions_count = 0;
    data->expressions_capacity = 0;
  }

  CalculatorAstParser parser;
  calculator_ast_parser_init(&parser, data->text);

  if (data->expressions) {
    for (int64_t i = 0; i < data->expressions_count; i++) {
      ast_node_free(data->expressions[i]);
    }
    free(data->expressions);
    data->expressions = NULL;
    data->expressions_count = 0;
    data->expressions_capacity = 0;
  }

  calculator_ast_parser_parse_all(&parser, data);

  uint32_t iteration_result = (uint32_t)data->expressions_count;

  if (data->expressions_count > 0 &&
      data->expressions[data->expressions_count - 1]->type == AST_ASSIGNMENT) {
    AST_Node *last = data->expressions[data->expressions_count - 1];
    iteration_result += Helper_checksum_string(last->data.assignment->var_name);
  }

  data->result_val += iteration_result;
}

uint32_t CalculatorAst_checksum(Benchmark *self) {
  CalculatorAstData *data = (CalculatorAstData *)self->data;
  return data->result_val;
}

void CalculatorAst_cleanup(Benchmark *self) {
  CalculatorAstData *data = (CalculatorAstData *)self->data;

  if (data->text) {
    free(data->text);
    data->text = NULL;
  }

  if (data->expressions) {
    for (int64_t i = 0; i < data->expressions_count; i++) {
      ast_node_free(data->expressions[i]);
    }
    free(data->expressions);
    data->expressions = NULL;
  }
}

Benchmark *CalculatorAst_create(void) {
  Benchmark *bench = Benchmark_create("Calculator::Ast");

  CalculatorAstData *data = malloc(sizeof(CalculatorAstData));
  memset(data, 0, sizeof(CalculatorAstData));

  bench->data = data;

  bench->prepare = CalculatorAst_prepare;
  bench->run = CalculatorAst_run;
  bench->checksum = CalculatorAst_checksum;
  bench->cleanup = CalculatorAst_cleanup;

  return bench;
}

typedef struct {
  char *name;
  int64_t value;
  UT_hash_handle hh;
} VariableEntry;

typedef struct {
  VariableEntry *variables_hash;
} CalculatorInterpreterContext;

static CalculatorInterpreterContext *calculator_interpreter_context_new(void) {
  CalculatorInterpreterContext *ctx =
      malloc(sizeof(CalculatorInterpreterContext));
  ctx->variables_hash = NULL;
  return ctx;
}

static void
calculator_interpreter_context_free(CalculatorInterpreterContext *ctx) {
  VariableEntry *entry, *tmp;

  HASH_ITER(hh, ctx->variables_hash, entry, tmp) {
    free(entry->name);
    HASH_DEL(ctx->variables_hash, entry);
    free(entry);
  }

  free(ctx);
}

static int64_t *
calculator_interpreter_context_get(CalculatorInterpreterContext *ctx,
                                   const char *name) {
  VariableEntry *entry = NULL;
  HASH_FIND_STR(ctx->variables_hash, name, entry);
  return entry ? &entry->value : NULL;
}

static void
calculator_interpreter_context_set(CalculatorInterpreterContext *ctx,
                                   const char *name, int64_t value) {
  VariableEntry *entry = NULL;
  HASH_FIND_STR(ctx->variables_hash, name, entry);

  if (entry) {
    entry->value = value;
  } else {
    entry = malloc(sizeof(VariableEntry));
    entry->name = strdup(name);
    entry->value = value;
    HASH_ADD_KEYPTR(hh, ctx->variables_hash, entry->name, strlen(entry->name),
                    entry);
  }
}

static int64_t calculator_interpreter_simple_div(int64_t a, int64_t b) {
  if (b == 0)
    return 0;
  if ((a >= 0 && b > 0) || (a < 0 && b < 0)) {
    return a / b;
  } else {
    int64_t abs_a = a >= 0 ? a : -a;
    int64_t abs_b = b >= 0 ? b : -b;
    return -(abs_a / abs_b);
  }
}

static int64_t calculator_interpreter_simple_mod(int64_t a, int64_t b) {
  if (b == 0)
    return 0;
  return a - calculator_interpreter_simple_div(a, b) * b;
}

static int64_t
calculator_interpreter_evaluate(AST_Node *node,
                                CalculatorInterpreterContext *ctx) {
  switch (node->type) {
  case AST_NUMBER:
    return node->data.number_value;

  case AST_VARIABLE: {
    int64_t *value =
        calculator_interpreter_context_get(ctx, node->data.variable_name);
    return value ? *value : 0;
  }

  case AST_BINARY_OP: {
    AST_BinaryOp *binop = node->data.binary_op;
    int64_t left = calculator_interpreter_evaluate(binop->left, ctx);
    int64_t right = calculator_interpreter_evaluate(binop->right, ctx);

    switch (binop->op) {
    case '+':
      return left + right;
    case '-':
      return left - right;
    case '*':
      return left * right;
    case '/':
      return calculator_interpreter_simple_div(left, right);
    case '%':
      return calculator_interpreter_simple_mod(left, right);
    default:
      return 0;
    }
  }

  case AST_ASSIGNMENT: {
    AST_Assignment *assign = node->data.assignment;
    int64_t value = calculator_interpreter_evaluate(assign->expr, ctx);
    calculator_interpreter_context_set(ctx, assign->var_name, value);
    return value;
  }
  }
  return 0;
}

typedef struct {
  CalculatorAstData ast_data;
  uint32_t result_val;
} CalculatorInterpreterData;

void CalculatorInterpreter_prepare(Benchmark *self) {
  CalculatorInterpreterData *data = (CalculatorInterpreterData *)self->data;

  data->ast_data.operations = Helper_config_i64(self->name, "operations");
  if (data->ast_data.operations <= 0) {
    data->ast_data.operations = 1000;
  }

  data->ast_data.text =
      calculator_ast_generate_random_program(data->ast_data.operations);
  data->ast_data.expressions = NULL;
  data->ast_data.expressions_count = 0;
  data->ast_data.expressions_capacity = 0;

  CalculatorAstParser parser;
  calculator_ast_parser_init(&parser, data->ast_data.text);
  calculator_ast_parser_parse_all(&parser, &data->ast_data);

  data->result_val = 0;
}

void CalculatorInterpreter_run(Benchmark *self, int iteration_id) {
  (void)iteration_id;
  CalculatorInterpreterData *data = (CalculatorInterpreterData *)self->data;

  CalculatorInterpreterContext *ctx = calculator_interpreter_context_new();
  int64_t iteration_result = 0;

  for (int64_t i = 0; i < data->ast_data.expressions_count; i++) {
    iteration_result =
        calculator_interpreter_evaluate(data->ast_data.expressions[i], ctx);
  }

  calculator_interpreter_context_free(ctx);

  data->result_val += iteration_result;
}

uint32_t CalculatorInterpreter_checksum(Benchmark *self) {
  CalculatorInterpreterData *data = (CalculatorInterpreterData *)self->data;
  return data->result_val;
}

void CalculatorInterpreter_cleanup(Benchmark *self) {
  CalculatorInterpreterData *data = (CalculatorInterpreterData *)self->data;

  if (data->ast_data.text) {
    free(data->ast_data.text);
    data->ast_data.text = NULL;
  }

  if (data->ast_data.expressions) {
    for (int64_t i = 0; i < data->ast_data.expressions_count; i++) {
      ast_node_free(data->ast_data.expressions[i]);
    }
    free(data->ast_data.expressions);
    data->ast_data.expressions = NULL;
  }
}

Benchmark *CalculatorInterpreter_create(void) {
  Benchmark *bench = Benchmark_create("Calculator::Interpreter");

  CalculatorInterpreterData *data = malloc(sizeof(CalculatorInterpreterData));
  memset(data, 0, sizeof(CalculatorInterpreterData));

  bench->data = data;

  bench->prepare = CalculatorInterpreter_prepare;
  bench->run = CalculatorInterpreter_run;
  bench->checksum = CalculatorInterpreter_checksum;
  bench->cleanup = CalculatorInterpreter_cleanup;

  return bench;
}


static uint8_t *generate_test_data(int64_t size, size_t *data_size) {
  const char *pattern = "ABRACADABRA";
  size_t pattern_len = strlen(pattern);

  uint8_t *data = malloc(size);
  if (!data) {
    *data_size = 0;
    return NULL;
  }

  *data_size = size;
  for (int64_t i = 0; i < size; i++) {
    data[i] = pattern[i % pattern_len];
  }

  return data;
}

typedef struct {
  uint8_t *transformed;
  size_t transformed_size;
  size_t original_idx;
} BWTResult;

static BWTResult bwt_transform(uint8_t *input, size_t n) {
  BWTResult result = {0};

  if (n == 0 || !input) {
    return result;
  }

  uint8_t *doubled = malloc(n * 2);
  memcpy(doubled, input, n);
  memcpy(doubled + n, input, n);

  size_t *sa = malloc(sizeof(size_t) * n);

  for (size_t i = 0; i < n; i++) {
    sa[i] = i;
  }

  size_t *temp_buffer = malloc(sizeof(size_t) * n);
  size_t bucket_counts[256] = {0};

  for (size_t i = 0; i < n; i++) {
    bucket_counts[input[sa[i]]]++;
  }

  size_t bucket_starts[256];
  size_t sum = 0;
  for (int i = 0; i < 256; i++) {
    bucket_starts[i] = sum;
    sum += bucket_counts[i];
  }

  for (size_t i = 0; i < n; i++) {
    uint8_t c = input[sa[i]];
    temp_buffer[bucket_starts[c]++] = sa[i];
  }

  memcpy(sa, temp_buffer, sizeof(size_t) * n);

  if (n > 1) {
    int *rank = malloc(sizeof(int) * n);

    int current_rank = 0;
    uint8_t prev_char = input[sa[0]];

    for (size_t i = 0; i < n; i++) {
      size_t idx = sa[i];
      uint8_t curr_char = input[idx];
      if (curr_char != prev_char) {
        current_rank++;
        prev_char = curr_char;
      }
      rank[idx] = current_rank;
    }

    size_t k = 1;
    while (k < n) {
      struct Pair {
        int first, second;
      };
      struct Pair *pairs = malloc(sizeof(struct Pair) * n);

      for (size_t i = 0; i < n; i++) {
        pairs[i].first = rank[i];
        pairs[i].second = rank[(i + k) % n];
      }

      int *count = calloc(n + 1, sizeof(int));

      for (size_t i = 0; i < n; i++) {
        count[pairs[sa[i]].second]++;
      }

      for (size_t i = 1; i <= n; i++) {
        count[i] += count[i - 1];
      }

      memcpy(temp_buffer, sa, sizeof(size_t) * n);
      for (int i = n - 1; i >= 0; i--) {
        int key = pairs[temp_buffer[i]].second;
        sa[--count[key]] = temp_buffer[i];
      }

      memset(count, 0, (n + 1) * sizeof(int));

      for (size_t i = 0; i < n; i++) {
        count[pairs[sa[i]].first]++;
      }

      for (size_t i = 1; i <= n; i++) {
        count[i] += count[i - 1];
      }

      memcpy(temp_buffer, sa, sizeof(size_t) * n);
      for (int i = n - 1; i >= 0; i--) {
        int key = pairs[temp_buffer[i]].first;
        sa[--count[key]] = temp_buffer[i];
      }

      int *new_rank = malloc(sizeof(int) * n);

      new_rank[sa[0]] = 0;
      for (size_t i = 1; i < n; i++) {
        struct Pair prev_pair = pairs[sa[i - 1]];
        struct Pair curr_pair = pairs[sa[i]];
        new_rank[sa[i]] =
            new_rank[sa[i - 1]] + (prev_pair.first != curr_pair.first ||
                                           prev_pair.second != curr_pair.second
                                       ? 1
                                       : 0);
      }

      free(count);
      free(pairs);
      free(rank);
      rank = new_rank;
      k *= 2;
    }

    free(rank);
  }

  result.transformed = malloc(n);
  result.transformed_size = n;
  result.original_idx = 0;

  for (size_t i = 0; i < n; i++) {
    size_t suffix = sa[i];
    if (suffix == 0) {
      result.transformed[i] = input[n - 1];
      result.original_idx = i;
    } else {
      result.transformed[i] = input[suffix - 1];
    }
  }

  free(doubled);
  free(sa);
  free(temp_buffer);

  return result;
}

typedef struct {
  int64_t size_val;
  uint8_t *test_data;
  size_t test_data_size;
  uint32_t result_val;
} BWTEncodeData;

static void bwt_encode_prepare(Benchmark *self) {
  BWTEncodeData *data = (BWTEncodeData *)self->data;
  data->size_val = Helper_config_i64(self->name, "size");
  data->test_data = generate_test_data(data->size_val, &data->test_data_size);
  data->result_val = 0;
}

static void bwt_encode_run(Benchmark *self, int iteration_id) {
  (void)iteration_id;
  BWTEncodeData *data = (BWTEncodeData *)self->data;
  BWTResult result = bwt_transform(data->test_data, data->test_data_size);

  if (result.transformed) {
    data->result_val =
        (data->result_val + (uint32_t)result.transformed_size) & 0xFFFFFFFFu;
    free(result.transformed);
  }
}

static uint32_t bwt_encode_checksum(Benchmark *self) {
  BWTEncodeData *data = (BWTEncodeData *)self->data;
  return data->result_val;
}

static void bwt_encode_cleanup(Benchmark *self) {
  BWTEncodeData *data = (BWTEncodeData *)self->data;
  if (data->test_data) {
    free(data->test_data);
    data->test_data = NULL;
  }
  free(data);
  self->data = NULL;
}

Benchmark *BWTEncode_create(void) {
  Benchmark *bench = Benchmark_create("Compress::BWTEncode");
  BWTEncodeData *data = malloc(sizeof(BWTEncodeData));
  memset(data, 0, sizeof(BWTEncodeData));

  bench->data = data;
  bench->prepare = bwt_encode_prepare;
  bench->run = bwt_encode_run;
  bench->checksum = bwt_encode_checksum;
  bench->cleanup = bwt_encode_cleanup;

  return bench;
}

static uint8_t *bwt_inverse(BWTResult *bwt_result, size_t *result_size) {
  uint8_t *bwt = bwt_result->transformed;
  size_t n = bwt_result->transformed_size;

  if (n == 0) {
    *result_size = 0;
    return NULL;
  }

  int counts[256] = {0};
  for (size_t i = 0; i < n; i++) {
    counts[bwt[i]]++;
  }

  int positions[256];
  int total = 0;
  for (int i = 0; i < 256; i++) {
    positions[i] = total;
    total += counts[i];
  }

  size_t *next = malloc(sizeof(size_t) * n);

  int temp_counts[256] = {0};

  for (size_t i = 0; i < n; i++) {
    int byte_idx = bwt[i];
    int pos = positions[byte_idx] + temp_counts[byte_idx];
    next[pos] = i;
    temp_counts[byte_idx]++;
  }

  uint8_t *result = malloc(n);

  size_t idx = bwt_result->original_idx;

  for (size_t i = 0; i < n; i++) {
    idx = next[idx];
    result[i] = bwt[idx];
  }

  free(next);
  *result_size = n;
  return result;
}

typedef struct {
  int64_t size_val;
  uint8_t *test_data;
  size_t test_data_size;
  uint8_t *inverted;
  size_t inverted_size;
  BWTResult bwt_result;
  uint32_t result_val;
} BWTDecodeData;

static void bwt_decode_prepare(Benchmark *self) {
  BWTDecodeData *data = (BWTDecodeData *)self->data;
  data->size_val = Helper_config_i64(self->name, "size");
  data->test_data = generate_test_data(data->size_val, &data->test_data_size);
  data->bwt_result = bwt_transform(data->test_data, data->test_data_size);
  data->inverted = NULL;
  data->inverted_size = 0;
  data->result_val = 0;
}

static void bwt_decode_run(Benchmark *self, int iteration_id) {
  (void)iteration_id;
  BWTDecodeData *data = (BWTDecodeData *)self->data;

  if (data->inverted) {
    free(data->inverted);
    data->inverted = NULL;
  }

  data->inverted = bwt_inverse(&data->bwt_result, &data->inverted_size);

  if (data->inverted) {
    data->result_val =
        (data->result_val + (uint32_t)data->inverted_size) & 0xFFFFFFFFu;
  }
}

static uint32_t bwt_decode_checksum(Benchmark *self) {
  BWTDecodeData *data = (BWTDecodeData *)self->data;
  uint32_t res = data->result_val;

  if (data->inverted && data->test_data &&
      data->inverted_size == data->test_data_size) {
    if (memcmp(data->inverted, data->test_data, data->test_data_size) == 0) {
      res = (res + 100000) & 0xFFFFFFFFu;
    }
  }

  return res;
}

static void bwt_decode_cleanup(Benchmark *self) {
  BWTDecodeData *data = (BWTDecodeData *)self->data;

  if (data->test_data)
    free(data->test_data);
  if (data->inverted)
    free(data->inverted);
  if (data->bwt_result.transformed)
    free(data->bwt_result.transformed);

  free(data);
  self->data = NULL;
}

Benchmark *BWTDecode_create(void) {
  Benchmark *bench = Benchmark_create("Compress::BWTDecode");
  BWTDecodeData *data = malloc(sizeof(BWTDecodeData));
  memset(data, 0, sizeof(BWTDecodeData));

  bench->data = data;
  bench->prepare = bwt_decode_prepare;
  bench->run = bwt_decode_run;
  bench->checksum = bwt_decode_checksum;
  bench->cleanup = bwt_decode_cleanup;

  return bench;
}

typedef struct HuffmanNode {
  int frequency;
  uint8_t byte_val;
  bool is_leaf;
  struct HuffmanNode *left;
  struct HuffmanNode *right;
} HuffmanNode;

typedef struct {
  int code_lengths[256];
  int codes[256];
} HuffmanCodes;

typedef struct {
  uint8_t *data;
  size_t data_size;
  int bit_count;
  int frequencies[256];
} EncodedResult;

static int compare_huffman_nodes(const void *a, const void *b) {
  HuffmanNode *node_a = *(HuffmanNode **)a;
  HuffmanNode *node_b = *(HuffmanNode **)b;
  return node_a->frequency - node_b->frequency;
}

static HuffmanNode *build_huffman_tree(int frequencies[256]) {
  HuffmanNode **nodes = malloc(sizeof(HuffmanNode *) * 256);

  int node_count = 0;

  for (int i = 0; i < 256; i++) {
    if (frequencies[i] > 0) {
      HuffmanNode *node = malloc(sizeof(HuffmanNode));
      node->frequency = frequencies[i];
      node->byte_val = i;
      node->is_leaf = true;
      node->left = NULL;
      node->right = NULL;
      nodes[node_count++] = node;
    }
  }

  if (node_count == 0) {
    free(nodes);
    return NULL;
  }

  if (node_count == 1) {
    HuffmanNode *root = malloc(sizeof(HuffmanNode));
    root->frequency = nodes[0]->frequency;
    root->byte_val = 0;
    root->is_leaf = false;
    root->left = nodes[0];

    root->right = malloc(sizeof(HuffmanNode));
    root->right->frequency = 0;
    root->right->byte_val = 0;
    root->right->is_leaf = true;
    root->right->left = NULL;
    root->right->right = NULL;

    free(nodes);
    return root;
  }

  qsort(nodes, node_count, sizeof(HuffmanNode *), compare_huffman_nodes);

  while (node_count > 1) {
    HuffmanNode *left = nodes[0];
    HuffmanNode *right = nodes[1];

    HuffmanNode *parent = malloc(sizeof(HuffmanNode));

    parent->frequency = left->frequency + right->frequency;
    parent->byte_val = 0;
    parent->is_leaf = false;
    parent->left = left;
    parent->right = right;

    nodes[0] = parent;
    for (int i = 1; i < node_count - 1; i++) {
      nodes[i] = nodes[i + 1];
    }
    node_count--;

    qsort(nodes, node_count, sizeof(HuffmanNode *), compare_huffman_nodes);
  }

  HuffmanNode *root = nodes[0];
  free(nodes);
  return root;
}

static void build_huffman_codes(HuffmanNode *node, int code, int length,
                                HuffmanCodes *huffman_codes) {
  if (!node)
    return;

  if (node->is_leaf) {
    int idx = node->byte_val;
    huffman_codes->code_lengths[idx] = length;
    huffman_codes->codes[idx] = code;
  } else {
    if (node->left) {
      build_huffman_codes(node->left, code << 1, length + 1, huffman_codes);
    }
    if (node->right) {
      build_huffman_codes(node->right, (code << 1) | 1, length + 1,
                          huffman_codes);
    }
  }
}

static void free_huffman_tree(HuffmanNode *node) {
  if (!node)
    return;
  if (!node->is_leaf) {
    free_huffman_tree(node->left);
    free_huffman_tree(node->right);
  }
  free(node);
}

static EncodedResult huffman_encode(uint8_t *data, size_t data_size,
                                    HuffmanCodes *huffman_codes,
                                    int frequencies[256]) {
  EncodedResult result = {0};

  if (!data || data_size == 0 || !huffman_codes) {
    return result;
  }

  memcpy(result.frequencies, frequencies, sizeof(int) * 256);

  size_t max_size = data_size * 2;
  result.data = malloc(max_size);
  if (!result.data) {
    return result;
  }

  result.data_size = 0;
  result.bit_count = 0;

  uint8_t current_byte = 0;
  int bit_pos = 0;

  for (size_t i = 0; i < data_size; i++) {
    int idx = data[i];
    int code = huffman_codes->codes[idx];
    int length = huffman_codes->code_lengths[idx];

    if (length <= 0)
      continue;

    for (int j = length - 1; j >= 0; j--) {
      if ((code & (1 << j)) != 0) {
        current_byte |= (1 << (7 - bit_pos));
      }
      bit_pos++;
      result.bit_count++;

      if (bit_pos == 8) {

        if (result.data_size >= max_size) {
          max_size *= 2;
          uint8_t *new_data = realloc(result.data, max_size);
          if (!new_data) {
            free(result.data);
            result.data = NULL;
            return result;
          }
          result.data = new_data;
        }
        result.data[result.data_size++] = current_byte;
        current_byte = 0;
        bit_pos = 0;
      }
    }
  }

  if (bit_pos > 0) {
    if (result.data_size >= max_size) {
      max_size *= 2;
      uint8_t *new_data = realloc(result.data, max_size);
      if (!new_data) {
        free(result.data);
        result.data = NULL;
        return result;
      }
      result.data = new_data;
    }
    result.data[result.data_size++] = current_byte;
  }

  if (result.data_size < max_size) {
    uint8_t *new_data = realloc(result.data, result.data_size);
    if (new_data) {
      result.data = new_data;
    }
  }

  return result;
}

static uint8_t *huffman_decode(uint8_t *encoded, size_t encoded_size,
                               HuffmanNode *root, int bit_count,
                               size_t *result_size) {
  if (!root || bit_count <= 0 || !encoded) {
    *result_size = 0;
    return NULL;
  }

  size_t max_size = bit_count;
  uint8_t *result = malloc(max_size);
  if (!result) {
    *result_size = 0;
    return NULL;
  }

  size_t result_idx = 0;
  HuffmanNode *current_node = root;
  int bits_processed = 0;
  size_t byte_index = 0;

  while (bits_processed < bit_count && byte_index < encoded_size) {
    uint8_t byte_val = encoded[byte_index++];

    for (int bit_pos = 7; bit_pos >= 0 && bits_processed < bit_count;
         bit_pos--) {
      int bit = (byte_val >> bit_pos) & 1;
      bits_processed++;

      current_node = bit ? current_node->right : current_node->left;

      if (!current_node)
        break;

      if (current_node->is_leaf) {
        result[result_idx++] = current_node->byte_val;
        current_node = root;
      }
    }
  }

  *result_size = result_idx;
  return result;
}

typedef struct {
  int64_t size_val;
  uint8_t *test_data;
  size_t test_data_size;
  EncodedResult encoded;
  uint32_t result_val;
} HuffEncodeData;

static void huff_encode_prepare(Benchmark *self) {
  HuffEncodeData *data = (HuffEncodeData *)self->data;
  data->size_val = Helper_config_i64(self->name, "size");
  data->test_data = generate_test_data(data->size_val, &data->test_data_size);
  data->result_val = 0;
}

static void huff_encode_run(Benchmark *self, int iteration_id) {
  (void)iteration_id;
  HuffEncodeData *data = (HuffEncodeData *)self->data;

  if (data->encoded.data) {
    free(data->encoded.data);
    data->encoded.data = NULL;
  }

  int frequencies[256] = {0};
  for (size_t i = 0; i < data->test_data_size; i++) {
    frequencies[data->test_data[i]]++;
  }

  HuffmanNode *tree = build_huffman_tree(frequencies);
  if (!tree)
    return;

  HuffmanCodes codes = {0};
  build_huffman_codes(tree, 0, 0, &codes);

  data->encoded = huffman_encode(data->test_data, data->test_data_size, &codes,
                                 frequencies);

  if (data->encoded.data) {
    data->result_val =
        (data->result_val + (uint32_t)data->encoded.data_size) & 0xFFFFFFFFu;
  }

  free_huffman_tree(tree);
}

static uint32_t huff_encode_checksum(Benchmark *self) {
  HuffEncodeData *data = (HuffEncodeData *)self->data;
  return data->result_val;
}

static void huff_encode_cleanup(Benchmark *self) {
  HuffEncodeData *data = (HuffEncodeData *)self->data;
  if (data->test_data) {
    free(data->test_data);
    data->test_data = NULL;
  }
  if (data->encoded.data) {
    free(data->encoded.data);
    data->encoded.data = NULL;
  }
  free(data);
  self->data = NULL;
}

Benchmark *HuffEncode_create(void) {
  Benchmark *bench = Benchmark_create("Compress::HuffEncode");
  HuffEncodeData *data = malloc(sizeof(HuffEncodeData));
  memset(data, 0, sizeof(HuffEncodeData));

  bench->data = data;
  bench->prepare = huff_encode_prepare;
  bench->run = huff_encode_run;
  bench->checksum = huff_encode_checksum;
  bench->cleanup = huff_encode_cleanup;

  return bench;
}

typedef struct {
  int64_t size_val;
  uint8_t *test_data;
  size_t test_data_size;
  uint8_t *decoded;
  size_t decoded_size;
  EncodedResult encoded;
  uint32_t result_val;
} HuffDecodeData;

static void huff_decode_prepare(Benchmark *self) {
  HuffDecodeData *data = (HuffDecodeData *)self->data;
  data->size_val = Helper_config_i64(self->name, "size");
  data->test_data = generate_test_data(data->size_val, &data->test_data_size);
  data->result_val = 0;
  data->decoded = NULL;
  data->decoded_size = 0;

  HuffEncodeData *encoder_data = malloc(sizeof(HuffEncodeData));
  encoder_data->size_val = data->size_val;
  encoder_data->test_data = data->test_data;
  encoder_data->test_data_size = data->test_data_size;

  int frequencies[256] = {0};
  for (size_t i = 0; i < data->test_data_size; i++) {
    frequencies[data->test_data[i]]++;
  }

  HuffmanNode *tree = build_huffman_tree(frequencies);
  if (tree) {
    HuffmanCodes codes = {0};
    build_huffman_codes(tree, 0, 0, &codes);
    data->encoded = huffman_encode(data->test_data, data->test_data_size,
                                   &codes, frequencies);
    free_huffman_tree(tree);
  }

  free(encoder_data);
}

static void huff_decode_run(Benchmark *self, int iteration_id) {
  (void)iteration_id;
  HuffDecodeData *data = (HuffDecodeData *)self->data;

  HuffmanNode *tree = build_huffman_tree(data->encoded.frequencies);
  if (!tree)
    return;

  if (data->decoded)
    free(data->decoded);

  data->decoded =
      huffman_decode(data->encoded.data, data->encoded.data_size, tree,
                     data->encoded.bit_count, &data->decoded_size);

  if (data->decoded) {
    data->result_val =
        (data->result_val + (uint32_t)data->decoded_size) & 0xFFFFFFFFu;
  }

  free_huffman_tree(tree);
}

static uint32_t huff_decode_checksum(Benchmark *self) {
  HuffDecodeData *data = (HuffDecodeData *)self->data;
  uint32_t res = data->result_val;

  if (data->decoded && data->test_data &&
      data->decoded_size == data->test_data_size) {
    if (memcmp(data->decoded, data->test_data, data->test_data_size) == 0) {
      res = (res + 100000) & 0xFFFFFFFFu;
    }
  }

  return res;
}

static void huff_decode_cleanup(Benchmark *self) {
  HuffDecodeData *data = (HuffDecodeData *)self->data;

  if (data->test_data)
    free(data->test_data);
  if (data->decoded)
    free(data->decoded);
  if (data->encoded.data)
    free(data->encoded.data);

  free(data);
  self->data = NULL;
}

Benchmark *HuffDecode_create(void) {
  Benchmark *bench = Benchmark_create("Compress::HuffDecode");
  HuffDecodeData *data = malloc(sizeof(HuffDecodeData));
  memset(data, 0, sizeof(HuffDecodeData));

  bench->data = data;
  bench->prepare = huff_decode_prepare;
  bench->run = huff_decode_run;
  bench->checksum = huff_decode_checksum;
  bench->cleanup = huff_decode_cleanup;

  return bench;
}

typedef struct {
  int total;
  int low[256];
  int high[256];
} ArithFreqTable;

static ArithFreqTable *create_arith_freq_table(int frequencies[256]) {
  ArithFreqTable *ft = malloc(sizeof(ArithFreqTable));
  if (!ft)
    return NULL;

  ft->total = 0;
  for (int i = 0; i < 256; i++)
    ft->total += frequencies[i];

  int cum = 0;
  for (int i = 0; i < 256; i++) {
    ft->low[i] = cum;
    cum += frequencies[i];
    ft->high[i] = cum;
  }

  return ft;
}

typedef struct {
  int buffer;
  int bit_pos;
  uint8_t *bytes;
  size_t bytes_size;
  size_t bytes_capacity;
  int bits_written;
} BitOutputStream;

static BitOutputStream *bit_output_create(void) {
  BitOutputStream *out = malloc(sizeof(BitOutputStream));
  if (!out)
    return NULL;

  out->buffer = 0;
  out->bit_pos = 0;
  out->bytes_capacity = 1024;
  out->bytes = malloc(out->bytes_capacity);
  if (!out->bytes) {
    free(out);
    return NULL;
  }
  out->bytes_size = 0;
  out->bits_written = 0;

  return out;
}

static void bit_output_write_bit(BitOutputStream *out, int bit) {
  if (!out)
    return;

  out->buffer = (out->buffer << 1) | (bit & 1);
  out->bit_pos++;
  out->bits_written++;

  if (out->bit_pos == 8) {
    if (out->bytes_size >= out->bytes_capacity) {
      out->bytes_capacity *= 2;
      uint8_t *new_bytes = realloc(out->bytes, out->bytes_capacity);
      if (!new_bytes)
        return;
      out->bytes = new_bytes;
    }
    out->bytes[out->bytes_size++] = (uint8_t)out->buffer;
    out->buffer = 0;
    out->bit_pos = 0;
  }
}

static uint8_t *bit_output_flush(BitOutputStream *out, size_t *size) {
  if (!out) {
    *size = 0;
    return NULL;
  }

  if (out->bit_pos > 0) {
    out->buffer <<= (8 - out->bit_pos);
    if (out->bytes_size >= out->bytes_capacity) {
      out->bytes_capacity *= 2;
      uint8_t *new_bytes = realloc(out->bytes, out->bytes_capacity);
      if (!new_bytes) {
        *size = out->bytes_size;
        return out->bytes;
      }
      out->bytes = new_bytes;
    }
    out->bytes[out->bytes_size++] = (uint8_t)out->buffer;
    out->buffer = 0;
    out->bit_pos = 0;
  }

  *size = out->bytes_size;

  uint8_t *result = malloc(out->bytes_size);
  if (result) {
    memcpy(result, out->bytes, out->bytes_size);
  }
  return result;
}

static void bit_output_free(BitOutputStream *out) {
  if (out) {
    if (out->bytes) {
      free(out->bytes);
      out->bytes = NULL;
    }
    free(out);
  }
}

typedef struct {
  uint8_t *data;
  size_t data_size;
  int bit_count;
  int frequencies[256];
} ArithEncodedResult;

static ArithEncodedResult arith_encode(uint8_t *data, size_t data_size) {
  ArithEncodedResult result = {0};

  if (!data || data_size == 0)
    return result;

  int frequencies[256] = {0};
  for (size_t i = 0; i < data_size; i++) {
    frequencies[data[i]]++;
  }

  ArithFreqTable *freq_table = create_arith_freq_table(frequencies);
  if (!freq_table)
    return result;

  uint64_t low = 0;
  uint64_t high = 0xFFFFFFFF;
  int pending = 0;
  BitOutputStream *output = bit_output_create();
  if (!output) {
    free(freq_table);
    return result;
  }

  for (size_t i = 0; i < data_size; i++) {
    int idx = data[i];
    uint64_t range = high - low + 1;

    high = low +
           (range * (uint64_t)freq_table->high[idx] /
            (uint64_t)freq_table->total) -
           1;
    low = low + (range * (uint64_t)freq_table->low[idx] /
                 (uint64_t)freq_table->total);

    while (1) {
      if (high < 0x80000000) {
        bit_output_write_bit(output, 0);
        for (int i = 0; i < pending; i++)
          bit_output_write_bit(output, 1);
        pending = 0;
      } else if (low >= 0x80000000) {
        bit_output_write_bit(output, 1);
        for (int i = 0; i < pending; i++)
          bit_output_write_bit(output, 0);
        pending = 0;
        low -= 0x80000000;
        high -= 0x80000000;
      } else if (low >= 0x40000000 && high < 0xC0000000) {
        pending++;
        low -= 0x40000000;
        high -= 0x40000000;
      } else {
        break;
      }

      low <<= 1;
      high = (high << 1) | 1;
      high &= 0xFFFFFFFF;
    }
  }

  pending++;
  if (low < 0x40000000) {
    bit_output_write_bit(output, 0);
    for (int i = 0; i < pending; i++)
      bit_output_write_bit(output, 1);
  } else {
    bit_output_write_bit(output, 1);
    for (int i = 0; i < pending; i++)
      bit_output_write_bit(output, 0);
  }

  size_t data_size_out;
  uint8_t *encoded_data = bit_output_flush(output, &data_size_out);

  result.data = encoded_data;
  result.data_size = data_size_out;
  result.bit_count = output->bits_written;
  memcpy(result.frequencies, frequencies, sizeof(frequencies));

  bit_output_free(output);
  free(freq_table);

  return result;
}

typedef struct {
  int64_t size_val;
  uint8_t *test_data;
  size_t test_data_size;
  uint32_t result_val;
} ArithEncodeData;

static void arith_encode_prepare(Benchmark *self) {
  ArithEncodeData *data = (ArithEncodeData *)self->data;
  if (!data)
    return;

  data->size_val = Helper_config_i64(self->name, "size");
  if (data->size_val == 0)
    data->size_val = 1000;

  if (data->test_data) {
    free(data->test_data);
    data->test_data = NULL;
  }

  data->test_data = generate_test_data(data->size_val, &data->test_data_size);
  data->result_val = 0;
}

static void arith_encode_run(Benchmark *self, int iteration_id) {
  (void)iteration_id;
  ArithEncodeData *data = (ArithEncodeData *)self->data;
  if (!data || !data->test_data)
    return;

  ArithEncodedResult result =
      arith_encode(data->test_data, data->test_data_size);

  if (result.data) {
    data->result_val =
        (data->result_val + (uint32_t)result.data_size) & 0xFFFFFFFFu;
    free(result.data);
  }
}

static uint32_t arith_encode_checksum(Benchmark *self) {
  ArithEncodeData *data = (ArithEncodeData *)self->data;
  return data ? data->result_val : 0;
}

static void arith_encode_cleanup(Benchmark *self) {
  ArithEncodeData *data = (ArithEncodeData *)self->data;
  if (data) {
    if (data->test_data)
      free(data->test_data);
    free(data);
  }
  self->data = NULL;
}

Benchmark *ArithEncode_create(void) {
  Benchmark *bench = Benchmark_create("Compress::ArithEncode");
  if (!bench)
    return NULL;

  ArithEncodeData *data = malloc(sizeof(ArithEncodeData));
  if (!data) {
    free(bench);
    return NULL;
  }
  memset(data, 0, sizeof(ArithEncodeData));

  bench->data = data;
  bench->prepare = arith_encode_prepare;
  bench->run = arith_encode_run;
  bench->checksum = arith_encode_checksum;
  bench->cleanup = arith_encode_cleanup;

  return bench;
}

typedef struct {
  int64_t size_val;
  uint8_t *test_data;
  size_t test_data_size;
  uint8_t *decoded;
  size_t decoded_size;
  ArithEncodedResult encoded;
  uint32_t result_val;
} ArithDecodeData;

typedef struct {
  const uint8_t *bytes;
  size_t bytes_size;
  size_t byte_pos;
  int bit_pos;
  uint8_t current_byte;
} BitInputStream;

static void bit_input_init(BitInputStream *in, const uint8_t *bytes,
                           size_t size) {
  in->bytes = bytes;
  in->bytes_size = size;
  in->byte_pos = 0;
  in->bit_pos = 0;
  in->current_byte = size > 0 ? bytes[0] : 0;
}

static int bit_input_read_bit(BitInputStream *in) {
  if (in->bit_pos == 8) {
    in->byte_pos++;
    in->bit_pos = 0;
    in->current_byte =
        in->byte_pos < in->bytes_size ? in->bytes[in->byte_pos] : 0;
  }

  int bit = (in->current_byte >> (7 - in->bit_pos)) & 1;
  in->bit_pos++;
  return bit;
}

static uint8_t *arith_decode(ArithEncodedResult *encoded, size_t *result_size) {
  if (!encoded || !encoded->data || encoded->data_size == 0) {
    *result_size = 0;
    return NULL;
  }

  int total = 0;
  for (int i = 0; i < 256; i++)
    total += encoded->frequencies[i];
  int data_size = total;

  if (data_size == 0) {
    *result_size = 0;
    return NULL;
  }

  int low_table[256], high_table[256];
  int cum = 0;
  for (int i = 0; i < 256; i++) {
    low_table[i] = cum;
    cum += encoded->frequencies[i];
    high_table[i] = cum;
  }

  uint8_t *result = malloc(data_size);
  if (!result) {
    *result_size = 0;
    return NULL;
  }

  BitInputStream input;
  bit_input_init(&input, encoded->data, encoded->data_size);

  uint64_t value = 0;
  for (int i = 0; i < 32; i++) {
    value = (value << 1) | bit_input_read_bit(&input);
  }

  uint64_t low = 0;
  uint64_t high = 0xFFFFFFFF;

  for (int j = 0; j < data_size; j++) {
    uint64_t range = high - low + 1;
    uint64_t scaled = ((value - low + 1) * (uint64_t)total - 1) / range;

    int left = 0;
    int right = 256;
    while (left < right) {
      int mid = (left + right) / 2;
      if ((uint64_t)high_table[mid] <= scaled) {
        left = mid + 1;
      } else {
        right = mid;
      }
    }
    int symbol = left;

    result[j] = (uint8_t)symbol;

    high = low + (range * (uint64_t)high_table[symbol] / (uint64_t)total) - 1;
    low = low + (range * (uint64_t)low_table[symbol] / (uint64_t)total);

    while (1) {
      if (high < 0x80000000) {

      } else if (low >= 0x80000000) {
        value -= 0x80000000;
        low -= 0x80000000;
        high -= 0x80000000;
      } else if (low >= 0x40000000 && high < 0xC0000000) {
        value -= 0x40000000;
        low -= 0x40000000;
        high -= 0x40000000;
      } else {
        break;
      }

      low <<= 1;
      high = (high << 1) | 1;
      value = (value << 1) | bit_input_read_bit(&input);
    }
  }

  *result_size = data_size;
  return result;
}

static void arith_decode_prepare(Benchmark *self) {
  ArithDecodeData *data = (ArithDecodeData *)self->data;
  if (!data)
    return;

  data->size_val = Helper_config_i64(self->name, "size");
  if (data->size_val == 0)
    data->size_val = 1000;

  if (data->test_data) {
    free(data->test_data);
    data->test_data = NULL;
  }

  data->test_data = generate_test_data(data->size_val, &data->test_data_size);

  if (data->encoded.data) {
    free(data->encoded.data);
    memset(&data->encoded, 0, sizeof(ArithEncodedResult));
  }

  if (data->encoded.data) {
    free(data->encoded.data);
    memset(&data->encoded, 0, sizeof(ArithEncodedResult));
  }

  data->encoded = arith_encode(data->test_data, data->test_data_size);

  data->decoded = NULL;
  data->decoded_size = 0;
  data->result_val = 0;
}

static void arith_decode_run(Benchmark *self, int iteration_id) {
  (void)iteration_id;
  ArithDecodeData *data = (ArithDecodeData *)self->data;
  if (data->decoded) {
    free(data->decoded);
    data->decoded = NULL;
  }

  data->decoded = arith_decode(&data->encoded, &data->decoded_size);

  if (data->decoded) {
    data->result_val =
        (data->result_val + (uint32_t)data->decoded_size) & 0xFFFFFFFFu;
  }
}

static uint32_t arith_decode_checksum(Benchmark *self) {
  ArithDecodeData *data = (ArithDecodeData *)self->data;
  if (!data)
    return 0;

  uint32_t res = data->result_val;

  if (data->decoded && data->test_data &&
      data->decoded_size == data->test_data_size) {
    if (memcmp(data->decoded, data->test_data, data->test_data_size) == 0) {
      res = (res + 100000) & 0xFFFFFFFFu;
    }
  }

  return res;
}

static void arith_decode_cleanup(Benchmark *self) {
  ArithDecodeData *data = (ArithDecodeData *)self->data;
  if (data) {
    if (data->test_data)
      free(data->test_data);
    if (data->decoded)
      free(data->decoded);
    if (data->encoded.data)
      free(data->encoded.data);
    free(data);
  }
  self->data = NULL;
}

Benchmark *ArithDecode_create(void) {
  Benchmark *bench = Benchmark_create("Compress::ArithDecode");
  if (!bench)
    return NULL;

  ArithDecodeData *data = malloc(sizeof(ArithDecodeData));
  if (!data) {
    free(bench);
    return NULL;
  }
  memset(data, 0, sizeof(ArithDecodeData));

  bench->data = data;
  bench->prepare = arith_decode_prepare;
  bench->run = arith_decode_run;
  bench->checksum = arith_decode_checksum;
  bench->cleanup = arith_decode_cleanup;

  return bench;
}

typedef struct {
  uint8_t *data;
  size_t data_size;
  int dict_size;
} LZWResult;

typedef struct {
  int64_t size_val;
  uint8_t *test_data;
  size_t test_data_size;
  uint32_t result_val;
} LZWEncodeData;

typedef struct {
  char *key;
  int32_t value;
  UT_hash_handle hh;
} DictEntry;

static LZWResult lzw_encode(uint8_t *input, size_t input_size) {
  LZWResult result = {0};
  result.dict_size = 256;

  if (input_size == 0) {
    result.data = NULL;
    result.data_size = 0;
    return result;
  }

  DictEntry *dict = NULL;

  for (int i = 0; i < 256; i++) {
    char *key = malloc(2);
    key[0] = (char)i;
    key[1] = '\0';

    DictEntry *entry = malloc(sizeof(DictEntry));
    entry->key = key;
    entry->value = i;
    HASH_ADD_KEYPTR(hh, dict, entry->key, strlen(entry->key), entry);
  }

  int32_t next_code = 256;

  result.data = malloc(input_size * 2);
  result.data_size = 0;

  char *current = malloc(2);
  current[0] = (char)input[0];
  current[1] = '\0';

  for (size_t i = 1; i < input_size; i++) {

    char next_char[2] = {(char)input[i], '\0'};

    char *new_str = malloc(strlen(current) + 2);
    strcpy(new_str, current);
    strcat(new_str, next_char);

    DictEntry *entry = NULL;
    HASH_FIND_STR(dict, new_str, entry);

    if (entry) {

      free(current);
      current = new_str;
    } else {

      DictEntry *current_entry = NULL;
      HASH_FIND_STR(dict, current, current_entry);
      int32_t code = current_entry->value;

      result.data[result.data_size++] = (code >> 8) & 0xFF;
      result.data[result.data_size++] = code & 0xFF;

      DictEntry *new_entry = malloc(sizeof(DictEntry));
      new_entry->key = new_str;
      new_entry->value = next_code++;
      HASH_ADD_KEYPTR(hh, dict, new_entry->key, strlen(new_entry->key),
                      new_entry);

      free(current);
      current = malloc(2);
      current[0] = (char)input[i];
      current[1] = '\0';
    }
  }

  DictEntry *last_entry = NULL;
  HASH_FIND_STR(dict, current, last_entry);
  int32_t last_code = last_entry->value;
  result.data[result.data_size++] = (last_code >> 8) & 0xFF;
  result.data[result.data_size++] = last_code & 0xFF;

  result.dict_size = next_code;

  free(current);

  DictEntry *entry, *tmp;
  HASH_ITER(hh, dict, entry, tmp) {
    HASH_DEL(dict, entry);
    free(entry->key);
    free(entry);
  }

  return result;
}

static void lzw_encode_prepare(Benchmark *self) {
  LZWEncodeData *data = (LZWEncodeData *)self->data;
  data->size_val = Helper_config_i64(self->name, "size");
  data->test_data = generate_test_data(data->size_val, &data->test_data_size);
  data->result_val = 0;
}

static void lzw_encode_run(Benchmark *self, int iteration_id) {
  (void)iteration_id;
  LZWEncodeData *data = (LZWEncodeData *)self->data;
  LZWResult result = lzw_encode(data->test_data, data->test_data_size);

  if (result.data) {
    data->result_val =
        (data->result_val + (uint32_t)result.data_size) & 0xFFFFFFFFu;
    free(result.data);
  }
}

static uint32_t lzw_encode_checksum(Benchmark *self) {
  LZWEncodeData *data = (LZWEncodeData *)self->data;
  return data->result_val;
}

static void lzw_encode_cleanup(Benchmark *self) {
  LZWEncodeData *data = (LZWEncodeData *)self->data;
  if (data->test_data)
    free(data->test_data);
  free(data);
  self->data = NULL;
}

Benchmark *LZWEncode_create(void) {
  Benchmark *bench = Benchmark_create("Compress::LZWEncode");
  LZWEncodeData *data = malloc(sizeof(LZWEncodeData));
  memset(data, 0, sizeof(LZWEncodeData));

  bench->data = data;
  bench->prepare = lzw_encode_prepare;
  bench->run = lzw_encode_run;
  bench->checksum = lzw_encode_checksum;
  bench->cleanup = lzw_encode_cleanup;

  return bench;
}

typedef struct {
  int64_t size_val;
  uint8_t *test_data;
  size_t test_data_size;
  uint8_t *decoded;
  size_t decoded_size;
  LZWResult encoded;
  uint32_t result_val;
} LZWDecodeData;

static uint8_t *lzw_decode(LZWResult *encoded, size_t *result_size) {
  if (!encoded || encoded->data_size == 0) {
    *result_size = 0;
    return NULL;
  }

  size_t dict_capacity = 4096;
  char **dict = malloc(dict_capacity * sizeof(char *));
  if (!dict) {
    *result_size = 0;
    return NULL;
  }
  size_t dict_size = 256;

  for (int i = 0; i < 256; i++) {
    dict[i] = malloc(2);
    dict[i][0] = (char)i;
    dict[i][1] = '\0';
  }

  size_t result_capacity = encoded->data_size * 2;
  uint8_t *result = malloc(result_capacity);
  if (!result) {
    for (size_t i = 0; i < dict_size; i++)
      free(dict[i]);
    free(dict);
    *result_size = 0;
    return NULL;
  }
  size_t result_pos = 0;

  uint8_t *data = encoded->data;
  size_t pos = 0;

  if (pos + 1 >= encoded->data_size) {
    free(result);
    for (size_t i = 0; i < dict_size; i++)
      free(dict[i]);
    free(dict);
    *result_size = 0;
    return NULL;
  }

  uint16_t high = data[pos];
  uint16_t low = data[pos + 1];
  int32_t old_code = (high << 8) | low;
  pos += 2;

  if (old_code < 0 || old_code >= (int32_t)dict_size) {
    free(result);
    for (size_t i = 0; i < dict_size; i++)
      free(dict[i]);
    free(dict);
    *result_size = 0;
    return NULL;
  }

  char *old_str = dict[old_code];
  size_t old_len = strlen(old_str);

  if (result_pos + old_len >= result_capacity) {
    result_capacity *= 2;
    result = realloc(result, result_capacity);
  }
  for (size_t i = 0; i < old_len; i++) {
    result[result_pos++] = old_str[i];
  }

  int32_t next_code = 256;

  while (pos + 1 < encoded->data_size) {
    high = data[pos];
    low = data[pos + 1];
    int32_t new_code = (high << 8) | low;
    pos += 2;

    char *new_str;
    if (new_code >= 0 && new_code < (int32_t)dict_size) {
      new_str = dict[new_code];
    } else if (new_code == next_code) {
      size_t len = strlen(dict[old_code]);
      new_str = malloc(len + 2);
      strcpy(new_str, dict[old_code]);
      new_str[len] = dict[old_code][0];
      new_str[len + 1] = '\0';
    } else {
      free(result);
      for (size_t i = 0; i < dict_size; i++)
        free(dict[i]);
      free(dict);
      *result_size = 0;
      return NULL;
    }

    size_t new_len = strlen(new_str);

    if (result_pos + new_len >= result_capacity) {
      result_capacity *= 2;
      result = realloc(result, result_capacity);
    }
    for (size_t i = 0; i < new_len; i++) {
      result[result_pos++] = new_str[i];
    }

    if (new_code == next_code) {
      if (dict_size >= dict_capacity) {
        dict_capacity *= 2;
        dict = realloc(dict, dict_capacity * sizeof(char *));
      }
      dict[dict_size++] = new_str;
    } else {
      size_t len = strlen(dict[old_code]);
      char *entry = malloc(len + 2);
      strcpy(entry, dict[old_code]);
      entry[len] = new_str[0];
      entry[len + 1] = '\0';

      if (dict_size >= dict_capacity) {
        dict_capacity *= 2;
        dict = realloc(dict, dict_capacity * sizeof(char *));
      }
      dict[dict_size++] = entry;
    }

    next_code++;
    old_code = new_code;
  }

  *result_size = result_pos;

  for (size_t i = 0; i < dict_size; i++) {
    free(dict[i]);
  }
  free(dict);

  return result;
}

static void lzw_decode_prepare(Benchmark *self) {
  LZWDecodeData *data = (LZWDecodeData *)self->data;
  data->size_val = Helper_config_i64(self->name, "size");
  data->test_data = generate_test_data(data->size_val, &data->test_data_size);
  data->encoded = lzw_encode(data->test_data, data->test_data_size);
  data->decoded = NULL;
  data->decoded_size = 0;
  data->result_val = 0;
}

static void lzw_decode_run(Benchmark *self, int iteration_id) {
  (void)iteration_id;
  LZWDecodeData *data = (LZWDecodeData *)self->data;

  if (data->decoded) {
    free(data->decoded);
    data->decoded = NULL;
  }

  data->decoded = lzw_decode(&data->encoded, &data->decoded_size);

  if (data->decoded) {
    data->result_val =
        (data->result_val + (uint32_t)data->decoded_size) & 0xFFFFFFFFu;
  }
}

static uint32_t lzw_decode_checksum(Benchmark *self) {
  LZWDecodeData *data = (LZWDecodeData *)self->data;
  uint32_t res = data->result_val;

  if (data->decoded && data->test_data &&
      data->decoded_size == data->test_data_size) {
    if (memcmp(data->decoded, data->test_data, data->test_data_size) == 0) {
      res = (res + 100000) & 0xFFFFFFFFu;
    }
  }

  return res;
}

static void lzw_decode_cleanup(Benchmark *self) {
  LZWDecodeData *data = (LZWDecodeData *)self->data;

  if (data->test_data)
    free(data->test_data);
  if (data->decoded)
    free(data->decoded);
  if (data->encoded.data)
    free(data->encoded.data);

  free(data);
  self->data = NULL;
}

Benchmark *LZWDecode_create(void) {
  Benchmark *bench = Benchmark_create("Compress::LZWDecode");
  LZWDecodeData *data = malloc(sizeof(LZWDecodeData));
  memset(data, 0, sizeof(LZWDecodeData));

  bench->data = data;
  bench->prepare = lzw_decode_prepare;
  bench->run = lzw_decode_run;
  bench->checksum = lzw_decode_checksum;
  bench->cleanup = lzw_decode_cleanup;

  return bench;
}


typedef struct {
  char *csv_data;
  uint32_t result_val;
  int64_t rows;
} CsvParseData;

typedef struct {
  double x, y, z;
} Point;

typedef struct {
  Point *items;
  int count;
  int capacity;
} PointList;

static char *generate_csv_for_parsing(int64_t rows) {
  size_t capacity = rows * 100 + 1;
  char *buffer = malloc(capacity);
  if (!buffer)
    return NULL;

  size_t pos = 0;
  for (int64_t i = 0; i < rows; i++) {
    char c = 'A' + (i % 26);
    double x = Helper_next_float(1.0);
    double z = Helper_next_float(1.0);
    double y = Helper_next_float(1.0);
    pos += snprintf(
        buffer + pos, capacity - pos,
        "\"point %c\\n, \"\"%lld\"\"\",%.10f,,%.10f,\"[%s\\n, %lld]\",%.10f\n",
        c, (long long)(i % 100), x, z, (i % 2 == 0) ? "true" : "false",
        (long long)(i % 100), y);
  }
  return buffer;
}

void CsvParse_prepare(Benchmark *self) {
  CsvParseData *data = (CsvParseData *)self->data;
  data->rows = Helper_config_i64(self->name, "rows");
  data->csv_data = generate_csv_for_parsing(data->rows);
  data->result_val = 0;
}

static void point_list_init(PointList *list) {
  list->items = NULL;
  list->count = 0;
  list->capacity = 0;
}

static int point_list_add(PointList *list, Point point) {
  if (list->count >= list->capacity) {
    int new_capacity = list->capacity == 0 ? 16 : list->capacity * 2;
    Point *new_items = realloc(list->items, new_capacity * sizeof(Point));
    if (!new_items)
      return -1;
    list->items = new_items;
    list->capacity = new_capacity;
  }
  list->items[list->count++] = point;
  return 0;
}

static void point_list_free(PointList *list) {
  free(list->items);
  list->items = NULL;
  list->count = 0;
  list->capacity = 0;
}

static int parse_field_value(const char *start, size_t len, double *value) {
  char buf[64];
  if (len >= sizeof(buf))
    return -1;
  memcpy(buf, start, len);
  buf[len] = '\0';
  *value = strtod(buf, NULL);
  return 0;
}

static int parse_points(const char *csv_data, PointList *points) {
  int field_idx = 0;
  const char *field_start = csv_data;
  int in_quotes = 0;
  double values[6] = {0};

  const char *s = csv_data;

  while (*s) {
    char ch = *s;

    if (ch == '"') {
      if (in_quotes && s[1] == '"') {
        s += 2;
        continue;
      }
      in_quotes = !in_quotes;
      s++;
    } else if (ch == ',' && !in_quotes) {
      if (field_idx == 1 || field_idx == 3 || field_idx == 5) {
        if (parse_field_value(field_start, s - field_start,
                              &values[field_idx]) != 0)
          return -1;
      }
      field_idx++;
      field_start = s + 1;
      s++;
    } else if (ch == '\n' && !in_quotes) {
      if (field_idx < 6) {
        if (field_idx == 1 || field_idx == 3 || field_idx == 5) {
          if (parse_field_value(field_start, s - field_start,
                                &values[field_idx]) != 0)
            return -1;
        }
        field_idx++;
      }

      if (field_idx >= 6) {
        Point point = {values[1], values[5], values[3]};
        if (point_list_add(points, point) != 0)
          return -1;
      }

      field_idx = 0;
      field_start = s + 1;
      s++;
    } else {
      s++;
    }
  }

  if (field_start < s && field_idx > 0) {
    if (field_idx < 6) {
      if (field_idx == 1 || field_idx == 3 || field_idx == 5) {
        if (parse_field_value(field_start, s - field_start,
                              &values[field_idx]) != 0)
          return -1;
      }
      field_idx++;
    }

    if (field_idx >= 6) {
      Point point = {values[1], values[5], values[3]};
      if (point_list_add(points, point) != 0)
        return -1;
    }
  }

  return points->count;
}

void CsvParse_run(Benchmark *self, int iteration_id) {
  (void)iteration_id;
  CsvParseData *data = (CsvParseData *)self->data;
  if (!data->csv_data)
    return;

  PointList points;
  point_list_init(&points);

  int point_count = parse_points(data->csv_data, &points);

  if (point_count > 0) {
    double x_sum = 0, y_sum = 0, z_sum = 0;
    for (int i = 0; i < points.count; i++) {
      x_sum += points.items[i].x;
      y_sum += points.items[i].y;
      z_sum += points.items[i].z;
    }

    double len = points.count;
    data->result_val += Helper_checksum_f64(x_sum / len) +
                        Helper_checksum_f64(y_sum / len) +
                        Helper_checksum_f64(z_sum / len);
  }

  point_list_free(&points);
}

uint32_t CsvParse_checksum(Benchmark *self) {
  CsvParseData *data = (CsvParseData *)self->data;
  return data->result_val + Helper_checksum_string(data->csv_data);
}

void CsvParse_cleanup(Benchmark *self) {
  CsvParseData *data = (CsvParseData *)self->data;
  if (data->csv_data) {
    free(data->csv_data);
    data->csv_data = NULL;
  }
}

Benchmark *CsvParse_create(void) {
  Benchmark *bench = Benchmark_create("CSV::Parse");
  if (!bench)
    return NULL;

  CsvParseData *data = calloc(1, sizeof(CsvParseData));
  if (!data) {
    free(bench);
    return NULL;
  }

  bench->data = data;
  bench->prepare = CsvParse_prepare;
  bench->run = CsvParse_run;
  bench->checksum = CsvParse_checksum;
  bench->cleanup = CsvParse_cleanup;

  return bench;
}


typedef struct {
  char **s1;
  char **s2;
  size_t pair_count;
} StringPairs;

StringPairs *generate_pair_strings(int64_t n, int64_t m) {
  StringPairs *pairs = malloc(sizeof(StringPairs));
  pairs->pair_count = n;
  pairs->s1 = malloc(n * sizeof(char *));
  pairs->s2 = malloc(n * sizeof(char *));

  for (int64_t i = 0; i < n; i++) {
    int len1 = Helper_next_int(m) + 4;
    int len2 = Helper_next_int(m) + 4;

    pairs->s1[i] = malloc(len1 + 1);
    pairs->s2[i] = malloc(len2 + 1);

    for (int j = 0; j < len1; j++) {
      pairs->s1[i][j] = 'a' + Helper_next_int(10);
    }
    pairs->s1[i][len1] = '\0';

    for (int j = 0; j < len2; j++) {
      pairs->s2[i][j] = 'a' + Helper_next_int(10);
    }
    pairs->s2[i][len2] = '\0';
  }

  return pairs;
}

void free_string_pairs(StringPairs *pairs) {
  if (!pairs)
    return;

  for (size_t i = 0; i < pairs->pair_count; i++) {
    if (pairs->s1[i])
      free(pairs->s1[i]);
    if (pairs->s2[i])
      free(pairs->s2[i]);
  }
  free(pairs->s1);
  free(pairs->s2);
  free(pairs);
}

typedef struct {
  StringPairs *pairs;
  uint32_t result;
  int64_t count;
  int64_t size;
} JaroData;

void Jaro_prepare(Benchmark *self) {
  JaroData *data = (JaroData *)self->data;

  data->count = Helper_config_i64(self->name, "count");
  data->size = Helper_config_i64(self->name, "size");

  data->pairs = generate_pair_strings(data->count, data->size);
  data->result = 0;
}

double Jaro_calc(const char *s1, const char *s2) {
  size_t len1 = strlen(s1);
  size_t len2 = strlen(s2);

  if (len1 == 0 || len2 == 0)
    return 0.0;

  int64_t match_dist = (len1 > len2 ? len1 : len2) / 2 - 1;
  if (match_dist < 0)
    match_dist = 0;

  bool *s1_matches = calloc(len1, sizeof(bool));
  bool *s2_matches = calloc(len2, sizeof(bool));

  int matches = 0;
  for (size_t i = 0; i < len1; i++) {
    size_t start = i > (size_t)match_dist ? i - match_dist : 0;
    size_t end = (len2 - 1 < i + match_dist) ? len2 - 1 : i + match_dist;

    for (size_t j = start; j <= end; j++) {
      if (!s2_matches[j] && s1[i] == s2[j]) {
        s1_matches[i] = true;
        s2_matches[j] = true;
        matches++;
        break;
      }
    }
  }

  double result = 0.0;

  if (matches > 0) {
    int transpositions = 0;
    size_t k = 0;

    for (size_t i = 0; i < len1; i++) {
      if (s1_matches[i]) {
        while (k < len2 && !s2_matches[k]) {
          k++;
        }
        if (k < len2) {
          if (s1[i] != s2[k]) {
            transpositions++;
          }
          k++;
        }
      }
    }
    transpositions /= 2;

    double m = (double)matches;
    result = (m / len1 + m / len2 + (m - transpositions) / m) / 3.0;
  }

  free(s1_matches);
  free(s2_matches);

  return result;
}

void Jaro_run(Benchmark *self, int iteration_id) {
  (void)iteration_id;
  JaroData *data = (JaroData *)self->data;
  StringPairs *pairs = data->pairs;

  for (size_t i = 0; i < pairs->pair_count; i++) {
    double jaro = Jaro_calc(pairs->s1[i], pairs->s2[i]);
    data->result += (uint32_t)(jaro * 1000);
  }
}

uint32_t Jaro_checksum(Benchmark *self) {
  JaroData *data = (JaroData *)self->data;
  return data->result;
}

void Jaro_cleanup(Benchmark *self) {
  JaroData *data = (JaroData *)self->data;

  if (data->pairs) {
    free_string_pairs(data->pairs);
    data->pairs = NULL;
  }
}

Benchmark *Jaro_create(void) {
  Benchmark *bench = Benchmark_create("Distance::Jaro");

  JaroData *data = malloc(sizeof(JaroData));
  memset(data, 0, sizeof(JaroData));

  bench->data = data;
  bench->prepare = Jaro_prepare;
  bench->run = Jaro_run;
  bench->checksum = Jaro_checksum;
  bench->cleanup = Jaro_cleanup;

  return bench;
}

typedef struct {
  uint32_t gram;
  int count;
  UT_hash_handle hh;
} GramHash;

typedef struct {
  StringPairs *pairs;
  uint32_t result;
  int64_t count;
  int64_t size;
} NGramData;

void NGram_prepare(Benchmark *self) {
  NGramData *data = (NGramData *)self->data;

  data->count = Helper_config_i64(self->name, "count");
  data->size = Helper_config_i64(self->name, "size");

  data->pairs = generate_pair_strings(data->count, data->size);
  data->result = 0;
}

double NGram_calc(const char *s1, const char *s2) {
  size_t len1 = strlen(s1);
  size_t len2 = strlen(s2);

  if (len1 < 4 || len2 < 4)
    return 0.0;

  GramHash *grams1 = NULL;

  for (size_t i = 0; i <= len1 - 4; i++) {
    uint32_t gram = ((uint32_t)(uint8_t)s1[i] << 24) |
                    ((uint32_t)(uint8_t)s1[i + 1] << 16) |
                    ((uint32_t)(uint8_t)s1[i + 2] << 8) |
                    (uint32_t)(uint8_t)s1[i + 3];

    GramHash *g;
    HASH_FIND_INT(grams1, &gram, g);
    if (g) {
      g->count++;
    } else {
      g = (GramHash *)malloc(sizeof(GramHash));
      g->gram = gram;
      g->count = 1;
      HASH_ADD_INT(grams1, gram, g);
    }
  }

  GramHash *grams2 = NULL;
  int intersection = 0;

  for (size_t i = 0; i <= len2 - 4; i++) {
    uint32_t gram = ((uint32_t)(uint8_t)s2[i] << 24) |
                    ((uint32_t)(uint8_t)s2[i + 1] << 16) |
                    ((uint32_t)(uint8_t)s2[i + 2] << 8) |
                    (uint32_t)(uint8_t)s2[i + 3];

    GramHash *g2;
    HASH_FIND_INT(grams2, &gram, g2);
    if (g2) {
      g2->count++;
    } else {
      g2 = (GramHash *)malloc(sizeof(GramHash));
      g2->gram = gram;
      g2->count = 1;
      HASH_ADD_INT(grams2, gram, g2);
    }

    GramHash *g1;
    HASH_FIND_INT(grams1, &gram, g1);
    if (g1 && g2->count <= g1->count) {
      intersection++;
    }
  }

  int total = HASH_COUNT(grams1) + HASH_COUNT(grams2);

  GramHash *current, *tmp;
  HASH_ITER(hh, grams1, current, tmp) {
    HASH_DEL(grams1, current);
    free(current);
  }
  HASH_ITER(hh, grams2, current, tmp) {
    HASH_DEL(grams2, current);
    free(current);
  }

  return total > 0 ? (double)intersection / total : 0.0;
}

void NGram_run(Benchmark *self, int iteration_id) {
  (void)iteration_id;
  NGramData *data = (NGramData *)self->data;
  StringPairs *pairs = data->pairs;

  for (size_t i = 0; i < pairs->pair_count; i++) {
    double sim = NGram_calc(pairs->s1[i], pairs->s2[i]);
    data->result += (uint32_t)(sim * 1000);
  }
}

uint32_t NGram_checksum(Benchmark *self) {
  NGramData *data = (NGramData *)self->data;
  return data->result;
}

void NGram_cleanup(Benchmark *self) {
  NGramData *data = (NGramData *)self->data;

  if (data->pairs) {
    free_string_pairs(data->pairs);
    data->pairs = NULL;
  }
}

Benchmark *NGram_create(void) {
  Benchmark *bench = Benchmark_create("Distance::NGram");

  NGramData *data = malloc(sizeof(NGramData));
  memset(data, 0, sizeof(NGramData));

  bench->data = data;
  bench->prepare = NGram_prepare;
  bench->run = NGram_run;
  bench->checksum = NGram_checksum;
  bench->cleanup = NGram_cleanup;

  return bench;
}


typedef struct {
  int64_t n;
  uint32_t result_val;
} FannkuchreduxData;

static void fannkuchredux_swap(int *a, int *b) {
  int temp = *a;
  *a = *b;
  *b = temp;
}

static void fannkuchredux_calculate(int n, int *checksum, int *max_flips) {

  int perm1[32];
  int perm[32];
  int count[32];

  if (n > 32)
    n = 32;

  for (int i = 0; i < n; i++)
    perm1[i] = i;

  *max_flips = 0;
  *checksum = 0;
  int permCount = 0;
  int r = n;

  while (1) {
    while (r > 1) {
      count[r - 1] = r;
      r--;
    }

    memcpy(perm, perm1, n * sizeof(int));
    int flipsCount = 0;

    int k = perm[0];
    while (k != 0) {
      int k2 = (k + 1) >> 1;
      for (int i = 0; i < k2; i++) {
        int j = k - i;
        fannkuchredux_swap(&perm[i], &perm[j]);
      }
      flipsCount++;
      k = perm[0];
    }

    if (flipsCount > *max_flips)
      *max_flips = flipsCount;
    *checksum += (permCount % 2 == 0) ? flipsCount : -flipsCount;

    while (1) {
      if (r == n) {
        return;
      }

      int perm0 = perm1[0];
      for (int i = 0; i < r; i++) {
        perm1[i] = perm1[i + 1];
      }
      perm1[r] = perm0;

      count[r]--;
      if (count[r] > 0)
        break;
      r++;
    }
    permCount++;
  }
}

void Fannkuchredux_prepare(Benchmark *self) {
  FannkuchreduxData *data = (FannkuchreduxData *)self->data;
  data->n = Helper_config_i64(self->name, "n");
  if (data->n == 0) {
    data->n = 12;
  }
  data->result_val = 0;
}

void Fannkuchredux_run(Benchmark *self, int iteration_id) {
  (void)iteration_id;
  FannkuchreduxData *data = (FannkuchreduxData *)self->data;

  int checksum, max_flips;
  fannkuchredux_calculate((int)data->n, &checksum, &max_flips);

  data->result_val =
      (data->result_val + (uint32_t)(checksum * 100 + max_flips)) & 0xFFFFFFFFu;
}

uint32_t Fannkuchredux_checksum(Benchmark *self) {
  FannkuchreduxData *data = (FannkuchreduxData *)self->data;
  return data->result_val;
}

void Fannkuchredux_cleanup(Benchmark *self) { (void)self; }

Benchmark *Fannkuchredux_create(void) {
  Benchmark *bench = Benchmark_create("CLBG::Fannkuchredux");

  FannkuchreduxData *data = malloc(sizeof(FannkuchreduxData));
  data->n = 0;
  data->result_val = 0;

  bench->data = data;
  bench->prepare = Fannkuchredux_prepare;
  bench->run = Fannkuchredux_run;
  bench->checksum = Fannkuchredux_checksum;
  bench->cleanup = Fannkuchredux_cleanup;

  return bench;
}


typedef struct Cell Cell;

struct Cell {
  bool alive;
  bool next_state;
  Cell **neighbors;
  int neighbor_count;
};

typedef struct {
  int width;
  int height;
  Cell ***cells;
} GameOfLifeGrid;

typedef struct {
  uint32_t result_val;
  int64_t width_val;
  int64_t height_val;
  GameOfLifeGrid grid;
} GameOfLifeData;

static uint32_t fnv1a_hash(uint32_t hash, uint32_t value) {
  const uint32_t FNV_OFFSET_BASIS = 2166136261UL;
  const uint32_t FNV_PRIME = 16777619UL;

  if (hash == 0)
    hash = FNV_OFFSET_BASIS;
  hash ^= value;
  hash *= FNV_PRIME;
  return hash;
}

static Cell *cell_create(void) {
  Cell *cell = malloc(sizeof(Cell));
  cell->alive = false;
  cell->next_state = false;
  cell->neighbors = malloc(8 * sizeof(Cell *));
  cell->neighbor_count = 0;
  return cell;
}

static void cell_destroy(Cell *cell) {
  free(cell->neighbors);
  free(cell);
}

static void cell_add_neighbor(Cell *cell, Cell *neighbor) {
  cell->neighbors[cell->neighbor_count++] = neighbor;
}

static void cell_compute_next_state(Cell *cell) {
  int alive_neighbors = 0;
  for (int i = 0; i < cell->neighbor_count; i++) {
    if (cell->neighbors[i]->alive) {
      alive_neighbors++;
    }
  }

  if (cell->alive) {
    cell->next_state = (alive_neighbors == 2 || alive_neighbors == 3);
  } else {
    cell->next_state = (alive_neighbors == 3);
  }
}

static void cell_update(Cell *cell) { cell->alive = cell->next_state; }

static void game_of_life_grid_init(GameOfLifeGrid *grid, int width,
                                   int height) {
  grid->width = width;
  grid->height = height;

  grid->cells = malloc(height * sizeof(Cell **));
  for (int y = 0; y < height; y++) {
    grid->cells[y] = malloc(width * sizeof(Cell *));
    for (int x = 0; x < width; x++) {
      grid->cells[y][x] = cell_create();
    }
  }

  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      Cell *cell = grid->cells[y][x];

      for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
          if (dx == 0 && dy == 0)
            continue;

          int ny = (y + dy + height) % height;
          int nx = (x + dx + width) % width;

          cell_add_neighbor(cell, grid->cells[ny][nx]);
        }
      }
    }
  }
}

static void game_of_life_grid_free(GameOfLifeGrid *grid) {
  for (int y = 0; y < grid->height; y++) {
    for (int x = 0; x < grid->width; x++) {
      cell_destroy(grid->cells[y][x]);
    }
    free(grid->cells[y]);
  }
  free(grid->cells);
}

static void game_of_life_next_generation(GameOfLifeGrid *grid) {

  for (int y = 0; y < grid->height; y++) {
    for (int x = 0; x < grid->width; x++) {
      cell_compute_next_state(grid->cells[y][x]);
    }
  }

  for (int y = 0; y < grid->height; y++) {
    for (int x = 0; x < grid->width; x++) {
      cell_update(grid->cells[y][x]);
    }
  }
}

static int game_of_life_count_alive(GameOfLifeGrid *grid) {
  int count = 0;
  for (int y = 0; y < grid->height; y++) {
    for (int x = 0; x < grid->width; x++) {
      if (grid->cells[y][x]->alive)
        count++;
    }
  }
  return count;
}

static uint32_t game_of_life_grid_hash(GameOfLifeGrid *grid) {
  uint32_t hash = 0;

  for (int y = 0; y < grid->height; y++) {
    for (int x = 0; x < grid->width; x++) {
      uint32_t alive = (grid->cells[y][x]->alive) ? 1 : 0;
      hash = fnv1a_hash(hash, alive);
    }
  }

  return hash;
}

void GameOfLife_prepare(Benchmark *self) {
  GameOfLifeData *data = (GameOfLifeData *)self->data;

  data->width_val = Helper_config_i64(self->name, "w");
  data->height_val = Helper_config_i64(self->name, "h");

  if (data->width_val <= 0)
    data->width_val = 256;
  if (data->height_val <= 0)
    data->height_val = 256;

  game_of_life_grid_init(&data->grid, (int)data->width_val,
                         (int)data->height_val);

  for (int y = 0; y < data->grid.height; y++) {
    for (int x = 0; x < data->grid.width; x++) {
      if (Helper_next_float(1.0) < 0.1) {
        data->grid.cells[y][x]->alive = true;
      }
    }
  }

  data->result_val = 0;
}

void GameOfLife_run(Benchmark *self, int iteration_id) {
  (void)iteration_id;
  GameOfLifeData *data = (GameOfLifeData *)self->data;
  game_of_life_next_generation(&data->grid);
}

uint32_t GameOfLife_checksum(Benchmark *self) {
  GameOfLifeData *data = (GameOfLifeData *)self->data;
  int alive = game_of_life_count_alive(&data->grid);
  return game_of_life_grid_hash(&data->grid) + (uint32_t)alive;
}

void GameOfLife_cleanup(Benchmark *self) {
  GameOfLifeData *data = (GameOfLifeData *)self->data;
  game_of_life_grid_free(&data->grid);
}

Benchmark *GameOfLife_create(void) {
  Benchmark *bench = Benchmark_create("Etc::GameOfLife");

  GameOfLifeData *data = malloc(sizeof(GameOfLifeData));
  memset(data, 0, sizeof(GameOfLifeData));

  bench->data = data;
  bench->prepare = GameOfLife_prepare;
  bench->run = GameOfLife_run;
  bench->checksum = GameOfLife_checksum;
  bench->cleanup = GameOfLife_cleanup;

  return bench;
}

typedef struct {
  int vertex;
  int distance;
} Pair;

typedef struct {
  int vertices;
  int jumps;
  int jump_len;
  int **adj;
  int *adj_count;
  int *adj_capacity;
} GraphPathGraph;

typedef struct {
  GraphPathGraph *graph;
  uint32_t result_val;
} GraphPathBaseData;

static GraphPathGraph *graph_path_graph_new(int vertices, int jumps,
                                            int jump_len) {
  GraphPathGraph *graph = malloc(sizeof(GraphPathGraph));
  graph->vertices = vertices;
  graph->jumps = jumps;
  graph->jump_len = jump_len;

  graph->adj = malloc(vertices * sizeof(int *));
  graph->adj_count = malloc(vertices * sizeof(int));
  graph->adj_capacity = malloc(vertices * sizeof(int));

  for (int i = 0; i < vertices; i++) {
    graph->adj_capacity[i] = 4;
    graph->adj[i] = malloc(graph->adj_capacity[i] * sizeof(int));
    graph->adj_count[i] = 0;
  }

  return graph;
}

static void graph_path_graph_free(GraphPathGraph *graph) {
  if (!graph)
    return;

  if (graph->adj) {
    for (int i = 0; i < graph->vertices; i++) {
      if (graph->adj[i])
        free(graph->adj[i]);
    }
    free(graph->adj);
  }
  if (graph->adj_count)
    free(graph->adj_count);
  if (graph->adj_capacity)
    free(graph->adj_capacity);
  free(graph);
}

static void graph_path_graph_add_edge(GraphPathGraph *graph, int u, int v) {
  if (graph->adj_count[u] >= graph->adj_capacity[u]) {
    graph->adj_capacity[u] *= 2;
    graph->adj[u] =
        realloc(graph->adj[u], graph->adj_capacity[u] * sizeof(int));
  }
  graph->adj[u][graph->adj_count[u]++] = v;

  if (graph->adj_count[v] >= graph->adj_capacity[v]) {
    graph->adj_capacity[v] *= 2;
    graph->adj[v] =
        realloc(graph->adj[v], graph->adj_capacity[v] * sizeof(int));
  }
  graph->adj[v][graph->adj_count[v]++] = u;
}

static void graph_path_graph_generate_random(GraphPathGraph *graph) {
  for (int i = 1; i < graph->vertices; i++) {
    graph_path_graph_add_edge(graph, i, i - 1);
  }

  for (int v = 0; v < graph->vertices; v++) {
    int num_jumps = Helper_next_int(graph->jumps);
    for (int j = 0; j < num_jumps; j++) {
      int offset = Helper_next_int(graph->jump_len) - graph->jump_len / 2;
      int u = v + offset;

      if (u >= 0 && u < graph->vertices && u != v) {
        graph_path_graph_add_edge(graph, v, u);
      }
    }
  }
}

static void graph_path_base_prepare(Benchmark *self, const char *bench_name,
                                    GraphPathBaseData *data) {
  (void)self;
  int vertices = (int)Helper_config_i64(bench_name, "vertices");
  int jumps = (int)Helper_config_i64(bench_name, "jumps");
  int jump_len = (int)Helper_config_i64(bench_name, "jump_len");

  data->graph = graph_path_graph_new(vertices, jumps, jump_len);
  graph_path_graph_generate_random(data->graph);
  data->result_val = 0;
}

static void graph_path_base_cleanup(GraphPathBaseData *data) {
  if (data->graph) {
    graph_path_graph_free(data->graph);
    data->graph = NULL;
  }
}

typedef struct {
  GraphPathBaseData base;
} GraphPathBFSData;

static int graph_path_bfs_search(GraphPathGraph *graph, int start, int target) {
  if (start == target)
    return 0;

  uint8_t *visited = calloc(graph->vertices, sizeof(uint8_t));
  Pair *queue = malloc(graph->vertices * sizeof(Pair));
  int front = 0, rear = 0;

  visited[start] = 1;
  queue[rear++] = (Pair){start, 0};

  while (front < rear) {
    Pair current = queue[front++];
    int v = current.vertex;
    int dist = current.distance;

    for (int i = 0; i < graph->adj_count[v]; i++) {
      int neighbor = graph->adj[v][i];
      if (neighbor == target) {
        free(visited);
        free(queue);
        return dist + 1;
      }

      if (!visited[neighbor]) {
        visited[neighbor] = 1;
        queue[rear++] = (Pair){neighbor, dist + 1};
      }
    }
  }

  free(visited);
  free(queue);
  return -1;
}

void GraphPathBFS_prepare(Benchmark *self) {
  GraphPathBFSData *data = (GraphPathBFSData *)self->data;
  graph_path_base_prepare(self, "Graph::BFS", &data->base);
}

void GraphPathBFS_run(Benchmark *self, int iteration_id) {
  (void)iteration_id;
  GraphPathBFSData *data = (GraphPathBFSData *)self->data;
  int length = graph_path_bfs_search(data->base.graph, 0,
                                     data->base.graph->vertices - 1);
  data->base.result_val += length;
}

uint32_t GraphPathBFS_checksum(Benchmark *self) {
  GraphPathBFSData *data = (GraphPathBFSData *)self->data;
  return data->base.result_val;
}

void GraphPathBFS_cleanup(Benchmark *self) {
  GraphPathBFSData *data = (GraphPathBFSData *)self->data;
  graph_path_base_cleanup(&data->base);
}

Benchmark *GraphPathBFS_create(void) {
  Benchmark *bench = Benchmark_create("Graph::BFS");
  GraphPathBFSData *data = malloc(sizeof(GraphPathBFSData));
  memset(data, 0, sizeof(GraphPathBFSData));
  bench->data = data;
  bench->prepare = GraphPathBFS_prepare;
  bench->run = GraphPathBFS_run;
  bench->checksum = GraphPathBFS_checksum;
  bench->cleanup = GraphPathBFS_cleanup;
  return bench;
}

typedef struct {
  GraphPathBaseData base;
} GraphPathDFSData;

static int graph_path_dfs_search(GraphPathGraph *graph, int start, int target) {
  if (start == target)
    return 0;

  uint8_t *visited = calloc(graph->vertices, sizeof(uint8_t));
  Pair *stack = malloc(graph->vertices * sizeof(Pair));
  int stack_top = -1;
  int best_path = INT_MAX;

  stack[++stack_top] = (Pair){start, 0};

  while (stack_top >= 0) {
    Pair current = stack[stack_top--];
    int v = current.vertex;
    int dist = current.distance;

    if (visited[v] || dist >= best_path)
      continue;
    visited[v] = 1;

    for (int i = 0; i < graph->adj_count[v]; i++) {
      int neighbor = graph->adj[v][i];
      if (neighbor == target) {
        if (dist + 1 < best_path)
          best_path = dist + 1;
      } else if (!visited[neighbor]) {
        stack[++stack_top] = (Pair){neighbor, dist + 1};
      }
    }
  }

  free(visited);
  free(stack);
  return best_path == INT_MAX ? -1 : best_path;
}

void GraphPathDFS_prepare(Benchmark *self) {
  GraphPathDFSData *data = (GraphPathDFSData *)self->data;
  graph_path_base_prepare(self, "Graph::DFS", &data->base);
}

void GraphPathDFS_run(Benchmark *self, int iteration_id) {
  (void)iteration_id;
  GraphPathDFSData *data = (GraphPathDFSData *)self->data;
  int length = graph_path_dfs_search(data->base.graph, 0,
                                     data->base.graph->vertices - 1);
  data->base.result_val += length;
}

uint32_t GraphPathDFS_checksum(Benchmark *self) {
  GraphPathDFSData *data = (GraphPathDFSData *)self->data;
  return data->base.result_val;
}

void GraphPathDFS_cleanup(Benchmark *self) {
  GraphPathDFSData *data = (GraphPathDFSData *)self->data;
  graph_path_base_cleanup(&data->base);
}

Benchmark *GraphPathDFS_create(void) {
  Benchmark *bench = Benchmark_create("Graph::DFS");
  GraphPathDFSData *data = malloc(sizeof(GraphPathDFSData));
  memset(data, 0, sizeof(GraphPathDFSData));
  bench->data = data;
  bench->prepare = GraphPathDFS_prepare;
  bench->run = GraphPathDFS_run;
  bench->checksum = GraphPathDFS_checksum;
  bench->cleanup = GraphPathDFS_cleanup;
  return bench;
}

typedef struct {
  GraphPathBaseData base;
} GraphPathAStarData;

typedef struct {
  int vertex;
  int priority;
} PriorityQueueItem;

typedef struct {
  PriorityQueueItem *items;
  int size;
  int capacity;
} PriorityQueue;

static void priority_queue_push(PriorityQueue *pq, int vertex, int priority) {
  if (pq->size >= pq->capacity) {
    pq->capacity = pq->capacity == 0 ? 16 : pq->capacity * 2;
    pq->items = realloc(pq->items, pq->capacity * sizeof(PriorityQueueItem));
  }

  int i = pq->size++;
  while (i > 0) {
    int parent = (i - 1) / 2;
    if (pq->items[parent].priority <= priority)
      break;
    pq->items[i] = pq->items[parent];
    i = parent;
  }
  pq->items[i] = (PriorityQueueItem){vertex, priority};
}

static PriorityQueueItem priority_queue_pop(PriorityQueue *pq) {
  PriorityQueueItem min = pq->items[0];
  pq->size--;
  if (pq->size > 0) {
    PriorityQueueItem last = pq->items[pq->size];
    int i = 0;
    while (true) {
      int left = 2 * i + 1;
      int right = 2 * i + 2;
      int smallest = i;

      if (left < pq->size &&
          pq->items[left].priority < pq->items[smallest].priority)
        smallest = left;
      if (right < pq->size &&
          pq->items[right].priority < pq->items[smallest].priority)
        smallest = right;

      if (smallest == i)
        break;

      pq->items[i] = pq->items[smallest];
      i = smallest;
    }
    pq->items[i] = last;
  }
  return min;
}

static int heuristic(int v, int target) { return target - v; }

static int graph_path_astar_search(GraphPathGraph *graph, int start,
                                   int target) {
  if (start == target)
    return 0;

  int n = graph->vertices;

  int *g_score = malloc(n * sizeof(int));
  int *best_f = malloc(n * sizeof(int));

  for (int i = 0; i < n; i++) {
    g_score[i] = INT_MAX;
    best_f[i] = INT_MAX;
  }

  g_score[start] = 0;
  int f_start = heuristic(start, target);
  best_f[start] = f_start;

  PriorityQueue open_set = {0};
  priority_queue_push(&open_set, start, f_start);

  while (open_set.size > 0) {
    PriorityQueueItem current_item = priority_queue_pop(&open_set);
    int current = current_item.vertex;

    if (current == target) {
      int result = g_score[current];
      free(g_score);
      free(best_f);
      free(open_set.items);
      return result;
    }

    for (int i = 0; i < graph->adj_count[current]; i++) {
      int neighbor = graph->adj[current][i];
      int tentative_g = g_score[current] + 1;

      if (tentative_g < g_score[neighbor]) {
        g_score[neighbor] = tentative_g;
        int f_new = tentative_g + heuristic(neighbor, target);

        if (f_new < best_f[neighbor]) {
          best_f[neighbor] = f_new;
          priority_queue_push(&open_set, neighbor, f_new);
        }
      }
    }
  }

  free(g_score);
  free(best_f);
  free(open_set.items);
  return -1;
}

void GraphPathAStar_prepare(Benchmark *self) {
  GraphPathAStarData *data = (GraphPathAStarData *)self->data;
  graph_path_base_prepare(self, "Graph::AStar", &data->base);
}

void GraphPathAStar_run(Benchmark *self, int iteration_id) {
  (void)iteration_id;
  GraphPathAStarData *data = (GraphPathAStarData *)self->data;
  int length = graph_path_astar_search(data->base.graph, 0,
                                       data->base.graph->vertices - 1);
  data->base.result_val += length;
}

uint32_t GraphPathAStar_checksum(Benchmark *self) {
  GraphPathAStarData *data = (GraphPathAStarData *)self->data;
  return data->base.result_val;
}

void GraphPathAStar_cleanup(Benchmark *self) {
  GraphPathAStarData *data = (GraphPathAStarData *)self->data;
  graph_path_base_cleanup(&data->base);
}

Benchmark *GraphPathAStar_create(void) {
  Benchmark *bench = Benchmark_create("Graph::AStar");
  GraphPathAStarData *data = malloc(sizeof(GraphPathAStarData));
  memset(data, 0, sizeof(GraphPathAStarData));
  bench->data = data;
  bench->prepare = GraphPathAStar_prepare;
  bench->run = GraphPathAStar_run;
  bench->checksum = GraphPathAStar_checksum;
  bench->cleanup = GraphPathAStar_cleanup;
  return bench;
}

uint32_t Helper_last = INIT;
cJSON *global_config = NULL;
char **global_order = NULL;
size_t global_order_count = 0;

void Helper_reset(void) { Helper_last = INIT; }

uint32_t Helper_next_int(uint32_t max) {
  Helper_last = (Helper_last * IA + IC) % IM;
  return (uint32_t)((Helper_last * (int64_t)max) / IM);
}

uint32_t Helper_next_int_range(uint32_t from, uint32_t to) {
  return Helper_next_int(to - from + 1) + from;
}

double Helper_next_float(double max) {
  Helper_last = (Helper_last * IA + IC) % IM;
  return max * Helper_last / IM;
}

uint32_t Helper_checksum_string(const char *v) {
  uint32_t hash = 5381;
  while (*v) {
    unsigned char c = (unsigned char)(*v);
    hash = ((hash << 5) + hash) + c;
    v++;
  }
  return hash;
}

uint32_t Helper_checksum_bytes(const uint8_t *data, size_t length) {
  uint32_t hash = 5381;
  for (size_t i = 0; i < length; i++) {
    hash = ((hash << 5) + hash) + data[i];
  }
  return hash;
}

uint32_t Helper_checksum_f64(double v) {
  char buffer[32];
  snprintf(buffer, sizeof(buffer), "%.7f", v);
  return Helper_checksum_string(buffer);
}

void Helper_load_config(const char *json_data) {
  cJSON *parsed = cJSON_Parse(json_data);

  if (!parsed) {
    exit(1);
  }

  if (cJSON_IsArray(parsed)) {
    cJSON *config_map = cJSON_CreateObject();
    int array_size = cJSON_GetArraySize(parsed);
    global_order = malloc(sizeof(char *) * array_size);
    global_order_count = 0;

    cJSON *item;
    cJSON_ArrayForEach(item, parsed) {
      cJSON *name_item = cJSON_GetObjectItem(item, "name");
      if (name_item && cJSON_IsString(name_item)) {
        const char *name = name_item->valuestring;
        cJSON_AddItemToObject(config_map, name, cJSON_Duplicate(item, 1));
        global_order[global_order_count] = strdup(name);
        global_order_count++;
      }
    }

    global_config = config_map;
    cJSON_Delete(parsed);
  } else {
    global_config = parsed;
  }
}

void Helper_free_config(void) {
  if (global_config) {
    cJSON_Delete(global_config);
    global_config = NULL;
  }
  if (global_order) {
    for (size_t i = 0; i < global_order_count; i++) {
      free(global_order[i]);
    }
    free(global_order);
    global_order = NULL;
    global_order_count = 0;
  }
}

int64_t Helper_config_i64(const char *class_name, const char *field_name) {
  if (!global_config) {
    return 0;
  }

  cJSON *class_obj =
      cJSON_GetObjectItemCaseSensitive(global_config, class_name);
  if (!class_obj) {
    return 0;
  }

  cJSON *field = cJSON_GetObjectItemCaseSensitive(class_obj, field_name);
  if (!field) {
    return 0;
  }

  if (cJSON_IsNumber(field)) {
    return (int64_t)field->valuedouble;
  } else if (cJSON_IsString(field)) {
    return atoll(field->valuestring);
  } else {
    return 0;
  }
}

const char *Helper_config_s(const char *class_name, const char *field_name) {
  if (!global_config) {
    return "";
  }

  cJSON *class_obj =
      cJSON_GetObjectItemCaseSensitive(global_config, class_name);
  if (!class_obj) {
    return "";
  }

  cJSON *field = cJSON_GetObjectItemCaseSensitive(class_obj, field_name);
  if (!field || !cJSON_IsString(field)) {
    return "";
  }

  return field->valuestring;
}

typedef struct {
    double x, y, z;
    char name[64];
    int opts_val;
    bool opts_bool;
} Coord;

typedef struct {
    Coord *coords;
    int64_t n;
    char *result_str;
    uint32_t result_val;
} JsonGenerateData;

static const char *INFO_STR = "some info";

static double round_to_8_digits(double value) {
    return round(value * 1e8) / 1e8;
}

void JsonGenerate_prepare(Benchmark *self) {
    JsonGenerateData *data = (JsonGenerateData *)self->data;

    if (data->coords) {
        free(data->coords);
        data->coords = NULL;
    }
    if (data->result_str) {
        free(data->result_str);
        data->result_str = NULL;
    }

    data->coords = malloc(data->n * sizeof(Coord));

    for (int64_t i = 0; i < data->n; i++) {
        data->coords[i].x = round_to_8_digits(Helper_next_float(1.0));
        data->coords[i].y = round_to_8_digits(Helper_next_float(1.0));
        data->coords[i].z = round_to_8_digits(Helper_next_float(1.0));

        snprintf(data->coords[i].name, sizeof(data->coords[i].name), "%.7f %u",
                 Helper_next_float(1.0), Helper_next_int(10000));

        data->coords[i].opts_val = 1;
        data->coords[i].opts_bool = true;
    }

    data->result_val = 0;
}

void JsonGenerate_run(Benchmark *self, int iteration_id) {
    (void)iteration_id;
    JsonGenerateData *data = (JsonGenerateData *)self->data;

    if (data->result_str) {
        free(data->result_str);
        data->result_str = NULL;
    }

    cJSON *root = cJSON_CreateObject();
    if (!root) return;

    cJSON *coordinates = cJSON_CreateArray();
    cJSON_AddItemToObject(root, "coordinates", coordinates);

    for (int64_t i = 0; i < data->n; i++) {
        Coord *c = &data->coords[i];

        cJSON *coord = cJSON_CreateObject();

        cJSON_AddNumberToObject(coord, "x", c->x);
        cJSON_AddNumberToObject(coord, "y", c->y);
        cJSON_AddNumberToObject(coord, "z", c->z);
        cJSON_AddStringToObject(coord, "name", c->name);

        cJSON *opts = cJSON_CreateObject();
        cJSON *arr = cJSON_CreateArray();
        cJSON_AddItemToArray(arr, cJSON_CreateNumber(c->opts_val));
        cJSON_AddItemToArray(arr, cJSON_CreateBool(c->opts_bool));
        cJSON_AddItemToObject(opts, "1", arr);
        cJSON_AddItemToObject(coord, "opts", opts);

        cJSON_AddItemToArray(coordinates, coord);
    }

    cJSON_AddStringToObject(root, "info", INFO_STR);

    data->result_str = cJSON_PrintUnformatted(root);

    if (data->result_str && strlen(data->result_str) >= 15 &&
        strncmp(data->result_str, "{\"coordinates\":", 15) == 0) {
        data->result_val++;
    }

    cJSON_Delete(root);
}

uint32_t JsonGenerate_checksum(Benchmark *self) {
    JsonGenerateData *data = (JsonGenerateData *)self->data;
    return data->result_val;
}

void JsonGenerate_cleanup(Benchmark *self) {
    JsonGenerateData *data = (JsonGenerateData *)self->data;

    if (data->coords) {
        free(data->coords);
        data->coords = NULL;
    }
    if (data->result_str) {
        free(data->result_str);
        data->result_str = NULL;
    }
}

Benchmark *JsonGenerate_create(void) {
    Benchmark *bench = Benchmark_create("Json::Generate");

    JsonGenerateData *data = malloc(sizeof(JsonGenerateData));
    memset(data, 0, sizeof(JsonGenerateData));

    bench->data = data;
    data->n = Helper_config_i64(bench->name, "coords");
    bench->prepare = JsonGenerate_prepare;
    bench->run = JsonGenerate_run;
    bench->checksum = JsonGenerate_checksum;
    bench->cleanup = JsonGenerate_cleanup;

    return bench;
}

char *JsonGenerate_get_result(Benchmark *self) {
    JsonGenerateData *data = (JsonGenerateData *)self->data;
    return data->result_str ? strdup(data->result_str) : NULL;
}

typedef struct {
    char *json_text;
    uint32_t result_val;
    int64_t coords_count;
} JsonParseDomData;

static char *generate_json_for_parsing(int64_t coords_count) {
    Benchmark *json_gen_bench = JsonGenerate_create();
    JsonGenerateData *gen_data = (JsonGenerateData *)json_gen_bench->data;

    gen_data->n = coords_count;
    json_gen_bench->prepare(json_gen_bench);
    json_gen_bench->run(json_gen_bench, 0);

    char *json_text = strdup(gen_data->result_str);

    json_gen_bench->cleanup(json_gen_bench);
    free(json_gen_bench->data);
    free(json_gen_bench);

    return json_text;
}

void JsonParseDom_prepare(Benchmark *self) {
    JsonParseDomData *data = (JsonParseDomData *)self->data;

    data->coords_count = Helper_config_i64(self->name, "coords");
    if (data->coords_count <= 0) {
        data->coords_count = 1000;
    }

    data->json_text = generate_json_for_parsing(data->coords_count);
    data->result_val = 0;
}

void JsonParseDom_run(Benchmark *self, int iteration_id) {
    (void)iteration_id;
    JsonParseDomData *data = (JsonParseDomData *)self->data;

    cJSON *root = cJSON_Parse(data->json_text);
    if (!root) {
        return;
    }

    cJSON *coordinates = cJSON_GetObjectItem(root, "coordinates");
    if (!coordinates || !cJSON_IsArray(coordinates)) {
        cJSON_Delete(root);
        return;
    }

    double x_sum = 0.0, y_sum = 0.0, z_sum = 0.0;
    int64_t len = 0;

    cJSON *coord_item = NULL;
    cJSON_ArrayForEach(coord_item, coordinates) {
        cJSON *x_item = cJSON_GetObjectItem(coord_item, "x");
        cJSON *y_item = cJSON_GetObjectItem(coord_item, "y");
        cJSON *z_item = cJSON_GetObjectItem(coord_item, "z");

        if (x_item && y_item && z_item && 
            cJSON_IsNumber(x_item) && 
            cJSON_IsNumber(y_item) && 
            cJSON_IsNumber(z_item)) {

            x_sum += x_item->valuedouble;
            y_sum += y_item->valuedouble;
            z_sum += z_item->valuedouble;
            len++;
        }
    }

    cJSON_Delete(root);

    if (len > 0) {
        double x_avg = x_sum / len;
        double y_avg = y_sum / len;
        double z_avg = z_sum / len;

        uint32_t checksum = Helper_checksum_f64(x_avg) +
                           Helper_checksum_f64(y_avg) + Helper_checksum_f64(z_avg);
        data->result_val += checksum;
    }
}

uint32_t JsonParseDom_checksum(Benchmark *self) {
    JsonParseDomData *data = (JsonParseDomData *)self->data;
    return data->result_val;
}

void JsonParseDom_cleanup(Benchmark *self) {
    JsonParseDomData *data = (JsonParseDomData *)self->data;
    if (data->json_text) {
        free(data->json_text);
        data->json_text = NULL;
    }
}

Benchmark *JsonParseDom_create(void) {
    Benchmark *bench = Benchmark_create("Json::ParseDom");

    JsonParseDomData *data = malloc(sizeof(JsonParseDomData));
    memset(data, 0, sizeof(JsonParseDomData));

    bench->data = data;

    bench->prepare = JsonParseDom_prepare;
    bench->run = JsonParseDom_run;
    bench->checksum = JsonParseDom_checksum;
    bench->cleanup = JsonParseDom_cleanup;

    return bench;
}

typedef struct {
    char *json_text;
    uint32_t result_val;
    int64_t coords_count;
} JsonParseMappingData;

void JsonParseMapping_prepare(Benchmark *self) {
    JsonParseMappingData *data = (JsonParseMappingData *)self->data;

    data->coords_count = Helper_config_i64(self->name, "coords");
    if (data->coords_count <= 0) {
        data->coords_count = 1000;
    }

    data->json_text = generate_json_for_parsing(data->coords_count);
    data->result_val = 0;
}

void JsonParseMapping_run(Benchmark *self, int iteration_id) {
    (void)iteration_id;
    JsonParseMappingData *data = (JsonParseMappingData *)self->data;

    cJSON *root = cJSON_Parse(data->json_text);
    if (!root) {
        return;
    }

    cJSON *coordinates = cJSON_GetObjectItem(root, "coordinates");
    if (!coordinates || !cJSON_IsArray(coordinates)) {
        cJSON_Delete(root);
        return;
    }

    double x_sum = 0.0, y_sum = 0.0, z_sum = 0.0;
    int64_t len = 0;

    cJSON *coord_item = NULL;
    cJSON_ArrayForEach(coord_item, coordinates) {
        cJSON *x_item = cJSON_GetObjectItem(coord_item, "x");
        cJSON *y_item = cJSON_GetObjectItem(coord_item, "y");
        cJSON *z_item = cJSON_GetObjectItem(coord_item, "z");

        if (x_item && y_item && z_item) {
            x_sum += x_item->valuedouble;
            y_sum += y_item->valuedouble;
            z_sum += z_item->valuedouble;
            len++;
        }
    }

    cJSON_Delete(root);

    if (len > 0) {
        double x_avg = x_sum / len;
        double y_avg = y_sum / len;
        double z_avg = z_sum / len;

        uint32_t checksum = Helper_checksum_f64(x_avg) +
                           Helper_checksum_f64(y_avg) + Helper_checksum_f64(z_avg);
        data->result_val += checksum;
    }
}

uint32_t JsonParseMapping_checksum(Benchmark *self) {
    JsonParseMappingData *data = (JsonParseMappingData *)self->data;
    return data->result_val;
}

void JsonParseMapping_cleanup(Benchmark *self) {
    JsonParseMappingData *data = (JsonParseMappingData *)self->data;
    if (data->json_text) {
        free(data->json_text);
        data->json_text = NULL;
    }
}

Benchmark *JsonParseMapping_create(void) {
    Benchmark *bench = Benchmark_create("Json::ParseMapping");

    JsonParseMappingData *data = malloc(sizeof(JsonParseMappingData));
    memset(data, 0, sizeof(JsonParseMappingData));

    bench->data = data;

    bench->prepare = JsonParseMapping_prepare;
    bench->run = JsonParseMapping_run;
    bench->checksum = JsonParseMapping_checksum;
    bench->cleanup = JsonParseMapping_cleanup;

    return bench;
}


#define PATTERNS_COUNT 13
#define MAX_TOKENS     512

typedef struct {
  int      lines_count;
  char    *log;
  size_t   log_size;
  uint32_t checksum_val;
  RegexToken compiled[PATTERNS_COUNT][MAX_TOKENS];
  int16_t    token_counts[PATTERNS_COUNT];
  int        patterns_count;
} LogParserData;

static const char *PATTERNS[PATTERNS_COUNT] = {
    " [45][0-9]{2} ",
    "bot|crawler|scanner|spider|indexing|crawl|robot|Googlebot|Mozilla|curl",
    "etc/passwd|wp-admin|\\.\\./",
    "\\d+\\.\\d+\\.\\d+\\.35",
    "/api/[^ \" ]+",
    "POST [^ ]* HTTP",
    "/login|/signin",
    "GET|POST|PUT|DELETE",
    "[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}",
    "password=[^&\\s\"]+",
    "token=[^&\\s\"]+|api[_-]?key=[^&\\s\"]+",
    "session[_-]?id=[^&\\s\"]+",
    "\\[\\d+/\\w+/\\d+:1[3-7]:\\d+:\\d+ [+\\-]\\d+\\]",
};

static const char *IPS[255];
static const char *METHODS[] = {"GET", "POST", "PUT", "DELETE"};
static const char *PATHS[]   = {"/index.html", "/api/users",
                                "/admin",      "/images/logo.png",
                                "/etc/passwd", "/wp-admin/setup.php"};
static int         STATUSES[] = {200, 201, 301, 302, 400, 401, 403,
                                 404, 500, 502, 503};
static const char *AGENTS[]  = {"Mozilla/5.0", "Googlebot/2.1",
                                "curl/7.68.0", "scanner/2.0"};
static const char *USERS[]   = {"john", "jane", "alex",  "sarah",
                                "mike", "anna", "david", "elena"};
static const char *DOMAINS[] = {"example.com", "gmail.com",   "yahoo.com",
                                "hotmail.com", "company.org", "mail.ru"};

static void init_ips(void) {
  static int initialized = 0;
  if (initialized)
    return;

  for (int i = 0; i < 255; i++) {
    char *ip = malloc(16);
    if (!ip)
      return;
    snprintf(ip, 16, "192.168.1.%d", i + 1);
    IPS[i] = ip;
  }
  initialized = 1;
}

static void generate_log_line(char *buffer, size_t *pos, int i) {
  if (i % 3 == 0) {
    *pos += (size_t)sprintf(buffer + *pos,
                    "%s - - [%d/Oct/2023:%d:55:36 +0000] \"%s "
                    "/login?email=%s%d@%s&password=secret%d HTTP/1.1\" %d 2326 "
                    "\"http://%s\" \"%s\"\n",
                    IPS[i % 255], i % 31, i % 60, METHODS[i % 4], USERS[i % 8],
                    i % 100, DOMAINS[i % 6], i % 10000, STATUSES[i % 11],
                    DOMAINS[i % 6], AGENTS[i % 4]);
  } else if (i % 5 == 0) {
    char token[200] = "";
    for (int j = 0; j < (i % 3) + 1; j++) {
      strcat(token, "abcdef123456");
    }
    *pos += (size_t)sprintf(buffer + *pos,
                    "%s - - [%d/Oct/2023:%d:55:36 +0000] \"%s /api/data?token=%s "
                    "HTTP/1.1\" %d 2326 \"http://%s\" \"%s\"\n",
                    IPS[i % 255], i % 31, i % 60, METHODS[i % 4], token,
                    STATUSES[i % 11], DOMAINS[i % 6], AGENTS[i % 4]);
  } else if (i % 7 == 0) {
    *pos += (size_t)sprintf(buffer + *pos,
                    "%s - - [%d/Oct/2023:%d:55:36 +0000] \"%s "
                    "/user/profile?session_id=sess_%x HTTP/1.1\" %d 2326 "
                    "\"http://%s\" \"%s\"\n",
                    IPS[i % 255], i % 31, i % 60, METHODS[i % 4], i * 12345,
                    STATUSES[i % 11], DOMAINS[i % 6], AGENTS[i % 4]);
  } else {
    *pos += (size_t)sprintf(buffer + *pos,
                    "%s - - [%d/Oct/2023:%d:55:36 +0000] \"%s %s HTTP/1.1\" %d "
                    "2326 \"http://%s\" \"%s\"\n",
                    IPS[i % 255], i % 31, i % 60, METHODS[i % 4], PATHS[i % 6],
                    STATUSES[i % 11], DOMAINS[i % 6], AGENTS[i % 4]);
  }
}

void LogParser_prepare(Benchmark *self) {
  LogParserData *data = (LogParserData *)self->data;

  init_ips();

  if (data->log) {
    free(data->log);
    data->log = NULL;
  }

  data->lines_count = (int)Helper_config_i64(self->name, "lines_count");

  size_t estimated_size = (size_t)data->lines_count * 256 + 1;
  char *log_buf = malloc(estimated_size);
  if (!log_buf)
    return;

  size_t pos = 0;
  for (int i = 0; i < data->lines_count; i++) {
    generate_log_line(log_buf, &pos, i);
  }
  log_buf[pos] = '\0';

  data->log      = log_buf;
  data->log_size = pos;

  for (int i = 0; i < PATTERNS_COUNT; i++) {
    data->token_counts[i] = MAX_TOKENS;
    int e = regex_parse(PATTERNS[i],
                        data->compiled[i],
                        &data->token_counts[i],
                        0);
    if (e) {
      fprintf(stderr,
              "[LogParser] regex_parse failed for pattern %d (\"%s\"): %d\n",
              i, PATTERNS[i], e);
      data->token_counts[i] = 0;
    }
  }
  data->patterns_count = PATTERNS_COUNT;
}

void LogParser_run(Benchmark *self, int iteration_id) {
  (void)iteration_id;
  LogParserData *data = (LogParserData *)self->data;

  uint32_t total = 0;

  for (int i = 0; i < data->patterns_count; i++) {
    if (data->token_counts[i] <= 0)
      continue;

    int    matches = 0;
    size_t offset  = 0;

    while (offset < data->log_size) {
      int64_t end = regex_match(data->compiled[i], data->log, offset, 0, NULL, NULL);
      if (end < 0) {
        offset++;
      } else {
        matches++;
        if ((size_t)end > offset) {
          offset = (size_t)end;
        } else {
          offset++;
        }
      }
    }

    total += (uint32_t)matches;
  }

  data->checksum_val += total;
}

uint32_t LogParser_checksum(Benchmark *self) {
  LogParserData *data = (LogParserData *)self->data;
  return data->checksum_val;
}

void LogParser_cleanup(Benchmark *self) {
  LogParserData *data = (LogParserData *)self->data;

  if (data->log) {
    free(data->log);
    data->log = NULL;
  }

  data->log_size       = 0;
  data->checksum_val   = 0;
  data->patterns_count = 0;
}

Benchmark *LogParser_create(void) {
  Benchmark *bench = Benchmark_create("Etc::LogParser");

  LogParserData *data = calloc(1, sizeof(LogParserData));
  if (!data) {
    return bench;
  }

  for (int i = 0; i < PATTERNS_COUNT; i++) {
    data->token_counts[i] = 0;
  }
  data->patterns_count = 0;

  bench->data    = data;
  bench->prepare = LogParser_prepare;
  bench->run     = LogParser_run;
  bench->checksum= LogParser_checksum;
  bench->cleanup = LogParser_cleanup;

  return bench;
}


typedef struct {
  int64_t w;
  int64_t h;
  uint8_t *result_bin;
  size_t result_size;
  size_t result_capacity;
} MandelbrotData;

static void Mandelbrot_grow_result(MandelbrotData *self, size_t needed) {
  size_t new_capacity = self->result_capacity;
  while (self->result_size + needed >= new_capacity) {
    new_capacity = new_capacity ? new_capacity * 2 : 1024;
  }
  if (new_capacity > self->result_capacity) {
    self->result_bin = realloc(self->result_bin, new_capacity);
    if (!self->result_bin) {
      return;
    }
    self->result_capacity = new_capacity;
  }
}

static void Mandelbrot_append(MandelbrotData *self, const uint8_t *data,
                              size_t size) {
  Mandelbrot_grow_result(self, size);
  memcpy(self->result_bin + self->result_size, data, size);
  self->result_size += size;
}

void Mandelbrot_prepare(Benchmark *self) {
  MandelbrotData *data = (MandelbrotData *)self->data;

  data->w = Helper_config_i64(self->name, "w");
  data->h = Helper_config_i64(self->name, "h");

  if (data->w == 0)
    data->w = 200;
  if (data->h == 0)
    data->h = 200;

  data->result_size = 0;
}

void Mandelbrot_run(Benchmark *self, int iteration_id) {
  (void)iteration_id;
  MandelbrotData *data = (MandelbrotData *)self->data;

  int w = (int)data->w;
  int h = (int)data->h;

  char header[256];
  int header_len = snprintf(header, sizeof(header), "P4\n%d %d\n", w, h);
  Mandelbrot_append(data, (uint8_t *)header, header_len);

  const int ITER = 50;
  const double LIMIT = 2.0;

  int bit_num = 0;
  uint8_t byte_acc = 0;

  for (int y = 0; y < h; y++) {
    double ci = 2.0 * y / (double)h - 1.0;

    for (int x = 0; x < w; x++) {
      double cr = 2.0 * x / (double)w - 1.5;

      double zr = 0.0, zi = 0.0;
      double tr = 0.0, ti = 0.0;

      int i = 0;
      while (i < ITER && tr + ti <= LIMIT * LIMIT) {
        zi = 2.0 * zr * zi + ci;
        zr = tr - ti + cr;
        tr = zr * zr;
        ti = zi * zi;
        i++;
      }

      byte_acc <<= 1;
      if (tr + ti <= LIMIT * LIMIT) {
        byte_acc |= 0x01;
      }
      bit_num++;

      if (bit_num == 8) {
        Mandelbrot_append(data, &byte_acc, 1);
        byte_acc = 0;
        bit_num = 0;
      } else if (x == w - 1) {

        byte_acc <<= (8 - (w % 8));
        Mandelbrot_append(data, &byte_acc, 1);
        byte_acc = 0;
        bit_num = 0;
      }
    }
  }
}

uint32_t Mandelbrot_checksum(Benchmark *self) {
  MandelbrotData *data = (MandelbrotData *)self->data;

  if (!data->result_bin || data->result_size == 0) {
    return 0;
  }

  return Helper_checksum_bytes(data->result_bin, data->result_size);
}

void Mandelbrot_cleanup(Benchmark *self) {
  MandelbrotData *data = (MandelbrotData *)self->data;

  if (data->result_bin) {
    free(data->result_bin);
    data->result_bin = NULL;
  }

  data->result_size = 0;
  data->result_capacity = 0;
}

Benchmark *Mandelbrot_create(void) {
  Benchmark *bench = Benchmark_create("CLBG::Mandelbrot");

  MandelbrotData *data = calloc(1, sizeof(MandelbrotData));

  data->w = Helper_config_i64("CLBG::Mandelbrot", "w");
  data->h = Helper_config_i64("CLBG::Mandelbrot", "h");

  if (data->w == 0)
    data->w = 200;
  if (data->h == 0)
    data->h = 200;

  data->result_bin = NULL;
  data->result_size = 0;
  data->result_capacity = 0;

  bench->data = data;
  bench->prepare = Mandelbrot_prepare;
  bench->run = Mandelbrot_run;
  bench->checksum = Mandelbrot_checksum;
  bench->cleanup = Mandelbrot_cleanup;

  return bench;
}

static double **matgen(int n) {
  double tmp = 1.0 / n / n;
  double **a = malloc(n * sizeof(double *));

  for (int i = 0; i < n; i++) {
    a[i] = malloc(n * sizeof(double));
    for (int j = 0; j < n; j++) {
      a[i][j] = tmp * (i - j) * (i + j);
    }
  }
  return a;
}

static void free_matrix(double **a, int n) {
  if (!a)
    return;
  for (int i = 0; i < n; i++) {
    free(a[i]);
  }
  free(a);
}

static double **transpose(double **b, int n) {
  double **b_t = malloc(n * sizeof(double *));
  for (int j = 0; j < n; j++) {
    b_t[j] = malloc(n * sizeof(double));
    for (int i = 0; i < n; i++) {
      b_t[j][i] = b[i][j];
    }
  }
  return b_t;
}

typedef struct {
  int64_t n;
  uint32_t result_val;
  double **a;
  double **b;
} MatmulBaseData;

static uint32_t Matmul_checksum(Benchmark *self) {
  MatmulBaseData *data = (MatmulBaseData *)self->data;
  return data->result_val;
}

static void MatmulBase_prepare(Benchmark *self) {
  MatmulBaseData *data = (MatmulBaseData *)self->data;
  int n = (int)data->n;

  data->a = matgen(n);
  data->b = matgen(n);
  data->result_val = 0;
}

static void MatmulBase_cleanup(Benchmark *self) {
  MatmulBaseData *data = (MatmulBaseData *)self->data;
  int n = (int)data->n;

  if (data->a) {
    free_matrix(data->a, n);
    data->a = NULL;
  }
  if (data->b) {
    free_matrix(data->b, n);
    data->b = NULL;
  }
}

static double **matmul_sequential(double **a, double **b, int n) {
  double **b_t = transpose(b, n);
  double **c = malloc(n * sizeof(double *));

  for (int i = 0; i < n; i++) {
    c[i] = malloc(n * sizeof(double));
    double *ai = a[i];
    for (int j = 0; j < n; j++) {
      double s = 0.0;
      double *b_tj = b_t[j];

      for (int k = 0; k < n; k++) {
        s += ai[k] * b_tj[k];
      }
      c[i][j] = s;
    }
  }

  free_matrix(b_t, n);
  return c;
}

static void Matmul_run(Benchmark *self, int iteration_id) {
  (void)iteration_id;
  MatmulBaseData *data = (MatmulBaseData *)self->data;
  int n = (int)data->n;

  double **c = matmul_sequential(data->a, data->b, n);
  double center_value = c[n >> 1][n >> 1];
  free_matrix(c, n);

  uint32_t iter_checksum = Helper_checksum_f64(center_value);
  data->result_val += iter_checksum;
}

Benchmark *Matmul_create(void) {
  Benchmark *bench = Benchmark_create("Matmul::Single");
  MatmulBaseData *data = malloc(sizeof(MatmulBaseData));

  data->n = Helper_config_i64("Matmul::Single", "n");
  if (data->n == 0)
    data->n = 100;
  data->result_val = 0;
  data->a = NULL;
  data->b = NULL;

  bench->data = data;
  bench->prepare = MatmulBase_prepare;
  bench->run = Matmul_run;
  bench->checksum = Matmul_checksum;
  bench->cleanup = MatmulBase_cleanup;

  return bench;
}

typedef struct {
  MatmulBaseData base;
  int num_threads;
} MatmulParallelData;

typedef struct {
  double **a;
  double **b_t;
  double **c;
  int n;
  int start_row;
  int end_row;
} MatmulThreadData;

static void *thread_func(void *arg) {
  MatmulThreadData *data = (MatmulThreadData *)arg;

  for (int i = data->start_row; i < data->end_row; i++) {
    double *ai = data->a[i];
    double *ci = data->c[i];

    for (int j = 0; j < data->n; j++) {
      double sum = 0.0;
      double *b_tj = data->b_t[j];

      for (int k = 0; k < data->n; k++) {
        sum += ai[k] * b_tj[k];
      }
      ci[j] = sum;
    }
  }
  return NULL;
}

static double **matmul_parallel(double **a, double **b, int n,
                                int num_threads) {
  pthread_t threads[num_threads];
  MatmulThreadData thread_data[num_threads];

  double **b_t = transpose(b, n);
  double **c = malloc(n * sizeof(double *));
  for (int i = 0; i < n; i++) {
    c[i] = calloc(n, sizeof(double));
  }

  int rows_per_thread = (n + num_threads - 1) / num_threads;

  for (int t = 0; t < num_threads; t++) {
    thread_data[t].a = a;
    thread_data[t].b_t = b_t;
    thread_data[t].c = c;
    thread_data[t].n = n;
    thread_data[t].start_row = t * rows_per_thread;
    thread_data[t].end_row = thread_data[t].start_row + rows_per_thread;
    if (thread_data[t].end_row > n || t == num_threads - 1) {
      thread_data[t].end_row = n;
    }

    int rc = pthread_create(&threads[t], NULL, thread_func, &thread_data[t]);
    if (rc != 0) {
      thread_func(&thread_data[t]);
      threads[t] = 0;
    }
  }

  for (int t = 0; t < num_threads; t++) {
    if (threads[t] != 0) {
      pthread_join(threads[t], NULL);
    }
  }

  free_matrix(b_t, n);
  return c;
}

static void MatmulParallel_run(Benchmark *self, int iteration_id) {
  (void)iteration_id;
  MatmulParallelData *data = (MatmulParallelData *)self->data;
  int n = (int)data->base.n;

  double **c =
      matmul_parallel(data->base.a, data->base.b, n, data->num_threads);
  double center_value = c[n >> 1][n >> 1];
  free_matrix(c, n);

  uint32_t iter_checksum = Helper_checksum_f64(center_value);
  data->base.result_val += iter_checksum;
}

Benchmark *Matmul4T_create(void) {
  Benchmark *bench = Benchmark_create("Matmul::T4");
  MatmulParallelData *data = malloc(sizeof(MatmulParallelData));

  data->base.n = Helper_config_i64("Matmul::T4", "n");
  if (data->base.n == 0)
    data->base.n = 100;
  data->base.result_val = 0;
  data->base.a = NULL;
  data->base.b = NULL;
  data->num_threads = 4;

  bench->data = data;
  bench->prepare = MatmulBase_prepare;
  bench->run = MatmulParallel_run;
  bench->checksum = Matmul_checksum;
  bench->cleanup = MatmulBase_cleanup;

  return bench;
}

Benchmark *Matmul8T_create(void) {
  Benchmark *bench = Benchmark_create("Matmul::T8");
  MatmulParallelData *data = malloc(sizeof(MatmulParallelData));

  data->base.n = Helper_config_i64("Matmul::T8", "n");
  if (data->base.n == 0)
    data->base.n = 100;
  data->base.result_val = 0;
  data->base.a = NULL;
  data->base.b = NULL;
  data->num_threads = 8;

  bench->data = data;
  bench->prepare = MatmulBase_prepare;
  bench->run = MatmulParallel_run;
  bench->checksum = Matmul_checksum;
  bench->cleanup = MatmulBase_cleanup;

  return bench;
}

Benchmark *Matmul16T_create(void) {
  Benchmark *bench = Benchmark_create("Matmul::T16");
  MatmulParallelData *data = malloc(sizeof(MatmulParallelData));

  data->base.n = Helper_config_i64("Matmul::T16", "n");
  if (data->base.n == 0)
    data->base.n = 100;
  data->base.result_val = 0;
  data->base.a = NULL;
  data->base.b = NULL;
  data->num_threads = 16;

  bench->data = data;
  bench->prepare = MatmulBase_prepare;
  bench->run = MatmulParallel_run;
  bench->checksum = Matmul_checksum;
  bench->cleanup = MatmulBase_cleanup;

  return bench;
}


typedef enum {
  MAZE_CELL_WALL = 0,
  MAZE_CELL_SPACE,
  MAZE_CELL_START,
  MAZE_CELL_FINISH,
  MAZE_CELL_BORDER,
  MAZE_CELL_PATH
} MazeCellKind;

typedef struct MazeCell {
  MazeCellKind kind;
  struct MazeCell *neighbors[4];
  int neighbor_count;
  int x;
  int y;
} MazeCell;

typedef struct {
  int width;
  int height;
  MazeCell **cells;
  MazeCell *start;
  MazeCell *finish;
} Maze;

static void maze_cell_init(MazeCell *cell, int x, int y) {
  cell->kind = MAZE_CELL_WALL;
  cell->x = x;
  cell->y = y;
  cell->neighbor_count = 0;
}

static void maze_cell_add_neighbor(MazeCell *cell, MazeCell *neighbor) {
  if (cell->neighbor_count < 4) {
    cell->neighbors[cell->neighbor_count++] = neighbor;
  }
}

static bool maze_cell_is_walkable(MazeCell *cell) {
  return cell->kind == MAZE_CELL_SPACE || cell->kind == MAZE_CELL_START ||
         cell->kind == MAZE_CELL_FINISH;
}

static void maze_cell_reset(MazeCell *cell) {
  if (cell->kind == MAZE_CELL_SPACE) {
    cell->kind = MAZE_CELL_WALL;
  }
}

static Maze *maze_create(int width, int height) {
  if (width < 5)
    width = 5;
  if (height < 5)
    height = 5;

  Maze *maze = malloc(sizeof(Maze));
  maze->width = width;
  maze->height = height;

  maze->cells = malloc(height * sizeof(MazeCell *));
  for (int y = 0; y < height; y++) {
    maze->cells[y] = malloc(width * sizeof(MazeCell));
    for (int x = 0; x < width; x++) {
      maze_cell_init(&maze->cells[y][x], x, y);
    }
  }

  maze->start = &maze->cells[1][1];
  maze->finish = &maze->cells[height - 2][width - 2];
  maze->start->kind = MAZE_CELL_START;
  maze->finish->kind = MAZE_CELL_FINISH;

  return maze;
}

static void maze_update_neighbors(Maze *maze) {
  for (int y = 0; y < maze->height; y++) {
    for (int x = 0; x < maze->width; x++) {
      MazeCell *cell = &maze->cells[y][x];
      cell->neighbor_count = 0;

      if (x > 0 && y > 0 && x < maze->width - 1 && y < maze->height - 1) {
        maze_cell_add_neighbor(cell, &maze->cells[y - 1][x]);
        maze_cell_add_neighbor(cell, &maze->cells[y + 1][x]);
        maze_cell_add_neighbor(cell, &maze->cells[y][x + 1]);
        maze_cell_add_neighbor(cell, &maze->cells[y][x - 1]);

        for (int t = 0; t < 4; t++) {
          int i = Helper_next_int(4);
          int j = Helper_next_int(4);
          if (i != j) {
            MazeCell *temp = cell->neighbors[i];
            cell->neighbors[i] = cell->neighbors[j];
            cell->neighbors[j] = temp;
          }
        }
      } else {
        cell->kind = MAZE_CELL_BORDER;
      }
    }
  }
}

static void maze_reset(Maze *maze) {
  for (int y = 0; y < maze->height; y++) {
    for (int x = 0; x < maze->width; x++) {
      maze_cell_reset(&maze->cells[y][x]);
    }
  }
  maze->start->kind = MAZE_CELL_START;
  maze->finish->kind = MAZE_CELL_FINISH;
}

static void maze_dig(Maze *maze, MazeCell *start_cell) {
  size_t max_size = (size_t)maze->width * (size_t)maze->height;
  MazeCell **stack = malloc(max_size * sizeof(MazeCell *));
  if (!stack)
    return;

  size_t stack_size = 0;
  stack[stack_size++] = start_cell;

  while (stack_size > 0) {
    MazeCell *cell = stack[--stack_size];

    int walkable = 0;
    for (int i = 0; i < cell->neighbor_count; i++) {
      if (maze_cell_is_walkable(cell->neighbors[i]))
        walkable++;
    }

    if (walkable != 1)
      continue;

    cell->kind = MAZE_CELL_SPACE;

    for (int i = 0; i < cell->neighbor_count; i++) {
      MazeCell *n = cell->neighbors[i];
      if (n->kind == MAZE_CELL_WALL) {
        if (stack_size >= max_size) {
          max_size *= 2;
          MazeCell **new_stack = realloc(stack, max_size * sizeof(MazeCell *));
          if (!new_stack) {
            free(stack);
            return;
          }
          stack = new_stack;
        }
        stack[stack_size++] = n;
      }
    }
  }

  free(stack);
}

static void maze_ensure_open_finish(Maze *maze, MazeCell *cell) {
  (void)maze;
  cell->kind = MAZE_CELL_SPACE;

  int walkable = 0;
  for (int i = 0; i < cell->neighbor_count; i++) {
    if (maze_cell_is_walkable(cell->neighbors[i]))
      walkable++;
  }

  if (walkable > 1)
    return;

  for (int i = 0; i < cell->neighbor_count; i++) {
    MazeCell *n = cell->neighbors[i];
    if (n->kind == MAZE_CELL_WALL) {
      maze_ensure_open_finish(maze, n);
    }
  }
}

static void maze_generate(Maze *maze) {
  for (int i = 0; i < maze->start->neighbor_count; i++) {
    MazeCell *n = maze->start->neighbors[i];
    if (n->kind == MAZE_CELL_WALL) {
      maze_dig(maze, n);
    }
  }

  for (int i = 0; i < maze->finish->neighbor_count; i++) {
    MazeCell *n = maze->finish->neighbors[i];
    if (n->kind == MAZE_CELL_WALL) {
      maze_ensure_open_finish(maze, n);
    }
  }
}

static MazeCell *maze_middle_cell(Maze *maze) {
  return &maze->cells[maze->height / 2][maze->width / 2];
}

static uint32_t maze_checksum(Maze *maze) {
  uint32_t hasher = 2166136261UL;
  uint32_t prime = 16777619UL;

  for (int y = 0; y < maze->height; y++) {
    for (int x = 0; x < maze->width; x++) {
      if (maze->cells[y][x].kind == MAZE_CELL_SPACE) {
        uint32_t val = (uint32_t)(x * y);
        hasher = (hasher ^ val) * prime;
      }
    }
  }
  return hasher;
}

static void maze_free(Maze *maze) {
  if (!maze)
    return;
  for (int y = 0; y < maze->height; y++) {
    free(maze->cells[y]);
  }
  free(maze->cells);
  free(maze);
}

typedef struct {
  uint32_t result_val;
  int width;
  int height;
  Maze *maze;
} MazeGeneratorData;

void MazeGenerator_prepare(Benchmark *self) {
  MazeGeneratorData *data = (MazeGeneratorData *)self->data;
  data->result_val = 0;
}

void MazeGenerator_run(Benchmark *self, int iteration_id) {
  (void)iteration_id;
  MazeGeneratorData *data = (MazeGeneratorData *)self->data;

  maze_reset(data->maze);
  maze_generate(data->maze);

  data->result_val += maze_middle_cell(data->maze)->kind;
}

uint32_t MazeGenerator_checksum(Benchmark *self) {
  MazeGeneratorData *data = (MazeGeneratorData *)self->data;
  return data->result_val + maze_checksum(data->maze);
}

void MazeGenerator_cleanup(Benchmark *self) {
  MazeGeneratorData *data = (MazeGeneratorData *)self->data;
  if (data->maze) {
    maze_free(data->maze);
    data->maze = NULL;
  }
}

Benchmark *MazeGenerator_create(void) {
  Benchmark *bench = Benchmark_create("Maze::Generator");
  MazeGeneratorData *data = calloc(1, sizeof(MazeGeneratorData));

  data->width = (int)Helper_config_i64(bench->name, "w");
  data->height = (int)Helper_config_i64(bench->name, "h");
  if (data->width < 5)
    data->width = 5;
  if (data->height < 5)
    data->height = 5;

  data->maze = maze_create(data->width, data->height);
  maze_update_neighbors(data->maze);
  data->result_val = 0;

  bench->data = data;
  bench->prepare = MazeGenerator_prepare;
  bench->run = MazeGenerator_run;
  bench->checksum = MazeGenerator_checksum;
  bench->cleanup = MazeGenerator_cleanup;
  return bench;
}

typedef struct PathNode {
  MazeCell *cell;
  int parent;
} PathNode;

typedef struct {
  uint32_t result_val;
  int width;
  int height;
  Maze *maze;
  MazeCell **path;
  int path_length;
} MazeBFSData;

static MazeCell **maze_bfs(Maze *maze, MazeCell *start, MazeCell *target,
                           int *out_length) {
  if (start == target) {
    *out_length = 1;
    MazeCell **result = malloc(sizeof(MazeCell *));
    result[0] = start;
    return result;
  }

  int width = maze->width;
  int height = maze->height;

  int *queue = malloc(width * height * sizeof(int));
  int queue_head = 0;
  int queue_tail = 0;

  bool **visited = malloc(height * sizeof(bool *));
  for (int y = 0; y < height; y++) {
    visited[y] = calloc(width, sizeof(bool));
  }

  PathNode *path_nodes = malloc(width * height * sizeof(PathNode));
  int path_count = 0;

  visited[start->y][start->x] = true;
  path_nodes[path_count].cell = start;
  path_nodes[path_count].parent = -1;
  path_count++;
  queue[queue_tail++] = 0;

  while (queue_head < queue_tail) {
    int path_id = queue[queue_head++];
    MazeCell *cell = path_nodes[path_id].cell;

    for (int i = 0; i < cell->neighbor_count; i++) {
      MazeCell *neighbor = cell->neighbors[i];

      if (neighbor == target) {
        int length = 1;
        int cur = path_id;
        while (cur >= 0) {
          length++;
          cur = path_nodes[cur].parent;
        }

        MazeCell **result = malloc(length * sizeof(MazeCell *));
        int idx = length - 1;
        result[idx--] = target;

        cur = path_id;
        while (cur >= 0) {
          result[idx--] = path_nodes[cur].cell;
          cur = path_nodes[cur].parent;
        }

        *out_length = length;

        free(queue);
        for (int y = 0; y < height; y++)
          free(visited[y]);
        free(visited);
        free(path_nodes);

        return result;
      }

      if (maze_cell_is_walkable(neighbor) &&
          !visited[neighbor->y][neighbor->x]) {
        visited[neighbor->y][neighbor->x] = true;
        path_nodes[path_count].cell = neighbor;
        path_nodes[path_count].parent = path_id;
        queue[queue_tail++] = path_count;
        path_count++;
      }
    }
  }

  *out_length = 0;

  free(queue);
  for (int y = 0; y < height; y++)
    free(visited[y]);
  free(visited);
  free(path_nodes);

  return NULL;
}

static uint32_t mid_cell_checksum(MazeCell **path, int length) {
  if (length == 0)
    return 0;
  int mid = length / 2;
  MazeCell *cell = path[mid];
  return (uint32_t)(cell->x * cell->y);
}

void MazeBFS_prepare(Benchmark *self) {
  MazeBFSData *data = (MazeBFSData *)self->data;
  maze_generate(data->maze);
  data->result_val = 0;
  data->path = NULL;
  data->path_length = 0;
}

void MazeBFS_run(Benchmark *self, int iteration_id) {
  (void)iteration_id;
  MazeBFSData *data = (MazeBFSData *)self->data;

  if (data->path) {
    free(data->path);
    data->path = NULL;
  }

  data->path = maze_bfs(data->maze, data->maze->start, data->maze->finish,
                        &data->path_length);
  data->result_val += data->path_length;
}

uint32_t MazeBFS_checksum(Benchmark *self) {
  MazeBFSData *data = (MazeBFSData *)self->data;
  return data->result_val + mid_cell_checksum(data->path, data->path_length);
}

void MazeBFS_cleanup(Benchmark *self) {
  MazeBFSData *data = (MazeBFSData *)self->data;
  if (data->path)
    free(data->path);
  if (data->maze)
    maze_free(data->maze);
}

Benchmark *MazeBFS_create(void) {
  Benchmark *bench = Benchmark_create("Maze::BFS");
  MazeBFSData *data = calloc(1, sizeof(MazeBFSData));

  data->width = (int)Helper_config_i64(bench->name, "w");
  data->height = (int)Helper_config_i64(bench->name, "h");
  if (data->width < 5)
    data->width = 5;
  if (data->height < 5)
    data->height = 5;

  data->maze = maze_create(data->width, data->height);
  maze_update_neighbors(data->maze);
  data->result_val = 0;
  data->path = NULL;
  data->path_length = 0;

  bench->data = data;
  bench->prepare = MazeBFS_prepare;
  bench->run = MazeBFS_run;
  bench->checksum = MazeBFS_checksum;
  bench->cleanup = MazeBFS_cleanup;
  return bench;
}

typedef struct {
  int priority;
  int vertex;
} AStarPriorityQueueEntry;

typedef struct {
  AStarPriorityQueueEntry *heap;
  int size;
  int capacity;
} AStarPriorityQueue;

static AStarPriorityQueue *astar_pq_create(int capacity) {
  AStarPriorityQueue *pq = malloc(sizeof(AStarPriorityQueue));
  pq->heap = malloc(capacity * sizeof(AStarPriorityQueueEntry));
  pq->size = 0;
  pq->capacity = capacity;
  return pq;
}

static void astar_pq_push(AStarPriorityQueue *pq, int vertex, int priority) {

  if (pq->size >= pq->capacity) {
    pq->capacity *= 2;
    pq->heap =
        realloc(pq->heap, pq->capacity * sizeof(AStarPriorityQueueEntry));
  }

  int i = pq->size++;
  while (i > 0) {
    int parent = (i - 1) / 2;
    if (pq->heap[parent].priority <= priority)
      break;
    pq->heap[i] = pq->heap[parent];
    i = parent;
  }
  pq->heap[i].priority = priority;
  pq->heap[i].vertex = vertex;
}

static AStarPriorityQueueEntry astar_pq_pop(AStarPriorityQueue *pq) {
  AStarPriorityQueueEntry min = pq->heap[0];
  pq->size--;

  if (pq->size > 0) {
    AStarPriorityQueueEntry last = pq->heap[pq->size];
    int i = 0;
    while (1) {
      int left = 2 * i + 1;
      int right = 2 * i + 2;
      int smallest = i;

      if (left < pq->size &&
          pq->heap[left].priority < pq->heap[smallest].priority) {
        smallest = left;
      }
      if (right < pq->size &&
          pq->heap[right].priority < pq->heap[smallest].priority) {
        smallest = right;
      }

      if (smallest == i)
        break;

      pq->heap[i] = pq->heap[smallest];
      i = smallest;
    }
    pq->heap[i] = last;
  }

  return min;
}

static bool astar_pq_empty(AStarPriorityQueue *pq) { return pq->size == 0; }

static void astar_pq_free(AStarPriorityQueue *pq) {
  free(pq->heap);
  free(pq);
}

typedef struct {
  uint32_t result_val;
  int width;
  int height;
  Maze *maze;
  MazeCell **path;
  int path_length;
} MazeAStarData;

static int astar_heuristic(MazeCell *a, MazeCell *b) {
  return abs(a->x - b->x) + abs(a->y - b->y);
}

static int astar_idx(int y, int x, int width) { return y * width + x; }

static MazeCell **maze_astar_reconstruct(Maze *maze, int *came_from,
                                         int current_idx, int width,
                                         int *out_length) {
  int length = 0;
  int cur = current_idx;
  while (cur != -1) {
    length++;
    cur = came_from[cur];
  }

  MazeCell **result = malloc(length * sizeof(MazeCell *));
  cur = current_idx;
  for (int i = length - 1; i >= 0; i--) {
    int y = cur / width;
    int x = cur % width;
    result[i] = &maze->cells[y][x];
    cur = came_from[cur];
  }

  *out_length = length;
  return result;
}

static MazeCell **maze_astar(Maze *maze, MazeCell *start, MazeCell *target,
                             int *out_length) {
  if (start == target) {
    *out_length = 1;
    MazeCell **result = malloc(sizeof(MazeCell *));
    result[0] = start;
    return result;
  }

  int width = maze->width;
  int height = maze->height;
  int size = width * height;

  int *came_from = malloc(size * sizeof(int));
  int *g_score = malloc(size * sizeof(int));
  int *best_f = malloc(size * sizeof(int));
  for (int i = 0; i < size; i++) {
    came_from[i] = -1;
    g_score[i] = INT_MAX;
    best_f[i] = INT_MAX;
  }

  int start_idx = astar_idx(start->y, start->x, width);
  int target_idx = astar_idx(target->y, target->x, width);

  AStarPriorityQueue *open_set = astar_pq_create(size);

  g_score[start_idx] = 0;
  int f_start = astar_heuristic(start, target);
  astar_pq_push(open_set, start_idx, f_start);
  best_f[start_idx] = f_start;

  while (!astar_pq_empty(open_set)) {
    AStarPriorityQueueEntry entry = astar_pq_pop(open_set);
    int current_idx = entry.vertex;

    if (current_idx == target_idx) {
      MazeCell **result = maze_astar_reconstruct(maze, came_from, current_idx,
                                                 width, out_length);

      free(came_from);
      free(g_score);
      free(best_f);
      astar_pq_free(open_set);

      return result;
    }

    int current_y = current_idx / width;
    int current_x = current_idx % width;
    MazeCell *current = &maze->cells[current_y][current_x];
    int current_g = g_score[current_idx];

    for (int i = 0; i < current->neighbor_count; i++) {
      MazeCell *neighbor = current->neighbors[i];
      if (!maze_cell_is_walkable(neighbor))
        continue;

      int neighbor_idx = astar_idx(neighbor->y, neighbor->x, width);
      int tentative_g = current_g + 1;

      if (tentative_g < g_score[neighbor_idx]) {
        came_from[neighbor_idx] = current_idx;
        g_score[neighbor_idx] = tentative_g;
        int f_new = tentative_g + astar_heuristic(neighbor, target);

        if (f_new < best_f[neighbor_idx]) {
          best_f[neighbor_idx] = f_new;
          astar_pq_push(open_set, neighbor_idx, f_new);
        }
      }
    }
  }

  *out_length = 0;

  free(came_from);
  free(g_score);
  free(best_f);
  astar_pq_free(open_set);

  return NULL;
}

void MazeAStar_prepare(Benchmark *self) {
  MazeAStarData *data = (MazeAStarData *)self->data;
  maze_generate(data->maze);
  data->result_val = 0;
  data->path = NULL;
  data->path_length = 0;
}

void MazeAStar_run(Benchmark *self, int iteration_id) {
  (void)iteration_id;
  MazeAStarData *data = (MazeAStarData *)self->data;

  if (data->path) {
    free(data->path);
    data->path = NULL;
  }

  data->path = maze_astar(data->maze, data->maze->start, data->maze->finish,
                          &data->path_length);
  data->result_val += data->path_length;
}

uint32_t MazeAStar_checksum(Benchmark *self) {
  MazeAStarData *data = (MazeAStarData *)self->data;
  if (data->path_length == 0)
    return data->result_val;
  int mid = data->path_length / 2;
  MazeCell *cell = data->path[mid];
  return data->result_val + (uint32_t)(cell->x * cell->y);
}

void MazeAStar_cleanup(Benchmark *self) {
  MazeAStarData *data = (MazeAStarData *)self->data;
  if (data->path)
    free(data->path);
  if (data->maze)
    maze_free(data->maze);
}

Benchmark *MazeAStar_create(void) {
  Benchmark *bench = Benchmark_create("Maze::AStar");
  MazeAStarData *data = calloc(1, sizeof(MazeAStarData));

  data->width = (int)Helper_config_i64(bench->name, "w");
  data->height = (int)Helper_config_i64(bench->name, "h");
  if (data->width < 5)
    data->width = 5;
  if (data->height < 5)
    data->height = 5;

  data->maze = maze_create(data->width, data->height);
  maze_update_neighbors(data->maze);
  data->result_val = 0;
  data->path = NULL;
  data->path_length = 0;

  bench->data = data;
  bench->prepare = MazeAStar_prepare;
  bench->run = MazeAStar_run;
  bench->checksum = MazeAStar_checksum;
  bench->cleanup = MazeAStar_cleanup;
  return bench;
}



#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define SOLAR_MASS (4 * M_PI * M_PI)
#define DAYS_PER_YEAR 365.24

typedef struct {
  double x, y, z;
  double vx, vy, vz;
  double mass;
} NbodyPlanet;

typedef struct {
  int64_t iterations;
  NbodyPlanet *bodies;
  int64_t nbodies;
  double energy_before;
} NbodyData;

static void Nbody_Planet_init(NbodyPlanet *p, double x, double y, double z,
                              double vx, double vy, double vz, double mass) {
  p->x = x;
  p->y = y;
  p->z = z;
  p->vx = vx * DAYS_PER_YEAR;
  p->vy = vy * DAYS_PER_YEAR;
  p->vz = vz * DAYS_PER_YEAR;
  p->mass = mass * SOLAR_MASS;
}

static double Nbody_energy(NbodyPlanet *bodies, int64_t nbodies) {
  double e = 0.0;

  for (int64_t i = 0; i < nbodies; i++) {
    NbodyPlanet *b = &bodies[i];
    e += 0.5 * b->mass * (b->vx * b->vx + b->vy * b->vy + b->vz * b->vz);
    for (int64_t j = i + 1; j < nbodies; j++) {
      NbodyPlanet *b2 = &bodies[j];
      double dx = b->x - b2->x;
      double dy = b->y - b2->y;
      double dz = b->z - b2->z;
      double distance = sqrt(dx * dx + dy * dy + dz * dz);
      e -= (b->mass * b2->mass) / distance;
    }
  }
  return e;
}

static void Nbody_offset_momentum(NbodyPlanet *bodies, int64_t nbodies) {
  double px = 0.0, py = 0.0, pz = 0.0;

  for (int64_t i = 0; i < nbodies; i++) {
    NbodyPlanet *b = &bodies[i];
    px += b->vx * b->mass;
    py += b->vy * b->mass;
    pz += b->vz * b->mass;
  }

  NbodyPlanet *b = &bodies[0];
  b->vx = -px / SOLAR_MASS;
  b->vy = -py / SOLAR_MASS;
  b->vz = -pz / SOLAR_MASS;
}

static void Nbody_Planet_move_from_i(NbodyPlanet *bodies, int64_t nbodies,
                                     double dt, int64_t start) {

  NbodyPlanet *b1 = &bodies[start - 1];

  for (int64_t j = start; j < nbodies; j++) {
    NbodyPlanet *b2 = &bodies[j];
    double dx = b1->x - b2->x;
    double dy = b1->y - b2->y;
    double dz = b1->z - b2->z;

    double distance_sq = dx * dx + dy * dy + dz * dz;
    double distance = sqrt(distance_sq);
    double mag = dt / (distance * distance * distance);
    double b1_mass_mag = b1->mass * mag;
    double b2_mass_mag = b2->mass * mag;

    b1->vx -= dx * b2_mass_mag;
    b1->vy -= dy * b2_mass_mag;
    b1->vz -= dz * b2_mass_mag;
    b2->vx += dx * b1_mass_mag;
    b2->vy += dy * b1_mass_mag;
    b2->vz += dz * b1_mass_mag;
  }

  b1->x += dt * b1->vx;
  b1->y += dt * b1->vy;
  b1->z += dt * b1->vz;
}

void Nbody_prepare(Benchmark *self) {
  NbodyData *data = (NbodyData *)self->data;

  if (!data->bodies) {
    data->nbodies = 5;
    data->bodies = malloc(data->nbodies * sizeof(NbodyPlanet));

    Nbody_Planet_init(&data->bodies[0], 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0);

    Nbody_Planet_init(&data->bodies[1], 4.84143144246472090e+00,
                      -1.16032004402742839e+00, -1.03622044471123109e-01,
                      1.66007664274403694e-03, 7.69901118419740425e-03,
                      -6.90460016972063023e-05, 9.54791938424326609e-04);

    Nbody_Planet_init(&data->bodies[2], 8.34336671824457987e+00,
                      4.12479856412430479e+00, -4.03523417114321381e-01,
                      -2.76742510726862411e-03, 4.99852801234917238e-03,
                      2.30417297573763929e-05, 2.85885980666130812e-04);

    Nbody_Planet_init(&data->bodies[3], 1.28943695621391310e+01,
                      -1.51111514016986312e+01, -2.23307578892655734e-01,
                      2.96460137564761618e-03, 2.37847173959480950e-03,
                      -2.96589568540237556e-05, 4.36624404335156298e-05);

    Nbody_Planet_init(&data->bodies[4], 1.53796971148509165e+01,
                      -2.59193146099879641e+01, 1.79258772950371181e-01,
                      2.68067772490389322e-03, 1.62824170038242295e-03,
                      -9.51592254519715870e-05, 5.15138902046611451e-05);
  }

  Nbody_offset_momentum(data->bodies, data->nbodies);
  data->energy_before = Nbody_energy(data->bodies, data->nbodies);
}

void Nbody_run(Benchmark *self, int iteration_id) {
  (void)iteration_id;
  NbodyData *data = (NbodyData *)self->data;

  double dt = 0.01;
  int nbodies = (int)data->nbodies;

  for (int n = 0; n < 1000; n++) {
    for (int i = 0; i < nbodies; i++) {
      Nbody_Planet_move_from_i(data->bodies, data->nbodies, dt, i + 1);
    }
  }
}

uint32_t Nbody_checksum(Benchmark *self) {
  NbodyData *data = (NbodyData *)self->data;

  double energy_after = Nbody_energy(data->bodies, data->nbodies);

  uint32_t checksum_before = Helper_checksum_f64(data->energy_before);
  uint32_t checksum_after = Helper_checksum_f64(energy_after);

  return ((int64_t)checksum_before << 5) & checksum_after;
}

void Nbody_cleanup(Benchmark *self) {
  NbodyData *data = (NbodyData *)self->data;

  if (data) {
    if (data->bodies) {
      free(data->bodies);
      data->bodies = NULL;
    }
  }
}

Benchmark *Nbody_create(void) {
  Benchmark *bench = Benchmark_create("CLBG::Nbody");

  NbodyData *data = malloc(sizeof(NbodyData));
  memset(data, 0, sizeof(NbodyData));

  bench->data = data;

  bench->prepare = Nbody_prepare;
  bench->run = Nbody_run;
  bench->checksum = Nbody_checksum;
  bench->cleanup = Nbody_cleanup;

  return bench;
}


#define LEARNING_RATE 1.0
#define MOMENTUM 0.3
#define TRAIN_RATE 0.3

typedef struct NeuralNetNeuron NeuralNetNeuron;
typedef struct NeuralNetSynapse NeuralNetSynapse;

struct NeuralNetSynapse {
  double weight;
  double prev_weight;
  NeuralNetNeuron *source_neuron;
  NeuralNetNeuron *dest_neuron;
};

struct NeuralNetNeuron {

  NeuralNetSynapse **synapses_in;
  int synapses_in_count;
  int synapses_in_capacity;

  NeuralNetSynapse **synapses_out;
  int synapses_out_count;
  int synapses_out_capacity;

  double threshold;
  double prev_threshold;
  double error;
  double output;
};

static void neuron_update_weights(NeuralNetNeuron *neuron);

typedef struct {
  NeuralNetNeuron *neurons;
  int total_neurons;

  int *input_layer;
  int input_count;
  int *hidden_layer;
  int hidden_count;
  int *output_layer;
  int output_count;

  NeuralNetSynapse *synapses;
  int synapse_count;
  int synapse_capacity;
} NeuralNetwork;

typedef struct {
  uint32_t result_val;
  double sum_outputs;
  NeuralNetwork *xor_net;
} NeuralNetData;

static void neuron_init(NeuralNetNeuron *neuron) {

  double r = Helper_next_float(1.0);
  neuron->threshold = neuron->prev_threshold = r * 2.0 - 1.0;
  neuron->output = 0.0;
  neuron->error = 0.0;

  neuron->synapses_in_count = 0;
  neuron->synapses_in_capacity = 4;
  neuron->synapses_in =
      malloc(neuron->synapses_in_capacity * sizeof(NeuralNetSynapse *));

  neuron->synapses_out_count = 0;
  neuron->synapses_out_capacity = 4;
  neuron->synapses_out =
      malloc(neuron->synapses_out_capacity * sizeof(NeuralNetSynapse *));
}

static void neuron_add_synapse_in(NeuralNetNeuron *neuron,
                                  NeuralNetSynapse *synapse) {
  if (neuron->synapses_in_count >= neuron->synapses_in_capacity) {
    neuron->synapses_in_capacity *= 2;
    neuron->synapses_in =
        realloc(neuron->synapses_in,
                neuron->synapses_in_capacity * sizeof(NeuralNetSynapse *));
  }
  neuron->synapses_in[neuron->synapses_in_count++] = synapse;
}

static void neuron_add_synapse_out(NeuralNetNeuron *neuron,
                                   NeuralNetSynapse *synapse) {
  if (neuron->synapses_out_count >= neuron->synapses_out_capacity) {
    neuron->synapses_out_capacity *= 2;
    neuron->synapses_out =
        realloc(neuron->synapses_out,
                neuron->synapses_out_capacity * sizeof(NeuralNetSynapse *));
  }
  neuron->synapses_out[neuron->synapses_out_count++] = synapse;
}

static double neuron_derivative(NeuralNetNeuron *neuron) {
  return neuron->output * (1.0 - neuron->output);
}

static void neuron_calculate_output(NeuralNetNeuron *neuron) {
  double activation = 0.0;
  for (int i = 0; i < neuron->synapses_in_count; i++) {
    NeuralNetSynapse *synapse = neuron->synapses_in[i];
    activation += synapse->weight * synapse->source_neuron->output;
  }
  activation -= neuron->threshold;
  neuron->output = 1.0 / (1.0 + exp(-activation));
}

static void neuron_output_train(NeuralNetNeuron *neuron, double target) {
  neuron->error = (target - neuron->output) * neuron_derivative(neuron);
  neuron_update_weights(neuron);
}

static void neuron_hidden_train(NeuralNetNeuron *neuron) {
  double sum = 0.0;
  for (int i = 0; i < neuron->synapses_out_count; i++) {
    NeuralNetSynapse *synapse = neuron->synapses_out[i];
    sum += synapse->prev_weight * synapse->dest_neuron->error;
  }
  neuron->error = sum * neuron_derivative(neuron);
  neuron_update_weights(neuron);
}

static void neuron_update_weights(NeuralNetNeuron *neuron) {

  for (int i = 0; i < neuron->synapses_in_count; i++) {
    NeuralNetSynapse *synapse = neuron->synapses_in[i];
    double temp_weight = synapse->weight;
    synapse->weight += (TRAIN_RATE * LEARNING_RATE * neuron->error *
                        synapse->source_neuron->output) +
                       (MOMENTUM * (synapse->weight - synapse->prev_weight));
    synapse->prev_weight = temp_weight;
  }

  double temp_threshold = neuron->threshold;
  neuron->threshold +=
      (TRAIN_RATE * LEARNING_RATE * neuron->error * -1.0) +
      (MOMENTUM * (neuron->threshold - neuron->prev_threshold));
  neuron->prev_threshold = temp_threshold;
}

static NeuralNetwork *network_new(int inputs, int hidden, int outputs) {
  NeuralNetwork *net = malloc(sizeof(NeuralNetwork));

  net->total_neurons = inputs + hidden + outputs;
  net->neurons = malloc(net->total_neurons * sizeof(NeuralNetNeuron));

  for (int i = 0; i < net->total_neurons; i++) {
    neuron_init(&net->neurons[i]);
  }

  net->input_count = inputs;
  net->hidden_count = hidden;
  net->output_count = outputs;

  net->input_layer = malloc(inputs * sizeof(int));
  net->hidden_layer = malloc(hidden * sizeof(int));
  net->output_layer = malloc(outputs * sizeof(int));

  for (int i = 0; i < inputs; i++) {
    net->input_layer[i] = i;
  }
  for (int i = 0; i < hidden; i++) {
    net->hidden_layer[i] = inputs + i;
  }
  for (int i = 0; i < outputs; i++) {
    net->output_layer[i] = inputs + hidden + i;
  }

  net->synapse_count = 0;
  net->synapse_capacity = (inputs * hidden) + (hidden * outputs);
  net->synapses = malloc(net->synapse_capacity * sizeof(NeuralNetSynapse));

  for (int i = 0; i < inputs; i++) {
    NeuralNetNeuron *source = &net->neurons[net->input_layer[i]];
    for (int j = 0; j < hidden; j++) {
      NeuralNetNeuron *dest = &net->neurons[net->hidden_layer[j]];

      NeuralNetSynapse *synapse = &net->synapses[net->synapse_count++];

      double r = Helper_next_float(1.0);
      synapse->weight = synapse->prev_weight = r * 2.0 - 1.0;

      synapse->source_neuron = source;
      synapse->dest_neuron = dest;

      neuron_add_synapse_out(source, synapse);
      neuron_add_synapse_in(dest, synapse);
    }
  }

  for (int i = 0; i < hidden; i++) {
    NeuralNetNeuron *source = &net->neurons[net->hidden_layer[i]];
    for (int j = 0; j < outputs; j++) {
      NeuralNetNeuron *dest = &net->neurons[net->output_layer[j]];

      NeuralNetSynapse *synapse = &net->synapses[net->synapse_count++];

      double r = Helper_next_float(1.0);
      synapse->weight = synapse->prev_weight = r * 2.0 - 1.0;

      synapse->source_neuron = source;
      synapse->dest_neuron = dest;

      neuron_add_synapse_out(source, synapse);
      neuron_add_synapse_in(dest, synapse);
    }
  }

  return net;
}

static void network_free(NeuralNetwork *net) {
  if (!net)
    return;

  for (int i = 0; i < net->total_neurons; i++) {
    free(net->neurons[i].synapses_in);
    free(net->neurons[i].synapses_out);
  }

  free(net->neurons);
  free(net->input_layer);
  free(net->hidden_layer);
  free(net->output_layer);
  free(net->synapses);
  free(net);
}

static void network_train(NeuralNetwork *net, double *inputs, double *targets) {

  for (int i = 0; i < net->input_count; i++) {
    net->neurons[net->input_layer[i]].output = inputs[i];
  }

  for (int i = 0; i < net->hidden_count; i++) {
    neuron_calculate_output(&net->neurons[net->hidden_layer[i]]);
  }

  for (int i = 0; i < net->output_count; i++) {
    neuron_calculate_output(&net->neurons[net->output_layer[i]]);
  }

  for (int i = 0; i < net->output_count; i++) {
    neuron_output_train(&net->neurons[net->output_layer[i]], targets[i]);
  }

  for (int i = 0; i < net->hidden_count; i++) {
    neuron_hidden_train(&net->neurons[net->hidden_layer[i]]);
  }
}

static void network_feed_forward(NeuralNetwork *net, double *inputs) {
  for (int i = 0; i < net->input_count; i++) {
    net->neurons[net->input_layer[i]].output = inputs[i];
  }

  for (int i = 0; i < net->hidden_count; i++) {
    neuron_calculate_output(&net->neurons[net->hidden_layer[i]]);
  }

  for (int i = 0; i < net->output_count; i++) {
    neuron_calculate_output(&net->neurons[net->output_layer[i]]);
  }
}

static double network_get_output(NeuralNetwork *net) {
  return net->neurons[net->output_layer[0]].output;
}

void NeuralNet_prepare(Benchmark *self) {
  NeuralNetData *data = (NeuralNetData *)self->data;

  data->result_val = 0;
  data->sum_outputs = 0.0;

  if (data->xor_net) {
    network_free(data->xor_net);
  }

  Helper_reset();

  data->xor_net = network_new(2, 10, 1);
}

void NeuralNet_run(Benchmark *self, int iteration_id) {
  (void)iteration_id;
  NeuralNetData *data = (NeuralNetData *)self->data;

  double inputs_00[2] = {0, 0};
  double targets_0[1] = {0};

  double inputs_10[2] = {1, 0};
  double inputs_01[2] = {0, 1};
  double targets_1[1] = {1};

  double inputs_11[2] = {1, 1};
  double targets_0_again[1] = {0};

  for (int iter = 0; iter < 1000; iter++) {
    network_train(data->xor_net, inputs_00, targets_0);
    network_train(data->xor_net, inputs_10, targets_1);
    network_train(data->xor_net, inputs_01, targets_1);
    network_train(data->xor_net, inputs_11, targets_0_again);
  }
}

uint32_t NeuralNet_checksum(Benchmark *self) {
  NeuralNetData *data = (NeuralNetData *)self->data;

  if (!data->xor_net) {
    return 0;
  }

  double inputs_00[2] = {0, 0};
  double inputs_01[2] = {0, 1};
  double inputs_10[2] = {1, 0};
  double inputs_11[2] = {1, 1};

  double sum = 0.0;

  network_feed_forward(data->xor_net, inputs_00);
  sum += network_get_output(data->xor_net);

  network_feed_forward(data->xor_net, inputs_01);
  sum += network_get_output(data->xor_net);

  network_feed_forward(data->xor_net, inputs_10);
  sum += network_get_output(data->xor_net);

  network_feed_forward(data->xor_net, inputs_11);
  sum += network_get_output(data->xor_net);

  data->sum_outputs = sum;

  return Helper_checksum_f64(sum);
}

void NeuralNet_cleanup(Benchmark *self) {
  NeuralNetData *data = (NeuralNetData *)self->data;

  if (data->xor_net) {
    network_free(data->xor_net);
    data->xor_net = NULL;
  }
}

Benchmark *NeuralNet_create(void) {
  Benchmark *bench = Benchmark_create("Etc::NeuralNet");

  NeuralNetData *data = malloc(sizeof(NeuralNetData));
  memset(data, 0, sizeof(NeuralNetData));

  bench->data = data;

  bench->prepare = NeuralNet_prepare;
  bench->run = NeuralNet_run;
  bench->checksum = NeuralNet_checksum;
  bench->cleanup = NeuralNet_cleanup;

  return bench;
}


typedef struct {
  int64_t limit;
  uint32_t checksum;
} SieveData;

static int *sieve_generate(int64_t limit, int *count) {
  if (limit < 2) {
    *count = 0;
    return NULL;
  }

  uint8_t *primes = malloc((limit + 1) * sizeof(uint8_t));
  if (!primes)
    return NULL;

  memset(primes, 1, (limit + 1) * sizeof(uint8_t));
  primes[0] = 0;
  primes[1] = 0;

  int sqrt_limit = (int)sqrt((double)limit);

  for (int p = 2; p <= sqrt_limit; p++) {
    if (primes[p] == 1) {

      for (int multiple = p * p; multiple <= limit; multiple += p) {
        primes[multiple] = 0;
      }
    }
  }

  int last_prime = 2;
  int count_primes = 1;

  for (int n = 3; n <= limit; n += 2) {
    if (primes[n] == 1) {
      last_prime = n;
      count_primes++;
    }
  }

  free(primes);

  *count = last_prime + count_primes;
  return NULL;
}

void Sieve_prepare(Benchmark *self) {
  SieveData *data = (SieveData *)self->data;
  data->limit = Helper_config_i64(self->name, "limit");
  if (data->limit <= 0)
    data->limit = 1000000;
  data->checksum = 0;
}

void Sieve_run(Benchmark *self, int iteration_id) {
  (void)iteration_id;
  SieveData *data = (SieveData *)self->data;

  int sum = 0;
  sieve_generate(data->limit, &sum);

  data->checksum += (uint32_t)sum;
}

uint32_t Sieve_checksum(Benchmark *self) {
  SieveData *data = (SieveData *)self->data;
  return data->checksum;
}

Benchmark *Sieve_create(void) {
  Benchmark *bench = Benchmark_create("Etc::Sieve");

  SieveData *data = malloc(sizeof(SieveData));
  memset(data, 0, sizeof(SieveData));

  bench->data = data;
  bench->prepare = Sieve_prepare;
  bench->run = Sieve_run;
  bench->checksum = Sieve_checksum;

  return bench;
}


typedef struct {
  int64_t size_val;
  int32_t *data;
  uint32_t result_val;
} SortBaseData;

static void sort_base_prepare(Benchmark *self, const char *bench_name) {
  SortBaseData *data = (SortBaseData *)self->data;

  data->size_val = Helper_config_i64(bench_name, "size");
  if (data->size_val <= 0) {
    data->size_val = 100000;
  }

  data->data = malloc(data->size_val * sizeof(int32_t));

  for (int64_t i = 0; i < data->size_val; i++) {
    data->data[i] = Helper_next_int(1000000);
  }

  data->result_val = 0;
}

static void sort_base_cleanup(SortBaseData *data) {
  if (data->data) {
    free(data->data);
    data->data = NULL;
  }
}

typedef struct {
  SortBaseData base;
  int32_t *sorted_data;
} SortQuickData;

static void sort_quick_quick_sort(int32_t *arr, int64_t low, int64_t high) {
  if (low >= high)
    return;

  int32_t pivot = arr[(low + high) / 2];
  int64_t i = low, j = high;

  while (i <= j) {
    while (arr[i] < pivot)
      i++;
    while (arr[j] > pivot)
      j--;
    if (i <= j) {
      int32_t temp = arr[i];
      arr[i] = arr[j];
      arr[j] = temp;
      i++;
      j--;
    }
  }

  sort_quick_quick_sort(arr, low, j);
  sort_quick_quick_sort(arr, i, high);
}

static void sort_quick_test(SortQuickData *data) {
  if (data->sorted_data) {
    free(data->sorted_data);
  }

  data->sorted_data = malloc(data->base.size_val * sizeof(int32_t));
  memcpy(data->sorted_data, data->base.data,
         data->base.size_val * sizeof(int32_t));

  sort_quick_quick_sort(data->sorted_data, 0, data->base.size_val - 1);
}

void SortQuick_prepare(Benchmark *self) {
  SortQuickData *data = (SortQuickData *)self->data;
  sort_base_prepare(self, "Sort::Quick");
  data->sorted_data = NULL;
}

void SortQuick_run(Benchmark *self, int iteration_id) {
  (void)iteration_id;
  SortQuickData *data = (SortQuickData *)self->data;

  sort_quick_test(data);

  int32_t random_index1 = Helper_next_int((uint32_t)data->base.size_val);
  int32_t random_index2 = Helper_next_int((uint32_t)data->base.size_val);

  uint32_t iteration_result = (uint32_t)data->base.data[random_index1] +
                              (uint32_t)data->sorted_data[random_index2];
  data->base.result_val += iteration_result;
}

uint32_t SortQuick_checksum(Benchmark *self) {
  SortQuickData *data = (SortQuickData *)self->data;
  return data->base.result_val;
}

void SortQuick_cleanup(Benchmark *self) {
  SortQuickData *data = (SortQuickData *)self->data;
  sort_base_cleanup(&data->base);
  if (data->sorted_data) {
    free(data->sorted_data);
    data->sorted_data = NULL;
  }
}

Benchmark *SortQuick_create(void) {
  Benchmark *bench = Benchmark_create("Sort::Quick");

  SortQuickData *data = malloc(sizeof(SortQuickData));
  memset(data, 0, sizeof(SortQuickData));

  bench->data = data;

  bench->prepare = SortQuick_prepare;
  bench->run = SortQuick_run;
  bench->checksum = SortQuick_checksum;
  bench->cleanup = SortQuick_cleanup;

  return bench;
}

typedef struct {
  SortBaseData base;
  int32_t *sorted_data;
} SortMergeData;

static void sort_merge_merge(int32_t *arr, int32_t *temp, int64_t left,
                             int64_t mid, int64_t right) {
  for (int64_t i = left; i <= right; i++) {
    temp[i] = arr[i];
  }

  int64_t i = left, j = mid + 1, k = left;

  while (i <= mid && j <= right) {
    if (temp[i] <= temp[j]) {
      arr[k] = temp[i];
      i++;
    } else {
      arr[k] = temp[j];
      j++;
    }
    k++;
  }

  while (i <= mid) {
    arr[k] = temp[i];
    i++;
    k++;
  }
}

static void sort_merge_merge_sort_helper(int32_t *arr, int32_t *temp,
                                         int64_t left, int64_t right) {
  if (left >= right)
    return;

  int64_t mid = (left + right) / 2;
  sort_merge_merge_sort_helper(arr, temp, left, mid);
  sort_merge_merge_sort_helper(arr, temp, mid + 1, right);
  sort_merge_merge(arr, temp, left, mid, right);
}

static void sort_merge_merge_sort_inplace(int32_t *arr, int64_t size) {
  int32_t *temp = malloc(size * sizeof(int32_t));
  sort_merge_merge_sort_helper(arr, temp, 0, size - 1);
  free(temp);
}

static void sort_merge_test(SortMergeData *data) {
  if (data->sorted_data) {
    free(data->sorted_data);
  }

  data->sorted_data = malloc(data->base.size_val * sizeof(int32_t));
  memcpy(data->sorted_data, data->base.data,
         data->base.size_val * sizeof(int32_t));

  sort_merge_merge_sort_inplace(data->sorted_data, data->base.size_val);
}

void SortMerge_prepare(Benchmark *self) {
  SortMergeData *data = (SortMergeData *)self->data;
  sort_base_prepare(self, "Sort::Merge");
  data->sorted_data = NULL;
}

void SortMerge_run(Benchmark *self, int iteration_id) {
  (void)iteration_id;
  SortMergeData *data = (SortMergeData *)self->data;

  sort_merge_test(data);

  int32_t random_index1 = Helper_next_int((uint32_t)data->base.size_val);
  int32_t random_index2 = Helper_next_int((uint32_t)data->base.size_val);

  uint32_t iteration_result = (uint32_t)data->base.data[random_index1] +
                              (uint32_t)data->sorted_data[random_index2];
  data->base.result_val += iteration_result;
}

uint32_t SortMerge_checksum(Benchmark *self) {
  SortMergeData *data = (SortMergeData *)self->data;
  return data->base.result_val;
}

void SortMerge_cleanup(Benchmark *self) {
  SortMergeData *data = (SortMergeData *)self->data;
  sort_base_cleanup(&data->base);
  if (data->sorted_data) {
    free(data->sorted_data);
    data->sorted_data = NULL;
  }
}

Benchmark *SortMerge_create(void) {
  Benchmark *bench = Benchmark_create("Sort::Merge");

  SortMergeData *data = malloc(sizeof(SortMergeData));
  memset(data, 0, sizeof(SortMergeData));

  bench->data = data;

  bench->prepare = SortMerge_prepare;
  bench->run = SortMerge_run;
  bench->checksum = SortMerge_checksum;
  bench->cleanup = SortMerge_cleanup;

  return bench;
}

typedef struct {
  SortBaseData base;
  int32_t *sorted_data;
} SortSelfData;

static int sort_self_compare(const void *a, const void *b) {
  int32_t ia = *(const int32_t *)a;
  int32_t ib = *(const int32_t *)b;
  return (ia > ib) - (ia < ib);
}

static void sort_self_test(SortSelfData *data) {
  if (data->sorted_data) {
    free(data->sorted_data);
  }

  data->sorted_data = malloc(data->base.size_val * sizeof(int32_t));
  memcpy(data->sorted_data, data->base.data,
         data->base.size_val * sizeof(int32_t));

  qsort(data->sorted_data, (size_t)data->base.size_val, sizeof(int32_t),
        sort_self_compare);
}

void SortSelf_prepare(Benchmark *self) {
  SortSelfData *data = (SortSelfData *)self->data;
  sort_base_prepare(self, "Sort::Self");
  data->sorted_data = NULL;
}

void SortSelf_run(Benchmark *self, int iteration_id) {
  (void)iteration_id;
  SortSelfData *data = (SortSelfData *)self->data;

  sort_self_test(data);

  int32_t random_index1 = Helper_next_int((uint32_t)data->base.size_val);
  int32_t random_index2 = Helper_next_int((uint32_t)data->base.size_val);

  uint32_t iteration_result = (uint32_t)data->base.data[random_index1] +
                              (uint32_t)data->sorted_data[random_index2];
  data->base.result_val += iteration_result;
}

uint32_t SortSelf_checksum(Benchmark *self) {
  SortSelfData *data = (SortSelfData *)self->data;
  return data->base.result_val;
}

void SortSelf_cleanup(Benchmark *self) {
  SortSelfData *data = (SortSelfData *)self->data;
  sort_base_cleanup(&data->base);
  if (data->sorted_data) {
    free(data->sorted_data);
    data->sorted_data = NULL;
  }
}

Benchmark *SortSelf_create(void) {
  Benchmark *bench = Benchmark_create("Sort::Self");

  SortSelfData *data = malloc(sizeof(SortSelfData));
  memset(data, 0, sizeof(SortSelfData));

  bench->data = data;

  bench->prepare = SortSelf_prepare;
  bench->run = SortSelf_run;
  bench->checksum = SortSelf_checksum;
  bench->cleanup = SortSelf_cleanup;

  return bench;
}


typedef struct {
  int64_t size_val;
  double *u;
  double *v;
} SpectralnormData;

void Spectralnorm_prepare(Benchmark *self) {
  SpectralnormData *data = (SpectralnormData *)self->data;
  data->size_val = Helper_config_i64(self->name, "size");
  if (data->size_val <= 0) {
    data->size_val = 100;
  }

  data->u = malloc(data->size_val * sizeof(double));
  data->v = malloc(data->size_val * sizeof(double));

  for (int64_t i = 0; i < data->size_val; i++) {
    data->u[i] = 1.0;
    data->v[i] = 1.0;
  }
}

static double eval_A(int64_t i, int64_t j) {
  return 1.0 / ((i + j) * (i + j + 1.0) / 2.0 + i + 1.0);
}

static double *eval_A_times_u(const double *u, int64_t n) {
  double *result = malloc(n * sizeof(double));
  for (int64_t i = 0; i < n; i++) {
    double sum = 0.0;
    for (int64_t j = 0; j < n; j++) {
      sum += eval_A(i, j) * u[j];
    }
    result[i] = sum;
  }
  return result;
}

static double *eval_At_times_u(const double *u, int64_t n) {
  double *result = malloc(n * sizeof(double));
  for (int64_t i = 0; i < n; i++) {
    double sum = 0.0;
    for (int64_t j = 0; j < n; j++) {
      sum += eval_A(j, i) * u[j];
    }
    result[i] = sum;
  }
  return result;
}

static double *eval_AtA_times_u(const double *u, int64_t n) {
  double *temp = eval_A_times_u(u, n);
  double *result = eval_At_times_u(temp, n);
  free(temp);
  return result;
}

void Spectralnorm_run(Benchmark *self, int iteration_id) {
  (void)iteration_id;
  SpectralnormData *data = (SpectralnormData *)self->data;

  double *new_v = eval_AtA_times_u(data->u, data->size_val);
  double *new_u = eval_AtA_times_u(new_v, data->size_val);

  free(data->u);
  free(data->v);

  data->u = new_u;
  data->v = new_v;
}

uint32_t Spectralnorm_checksum(Benchmark *self) {
  SpectralnormData *data = (SpectralnormData *)self->data;

  double vBv = 0.0, vv = 0.0;
  for (int64_t i = 0; i < data->size_val; i++) {
    vBv += data->u[i] * data->v[i];
    vv += data->v[i] * data->v[i];
  }

  double result = sqrt(vBv / vv);
  return Helper_checksum_f64(result);
}

void Spectralnorm_cleanup(Benchmark *self) {
  SpectralnormData *data = (SpectralnormData *)self->data;
  if (data->u)
    free(data->u);
  if (data->v)
    free(data->v);
}

Benchmark *Spectralnorm_create(void) {
  Benchmark *bench = Benchmark_create("CLBG::Spectralnorm");

  SpectralnormData *data = malloc(sizeof(SpectralnormData));
  memset(data, 0, sizeof(SpectralnormData));

  bench->data = data;

  bench->prepare = Spectralnorm_prepare;
  bench->run = Spectralnorm_run;
  bench->checksum = Spectralnorm_checksum;
  bench->cleanup = Spectralnorm_cleanup;

  return bench;
}

typedef struct VarEntry {
    char *key;
    char *value;
    UT_hash_handle hh;
} VarEntry;

typedef struct {
    int count;
    uint32_t checksum_val;
    char *text;
    size_t text_size;
    char *rendered;
    size_t rendered_size;
    VarEntry *vars;
} TemplateBaseData;

typedef struct {
    TemplateBaseData base;
    re_t re;
} TemplateRegexData;

typedef struct {
    TemplateBaseData base;
} TemplateParseData;

static const char *FIRST_NAMES[] = {"John",    "Jane",  "Bob",   "Alice",
                                    "Charlie", "Diana", "Sarah", "Mike"};
static const char *LAST_NAMES[] = {"Smith",  "Johnson", "Brown",  "Taylor",
                                   "Wilson", "Davis",   "Miller", "Jones"};
static const char *CITIES[] = {"New York", "Los Angeles", "Chicago",
                               "Houston",  "Phoenix",     "San Francisco"};
static const char *LOREM =
    "Lorem {ipsum} dolor {sit} amet, consectetur adipiscing elit. Sed do "
    "eiusmod tempor incididunt ut labore {et} dolore magna aliqua. ";

#define FIRST_NAMES_COUNT 8
#define LAST_NAMES_COUNT 8
#define CITIES_COUNT 6

static void add_var(TemplateBaseData *data, const char *key,
                    const char *value) {
    VarEntry *entry = malloc(sizeof(VarEntry));
    entry->key = strdup(key);
    entry->value = strdup(value);
    HASH_ADD_KEYPTR(hh, data->vars, entry->key, strlen(entry->key), entry);
}

static const char *get_var(TemplateBaseData *data, const char *key) {
    VarEntry *entry = NULL;
    HASH_FIND_STR(data->vars, key, entry);
    return entry ? entry->value : NULL;
}

static void clear_vars(TemplateBaseData *data) {
    VarEntry *current, *tmp;
    HASH_ITER(hh, data->vars, current, tmp) {
        HASH_DEL(data->vars, current);
        free(current->key);
        free(current->value);
        free(current);
    }
}

static int prepare_template(TemplateBaseData *data) {
    if (data->text) {
        free(data->text);
        data->text = NULL;
    }

    clear_vars(data);

    size_t estimated_size = data->count * 200 + 1;
    char *text_buf = malloc(estimated_size);
    if (!text_buf)
        return -1;

    size_t pos = 0;

    pos += sprintf(text_buf + pos, "<html><body>");
    pos += sprintf(text_buf + pos, "<h1>{{TITLE}}</h1>");
    add_var(data, "TITLE", "Template title");
    pos += sprintf(text_buf + pos, "<p>");
    pos += sprintf(text_buf + pos, "%s", LOREM);
    pos += sprintf(text_buf + pos, "</p>");
    pos += sprintf(text_buf + pos, "<table>");

    for (int i = 0; i < data->count; i++) {
        if (i % 3 == 0) {
            pos += sprintf(text_buf + pos, "<!-- {comment} -->");
        }
        pos += sprintf(text_buf + pos, "<tr>");

        char key_buf[32];

        pos += sprintf(text_buf + pos, "<td>{{ FIRST_NAME%d }}</td>", i);
        sprintf(key_buf, "FIRST_NAME%d", i);
        add_var(data, key_buf, FIRST_NAMES[i % FIRST_NAMES_COUNT]);

        pos += sprintf(text_buf + pos, "<td>{{LAST_NAME%d}}</td>", i);
        sprintf(key_buf, "LAST_NAME%d", i);
        add_var(data, key_buf, LAST_NAMES[i % LAST_NAMES_COUNT]);

        pos += sprintf(text_buf + pos, "<td>{{  CITY%d  }}</td>", i);
        sprintf(key_buf, "CITY%d", i);
        add_var(data, key_buf, CITIES[i % CITIES_COUNT]);

        pos += sprintf(text_buf + pos, "<td>{balance: %d}</td>", i % 100);
        pos += sprintf(text_buf + pos, "</tr>\n");
    }

    pos += sprintf(text_buf + pos, "</table>");
    pos += sprintf(text_buf + pos, "</body></html>");

    data->text = text_buf;
    data->text_size = pos;

    return 0;
}

void TemplateRegex_prepare(Benchmark *self) {
    TemplateRegexData *data = (TemplateRegexData *)self->data;

    data->base.count = (int)Helper_config_i64(self->name, "count");
    prepare_template(&data->base);

    data->re = re_compile("\\{\\{[^}]*\\}\\}");
}

void TemplateRegex_run(Benchmark *self, int iteration_id) {
    (void)iteration_id;
    TemplateRegexData *data = (TemplateRegexData *)self->data;
    TemplateBaseData *base = &data->base;

    if (!data->re || !base->text)
        return;

    size_t estimated_size = base->text_size * 2;
    char *result_buf = malloc(estimated_size);
    if (!result_buf)
        return;

    size_t result_pos = 0;
    size_t last_pos = 0;
    size_t text_len = base->text_size;
    const char *text = base->text;

    int match_len;
    int match_idx = re_matchp(data->re, text + last_pos, &match_len);

    while (match_idx != -1) {
        match_idx += last_pos;

        if ((size_t)match_idx > last_pos) {
            size_t len = match_idx - last_pos;
            memcpy(result_buf + result_pos, text + last_pos, len);
            result_pos += len;
        }

        size_t key_start = match_idx + 2;
        size_t key_end = match_idx + match_len - 2;

        if (key_end > key_start) {
            size_t key_len = key_end - key_start;
            char *key = malloc(key_len + 1);
            memcpy(key, text + key_start, key_len);
            key[key_len] = '\0';

            char *start = key;
            while (*start == ' ' || *start == '\t')
                start++;
            char *end = key + key_len - 1;
            while (end > start && (*end == ' ' || *end == '\t'))
                end--;
            *(end + 1) = '\0';

            const char *value = get_var(base, start);
            if (value) {
                size_t value_len = strlen(value);
                memcpy(result_buf + result_pos, value, value_len);
                result_pos += value_len;
            }

            free(key);
        }

        last_pos = match_idx + match_len;

        if (last_pos >= text_len)
            break;

        match_idx = re_matchp(data->re, text + last_pos, &match_len);
    }

    if (last_pos < text_len) {
        size_t len = text_len - last_pos;
        memcpy(result_buf + result_pos, text + last_pos, len);
        result_pos += len;
    }

    result_buf[result_pos] = '\0';

    if (base->rendered) {
        free(base->rendered);
    }
    base->rendered = result_buf;
    base->rendered_size = result_pos;
    base->checksum_val += (uint32_t)result_pos;
}

uint32_t TemplateRegex_checksum(Benchmark *self) {
    TemplateRegexData *data = (TemplateRegexData *)self->data;
    TemplateBaseData *base = &data->base;
    return base->checksum_val + Helper_checksum_string(base->rendered);
}

void TemplateRegex_cleanup(Benchmark *self) {
    TemplateRegexData *data = (TemplateRegexData *)self->data;
    TemplateBaseData *base = &data->base;

    if (base->text) {
        free(base->text);
        base->text = NULL;
    }

    if (base->rendered) {
        free(base->rendered);
        base->rendered = NULL;
    }

    clear_vars(base);

    data->re = NULL;

    base->text_size = 0;
    base->rendered_size = 0;
    base->checksum_val = 0;
}

Benchmark *TemplateRegex_create(void) {
    Benchmark *bench = Benchmark_create("Template::Regex");

    TemplateRegexData *data = calloc(1, sizeof(TemplateRegexData));

    bench->data = data;
    bench->prepare = TemplateRegex_prepare;
    bench->run = TemplateRegex_run;
    bench->checksum = TemplateRegex_checksum;
    bench->cleanup = TemplateRegex_cleanup;

    return bench;
}

void TemplateParse_prepare(Benchmark *self) {
    TemplateParseData *data = (TemplateParseData *)self->data;

    data->base.count = (int)Helper_config_i64(self->name, "count");
    prepare_template(&data->base);
}

void TemplateParse_run(Benchmark *self, int iteration_id) {
    (void)iteration_id;
    TemplateParseData *data = (TemplateParseData *)self->data;
    TemplateBaseData *base = &data->base;

    if (!base->text)
        return;

    size_t estimated_size = (size_t)(base->text_size * 1.5) + 1;
    char *result_buf = malloc(estimated_size);
    if (!result_buf)
        return;

    size_t result_pos = 0;
    size_t i = 0;
    size_t len = base->text_size;

    while (i < len) {
        if (i + 1 < len && base->text[i] == '{' && base->text[i + 1] == '{') {
            size_t j = i + 2;
            while (j + 1 < len) {
                if (base->text[j] == '}' && base->text[j + 1] == '}') {
                    break;
                }
                j++;
            }

            if (j + 1 < len) {
                size_t key_len = j - i - 2;
                char *key = malloc(key_len + 1);
                memcpy(key, base->text + i + 2, key_len);
                key[key_len] = '\0';

                char *start = key;
                while (*start == ' ' || *start == '\t')
                    start++;
                char *end = key + key_len - 1;
                while (end > start && (*end == ' ' || *end == '\t'))
                    end--;
                *(end + 1) = '\0';

                const char *value = get_var(base, start);
                if (value) {
                    size_t value_len = strlen(value);
                    memcpy(result_buf + result_pos, value, value_len);
                    result_pos += value_len;
                }

                free(key);
                i = j + 2;
                continue;
            }
        }

        result_buf[result_pos++] = base->text[i];
        i++;
    }

    result_buf[result_pos] = '\0';

    if (base->rendered) {
        free(base->rendered);
    }
    base->rendered = result_buf;
    base->rendered_size = result_pos;
    base->checksum_val += (uint32_t)result_pos;
}

uint32_t TemplateParse_checksum(Benchmark *self) {
    TemplateParseData *data = (TemplateParseData *)self->data;
    TemplateBaseData *base = &data->base;
    return base->checksum_val + Helper_checksum_string(base->rendered);
}

void TemplateParse_cleanup(Benchmark *self) {
    TemplateParseData *data = (TemplateParseData *)self->data;
    TemplateBaseData *base = &data->base;

    if (base->text) {
        free(base->text);
        base->text = NULL;
    }

    if (base->rendered) {
        free(base->rendered);
        base->rendered = NULL;
    }

    clear_vars(base);

    base->text_size = 0;
    base->rendered_size = 0;
    base->checksum_val = 0;
}

Benchmark *TemplateParse_create(void) {
    Benchmark *bench = Benchmark_create("Template::Parse");

    TemplateParseData *data = calloc(1, sizeof(TemplateParseData));

    bench->data = data;
    bench->prepare = TemplateParse_prepare;
    bench->run = TemplateParse_run;
    bench->checksum = TemplateParse_checksum;
    bench->cleanup = TemplateParse_cleanup;

    return bench;
}


typedef struct {
  double x, y, z;
} TRVector;

typedef struct {
  TRVector orig, dir;
} TRRay;

typedef struct {
  double r, g, b;
} TRColor;

typedef struct {
  TRVector center;
  double radius;
  TRColor color;
} TRSphere;

typedef struct {
  TRVector position;
  TRColor color;
} TRLight;

typedef struct {
  int32_t width;
  int32_t height;
  uint32_t result_val;
} TextRaytracerData;

static TRVector tr_vector_scale(TRVector v, double s) {
  return (TRVector){v.x * s, v.y * s, v.z * s};
}

static TRVector tr_vector_add(TRVector a, TRVector b) {
  return (TRVector){a.x + b.x, a.y + b.y, a.z + b.z};
}

static TRVector tr_vector_sub(TRVector a, TRVector b) {
  return (TRVector){a.x - b.x, a.y - b.y, a.z - b.z};
}

static double tr_vector_dot(TRVector a, TRVector b) {
  return a.x * b.x + a.y * b.y + a.z * b.z;
}

static double tr_vector_magnitude(TRVector v) {
  return sqrt(tr_vector_dot(v, v));
}

static TRVector tr_vector_normalize(TRVector v) {
  double mag = tr_vector_magnitude(v);
  if (mag == 0.0)
    return (TRVector){0, 0, 0};
  return tr_vector_scale(v, 1.0 / mag);
}

static TRColor tr_color_scale(TRColor c, double s) {
  return (TRColor){c.r * s, c.g * s, c.b * s};
}

static TRColor tr_color_add(TRColor a, TRColor b) {
  return (TRColor){a.r + b.r, a.g + b.g, a.b + b.b};
}

static TRVector tr_sphere_get_normal(TRSphere *sphere, TRVector pt) {
  return tr_vector_normalize(tr_vector_sub(pt, sphere->center));
}

static double tr_clamp(double x, double a, double b) {
  if (x < a)
    return a;
  if (x > b)
    return b;
  return x;
}

static double tr_intersect_sphere(TRRay ray, TRVector center, double radius) {
  TRVector l = tr_vector_sub(center, ray.orig);
  double tca = tr_vector_dot(l, ray.dir);
  if (tca < 0.0)
    return -1.0;

  double d2 = tr_vector_dot(l, l) - tca * tca;
  double r2 = radius * radius;
  if (d2 > r2)
    return -1.0;

  double thc = sqrt(r2 - d2);
  double t0 = tca - thc;
  if (t0 > 10000.0)
    return -1.0;

  return t0;
}

static TRColor tr_diffuse_shading(TRVector pi, TRSphere *obj, TRLight light) {
  TRVector n = tr_sphere_get_normal(obj, pi);
  TRVector light_dir = tr_vector_normalize(tr_vector_sub(light.position, pi));
  double lam1 = tr_vector_dot(light_dir, n);
  double lam2 = tr_clamp(lam1, 0.0, 1.0);

  TRColor light_color = tr_color_scale(light.color, lam2 * 0.5);
  TRColor obj_color = tr_color_scale(obj->color, 0.3);
  return tr_color_add(light_color, obj_color);
}

static const char LUT[6] = {'.', '-', '+', '*', 'X', 'M'};

void TextRaytracer_prepare(Benchmark *self) {
  TextRaytracerData *data = (TextRaytracerData *)self->data;

  data->width = (int32_t)Helper_config_i64(self->name, "w");
  data->height = (int32_t)Helper_config_i64(self->name, "h");

  if (data->width <= 0)
    data->width = 10;
  if (data->height <= 0)
    data->height = 10;

  data->result_val = 0;
}

void TextRaytracer_run(Benchmark *self, int iteration_id) {
  (void)iteration_id;
  TextRaytracerData *data = (TextRaytracerData *)self->data;

  TRSphere scene[3] = {{{-1.0, 0.0, 3.0}, 0.3, {1.0, 0.0, 0.0}},
                       {{0.0, 0.0, 3.0}, 0.8, {0.0, 1.0, 0.0}},
                       {{1.0, 0.0, 3.0}, 0.4, {0.0, 0.0, 1.0}}};

  TRLight light1 = {{0.7, -1.0, 1.7}, {1.0, 1.0, 1.0}};

  uint32_t iteration_result = 0;

  for (int32_t j = 0; j < data->height; j++) {
    for (int32_t i = 0; i < data->width; i++) {
      double fw = data->width;
      double fh = data->height;
      double fi = i;
      double fj = j;

      TRRay ray;
      ray.orig = (TRVector){0.0, 0.0, 0.0};

      TRVector dir = {(fi - fw / 2.0) / fw, (fj - fh / 2.0) / fh, 1.0};
      ray.dir = tr_vector_normalize(dir);

      double tval = -1.0;
      TRSphere *hit_obj = NULL;

      for (int k = 0; k < 3; k++) {
        double intersect =
            tr_intersect_sphere(ray, scene[k].center, scene[k].radius);
        if (intersect >= 0.0) {
          tval = intersect;
          hit_obj = &scene[k];
          break;
        }
      }

      char pixel = ' ';
      if (hit_obj && tval >= 0.0) {
        TRVector pi = tr_vector_add(ray.orig, tr_vector_scale(ray.dir, tval));
        TRColor color = tr_diffuse_shading(pi, hit_obj, light1);
        double col = (color.r + color.g + color.b) / 3.0;
        int idx = (int)(col * 6.0);
        if (idx < 0)
          idx = 0;
        if (idx >= 6)
          idx = 5;
        pixel = LUT[idx];
      }

      iteration_result += (uint8_t)pixel;
    }
  }

  data->result_val += iteration_result;
}

uint32_t TextRaytracer_checksum(Benchmark *self) {
  TextRaytracerData *data = (TextRaytracerData *)self->data;
  return data->result_val;
}

void TextRaytracer_cleanup(Benchmark *self) { (void)self; }

Benchmark *TextRaytracer_create(void) {
  Benchmark *bench = Benchmark_create("Etc::TextRaytracer");

  TextRaytracerData *data = malloc(sizeof(TextRaytracerData));
  memset(data, 0, sizeof(TextRaytracerData));

  bench->data = data;

  bench->prepare = TextRaytracer_prepare;
  bench->run = TextRaytracer_run;
  bench->checksum = TextRaytracer_checksum;
  bench->cleanup = TextRaytracer_cleanup;

  return bench;
}


typedef struct {
  int64_t words;
  int64_t word_len;
  char *text;
  size_t text_size;
  uint32_t checksum_val;
} WordsData;

static void WordsData_init(WordsData *data) {
  data->words = Helper_config_i64("Etc::Words", "words");
  data->word_len = Helper_config_i64("Etc::Words", "word_len");
  data->text = NULL;
  data->text_size = 0;
  data->checksum_val = 0;
}

void Words_prepare(Benchmark *self) {
  WordsData *data = (WordsData *)self->data;

  if (data->text) {
    free(data->text);
    data->text = NULL;
  }

  const char *chars = "abcdefghijklmnopqrstuvwxyz";
  int char_count = 26;

  char **words_array = malloc(data->words * sizeof(char *));
  size_t total_len = 0;

  for (int i = 0; i < (int)data->words; i++) {
    int len = Helper_next_int((int)data->word_len);
    len += Helper_next_int(3) + 3;
    char *word = malloc(len + 1);
    for (int j = 0; j < len; j++) {
      int idx = Helper_next_int(char_count);
      word[j] = chars[idx];
    }
    word[len] = '\0';
    words_array[i] = word;
    total_len += len + 1;
  }

  char *buffer = malloc(total_len + 1);
  size_t pos = 0;
  for (int i = 0; i < (int)data->words; i++) {
    size_t word_len = strlen(words_array[i]);
    memcpy(buffer + pos, words_array[i], word_len);
    pos += word_len;
    if (i < data->words - 1) {
      buffer[pos++] = ' ';
    }
    free(words_array[i]);
  }
  buffer[pos] = '\0';
  free(words_array);

  data->text = buffer;
  data->text_size = pos;
}

typedef struct {
  char *word;
  int count;
  UT_hash_handle hh;
} WordEntry;

void Words_run(Benchmark *self, int iteration_id) {
  (void)iteration_id;
  WordsData *data = (WordsData *)self->data;

  WordEntry *frequencies = NULL;

  char *text_copy = strdup(data->text);
  char *saveptr;
  char *word = strtok_r(text_copy, " ", &saveptr);

  while (word != NULL) {
    if (strlen(word) >= 3) {
      WordEntry *entry;
      HASH_FIND_STR(frequencies, word, entry);
      if (entry) {
        entry->count++;
      } else {
        entry = malloc(sizeof(WordEntry));
        entry->word = strdup(word);
        entry->count = 1;
        HASH_ADD_STR(frequencies, word, entry);
      }
    }
    word = strtok_r(NULL, " ", &saveptr);
  }

  free(text_copy);

  char max_word[256] = "";
  int max_count = 0;
  WordEntry *entry, *tmp;

  HASH_ITER(hh, frequencies, entry, tmp) {
    if (entry->count > max_count) {
      max_count = entry->count;
      strncpy(max_word, entry->word, 255);
      max_word[255] = '\0';
    }
  }

  int freq_size = HASH_COUNT(frequencies);
  uint32_t word_checksum = Helper_checksum_string(max_word);

  data->checksum_val +=
      (uint32_t)max_count + word_checksum + (uint32_t)freq_size;

  HASH_ITER(hh, frequencies, entry, tmp) {
    HASH_DEL(frequencies, entry);
    free(entry->word);
    free(entry);
  }
}

uint32_t Words_checksum(Benchmark *self) {
  WordsData *data = (WordsData *)self->data;
  return data->checksum_val;
}

void Words_cleanup(Benchmark *self) {
  WordsData *data = (WordsData *)self->data;
  if (data->text) {
    free(data->text);
    data->text = NULL;
  }
  data->text_size = 0;
}

Benchmark *Words_create(void) {
  Benchmark *bench = Benchmark_create("Etc::Words");

  WordsData *data = calloc(1, sizeof(WordsData));
  WordsData_init(data);

  bench->data = data;
  bench->prepare = Words_prepare;
  bench->run = Words_run;
  bench->checksum = Words_checksum;
  bench->cleanup = Words_cleanup;

  return bench;
}

void register_all_benchmarks(void) {
  Benchmark_register("Binarytrees::Obj", BinarytreesObj_create);
  Benchmark_register("Binarytrees::Arena", BinarytreesArena_create);
  Benchmark_register("Brainfuck::Array", BrainfuckArray_create);
  Benchmark_register("Brainfuck::Recursion", BrainfuckRecursion_create);
  Benchmark_register("CLBG::Fannkuchredux", Fannkuchredux_create);
  Benchmark_register("CLBG::Mandelbrot", Mandelbrot_create);
  Benchmark_register("Matmul::Single", Matmul_create);
  Benchmark_register("Matmul::T4", Matmul4T_create);
  Benchmark_register("Matmul::T8", Matmul8T_create);
  Benchmark_register("Matmul::T16", Matmul16T_create);
  Benchmark_register("CLBG::Nbody", Nbody_create);
  Benchmark_register("CLBG::Spectralnorm", Spectralnorm_create);
  Benchmark_register("Base64::Encode", Base64Encode_create);
  Benchmark_register("Base64::Decode", Base64Decode_create);
  Benchmark_register("Json::Generate", JsonGenerate_create);
  Benchmark_register("Json::ParseDom", JsonParseDom_create);
  Benchmark_register("Json::ParseMapping", JsonParseMapping_create);
  Benchmark_register("Etc::Sieve", Sieve_create);
  Benchmark_register("Etc::TextRaytracer", TextRaytracer_create);
  Benchmark_register("Etc::NeuralNet", NeuralNet_create);
  Benchmark_register("Sort::Quick", SortQuick_create);
  Benchmark_register("Sort::Merge", SortMerge_create);
  Benchmark_register("Sort::Self", SortSelf_create);
  Benchmark_register("Graph::BFS", GraphPathBFS_create);
  Benchmark_register("Graph::DFS", GraphPathDFS_create);
  Benchmark_register("Graph::AStar", GraphPathAStar_create);
  Benchmark_register("Hash::SHA256", BufferHashSHA256_create);
  Benchmark_register("Hash::CRC32", BufferHashCRC32_create);
  Benchmark_register("Etc::CacheSimulation", CacheSimulation_create);
  Benchmark_register("Calculator::Ast", CalculatorAst_create);
  Benchmark_register("Calculator::Interpreter", CalculatorInterpreter_create);
  Benchmark_register("Etc::GameOfLife", GameOfLife_create);
  Benchmark_register("Maze::Generator", MazeGenerator_create);
  Benchmark_register("Maze::BFS", MazeBFS_create);
  Benchmark_register("Maze::AStar", MazeAStar_create);
  Benchmark_register("Compress::BWTEncode", BWTEncode_create);
  Benchmark_register("Compress::BWTDecode", BWTDecode_create);
  Benchmark_register("Compress::HuffEncode", HuffEncode_create);
  Benchmark_register("Compress::HuffDecode", HuffDecode_create);
  Benchmark_register("Compress::ArithEncode", ArithEncode_create);
  Benchmark_register("Compress::ArithDecode", ArithDecode_create);
  Benchmark_register("Compress::LZWEncode", LZWEncode_create);
  Benchmark_register("Compress::LZWDecode", LZWDecode_create);
  Benchmark_register("Distance::Jaro", Jaro_create);
  Benchmark_register("Distance::NGram", NGram_create);
  Benchmark_register("Etc::Words", Words_create);
  Benchmark_register("Etc::LogParser", LogParser_create);
  Benchmark_register("Template::Regex", TemplateRegex_create);
  Benchmark_register("Template::Parse", TemplateParse_create);
  Benchmark_register("CSV::Parse", CsvParse_create);
}

int main(int argc, char *argv[]) {
  struct timespec start_time;
  clock_gettime(CLOCK_REALTIME, &start_time);
  printf("start: %ld\n",
         start_time.tv_sec * 1000 + start_time.tv_nsec / 1000000);

  static const char config[] =
    "[\n"
    "  {\n"
    "    \"name\": \"Binarytrees::Obj\",\n"
    "    \"checksum\": 37748700,\n"
    "    \"depth\": 19,\n"
    "    \"iterations\": 30\n"
    "  },\n"
    "  {\n"
    "    \"name\": \"Binarytrees::Arena\",\n"
    "    \"checksum\": 113246181,\n"
    "    \"depth\": 21,\n"
    "    \"iterations\": 23\n"
    "  },\n"
    "  {\n"
    "    \"name\": \"Brainfuck::Array\",\n"
    "    \"checksum\": 954437102,\n"
    "    \"program\": \">++[<+++++++++++++>-]<[[>+>+<<-]>[<+>-]++++++++[>++++++++<-]>.[-]<<>++++++++++[>++++++++++[>++++++++++[>++++++++++[>++++++++++[>++++++++++[>++++++++++[-]<-]<-]<-]<-]<-]<-]<-]++++++++++.\",\n"
    "    \"warmup_program\": \"++++++++++[>+++++++>++++++++++>+++>+<<<<-]>++.>+.+++++++..+++.>++.<<+++++++++++++++.>.+++.------.--------.>+.>.\",\n"
    "    \"warmup_iterations\": 1000,\n"
    "    \"iterations\": 1\n"
    "  },\n"
    "  {\n"
    "    \"name\": \"Brainfuck::Recursion\",\n"
    "    \"checksum\": 954437102,\n"
    "    \"program\": \">++[<+++++++++++++>-]<[[>+>+<<-]>[<+>-]++++++++[>++++++++<-]>.[-]<<>++++++++++[>++++++++++[>++++++++++[>++++++++++[>++++++++++[>++++++++++[>++++++++++[-]<-]<-]<-]<-]<-]<-]<-]++++++++++.\",\n"
    "    \"warmup_program\": \"++++++++++[>+++++++>++++++++++>+++>+<<<<-]>++.>+.+++++++..+++.>++.<<+++++++++++++++.>.+++.------.--------.>+.>.\",\n"
    "    \"warmup_iterations\": 1000,\n"
    "    \"iterations\": 1\n"
    "  },\n"
    "  {\n"
    "    \"name\": \"Matmul::Single\",\n"
    "    \"checksum\": 2636317752,\n"
    "    \"n\": 543,\n"
    "    \"iterations\": 10\n"
    "  },\n"
    "  {\n"
    "    \"name\": \"Matmul::T4\",\n"
    "    \"checksum\": 3406759060,\n"
    "    \"n\": 850,\n"
    "    \"iterations\": 10\n"
    "  },\n"
    "  {\n"
    "    \"name\": \"Matmul::T8\",\n"
    "    \"checksum\": 2387257924,\n"
    "    \"n\": 1051,\n"
    "    \"iterations\": 10\n"
    "  },\n"
    "  {\n"
    "    \"name\": \"Matmul::T16\",\n"
    "    \"checksum\": 1960278388,\n"
    "    \"n\": 1265,\n"
    "    \"iterations\": 10\n"
    "  },\n"
    "  {\n"
    "    \"name\": \"Base64::Encode\",\n"
    "    \"checksum\": 3208476047,\n"
    "    \"size\": 2700000,\n"
    "    \"iterations\": 5000\n"
    "  },\n"
    "  {\n"
    "    \"name\": \"Base64::Decode\",\n"
    "    \"checksum\": 2829431115,\n"
    "    \"size\": 530000,\n"
    "    \"iterations\": 4000\n"
    "  },\n"
    "  {\n"
    "    \"name\": \"Json::Generate\",\n"
    "    \"checksum\": 120,\n"
    "    \"coords\": 5500,\n"
    "    \"iterations\": 100\n"
    "  },\n"
    "  {\n"
    "    \"name\": \"Json::ParseDom\",\n"
    "    \"checksum\": 4157826592,\n"
    "    \"coords\": 11000,\n"
    "    \"iterations\": 100\n"
    "  },\n"
    "  {\n"
    "    \"name\": \"Json::ParseMapping\",\n"
    "    \"checksum\": 4157826592,\n"
    "    \"coords\": 11000,\n"
    "    \"iterations\": 100\n"
    "  },\n"
    "  {\n"
    "    \"name\": \"CSV::Parse\",\n"
    "    \"checksum\": 2039510507,\n"
    "    \"rows\": 43000,\n"
    "    \"iterations\": 70\n"
    "  },\n"
    "  {\n"
    "    \"name\": \"Etc::Sieve\",\n"
    "    \"checksum\": 1305051852,\n"
    "    \"limit\": 3500000,\n"
    "    \"iterations\": 290\n"
    "  },\n"
    "  {\n"
    "    \"name\": \"Etc::TextRaytracer\",\n"
    "    \"checksum\": 2070855360,\n"
    "    \"w\": 800,\n"
    "    \"h\": 800,\n"
    "    \"iterations\": 75\n"
    "  },\n"
    "  {\n"
    "    \"name\": \"Etc::NeuralNet\",\n"
    "    \"checksum\": 144809111,\n"
    "    \"iterations\": 1650\n"
    "  },\n"
    "  {\n"
    "    \"name\": \"Etc::CacheSimulation\",\n"
    "    \"checksum\": 3382401640,\n"
    "    \"values\": 5500,\n"
    "    \"size\": 2600,\n"
    "    \"iterations\": 5500\n"
    "  },\n"
    "  {\n"
    "    \"name\": \"Etc::GameOfLife\",\n"
    "    \"checksum\": 3596232155,\n"
    "    \"w\": 440,\n"
    "    \"h\": 440,\n"
    "    \"iterations\": 440\n"
    "  },\n"
    "  {\n"
    "    \"name\": \"Etc::Words\",\n"
    "    \"checksum\": 712245858,\n"
    "    \"words\": 50000,\n"
    "    \"word_len\": 4,\n"
    "    \"iterations\": 225\n"
    "  },\n"
    "  {\n"
    "    \"name\": \"Etc::LogParser\",\n"
    "    \"checksum\": 67092,\n"
    "    \"iterations\": 10,\n"
    "    \"lines_count\": 1200\n"
    "  },\n"
    "  {\n"
    "    \"name\": \"Template::Regex\",\n"
    "    \"checksum\": 3403329035,\n"
    "    \"iterations\": 20,\n"
    "    \"count\": 50000\n"
    "  },\n"
    "  {\n"
    "    \"name\": \"Template::Parse\",\n"
    "    \"checksum\": 3305896679,\n"
    "    \"iterations\": 60,\n"
    "    \"count\": 56000\n"
    "  },\n"
    "  {\n"
    "    \"name\": \"Sort::Quick\",\n"
    "    \"checksum\": 58455215,\n"
    "    \"size\": 370000,\n"
    "    \"iterations\": 50\n"
    "  },\n"
    "  {\n"
    "    \"name\": \"Sort::Merge\",\n"
    "    \"checksum\": 61652378,\n"
    "    \"size\": 395000,\n"
    "    \"iterations\": 50\n"
    "  },\n"
    "  {\n"
    "    \"name\": \"Sort::Self\",\n"
    "    \"checksum\": 63111306,\n"
    "    \"size\": 240000,\n"
    "    \"iterations\": 53\n"
    "  },\n"
    "  {\n"
    "    \"name\": \"Graph::BFS\",\n"
    "    \"checksum\": 42000,\n"
    "    \"vertices\": 500000,\n"
    "    \"jumps\": 20,\n"
    "    \"jump_len\": 1000,\n"
    "    \"iterations\": 34\n"
    "  },\n"
    "  {\n"
    "    \"name\": \"Graph::DFS\",\n"
    "    \"checksum\": 24660,\n"
    "    \"vertices\": 500000,\n"
    "    \"jumps\": 20,\n"
    "    \"jump_len\": 1000,\n"
    "    \"iterations\": 17\n"
    "  },\n"
    "  {\n"
    "    \"name\": \"Graph::AStar\",\n"
    "    \"checksum\": 3001890,\n"
    "    \"vertices\": 1200000,\n"
    "    \"jumps\": 20,\n"
    "    \"jump_len\": 27,\n"
    "    \"iterations\": 25\n"
    "  },\n"
    "  {\n"
    "    \"name\": \"Hash::SHA256\",\n"
    "    \"checksum\": 2682354976,\n"
    "    \"size\": 1100000,\n"
    "    \"iterations\": 1200\n"
    "  },\n"
    "  {\n"
    "    \"name\": \"Hash::CRC32\",\n"
    "    \"checksum\": 610363760,\n"
    "    \"size\": 1200000,\n"
    "    \"iterations\": 150\n"
    "  },\n"
    "  {\n"
    "    \"name\": \"Calculator::Ast\",\n"
    "    \"checksum\": 3196123703,\n"
    "    \"operations\": 45500,\n"
    "    \"iterations\": 26\n"
    "  },\n"
    "  {\n"
    "    \"name\": \"Calculator::Interpreter\",\n"
    "    \"checksum\": 3726790644,\n"
    "    \"operations\": 70000,\n"
    "    \"iterations\": 45\n"
    "  },\n"
    "  {\n"
    "    \"name\": \"Maze::Generator\",\n"
    "    \"checksum\": 3738188578,\n"
    "    \"w\": 1400,\n"
    "    \"h\": 1400,\n"
    "    \"iterations\": 23\n"
    "  },\n"
    "  {\n"
    "    \"name\": \"Maze::BFS\",\n"
    "    \"checksum\": 6218187,\n"
    "    \"w\": 1400,\n"
    "    \"h\": 1400,\n"
    "    \"iterations\": 160\n"
    "  },\n"
    "  {\n"
    "    \"name\": \"Maze::AStar\",\n"
    "    \"checksum\": 3734646,\n"
    "    \"w\": 1400,\n"
    "    \"h\": 1400,\n"
    "    \"iterations\": 93\n"
    "  },\n"
    "  {\n"
    "    \"name\": \"CLBG::Fannkuchredux\",\n"
    "    \"checksum\": 124125696,\n"
    "    \"n\": 8,\n"
    "    \"iterations\": 640\n"
    "  },\n"
    "  {\n"
    "    \"name\": \"CLBG::Mandelbrot\",\n"
    "    \"checksum\": 2142473783,\n"
    "    \"w\": 400,\n"
    "    \"h\": 400,\n"
    "    \"iterations\": 115\n"
    "  },\n"
    "  {\n"
    "    \"name\": \"CLBG::Nbody\",\n"
    "    \"checksum\": 338797504,\n"
    "    \"iterations\": 14000\n"
    "  },\n"
    "  {\n"
    "    \"name\": \"CLBG::Spectralnorm\",\n"
    "    \"checksum\": 1052198219,\n"
    "    \"size\": 1200,\n"
    "    \"iterations\": 145\n"
    "  },\n"
    "  {\n"
    "    \"name\": \"Compress::BWTEncode\",\n"
    "    \"checksum\": 5100000,\n"
    "    \"size\": 100000,\n"
    "    \"iterations\": 43\n"
    "  },\n"
    "  {\n"
    "    \"name\": \"Compress::BWTDecode\",\n"
    "    \"checksum\": 252100252,\n"
    "    \"size\": 1000001,\n"
    "    \"iterations\": 210\n"
    "  },\n"
    "  {\n"
    "    \"name\": \"Compress::HuffEncode\",\n"
    "    \"checksum\": 84682260,\n"
    "    \"size\": 1000002,\n"
    "    \"iterations\": 270\n"
    "  },\n"
    "  {\n"
    "    \"name\": \"Compress::HuffDecode\",\n"
    "    \"checksum\": 984102952,\n"
    "    \"size\": 1000003,\n"
    "    \"iterations\": 820\n"
    "  },\n"
    "  {\n"
    "    \"name\": \"Compress::ArithEncode\",\n"
    "    \"checksum\": 14079312,\n"
    "    \"size\": 100004,\n"
    "    \"iterations\": 460\n"
    "  },\n"
    "  {\n"
    "    \"name\": \"Compress::ArithDecode\",\n"
    "    \"checksum\": 42102100,\n"
    "    \"size\": 100005,\n"
    "    \"iterations\": 350\n"
    "  },\n"
    "  {\n"
    "    \"name\": \"Compress::LZWEncode\",\n"
    "    \"checksum\": 300528,\n"
    "    \"size\": 200006,\n"
    "    \"iterations\": 60\n"
    "  },\n"
    "  {\n"
    "    \"name\": \"Compress::LZWDecode\",\n"
    "    \"checksum\": 3888100000,\n"
    "    \"size\": 12000000,\n"
    "    \"iterations\": 270\n"
    "  },\n"
    "  {\n"
    "    \"name\": \"Distance::Jaro\",\n"
    "    \"checksum\": 625538544,\n"
    "    \"iterations\": 860,\n"
    "    \"count\": 1000,\n"
    "    \"size\": 50\n"
    "  },\n"
    "  {\n"
    "    \"name\": \"Distance::NGram\",\n"
    "    \"checksum\": 267408,\n"
    "    \"iterations\": 90,\n"
    "    \"count\": 10,\n"
    "    \"size\": 15000\n"
    "  }\n"
    "]\n"
  ;

  Helper_load_config(config);
  register_all_benchmarks();

  const char *single_bench = argc > 1 ? argv[1] : NULL;
  Benchmark_all(single_bench);

  Helper_free_config();

  if (benchmark_factories) {
    free(benchmark_factories);
    benchmark_factories = NULL;
    benchmark_factories_count = 0;
    benchmark_factories_capacity = 0;
  }

  FILE *f = fopen("/tmp/recompile_marker", "w");
  if (f) {
    fprintf(f, "RECOMPILE_MARKER_0");
    fclose(f);
  }

  return 0;
}
