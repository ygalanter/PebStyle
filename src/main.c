#ifdef PBL_RECT
  #include "rect.h"
#else 
  #include "round.h"
#endif


int main() {
  init();
  app_event_loop();
  deinit();
}

