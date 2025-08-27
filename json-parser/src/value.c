#include <stddef.h>
#include <stdio.h>
#include <value.h>
#include <stdlib.h>
#include <string.h>


Value *jv_new(void) {
    Value *value = malloc(sizeof(Value));
    if (value) {
        *value = (Value){0};
    }
    return value;
}

void jv_free(Value *v) {
    if (!v) return;

    switch(v->kind) {
        case JV_OBJECT:
            for(size_t i=0; i<v->u.obj.len; i++) {
                if (v ->u.obj.items[i].key.is_owned) {
                    free((void *) v ->u.obj.items[i].key.ptr);
                }
                jv_free(v->u.obj.items[i].value);
            }
            free(v->u.obj.items);
            break;
        case JV_ARRAY:
            for (size_t i=0; i < v->u.arr.len; i++) {
                jv_free(v->u.arr.items[i]);
            }
            free(v->u.arr.items);
            break;
        case JV_STRING:
            if (v->u.str.is_owned)
                free((void *)v->u.str.ptr);
            break;
        case JV_NUMBER:    
        case JV_FALSE:
        case JV_TRUE:
        case JV_NULL:
        default:
            break;
    }
    free(v);
}

Value *jv_copy(const Value *v) {
    
    if (!v) return NULL;  // 처음에 추가

    Value *newValue = malloc(sizeof(Value));

    if (newValue == NULL) {
        free(newValue);
        return NULL;
    } else {
        *newValue = (Value){0};
    }
    
    newValue->kind = v->kind;

    switch (v->kind) {
        case JV_ARRAY:
            newValue->u.arr.len = v->u.arr.len;
            newValue->u.arr.cap = v->u.arr.cap;
            
            newValue->u.arr.items = malloc(sizeof(Value *) * v->u.arr.len);

            if (!newValue->u.arr.items) {
                free(newValue);
                return NULL;
            }

            for (size_t i=0; i<v->u.arr.len; i++) {
                newValue->u.arr.items[i] = jv_copy(v->u.arr.items[i]);
                if (!newValue->u.arr.items[i]) {
                    for (size_t q=0; q<=i; q++) {
                        jv_free(newValue->u.arr.items[q]);
                    }
                    free(newValue ->u.arr.items);
                    free(newValue);
                    return NULL;
                }
            }

            break;

        case JV_OBJECT:
            newValue->u.obj.cap = v->u.obj.cap;
            newValue->u.obj.len = v->u.obj.len;

            newValue->u.obj.items = malloc(sizeof(JMember) * v->u.obj.len);

            if (!newValue->u.obj.items) {
                free(newValue);
                return NULL;
            }

            for (size_t i = 0; i < v->u.obj.len; i++) {
                JMember *src = &v->u.obj.items[i];
                JMember *dst = &newValue->u.obj.items[i];
                
                // 키 깊은 복사
                if (src->key.is_owned) {
                    // 원본이 소유하는 키면 새로 할당
                    char *new_key = malloc(src->key.len + 1);

                    // 실패시 전부 롤백.
                    if (!new_key) {
                        for (size_t j = 0; j<i; j++) {
                            free((void *) newValue->u.obj.items[j].key.ptr);
                            jv_free(newValue->u.obj.items[j].value);
                        }
                        free(newValue->u.obj.items);
                        jv_free(newValue);
                        return NULL;
                    }

                    memcpy(new_key, src->key.ptr, src->key.len);
                    new_key[src->key.len] = '\0';
                    
                    dst->key.ptr = new_key;
                    dst->key.len = src->key.len;
                    dst->key.is_owned = 1;
                } else {
                    // 원본이 소유하지 않으면 참조만 복사
                    dst->key = src->key;
                }
                
                // 값 재귀 복사
                dst->value = jv_copy(src->value);
                if (!dst->value) {
                    jv_free(newValue);
                    return NULL;
                }
            }

            break;
        case JV_STRING: {
            char *new_ptr = malloc(v->u.str.len + 1);
            if (!new_ptr) {
                return NULL;
            }
            memcpy(new_ptr, v->u.str.ptr, v->u.str.len);
            new_ptr[v->u.str.len] = '\0';

            newValue->u.str.ptr = new_ptr;
            newValue->u.str.len = v->u.str.len;
            newValue->u.str.is_owned = 1;
            break;
        }
        case JV_NUMBER:
            newValue->u.num = v->u.num;
            break;
        default:
            break;
    }

    return newValue;
}

Value *jv_get_object(const Value *v, const char *key) {
    if (!v || v->kind != JV_OBJECT) {
        return NULL;
    }

    for (int i=0; i<v->u.obj.len; i++) {
        if (strcmp(v->u.obj.items[i].key.ptr, key) == 0) {
            return v->u.obj.items[i].value;
        }
    }

    return NULL;
}

Value *jv_get_array(const Value *v, size_t index) {
    if (!v || v->kind != JV_ARRAY) {
        return NULL;
    }

    // 올바른 길이 사용
    if (index >= v->u.arr.len) {  // >= 사용 (>= 가 맞음)
        return NULL;
    }

    return v->u.arr.items[index];
}

const char *jv_get_string(const Value *v, size_t *len) {
    if (!v || v->kind != JV_STRING) {
        return NULL;
    }

    if (v->u.str.len <= *len) {
        return v->u.str.ptr;
    }

    // len 길이보다 길다? 그럼 잘라야.
    
    char *sliced = malloc(sizeof(char) * (*len) + 1);
    strncpy(sliced, v->u.str.ptr, *len);
    sliced[*len] = '\0';

    return sliced;
}

double jv_get_number(const Value *v) {
    if (!v || v->kind != JV_NUMBER) {
        return 0.0;
    }

    return v->u.num;
}