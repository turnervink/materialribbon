#include <pebble.h>
#include "messaging.h"
#include "languages.h"
#include "gbitmap_color_palette_manipulator.h"

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

void init_appmessage() {
	app_message_register_inbox_received(inbox_received_handler);
	app_message_register_inbox_dropped(inbox_dropped_callback);
	app_message_register_outbox_failed(outbox_failed_callback);
	app_message_register_outbox_sent(outbox_sent_callback);
	
	// Create buffers based on what we are sending/receiving
	int buffer_in = dict_calc_buffer_size(10, sizeof(char), sizeof(int32_t), sizeof(int32_t), sizeof(int32_t), sizeof(char), sizeof(int8_t), sizeof(int8_t), sizeof(int8_t), sizeof(int8_t), sizeof(int8_t), sizeof(int8_t));
	int buffer_out = dict_calc_buffer_size(1, sizeof(int32_t));
	app_message_open(buffer_in, buffer_out);
}

static void weather_ended() {
	// If the weather can't be updated show the error icon
	APP_LOG(APP_LOG_LEVEL_INFO, "Weather timer ended");
	
	if (weather_timeout != NULL) {
		APP_LOG(APP_LOG_LEVEL_INFO, "Weather timer is not NULL");
		weather_icon = gbitmap_create_with_resource(RESOURCE_ID_ICON_ERROR);
		layer_mark_dirty(weathericon_layer);
		text_layer_set_text(temp_layer, " ");
	}
}

void update_weather() {
	// Show the loading icon, request the weather, and start the timeout
	weather_icon = gbitmap_create_with_resource(RESOURCE_ID_ICON_LOADING);
	layer_mark_dirty(weathericon_layer);
	GRect icon = gbitmap_get_bounds(weather_icon);
	layer_set_frame(weathericon_layer, GRect(PBL_IF_ROUND_ELSE(165 - (icon.size.w / 2), 133 - (icon.size.w / 2)), 73, 20, 23));
	text_layer_set_text(temp_layer, " ");
	
	// Begin dictionary
	DictionaryIterator *iter;
	app_message_outbox_begin(&iter);

	// Add a key-value pair
	dict_write_uint8(iter, 3, 0);

	// Send the message!
	app_message_outbox_send();
	APP_LOG(APP_LOG_LEVEL_INFO, "Requested weather, starting weather_timeout...");
	weather_timeout = app_timer_register(timeout, weather_ended, NULL);
}

static void cancel_ready_timeout() {
	// Cancel the timeout once JS is ready
	if (ready_timeout != NULL) {
			APP_LOG(APP_LOG_LEVEL_INFO, "Cancelling ready timer");
			app_timer_cancel(ready_timeout);
			ready_timeout = NULL;
	}
}

static void cancel_weather_timeout() {
	// Cancel the timeout once weather is received
	if (weather_timeout != NULL) {
			APP_LOG(APP_LOG_LEVEL_INFO, "Cancelling weather timer");
			app_timer_cancel(weather_timeout);
			weather_timeout = NULL;
	}
}

void inbox_received_handler(DictionaryIterator *iter, void *context) {
	static char temp_buffer[10];
	static char tempc_buffer[10];
	
	Tuple *ready_tup = dict_find(iter, KEY_READY); // cstring
	
	Tuple *temp_tup = dict_find(iter, KEY_TEMP); // int32
	Tuple *tempc_tup = dict_find(iter, KEY_TEMPC); // int32
	Tuple *id_tup = dict_find(iter, KEY_WEATHERID); // int32
	
	Tuple *lang_tup = dict_find(iter, KEY_LANG); // cstring
	Tuple *usecelsius_tup = dict_find(iter, KEY_USECELSIUS); // int8
	Tuple *showweather_tup = dict_find(iter, KEY_SHOW_WEATHER); //int8
	Tuple *vibeconnect_tup = dict_find(iter, KEY_VIBE_ON_CONNECT); // int8
	Tuple *vibedisconnect_tup = dict_find(iter, KEY_VIBE_ON_DISCONNECT); // int8
	Tuple *colourscheme_tup = dict_find(iter, KEY_COLOUR_SCHEME); // int8
	Tuple *updatetime_tup = dict_find(iter, KEY_UPDATE_TIME); // int8
	
	if (ready_tup) {
		int status = (int)ready_tup->value->int32;
		APP_LOG(APP_LOG_LEVEL_INFO, "Ready status is %d", status);
		
		cancel_ready_timeout();
		
		if (status == 0) {
			if (show_weather == 1) {
				APP_LOG(APP_LOG_LEVEL_INFO, "Ready status is 0 and weather is shown, updating weather.");
				update_weather();
			}
		}
	}
	
	if (temp_tup) {
		APP_LOG(APP_LOG_LEVEL_INFO, "KEY_TEMP received");
		cancel_weather_timeout();
		
		snprintf(temp_buffer, sizeof(temp_buffer), "%d°", (int)temp_tup->value->int32);
		//snprintf(tempc_buffer, sizeof(tempc_buffer), "13°");
	}
	
	if (tempc_tup) {
		APP_LOG(APP_LOG_LEVEL_INFO, "KEY_TEMPC received");
		cancel_weather_timeout();
		
		snprintf(tempc_buffer, sizeof(tempc_buffer), "%d°", (int)tempc_tup->value->int32);
		//snprintf(tempc_buffer, sizeof(tempc_buffer), "13°");
	}
	
	if (usecelsius_tup) {
		APP_LOG(APP_LOG_LEVEL_INFO, "KEY_USE_CELSIUS received!");

  	use_celsius = usecelsius_tup->value->int8;

  	persist_write_int(KEY_USECELSIUS, use_celsius);
	}	
	
	if (showweather_tup) {
		APP_LOG(APP_LOG_LEVEL_INFO, "KEY_SHOW_WEATHER received!");
		
		show_weather = showweather_tup->value->int8;
		APP_LOG(APP_LOG_LEVEL_INFO, "show_weather: %d", show_weather);
		
		persist_write_int(KEY_SHOW_WEATHER, show_weather);
		if (show_weather == 1) {
			update_weather();
		}
	}
	
	if (use_celsius == 1) {
		text_layer_set_text(temp_layer, tempc_buffer);
		//text_layer_set_text(temp_layer, "13°");
		//GSize temp_size = text_layer_get_content_size(temp_layer);
		//APP_LOG(APP_LOG_LEVEL_INFO, "Width is %d", temp_size.w);
		//layer_set_frame(text_layer_get_layer(temp_layer), GRect(PBL_IF_ROUND_ELSE(145 - (temp_size.w / 2), 123 - (temp_size.w / 2)), 95, 30, 30));
	} else {
		text_layer_set_text(temp_layer, temp_buffer);
		//text_layer_set_text(temp_layer, "13°");
		//GSize temp_size = text_layer_get_content_size(temp_layer);
		//APP_LOG(APP_LOG_LEVEL_INFO, "Width is %d", temp_size.w);
		//layer_set_frame(text_layer_get_layer(temp_layer), GRect(PBL_IF_ROUND_ELSE(145 - (temp_size.w / 2), 123 - (temp_size.w / 2)), 95, 30, 30));
	}
	
	if (show_weather == 0) {
		APP_LOG(APP_LOG_LEVEL_INFO, "Hiding weather");
		layer_set_hidden(text_layer_get_layer(temp_layer), true);
		layer_set_hidden(weathericon_layer, true);
	} else {
		APP_LOG(APP_LOG_LEVEL_INFO, "Showing weather");
		layer_set_hidden(text_layer_get_layer(temp_layer), false);
		layer_set_hidden(weathericon_layer, false);
	}
	
	if (id_tup) {
		APP_LOG(APP_LOG_LEVEL_INFO, "Weather ID is %d", (int)id_tup->value->int32);
		cancel_weather_timeout();
		
		int weatherid = (int)id_tup->value->int32;
		//int weatherid = 800; // Hardcoded for testing
		
		if (weather_icon != NULL) {
			APP_LOG(APP_LOG_LEVEL_INFO, "Destroying weather icon in inbox");
			gbitmap_destroy(weather_icon);
		}
		
		time_t temp = time(NULL);
		struct tm *tick_time = localtime(&temp);
		//int hour = tick_time->tm_hour;
		int hour = 10;
		
		// Pick the right icon
		if ((weatherid % 900) < 100) {
			APP_LOG(APP_LOG_LEVEL_INFO, "Picking 900");
			weather_icon = gbitmap_create_with_resource(WEATHER_ICONS[0]);
		} else if ((weatherid % 801) == 0) {
			APP_LOG(APP_LOG_LEVEL_INFO, "Picking 801");
			if (hour >= 18) { // If it's past 6PM, display the night variant (partly cloudy moon)
				weather_icon = gbitmap_create_with_resource(WEATHER_ICONS[9]);
			} else { // Display the day varient (partly cloudy sun)
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
		
		// Put it in the right place
		GRect icon = gbitmap_get_bounds(weather_icon);
		layer_set_frame(weathericon_layer, GRect(PBL_IF_ROUND_ELSE(165 - (icon.size.w / 2), 133 - (icon.size.w / 2)), 73, 20, 23));
		layer_mark_dirty(weathericon_layer);
	}
	
	if (lang_tup) {
		APP_LOG(APP_LOG_LEVEL_INFO, "KEY_LANGUAGE received!");
  	if (strcmp(lang_tup->value->cstring, "en") == 0) {
  		APP_LOG(APP_LOG_LEVEL_INFO, "Using English");
  		lang = 0;
  	} else if (strcmp(lang_tup->value->cstring, "fr") == 0){
  		APP_LOG(APP_LOG_LEVEL_INFO, "Using French");
  		lang = 1;
  	} else if (strcmp(lang_tup->value->cstring, "es") == 0){
  		APP_LOG(APP_LOG_LEVEL_INFO, "Using Spanish");
  		lang = 2;
  	} else if (strcmp(lang_tup->value->cstring, "de") == 0){
  		APP_LOG(APP_LOG_LEVEL_INFO, "Using German");
  		lang = 3;
		} else if (strcmp(lang_tup->value->cstring, "ru") == 0) {
			APP_LOG(APP_LOG_LEVEL_INFO, "Using Russian");
  		lang = 4;
		} else if (strcmp(lang_tup->value->cstring, "it") == 0) {
			APP_LOG(APP_LOG_LEVEL_INFO, "Using Russian");
  		lang = 5;
		} else if (strcmp(lang_tup->value->cstring, "po") == 0) {
			APP_LOG(APP_LOG_LEVEL_INFO, "Using Russian");
  		lang = 6;
		} else if (strcmp(lang_tup->value->cstring, "uk") == 0) {
			APP_LOG(APP_LOG_LEVEL_INFO, "Using Ukrainian");
  		lang = 7;
		} else {
  		lang = 0;
  	}
		
		persist_write_int(KEY_LANG, lang);
		update_time();
	}
	
	if (vibeconnect_tup) {
		vibes_short_pulse();
  	APP_LOG(APP_LOG_LEVEL_INFO, "KEY_VIBE_ON_CONNECT received!");
  	vibe_on_connect = vibeconnect_tup->value->int8;
		
		persist_write_int(KEY_VIBE_ON_CONNECT, vibe_on_connect);
  }

  if (vibedisconnect_tup) {
  	APP_LOG(APP_LOG_LEVEL_INFO, "KEY_VIBE_ON_DISCONNECT received!");
  	vibe_on_disconnect = vibedisconnect_tup->value->int8;
		
		persist_write_int(KEY_VIBE_ON_DISCONNECT, vibe_on_connect);
  }
	
	if (colourscheme_tup) {
		APP_LOG(APP_LOG_LEVEL_INFO, "KEY_COLOUR_SCHEME received!");
		colourscheme = colourscheme_tup->value->int8;
		APP_LOG(APP_LOG_LEVEL_INFO, "Colour scheme is %d", colourscheme);
		persist_write_int(KEY_COLOUR_SCHEME, colourscheme);
		pick_colours();
	}
	
	if (updatetime_tup) {
		APP_LOG(APP_LOG_LEVEL_INFO, "KEY_UPDATE_TIME received!");
		weatherupdatetime = updatetime_tup->value->int8;
		APP_LOG(APP_LOG_LEVEL_INFO, "Weather update time is %d", weatherupdatetime);
		persist_write_int(KEY_UPDATE_TIME, weatherupdatetime);
	}
}

void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped");
}

void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed");
}

void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}