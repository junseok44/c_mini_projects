#ifndef SIG_H
#define SIG_H

void ignore_sigint_in_parent(void);
void restore_default_sigint(void);

#endif /* SIG_H */
