#ifndef BPM_H
#define BPM_H

enum bpm_modes {
	BPM_MODE_RUN,
	BPM_MODE_LEARN,
	BPM_MODE_STOP
};

struct bpm_dev {
	void (*callback)(void);
	enum bpm_modes mode;
	unsigned rate;
	unsigned div;
};

void bpm_update_div(int div);
void bpm_button_callback(void);
void bpm_init(void (*callback)(void));

#endif /* BPM_H */
