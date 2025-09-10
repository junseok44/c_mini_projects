#ifndef SIG_H
#define SIG_H
#include "common.h"

void ignore_sigint_in_parent(void);
void restore_default_sigint(void);

#endif /* SIG_H */
