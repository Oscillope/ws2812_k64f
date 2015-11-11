#include <stdio.h>
#include <stdlib.h>
#include "fsl_gpio_driver.h"
#include "board.h"
#include "ledctl.h"
#include "bpm.h"
static struct led_array array;

/*
 * RGB <-> HSV functions taken from:
 * http://stackoverflow.com/questions/3018313/algorithm-to-convert-rgb-to-hsv-and-hsv-to-rgb-in-range-0-255-for-both
 */

hsv rgb_to_hsv(rgb rgb)
{
    hsv hsv;
    unsigned char rgbMin, rgbMax;

    rgbMin = rgb.r < rgb.g ? (rgb.r < rgb.b ? rgb.r : rgb.b) : (rgb.g < rgb.b ? rgb.g : rgb.b);
    rgbMax = rgb.r > rgb.g ? (rgb.r > rgb.b ? rgb.r : rgb.b) : (rgb.g > rgb.b ? rgb.g : rgb.b);

    hsv.v = rgbMax;
    if (hsv.v == 0)
    {
        hsv.h = 0;
        hsv.s = 0;
        return hsv;
    }

    hsv.s = 255 * (long)(rgbMax - rgbMin) / hsv.v;
    if (hsv.s == 0)
    {
        hsv.h = 0;
        return hsv;
    }

    if (rgbMax == rgb.r)
        hsv.h = 0 + 43 * (rgb.g - rgb.b) / (rgbMax - rgbMin);
    else if (rgbMax == rgb.g)
        hsv.h = 85 + 43 * (rgb.b - rgb.r) / (rgbMax - rgbMin);
    else
        hsv.h = 171 + 43 * (rgb.r - rgb.g) / (rgbMax - rgbMin);

    return hsv;
}

rgb hsv_to_rgb(hsv hsv)
{
    rgb rgb;
    unsigned char region, remainder, p, q, t;

    if (hsv.s == 0)
    {
        rgb.r = hsv.v;
        rgb.g = hsv.v;
        rgb.b = hsv.v;
        return rgb;
    }

    region = hsv.h / 43;
    remainder = (hsv.h - (region * 43)) * 6;

    p = (hsv.v * (255 - hsv.s)) >> 8;
    q = (hsv.v * (255 - ((hsv.s * remainder) >> 8))) >> 8;
    t = (hsv.v * (255 - ((hsv.s * (255 - remainder)) >> 8))) >> 8;

    switch (region)
    {
        case 0:
            rgb.r = hsv.v; rgb.g = t; rgb.b = p;
            break;
        case 1:
            rgb.r = q; rgb.g = hsv.v; rgb.b = p;
            break;
        case 2:
            rgb.r = p; rgb.g = hsv.v; rgb.b = t;
            break;
        case 3:
            rgb.r = p; rgb.g = q; rgb.b = hsv.v;
            break;
        case 4:
            rgb.r = t; rgb.g = p; rgb.b = hsv.v;
            break;
        default:
            rgb.r = hsv.v; rgb.g = p; rgb.b = q;
            break;
    }

    return rgb;
}

void ledctl_make_fade(rgb *dest, hsv color1, hsv color2, int num_steps, int phase)
{
	unsigned char step;
	unsigned char hue = color1.h + phase;
	int i;
	step = (color1.h - color2.h) / num_steps;
	printf("\nMade fade %d in %d steps %d phase\r\n", step, num_steps, phase);
	for(i = num_steps; i > 0; i--) {
		hue += step;
		dest[i - 1] = hsv_to_rgb((hsv){hue, color1.s, color1.v});
	}
	for(i = num_steps; i < num_steps << 1; i++) {
		hue -= step;
		dest[i - 1] = hsv_to_rgb((hsv){hue, color1.s, color1.v});
	}
}

void ledctl_make_swoosh(void)
{
	int i;
	for(i = 0; i < array.num_leds; i++) {
		ledctl_make_fade(array.leds[i], (hsv){255, 255, 255}, (hsv){0, 255, 255}, 64, i * 12);
	}
	array.len = 64;
	bpm_update_div(64);
}

void ledctl_make_flasher(int dir)
{
	int i;
	if(dir > 0) {
		for(i = 0; i < (NUM_LEDS >> 2); i++) {
			array.leds[i][0] = (rgb){0xff, 0x55, 0x00};
			array.leds[i][1] = (rgb){0x00, 0x00, 0x00};
		}
		for(; i < NUM_LEDS; i++) {
			array.leds[i][0] = (rgb){0xff, 0xff, 0xff};
			array.leds[i][1] = (rgb){0xff, 0xff, 0xff};
		}
		array.len = 2;
	} else if (dir < 0) {
		for(i = 0; i < (3 * (NUM_LEDS >> 2)); i++) {
			array.leds[i][0] = (rgb){0xff, 0xff, 0xff};
			array.leds[i][1] = (rgb){0xff, 0xff, 0xff};
		}
		for(; i < NUM_LEDS; i++) {
			array.leds[i][0] = (rgb){0xff, 0x55, 0x00};
			array.leds[i][1] = (rgb){0x00, 0x00, 0x00};
		}
		array.len = 2;
	} else {
		for(i = 0; i < NUM_LEDS; i++) {
			array.leds[i][0] = (rgb){0xff, 0xff, 0xff};
		}
		array.len = 1;
	}
	bpm_update_div(1);
}

void ledctl_update(void)
{
	int i;
	for(i = 0; i < array.num_leds; i++) {
		array.buffer[i] = array.leds[i][array.idx];
	}
	array.idx++;
	//if (i == 0)
	//	printf("\nCurrent buffer: %d %d %d, idx %d\r\n", array.buffer[i].r, array.buffer[i].g, array.buffer[i].b, array.leds[i].idx);
	if (array.idx >= array.len) {
		array.idx = 0;
	}
	GPIO_DRV_TogglePinOutput(kGpioLED1);
}

void ledctl_init(rgb *super_buffer)
{
	array.buffer = super_buffer;
	array.num_leds = NUM_LEDS;
}
