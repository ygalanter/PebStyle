#include "pebble.h"
#include "utils.h"
#include "main.h"


static Window *s_window;
static Layer *s_hands_layer, *s_info_layer;
static GFont font_18, font_24, font_27;
static GBitmap *bluetooth_sprite = NULL;

char s_dow[] = "WED   "; //test  
char s_day[] = "05"; //test  
char s_month[] = "OCT   "; //test  
char s_battery[] = "100"; //test 
char s_time[] = "12:34"; //test 

static void battery_handler(BatteryChargeState state) {
  #ifdef PBL_COLOR
  change_battery_color(state.charge_percent);
  #endif
  
  layer_mark_dirty(s_info_layer);
  
}

void bluetooth_handler(bool connected) {
  
   if (bluetooth_sprite != NULL) {
     gbitmap_destroy(bluetooth_sprite);
     bluetooth_sprite = NULL;
   }
  
  if (connected) {
    bluetooth_sprite = gbitmap_create_with_resource(RESOURCE_ID_BT_CONNECTED);
  } else {
    bluetooth_sprite = gbitmap_create_with_resource(RESOURCE_ID_BT_DISCONNECTED);
  }
  
  layer_mark_dirty(s_hands_layer);
}  

static void info_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  
  {//***************************** setting background *****************************
  #ifdef PBL_COLOR
    graphics_context_set_fill_color(ctx, color_battery_major);
    graphics_fill_rect(ctx, GRect(bounds.origin.x + 2, bounds.origin.y, 32, bounds.size.h), 0, GCornerNone);
  
    graphics_context_set_fill_color(ctx, color_battery_minor);
    graphics_fill_rect(ctx, GRect(bounds.origin.x + 2, bounds.origin.y, 2, bounds.size.h ), 0, GCornerNone);
  #else
  
  #endif
  
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, GRect(bounds.origin.x, bounds.origin.y, 2, bounds.size.h), 0, GCornerNone);
  }//***************************** setting background *****************************
  
  {// *************************displaying date ********************
  time_t temp = time(NULL);
  struct tm *t = localtime(&temp);
  
  graphics_context_set_text_color(ctx, GColorWhite);
  
  // day of the week
  strftime(s_dow, sizeof(s_dow), "%a", t);
  graphics_draw_text(ctx, s_dow, font_18, GRect(bounds.origin.x + 4,bounds.origin.y, 28, 20), GTextOverflowModeFill, GTextAlignmentCenter, NULL);
  
  // day of the month
  strftime(s_day, sizeof(s_day), "%d", t);
  graphics_draw_text(ctx, s_day, font_27, GRect(bounds.origin.x + 4,bounds.origin.y + 16, 28, 25), GTextOverflowModeFill, GTextAlignmentCenter, NULL);
  
  // month
  strftime(s_month, sizeof(s_month), "%b", t);
  graphics_draw_text(ctx, s_month, font_18, GRect(bounds.origin.x + 4,bounds.origin.y + 16 + 28, 28, 20), GTextOverflowModeFill, GTextAlignmentCenter, NULL);
  }//************************* displaying date ********************
  
  {//***************************** displaying battery *****************************
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, GRect(bounds.origin.x + 6, bounds.size.h/2 - 7, 24, 14), 1, GCornersAll);
  
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, GRect(bounds.origin.x + 6 + 2, bounds.size.h/2 - 7 + 1, 24 - 4, 14 - 2), 0, GCornerNone);  
    
  BatteryChargeState battry_state = battery_state_service_peek();
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, GRect(bounds.origin.x + 6 + 2 + 1, bounds.size.h/2 - 7 + 1 + 1, (24 - 4 - 2) * battry_state.charge_percent /100, 14 - 2 - 2), 0, GCornerNone);    
  
  // battery text
  snprintf(s_battery, sizeof(s_battery), "%d", battry_state.charge_percent);  
  graphics_draw_text(ctx, s_battery, font_24, GRect(bounds.origin.x + 4,bounds.size.h/2 + 8, 28, 20), GTextOverflowModeFill, GTextAlignmentCenter, NULL);  
    
    
  }//***************************** displaying battery *****************************  
  
  {//***************************** drawing separators *****************************
  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_draw_line(ctx, GPoint(bounds.origin.x + 6, bounds.origin.y + 69), GPoint(bounds.origin.x + 4 + 25, bounds.origin.y + 69));
  graphics_draw_line(ctx, GPoint(bounds.origin.x + 6, bounds.origin.y + 123), GPoint(bounds.origin.x + 4 + 25, bounds.origin.y + 123));  
    
  }//***************************** drawing separators *****************************  
  
}  

static void hands_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  GPoint center = grect_center_point(&bounds);
  
  const int16_t max_hand_length = bounds.size.w / 2 - 2;
  
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  
  {//************************ drawing hands ****************************
  graphics_context_set_stroke_color(ctx, GColorWhite);
  
  // ******************* hour hand
  int32_t angle = (TRIG_MAX_ANGLE * (((t->tm_hour % 12) * 6) + (t->tm_min / 10))) / (12 * 6);
  
  GPoint hand_endpoint = {
    .x = (int16_t)(sin_lookup(angle) * (int32_t)(max_hand_length-15) / TRIG_MAX_RATIO) + center.x,
    .y = (int16_t)(-cos_lookup(angle) * (int32_t)(max_hand_length-15) / TRIG_MAX_RATIO) + center.y,
  };
  
  // using separte approach for PBL_SDK_2 to compensate for set_stroke_width
  #ifdef PBL_SDK_2
    graphics_draw_line2(ctx, center, hand_endpoint, 3);
  #else
    graphics_context_set_stroke_width(ctx, 3);
    graphics_draw_line(ctx, center, hand_endpoint);
  #endif
   
  // ******************** minute hand
  angle = TRIG_MAX_ANGLE * t->tm_min / 60;
  
  hand_endpoint.x = (int16_t)(sin_lookup(angle) * (int32_t)max_hand_length / TRIG_MAX_RATIO) + center.x;
  hand_endpoint.y = (int16_t)(-cos_lookup(angle) * (int32_t)max_hand_length / TRIG_MAX_RATIO) + center.y;
  
  // using separte approach for PBL_SDK_2 to compensate for set_stroke_width
  #ifdef PBL_SDK_2
    graphics_draw_line2(ctx, center, hand_endpoint, 2);
  #else
    graphics_context_set_stroke_width(ctx, 2);
    graphics_draw_line(ctx, center, hand_endpoint);
  #endif
 
  // ***************** second hand
  angle = TRIG_MAX_ANGLE * t->tm_sec / 60;
  
  hand_endpoint.x = (int16_t)(sin_lookup(angle) * (int32_t)max_hand_length / TRIG_MAX_RATIO) + center.x;
  hand_endpoint.y = (int16_t)(-cos_lookup(angle) * (int32_t)max_hand_length / TRIG_MAX_RATIO) + center.y;
  
  // using separte approach for PBL_SDK_2 to compensate for set_stroke_width
  #ifdef PBL_SDK_2
    graphics_draw_line2(ctx, center, hand_endpoint, 1);
  #else
    #ifdef PBL_COLOR
    graphics_context_set_stroke_color(ctx, color_battery_major);
    #endif
    graphics_context_set_stroke_width(ctx, 1);
    graphics_draw_line(ctx, center, hand_endpoint);
  #endif
  
  // first circle in the middle
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_circle(ctx, center, 5);
  

  // second circle in the middle
  #ifdef PBL_COLOR
    graphics_context_set_stroke_color(ctx, color_battery_minor);
  #else
    graphics_context_set_stroke_color(ctx, GColorBlack);
  #endif
  graphics_draw_circle(ctx, center, 3);
  }//************************ drawing hands **************************** 
 
  {//************************ drawing text time **************************** 
   char format[5];
     
   // building format 12h/24h
   if (clock_is_24h_style()) {
      strcpy(format, "%H:%M"); // e.g "14:46"
   } else {
      strcpy(format, "%l:%M"); // e.g " 2:46" -- with leading space
   }
   
   strftime(s_time, sizeof(s_time), format, t);
   graphics_draw_text(ctx, s_time, font_24, GRect(bounds.origin.x,bounds.size.h - 27 , bounds.size.w, 30), GTextOverflowModeFill, GTextAlignmentCenter, NULL); 
      
  
  }//************************ drawing text time **************************** 
  
  {//************************ drawing bluetooth **************************** 
  graphics_draw_bitmap_in_rect(ctx, bluetooth_sprite, GRect(bounds.size.w/2 - 8, 3, 16, 22));
  }//************************ drawing bluetooth ****************************   
  
}

static void handle_second_tick(struct tm *tick_time, TimeUnits units_changed) {
  layer_mark_dirty(s_hands_layer);
}

static void window_load(Window *window) {
  
  Layer *window_layer = window_get_root_layer(window);
  GRect top_bound = layer_get_bounds(window_layer);
  
  //layer with hands
  s_hands_layer = layer_create(GRect(top_bound.origin.x, top_bound.origin.y, top_bound.size.w - 32, top_bound.size.h));
  layer_set_update_proc(s_hands_layer, hands_update_proc);
  layer_add_child(window_layer, s_hands_layer);
    
  //layer with info
  s_info_layer = layer_create(GRect(top_bound.size.w - 32, 0, 32, top_bound.size.h));
  layer_set_update_proc(s_info_layer, info_update_proc);
  layer_add_child(window_layer, s_info_layer);
  
  
}

static void window_unload(Window *window) {
  layer_destroy(s_hands_layer);
  layer_destroy(s_info_layer);
  
  if (bluetooth_sprite) {
     gbitmap_destroy(bluetooth_sprite);
     bluetooth_sprite = NULL;
   }
}

static void init() {
  
  font_18 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_BIG_NOODLE_TILTING_18));
  font_24 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_BIG_NOODLE_TILTING_24));
  font_27 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_BIG_NOODLE_TILTING_27));
  
  s_window = window_create();
  window_set_background_color(s_window, GColorBlack);
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(s_window, true);
  
  bluetooth_connection_service_subscribe(bluetooth_handler);
  bluetooth_handler(bluetooth_connection_service_peek());
  
  battery_state_service_subscribe(battery_handler);
  battery_handler(battery_state_service_peek());
  
  tick_timer_service_subscribe(SECOND_UNIT, handle_second_tick);
  
}

static void deinit() {
  bluetooth_connection_service_unsubscribe();
  battery_state_service_unsubscribe();
  tick_timer_service_unsubscribe();
  window_destroy(s_window);
  
  fonts_unload_custom_font(font_18);
  fonts_unload_custom_font(font_24);
  fonts_unload_custom_font(font_27);
}

int main() {
  init();
  app_event_loop();
  deinit();
}

