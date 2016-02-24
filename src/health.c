#include <pebble.h>
#include "health.h"

// external event to raise from health event
extern void update_health_info();

// to determine if health services are available
static bool s_health_available;

bool health_is_available() {
  return s_health_available;
}

// getting requested metric value
int health_get_metric_sum(HealthMetric metric) {
  
  HealthServiceAccessibilityMask mask = health_service_metric_accessible(metric, time_start_of_today(), time(NULL));
  
  if(mask == HealthServiceAccessibilityMaskAvailable) {
    return (int)health_service_sum_today(metric);
  } else {
    return 0;
  }
}

// raising event in main code to retreive health data
static void health_handler(HealthEventType event, void *context) {
    update_health_info();
}


// initializing health services
void health_init() {
  #ifdef PBL_HEALTH
    s_health_available = health_service_events_subscribe(health_handler, NULL);
  #else
    s_health_available = false;
  #endif
}

// unsubscribing from health services
void health_deinit(){
  #ifdef PBL_HEALTH
    health_service_events_unsubscribe();
  #endif
}