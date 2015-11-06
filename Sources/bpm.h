#ifndef BPM_H
#define BPM_H
#include "fsl_hwtimer.h"
#include "board.h"

struct bpm_dev {
	void (*callback)(void *data);
	hwtimer_t hwtimer;
};

#endif /* BPM_H */
