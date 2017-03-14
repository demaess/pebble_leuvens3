#include "pebble.h"

//CANVAS     ;
static Window *window;

//BATTERY
static uint8_t battery_level;
static bool battery_plugged;
static GBitmap *icon_battery_charge;
static Layer *battery_layer;

//BLUETOOTH


bool was_BTconnected_last_time;
// InverterLayer *inv_layer;	

#define GColorBlack uint8_t(0b11000000);

GColor8
	gcolor_legible_over(GColor8 background_color);
//GColor8 background_color = GColorBlack;	
GColor8 background_color = GColorBlack;




//FONT
static GFont font_thin;
static GFont font_thick;
static GFont font_small;
static GFont font_smallest;
//static GFont font_tiny;

//TEXT LAYERS
typedef struct {
	TextLayer * layer;
	PropertyAnimation *anim;
	const char * text;
	const char * old_text;
} word_t;

static word_t obere_minute;
static word_t untere_minute;
static word_t stunde;
static word_t datum_gross;
static word_t datum_klein;

static const char *hours[] = {"twèlef","ieën","twieë","droë","vier","voëf","zes","zeive","acht","neige","tien","elf","twèlef"};

TextLayer *text_layer_setup(Window * window, GRect frame, GFont font) {
	TextLayer *layer = text_layer_create(frame);
	text_layer_set_text(layer, "");
	text_layer_set_text_color(layer, GColorWhite);
	text_layer_set_background_color(layer, GColorClear);
	text_layer_set_text_alignment(layer, GTextAlignmentCenter);
	text_layer_set_font(layer, font);
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(layer));
	return layer;
}

//ANIMATION
static void update_word(word_t * const word) {
	if(word->text != word->old_text) {
		// Move the layer offscreen before changing text.
		// Without this, the new text will be visible briefly before the animation starts.
	//	layer_set_frame(text_layer_get_layer(word->layer), word->anim->values.from.grect);
	//	animation_schedule(&word->anim->animation);
	}
	text_layer_set_text(word->layer, word->text);
}

//SET TIMES
static void steirisch_format(int mm, int dd, int h, int m, int s) {
	obere_minute.text = "";
	untere_minute.text = "";
	stunde.text = "";
	datum_gross.text = "";
	datum_klein.text = "";
	
	//MINUTES
	if 		((m == 59 && s >= 30) || m > 59)				{ obere_minute.text = "";   		 untere_minute.text = "krak";			h++; }	
	else if ((m == 57 && s >= 30) || m > 57) 				{ obere_minute.text = "";          untere_minute.text = "apeprè";   	    h++; }
    else if ((m == 52 && s >= 30) || m > 52) 				{ obere_minute.text = "voëf";      untere_minute.text = "vei";  	    	h++; }
    else if ((m == 47 && s >= 30) || m > 47) 				{ obere_minute.text = "tien";       untere_minute.text = "vei";   	    h++; }
    else if ((m == 42 && s >= 30) || m > 42) 				{ obere_minute.text = "";   	 	 untere_minute.text = "kotie vei";   	h++; }
    else if ((m == 37 && s >= 30) || m > 37) 				{ obere_minute.text = "tien noo";  untere_minute.text = "halver"; 	    h++; }
    else if ((m == 32 && s >= 30) || m > 32) 				{ obere_minute.text = "voëf noo"; untere_minute.text = "halver";       	h++; }
    else if ((m == 27 && s >= 30) || m > 27) 				{ obere_minute.text = "";          untere_minute.text = "halver";       	h++; }
    else if ((m == 22 && s >= 30) || m > 22) 				{ obere_minute.text = "voëf van";  untere_minute.text = "halver";       	h++; }
    else if ((m == 17 && s >= 30) || m > 17) 				{ obere_minute.text = "tien van";   untere_minute.text = "halver";       	h++; }
	else if ((m == 12 && s >= 30) || m > 12) 				{ obere_minute.text = "";    		 untere_minute.text = "kotie noo";	}		//h++; }   //JDM
	else if ((m ==  7 && s >= 30) || m >  7) 				{ obere_minute.text = "tien";       untere_minute.text = "noo";       		 } 
    else if ((m ==  2 && s >= 30) || m >  2) 				{ obere_minute.text = "voàf";      untere_minute.text = "noo";      		 }
	else if ((m ==  0 && s >= 30) || m > 0)					{ obere_minute.text = "";     	 untere_minute.text = "verboä";                   }
	else if ( m ==  0 && s <  30)							{ obere_minute.text = "";     	 untere_minute.text = "krak";  		     }                                                                                          
	else if (h == 12)                                       { obere_minute.text = "";     	 untere_minute.text = "";          }// JDM      
	
	//HOURS
	if 	(h < 12)  				stunde.text = hours[h];
   	else    				 	stunde.text = hours[h - 12];
   	if 	(h == 0 || h == 12)		stunde.text = hours[12];
	if  (h == 0)                stunde.text = "spuekier";                                                                       // JDM
	if  ((h == 12) &&  ( m ==  0 && s <  30))           {  stunde.text = "tis neun";   obere_minute.text = "";     	 untere_minute.text = ""; }  // JDM

	//DAYS
	if    	     (dd == 31)				{	datum_gross.text = "";					datum_klein.text = "den ieënendettigste";  }
	else if      (dd == 30)				{	datum_gross.text = "";					datum_klein.text = "den dettigste";     }
    else if      (dd == 29)				{	datum_gross.text = "";					datum_klein.text = "den negeentwintegste"; }
    else if      (dd == 28)				{	datum_gross.text = "";					datum_klein.text = "den achtentwintegste"; }	
    else if      (dd == 27)				{	datum_gross.text = "";					datum_klein.text = "den zeiveentwintegste"; }
    else if      (dd == 26)				{	datum_gross.text = "";					datum_klein.text = "den zesentwintegste";  }
    else if      (dd == 25)				{	datum_gross.text = "";					datum_klein.text = "den voëfentwintegste"; }
    else if      (dd == 24)				{	datum_gross.text = "";					datum_klein.text = "den vierentwintegste"; }
    else if      (dd == 23)				{	datum_gross.text = "";					datum_klein.text = "den droëentwintegste"; }
    else if      (dd == 22)				{	datum_gross.text = "";					datum_klein.text = "den twieëentwintegste"; }// <---------
    else if      (dd == 21)				{	datum_gross.text = "";					datum_klein.text = "den ieënentwintegste";   }
	else if      (dd == 20)				{	datum_gross.text = "den twintegste";		datum_klein.text = "";					 }
    else if      (dd == 19)				{	datum_gross.text = "";		datum_klein.text = "den neigetiende";					 }
    else if      (dd == 18)				{	datum_gross.text = "den achttiende";		datum_klein.text = "";					 }
    else if      (dd == 17)				{	datum_gross.text = "";		datum_klein.text = "de zeivetiende";					 }
    else if      (dd == 16)				{	datum_gross.text = "de zestiende";		datum_klein.text = "";					 }
    else if      (dd == 15)				{	datum_gross.text = "de voëftiende";		datum_klein.text = "";					 }
    else if      (dd == 14)				{	datum_gross.text = "de fieëtiende";		datum_klein.text = "";					 }
    else if      (dd == 13)				{	datum_gross.text = "den dèttiende";		datum_klein.text = "";					 }
    else if      (dd == 12)				{	datum_gross.text = "den twèlefde";			datum_klein.text = "";					 }
    else if      (dd == 11)				{	datum_gross.text = "den elfde";			datum_klein.text = "";					 }
    else if      (dd == 10)				{	datum_gross.text = "den tindes";			datum_klein.text = "";					 }
    else if      (dd ==  9)				{	datum_gross.text = "de neigende";			datum_klein.text = "";					 }
    else if      (dd ==  8)				{	datum_gross.text = "den achtste";			datum_klein.text = "";					 }
    else if      (dd ==  7)				{	datum_gross.text = "de zeivende";			datum_klein.text = "";					 }
    else if      (dd ==  6)				{	datum_gross.text = "de zesdes";			datum_klein.text = "";					 }
    else if      (dd ==  5)				{	datum_gross.text = "de voëfdes"; 		datum_klein.text = "";					 }
    else if      (dd ==  4)				{	datum_gross.text = "de vierdes";			datum_klein.text = "";					 }
    else if      (dd ==  3)				{	datum_gross.text = "den dèrdes";			datum_klein.text = "";					 }
    else if      (dd ==  2)				{	datum_gross.text = "den twieèdes";			datum_klein.text = "";					 }
    else if      (dd ==  1)				{	datum_gross.text = "den ieëste";			datum_klein.text = "";					 }
	
}



//UPDATE WATCHFACE
static void handle_tick(struct tm *tm, TimeUnits units_changed) {
	if (was_BTconnected_last_time)  {
		window_set_background_color(window, background_color);
		
		obere_minute.old_text = obere_minute.text;
		untere_minute.old_text = untere_minute.text;
		stunde.old_text = stunde.text;
		datum_gross.old_text = datum_gross.text;
		datum_klein.old_text = datum_klein.text;

		steirisch_format(tm->tm_mon, tm->tm_mday, tm->tm_hour,  tm->tm_min,  tm->tm_sec);

		update_word(&obere_minute);
		update_word(&untere_minute);
		update_word(&stunde);
		update_word(&datum_gross);
		update_word(&datum_klein);
	}
	else  {
		// inverter_layer_destroy(inv_layer);
        window_set_background_color(window, background_color);
		
		obere_minute.old_text = obere_minute.text;
		untere_minute.old_text = untere_minute.text;
		stunde.old_text = stunde.text;
		datum_gross.old_text = datum_gross.text;
		datum_klein.old_text = datum_klein.text;

		steirisch_format(tm->tm_mon, tm->tm_mday, tm->tm_hour,  tm->tm_min,  tm->tm_sec);

		update_word(&obere_minute);
		update_word(&untere_minute);
		update_word(&stunde);
		update_word(&datum_gross);
		update_word(&datum_klein);

	//	inv_layer = inverter_layer_create(GRect(0, 0, 144, 168));
     //   layer_add_child(window_get_root_layer(window), (Layer*) inv_layer); 
	}
}

//HANDLE TEXT LAYERS
static void text_layer(word_t * word, GRect frame, GFont font) {
	word->layer = text_layer_setup(window, frame, font);

	GRect frame_right = frame;
	frame_right.origin.x = 150;

	word->anim = property_animation_create_layer_frame(text_layer_get_layer(word->layer), &frame_right, &frame);
//	animation_set_duration(&word->anim->animation, 500);
//	animation_set_curve(&word->anim->animation, AnimationCurveEaseIn);
}

void word_destroy(word_t * word) {
	property_animation_destroy(word->anim);
	text_layer_destroy(word->layer);
}

//BATTERY LAYER
void battery_layer_update_callback(Layer *layer, GContext *ctx) {

  graphics_context_set_compositing_mode(ctx, GCompOpAssign);

  if (!battery_plugged) {
    graphics_context_set_stroke_color(ctx, GColorBlack);
    graphics_context_set_fill_color(ctx, GColorWhite);
	graphics_fill_rect(ctx, GRect(0, 5, 144, 4), 0, GCornerNone);  
	  
    graphics_context_set_stroke_color(ctx, GColorWhite);
    graphics_context_set_fill_color(ctx, GColorBlack);
	graphics_fill_rect(ctx, GRect(2, 6, (uint8_t)(battery_level)*1.4, 2), 0, GCornerNone);
	  
	graphics_draw_line(ctx, GPoint(15,6), GPoint(15,7));
	graphics_draw_line(ctx, GPoint(29,6), GPoint(29,7));
	graphics_draw_line(ctx, GPoint(43,6), GPoint(43,7));
	graphics_draw_line(ctx, GPoint(57,6), GPoint(57,7));
	graphics_draw_line(ctx, GPoint(71,6), GPoint(71,7));
	graphics_draw_line(ctx, GPoint(85,6), GPoint(85,7));
	graphics_draw_line(ctx, GPoint(99,6), GPoint(99,7));
	graphics_draw_line(ctx, GPoint(113,6), GPoint(113,7));
	graphics_draw_line(ctx, GPoint(127,6), GPoint(127,7));
	  
  } 
  else {
    graphics_draw_bitmap_in_rect(ctx, icon_battery_charge, GRect(0, 0, 144, 12));
  }
}

//HANDLE BATTERY
void battery_state_handler(BatteryChargeState charge) {
        battery_level = charge.charge_percent;
        battery_plugged = charge.is_plugged;
        layer_mark_dirty(battery_layer);
}

//HANDLE BLUETOOTH
void bluetooth_handler(bool connected) {
        // This handler is called when BT connection state changes
        // Destroy inverter layer if BT changed from disconnected to connected
        if ((connected) && (!(was_BTconnected_last_time)))  {
       // 	inverter_layer_destroy(inv_layer);
            vibes_short_pulse();			
        }
        time_t now = time(NULL);
    	struct tm *tm = localtime(&now);
        window_set_background_color(window, background_color);
        //Inverter layer in case of disconnect
        if (!(connected)) {
       //     inv_layer = inverter_layer_create(GRect(0, 0, 144, 168));
        //    layer_add_child(window_get_root_layer(window), (Layer*) inv_layer);        
            vibes_long_pulse();
        }
        was_BTconnected_last_time = connected;
}

//INITIALIZE WATCHFACE
static void init() {

	window = window_create();
	window_stack_push(window, false);
	
	//BLUETOOTH
	was_BTconnected_last_time = bluetooth_connection_service_peek();            
	bluetooth_handler(was_BTconnected_last_time);
	bluetooth_connection_service_subscribe( bluetooth_handler );
	
	//BATTERY
    icon_battery_charge = gbitmap_create_with_resource(RESOURCE_ID_BATTERY_CHARGE);	
	BatteryChargeState initial = battery_state_service_peek();
    battery_level = initial.charge_percent;
    battery_plugged = initial.is_plugged;
    battery_layer = layer_create(GRect(0,124,144,12)); //SIZE = 144*12
    layer_set_update_proc(battery_layer, &battery_layer_update_callback);
    layer_add_child(window_get_root_layer(window), battery_layer);
		
	//FONTS
    font_thin = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_THIN_32));
    font_thick = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_THICK_32));
	font_small = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_THIN_20));
	font_smallest = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_THIN_14));
	//font_tiny = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_TINY_10));
	
	//TEXT
	text_layer(&obere_minute, GRect(0, 12, 143, 48), font_thin);
	text_layer(&untere_minute, GRect(0, 46, 143, 48), font_thin);
	text_layer(&stunde, GRect(0, 80, 143, 42), font_thick);
	text_layer(&datum_gross, GRect(0, 136, 143, 48), font_small);
	text_layer(&datum_klein, GRect(0, 140, 143, 48), font_smallest);

	
	tick_timer_service_subscribe(MINUTE_UNIT, handle_tick);
}

//TERMINATE WATCHFACE
static void deinit() {
	tick_timer_service_unsubscribe();
	word_destroy(&datum_klein);
	word_destroy(&datum_gross);
	word_destroy(&stunde);
	word_destroy(&untere_minute);
	word_destroy(&obere_minute);
	fonts_unload_custom_font(font_thin);
	fonts_unload_custom_font(font_thick);
	fonts_unload_custom_font(font_small);
	fonts_unload_custom_font(font_smallest);

	window_destroy(window);
	//BLUETOOTH
	// Destroy inverter if last screen was inverted
    if (!(was_BTconnected_last_time))  {
   //     inverter_layer_destroy(inv_layer);
    }
	bluetooth_connection_service_unsubscribe();
	battery_state_service_unsubscribe();
	//BATTERY
    gbitmap_destroy(icon_battery_charge);
	layer_destroy(battery_layer);
}

//MAIN (RUN)
int main(void) {
	init();
	battery_state_service_subscribe (&battery_state_handler);
	app_event_loop();
	deinit();
}
