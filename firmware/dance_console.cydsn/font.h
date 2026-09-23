/*
 * font.h
 * 5x7 bitmap font and text drawing: plain, centered, two-color and numeric text.
 */

#ifndef FONT_H
#define FONT_H

#include <project.h>



void TFT_PrintString(uint16 x, uint16 y, const char *s, uint16 color, uint16 bg, uint8 size);
void TFT_PrintCentered(uint16 y, const char *s, uint16 color, uint16 bg, uint8 size);
void TFT_PrintAlternatingCentered(uint16 y, const char *s, uint16 color1, uint16 color2, uint16 bg, uint8 size);
void TFT_PrintNumber(uint16 x, uint16 y, uint32 num, uint16 color, uint16 bg, uint8 size);

#endif /* FONT_H */
