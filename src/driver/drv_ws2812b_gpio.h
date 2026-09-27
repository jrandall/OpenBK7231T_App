#pragma once
#include "../cmnds/cmd_public.h"

static void WS2812B_Send(byte *data, int dataSize);
void WS2812B_GPIO_Send(void);

void WS2812B_GPIO_Init(void);
void WS2812B_GPIO_RunQuickTick(void);
commandResult_t WS2812B_GPIO_Init_cmd(const void *context, const char *cmd, const char *args, int flags);
commandResult_t WS2812B_GPIO_SetPixel_cmd(const void *context, const char *cmd, const char *args, int flags);
commandResult_t WS2812B_GPIO_Test_cmd(const void *context, const char *cmd, const char *args, int flags);

