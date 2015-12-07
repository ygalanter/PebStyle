#include <pebble.h>
#include "utils.h"

#ifdef PBL_SDK_2
// from: https://github.com/SeaPea/Draw: To substitute set_stroke_width - won't be needed when Aplite get OS3
void graphics_draw_line2(GContext *ctx, GPoint p0, GPoint p1, int8_t width) {
  // Order points so that lower x is first
  int16_t x0, x1, y0, y1;
  if (p0.x <= p1.x) {
    x0 = p0.x; x1 = p1.x; y0 = p0.y; y1 = p1.y;
  } else {
    x0 = p1.x; x1 = p0.x; y0 = p1.y; y1 = p0.y;
  }
  
  // Init loop variables
  int16_t dx = x1-x0;
  int16_t dy = abs(y1-y0);
  int16_t sy = y0<y1 ? 1 : -1; 
  int16_t err = (dx>dy ? dx : -dy)/2;
  int16_t e2;
  
  // Calculate whether line thickness will be added vertically or horizontally based on line angle
  int8_t xdiff, ydiff;
  
  if (dx > dy) {
    xdiff = 0;
    ydiff = width/2;
  } else {
    xdiff = width/2;
    ydiff = 0;
  }
  
  // Use Bresenham's integer algorithm, with slight modification for line width, to draw line at any angle
  while (true) {
    // Draw line thickness at each point by drawing another line 
    // (horizontally when > +/-45 degrees, vertically when <= +/-45 degrees)
    graphics_draw_line(ctx, GPoint(x0-xdiff, y0-ydiff), GPoint(x0+xdiff, y0+ydiff));
    
    if (x0==x1 && y0==y1) break;
    e2 = err;
    if (e2 >-dx) { err -= dy; x0++; }
    if (e2 < dy) { err += dx; y0 += sy; }
  }
}
#endif

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

