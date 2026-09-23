/*
 * game.c
 * Game state and flow: LEDs, cue selection, scoring, difficulty, the start screen loop and Freestyle mode.
 */

#include <project.h>
#include "game.h"
#include "tft.h"
#include "screens.h"
#include "dancer.h"
#include "audio.h"

uint8 score = 0;
uint8 lives = 3;
uint8 seed = 7;

uint8 highScore = 0;
uint8 correctSinceLifeBonus = 0;
uint8 bonusLifeEarned = 0;

uint32 totalReactionTime = 0;
uint16 correctResponses = 0;
uint16 avgReactionTime = 0;
uint16 bestReactionTime = 9999;

uint16 timeoutMs = 1000;
uint16 interCueDelayMs = 800;
uint8 difficulty = 0;


void AllLEDsOff(void)
{
    led1_Write(0);   /* Red LED:   P4[0] */
    led2_Write(0);   /* White LED: P4[2] */
    led3_Write(0);   /* Blue LED:  P4[4] */
    led4_Write(0);   /* Green LED: P4[6] */
}

void TurnOnCueLED(uint8 cue)
{
    AllLEDsOff();

    if(cue == 0)
    {
        led1_Write(1);   /* B1 = RED,   LED pin P4[0] */
    }
    else if(cue == 1)
    {
        led2_Write(1);   /* B2 = WHITE, LED pin P4[2] */
    }
    else if(cue == 2)
    {
        led3_Write(1);   /* B3 = BLUE,  LED pin P4[4] */
    }
    else
    {
        led4_Write(1);   /* B4 = GREEN, LED pin P4[6] */
    }
}

void WaitForRelease(void)
{
    while(sw1_Read() == 0 || sw2_Read() == 0 || sw3_Read() == 0 || sw4_Read() == 0)
    {
        AudioDelay(1);
    }
}

uint8 GetRandomCue(void)
{
    seed = seed * 17 + 23;
    return seed % 4;
}

static void ResetGame(void)
{
    score = 0;
    lives = 3;
    totalReactionTime = 0;
    correctResponses = 0;
    avgReactionTime = 0;
    bestReactionTime = 9999;
    correctSinceLifeBonus = 0;
    bonusLifeEarned = 0;
}

void RecordCorrectResponse(uint16 reactionTime)
{
    totalReactionTime = totalReactionTime + reactionTime;
    correctResponses++;

    if(reactionTime < bestReactionTime)
    {
        bestReactionTime = reactionTime;
    }

    score++;

    if(score > highScore)
    {
        highScore = score;
    }

    correctSinceLifeBonus++;
    bonusLifeEarned = 0;

    if(correctSinceLifeBonus >= 10)
    {
        lives++;
        correctSinceLifeBonus = 0;
        bonusLifeEarned = 1;
    }
}

static void UpdateDifficultyFromADC(void)
{
    int16 adcResult;
    uint8 newDifficulty;

    if(ADC_DelSig_1_IsEndConversion(ADC_DelSig_1_WAIT_FOR_RESULT))
    {
        adcResult = ADC_DelSig_1_GetResult16();

        if(adcResult < 0)
        {
            adcResult = 0;
        }
        else if(adcResult > 4095)
        {
            adcResult = 4095;
        }

        if(adcResult < 1024)
        {
            newDifficulty = 0;

            /* EASY: longest reaction window and more time between cues */
            timeoutMs = 1800;
            interCueDelayMs = 1800;
        }
        else if(adcResult < 2048)
        {
            newDifficulty = 1;

            /* MEDIUM */
            timeoutMs = 900;
            interCueDelayMs = 900;
        }
        else if(adcResult < 3072)
        {
            newDifficulty = 2;

            /* HARD */
            timeoutMs = 300;
            interCueDelayMs = 60;
        }
        else
        {
            newDifficulty = 3;

            /* FREESTYLE:
             * Free-dance mode. Buttons directly control the dancer pose
             * and the matching LED while held.
             */
            timeoutMs = 0;
            interCueDelayMs = 0;
        }

        difficulty = newDifficulty;
    }
}

void StartScreen(void)
{
    uint8 lastDifficulty;

    AllLEDsOff();

    /*
     * Silence speaker while drawing the start screen to avoid transition buzz.
     */
    audioMode = 0;
    PWM_Audio_WriteCompare(0);

    UpdateDifficultyFromADC();
    DrawStartScreenStatic();
    UpdateDifficultyText();

    /*
     * Start background music only after the screen is drawn.
     */
    audioMode = 1;
    AudioRestartBackground();

    lastDifficulty = difficulty;

    /* If we arrived here from GAME OVER while the switch is still held,
     * show the START screen immediately, then wait for release so the
     * same press does not instantly start a new game.
     */
    if(sw1_Read() == 0 || sw2_Read() == 0 || sw3_Read() == 0 || sw4_Read() == 0)
    {
        WaitForRelease();
    }

    while(sw1_Read() == 1 && sw2_Read() == 1 && sw3_Read() == 1 && sw4_Read() == 1)
    {
        seed++;
        UpdateDifficultyFromADC();

        if(difficulty != lastDifficulty)
        {
            UpdateDifficultyText();
            lastDifficulty = difficulty;
        }

        AudioDelay(1);
    }

    /* Do not wait for release here. Start drawing the game screen right away.
     * The main game loop will wait for release before showing the first cue.
     */
    ResetGame();
}

void DanceDelay(uint8 pose, uint8 *frame, uint16 durationMs)
{
    uint16 elapsed = 0;
    uint16 stepMs;
    uint16 waitMs;

    if(pose == 0)
    {
        DrawDancerFrame(0, 0);
        AudioDelay(durationMs);
        return;
    }

    if(difficulty == 0)
    {
        stepMs = 160;
    }
    else if(difficulty == 1)
    {
        stepMs = 120;
    }
    else
    {
        stepMs = 80;
    }

    while(elapsed < durationMs)
    {
        DrawDancerFrame(pose, *frame);
        *frame = !(*frame);

        if((durationMs - elapsed) < stepMs)
        {
            waitMs = durationMs - elapsed;
        }
        else
        {
            waitMs = stepMs;
        }

        AudioDelay(waitMs);
        elapsed = elapsed + waitMs;
    }
}

static void FreestyleHoldSound(uint8 buttonNumber)
{
    uint8 period;
    uint8 compareValue;

    /*
     * Freestyle sound is held manually while the button is pressed.
     * This flag prevents AudioTick1ms() from immediately turning the
     * speaker back off.
     */
    freestyleAudioActive = 1;

    if(buttonNumber == 1)
    {
        period = NOTE_C5;
    }
    else if(buttonNumber == 2)
    {
        period = NOTE_G4;
    }
    else if(buttonNumber == 3)
    {
        period = NOTE_DS4;
    }
    else
    {
        period = NOTE_C4;
    }

    PWM_Audio_WritePeriod(period);

    compareValue = period / BUTTON_VOLUME_DIVIDER;

    if(compareValue < 1)
    {
        compareValue = 1;
    }

    PWM_Audio_WriteCompare(compareValue);
}

static void FreestyleStopSound(void)
{
    freestyleAudioActive = 0;
    PWM_Audio_WriteCompare(0);
}




void FreestyleMode(void)
{
    uint8 frame = 0;
    uint8 currentPose = 0;

    /*
     * Freestyle mode:
     * No score, no lives, no cue text.
     * The screen is only the dancer.
     * Holding a button keeps its LED, sound, and matching pose active.
     * Releasing all buttons turns LEDs off, stops sound, and returns dancer to idle.
     */
    audioMode = 0;
    PWM_Audio_WriteCompare(0);

    DrawTopBar(BLACK);
    TFT_FillRect(0, 28, 240, 292, BLACK);
    AllLEDsOff();

    DrawDancerFrame(0, 0);

    while(difficulty == 3)
    {
        UpdateDifficultyFromADC();

        if(sw1_Read() == 0)
        {
            currentPose = 1;

            led1_Write(1);
            led2_Write(0);
            led3_Write(0);
            led4_Write(0);

            FreestyleHoldSound(1);
        }
        else if(sw2_Read() == 0)
        {
            currentPose = 2;

            led1_Write(0);
            led2_Write(1);
            led3_Write(0);
            led4_Write(0);

            FreestyleHoldSound(2);
        }
        else if(sw3_Read() == 0)
        {
            currentPose = 3;

            led1_Write(0);
            led2_Write(0);
            led3_Write(1);
            led4_Write(0);

            FreestyleHoldSound(3);
        }
        else if(sw4_Read() == 0)
        {
            currentPose = 4;

            led1_Write(0);
            led2_Write(0);
            led3_Write(0);
            led4_Write(1);

            FreestyleHoldSound(4);
        }
        else
        {
            currentPose = 0;
            AllLEDsOff();
            FreestyleStopSound();
        }

        DrawDancerFrame(currentPose, frame);
        frame = !frame;

        AudioDelay(120);
    }

    AllLEDsOff();
    FreestyleStopSound();
    WaitForRelease();
}
