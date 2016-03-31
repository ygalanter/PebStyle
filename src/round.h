//********************** Round Implementation of CobbleStyle watchface

#ifndef PBL_RECT

#pragma once
#include "utils.h"
#include "main.h"
#include "gbitmap_color_palette_manipulator.h"
#include "fctx.h"
#include "ffont.h"
#include "fpath.h"
#include "health.h"

//weather icons
#define ICON_WIDTH  21
#define ICON_HEIGHT 17


//**************** RADIAL TEXT STUFF
static struct Globals {
	Window* window;
	Layer* layer;
	
  FFont* font;
  GFont gfont; 
  FPoint center;
  
  int hour;
  int minute;

  GRect bounds;
  
} g;

//moved here to free stack
fixed_t s;
fixed_t t;
const char* p;
fixed_t arc;
fixed_t r;
char ch;
fixed_t rotation; 
FGlyph* glyph;
FContext fctx;

/* TAU = 2*PI * FIXED_POINT_SCALE * TRIG_MAX_ANGLE
   325 is the largest number you can multiply by TAU without overflow.
*/
static const int32_t TAU = 6588397;

uint32_t string_width_radial(
    FContext* fctx,
    const char* text,
    FFont* font,
    int16_t font_size,
    FPoint center,
    uint16_t radius,
    bool clockwise)
{
    fctx_set_text_size(fctx, font, font_size);
    s = radius * TAU / TRIG_MAX_ANGLE;
    t = s * fctx->transform_scale_from.x / fctx->transform_scale_to.x;
    arc = 0;
    for (p = text; *p; ++p) {
        ch = *p;
        glyph = ffont_glyph_info(font, ch);
        if (glyph && p!=text) {
            arc += glyph->horiz_adv_x * (clockwise? 1 : -1);
        }
    }
    return arc * TRIG_MAX_ANGLE / t;
}




void draw_string_radial(
    FContext* fctx,
    const char* text,
    FFont* font,
    int16_t font_size,
    FPoint center,
    uint16_t radius,
    uint32_t angle,
    bool clockwise)
  
  
{
    //correcting angle for half string width
    angle -= string_width_radial(fctx,text,font,font_size, center, radius, clockwise)/2;
  
    fctx_set_text_size(fctx, font, font_size);
    fctx_set_offset(fctx, center);
    fctx_set_rotation(fctx, 0);

    s = radius * TAU / TRIG_MAX_ANGLE;
    t = s * fctx->transform_scale_from.x / fctx->transform_scale_to.x;
    r = INT_TO_FIXED(radius) * fctx->transform_scale_from.x / fctx->transform_scale_to.x;
    arc = 0;
    rotation = 0; // YG moved here so final angle can be returned


    for (p = text; *p; ++p) {
        ch = *p;
        glyph = ffont_glyph_info(font, ch);
        if (glyph) {

            if (p != text) {
                arc += glyph->horiz_adv_x / 2;
            }
            rotation = angle + arc * TRIG_MAX_ANGLE / t * (clockwise? 1 : -1);
            fctx_set_rotation(fctx, rotation);
            arc += glyph->horiz_adv_x / 2;

            FPoint advance;
            advance.x = glyph->horiz_adv_x / -2;
            advance.y = clockwise? r : -(r + font->ascent);;

            void* path_data = ffont_glyph_outline(font, glyph);
            fctx_draw_commands(fctx, advance, path_data, glyph->path_data_length);
        }
    }
  
 
}
//**************** RADIAL TEXT STUFF




static Window *s_window;
static Layer *s_main_layer;
static GFont font_18, font_24, font_27, font_90;
static GBitmap *bluetooth_sprite = NULL;

char s_date[] = "WED 19 DEC      "; 

char s_battery[] = "100";
char s_time[] = "PAR 88:44 PM";
char s_second_info[25], s_health_info[8];;
char s_timezone_name[25];
char s_temp[6];
char s_ampm_text[] = "123456789012345678901234567890";

int flag_main_clock, flag_second_hand, flag_bluetooth_alert, flag_locationService, flag_weatherInterval, flag_secondary_info_type;
int flag_time_separator, flag_js_timezone_offset, flag_sidebar_location, flag_color_selection, flag_bluetooth_icon;
GColor flag_main_bg_color, flag_main_color, flag_sidebar_bg_color, flag_sidebar_color;
bool is_bluetooth_buzz_enabled = false, flag_messaging_is_busy = false, flag_js_is_ready = false;;
int msg_result;

GBitmap *meteoicons_all, *meteoicon_current, *sneaker;;
GRect top_bound; 
time_t temp;
struct tm *tm_time;

// raised from health.c "health_handler" event handler
void update_health_info() {
  if (health_is_available()) {
      snprintf(s_health_info, sizeof(s_health_info), "%d",  health_get_metric_sum(HealthMetricStepCount));
  } else {
      snprintf(s_health_info, sizeof(s_health_info), "N/A");
  }
}

//calling for weather update
static void update_weather() {
  // Only grab the weather if we can talk to phone AND weather is enabled AND currently message is not being processed and JS on phone is ready
  if (flag_locationService != LOCATION_DISABLED && bluetooth_connection_service_peek() && !flag_messaging_is_busy && flag_js_is_ready) {
    //APP_LOG(//APP_LOG_LEVEL_INFO, "**** I am inside 'update_weather()' about to request weather from the phone ***");
    
    //need to have some data - sending dummy
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);  
    Tuplet dictionary[] = {
      TupletInteger(0, 0),
    };
    dict_write_tuplet(iter, &dictionary[0]);
    
     flag_messaging_is_busy = true;
     msg_result = app_message_outbox_send(); // need to assign result for successfull call
     //APP_LOG(//APP_LOG_LEVEL_INFO, "**** I am inside 'update_weather()' message sent and result code = %d***", msg_result);
  } 
}

// showing temp
static void set_temperature(int w_current) {
    //APP_LOG(//APP_LOG_LEVEL_INFO, "**** I am inside 'show_temperature()'; TEMP in Pebble: %d", w_current);
    snprintf(s_temp, sizeof(s_temp), "%i\u00B0", w_current);
    layer_mark_dirty(s_main_layer);
}

//showing weather icon
static void set_weather_icon(int w_icon) {
   
   if (w_icon < 0 || w_icon > 47) {w_icon = 48;}  // in case icon not available - show "N/A" icon

   if (meteoicon_current)  gbitmap_destroy(meteoicon_current);
   meteoicon_current = gbitmap_create_as_sub_bitmap(meteoicons_all, GRect(0, ICON_HEIGHT*w_icon, ICON_WIDTH, ICON_HEIGHT)); 
   //APP_LOG(//APP_LOG_LEVEL_INFO, "**** I am inside 'set_weather_icon'; Icon IS: %d", w_icon);
   layer_mark_dirty(s_main_layer);
}

static void battery_handler(BatteryChargeState state) {
  
  // on color watches change color of sidebar and second hand according battery health (*when not in custom color mode*)
  if (flag_color_selection == COLOR_SELECTION_AUTOMATIC) {
    change_battery_color(state.charge_percent);
  }  
  
  layer_mark_dirty(s_main_layer);
  
}

void bluetooth_handler(bool connected) {
  
  if (connected){ // on bluetooth reconnect - update weather
    //APP_LOG(//APP_LOG_LEVEL_INFO, "***** I am inside of 'bluetooth_handler()' about to call 'update_weather();");
    update_weather();
  } 
  
  if (bluetooth_sprite != NULL) {
     gbitmap_destroy(bluetooth_sprite);
     bluetooth_sprite = NULL;
   }
  
  // if Bluetooth alert is totally disabled - exit from here
  if (flag_bluetooth_alert == BLUETOOTH_ALERT_DISABLED) return;  
  
  if (connected) {
    
      if (flag_bluetooth_icon == BLUETOOTH_ICON_ALWAYS_VISIBLE)  { // only display "connected" icon if it's not "hide when coonected" or "always hide"
          bluetooth_sprite = gbitmap_create_with_resource(RESOURCE_ID_BT_CONNECTED);
          if (flag_color_selection == COLOR_SELECTION_CUSTOM) { // in custom color mode colorin bitmaps as well
             replace_gbitmap_color(GColorWhite, flag_main_color, bluetooth_sprite, NULL);
          }
      }  
  } else {
    if (flag_bluetooth_icon != BLUETOOTH_ICON_ALWAYS_HIDDEN)  { // only display "disconnected" icon if it's either "always visible" or "hide when conected"
      bluetooth_sprite = gbitmap_create_with_resource(RESOURCE_ID_BT_DISCONNECTED);
    }  
  }
  
  layer_mark_dirty(s_main_layer);
  
  // if this is initial load OR bluetooth alert is silent return without buzz
  if (flag_bluetooth_alert == BLUETOOTH_ALERT_SILENT || is_bluetooth_buzz_enabled == false) return;
    
  switch (flag_bluetooth_alert){
    case BLUETOOTH_ALERT_WEAK:
      vibes_enqueue_custom_pattern(VIBE_PATTERN_WEAK);
      break;
    case BLUETOOTH_ALERT_NORMAL:
      vibes_enqueue_custom_pattern(VIBE_PATTERN_NORMAL);
      break;
    case BLUETOOTH_ALERT_STRONG:
    vibes_enqueue_custom_pattern(VIBE_PATTERN_STRONG);
      break;
    case BLUETOOTH_ALERT_DOUBLE:
      vibes_enqueue_custom_pattern(VIBE_PATTERN_DOUBLE);
      break;    
  }
  
  
}  

static void info_update_proc(Layer *layer, GContext *ctx) {
  
  {//***************************** setting background *****************************
  graphics_context_set_fill_color(ctx, flag_main_bg_color);
  graphics_fill_radial(ctx, top_bound, GOvalScaleModeFillCircle, 90, 0, TRIG_MAX_ANGLE);
    
  graphics_context_set_fill_color(ctx, flag_main_color);
  graphics_fill_radial(ctx, top_bound, GOvalScaleModeFillCircle, 34, 0, TRIG_MAX_ANGLE);
  
  graphics_context_set_fill_color(ctx, color_battery_minor);
  graphics_fill_radial(ctx, top_bound, GOvalScaleModeFillCircle, 31, 0, TRIG_MAX_ANGLE);
    
  graphics_context_set_fill_color(ctx, color_battery_major);
  graphics_fill_radial(ctx, top_bound, GOvalScaleModeFillCircle, 29, 0, TRIG_MAX_ANGLE);

  }//***************************** setting background *****************************
  
  {// *************************displaying date ********************
  temp = time(NULL);
  tm_time = localtime(&temp);
  
  graphics_context_set_text_color(ctx, flag_main_color);
  strftime(s_date, sizeof(s_date), "%a %d %b", tm_time);
  graphics_draw_text(ctx, s_date, font_18, GRect(top_bound.size.w/2 - 50 ,top_bound.size.h/2 + 20, 100, 20), GTextOverflowModeFill, GTextAlignmentCenter, NULL); 
  
  }//************************* displaying date ********************
  
  {//***************************** displaying battery *****************************
  graphics_context_set_text_color(ctx, flag_sidebar_color);  
  graphics_context_set_fill_color(ctx, flag_sidebar_color);
  graphics_fill_rect(ctx, GRect(top_bound.size.w - 26, top_bound.size.h/2 - 12, 20, 12), 1, GCornersAll);
  
  graphics_context_set_fill_color(ctx, color_battery_major);
  graphics_fill_rect(ctx, GRect(top_bound.size.w - 26 + 2, top_bound.size.h/2 - 12 + 1, 20 - 4, 12 - 2), 0, GCornerNone);  
    
  BatteryChargeState battry_state = battery_state_service_peek();
  graphics_context_set_fill_color(ctx, flag_sidebar_color);
  graphics_fill_rect(ctx, GRect(top_bound.size.w - 26 + 2 + 1, top_bound.size.h/2 - 12 + 1 + 1, (20 - 4 - 2) * battry_state.charge_percent /100, 12 - 2 - 2), 0, GCornerNone);    
  
  // battery text
  snprintf(s_battery, sizeof(s_battery), "%d", battry_state.charge_percent);  
  graphics_draw_text(ctx, s_battery, font_18, GRect(top_bound.size.w - 30, top_bound.size.h/2 - 1, 28, 30), GTextOverflowModeFill, GTextAlignmentCenter, NULL);  
    
    
  }//***************************** displaying battery *****************************  
  
  {//***************************** displaying weather *****************************
  if (flag_locationService != LOCATION_DISABLED) {
    
    graphics_context_set_compositing_mode(ctx, GCompOpSet);
    if (meteoicon_current) graphics_draw_bitmap_in_rect(ctx, meteoicon_current, GRect(top_bound.origin.x + 7, top_bound.size.h/2 - ICON_HEIGHT + 1, ICON_WIDTH, ICON_HEIGHT));
    graphics_draw_text(ctx, s_temp, font_18, GRect(top_bound.origin.x + 4, top_bound.size.h/2, 28, 20), GTextOverflowModeFill, GTextAlignmentCenter, NULL);
    
  }
  }//***************************** displaying weather *****************************
  
  
}  

static void hands_update_proc(Layer *layer, GContext *ctx) {
  GPoint center = grect_center_point(&top_bound);
  
   {//************************ drawing bluetooth **************************** 
  if (bluetooth_sprite != NULL) {
    graphics_context_set_compositing_mode(ctx, GCompOpSet);
    graphics_draw_bitmap_in_rect(ctx, bluetooth_sprite, GRect(top_bound.size.w/2 - 8, 43, 16, 22));
  }  
  }//************************ drawing bluetooth ****************************   
  
  const int16_t max_hand_length = top_bound.size.w / 2 - 43;
  
  fctx_init_context(&fctx, ctx);
  fctx_set_color_bias(&fctx, 0);
  
  temp = time(NULL);
  tm_time = localtime(&temp);
  
  if (flag_main_clock == MAIN_CLOCK_ANALOG) { // only displaying analog time if large digial is not enabled
  {//************************ drawing hands ****************************
  
    // ******************* hour hand
    int32_t angle = (TRIG_MAX_ANGLE * (((tm_time->tm_hour % 12) * 6) + (tm_time->tm_min / 10))) / (12 * 6);
  
    GPoint hand_endpoint = {
      .x = (int16_t)(sin_lookup(angle) * (int32_t)(max_hand_length-15) / TRIG_MAX_RATIO) + center.x,
      .y = (int16_t)(-cos_lookup(angle) * (int32_t)(max_hand_length-15) / TRIG_MAX_RATIO) + center.y,
    };
    
    //color under hand
    graphics_context_set_stroke_color(ctx, flag_main_bg_color);
    graphics_context_set_stroke_width(ctx, 4);
    graphics_draw_line(ctx, center, hand_endpoint);

    //hand
    graphics_context_set_stroke_color(ctx, flag_main_color);
    graphics_context_set_stroke_width(ctx, 2);
    graphics_draw_line(ctx, center, hand_endpoint);
     
    // ******************** minute hand
    angle = TRIG_MAX_ANGLE * tm_time->tm_min / 60;
    
    hand_endpoint.x = (int16_t)(sin_lookup(angle) * (int32_t)max_hand_length / TRIG_MAX_RATIO) + center.x;
    hand_endpoint.y = (int16_t)(-cos_lookup(angle) * (int32_t)max_hand_length / TRIG_MAX_RATIO) + center.y;
    
    //color under hand
    graphics_context_set_stroke_color(ctx, flag_main_bg_color);
    graphics_context_set_stroke_width(ctx, 4);
    graphics_draw_line(ctx, center, hand_endpoint);

    //hand
    graphics_context_set_stroke_color(ctx, flag_main_color);
    graphics_context_set_stroke_width(ctx, 2);
    graphics_draw_line(ctx, center, hand_endpoint);
    
    graphics_context_set_stroke_width(ctx, 1); //resetting for second hand and inner circle
   
    // ***************** second hand
    if (flag_second_hand == SECOND_HAND_ENABLED) {  
      angle = TRIG_MAX_ANGLE * tm_time->tm_sec / 60;
      
      hand_endpoint.x = (int16_t)(sin_lookup(angle) * (int32_t)max_hand_length / TRIG_MAX_RATIO) + center.x;
      hand_endpoint.y = (int16_t)(-cos_lookup(angle) * (int32_t)max_hand_length / TRIG_MAX_RATIO) + center.y;
      
      graphics_context_set_stroke_color(ctx, color_battery_major);  
      graphics_draw_line(ctx, center, hand_endpoint);
    } 
    
    // first circle in the middle
    graphics_context_set_fill_color(ctx, flag_main_color);
    graphics_fill_circle(ctx, center, 5);
    
    // second circle in the middle
    graphics_context_set_stroke_color(ctx, color_battery_minor);  

    graphics_draw_circle(ctx, center, 3);
    }//************************ drawing hands **************************** 
  }
 
  
   char format[5];
   char large_ampm[] = "am";


   graphics_context_set_text_color(ctx, flag_main_color);
     
  {//************************ drawing large text time **************************** 
  
   if (flag_main_clock == MAIN_CLOCK_DIGITAL) { // only displaying large text time if main mode is digital
     
     // building format 12h/24h
     if (clock_is_24h_style()) {
        strcpy(format, "%H:%M"); // e.g "14:46"
     } else {
        strcpy(format, "%l:%M"); // e.g " 2:46" -- with leading space
        strftime(large_ampm, sizeof(large_ampm), "%P", tm_time);
        graphics_draw_text(ctx, large_ampm, font_18, GRect(top_bound.size.w/2 + 12 , top_bound.origin.y + 43 , 40, 30), GTextOverflowModeFill, GTextAlignmentLeft, NULL);
      }
     
     if (flag_time_separator == TIME_SEPARATOR_DOT) format[2] = '.';
     strftime(s_time, sizeof(s_time), format, tm_time);

     // if time begin with space - eliminate it for even centering
     if (s_time[0] == ' ') {
       memcpy(&s_time[0], &s_time[1], strlen(s_time));
     }
     
     graphics_draw_text(ctx, s_time, font_90, GRect(top_bound.origin.x,top_bound.origin.y + 51 , top_bound.size.w, 70), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL); 
   }
    
   }//************************ drawing large text time ****************************  
  

  // if custom am/pm text is passed - draw it instead of am/pm
     if (s_ampm_text[0] !='\0') {
       
       fctx_begin_fill(&fctx);
        fctx_set_fill_color(&fctx, flag_sidebar_color); 
        draw_string_radial(&fctx,
              s_ampm_text,
              g.font,  (strlen(s_ampm_text) <=18 ? 22 : 16),
              g.center, g.bounds.size.w / 2 - (strlen(s_ampm_text) <=14 ? 25 : 23) , TRIG_MAX_ANGLE, true);
        fctx_end_fill(&fctx);
     }
  
  
   {//************************ drawing secondary info line **************************** 
     
   switch (flag_secondary_info_type)  {
     case SECONDARY_INFO_DISABLED:
       break;
     case SECONDARY_INFO_HEALTH_STEP_COUNTER:
     
       if (flag_color_selection == COLOR_SELECTION_CUSTOM) { // in custom color mode colorin bitmaps as well
          replace_gbitmap_color(GColorBlack, flag_sidebar_color, sneaker, NULL);
       }
       graphics_context_set_compositing_mode(ctx, GCompOpSet);
       graphics_draw_bitmap_in_rect(ctx, sneaker, GRect(top_bound.origin.x + 33, top_bound.size.h/2 + 51, 20, 9)); 
     
        fctx_begin_fill(&fctx);
        fctx_set_fill_color(&fctx, flag_sidebar_color); 
        draw_string_radial(&fctx,
              s_health_info,
              g.font,  22,
              g.center, g.bounds.size.w / 2 - 26, TRIG_MAX_ANGLE, false);
        fctx_end_fill(&fctx);
     
       break;
     case SECONDARY_INFO_CURRENT_LOCATION_STREET_LEVEL:
     case SECONDARY_INFO_CURRENT_LOCATION_TOWN_LEVEL:
     case SECONDARY_INFO_CURRENT_LOCATION_COUNTRY_LEVEL:
        fctx_begin_fill(&fctx);
        fctx_set_fill_color(&fctx, flag_sidebar_color); 
        draw_string_radial(&fctx,
              s_second_info,
              g.font,  22,
              g.center, g.bounds.size.w / 2 - 26, TRIG_MAX_ANGLE, false);
        fctx_end_fill(&fctx);
    
        break;
     case SECONDARY_INFO_CURRENT_MILITARY_TIME:
     case SECONDARY_INFO_CURRENT_TIME:
       // building format 12h/24h/military
       if (clock_is_24h_style() || flag_secondary_info_type == SECONDARY_INFO_CURRENT_MILITARY_TIME) {
          strcpy(format, "%H:%M"); // e.g "14:46"
       } else {
          strcpy(format, "%l:%M %P"); // e.g " 2:46 PM" -- with leading space
       }
     
       if (flag_time_separator == TIME_SEPARATOR_DOT) format[2] = '.';
   
       strftime(s_time, sizeof(s_time), format, tm_time);
     
        fctx_begin_fill(&fctx);
        fctx_set_fill_color(&fctx, flag_sidebar_color); 
        draw_string_radial(&fctx,
              s_time,
              g.font,  22,
              g.center, g.bounds.size.w / 2 - 26, TRIG_MAX_ANGLE, false);
        fctx_end_fill(&fctx);
    
     
       break;
     default: // displaying time in different timezone
       temp  += flag_secondary_info_type * 60;
       tm_time = gmtime(&temp);
     
       strcpy(s_second_info, s_timezone_name); //prepending with timezone name
       s_second_info[3]=' ';
       
       // building format 12h/24h
       if (clock_is_24h_style()) {
          strcpy(format, "%H:%M"); // e.g "14:46"
       } else {
          strcpy(format, "%l:%M %P"); // e.g " 2:46 PM" -- with leading space
       }   
     
       if (flag_time_separator == TIME_SEPARATOR_DOT) format[2] = '.';
     
       strftime(&s_second_info[4], sizeof(s_second_info), format, tm_time); //adding timezone time
     
        fctx_begin_fill(&fctx);
        fctx_set_fill_color(&fctx, flag_sidebar_color); 
        draw_string_radial(&fctx,
              s_second_info,
              g.font,  22,
              g.center, g.bounds.size.w / 2 - 26, TRIG_MAX_ANGLE, false);
        fctx_end_fill(&fctx);
       
       break;
     
   }
   }//************************ drawing secondary info line ****************************   
   
  
  fctx_deinit_context(&fctx);
 
}

static void main_update_proc(Layer *layer, GContext *ctx) {
  
   info_update_proc(layer, ctx);
   hands_update_proc(layer, ctx);
  
}

static void handle_second_tick(struct tm *tick_time, TimeUnits units_changed) {
 
  if (!(tick_time->tm_min % flag_weatherInterval) && (tick_time->tm_sec == 0)) { // on configured weather interval change - update the weather
        //APP_LOG(//APP_LOG_LEVEL_INFO, "**** I am inside 'tick_handler()' about to call 'update_weather();' at minute %d min on %d interval", tick_time->tm_min, flag_weatherInterval);
        update_weather();
  } 
  
  layer_mark_dirty(s_main_layer);
  
  // APP_LOG(APP_LOG_LEVEL_DEBUG, "TICK-TOCK: Free heap = %d", (int)heap_bytes_free());
  
}

// changes tick inteval to second or minute, depending on flags
void change_time_tick_inteval() {
  
  tick_timer_service_unsubscribe(); // unsubscribing from old interval
  
  if (flag_main_clock == MAIN_CLOCK_DIGITAL || flag_second_hand == SECOND_HAND_DISABLED) { // if analog face is hidden or second hand disabled
    tick_timer_service_subscribe(MINUTE_UNIT, handle_second_tick);
  } else {
    tick_timer_service_subscribe(SECOND_UNIT, handle_second_tick);
  }
  
  layer_mark_dirty(s_main_layer);
  
}

// handle configuration change
static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  Tuple *t = dict_read_first(iterator);
  
  bool need_weather = 0;
  bool need_rerender = 0;
  bool bluetooth_or_battery_needed = 0;

  while (t)  {
    
    switch(t->key)    {
        
      // weather data keys
      case KEY_WEATHER_TEMP:
        if (persist_read_int(KEY_WEATHER_TEMP) != t->value->int32) {
          persist_write_int(KEY_WEATHER_TEMP, t->value->int32);
          set_temperature(t->value->int32);
        }   
        break;
      
      case KEY_WEATHER_CODE:
        if (persist_read_int(KEY_WEATHER_CODE) != t->value->int32) {
          persist_write_int(KEY_WEATHER_CODE, t->value->int32);
          set_weather_icon(t->value->int32);
        }  
        break;
      
      case KEY_CITY_NAME:
        snprintf(s_second_info, sizeof(s_second_info), "%s", t->value->cstring);  
        persist_write_string(KEY_CITY_NAME, s_second_info);
        need_rerender = 1;
        break;
      
      case KEY_JSREADY:
          // JS ready lets get the weather
          if (t->value->int16) {
            //APP_LOG(//APP_LOG_LEVEL_INFO, "***** I am inside of 'inbox_received_callback()' message 'JS is ready' received !");
            flag_js_is_ready = true;
            need_weather = 1;
          }
          break;
      
        // config keys
       case KEY_TEMPERATURE_FORMAT: //if temp format changed from F to C or back - need re-request weather
         //APP_LOG(//APP_LOG_LEVEL_INFO, "***** I am inside of 'inbox_received_callback()' switching temp format");
         need_weather = 1;
         break;
     
        case KEY_MAIN_CLOCK:
           if (flag_main_clock != t->value->uint8) {
             persist_write_int(KEY_MAIN_CLOCK, t->value->uint8);
             flag_main_clock = t->value->uint8;
             need_rerender = 1;
           }  
           break;
        case KEY_BLUETOOTH_ALERT:
           if (flag_bluetooth_alert != t->value->uint8){
             persist_write_int(KEY_BLUETOOTH_ALERT, t->value->uint8);
             flag_bluetooth_alert = t->value->uint8;
             bluetooth_or_battery_needed = 1;
           }  
           break;
        case KEY_BLUETOOTH_ICON:
           if (flag_bluetooth_icon != t->value->uint8){
             persist_write_int(KEY_BLUETOOTH_ICON, t->value->uint8);
             flag_bluetooth_icon = t->value->uint8;
             bluetooth_or_battery_needed = 1;
           }  
           break;      
        case KEY_SECOND_HAND:
           if (flag_second_hand != t->value->uint8){
             persist_write_int(KEY_SECOND_HAND, t->value->uint8);
             flag_second_hand = t->value->uint8;
             change_time_tick_inteval();
           }  
           break;
        case KEY_LOCATION_SERVICE:
           if (t->value->int32 != flag_locationService) {
             persist_write_int(KEY_LOCATION_SERVICE, t->value->int32);
             flag_locationService = t->value->int32;
             need_weather = 1;
             //APP_LOG(//APP_LOG_LEVEL_INFO, "***** I am inside of 'inbox_received_callback()' location set to %d type", flag_locationService);
           }  
           break;
        case KEY_WEATHER_INTERVAL:
           if (t->value->int32 != flag_weatherInterval && t->value->int32 !=1) { // precaution, dunno why we get 1 here as well
             persist_write_int(KEY_WEATHER_INTERVAL, t->value->int32);
             flag_weatherInterval = t->value->int32;
             need_weather = 1;
             //APP_LOG(//APP_LOG_LEVEL_INFO, "***** I am inside of 'inbox_received_callback()' Weather interval set to interval to %d min", flag_weatherInterval);
           }
           break;
      
          case KEY_TIMEZONE_NAME:
              snprintf(s_timezone_name, sizeof(s_timezone_name), "%s", t->value->cstring);  
              persist_write_string(KEY_TIMEZONE_NAME, s_timezone_name);
              need_rerender = 1;
              break;
      
          case KEY_AMPM_TEXT:
              snprintf(s_ampm_text, sizeof(s_ampm_text), "%s", t->value->cstring);  
              persist_write_string(KEY_AMPM_TEXT, s_ampm_text);
              need_rerender = 1;
              break;
      
          case KEY_SECONDARY_INFO_TYPE:
               if (flag_secondary_info_type != t->value->int32) {
                 
                 //if we enabled health services
                 if (flag_secondary_info_type != SECONDARY_INFO_HEALTH_STEP_COUNTER && t->value->int32 == SECONDARY_INFO_HEALTH_STEP_COUNTER){
                   health_init(); // initializig health service
                   update_health_info(); // showing health info for the first time
                 }
                 
                 //if we disable health services
                 if (flag_secondary_info_type == SECONDARY_INFO_HEALTH_STEP_COUNTER && t->value->int32 != SECONDARY_INFO_HEALTH_STEP_COUNTER){
                    health_deinit(); // initializig health service
                 }
                 
                 //if we change location display type - need to bring new location name (via weather call)
                 if (t->value->int32 == SECONDARY_INFO_CURRENT_LOCATION_STREET_LEVEL || t->value->int32 == SECONDARY_INFO_CURRENT_LOCATION_TOWN_LEVEL || t->value->int32 == SECONDARY_INFO_CURRENT_LOCATION_COUNTRY_LEVEL) {
                    need_weather = 1;
                 }
                 
                 
                 persist_write_int(KEY_SECONDARY_INFO_TYPE, t->value->int32);
                 flag_secondary_info_type = t->value->int32;
                 need_rerender = 1;
               }  
               break;
      
          case KEY_TIME_SEPARATOR:
               if (flag_time_separator != t->value->int32) {
                 persist_write_int(KEY_TIME_SEPARATOR, t->value->int32);
                 flag_time_separator = t->value->int32;
                 need_rerender = 1; 
               }  
               break;
          case KEY_JS_TIMEZONE_OFFSET:
               if (flag_js_timezone_offset != t->value->int32) {
                 persist_write_int(KEY_JS_TIMEZONE_OFFSET, t->value->int32);
                 flag_js_timezone_offset = t->value->int32;
                 need_rerender = 1; 
               }  
               break;
           break;
      
           // color assignments
           
           case KEY_MAIN_BG_COLOR:
               if (flag_main_bg_color.argb != t->value->int32) {
                 persist_write_int(KEY_MAIN_BG_COLOR, t->value->int32);
                 flag_main_bg_color.argb = t->value->int32;
                 need_rerender = 1;
               }  
               break;
           case KEY_MAIN_COLOR:
               if (flag_main_color.argb != t->value->int32) {
                 persist_write_int(KEY_MAIN_COLOR, t->value->int32);
                 flag_main_color.argb = t->value->int32;
                 
                 if (flag_bluetooth_alert != BLUETOOTH_ALERT_DISABLED) {
                   bluetooth_or_battery_needed = 1;
                 }
                 
               }  
               break;
           case KEY_SIDEBAR_BG_COLOR:
               if (flag_sidebar_bg_color.argb != t->value->int32) {
                 persist_write_int(KEY_SIDEBAR_BG_COLOR, t->value->int32);
                 flag_sidebar_bg_color.argb = t->value->int32;
                 need_rerender = 1; 
               }  
               break;
           case KEY_SIDEBAR_COLOR:
               if (flag_sidebar_color.argb != t->value->int32) {
                 replace_gbitmap_color(flag_sidebar_color, (GColor){.argb = t->value->int32}, meteoicons_all, NULL);
                                  
                 persist_write_int(KEY_SIDEBAR_COLOR, t->value->int32);
                 flag_sidebar_color.argb = t->value->int32;
                 need_rerender = 1; 
               }  
               break;
           case KEY_COLOR_SELECTION:
               if (flag_color_selection != t->value->int32) {
                 persist_write_int(KEY_COLOR_SELECTION, t->value->int32);
                 flag_color_selection = t->value->int32;
              
                 if (flag_bluetooth_alert != BLUETOOTH_ALERT_DISABLED) {
                   bluetooth_or_battery_needed = 1;
                 }
                   
                }  
               break;
           
      
    }    
    
    t = dict_read_next(iterator);
  
  }
  
     // if it's a manual color selection - setting battery/second hand to manual
     if (flag_color_selection == COLOR_SELECTION_CUSTOM) {
          color_battery_major = flag_sidebar_bg_color;
          color_battery_minor = flag_sidebar_bg_color;  
     } else { // otherwise assigning default colors
          replace_gbitmap_color(flag_sidebar_color, GColorBlack, meteoicons_all, NULL);
        
          flag_main_bg_color = GColorBlack;
          flag_main_color = GColorWhite;
          flag_sidebar_bg_color = PBL_IF_COLOR_ELSE(GColorJaegerGreen, GColorWhite);
          flag_sidebar_color = GColorBlack;
       
   
     }
  
  
  // recoloring icons
  if (bluetooth_or_battery_needed == 1) {
      is_bluetooth_buzz_enabled = false;
      bluetooth_handler(bluetooth_connection_service_peek());
      is_bluetooth_buzz_enabled = true;
      battery_handler(battery_state_service_peek());
  }
  
  if (need_weather == 1) {
    //APP_LOG(//APP_LOG_LEVEL_INFO, "***** I am inside of 'inbox_received_callback()' about to call 'update_weather();");
    update_weather();
  }
  
  if (need_rerender == 1){
    layer_mark_dirty(s_main_layer);
  }

}  

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  flag_messaging_is_busy = false;
  //APP_LOG(//APP_LOG_LEVEL_ERROR, "____Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  flag_messaging_is_busy = false;
  //APP_LOG(//APP_LOG_LEVEL_ERROR, "____Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  flag_messaging_is_busy = false;
  //APP_LOG(//APP_LOG_LEVEL_INFO, "_____Outbox send success!");
} 



static void window_load(Window *window) {
  
  Layer *window_layer = window_get_root_layer(window);
  top_bound = layer_get_bounds(window_layer);
  
  //create main layer
  s_main_layer = layer_create(top_bound);
  layer_set_update_proc(s_main_layer, main_update_proc);
  layer_add_child(window_layer, s_main_layer);
    
  
}

static void window_unload(Window *window) {
  layer_destroy(s_main_layer);
  
  if (bluetooth_sprite) {
     gbitmap_destroy(bluetooth_sprite);
     bluetooth_sprite = NULL;
   }
}

static void init() {
  
  //going international
  setlocale(LC_ALL, "");
    
  font_18 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_BIG_NOODLE_TILTING_18));
  font_24 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_BIG_NOODLE_TILTING_24));
  font_27 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_BIG_NOODLE_TILTING_27));
  font_90 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_BIG_NOODLE_TILTING_56));
  
  s_window = window_create();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(s_window, true);
  
  g.font = ffont_create_from_resource(RESOURCE_ID_BIG_NOODLE_FFONT);
  g.bounds = layer_get_frame(window_get_root_layer(s_window));
  g.center.x = INT_TO_FIXED(g.bounds.size.w) / 2;
  g.center.y = INT_TO_FIXED(g.bounds.size.h) / 2;
 
  
  
   // Register callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);

  // Open AppMessage
  app_message_open(500, 500); 
 
  //loading presets
  flag_main_clock = persist_exists(KEY_MAIN_CLOCK)? persist_read_int(KEY_MAIN_CLOCK) : MAIN_CLOCK_ANALOG;
  flag_second_hand = persist_exists(KEY_SECOND_HAND)? persist_read_int(KEY_SECOND_HAND) : SECOND_HAND_ENABLED;
  flag_bluetooth_alert = persist_exists(KEY_BLUETOOTH_ALERT)? persist_read_int(KEY_BLUETOOTH_ALERT) : BLUETOOTH_ALERT_WEAK;
  flag_bluetooth_icon = persist_exists(KEY_BLUETOOTH_ICON)? persist_read_int(KEY_BLUETOOTH_ICON) : BLUETOOTH_ICON_ALWAYS_VISIBLE;
  flag_locationService = persist_exists(KEY_LOCATION_SERVICE)? persist_read_int(KEY_LOCATION_SERVICE) : LOCATION_AUTOMATIC;
  flag_weatherInterval = persist_exists(KEY_WEATHER_INTERVAL)? persist_read_int(KEY_WEATHER_INTERVAL) : 60; // default weather update is 1 hour
  flag_secondary_info_type = persist_exists(KEY_SECONDARY_INFO_TYPE)? persist_read_int(KEY_SECONDARY_INFO_TYPE) : SECONDARY_INFO_CURRENT_TIME;
  flag_time_separator = persist_exists(KEY_TIME_SEPARATOR)? persist_read_int(KEY_TIME_SEPARATOR) : TIME_SEPARATOR_COLON;
  flag_js_timezone_offset = persist_exists(KEY_JS_TIMEZONE_OFFSET)? persist_read_int(KEY_JS_TIMEZONE_OFFSET) : 0;
  flag_sidebar_location = persist_exists(KEY_SIDEBAR_LOCATION)? persist_read_int(KEY_SIDEBAR_LOCATION) : SIDEBAR_LOCATION_RIGHT;
  persist_exists(KEY_CITY_NAME)? persist_read_string(KEY_CITY_NAME, s_second_info, sizeof(s_second_info)) : snprintf(s_second_info, sizeof(s_second_info), "%s", "");
  persist_exists(KEY_TIMEZONE_NAME)? persist_read_string(KEY_TIMEZONE_NAME, s_timezone_name, sizeof(s_timezone_name)) : snprintf(s_timezone_name, sizeof(s_timezone_name), "%s", "");
  persist_exists(KEY_AMPM_TEXT)? persist_read_string(KEY_AMPM_TEXT, s_ampm_text, sizeof(s_ampm_text)) : snprintf(s_ampm_text, sizeof(s_ampm_text), "%s", "");
  
  // if secondary info is set to health - init health services
  if (flag_secondary_info_type == SECONDARY_INFO_HEALTH_STEP_COUNTER){
    health_init(); // initializig health service
    update_health_info(); // showing health info for the first time
  }  
  
  //loading custom colors
  
    flag_color_selection = persist_exists(KEY_COLOR_SELECTION)? persist_read_int(KEY_COLOR_SELECTION) : COLOR_SELECTION_AUTOMATIC;
    flag_main_bg_color.argb = persist_exists(KEY_MAIN_BG_COLOR)? persist_read_int(KEY_MAIN_BG_COLOR) : GColorBlackARGB8;
    flag_main_color.argb = persist_exists(KEY_MAIN_COLOR)? persist_read_int(KEY_MAIN_COLOR) : GColorWhiteARGB8;
    flag_sidebar_bg_color.argb = persist_exists(KEY_SIDEBAR_BG_COLOR)? persist_read_int(KEY_SIDEBAR_BG_COLOR) : PBL_IF_COLOR_ELSE(GColorJaegerGreen.argb, GColorWhite.argb);
    flag_sidebar_color.argb = persist_exists(KEY_SIDEBAR_COLOR)? persist_read_int(KEY_SIDEBAR_COLOR) : GColorBlackARGB8;
    
    // if it's a manual color selection - setting battery/second hand to manual
    if (flag_color_selection == COLOR_SELECTION_CUSTOM) {
      color_battery_major = flag_sidebar_bg_color;
      color_battery_minor = flag_sidebar_bg_color;  
    } else { // otherwise assigning default colors
      flag_main_bg_color = GColorBlack;
      flag_main_color = GColorWhite;
      flag_sidebar_bg_color = PBL_IF_COLOR_ELSE(GColorJaegerGreen, GColorWhite);
      flag_sidebar_color = GColorBlack;
      
      color_battery_minor = PBL_IF_COLOR_ELSE(GColorDarkGreen, GColorBlack);
      color_battery_major = PBL_IF_COLOR_ELSE(GColorJaegerGreen, GColorWhite);  

   }
  
  
  is_bluetooth_buzz_enabled = false;
  bluetooth_connection_service_subscribe(bluetooth_handler);
  bluetooth_handler(bluetooth_connection_service_peek());
  is_bluetooth_buzz_enabled = true;  
  
  #ifdef PBL_HEALTH
    sneaker = gbitmap_create_with_resource(RESOURCE_ID_SNEAKER);
  #endif
  
  meteoicons_all = gbitmap_create_with_resource(RESOURCE_ID_METEOICONS);
  
  if (flag_color_selection == COLOR_SELECTION_CUSTOM) { // in custom color mode colorin bitmaps as well
    replace_gbitmap_color(GColorBlack, flag_sidebar_color, meteoicons_all, NULL);
  }
  
  battery_state_service_subscribe(battery_handler);
  battery_handler(battery_state_service_peek());
 
  // showing previouslu saved weather without waiting for update
  if (flag_locationService != LOCATION_DISABLED){
    // reading stored value
    if (persist_exists(KEY_WEATHER_CODE)) set_weather_icon(persist_read_int(KEY_WEATHER_CODE));
    if (persist_exists(KEY_WEATHER_TEMP)) set_temperature(persist_read_int(KEY_WEATHER_TEMP));
  }
  
  
  //tick_timer_service_subscribe(SECOND_UNIT, handle_second_tick);
  change_time_tick_inteval();
  
}

static void deinit() {
  bluetooth_connection_service_unsubscribe();
  battery_state_service_unsubscribe();
  tick_timer_service_unsubscribe();
  window_destroy(s_window);
  
  gbitmap_destroy(meteoicons_all);
  
  app_message_deregister_callbacks();
  
  fonts_unload_custom_font(font_18);
  fonts_unload_custom_font(font_24);
  fonts_unload_custom_font(font_27);
  fonts_unload_custom_font(font_90);
  
  ffont_destroy(g.font);
}



#endif