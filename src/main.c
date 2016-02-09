#include <pebble.h>
#include "languages.h"
#include "messaging.h"
#include "gbitmap_color_palette_manipulator.h"

static Window *main_window;
static Layer *horz_rect_layer, *diag_rect_layer, *batt_layer, *bt_layer;
static TextLayer *time_layer, *date_layer;
static GBitmap *batt_icon, *bt_icon, *batt_sprites;
static GFont time_font, date_font;

// Accessible across files:
AppTimer *weather_timeout, *ready_timeout;
GBitmap *weather_icon = NULL;
TextLayer *temp_layer;
Layer *weathericon_layer;

//static int lang = 4; // Hardcoded for testing
int lang;
int timeout = 60000;

// Config options
bool use_celsius = 1;
bool show_weather = 1;
bool vibe_on_connect = 0;
bool vibe_on_disconnect = 1;
int colourscheme = 0;
int weatherupdatetime = 60;
//static int testscheme = 4;
static bool usewhiteicons;

// Colours for schemes of the colour variety
static GColor horz;
static GColor horzdrop;
static GColor diag;
static GColor diagdrop;

static GPath *horz_rect, *diag_rect, *horz_drop, *diag_drop = NULL;

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

/*const int BATT_ICONS[] = {
	RESOURCE_ID_BATT_LOW,			// 0
	RESOURCE_ID_BATT_20,			// 1
	RESOURCE_ID_BATT_40,			// 2
	RESOURCE_ID_BATT_60,			// 3
	RESOURCE_ID_BATT_80,			// 4
	RESOURCE_ID_BATT_FULL,		// 5
	RESOURCE_ID_BATT_CHARGING	// 6
};*/

void on_animation_stopped(Animation *anim, bool finished, void *context) {
    //Free the memory used by the Animation
    property_animation_destroy((PropertyAnimation*) anim);
}

void animate_layer(Layer *layer, GRect *start, GRect *finish, int duration, int delay) {
    //Declare animation
    PropertyAnimation *anim = property_animation_create_layer_frame(layer, start, finish);
 
    //Set characteristics
    animation_set_duration((Animation*) anim, duration);
    animation_set_delay((Animation*) anim, delay);
 
    //Set stopped handler to free memory
    AnimationHandlers handlers = {
        //The reference to the stopped handler is the only one in the array
        .stopped = (AnimationStoppedHandler) on_animation_stopped
    };
    animation_set_handlers((Animation*) anim, handlers, NULL);
 
    //Start animation!
    animation_schedule((Animation*) anim);
}

void update_time() {
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
	
  static char time_buffer[] = "00:00"; // Buffer for full time (24h)
	static char min_buffer[] = "00"; // Buffer for minutes
  static char date_buffer[15]; // Buffer for entire date to display
  
	int hour = tick_time->tm_hour; // Get the current hour
	
	if (hour > 12) { // Convert to 12h time if needed
		hour = hour - 12;
	} else if (hour == 0) { // Show midnight as 12
		hour = 12;
	}
	
  if(clock_is_24h_style() == true) {
    strftime(time_buffer, sizeof("00:00"), "%H:%M", tick_time); // Display time_buffer if 24h time is selected
  } else {
    strftime(min_buffer, sizeof(min_buffer), "%M", tick_time); // Grab the minute from strftime
		snprintf(time_buffer, sizeof(time_buffer), "%d:%s", hour, min_buffer); // Combine our interger hour and strftime min
  }
	
	text_layer_set_text(time_layer, time_buffer); // Display the time info

	int day = tick_time->tm_mday;
	int weekday = tick_time->tm_wday; // Get current weekday as an integer (0 is Sunday)
	
	// Select the correct strings from languages.c and write to buffer along with date
	snprintf(date_buffer, sizeof(date_buffer), "%s %d", dayNames[lang][weekday], day);
	
	text_layer_set_text(date_layer, date_buffer); // Display the date info
}

static void init_animations() {
	// Move things all fancy like
	GRect bounds = layer_get_bounds(window_get_root_layer(main_window));
	GSize time_size = text_layer_get_content_size(time_layer);
	GSize date_size = text_layer_get_content_size(date_layer);
	
	GRect timestart = GRect(0 - time_size.w, PBL_IF_ROUND_ELSE(20, 0), time_size.w, time_size.h);
	GRect timeend = GRect(PBL_IF_ROUND_ELSE(27, 7), PBL_IF_ROUND_ELSE(20, 0), time_size.w, time_size.h);
	
	GRect datestart = GRect(0 - time_size.w, PBL_IF_ROUND_ELSE(time_size.h + 16, time_size.h - 2), date_size.w, date_size.h);
	GRect dateend = GRect(PBL_IF_ROUND_ELSE(29, 9), PBL_IF_ROUND_ELSE(time_size.h + 16, time_size.h - 2), date_size.w, date_size.h);
	
	GRect diagstart = GRect(200, 200, bounds.size.w, bounds.size.h);
	GRect diagend = GRect(0, 0, bounds.size.w, bounds.size.h);
	
	GRect horzstart = GRect(0, 200, bounds.size.w, bounds.size.h);
	GRect horzend = GRect(0, 0, bounds.size.w, bounds.size.h);
	
	animate_layer(text_layer_get_layer(time_layer), &timestart, &timeend, 500, 0);
	animate_layer(text_layer_get_layer(date_layer), &datestart, &dateend, 500, 100);
	
	animate_layer(diag_rect_layer, &diagstart, &diagend, 500, 0);
	animate_layer(horz_rect_layer, &horzstart, &horzend, 500, 0);
	
	update_time();
}

static void batt_handler(BatteryChargeState state) {
	APP_LOG(APP_LOG_LEVEL_INFO, "Start batt_handler");
	int pct = state.charge_percent;
	bool charging = state.is_charging;
	
	if (batt_sprites) {
		APP_LOG(APP_LOG_LEVEL_INFO, "Destroying batt_sprites in batt_handler");
		gbitmap_destroy(batt_sprites);
	}
	
	if (usewhiteicons == 1) { // If white icons are required by the colour scheme, load the correct PNG
		APP_LOG(APP_LOG_LEVEL_INFO, "Using white icons");
		batt_sprites = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATT_SPRITES_WHITE);
	} else {
		APP_LOG(APP_LOG_LEVEL_INFO, "Using black icons");
		batt_sprites = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATT_SPRITES);
	}
	
	if (batt_icon) {
		APP_LOG(APP_LOG_LEVEL_INFO, "Destroying batT_icon in batt_handler");
		gbitmap_destroy(batt_icon);
	}
	
	/*if (charging) {
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
	}*/
	
	if (charging) {
		batt_icon = gbitmap_create_as_sub_bitmap(batt_sprites, GRect(70, 0, 14, 26));
	} else if (pct <= 10) {
		batt_icon = gbitmap_create_as_sub_bitmap(batt_sprites, GRect(84, 0, 14, 26));
	} else if (pct <= 20) {
		batt_icon = gbitmap_create_as_sub_bitmap(batt_sprites, GRect(0, 0, 14, 26));
	} else if (pct <= 40) {
		batt_icon = gbitmap_create_as_sub_bitmap(batt_sprites, GRect(14, 0, 14, 26));
	} else if (pct <= 60) {
		batt_icon = gbitmap_create_as_sub_bitmap(batt_sprites, GRect(28, 0, 14, 26));
	} else if (pct <= 80) {
		batt_icon = gbitmap_create_as_sub_bitmap(batt_sprites, GRect(42, 0, 14, 26));
	} else if (pct <= 100) {
		batt_icon = gbitmap_create_as_sub_bitmap(batt_sprites, GRect(56, 0, 14, 26));
	}
	
	layer_mark_dirty(batt_layer);
}

static void bt_handler(bool connected) {
	if (bt_icon) {
		APP_LOG(APP_LOG_LEVEL_INFO, "Destroying bt_icon in bt_handler");
		gbitmap_destroy(bt_icon);
	}
	
	if (connected) {
		if (vibe_on_connect == 1) {
			vibes_double_pulse();
		}
		
	} else {
		if (vibe_on_disconnect == 1) {
			vibes_long_pulse();
		}
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

void pick_colours() {
	if (colourscheme == 0) { // Classic
		horz = GColorFromRGB(255, 167, 38);
		horzdrop = GColorFromRGB(245, 124, 0);
		diag = GColorFromRGB(33, 150, 243);
		diagdrop = GColorFromRGB(13, 71, 161);
		text_layer_set_text_color(time_layer, GColorBlack);
		text_layer_set_text_color(date_layer, GColorBlack);
		text_layer_set_text_color(temp_layer, GColorBlack);
		window_set_background_color(main_window, GColorFromRGB(38, 166, 154));
		usewhiteicons = 0;
	} else if (colourscheme == 1) { // Pop Tart (Yellow, green, purple)
		horz = GColorFromRGB(255, 255, 170);
		horzdrop = GColorFromRGB(255, 196, 0);
		diag = GColorFromRGB(170, 255, 85);
		diagdrop = GColorFromRGB(85, 255, 0);
		text_layer_set_text_color(time_layer, GColorWhite);
		text_layer_set_text_color(date_layer, GColorWhite);
		text_layer_set_text_color(temp_layer, GColorBlack);
		window_set_background_color(main_window, GColorFromRGB(170, 85, 255));
		usewhiteicons = 0;
	} else if (colourscheme == 2) { // Lemon Splash (Yellow, yellow, black)
		horz = GColorFromRGB(255, 255, 0);
		horzdrop = GColorFromRGB(170, 170, 0);
		diag = GColorFromRGB(255, 255, 0);
		diagdrop = GColorFromRGB(170, 170, 0);
		text_layer_set_text_color(time_layer, GColorYellow);
		text_layer_set_text_color(date_layer, GColorYellow);
		text_layer_set_text_color(temp_layer, GColorBlack);
		window_set_background_color(main_window, GColorFromRGB(0, 0, 0));
		usewhiteicons = 0;
	} else if (colourscheme == 3) { // Frozen Yoghurt (Purple, blue, yellow)
		horz = GColorFromRGB(170, 85, 170);
		horzdrop = GColorFromRGB(170, 0, 170);
		diag = GColorFromRGB(0, 170, 170);
		diagdrop = GColorFromRGB(0, 85, 170);
		text_layer_set_text_color(time_layer, GColorBlack);
		text_layer_set_text_color(date_layer, GColorBlack);
		text_layer_set_text_color(temp_layer, GColorBlack);
		window_set_background_color(main_window, GColorFromRGB(255, 255, 170));
		usewhiteicons = 1;
	} else if (colourscheme == 4) { // Watermelon (Blue, green, red)
		horz = GColorFromRGB(85, 255, 170);
		horzdrop = GColorFromRGB(0, 255, 255);
		diag = GColorFromRGB(170, 255, 0);
		diagdrop = GColorFromRGB(85, 255, 0);
		text_layer_set_text_color(time_layer, GColorWhite);
		text_layer_set_text_color(date_layer, GColorWhite);
		text_layer_set_text_color(temp_layer, GColorBlack);
		window_set_background_color(main_window, GColorFromRGB(255, 85, 85));
		usewhiteicons = 0;
	} else if (colourscheme == 5) { // Popsicle (Orange, green, blue)
		horz = GColorFromRGB(255, 170, 85);
		horzdrop = GColorFromRGB(255, 170, 0);
		diag = GColorFromRGB(170, 255, 0);
		diagdrop = GColorFromRGB(85, 255, 0);
		text_layer_set_text_color(time_layer, GColorWhite);
		text_layer_set_text_color(date_layer, GColorWhite);
		text_layer_set_text_color(temp_layer, GColorBlack);
		window_set_background_color(main_window, GColorFromRGB(0, 85, 170));
		usewhiteicons = 0;
	} else { // Classic
		horz = GColorFromRGB(255, 167, 38);
		horzdrop = GColorFromRGB(245, 124, 0);
		diag = GColorFromRGB(33, 150, 243);
		diagdrop = GColorFromRGB(13, 71, 161);
		text_layer_set_text_color(time_layer, GColorWhite);
		text_layer_set_text_color(date_layer, GColorWhite);
		text_layer_set_text_color(temp_layer, GColorWhite);
		window_set_background_color(main_window, GColorFromRGB(38, 166, 154));
		usewhiteicons = 0;
	}
	
	layer_mark_dirty(bt_layer);
	//layer_mark_dirty(batt_layer);
	batt_handler(battery_state_service_peek());
}

static void draw_horz_rect(Layer *layer, GContext *ctx) {
	graphics_context_set_antialiased(ctx, true);
	
	/*if (colourscheme == 0) {
		graphics_context_set_fill_color(ctx, GColorFromRGB(245, 124, 0));
		gpath_draw_filled(ctx, horz_drop);

		graphics_context_set_fill_color(ctx, GColorFromRGB(255, 167, 38));
		gpath_draw_filled(ctx, horz_rect);

		graphics_context_set_stroke_color(ctx, GColorFromRGB(245, 124, 0));
		gpath_draw_outline(ctx, horz_rect);
	} else if (colourscheme == 1) {
		graphics_context_set_fill_color(ctx, GColorFromRGB(255, 196, 0));
		gpath_draw_filled(ctx, horz_drop);

		graphics_context_set_fill_color(ctx, GColorFromRGB(255, 255, 170));
		gpath_draw_filled(ctx, horz_rect);

		graphics_context_set_stroke_color(ctx, GColorFromRGB(255, 196, 0));
		gpath_draw_outline(ctx, horz_rect);
	}*/
	
	graphics_context_set_fill_color(ctx, horzdrop);
	gpath_draw_filled(ctx, horz_drop);

	graphics_context_set_fill_color(ctx, horz);
	gpath_draw_filled(ctx, horz_rect);

	graphics_context_set_stroke_color(ctx, horzdrop);
	gpath_draw_outline(ctx, horz_rect);
}

static void draw_diag_rect(Layer *layer, GContext *ctx) {
	graphics_context_set_antialiased(ctx, true);
	
	/*if (colourscheme == 0) {
		graphics_context_set_fill_color(ctx, GColorFromRGB(13, 71, 161));
		gpath_draw_filled(ctx, diag_drop);

		graphics_context_set_fill_color(ctx, GColorFromRGB(33, 150, 243));
		gpath_draw_filled(ctx, diag_rect);

		graphics_context_set_stroke_color(ctx, GColorFromRGB(13, 71, 161));
		gpath_draw_outline(ctx, diag_rect);
	} else if (colourscheme == 1) {
		graphics_context_set_fill_color(ctx, GColorFromRGB(13, 71, 161));
		gpath_draw_filled(ctx, diag_drop);

		graphics_context_set_fill_color(ctx, GColorFromRGB(25, 118, 210));
		gpath_draw_filled(ctx, diag_rect);

		graphics_context_set_stroke_color(ctx, GColorFromRGB(13, 71, 161));
		gpath_draw_outline(ctx, diag_rect);
	}*/
	
	graphics_context_set_fill_color(ctx, diagdrop);
	gpath_draw_filled(ctx, diag_drop);

	graphics_context_set_fill_color(ctx, diag);
	gpath_draw_filled(ctx, diag_rect);

	graphics_context_set_stroke_color(ctx, diagdrop);
	gpath_draw_outline(ctx, diag_rect);
}

static void draw_batt(Layer *layer, GContext *ctx) {
	APP_LOG(APP_LOG_LEVEL_INFO, "Drawing battery icon");
	graphics_context_set_compositing_mode(ctx, GCompOpSet);
	//replace_gbitmap_color(GColorBlack, gcolor_legible_over(horz), batt_icon, NULL); // Pick white or black for icon based on colour scheme
	graphics_draw_bitmap_in_rect(ctx, batt_icon, layer_get_bounds(batt_layer));
}

static void draw_bt(Layer *layer, GContext *ctx) {
	APP_LOG(APP_LOG_LEVEL_INFO, "Drawing BT icon");
	graphics_context_set_compositing_mode(ctx, GCompOpSet);
	//bt_icon = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_CONNECTED);
	if (bluetooth_connection_service_peek()) {
		bt_icon = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_CONNECTED);
	} else {
		bt_icon = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_DISCONNECTED);
	}
	replace_gbitmap_color(GColorBlack, gcolor_legible_over(horz), bt_icon, NULL);
	graphics_draw_bitmap_in_rect(ctx, bt_icon, layer_get_bounds(bt_layer));
}

static void draw_weathericon(Layer *layer, GContext *ctx) {
	APP_LOG(APP_LOG_LEVEL_INFO, "Drawing weather icon");
	graphics_context_set_compositing_mode(ctx, GCompOpSet);
	replace_gbitmap_color(GColorBlack, gcolor_legible_over(diag), weather_icon, weathericon_layer);
	graphics_draw_bitmap_in_rect(ctx, weather_icon, layer_get_bounds(weathericon_layer));
}

static void ready_ended() {
	// Is JS does not init fast enough show the error icon
	APP_LOG(APP_LOG_LEVEL_INFO, "Ready timer ended");
	
	if (ready_timeout != NULL) {
		APP_LOG(APP_LOG_LEVEL_INFO, "Ready timer is not NULL");
		weather_icon = gbitmap_create_with_resource(RESOURCE_ID_ICON_ERROR);
		layer_mark_dirty(weathericon_layer);
	}
}

static void main_window_load(Window *window) {
	setup_rects();
	
	GRect bounds = layer_get_bounds(window_get_root_layer(window));
	
	// Set up shapes
	diag_rect_layer = layer_create(GRect(200, 200, bounds.size.w, bounds.size.h));
	layer_set_update_proc(diag_rect_layer, draw_diag_rect);
	
	horz_rect_layer = layer_create(GRect(-200, 0, bounds.size.w, bounds.size.h));
	layer_set_update_proc(horz_rect_layer, draw_horz_rect);

	layer_add_child(window_get_root_layer(window), diag_rect_layer);
	layer_add_child(window_get_root_layer(window), horz_rect_layer);
	
	// Set up time & date (Commented frames are final positions)
	time_layer = text_layer_create(GRect(2, 0, bounds.size.w, bounds.size.h));
	//text_layer_set_text_color(time_layer, GColorWhite);
	text_layer_set_background_color(time_layer, GColorClear);
	time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_BLACK_42));
	text_layer_set_font(time_layer, fonts_get_system_font(FONT_KEY_LECO_42_NUMBERS));
	text_layer_set_text(time_layer, "00:00");
	GSize time_size = text_layer_get_content_size(time_layer);
	//layer_set_frame(text_layer_get_layer(time_layer), GRect(PBL_IF_ROUND_ELSE(27, 7), PBL_IF_ROUND_ELSE(20, 0), time_size.w, time_size.h));
	layer_set_frame(text_layer_get_layer(time_layer), GRect(0 - time_size.w, PBL_IF_ROUND_ELSE(20, 0), time_size.w, time_size.h));
	
	date_layer = text_layer_create(GRect(2, time_size.h + 3, bounds.size.w, bounds.size.h));
	//text_layer_set_text_color(date_layer, GColorWhite);
	text_layer_set_background_color(date_layer, GColorClear);
	date_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_24));
	text_layer_set_font(date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28));
	text_layer_set_text(date_layer, "MON 01");
	GSize date_size = text_layer_get_content_size(date_layer);
	//layer_set_frame(text_layer_get_layer(date_layer), GRect(PBL_IF_ROUND_ELSE(29, 9), PBL_IF_ROUND_ELSE(time_size.h + 16, time_size.h - 2), date_size.w, date_size.h));
	layer_set_frame(text_layer_get_layer(date_layer), GRect(0 - time_size.w, PBL_IF_ROUND_ELSE(time_size.h + 16, time_size.h - 2), date_size.w, date_size.h));
	
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(time_layer));
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(date_layer));
	
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
	weathericon_layer = layer_create(GRect(PBL_IF_ROUND_ELSE(160, 122), 73, 20, 23));
	layer_set_update_proc(weathericon_layer, draw_weathericon);
	
	
	//temp_layer = text_layer_create(GRect(PBL_IF_ROUND_ELSE(130, 121), 75, 144, 168));
	#ifdef PBL_ROUND 
		temp_layer = text_layer_create(GRect(125, 95, 40, 25));
	#else
		temp_layer = text_layer_create(GRect(104, 95, 40, 25));
	#endif
	text_layer_set_text_color(temp_layer, gcolor_legible_over(diag));
	text_layer_set_background_color(temp_layer, GColorClear);
	text_layer_set_text_alignment(temp_layer, GTextAlignmentCenter);
	text_layer_set_font(temp_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
	
	layer_add_child(diag_rect_layer, weathericon_layer);
	layer_add_child(diag_rect_layer, text_layer_get_layer(temp_layer));
	
	// Check for existing keys
	if (persist_exists(KEY_LANG)) {
		lang = persist_read_int(KEY_LANG);
	}
	
	if (persist_exists(KEY_USECELSIUS)) {
		use_celsius = persist_read_int(KEY_USECELSIUS);
	}
	
	if (persist_exists(KEY_SHOW_WEATHER)) {
		show_weather = persist_read_int(KEY_SHOW_WEATHER);
	}
	
	if (show_weather == 0) {
		layer_set_hidden(text_layer_get_layer(temp_layer), true);
		layer_set_hidden(weathericon_layer, true);
	} else {
		layer_set_hidden(text_layer_get_layer(temp_layer), false);
		layer_set_hidden(weathericon_layer, false);
	}
	
	if (persist_exists(KEY_COLOUR_SCHEME)) {
		colourscheme = persist_read_int(KEY_COLOUR_SCHEME);
		//colourscheme = testscheme;
	} else {
		//colourscheme = testscheme;
	}
	
	if (persist_exists(KEY_UPDATE_TIME)) {
		weatherupdatetime = persist_read_int(KEY_UPDATE_TIME);
	}
	
	pick_colours(); // Pick the proper colour scheme
	weather_icon = gbitmap_create_with_resource(RESOURCE_ID_ICON_LOADING);
}

static void main_window_unload(Window *window) {
	// http://bit.ly/1ZvbJIb
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
	gbitmap_destroy(weather_icon);
	gpath_destroy(horz_rect);
	gpath_destroy(diag_rect);
	gpath_destroy(horz_drop);
	gpath_destroy(diag_drop);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
	update_time();
	
	if(tick_time->tm_min % weatherupdatetime == 0) {
			if (weather_icon != NULL) {
				APP_LOG(APP_LOG_LEVEL_INFO, "Destroying weather icon in tick_handler");
				gbitmap_destroy(weather_icon);
			}
			if (show_weather == 1) {
				update_weather();
			}
	}
}

static void init() {
  main_window = window_create();

  window_set_window_handlers(main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  window_stack_push(main_window, true);
	
	APP_LOG(APP_LOG_LEVEL_INFO, "Waiting for ready signal from JS, starting ready_timeout...");
	ready_timeout = app_timer_register(timeout, ready_ended, NULL);
	
	tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
	battery_state_service_subscribe(batt_handler);
	bluetooth_connection_service_subscribe(bt_handler);
	
	init_appmessage(); // Start appmessaging in messaging.c
	init_animations(); // I like to move it move it
}

static void deinit() {
  window_destroy(main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}