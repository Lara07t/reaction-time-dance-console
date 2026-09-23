/*
 * screens.c
 * Screen layouts: start screen, in-game layout, score and cue updates, and the game-over screen.
 */

#include <project.h>
#include "screens.h"
#include "tft.h"
#include "font.h"
#include "dancer.h"
#include "game.h"

void DrawTopBar(uint16 color)
{
    TFT_FillRect(0, 0, 240, 28, color);
}

void DrawStartScreenStatic(void)
{
    /*
     * FAST SCREEN DRAW:
     * Do not clear the whole 240x320 display here.
     * A full-screen SPI fill is the slow part, so only erase the
     * areas used by the start/game-over text.
     */
    DrawTopBar(BLACK);

    TFT_FillRect(0, 28, 240, 292, BLACK);

    /* Neon title box */
    TFT_FillRect(12, 42, 216, 86, MAGENTA);
    TFT_FillRect(16, 46, 208, 78, BLACK);
    TFT_FillRect(20, 50, 200, 70, MAGENTA);
    TFT_FillRect(24, 54, 192, 62, BLACK);

    /* Title */
    TFT_PrintAlternatingCentered(66, "REACTION TIME", MAGENTA, CYAN, BLACK, 2);
    TFT_PrintAlternatingCentered(96, "GAME", MAGENTA, CYAN, BLACK, 2);

    /* Decorative side bars */
    TFT_FillRect(24, 140, 42, 5, CYAN);
    TFT_FillRect(174, 140, 42, 5, CYAN);

    /* Difficulty label area */

    /* Start instruction box */
    TFT_FillRect(16, 244, 208, 50, MAGENTA);
    TFT_FillRect(20, 248, 200, 42, BLACK);

    TFT_PrintCentered(252, "PRESS ANY BUTTON", YELLOW, BLACK, 2);
    TFT_PrintCentered(272, "TO START", YELLOW, BLACK, 2);
    /* Clear the sprite area */
    TFT_FillRect(SPR_X, SPR_Y, SPR_W, SPR_H, BLACK);
}

void UpdateDifficultyText(void)
{
    TFT_FillRect(0, 155, 240, 25, BLACK);

    if(difficulty == 0)
    {
        TFT_PrintCentered(160, "EASY", GREEN, BLACK, 2);
    }
    else if(difficulty == 1)
    {
        TFT_PrintCentered(160, "MED", YELLOW, BLACK, 2);
    }
    else if(difficulty == 2)
    {
        TFT_PrintCentered(160, "HARD", RED, BLACK, 2);
    }
    else
    {
        TFT_PrintCentered(160, "FREESTYLE", CYAN, BLACK, 2);
    }
}

void DrawGameLayout(void)
{
    /* Clear the playable area: the start and game-over screens draw across all of it. */
    DrawTopBar(BLACK);

    TFT_FillRect(0, 28, 240, 292, BLACK);

    /* Freestyle mode keeps the TFT clean: only the dancer sprite is shown. */
    if(difficulty == 3)
    {
        TFT_FillRect(SPR_X, SPR_Y, SPR_W, SPR_H, BLACK);
        return;
    }

    /* Top stats area */
    TFT_PrintString(25, 8, "LIVES:", WHITE, BLACK, 2);
    TFT_PrintString(130, 8, "SCORE:", WHITE, BLACK, 2);

    /* Clear cue/message area */
    TFT_FillRect(0, 45, 240, 90, BLACK);
    TFT_FillRect(0, 115, 240, 28, BLACK);

    TFT_FillRect(SPR_X, SPR_Y, SPR_W, SPR_H, BLACK);
}
void UpdateScoreLives(void)
{
    if(difficulty == 3)
    {
        return;
    }

    /* Clear only the number areas, not the labels */
    TFT_FillRect(95, 8, 35, 16, BLACK);
    TFT_PrintNumber(95, 8, lives, YELLOW, BLACK, 2);

    TFT_FillRect(200, 8, 40, 16, BLACK);
    TFT_PrintNumber(200, 8, score, YELLOW, BLACK, 2);
}
void UpdateCueText(uint8 cue)
{
    if(difficulty == 3)
    {
        return;
    }

    TFT_FillRect(0, 45, 240, 65, BLACK);

    /*
     * Final tactile LED button wiring:
     * B1 = RED   switch sw1 P4[1], LED led1 P4[0]
     * B2 = WHITE switch sw2 P4[3], LED led2 P4[2]
     * B3 = BLUE  switch sw3 P4[5], LED led3 P4[4]
     * B4 = GREEN switch sw4 P4[7], LED led4 P4[6]
     */
    if(cue == 0)
    {
        TFT_PrintCentered(58, "R", RED, BLACK, 4);
    }
    else if(cue == 1)
    {
        TFT_PrintCentered(58, "W", WHITE, BLACK, 4);
    }
    else if(cue == 2)
    {
        TFT_PrintCentered(58, "B", BLUE, BLACK, 4);
    }
    else
    {
        TFT_PrintCentered(58, "G", GREEN, BLACK, 4);
    }
}

void ClearGameMessage(void)
{
    if(difficulty == 3)
    {
        return;
    }

    TFT_FillRect(0, 115, 240, 28, BLACK);
}
void UpdateGameMessage(const char *msg, uint16 color)
{
    if(difficulty == 3)
    {
        return;
    }

    ClearGameMessage();
    TFT_PrintCentered(120, msg, color, BLACK, 2);
}

void ShowGameOverScreen(void)
{
    /*
     * FAST SCREEN DRAW:
     * No full-screen clear. This makes the GAME OVER screen appear faster
     * and makes the next press feel more responsive.
     */
    DrawTopBar(BLACK);

    TFT_FillRect(0, 28, 240, 292, BLACK);
    TFT_FillRect(SPR_X, SPR_Y, SPR_W, SPR_H, BLACK);

    /* Red title box */
    TFT_FillRect(12, 42, 216, 72, RED);
    TFT_FillRect(16, 46, 208, 64, BLACK);
    TFT_FillRect(20, 50, 200, 56, RED);
    TFT_FillRect(24, 54, 192, 48, BLACK);

    /* Big centered GAME OVER */
    TFT_PrintCentered(66, "GAME OVER", RED, BLACK, 3);

    /* Score and reaction stats */
    TFT_PrintString(35, 130, "SCORE:", WHITE, BLACK, 2);
    TFT_PrintNumber(115, 130, score, YELLOW, BLACK, 2);

    TFT_PrintString(35, 162, "AVG:", WHITE, BLACK, 2);
    TFT_PrintNumber(115, 162, avgReactionTime, YELLOW, BLACK, 2);
    TFT_PrintString(170, 162, "MS", YELLOW, BLACK, 2);

    TFT_PrintString(35, 194, "BEST:", WHITE, BLACK, 2);
    TFT_PrintNumber(115, 194, bestReactionTime, YELLOW, BLACK, 2);
    TFT_PrintString(170, 194, "MS", YELLOW, BLACK, 2);

    TFT_PrintString(35, 226, "HIGH SCORE:", WHITE, BLACK, 2);
    TFT_PrintNumber(175, 226, highScore, YELLOW, BLACK, 2);

    /* Restart instruction box */
    TFT_FillRect(16, 246, 220, 48, RED);
    TFT_FillRect(20, 250, 212, 40, BLACK);

    TFT_PrintCentered(254, "PRESS ANY BUTTON", YELLOW, BLACK, 2);
    TFT_PrintCentered(274, "TO RESTART", YELLOW, BLACK, 2);
}
