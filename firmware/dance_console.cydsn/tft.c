/*
 * tft.c
 * ILI9341 TFT driver: SPI transfers, address windows, rectangle fills and display initialization.
 */

#include <project.h>
#include "tft.h"

static void TFT_Write8(uint8 data)
{
    SPIM_TFT_WriteTxData(data);

    while((SPIM_TFT_ReadTxStatus() & SPIM_TFT_STS_SPI_DONE) == 0)
    {
    }

    if(SPIM_TFT_GetRxBufferSize() > 0)
    {
        (void)SPIM_TFT_ReadRxData();
    }
}

static void TFT_Command(uint8 cmd)
{
    tft_dc_Write(0);
    TFT_Write8(cmd);
}

static void TFT_Data(uint8 data)
{
    tft_dc_Write(1);
    TFT_Write8(data);
}

void TFT_Data16(uint16 data)
{
    TFT_Data((uint8)(data >> 8));
    TFT_Data((uint8)(data & 0xFF));
}


void TFT_SetAddrWindow(uint16 x0, uint16 y0, uint16 x1, uint16 y1)
{
    TFT_Command(0x2A);
    TFT_Data16(x0);
    TFT_Data16(x1);

    TFT_Command(0x2B);
    TFT_Data16(y0);
    TFT_Data16(y1);

    TFT_Command(0x2C);
}

void TFT_FillRect(uint16 x, uint16 y, uint16 w, uint16 h, uint16 color)
{
    uint32 i;
    uint32 total;

    if(x >= ILI9341_WIDTH || y >= ILI9341_HEIGHT)
    {
        return;
    }

    if((x + w - 1) >= ILI9341_WIDTH)
    {
        w = ILI9341_WIDTH - x;
    }

    if((y + h - 1) >= ILI9341_HEIGHT)
    {
        h = ILI9341_HEIGHT - y;
    }

    TFT_SetAddrWindow(x, y, x + w - 1, y + h - 1);

    total = (uint32)w * h;

    for(i = 0; i < total; i++)
    {
        TFT_Data16(color);
    }
}

static void TFT_FillScreen(uint16 color)
{
    TFT_FillRect(0, 0, ILI9341_WIDTH, ILI9341_HEIGHT, color);
}


void TFT_Init(void)
{
    CyDelay(100);

    TFT_Command(0x01);
    CyDelay(150);

    TFT_Command(0x28);

    TFT_Command(0xCF);
    TFT_Data(0x00);
    TFT_Data(0xC1);
    TFT_Data(0x30);

    TFT_Command(0xED);
    TFT_Data(0x64);
    TFT_Data(0x03);
    TFT_Data(0x12);
    TFT_Data(0x81);

    TFT_Command(0xE8);
    TFT_Data(0x85);
    TFT_Data(0x00);
    TFT_Data(0x78);

    TFT_Command(0xCB);
    TFT_Data(0x39);
    TFT_Data(0x2C);
    TFT_Data(0x00);
    TFT_Data(0x34);
    TFT_Data(0x02);

    TFT_Command(0xF7);
    TFT_Data(0x20);

    TFT_Command(0xEA);
    TFT_Data(0x00);
    TFT_Data(0x00);

    TFT_Command(0xC0);
    TFT_Data(0x23);

    TFT_Command(0xC1);
    TFT_Data(0x10);

    TFT_Command(0xC5);
    TFT_Data(0x3E);
    TFT_Data(0x28);

    TFT_Command(0xC7);
    TFT_Data(0x86);

    TFT_Command(0x36);
    TFT_Data(0x48);

    TFT_Command(0x3A);
    TFT_Data(0x55);

    TFT_Command(0xB1);
    TFT_Data(0x00);
    TFT_Data(0x18);

    TFT_Command(0xB6);
    TFT_Data(0x08);
    TFT_Data(0x82);
    TFT_Data(0x27);

    TFT_Command(0x11);
    CyDelay(120);

    TFT_Command(0x29);
    CyDelay(50);

    TFT_FillScreen(BLACK);
}
