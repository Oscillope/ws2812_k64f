#ifndef LEDCTL_H
#define LEDCTL_H

#define NUM_LEDS 30

typedef struct {
	unsigned char r;
	unsigned char g;
	unsigned char b;
} rgb;

typedef struct {
	unsigned char h;
	unsigned char s;
	unsigned char v;
} hsv;

struct led_array {
	rgb leds[NUM_LEDS][256];
	int len;
	int idx;
	rgb *buffer;
	int num_leds;
};

void ledctl_init(rgb *super_buffer);
void ledctl_update(void);
void ledctl_make_swoosh(void);
void ledctl_make_flasher(int dir);

#endif /* LEDCTL_H */
