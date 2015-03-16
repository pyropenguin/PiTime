/*
This code is written by Jonathan Kunze
This code is released to the public domain.
*/

#include <pebble.h>
#include "PDUtils.h"

Window *window;
TextLayer *timeLayer;
TextLayer *cdLayer;
TextLayer *textLayer;
Layer *backgroundLayer;
typedef struct tm PblTm;
PblTm PI_TIME;

time_t computeDeltaTime(struct tm * time1, struct tm * time2)
{
  time_t time1Secs = p_mktime(time1);
  time_t time2Secs = p_mktime(time2);
  
  return time1Secs - time2Secs;
}

time_t getNextEvent(struct tm * refTime, struct tm * currTime)
{
  time_t dTime = computeDeltaTime(refTime, currTime);
  
  while (dTime < 0)
  {
    refTime->tm_year++; // That was fun. Let's do this again next year.
    dTime = computeDeltaTime(refTime, currTime);
  }
  
  return (dTime);
}

void getDeltaTimeString(struct tm * refTime, struct tm * currTime, char * buffer, int size)
{
  time_t dTime = getNextEvent(refTime, currTime);
  
  if (dTime == 0)
    vibes_double_pulse(); // PI TIME! Celebratory buzz!
  
  int days = (int)(dTime / (60*60*24));
  int hours = (int)(dTime / (60*60)) - (days * 24);
  int minutes = (int)(dTime / (60)) - (days * 24*60) - (hours * 60);
  int seconds = (int)(dTime) - (days * 24*60*60) - (hours * 60*60) - (minutes * 60);
  
  if (days == 0)
    snprintf(buffer, size, "%d:%02d:%02d", hours, minutes, seconds);
  else if (days == 1)
    snprintf(buffer, size, "1 Day\n%d:%02d:%02d", hours, minutes, seconds);
  else
    snprintf(buffer, size, "%d Days\n%d:%02d:%02d", days, hours, minutes, seconds);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  static char s_time_buffer[16];
  static char cd_buffer[64];
  
  // Set text layers for the next event
  if (clock_is_24h_style()) {
    strftime(s_time_buffer, sizeof(s_time_buffer), "%H:%M", tick_time);
  } else {
    strftime(s_time_buffer, sizeof(s_time_buffer), "%I:%M", tick_time);
  }

  getDeltaTimeString(&PI_TIME, tick_time, cd_buffer, sizeof(cd_buffer));

  text_layer_set_text(timeLayer, s_time_buffer);
  text_layer_set_text(cdLayer, cd_buffer);
}

void handle_init(void) {
  time_t currTimeSecs;
  static struct tm * currTime;
  static char text_buffer[] = "TILL PI\nTIME!";
  
  window = window_create();
  window_set_background_color(window, GColorBlack);
  
  backgroundLayer = window_get_root_layer(window);

  timeLayer = text_layer_create(GRect(0, -3, 144 /* width */, 19 /* height */));
  text_layer_set_text_color(timeLayer, GColorWhite);
  text_layer_set_text_alignment(timeLayer,GTextAlignmentCenter);
  text_layer_set_background_color(timeLayer, GColorClear);
  text_layer_set_font(timeLayer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text(timeLayer, "--:--");

  cdLayer = text_layer_create(GRect(0, 20, 144 /* width */, 60 /* height */));
  text_layer_set_text(cdLayer, "----------");
  text_layer_set_font(cdLayer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text_color(cdLayer, GColorWhite);
  text_layer_set_text_alignment(cdLayer,GTextAlignmentCenter);
  text_layer_set_background_color(cdLayer, GColorClear);
  
  textLayer = text_layer_create(GRect(0, 20+60, 144 /* width */, 109 /* height */));
  text_layer_set_text(textLayer, text_buffer);
  text_layer_set_font(textLayer, fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT));
  text_layer_set_text_color(textLayer, GColorWhite);
  text_layer_set_text_alignment(textLayer,GTextAlignmentCenter);
  text_layer_set_background_color(textLayer, GColorClear);
  
  layer_add_child(backgroundLayer, text_layer_get_layer(timeLayer));
  layer_add_child(backgroundLayer, text_layer_get_layer(cdLayer));
  layer_add_child(backgroundLayer, text_layer_get_layer(textLayer));
  
  // Initialize Pi Time Structure
  time(&currTimeSecs);
  currTime = localtime(&currTimeSecs);
  memcpy(&PI_TIME, currTime, sizeof(PblTm));
  // PI is 3.1415926, so 3/14 1:59:26
  PI_TIME.tm_sec = 26;
  PI_TIME.tm_min = 59;
  PI_TIME.tm_hour = 13; // No one wants to do Pi time at 1:59:26 AM.
  PI_TIME.tm_mday = 14;
  PI_TIME.tm_mon = 3 - 1; // 0 - 11
  
  window_stack_push(window, true);
  
  tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
}

void handle_deinit(void) {
  text_layer_destroy(timeLayer);
  text_layer_destroy(cdLayer);
  text_layer_destroy(textLayer);
  window_destroy(window);
}

int main(void) {
  handle_init();
  app_event_loop();
  handle_deinit();
}
