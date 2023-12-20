#pragma once
#include "../cmnds/cmd_public.h"

void WS2812B_UART_Inv_Init(void);
void WS2812B_UART_Inv_RunQuickTick(void);
commandResult_t WS2812B_UART_Inv_Init_cmd(const void *context, const char *cmd, const char *args, int flags);
commandResult_t WS2812B_UART_Inv_SetPixel_cmd(const void *context, const char *cmd, const char *args, int flags);
commandResult_t WS2812B_UART_Inv_Test_cmd(const void *context, const char *cmd, const char *args, int flags);

