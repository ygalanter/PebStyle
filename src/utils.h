#include <pebble.h>
#pragma once

GColor color_battery_major, color_battery_minor; 

#ifdef PBL_COLOR
void change_battery_color(int8_t battery_percentage); //sets battery color indicator according to percentage
#endif
