#ifndef BUTTONS_H
#define BUTTONS_H

enum button_cb_type {
	BUTTON_CB_BPM,
	BUTTON_CB_SIGL,
	BUTTON_CB_SIG,
	BUTTON_CB_SIGR,
	BUTTON_CB_BTN1,
	BUTTON_CB_BTN2,
	BUTTON_CB_SW2,
	BUTTON_CB_SW3,
	NUM_BTN_CB
};
void buttons_reg_callback(void (*callback)(void), int button, enum button_cb_type type);
void buttons_do_deferred(void);
void buttons_init(void);

#endif /* BUTTONS_H */
