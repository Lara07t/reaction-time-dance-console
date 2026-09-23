/*
 * game.h
 * Game state and flow: LEDs, cue selection, scoring, difficulty, the start screen loop and Freestyle mode.
 */

#ifndef GAME_H
#define GAME_H

#include <project.h>

/* Game state shared by the game logic, screens and main loop */
extern uint8 score;
extern uint8 lives;
extern uint8 seed;

extern uint8 highScore;
extern uint8 correctSinceLifeBonus;
extern uint8 bonusLifeEarned;

extern uint32 totalReactionTime;
extern uint16 correctResponses;
extern uint16 avgReactionTime;
extern uint16 bestReactionTime;

extern uint16 timeoutMs;
extern uint16 interCueDelayMs;
extern uint8 difficulty;

void AllLEDsOff(void);
void TurnOnCueLED(uint8 cue);
void WaitForRelease(void);
uint8 GetRandomCue(void);
void RecordCorrectResponse(uint16 reactionTime);
void StartScreen(void);
void DanceDelay(uint8 pose, uint8 *frame, uint16 durationMs);
void FreestyleMode(void);

#endif /* GAME_H */
