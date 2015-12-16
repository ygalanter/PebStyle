#include <pebble.h>
#include "utils.h"

#ifdef PBL_COLOR
//sets battery color indicator according to percentage
void change_battery_color(int8_t battery_percentage) {

    switch (battery_state_service_peek().charge_percent) {
       case 100: 
       case 90: 
       case 80: 
       case 70: 
       case 60: 
       case 50: color_battery_major = GColorJaegerGreen; color_battery_minor = GColorDarkGreen; break;
       case 40: 
       case 30: 
       case 20: color_battery_major = GColorOrange; color_battery_minor = GColorDarkCandyAppleRed; break;
       case 10: 
       case 0:  color_battery_major = GColorDarkCandyAppleRed; color_battery_minor = GColorBulgarianRose; break;
   }
    
}
#endif

