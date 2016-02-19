#include <pebble.h>
#pragma once

#define KEY_READY 0
#define KEY_TEMP 1
#define KEY_TEMPC 2
#define KEY_WEATHERID 3
#define KEY_LANG 4
#define KEY_USECELSIUS 5
#define KEY_VIBE_ON_CONNECT 6
#define KEY_VIBE_ON_DISCONNECT 7
#define KEY_SHOW_WEATHER 8
#define KEY_COLOUR_SCHEME 9
#define KEY_UPDATE_TIME 10
#define KEY_BATT_AS_NUM 11
#define KEY_SHOW_STEPS 12
#define KEY_STEP_GOAL 13

//#define DEMO_MODE

AppTimer *weather_timeout, *ready_timeout;
GBitmap *weather_icon;
TextLayer *temp_layer;
Layer *weathericon_layer;

//static int lang = 4; // Hardcoded for testing
int lang;
int timeout;
int colourscheme;
int weatherupdatetime;
int stepgoal;

// Config options
bool use_celsius;
bool show_weather;
bool vibe_on_connect;
bool vibe_on_disconnect;
bool batt_as_percent;
bool show_step_goal;

void init_appmessage();
void inbox_received_handler(DictionaryIterator *iter, void *context);
void inbox_dropped_callback(AppMessageResult reason, void *context);
void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context);
void outbox_sent_callback(DictionaryIterator *iterator, void *context);
void update_time();
void batt_handler(BatteryChargeState state);
void update_weather();
void pick_colours();

extern const int WEATHER_ICONS[];