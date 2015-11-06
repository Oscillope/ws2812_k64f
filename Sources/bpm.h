#ifndef BPM_H
#define BPM_H
#include "fsl_gpio_driver.h"
#include "fsl_pit_driver.h"
#include "board.h"

enum bpm_modes {
	BPM_MODE_RUN,
	BPM_MODE_LEARN,
	BPM_MODE_STOP
};

struct bpm_dev {
	void (*callback)(void *data);
	enum bpm_modes mode;
};

#endif /* BPM_H */
