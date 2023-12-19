#include "drv_ws2812b_uart_inv.h"

#include <math.h>
#include <stdint.h>

#include "../logging/logging.h"
#include "../new_pins.h"
#include "drv_uart.h"

#define WS2812B_UART_INV_BAUD_RATE 2500000

typedef struct {
  UINT8 red;
  UINT8 green;
  UINT8 blue;
} ws2812b_uart_inv_pixel_t;

ws2812b_uart_inv_pixel_t *pixels;
BOOLEAN ws2812b_uart_inv_initialized = false;
uint32_t ws2812b_uart_inv_pixel_count = 0;

void WS2812B_UART_Inv_Init(void) {
	UART_InitUART(WS2812B_UART_INV_BAUD_RATE, 0);

	//cmddetail:{"name":"WS2812B_UART_Inv_Init","args":"WS2812B_UART_Inv_Init_cmd",
	//cmddetail:"descr":"Initialize WS2812B_UART_Inv driver for an <N> light string",
	//cmddetail:"fn":"NULL);","file":"driver/drv_ws2812b_uart_inv.c","requires":"",
	//cmddetail:"examples":"WS2812B_UART_Inv_Init <N>"}
	CMD_RegisterCommand("WS2812B_UART_Inv_Init", WS2812B_UART_Inv_Init_cmd, NULL);

	//cmddetail:{"name":"WS2812B_UART_Inv_SetPixel","args":"WS2812B_UART_Inv_SetPixel_cmd",
	//cmddetail:"descr":"",
	//cmddetail:"fn":"NULL);","file":"driver/drv_ws2812b_uart_inv.c","requires":"",
	//cmddetail:"examples":""}
	CMD_RegisterCommand("WS2812B_UART_Inv_SetPixel", WS2812B_UART_Inv_SetPixel_cmd, NULL);
}

commandResult_t WS2812B_UART_Inv_Init_cmd(const void *context, const char *cmd, const char *args, int flags) {

	Tokenizer_TokenizeString(args, 0);

	if (Tokenizer_GetArgsCount() == 0) {
		ADDLOG_INFO(LOG_FEATURE_CMD, "Not Enough Arguments for init WS2812B_UART_Inv_Init: amount of LEDs missing");
		return CMD_RES_NOT_ENOUGH_ARGUMENTS;
	}

	ws2812b_uart_inv_pixel_count = Tokenizer_GetArgIntegerRange(0, 0, 255);

	ADDLOG_INFO(LOG_FEATURE_CMD, "Register driver with %i LEDs", ws2812b_uart_inv_pixel_count);

	// Prepare buffer
	pixels = (ws2812b_uart_inv_pixel_t *)os_malloc(ws2812b_uart_inv_pixel_count * sizeof(ws2812b_uart_inv_pixel_t));

	// Initialize all pixels
	for (int i = 0; i < ws2812b_uart_inv_pixel_count; i++) {
	  pixels[i].red = 0;
	  pixels[i].green = 0;
	  pixels[i].blue = 0;
	}

	ws2812b_uart_inv_initialized = true;

	return CMD_RES_OK;
}

commandResult_t WS2812B_UART_Inv_SetPixel_cmd(const void *context, const char *cmd, const char *args, int flags) {
	int pixel, i, r, g, b;
	Tokenizer_TokenizeString(args, 0);

	if (Tokenizer_GetArgsCount() != 4) {
		ADDLOG_INFO(LOG_FEATURE_CMD, "Not Enough Arguments for WS2812B_UART_Inv_SetPixel: requires 4 arguments (pixel, r, g, b)");
		return CMD_RES_NOT_ENOUGH_ARGUMENTS;
	}

	pixel = Tokenizer_GetArgIntegerRange(0, 1, ws2812b_uart_inv_pixel_count);
	r = Tokenizer_GetArgIntegerRange(1, 0, 255);
	g = Tokenizer_GetArgIntegerRange(2, 0, 255);
	b = Tokenizer_GetArgIntegerRange(3, 0, 255);

	ADDLOG_INFO(LOG_FEATURE_CMD, "Set Pixel %i to R %i G %i B %i", pixel, r, g, b);
	pixels[pixel-1].red = r;
	pixels[pixel-1].green = g;
	pixels[pixel-1].blue = b;

	return CMD_RES_OK;
}

void WS2812B_UART_Inv_RunQuickTick(void) {
    UART_InitUART(WS2812B_UART_INV_BAUD_RATE, 0);

    //    UART_SendByte(WS2812B_UART_INV_CMD_READ(WS2812B_UART_INV_ADDR));
    //    UART_SendByte(WS2812B_UART_INV_REG_PACKET);
    UART_SendByte(0xdb);
    UART_SendByte(0xdb);
    UART_SendByte(0x9b);
    UART_SendByte(0x92);
    UART_SendByte(0x92);
    UART_SendByte(0xda);
    UART_SendByte(0xdb);
    UART_SendByte(0xdb);
}

