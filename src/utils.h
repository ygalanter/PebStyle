#include <pebble.h>
#pragma once

GColor color_battery_major, color_battery_minor; 

void graphics_draw_line2(GContext *ctx, GPoint p0, GPoint p1, int8_t width); //draws line of arbitray thinkness on SDK2
void change_battery_color(int8_t battery_percentage); //sets battery color indicator according to percentage
