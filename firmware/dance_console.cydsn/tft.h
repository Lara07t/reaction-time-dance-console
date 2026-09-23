/*
 * tft.h
 * ILI9341 TFT driver: SPI transfers, address windows, rectangle fills and display initialization.
 */

#ifndef TFT_H
#define TFT_H

#include <project.h>

/* Display size in pixels (portrait) */
#define ILI9341_WIDTH   240
#define ILI9341_HEIGHT  320

/* RGB565 colors */
#define BLACK   0x0000
#define WHITE   0xFFFF
#define RED     0xF800
#define GREEN   0x07E0
#define BLUE    0x001F
#define YELLOW  0xFFE0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define ORANGE  0xFD20

void TFT_Data16(uint16 data);
void TFT_SetAddrWindow(uint16 x0, uint16 y0, uint16 x1, uint16 y1);
void TFT_FillRect(uint16 x, uint16 y, uint16 w, uint16 h, uint16 color);
void TFT_Init(void);

#endif /* TFT_H */
