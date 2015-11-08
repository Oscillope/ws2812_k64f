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

struct led_pixel {
	rgb profile[256];
	int len;
	int idx;
};

struct led_array {
	struct led_pixel leds[NUM_LEDS];
	rgb *buffer;
	int num_leds;
};

void ledctl_init(rgb *super_buffer);
void ledctl_update(void);

#endif /* LEDCTL_H */
