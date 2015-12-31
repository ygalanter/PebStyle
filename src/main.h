#pragma once
#include "pebble.h"

// Digital time
#define KEY_MAIN_CLOCK  0

  #define MAIN_CLOCK_ANALOG 0
  #define MAIN_CLOCK_DIGITAL  1

// Second hand
#define KEY_SECOND_HAND  1

  #define SECOND_HAND_DISABLED 0
  #define SECOND_HAND_ENABLED 1

// Bluetooth
#define KEY_BLUETOOTH_ALERT 2

  #define BLUETOOTH_ALERT_DISABLED 0  
  #define BLUETOOTH_ALERT_SILENT 1
  #define BLUETOOTH_ALERT_WEAK 2
  #define BLUETOOTH_ALERT_NORMAL 3
  #define BLUETOOTH_ALERT_STRONG 4
  #define BLUETOOTH_ALERT_DOUBLE 5

// weather
#define KEY_WEATHER_CODE 3
#define KEY_WEATHER_TEMP 4
#define KEY_WEATHER_INTERVAL 5
#define KEY_JSREADY 6 
#define KEY_LOCATION_SERVICE 7

#define LOCATION_AUTOMATIC  0  
  #define LOCATION_MANUAL  1
  #define LOCATION_DISABLED  2

#define KEY_TEMPERATURE_FORMAT 8
#define KEY_CITY_NAME 9
#define KEY_SECONDARY_INFO_TYPE 10
#define KEY_TIMEZONE_NAME 11

  #define SECONDARY_INFO_DISABLED 3
  #define SECONDARY_INFO_CURRENT_TIME 1
  #define SECONDARY_INFO_CURRENT_LOCATION 2
  #define SECONDARY_INFO_CURRENT_MILITARY_TIME 4
  //otherwise - timezone offset

#define KEY_TIME_SEPARATOR 12
  
  #define TIME_SEPARATOR_COLON 0
  #define TIME_SEPARATOR_DOT 1

#define KEY_JS_TIMEZONE_OFFSET 13

#define KEY_SIDEBAR_LOCATION 14
  
  #define SIDEBAR_LOCATION_RIGHT 0
  #define SIDEBAR_LOCATION_LEFT 1

#define KEY_COLOR_SELECTION 15

  #define COLOR_SELECTION_AUTOMATIC 0
  #define COLOR_SELECTION_CUSTOM 1

#define KEY_MAIN_BG_COLOR 16
#define KEY_MAIN_COLOR 17
#define KEY_SIDEBAR_BG_COLOR 18
#define KEY_SIDEBAR_COLOR 19

#define KEY_BLUETOOTH_ICON 20

  #define BLUETOOTH_ICON_ALWAYS_VISIBLE 0
  #define BLUETOOTH_ICON_HIDDEN_WHEN_CONNECTED 1
  #define BLUETOOTH_ICON_ALWAYS_HIDDEN 2

#define KEY_AMPM_TEXT 21


// bluetooth vibe patterns
const VibePattern VIBE_PATTERN_WEAK = {
	.durations = (uint32_t []) {100},
	.num_segments = 1
};

const VibePattern VIBE_PATTERN_NORMAL = {
	.durations = (uint32_t []) {300},
	.num_segments = 1
};

const VibePattern VIBE_PATTERN_STRONG = {
	.durations = (uint32_t []) {500},
	.num_segments = 1
};

const VibePattern VIBE_PATTERN_DOUBLE = {
	.durations = (uint32_t []) {500,100,500},
	.num_segments = 3
};
