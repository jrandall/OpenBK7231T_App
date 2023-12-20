#include "drv_ws2812b_uart_inv.h"

#include <math.h>
#include <stdint.h>

#include "../logging/logging.h"
#include "../new_pins.h"
// #include "drv_uart.h"

#if PLATFORM_BK7231T | PLATFORM_BK7231N
#include "../../beken378/func/user_driver/BkDriverUart.h"
#endif

#define WS2812B_UART_INV_BAUD_RATE 2500000

typedef struct {
  UINT8 red;
  UINT8 green;
  UINT8 blue;
} ws2812b_uart_inv_pixel_t;

ws2812b_uart_inv_pixel_t *pixels;
BOOLEAN ws2812b_uart_inv_initialized = false;
uint32_t ws2812b_uart_inv_pixel_count = 0;
bk_uart_t ws2812b_uart = BK_UART_2;

void WS2812B_UART_Inv_Init(void) {
  // UART_InitUART(WS2812B_UART_INV_BAUD_RATE, 0);

	//cmddetail:{"name":"WS2812B_UART_Inv_Init","args":"WS2812B_UART_Inv_Init_cmd",
	//cmddetail:"descr":"Initialize WS2812B_UART_Inv driver for an <N> light string at <baud> baud",
	//cmddetail:"fn":"NULL);","file":"driver/drv_ws2812b_uart_inv.c","requires":"",
	//cmddetail:"examples":"WS2812B_UART_Inv_Init <N> <baud>"}
	CMD_RegisterCommand("WS2812B_UART_Inv_Init", WS2812B_UART_Inv_Init_cmd, NULL);

	//cmddetail:{"name":"WS2812B_UART_Inv_SetPixel","args":"WS2812B_UART_Inv_SetPixel_cmd",
	//cmddetail:"descr":"",
	//cmddetail:"fn":"NULL);","file":"driver/drv_ws2812b_uart_inv.c","requires":"",
	//cmddetail:"examples":""}
	CMD_RegisterCommand("WS2812B_UART_Inv_SetPixel", WS2812B_UART_Inv_SetPixel_cmd, NULL);
	
	//cmddetail:{"name":"WS2812B_UART_Inv_Test","args":"WS2812B_UART_Inv_Test_cmd",
	//cmddetail:"descr":"",
	//cmddetail:"fn":"NULL);","file":"driver/drv_ws2812b_uart_inv.c","requires":"",
	//cmddetail:"examples":""}
	CMD_RegisterCommand("WS2812B_UART_Inv_Test", WS2812B_UART_Inv_Test_cmd, NULL);
	
	ADDLOG_INFO(LOG_FEATURE_CMD, "WS2812B_UART_Inv_Init commands registered");
}

commandResult_t WS2812B_UART_Inv_Init_cmd(const void *context, const char *cmd, const char *args, int flags) {

	Tokenizer_TokenizeString(args, 0);

	if (Tokenizer_GetArgsCount() != 2) {
		ADDLOG_INFO(LOG_FEATURE_CMD, "Not Enough Arguments for init WS2812B_UART_Inv_Init: amount of LEDs or baud missing");
		return CMD_RES_NOT_ENOUGH_ARGUMENTS;
	}

	ws2812b_uart_inv_pixel_count = Tokenizer_GetArgIntegerRange(0, 0, 255);
	ADDLOG_INFO(LOG_FEATURE_CMD, "WS2812B_UART_Inv_Init_cmd register driver with %i LEDs", ws2812b_uart_inv_pixel_count);

	int baud = Tokenizer_GetArgIntegerRange(1, 100, 10000000);
	ADDLOG_INFO(LOG_FEATURE_CMD, "WS2812B_UART_Inv_Init_cmd register driver at %i baud", baud);
	// UART_InitUART(baud, 0);
	

	// Prepare buffer
	pixels = (ws2812b_uart_inv_pixel_t *)os_malloc(ws2812b_uart_inv_pixel_count * sizeof(ws2812b_uart_inv_pixel_t));

	// Initialize all pixels
	for (int i = 0; i < ws2812b_uart_inv_pixel_count; i++) {
	  pixels[i].red = 0;
	  pixels[i].green = 0;
	  pixels[i].blue = 0;
	}

	ws2812b_uart_inv_initialized = true;

	ADDLOG_INFO(LOG_FEATURE_CMD, "WS2812B_UART_Inv_Init_cmd driver initialized to use UART %i", ws2812b_uart);
	return CMD_RES_OK;
}

commandResult_t WS2812B_UART_Inv_SetPixel_cmd(const void *context, const char *cmd, const char *args, int flags) {
  int pixel;
  int r;
  int g;
  int b;
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

commandResult_t WS2812B_UART_Inv_Test_cmd(const void *context, const char *cmd, const char *args, int flags) {
	Tokenizer_TokenizeString(args, 0);

	if (Tokenizer_GetArgsCount() > 0) {
		ADDLOG_INFO(LOG_FEATURE_CMD, "Too Many Arguments for WS2812B_UART_Inv_Test: requires no arguments");
		return CMD_RES_ERROR;
	}

    /* // send reset by sending high for 3 bytes (24 bits) */
    /* for (int i = 0; i < 3; i++) { */
    /*   ADDLOG_INFO(LOG_FEATURE_CMD, "WS2812B_UART_Inv_Test sending reset byte %i", i); */
    /* 	  UART_SendByte(0xff); */
    /* } */

    /* //UART_InitUART(WS2812B_UART_INV_BAUD_RATE, 0); */

    /* //    UART_SendByte(WS2812B_UART_INV_CMD_READ(WS2812B_UART_INV_ADDR)); */
    /* //    UART_SendByte(WS2812B_UART_INV_REG_PACKET); */
    /* 	for (int i = 0; i < ws2812b_uart_inv_pixel_count; i++) { */
    /*   ADDLOG_INFO(LOG_FEATURE_CMD, "WS2812B_UART_Inv_Test sending red pixel bytes %i", i); */
    /* UART_SendByte(0xdb); */
    /* UART_SendByte(0xdb); */
    /* UART_SendByte(0x9b); */
    /* UART_SendByte(0x92); */
    /* UART_SendByte(0x92); */
    /* UART_SendByte(0xda); */
    /* UART_SendByte(0xdb); */
    /* UART_SendByte(0xdb); */
    /* 	} */
	char *data;
	int size = 3 + ws2812b_uart_inv_pixel_count * 8;
	data = (char *)os_malloc(size);
	data[0] = 0xff;
	data[1] = 0xff;
	data[2] = 0xff;
	for (int i = 0; i < ws2812b_uart_inv_pixel_count; i++) {
	  int offset = 3 + i*8;
	  data[offset+0] = 0xdb;
	  data[offset+1] = 0xdb;
	  data[offset+2] = 0x9b;
	  data[offset+3] = 0x92;
	  data[offset+4] = 0x92;
	  data[offset+5] = 0xda;
	  data[offset+6] = 0xdb;
	  data[offset+7] = 0xdb;
	}

	bk_uart_config_t config;
	config.baud_rate = baud;
	config.data_width = 0x07;
        config.parity = 0;    //0:no parity,1:odd,2:even
        config.stop_bits = 0;   //0:1bit,1:2bit
        config.flow_control = 0;   //FLOW_CTRL_DISABLED
	config.flags = 0;

        ADDLOG_INFO(LOG_FEATURE_CMD, "WS2812B_UART_Inv_Test finalizing UART %i", ws2812b_uart);
	int status = bk_uart_finalize(ws2812b_uart);
        ADDLOG_INFO(LOG_FEATURE_CMD, "WS2812B_UART_Inv_Test finalize status %i", status);
	
        ADDLOG_INFO(LOG_FEATURE_CMD, "WS2812B_UART_Inv_Test initializing UART %i at baud %i", ws2812b_uart, baud);
        status = bk_uart_initialize(ws2812b_uart, &config, NULL);
	ADDLOG_INFO(LOG_FEATURE_CMD, "WS2812B_UART_Inv_Test initialize status %i", status);
        
	ADDLOG_INFO(LOG_FEATURE_CMD, "WS2812B_UART_Inv_Test clearing RX callback");
	bk_uart_set_rx_callback(ws2812b_uart, NULL, NULL);
	ADDLOG_INFO(LOG_FEATURE_CMD, "WS2812B_UART_Inv_Test clear RX callbackl status %i", status);
	
	ADDLOG_INFO(LOG_FEATURE_CMD, "WS2812B_UART_Inv_Test sending red pixel data of size %i", size);
	status = bk_uart_send(ws2812b_uart, data, size);
	ADDLOG_INFO(LOG_FEATURE_CMD, "WS2812B_UART_Inv_Test send status %i", status);

	ADDLOG_INFO(LOG_FEATURE_CMD, "WS2812B_UART_Inv_Test done");
	return CMD_RES_OK;
}


/* void WS2812B_UART_Inv_RunQuickTick(void) { */
/*     ADDLOG_INFO(LOG_FEATURE_CMD, "WS2812B_UART_Inv_RunQuickTick (setting pixels to red for debugging)"); */
/*     // send reset by sending high for 3 bytes (24 bits) */
/*     for (int i = 0; i < 3; i++) { */
/* 	  UART_SendByte(0xff); */
/*     } */

/*     //UART_InitUART(WS2812B_UART_INV_BAUD_RATE, 0); */

/*     //    UART_SendByte(WS2812B_UART_INV_CMD_READ(WS2812B_UART_INV_ADDR)); */
/*     //    UART_SendByte(WS2812B_UART_INV_REG_PACKET); */
/* 	for (int i = 0; i < ws2812b_uart_inv_pixel_count; i++) { */
/*     UART_SendByte(0xdb); */
/*     UART_SendByte(0xdb); */
/*     UART_SendByte(0x9b); */
/*     UART_SendByte(0x92); */
/*     UART_SendByte(0x92); */
/*     UART_SendByte(0xda); */
/*     UART_SendByte(0xdb); */
/*     UART_SendByte(0xdb); */
/* 	} */
/* } */

