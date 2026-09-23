/*
 * main.c
 * Reaction-Time Dance Console: entry point and main game loop.
 */

#include <project.h>
#include "tft.h"
#include "screens.h"
#include "dancer.h"
#include "audio.h"
#include "game.h"

int main(void)
{
    uint8 cue;
    uint16 timeCount;
    uint8 answered;
    uint8 resultType;
    uint16 reactionTime;
    uint16 maxCounts;
    uint8 frame = 0;
    uint8 dancePose = 0;
    uint8 pressedPose = 0;

    CyGlobalIntEnable;

    SPIM_TFT_Start();

    ADC_DelSig_1_Start();
    ADC_DelSig_1_StartConvert();

    AudioInit();

    AllLEDsOff();

    TFT_Init();

    for(;;)
    {
        StartScreen();

        if(difficulty == 3)
        {
            FreestyleMode();
            continue;
        }

        /*
         * Silence speaker while switching from start screen to game screen.
         */
        audioMode = 0;
        PWM_Audio_WriteCompare(0);

        DrawGameLayout();
        UpdateScoreLives();
        dancePose = 0;
        frame = 0;
        DrawDancerFrame(0, 0);

        /*
         * Resume music after the game screen is drawn.
         */
        audioMode = 1;
        AudioRestartBackground();

        while(lives > 0)
        {
            WaitForRelease();

            cue = GetRandomCue();
            answered = 0;
            resultType = 0;
            pressedPose = 0;
            bonusLifeEarned = 0;
            timeCount = 0;
            maxCounts = timeoutMs / 2;

            AllLEDsOff();

            UpdateCueText(cue);
            ClearGameMessage();

            DrawDancerFrame(dancePose, frame);

            /* Lower the music during the cue so it doesn't compete with the player's focus. */
            audioMode = 2;
            AudioResumeBackgroundOutput();

            TurnOnCueLED(cue);

            /*
             * FAST REACTION LOOP:
             * No TFT drawing here.
             */
            while(timeCount < maxCounts)
            {
                seed++;

                /*
                 * Final tactile LED button wiring:
                 * B1 = RED   switch sw1 P4[1]
                 * B2 = WHITE switch sw2 P4[3]
                 * B3 = BLUE  switch sw3 P4[5]
                 * B4 = GREEN switch sw4 P4[7]
                 */
                if(sw1_Read() == 0)
                {
                    pressedPose = 1;
                    if(cue == 0)
                    {
                        StartButtonSound(1);
                        reactionTime = timeCount * 2;
                        RecordCorrectResponse(reactionTime);
                        resultType = 1;
                    }
                    else
                    {
                        StartWrongButtonSound();
                        lives--;
                        resultType = 2;
                    }
                    answered = 1;
                    break;
                }
                else if(sw2_Read() == 0)
                {
                    pressedPose = 2;
                    if(cue == 1)
                    {
                        StartButtonSound(2);
                        reactionTime = timeCount * 2;
                        RecordCorrectResponse(reactionTime);
                        resultType = 1;
                    }
                    else
                    {
                        StartWrongButtonSound();
                        lives--;
                        resultType = 2;
                    }
                    answered = 1;
                    break;
                }
                else if(sw3_Read() == 0)
                {
                    pressedPose = 3;
                    if(cue == 2)
                    {
                        StartButtonSound(3);
                        reactionTime = timeCount * 2;
                        RecordCorrectResponse(reactionTime);
                        resultType = 1;
                    }
                    else
                    {
                        StartWrongButtonSound();
                        lives--;
                        resultType = 2;
                    }
                    answered = 1;
                    break;
                }
                else if(sw4_Read() == 0)
                {
                    pressedPose = 4;
                    if(cue == 3)
                    {
                        StartButtonSound(4);
                        reactionTime = timeCount * 2;
                        RecordCorrectResponse(reactionTime);
                        resultType = 1;
                    }
                    else
                    {
                        StartWrongButtonSound();
                        lives--;
                        resultType = 2;
                    }
                    answered = 1;
                    break;
                }

                AudioDelay(2);
                timeCount++;
            }

            /*
             * Return background music to normal volume after the cue/reaction window.
             */
            audioMode = 1;
            AudioResumeBackgroundOutput();

            AllLEDsOff();

            if(answered == 0)
            {
                StartWrongButtonSound();
                lives--;
                resultType = 3;
            }

            if(resultType == 1)
            {
                if(bonusLifeEarned)
                {
                    UpdateGameMessage("+1 LIFE", YELLOW);
                }
                else
                {
                    UpdateGameMessage("CORRECT", GREEN);
                }
            }
            else if(resultType == 2)
            {
                UpdateGameMessage("WRONG", RED);
            }
            else
            {
                UpdateGameMessage("MISSED", RED);
            }

            UpdateScoreLives();

            /*
             * Correct -> keep dancing in the pose for the switch that was pressed.
             * B1/B2/B3/B4 use poses 1/2/3/4.
             * Wrong/missed -> stop dancing immediately.
             */
            if(resultType == 1)
            {
                dancePose = pressedPose;
            }
            else
            {
                dancePose = 0;
                frame = 0;
            }

            /* Hold the button beat/pose longer on Easy so the player can
             * see the transition, hear the button sound, then hear a little
             * background music before the next cue.
             */
            if(difficulty == 0)
            {
                DanceDelay(dancePose, &frame, 800);
            }
            else if(difficulty == 1)
            {
                DanceDelay(dancePose, &frame, 550);
            }
            else
            {
                DanceDelay(dancePose, &frame, 60);
            }

            ClearGameMessage();

            /*
             * Keep dancing between cues after correct answers.
             * Difficulty also makes this faster because interCueDelayMs gets smaller
             * and DanceDelay uses a shorter frame delay on hard mode.
             */
            DanceDelay(dancePose, &frame, interCueDelayMs);
        }

        if(correctResponses > 0)
        {
            avgReactionTime = totalReactionTime / correctResponses;
        }
        else
        {
            avgReactionTime = 0;
            bestReactionTime = 0;
        }

        /*
         * Silence speaker while drawing game over screen, then resume music.
         */
        audioMode = 0;
        PWM_Audio_WriteCompare(0);

        ShowGameOverScreen();

        audioMode = 1;
        AudioRestartBackground();

        while(sw1_Read() == 1 && sw2_Read() == 1 && sw3_Read() == 1 && sw4_Read() == 1)
        {
            seed++;
            AudioDelay(1);
        }

        /* Do not wait here. Go back and draw the START screen immediately.
         * StartScreen() will show the screen first, then wait for release.
         */
    }
}
