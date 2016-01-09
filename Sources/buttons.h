#ifndef BUTTONS_H
#define BUTTONS_H

void buttons_reg_callback(void (*callback)(void), int button);
void buttons_do_deferred(void);
void buttons_init(void);

#endif /* BUTTONS_H */
