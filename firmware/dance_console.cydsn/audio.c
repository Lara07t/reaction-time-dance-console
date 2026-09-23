/*
 * audio.c
 * Audio engine: background melody, button tones and error beeps, timed by a 1 ms SysTick interrupt.
 */

#include <project.h>
#include "audio.h"

/*
 * Drives the speaker through Clock_Audio and PWM_Audio.
 *
 * A background melody loops continuously. Correct presses, wrong presses
 * and timeouts play a short effect that takes over the speaker and then
 * hands it back; the melody keeps its original timing throughout.
 *
 * All timing runs on a 1 ms SysTick interrupt, independent of the game
 * loop, so screen drawing never slows or stretches the music.
 */

/* Background melody timing, in ms */
#define BG_NOTE_TIME         220
#define BG_NOTE_GAP           90
#define BG_LOOP_GAP          160

/* One tone per correct press, and one low beep for a wrong press or timeout, in ms */
#define BUTTON_PRESS_TIME    200
#define WRONG_BEEP_TIME      160

/* Volume dividers: a bigger divider gives a softer sound */
#define MUSIC_VOLUME_DIVIDER        60
#define CUE_MUSIC_VOLUME_DIVIDER    90

#define AUDIO_SOURCE_BACKGROUND      0
#define AUDIO_SOURCE_BUTTON          1

typedef struct
{
    uint8 period;
    uint16 durationMs;
    uint8 isTone;
} AudioStep;

#define AUDIO_TONE(p, d) {p, d, 1}
#define AUDIO_REST(d)    {0, d, 0}

/* Background melody: a slow C minor loop with space between notes */
const AudioStep backgroundMelody[] = {
    AUDIO_TONE(NOTE_C4,  BG_NOTE_TIME), AUDIO_REST(BG_NOTE_GAP),
    AUDIO_TONE(NOTE_G4,  BG_NOTE_TIME), AUDIO_REST(BG_NOTE_GAP),
    AUDIO_TONE(NOTE_DS4, BG_NOTE_TIME), AUDIO_REST(BG_NOTE_GAP),
    AUDIO_TONE(NOTE_G4,  BG_NOTE_TIME), AUDIO_REST(BG_LOOP_GAP)
};

/* Button tones: one short tone per correct press, from highest (B1) to lowest (B4) */

/* B1 / red: highest */
const AudioStep button1Sound[] = {
    AUDIO_TONE(NOTE_C5, BUTTON_PRESS_TIME)
};

/* B2 / white */
const AudioStep button2Sound[] = {
    AUDIO_TONE(NOTE_G4, BUTTON_PRESS_TIME)
};

/* B3 / blue */
const AudioStep button3Sound[] = {
    AUDIO_TONE(NOTE_DS4, BUTTON_PRESS_TIME)
};

/* B4 / green: lowest */
const AudioStep button4Sound[] = {
    AUDIO_TONE(NOTE_C4, BUTTON_PRESS_TIME)
};

/* Wrong button or timeout */
const AudioStep wrongButtonSound[] = {
    AUDIO_TONE(NOTE_B3, WRONG_BEEP_TIME)
};

#define BACKGROUND_MELODY_LEN    (sizeof(backgroundMelody) / sizeof(backgroundMelody[0]))
#define BUTTON1_SOUND_LEN        (sizeof(button1Sound) / sizeof(button1Sound[0]))
#define BUTTON2_SOUND_LEN        (sizeof(button2Sound) / sizeof(button2Sound[0]))
#define BUTTON3_SOUND_LEN        (sizeof(button3Sound) / sizeof(button3Sound[0]))
#define BUTTON4_SOUND_LEN        (sizeof(button4Sound) / sizeof(button4Sound[0]))
#define WRONG_BUTTON_SOUND_LEN   (sizeof(wrongButtonSound) / sizeof(wrongButtonSound[0]))

volatile uint16 backgroundIndex = 0;
volatile uint16 backgroundCurrentIndex = 0;
volatile uint16 backgroundTimeRemaining = 0;
volatile uint8 backgroundStepValid = 0;

volatile uint16 effectIndex = 0;
volatile uint16 effectLength = 0;
volatile uint16 effectStepTimeRemaining = 0;
volatile uint16 effectTimeRemaining = 0;
volatile uint8 effectActive = 0;

/* Audio mode (see audio.h) */
volatile uint8 audioMode = 1;
volatile uint8 freestyleAudioActive = 0;

const AudioStep * volatile effectSound = 0;

static void AudioWriteStep(const AudioStep *step, uint8 source)
{
    uint8 compareValue;

    if(step->isTone)
    {
        PWM_Audio_WritePeriod(step->period);

        if(source == AUDIO_SOURCE_BUTTON)
        {
            compareValue = step->period / BUTTON_VOLUME_DIVIDER;
        }
        else if(audioMode == 2)
        {
            compareValue = step->period / CUE_MUSIC_VOLUME_DIVIDER;
        }
        else
        {
            compareValue = step->period / MUSIC_VOLUME_DIVIDER;
        }

        if(compareValue < 1)
        {
            compareValue = 1;
        }

        PWM_Audio_WriteCompare(compareValue);
    }
    else
    {
        PWM_Audio_WriteCompare(0);
    }
}

void AudioRestartBackground(void)
{
    backgroundIndex = 0;
    backgroundCurrentIndex = 0;
    backgroundTimeRemaining = 0;
    backgroundStepValid = 0;
}

static void AudioStartBackgroundStep(uint8 audible)
{
    if(backgroundIndex >= BACKGROUND_MELODY_LEN)
    {
        backgroundIndex = 0;
    }

    backgroundCurrentIndex = backgroundIndex;
    backgroundStepValid = 1;
    backgroundTimeRemaining = backgroundMelody[backgroundCurrentIndex].durationMs;

    if(audible)
    {
        AudioWriteStep(&backgroundMelody[backgroundCurrentIndex], AUDIO_SOURCE_BACKGROUND);
    }

    backgroundIndex++;
}

void AudioResumeBackgroundOutput(void)
{
    if(audioMode == 0)
    {
        PWM_Audio_WriteCompare(0);
        return;
    }

    if(backgroundStepValid && backgroundTimeRemaining > 0)
    {
        AudioWriteStep(&backgroundMelody[backgroundCurrentIndex], AUDIO_SOURCE_BACKGROUND);
    }
    else
    {
        backgroundTimeRemaining = 0;
    }
}

static void AudioAdvanceBackground(uint8 audible)
{
    if(backgroundTimeRemaining > 0)
    {
        backgroundTimeRemaining--;
        return;
    }

    if(audioMode == 0)
    {
        if(audible)
        {
            PWM_Audio_WriteCompare(0);
        }
        backgroundStepValid = 0;
        backgroundTimeRemaining = 1;
        return;
    }

    AudioStartBackgroundStep(audible);
}

static void StopButtonEffect(void)
{
    effectActive = 0;
    effectSound = 0;
    effectIndex = 0;
    effectLength = 0;
    effectStepTimeRemaining = 0;
    effectTimeRemaining = 0;

    /* End the effect; the background resumes at its current point in the melody. */
    PWM_Audio_WriteCompare(0);
    AudioResumeBackgroundOutput();
}

void AudioTick1ms(void)
{
    const AudioStep *step;
    uint16 stepTime;

    /*
     * In freestyle mode, the button tone is held manually while the
     * button is physically pressed. Do not let the SysTick background
     * audio system overwrite the PWM compare value during that hold.
     */
    if(freestyleAudioActive)
    {
        return;
    }

    /*
     * Effects have speaker priority. The background keeps counting its note
     * timing underneath, so it resumes in step when the effect ends.
     */
    if(effectActive)
    {
        if(effectTimeRemaining == 0)
        {
            StopButtonEffect();
            return;
        }

        if(effectStepTimeRemaining > 0)
        {
            effectStepTimeRemaining--;
            effectTimeRemaining--;
            return;
        }

        if(effectIndex < effectLength)
        {
            step = &effectSound[effectIndex];
            stepTime = step->durationMs;

            if(stepTime > effectTimeRemaining)
            {
                stepTime = effectTimeRemaining;
            }

            AudioWriteStep(step, AUDIO_SOURCE_BUTTON);
            effectStepTimeRemaining = stepTime;
            effectIndex++;
            return;
        }

        StopButtonEffect();
        return;
    }

    AudioAdvanceBackground(1);
}

void AudioDelay(uint16 durationMs)
{
    /* Plain delay: audio timing runs independently on the SysTick interrupt. */
    CyDelay(durationMs);
}

static void AudioSysTickCallback(void)
{
    AudioTick1ms();
}

static void AudioStartInterrupt(void)
{
    /* SysTick calls AudioTick1ms() every 1 ms, using callback slot 0. */
    CySysTickStart();
    CySysTickSetCallback(0, AudioSysTickCallback);
}

void StartButtonSound(uint8 buttonNumber)
{
    uint8 interruptState;

    interruptState = CyEnterCriticalSection();

    if(buttonNumber == 1)
    {
        effectSound = button1Sound;
        effectLength = BUTTON1_SOUND_LEN;
    }
    else if(buttonNumber == 2)
    {
        effectSound = button2Sound;
        effectLength = BUTTON2_SOUND_LEN;
    }
    else if(buttonNumber == 3)
    {
        effectSound = button3Sound;
        effectLength = BUTTON3_SOUND_LEN;
    }
    else
    {
        effectSound = button4Sound;
        effectLength = BUTTON4_SOUND_LEN;
    }

    /* Play the tone for this button over the background melody. */
    effectActive = 1;
    effectIndex = 0;
    effectStepTimeRemaining = 0;
    effectTimeRemaining = BUTTON_PRESS_TIME;

    CyExitCriticalSection(interruptState);
}

void StartWrongButtonSound(void)
{
    uint8 interruptState;

    interruptState = CyEnterCriticalSection();

    /* Wrong button or timeout: one low beep over the background melody. */
    effectSound = wrongButtonSound;
    effectLength = WRONG_BUTTON_SOUND_LEN;
    effectActive = 1;
    effectIndex = 0;
    effectStepTimeRemaining = 0;
    effectTimeRemaining = WRONG_BEEP_TIME;

    CyExitCriticalSection(interruptState);
}

void AudioInit(void)
{
    Clock_Audio_Start();
    PWM_Audio_Start();
    PWM_Audio_WriteCompare(0);

    AudioRestartBackground();

    effectIndex = 0;
    effectLength = 0;
    effectStepTimeRemaining = 0;
    effectTimeRemaining = 0;
    effectActive = 0;
    audioMode = 1;
    effectSound = 0;

    AudioStartInterrupt();
}
