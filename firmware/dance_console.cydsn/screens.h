/*
 * screens.h
 * Screen layouts: start screen, in-game layout, score and cue updates, and the game-over screen.
 */

#ifndef SCREENS_H
#define SCREENS_H

#include <project.h>



void DrawTopBar(uint16 color);
void DrawStartScreenStatic(void);
void UpdateDifficultyText(void);
void DrawGameLayout(void);
void UpdateScoreLives(void);
void UpdateCueText(uint8 cue);
void ClearGameMessage(void);
void UpdateGameMessage(const char *msg, uint16 color);
void ShowGameOverScreen(void);

#endif /* SCREENS_H */
