#pragma once
#include "pebble.h"

bool health_is_available(); // to determine if health services are available
int health_get_metric_sum(HealthMetric metric); // getting requested metric value
void health_init(); // init health service
void health_deinit(); // deinit health service