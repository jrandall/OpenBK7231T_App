#include "drv_ws2812b_gpio.h"

#include "../new_common.h"
#include "../new_pins.h"
#include "../new_cfg.h"
// Commands register, execution API and cmd tokenizer
#include "../cmnds/cmd_public.h"
#include "../mqtt/new_mqtt.h"
#include "../logging/logging.h"
#include "../hal/hal_pins.h"
#include "../httpserver/new_http.h"

// values in ns
#define WS2812B_T0H	350 // 200ns - 500ns
#define WS2812B_T0L	900 // 750ns - 1050ns

#define WS2812B_T1H	900 // 750ns - 1050ns
#define WS2812B_T1L	350 // 200ns - 500ns

#define WS2812B_RESET 50000 // 50000ns+

static int g_pin_di = 0;
static bool g_high_value = false; // false if output is inverted, true if uninverted

#define WS2812B_SLEEP_350	__asm__("nop\nnop\nnop");
#define WS2812B_SLEEP_900	__asm__("nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop");
#define WS2812B_SLEEP_50000	__asm__("nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop");

#define WS2812B_SEND_T0 HAL_PIN_SetOutputValue(g_pin_di,g_high_value); WS2812B_SLEEP_350; HAL_PIN_SetOutputValue(g_pin_di,!g_high_value); WS2812B_SLEEP_900;
#define WS2812B_SEND_T1 HAL_PIN_SetOutputValue(g_pin_di,g_high_value); WS2812B_SLEEP_900; HAL_PIN_SetOutputValue(g_pin_di,!g_high_value); WS2812B_SLEEP_350;
#define WS2812B_SEND_RESET HAL_PIN_SetOutputValue(g_pin_di,!g_high_value); WS2812B_SLEEP_50000;

#ifndef CHECK_BIT
#define CHECK_BIT(var,pos) ((var) & (1<<(pos)))
#endif

#define WS2812B_SEND_BIT(var,pos) if(((var) & (1<<(pos)))) { WS2812B_SEND_T1; } else { WS2812B_SEND_T0; };

__attribute__ ((noinline))
static void WS2812B_Send(byte *data, int dataSize){
	byte b;
	WS2812B_SEND_RESET; // 62us
	for(int i = 0; i < 20; i++) {
	  HAL_PIN_SetOutputValue(g_pin_di,!g_high_value); // 1.92us
	  HAL_PIN_SetOutputValue(g_pin_di,g_high_value); // 1.92us (i=16, 77.79us)
	}
	WS2812B_SEND_RESET; // 62us
	for(int i = 0; i < 20; i++) {
	  HAL_PIN_SetOutputValue(g_pin_di,!g_high_value);
	  __asm__("nop");
	  HAL_PIN_SetOutputValue(g_pin_di,g_high_value);
	}
	WS2812B_SEND_RESET;
	for(int i = 0; i < 20; i++) {
	  HAL_PIN_SetOutputValue(g_pin_di,!g_high_value);
	  __asm__("nop\nnop");
	  HAL_PIN_SetOutputValue(g_pin_di,g_high_value);
	}
	WS2812B_SEND_RESET;
	for(int i = 0; i < 20; i++) {
	  HAL_PIN_SetOutputValue(g_pin_di,!g_high_value);
	  __asm__("nop\nnop\nnop");
	  HAL_PIN_SetOutputValue(g_pin_di,g_high_value);
	}
	WS2812B_SEND_RESET;
	for(int i = 0; i < dataSize; i++) {
		b = data[i];
		WS2812B_SEND_BIT(b,0);
		WS2812B_SEND_BIT(b,1);
		WS2812B_SEND_BIT(b,2);
		WS2812B_SEND_BIT(b,3);
		WS2812B_SEND_BIT(b,4);
		WS2812B_SEND_BIT(b,5);
		WS2812B_SEND_BIT(b,6);
		WS2812B_SEND_BIT(b,7);
	}
}

typedef struct {
  UINT8 green;
  UINT8 red;
  UINT8 blue;
} ws2812b_gpio_pixel_t;

ws2812b_gpio_pixel_t *g_pixels;
BOOLEAN ws2812b_gpio_initialized = false;
uint32_t ws2812b_gpio_pixel_count = 0;

// startDriver WS2812B_GPIO
void WS2812B_GPIO_Init(void) {
  g_pin_di = PIN_FindPinIndexForRole(IOR_WS2812B_DIN, g_pin_di);

  HAL_PIN_Setup_Output(g_pin_di);
  
  //cmddetail:{"name":"WS2812B_GPIO_Init","args":"WS2812B_GPIO_Init_cmd",
  //cmddetail:"descr":"Initialize WS2812B_GPIO driver for an <N> light string",
  //cmddetail:"fn":"NULL);","file":"driver/drv_ws2812b_gpio.c","requires":"",
  //cmddetail:"examples":"WS2812B_GPIO_Init <N>"}
  CMD_RegisterCommand("WS2812B_GPIO_Init", WS2812B_GPIO_Init_cmd, NULL);
  
  //cmddetail:{"name":"WS2812B_GPIO_SetPixel","args":"WS2812B_GPIO_SetPixel_cmd",
  //cmddetail:"descr":"",
  //cmddetail:"fn":"NULL);","file":"driver/drv_ws2812b_gpio.c","requires":"",
  //cmddetail:"examples":""}
  CMD_RegisterCommand("WS2812B_GPIO_SetPixel", WS2812B_GPIO_SetPixel_cmd, NULL);
  
  //cmddetail:{"name":"WS2812B_GPIO_Test","args":"WS2812B_GPIO_Test_cmd",
  //cmddetail:"descr":"",
  //cmddetail:"fn":"NULL);","file":"driver/drv_ws2812b_gpio.c","requires":"",
  //cmddetail:"examples":""}
  CMD_RegisterCommand("WS2812B_GPIO_Test", WS2812B_GPIO_Test_cmd, NULL);
  
  ADDLOG_INFO(LOG_FEATURE_CMD, "WS2812B_GPIO_Init: commands registered");
}

commandResult_t WS2812B_GPIO_Init_cmd(const void *context, const char *cmd, const char *args, int flags) {
  if (ws2812b_gpio_initialized) {
    ADDLOG_INFO(LOG_FEATURE_CMD, "WS2812B_GPIO is already initialized with %i LEDs", ws2812b_gpio_pixel_count);
    return CMD_RES_ERROR;
  }
  
  Tokenizer_TokenizeString(args, 0);
  
  if (Tokenizer_GetArgsCount() != 1) {
    ADDLOG_INFO(LOG_FEATURE_CMD, "Not Enough Arguments for init WS2812B_GPIO_Init: amount of LEDs missing");
    return CMD_RES_NOT_ENOUGH_ARGUMENTS;
  }
  
  ws2812b_gpio_pixel_count = Tokenizer_GetArgIntegerRange(0, 0, 255);
  ADDLOG_INFO(LOG_FEATURE_CMD, "WS2812B_GPIO_Init_cmd register driver with %i LEDs", ws2812b_gpio_pixel_count);
  
  // Prepare buffer
  g_pixels = (ws2812b_gpio_pixel_t *)os_malloc(ws2812b_gpio_pixel_count * sizeof(ws2812b_gpio_pixel_t));
  
  // Initialize all pixels
  for (int i = 0; i < ws2812b_gpio_pixel_count; i++) {
    g_pixels[i].red = 0;
    g_pixels[i].green = 0;
    g_pixels[i].blue = 0;
  }
  
  ws2812b_gpio_initialized = true;
  
  ADDLOG_INFO(LOG_FEATURE_CMD, "WS2812B_GPIO_Init_cmd driver initialized for %i pixels", ws2812b_gpio_pixel_count);
  return CMD_RES_OK;
}

commandResult_t WS2812B_GPIO_SetPixel_cmd(const void *context, const char *cmd, const char *args, int flags) {
  int pixel;
  int r;
  int g;
  int b;
  Tokenizer_TokenizeString(args, 0);
  
  if (Tokenizer_GetArgsCount() != 4) {
    ADDLOG_INFO(LOG_FEATURE_CMD, "Not Enough Arguments for WS2812B_GPIO_SetPixel: requires 4 arguments (pixel, r, g, b)");
    return CMD_RES_NOT_ENOUGH_ARGUMENTS;
  }
  
  pixel = Tokenizer_GetArgIntegerRange(0, 1, ws2812b_gpio_pixel_count);
  r = Tokenizer_GetArgIntegerRange(1, 0, 255);
  g = Tokenizer_GetArgIntegerRange(2, 0, 255);
  b = Tokenizer_GetArgIntegerRange(3, 0, 255);
  
  ADDLOG_INFO(LOG_FEATURE_CMD, "Set Pixel %i to R %i G %i B %i", pixel, r, g, b);
  g_pixels[pixel-1].red = r;
  g_pixels[pixel-1].green = g;
  g_pixels[pixel-1].blue = b;
  
  return CMD_RES_OK;
}

commandResult_t WS2812B_GPIO_Test_cmd(const void *context, const char *cmd, const char *args, int flags) {
	Tokenizer_TokenizeString(args, 0);

	if (Tokenizer_GetArgsCount() > 0) {
		ADDLOG_INFO(LOG_FEATURE_CMD, "Too Many Arguments for WS2812B_GPIO_Test: requires no arguments");
		return CMD_RES_ERROR;
	}

	for(int i = 0; i < ws2812b_gpio_pixel_count; i++){
	  int r = rand() % 256;
	  int g = rand() % 256;
	  int b = rand() % 256;
	  g_pixels[i].red = r;
	  g_pixels[i].green = g;
	  g_pixels[i].blue = b;
	}

	WS2812B_Send((byte *)g_pixels, ws2812b_gpio_pixel_count*3);
	
 	ADDLOG_INFO(LOG_FEATURE_CMD, "WS2812B_GPIO_Test done");
	return CMD_RES_OK;
}


/* void WS2812B_GPIO_RunQuickTick(void) { */
/*     ADDLOG_INFO(LOG_FEATURE_CMD, "WS2812B_GPIO_RunQuickTick (setting pixels to red for debugging)"); */
/*     // send reset by sending high for 3 bytes (24 bits) */
/*     for (int i = 0; i < 3; i++) { */
/*     } */

/* 	for (int i = 0; i < ws2812b_gpio_pixel_count; i++) { */
/* 	} */
/* } */

