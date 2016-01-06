#include <pebble.h>
#include "languages.h"

#define KEY_READY 0
#define KEY_TEMP 1
#define KEY_TEMPC 2
#define KEY_WEATHERID 3

static Window *main_window;
static Layer *horz_rect_layer, *diag_rect_layer, *batt_layer, *bt_layer, *weathericon_layer;
static TextLayer *time_layer, *date_layer, *temp_layer;
static GBitmap *batt_icon, *bt_icon, *weather_icon;
static GPath *horz_rect, *diag_rect, *horz_drop, *diag_drop = NULL;
static AppTimer *weather_timeout;

static int lang = 0;
bool updated = false;

static const GPathInfo HORZ_PATH_POINTS = {
	.num_points = 4,
	.points = (GPoint []) {{-1, 120}, {-1, 155}, {180, 155}, {180, 120}}
};

static const GPathInfo HORZ_DROP_POINTS = {
	.num_points = 4,
	.points = (GPoint []) {{-1, 120}, {-1, 163}, {180, 163}, {180, 120}}
};

static const GPathInfo DIAG_PATH_POINTS = {
	.num_points = 4,
	.points = (GPoint []) {{-20, 125}, {-1, 160}, {180, 160}, {180, 125}}
};

static const GPathInfo DIAG_DROP_POINTS = {
	.num_points = 4,
	.points = (GPoint []) {{-20, 117}, {-1, 160}, {180, 160}, {180, 117}}
};

const int BATT_ICONS[] = {
	RESOURCE_ID_BATT_LOW,			// 0
	RESOURCE_ID_BATT_20,			// 1
	RESOURCE_ID_BATT_40,			// 2
	RESOURCE_ID_BATT_60,			// 3
	RESOURCE_ID_BATT_80,			// 4
	RESOURCE_ID_BATT_FULL,		// 5
	RESOURCE_ID_BATT_CHARGING	// 6
};

const int WEATHER_ICONS[] = {
	RESOURCE_ID_ICON_UNKNOWN,				// 0
	RESOURCE_ID_ICON_THUNDERSTORM,	// 1
	RESOURCE_ID_ICON_SUN,						// 2
	RESOURCE_ID_ICON_SNOW,					// 3
	RESOURCE_ID_ICON_RAIN,					// 4
	RESOURCE_ID_ICON_PARTLY_CLOUDY,	// 5
	RESOURCE_ID_ICON_NIGHT,					// 6
	RESOURCE_ID_ICON_FOG,						// 7
	RESOURCE_ID_ICON_CLOUDY,				// 8
	RESOURCE_ID_ICON_CLOUDY_NIGHT		// 9
};

static void update_time() {
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
	
  static char time_buffer[] = "00:00";
  static char datn_buffer[] = "DD"; // Buffer for date number
  static char date_buffer[] = "WWW MMM DD"; // Buffer for entire date to display
  
  if(clock_is_24h_style() == true) {
    strftime(time_buffer, sizeof("00:00"), "%H:%M", tick_time);
  } else {
    strftime(time_buffer, sizeof("00:00"), "%I:%M", tick_time);
  }
	
	text_layer_set_text(time_layer, time_buffer);

	strftime(datn_buffer, sizeof("DD"), "%e", tick_time); // Write current date to buffer
	int weekday = tick_time->tm_wday; // Get current weekday as an integer (0 is Sunday)
	
	// Select the correct strings from languages.c and write to buffer along with date
	snprintf(date_buffer, sizeof(date_buffer), "%s %s", dayNames[lang][weekday], datn_buffer);

	text_layer_set_text(date_layer, date_buffer); // Display the date info
}

static void batt_handler(BatteryChargeState state) {
	APP_LOG(APP_LOG_LEVEL_INFO, "Start batt_handler");
	int pct = state.charge_percent;
	bool charging = state.is_charging;
	
	if (batt_icon) {
		gbitmap_destroy(batt_icon);
	}
	
	if (charging) {
		batt_icon = gbitmap_create_with_resource(BATT_ICONS[6]);
	} else if (pct <= 10) {
		batt_icon = gbitmap_create_with_resource(BATT_ICONS[0]);
	} else if (pct <= 20) {
		batt_icon = gbitmap_create_with_resource(BATT_ICONS[1]);
	} else if (pct <= 40) {
		batt_icon = gbitmap_create_with_resource(BATT_ICONS[2]);
	} else if (pct <= 60) {
		batt_icon = gbitmap_create_with_resource(BATT_ICONS[3]);
	} else if (pct <= 80) {
		batt_icon = gbitmap_create_with_resource(BATT_ICONS[4]);
	} else if (pct <= 100) {
		batt_icon = gbitmap_create_with_resource(BATT_ICONS[5]);
	}
	
	layer_mark_dirty(batt_layer);
}

static void bt_handler(bool connected) {
	if (bt_icon) {
		gbitmap_destroy(bt_icon);
	}
	
	if (connected) {
		vibes_long_pulse();
		bt_icon = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_CONNECTED);
	} else {
		vibes_double_pulse();
		bt_icon = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_DISCONNECTED);
	}
	
	layer_mark_dirty(bt_layer);
}

static void setup_rects(void) {
	horz_rect = gpath_create(&HORZ_PATH_POINTS);
	diag_rect = gpath_create(&DIAG_PATH_POINTS);
	horz_drop = gpath_create(&HORZ_DROP_POINTS);
	diag_drop = gpath_create(&DIAG_DROP_POINTS);
	
	gpath_rotate_to(diag_rect, TRIG_MAX_ANGLE / -360 * PBL_IF_ROUND_ELSE(45, 55));
	gpath_move_to(diag_rect, GPoint(-40, 90));
	
	gpath_rotate_to(diag_drop, TRIG_MAX_ANGLE / -360 * PBL_IF_ROUND_ELSE(45, 55));
	gpath_move_to(diag_drop, GPoint(-40, 90));
}

static void draw_horz_rect(Layer *layer, GContext *ctx) {
	graphics_context_set_antialiased(ctx, true);
	
	graphics_context_set_fill_color(ctx, GColorFromRGB(245, 124, 0));
	gpath_draw_filled(ctx, horz_drop);
	
	graphics_context_set_fill_color(ctx, GColorFromRGB(255, 167, 38));
	gpath_draw_filled(ctx, horz_rect);
	
	graphics_context_set_stroke_color(ctx, GColorFromRGB(245, 124, 0));
	gpath_draw_outline(ctx, horz_rect);
}

static void draw_diag_rect(Layer *layer, GContext *ctx) {
	graphics_context_set_antialiased(ctx, true);
	
	graphics_context_set_fill_color(ctx, GColorFromRGB(13, 71, 161));
	gpath_draw_filled(ctx, diag_drop);
	
	graphics_context_set_fill_color(ctx, GColorFromRGB(33, 150, 243));
	gpath_draw_filled(ctx, diag_rect);
	
	graphics_context_set_stroke_color(ctx, GColorFromRGB(13, 71, 161));
	gpath_draw_outline(ctx, diag_rect);
}

static void draw_batt(Layer *layer, GContext *ctx) {
	APP_LOG(APP_LOG_LEVEL_INFO, "Drawing battery icon");
	graphics_context_set_compositing_mode(ctx, GCompOpSet);	
	graphics_draw_bitmap_in_rect(ctx, batt_icon, layer_get_bounds(batt_layer));
}

static void draw_bt(Layer *layer, GContext *ctx) {
	APP_LOG(APP_LOG_LEVEL_INFO, "Drawing BT icon");
	graphics_context_set_compositing_mode(ctx, GCompOpSet);	
	graphics_draw_bitmap_in_rect(ctx, bt_icon, layer_get_bounds(bt_layer));
}

static void draw_weathericon(Layer *layer, GContext *ctx) {
	APP_LOG(APP_LOG_LEVEL_INFO, "Drawing weather icon");
	graphics_context_set_compositing_mode(ctx, GCompOpSet);	
	graphics_draw_bitmap_in_rect(ctx, weather_icon, layer_get_bounds(weathericon_layer));
}

static void weather_ended() {
	APP_LOG(APP_LOG_LEVEL_INFO, "Weather timer ended");
	
	/*if (weather_icon) {
		APP_LOG(APP_LOG_LEVEL_INFO, "Destroying weather icon in weather_ended");
		gbitmap_destroy(weather_icon);
	}*/
	
	/*if (updated == false) {
		APP_LOG(APP_LOG_LEVEL_INFO, "Updated is false");
		weather_icon = gbitmap_create_with_resource(RESOURCE_ID_ICON_ERROR);
		layer_mark_dirty(weathericon_layer);
	}*/
	
	weather_icon = gbitmap_create_with_resource(RESOURCE_ID_ICON_ERROR);
	layer_mark_dirty(weathericon_layer);
}

static void main_window_load(Window *window) {
	setup_rects();
	
	GRect bounds = layer_get_bounds(window_get_root_layer(window));
	
	// Set up window & shapes
	window_set_background_color(window, GColorFromRGB(38, 166, 154));
	
	diag_rect_layer = layer_create(GRect(0, 0, bounds.size.w, bounds.size.h));
	layer_set_update_proc(diag_rect_layer, draw_diag_rect);
	
	horz_rect_layer = layer_create(GRect(0, 0, bounds.size.w, bounds.size.h));
	layer_set_update_proc(horz_rect_layer, draw_horz_rect);

	layer_add_child(window_get_root_layer(window), diag_rect_layer);
	layer_add_child(window_get_root_layer(window), horz_rect_layer);
	
	// Set up time & date
	
	time_layer = text_layer_create(GRect(2, 0, bounds.size.w, bounds.size.h));
	text_layer_set_background_color(time_layer, GColorClear);
	text_layer_set_font(time_layer, fonts_get_system_font(FONT_KEY_LECO_42_NUMBERS));
	text_layer_set_text(time_layer, "00:00");
	GSize time_size = text_layer_get_content_size(time_layer);
	layer_set_frame(text_layer_get_layer(time_layer), GRect(PBL_IF_ROUND_ELSE(27, 7), PBL_IF_ROUND_ELSE(20, 0), time_size.w, time_size.h));
	
	date_layer = text_layer_create(GRect(2, time_size.h + 3, bounds.size.w, bounds.size.h));
	text_layer_set_background_color(date_layer, GColorClear);
	text_layer_set_font(date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28));
	text_layer_set_text(date_layer, "MON 01");
	GSize date_size = text_layer_get_content_size(date_layer);
	layer_set_frame(text_layer_get_layer(date_layer), GRect(PBL_IF_ROUND_ELSE(29, 9), PBL_IF_ROUND_ELSE(time_size.h + 16, time_size.h - 2), date_size.w, date_size.h));
	
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(time_layer));
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(date_layer));
	
	update_time();
	
	// Set up battery & BT icons
	
	batt_layer = layer_create(GRect(PBL_IF_ROUND_ELSE(30, 5), 125, 14, 26));
	layer_set_update_proc(batt_layer, draw_batt);
	
	APP_LOG(APP_LOG_LEVEL_INFO, "Checking battery level");
	batt_handler(battery_state_service_peek());
	
	bt_layer = layer_create(GRect(PBL_IF_ROUND_ELSE(135, 118), 126, 22, 24));
	layer_set_update_proc(bt_layer, draw_bt);
	
	APP_LOG(APP_LOG_LEVEL_INFO, "Checking BT status");
	if (bluetooth_connection_service_peek()) {
		bt_icon = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_CONNECTED);
	} else {
		bt_icon = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_DISCONNECTED);
	}
	
	layer_add_child(horz_rect_layer, batt_layer);
	layer_add_child(horz_rect_layer, bt_layer);
	
	// Set up weather layers
	
	weathericon_layer = layer_create(GRect(PBL_IF_ROUND_ELSE(152, 122), 73, 20, 23));
	layer_set_update_proc(weathericon_layer, draw_weathericon);
	weather_icon = gbitmap_create_with_resource(RESOURCE_ID_ICON_LOADING);
	
	temp_layer = text_layer_create(GRect(PBL_IF_ROUND_ELSE(130, 121), 75, 144, 168));
	text_layer_set_background_color(temp_layer, GColorClear);
	text_layer_set_font(temp_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
	
	
	layer_add_child(diag_rect_layer, weathericon_layer);
	layer_add_child(diag_rect_layer, text_layer_get_layer(temp_layer));
}

static void main_window_unload(Window *window) {
	layer_destroy(horz_rect_layer);
	layer_destroy(diag_rect_layer);
	layer_destroy(batt_layer);
	layer_destroy(bt_layer);
	layer_destroy(weathericon_layer);
	text_layer_destroy(time_layer);
	text_layer_destroy(date_layer);
	text_layer_destroy(temp_layer);
	gbitmap_destroy(batt_icon);
	gbitmap_destroy(bt_icon);
	APP_LOG(APP_LOG_LEVEL_INFO, "Destroying weather icon in main_window_unload");
	gbitmap_destroy(weather_icon);
	gpath_destroy(horz_rect);
	gpath_destroy(diag_rect);
	gpath_destroy(horz_drop);
	gpath_destroy(diag_drop);
}

static void inbox_received_handler(DictionaryIterator *iter, void *context) {
	static char temp_buffer[5];
	static char tempc_buffer [5];
	
	Tuple *ready_t = dict_find(iter, KEY_READY); // cstring
	Tuple *temp_t = dict_find(iter, KEY_TEMP); // int32
	Tuple *tempc_t = dict_find(iter, KEY_TEMPC); // int32
	Tuple *id_t = dict_find(iter, KEY_WEATHERID); // int32
	
	if (ready_t) {
		int status = (int)ready_t->value->int32;
		APP_LOG(APP_LOG_LEVEL_INFO, "Ready status is %d", status);
		
		APP_LOG(APP_LOG_LEVEL_INFO, "Starting weather_timeout...");
		weather_timeout = app_timer_register(10000, weather_ended, NULL);
		
		if (status == 0) {
			APP_LOG(APP_LOG_LEVEL_INFO, "JS reports ready!");

			// Begin dictionary
			DictionaryIterator *iter;
			app_message_outbox_begin(&iter);

			// Add a key-value pair
			dict_write_uint8(iter, 3, 0);

			// Send the message!
			/*APP_LOG(APP_LOG_LEVEL_INFO, "Setting updated to false in ready_t");
			updated = false;*/
			app_message_outbox_send();
			APP_LOG(APP_LOG_LEVEL_INFO, "Requested weather, starting weather_timeout...");
			weather_timeout = app_timer_register(10000, weather_ended, NULL);
		}
	}
	
	if (temp_t) {
		APP_LOG(APP_LOG_LEVEL_INFO, "KEY_TEMP received");
		
		APP_LOG(APP_LOG_LEVEL_INFO, "Cancelling weather timer");
		if (weather_timeout != NULL) {
			app_timer_cancel(weather_timeout);
			weather_timeout = NULL;
		}
		
		snprintf(temp_buffer, sizeof(temp_buffer), "%d°", (int)temp_t->value->int32);
	}
	
	if (tempc_t) {
		APP_LOG(APP_LOG_LEVEL_INFO, "KEY_TEMPC received");
		
		APP_LOG(APP_LOG_LEVEL_INFO, "Cancelling weather timer");
		if (weather_timeout != NULL) {
			app_timer_cancel(weather_timeout);
			weather_timeout = NULL;
		}
		
		snprintf(tempc_buffer, sizeof(tempc_buffer), "%d°", (int)tempc_t->value->int32);
		text_layer_set_text(temp_layer, tempc_buffer);
		
		GRect bounds = layer_get_bounds(window_get_root_layer(main_window));
		GSize temp_size = text_layer_get_content_size(temp_layer);
		layer_set_frame(text_layer_get_layer(temp_layer), GRect(PBL_IF_ROUND_ELSE(131, 110), 95, temp_size.w, temp_size.h));
	}
	
	if (id_t) {
		APP_LOG(APP_LOG_LEVEL_INFO, "Weather ID is %d", (int)id_t->value->int32);
		//APP_LOG(APP_LOG_LEVEL_INFO, "Setting updated to true");
		//updated = true;
		
		APP_LOG(APP_LOG_LEVEL_INFO, "Cancelling weather timer");
		if (weather_timeout != NULL) {
			app_timer_cancel(weather_timeout);
			weather_timeout = NULL;
		}
		
		int weatherid = (int)id_t->value->int32;
		
		/*if (weather_icon) {
			APP_LOG(APP_LOG_LEVEL_INFO, "Destroying weather icon in inbox");
			gbitmap_destroy(weather_icon);
		}*/
		
		time_t temp = time(NULL);
		struct tm *tick_time = localtime(&temp);
		int hour = tick_time->tm_hour;
		
		if ((weatherid % 900) < 100) {
			APP_LOG(APP_LOG_LEVEL_INFO, "Picking 900");
			weather_icon = gbitmap_create_with_resource(WEATHER_ICONS[0]);
		} else if ((weatherid % 801) == 0) {
			APP_LOG(APP_LOG_LEVEL_INFO, "Picking 801");
			if (hour >= 18) { // If it's past 6PM, display the night variant (clear sky moon)
				weather_icon = gbitmap_create_with_resource(WEATHER_ICONS[9]);
			} else { // Display the day varient (clear sky sun)
				weather_icon = gbitmap_create_with_resource(WEATHER_ICONS[5]);
			}
		} else if ((weatherid % 800) == 0) {
			APP_LOG(APP_LOG_LEVEL_INFO, "Picking 800 exactly");
			if (hour >= 18) { // If it's past 6PM, display the night variant (clear sky moon)
				weather_icon = gbitmap_create_with_resource(WEATHER_ICONS[6]);
			} else { // Display the day varient (clear sky sun)
				weather_icon = gbitmap_create_with_resource(WEATHER_ICONS[2]);
			}
		} else if ((weatherid % 800) < 100) {
			APP_LOG(APP_LOG_LEVEL_INFO, "Picking 800");
			weather_icon = gbitmap_create_with_resource(WEATHER_ICONS[8]);
		} else if ((weatherid % 700) < 100) {
			APP_LOG(APP_LOG_LEVEL_INFO, "Picking 700");
			weather_icon = gbitmap_create_with_resource(WEATHER_ICONS[7]);
		} else if ((weatherid % 600) < 100) {
			APP_LOG(APP_LOG_LEVEL_INFO, "Picking 600");
			weather_icon = gbitmap_create_with_resource(WEATHER_ICONS[3]);
		} else if ((weatherid % 500) < 100) {
			APP_LOG(APP_LOG_LEVEL_INFO, "Picking 500");
			weather_icon = gbitmap_create_with_resource(WEATHER_ICONS[4]);
		} else if ((weatherid % 200) < 100) {
			APP_LOG(APP_LOG_LEVEL_INFO, "Picking 200");
			weather_icon = gbitmap_create_with_resource(WEATHER_ICONS[1]);
		} else {
			APP_LOG(APP_LOG_LEVEL_INFO, "Picking default");
			weather_icon = gbitmap_create_with_resource(WEATHER_ICONS[0]);
		}
		
		layer_mark_dirty(weathericon_layer);
	}
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
	update_time();
	
	if (tick_time->tm_min % 30 == 0) {
			/*if (weather_icon) {
				APP_LOG(APP_LOG_LEVEL_INFO, "Destroying weather icon in tick_handler");
				gbitmap_destroy(weather_icon);
			}*/
			weather_icon = gbitmap_create_with_resource(RESOURCE_ID_ICON_LOADING);
			layer_mark_dirty(weathericon_layer);
			// Begin dictionary
			DictionaryIterator *iter;
			app_message_outbox_begin(&iter);

			// Add a key-value pair
			dict_write_uint8(iter, 3, 0);

			// Send the message!
			APP_LOG(APP_LOG_LEVEL_INFO, "Setting updated to false in tick_handler");
			updated = false;
			app_message_outbox_send();
			APP_LOG(APP_LOG_LEVEL_INFO, "Requested weather update, starting weather_timeout...");
			weather_timeout = app_timer_register(10000, weather_ended, NULL);
	}
}

static void init() {
  main_window = window_create();

  window_set_window_handlers(main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  window_stack_push(main_window, true);
	
	tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
	battery_state_service_subscribe(batt_handler);
	bluetooth_connection_service_subscribe(bt_handler);
	
	app_message_register_inbox_received(inbox_received_handler);
	app_message_register_inbox_dropped(inbox_dropped_callback);
	app_message_register_outbox_failed(outbox_failed_callback);
	app_message_register_outbox_sent(outbox_sent_callback);
	
	int buffer_in = dict_calc_buffer_size(4, sizeof(char), sizeof(int32_t), sizeof(int32_t), sizeof(int32_t));
	int buffer_out = dict_calc_buffer_size(1, sizeof(int32_t));
	app_message_open(buffer_in, buffer_out);
}

static void deinit() {
  window_destroy(main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}