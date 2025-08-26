#ifndef VALUE_H
#define VALUE_H

#include <stddef.h>

typedef struct Value Value;

typedef struct {
    const char *ptr;
    size_t len;
    int is_owned;
} JString;

typedef struct {
    JString key;
    Value *value;
} JMember;

typedef struct {
    JMember *items;
    size_t len;
    size_t cap;
} JObject;

typedef struct {
    struct Value **items;
    size_t len;
    size_t cap;
} JArray;

typedef enum {
    JV_OBJECT,
    JV_ARRAY,
    JV_STRING,
    JV_NUMBER,
    JV_TRUE,
    JV_FALSE,
    JV_NULL,
} ValueKind;

typedef struct Value {
    ValueKind kind;
    union {
        JObject obj;
        JArray arr;
        JString str;
        double num;
    } u;
} Value;

Value *jv_new(void);

void jv_free(Value *v);

Value *jv_copy(const Value *v);

Value *jv_get_object(const Value *v, const char *key);

Value *jv_get_array(const Value *v, size_t index);

const char *jv_get_string(const Value *v, size_t *len);

double jv_get_number(const Value *v);

#endif