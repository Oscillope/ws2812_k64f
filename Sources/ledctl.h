#ifndef LEDCTL_H
#define LEDCTL_H

#define NUM_LEDS 30
#define NUM_RAINBOW_STATES 2
#define NUM_COLORS 6

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

const rgb colors[] = {
		{0xff, 0x00, 0x00},	//Red
		{0x00, 0xff, 0x00},	//Green
		{0x00, 0x00, 0xff},	//Blue
		{0x66, 0x00, 0xff},	//Purple
		{0x00, 0x55, 0xff},	//Cyan
		{0xff, 0x55, 0x00}, //Amber
};

struct led_array {
	rgb leds[NUM_LEDS][256];
	int len;
	int idx;
	rgb *buffer;
	int num_leds;
};

void ledctl_init(rgb *super_buffer);
void ledctl_update(void);
void ledctl_make_swoosh(int state);
void ledctl_make_flasher(int dir);
void ledctl_make_cylon(rgb color);
void ledctl_strobe(rgb color);

#endif /* LEDCTL_H */
