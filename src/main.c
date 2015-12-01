#include "pebble.h"
#include "utils.h"
#include "main.h"


static Window *s_window;
static Layer *s_hands_layer, *s_info_layer;

static void info_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  
  //setting background
  #ifdef PBL_COLOR
    graphics_context_set_fill_color(ctx, GColorMalachite);
    graphics_fill_rect(ctx, GRect(bounds.origin.x + 2, bounds.origin.y, 32, bounds.size.h), 0, GCornerNone);
  
    graphics_context_set_fill_color(ctx, GColorDarkGreen);
    graphics_fill_rect(ctx, GRect(bounds.origin.x + 3, bounds.origin.y, 2, bounds.size.h ), 0, GCornerNone);
  
    
  
  #else
  
  #endif
  
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, GRect(bounds.origin.x, bounds.origin.y, 2, bounds.size.h), 0, GCornerNone);
  
}  



static void hands_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  GPoint center = grect_center_point(&bounds);
  
  const int16_t max_hand_length = bounds.size.w / 2 - 1;
  
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  
  graphics_context_set_stroke_color(ctx, GColorWhite);
  
  // ******************* hour hand
  int32_t angle = (TRIG_MAX_ANGLE * (((t->tm_hour % 12) * 6) + (t->tm_min / 10))) / (12 * 6);
  
  GPoint hand_endpoint = {
    .x = (int16_t)(sin_lookup(angle) * (int32_t)(max_hand_length-15) / TRIG_MAX_RATIO) + center.x,
    .y = (int16_t)(-cos_lookup(angle) * (int32_t)(max_hand_length-15) / TRIG_MAX_RATIO) + center.y,
  };
  
  // using separte approach for APLITE to compensate for set_stroke_width; remove it after aplite gets OS3
  #ifdef PBL_PLATFORM_APLITE
    graphics_draw_line2(ctx, center, hand_endpoint, 3);
  #else
    graphics_context_set_stroke_width(ctx, 3);
    graphics_draw_line(ctx, center, hand_endpoint);
  #endif
  
  // ******************** minute hand
  angle = TRIG_MAX_ANGLE * t->tm_min / 60;
  
  hand_endpoint.x = (int16_t)(sin_lookup(angle) * (int32_t)max_hand_length / TRIG_MAX_RATIO) + center.x;
  hand_endpoint.y = (int16_t)(-cos_lookup(angle) * (int32_t)max_hand_length / TRIG_MAX_RATIO) + center.y;
  
  // using separte approach for APLITE to compensate for set_stroke_width; remove it after aplite gets OS3
  #ifdef PBL_PLATFORM_APLITE
    graphics_draw_line2(ctx, center, hand_endpoint, 2);
  #else
    graphics_context_set_stroke_width(ctx, 2);
    graphics_draw_line(ctx, center, hand_endpoint);
  #endif
  
 
  // ***************** second hand
  angle = TRIG_MAX_ANGLE * t->tm_sec / 60;
  
  hand_endpoint.x = (int16_t)(sin_lookup(angle) * (int32_t)max_hand_length / TRIG_MAX_RATIO) + center.x;
  hand_endpoint.y = (int16_t)(-cos_lookup(angle) * (int32_t)max_hand_length / TRIG_MAX_RATIO) + center.y;
  
  // using separte approach for APLITE to compensate for set_stroke_width; remove it after aplite gets OS3
  #ifdef PBL_PLATFORM_APLITE
    graphics_draw_line2(ctx, center, hand_endpoint, 1);
  #else
    graphics_context_set_stroke_color(ctx, GColorMalachite);
    graphics_context_set_stroke_width(ctx, 1);
    graphics_draw_line(ctx, center, hand_endpoint);
  #endif
  
  // first circle in the middle
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_circle(ctx, center, 5);
  

  // second circle in the middle
  #ifdef PBL_COLOR
    graphics_context_set_stroke_color(ctx, GColorDarkGreen);
  #else
    graphics_context_set_stroke_color(ctx, GColorBlack);
  #endif
  graphics_draw_circle(ctx, center, 3);
  

  
  
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
}

static void init() {
  s_window = window_create();
  window_set_background_color(s_window, GColorBlack);
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(s_window, true);

  tick_timer_service_subscribe(SECOND_UNIT, handle_second_tick);
}

static void deinit() {
  tick_timer_service_unsubscribe();
  window_destroy(s_window);
}

int main() {
  init();
  app_event_loop();
  deinit();
}

