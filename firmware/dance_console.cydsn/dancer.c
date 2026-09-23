/*
 * dancer.c
 * Pixel dancer sprite: a 64x96 off-screen buffer, line and rectangle drawing, and the pose animations.
 */

#include <project.h>
#include "dancer.h"
#include "tft.h"

/* Off-screen buffer the dancer is drawn into before being sent to the display */
static uint16 sprite[SPR_W * SPR_H];

static void TFT_PushSprite(void)
{
    uint32 i;
    uint32 total;

    TFT_SetAddrWindow(SPR_X, SPR_Y, SPR_X + SPR_W - 1, SPR_Y + SPR_H - 1);

    total = (uint32)SPR_W * SPR_H;

    for(i = 0; i < total; i++)
    {
        TFT_Data16(sprite[i]);
    }
}


static void Sprite_Clear(uint16 color)
{
    uint32 i;

    for(i = 0; i < ((uint32)SPR_W * SPR_H); i++)
    {
        sprite[i] = color;
    }
}

static void Sprite_Pixel(int x, int y, uint16 color)
{
    if(x < 0 || x >= SPR_W || y < 0 || y >= SPR_H)
    {
        return;
    }

    sprite[(y * SPR_W) + x] = color;
}

static void Sprite_FillRect(int x, int y, int w, int h, uint16 color)
{
    int i;
    int j;

    for(j = y; j < y + h; j++)
    {
        for(i = x; i < x + w; i++)
        {
            Sprite_Pixel(i, j, color);
        }
    }
}

static int AbsInt(int v)
{
    if(v < 0)
    {
        return -v;
    }

    return v;
}

static void Sprite_Line(int x0, int y0, int x1, int y1, uint16 color)
{
    int dx;
    int dy;
    int sx;
    int sy;
    int err;
    int e2;

    dx = AbsInt(x1 - x0);
    dy = AbsInt(y1 - y0);

    sx = (x0 < x1) ? 1 : -1;
    sy = (y0 < y1) ? 1 : -1;

    err = dx - dy;

    while(1)
    {
        Sprite_FillRect(x0 - 1, y0 - 1, 3, 3, color);

        if(x0 == x1 && y0 == y1)
        {
            break;
        }

        e2 = 2 * err;

        if(e2 > -dy)
        {
            err -= dy;
            x0 += sx;
        }

        if(e2 < dx)
        {
            err += dx;
            y0 += sy;
        }
    }
}

void DrawDancerFrame(uint8 pose, uint8 legFrame)
{
    int x = 32;
    int y = 10;
    int headOffset;
    int headBottom;
    int neckHeight;

    Sprite_Clear(BLACK);

    /* Bob the head up and down only while dancing */
    if(pose == 0)
    {
        headOffset = 0;
    }
    else if(legFrame == 0)
    {
        headOffset = -1;
    }
    else
    {
        headOffset = 2;
    }

    /* Head */
    Sprite_FillRect(x - 9, y + 2 + headOffset, 18, 16, ORANGE);

    /* Retro hair */
    Sprite_FillRect(x - 9, y + headOffset, 18, 4, MAGENTA);

    /* Sunglasses */
    Sprite_FillRect(x - 7, y + 7 + headOffset, 5, 3, BLACK);
    Sprite_FillRect(x - 1, y + 7 + headOffset, 2, 2, BLACK);
    Sprite_FillRect(x + 2, y + 7 + headOffset, 5, 3, BLACK);

    /* Smile */
    Sprite_FillRect(x - 3, y + 13 + headOffset, 7, 2, BLACK);

    /* Headphones */
    Sprite_Line(x - 8, y + 5 + headOffset, x - 3, y + 1 + headOffset, WHITE);
    Sprite_Line(x - 3, y + 1 + headOffset, x + 3, y + 1 + headOffset, WHITE);
    Sprite_Line(x + 3, y + 1 + headOffset, x + 8, y + 5 + headOffset, WHITE);
    Sprite_FillRect(x - 12, y + 6 + headOffset, 4, 8, CYAN);
    Sprite_FillRect(x + 8, y + 6 + headOffset, 4, 8, MAGENTA);

    /* Neck adjusts with head movement */
    headBottom = y + 18 + headOffset;
    neckHeight = ((y + 22) - headBottom) - 1;
    if(neckHeight < 1)
    {
        neckHeight = 1;
    }
    Sprite_FillRect(x - 2, headBottom, 4, neckHeight, WHITE);

    /* Retro jacket */
    Sprite_FillRect(x - 10, y + 22, 20, 24, CYAN);
    Sprite_FillRect(x - 4, y + 22, 8, 24, MAGENTA);
    Sprite_FillRect(x - 10, y + 31, 20, 2, YELLOW);
    Sprite_FillRect(x - 3, y + 24, 6, 6, WHITE);

    /* Arms:
     * pose 1 = button 1, right arm up
     * pose 2 = button 2, left arm up
     * pose 3 = button 3, right arm straight out
     * pose 4 = button 4, left arm straight out
     */
    if(pose == 1)
    {
        /* Button 1 pose: right arm up, pulsing up/down */
        if(legFrame == 0)
        {
            Sprite_Line(x + 9, y + 28, x + 22, y + 12, MAGENTA);
            Sprite_FillRect(x + 21, y + 9, 4, 4, YELLOW);
        }
        else
        {
            Sprite_Line(x + 9, y + 28, x + 25, y + 7, MAGENTA);
            Sprite_FillRect(x + 24, y + 4, 4, 4, YELLOW);
        }

        Sprite_Line(x - 9, y + 28, x - 24, y + 38, CYAN);
    }
    else if(pose == 2)
    {
        /* Button 2 pose: left arm up, pulsing up/down */
        if(legFrame == 0)
        {
            Sprite_Line(x - 9, y + 28, x - 22, y + 12, CYAN);
            Sprite_FillRect(x - 25, y + 9, 4, 4, YELLOW);
        }
        else
        {
            Sprite_Line(x - 9, y + 28, x - 25, y + 7, CYAN);
            Sprite_FillRect(x - 28, y + 4, 4, 4, YELLOW);
        }

        Sprite_Line(x + 9, y + 28, x + 24, y + 38, MAGENTA);
    }
    else if(pose == 3)
    {
        /* Button 3 pose: right arm straight out, pulsing in/out */
        if(legFrame == 0)
        {
            Sprite_Line(x + 9, y + 28, x + 25, y + 28, MAGENTA);
            Sprite_FillRect(x + 25, y + 26, 4, 4, YELLOW);
        }
        else
        {
            Sprite_Line(x + 9, y + 28, x + 31, y + 28, MAGENTA);
            Sprite_FillRect(x + 31, y + 26, 4, 4, YELLOW);
        }

        Sprite_Line(x - 9, y + 28, x - 24, y + 38, CYAN);
    }
    else if(pose == 4)
    {
        /* Button 4 pose: left arm straight out, pulsing in/out */
        if(legFrame == 0)
        {
            Sprite_Line(x - 9, y + 28, x - 25, y + 28, CYAN);
            Sprite_FillRect(x - 29, y + 26, 4, 4, YELLOW);
        }
        else
        {
            Sprite_Line(x - 9, y + 28, x - 31, y + 28, CYAN);
            Sprite_FillRect(x - 35, y + 26, 4, 4, YELLOW);
        }

        Sprite_Line(x + 9, y + 28, x + 24, y + 38, MAGENTA);
    }
    else
    {
        if(legFrame == 0)
        {
            Sprite_Line(x - 9, y + 28, x - 24, y + 34, CYAN);
            Sprite_Line(x + 9, y + 28, x + 24, y + 34, MAGENTA);
        }
        else
        {
            Sprite_Line(x - 9, y + 28, x - 24, y + 20, CYAN);
            Sprite_Line(x + 9, y + 28, x + 24, y + 20, MAGENTA);
        }
    }

    /* Waist */
    Sprite_FillRect(x - 8, y + 46, 16, 5, BLUE);

    /* Legs */
    if(legFrame == 0)
    {
        Sprite_Line(x - 4, y + 50, x - 16, y + 80, WHITE);
        Sprite_Line(x + 4, y + 50, x + 14, y + 80, WHITE);
        Sprite_FillRect(x - 18, y + 80, 8, 3, YELLOW);
        Sprite_FillRect(x + 12, y + 80, 8, 3, YELLOW);
    }
    else
    {
        Sprite_Line(x - 4, y + 50, x - 8, y + 80, WHITE);
        Sprite_Line(x + 4, y + 50, x + 22, y + 80, WHITE);
        Sprite_FillRect(x - 10, y + 80, 8, 3, YELLOW);
        Sprite_FillRect(x + 20, y + 80, 8, 3, YELLOW);
    }

    TFT_PushSprite();
}
