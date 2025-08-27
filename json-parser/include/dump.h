#pragma once
#include <stdio.h>
#include "value.h"

void dump_json(const Value *v, FILE *out);
void dump_json_compact(const Value *v, FILE *out);