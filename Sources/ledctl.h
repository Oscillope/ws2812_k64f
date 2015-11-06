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
	rgb *profile;
	int len;
	int idx;
};

struct led_array {
	struct led_pixel *leds;
	rgb *buffer;
	int num_leds;
	int speed;
};

void ledctl_init(rgb *super_buffer, int speed);
void ledctl_update(void);

#endif /* LEDCTL_H */
