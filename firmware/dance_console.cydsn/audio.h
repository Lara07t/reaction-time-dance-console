/*
 * audio.h
 * Audio engine: background melody, button tones and error beeps, timed by a 1 ms SysTick interrupt.
 */

#ifndef AUDIO_H
#define AUDIO_H

#include <project.h>

/* Notes as PWM period values: a smaller period gives a higher pitch */
#define NOTE_C5      47
#define NOTE_G4      63
#define NOTE_DS4     79    /* E-flat 4 */
#define NOTE_C4      95
#define NOTE_B3     100    /* error beep */

/* Button tones are louder than the background (smaller divider = louder) */
#define BUTTON_VOLUME_DIVIDER        6

/*
 * Audio mode:
 * 0 = silent
 * 1 = normal background music
 * 2 = quiet background music during the cue/reaction window
 */
extern volatile uint8 audioMode;

/* Set while a Freestyle tone is held, so the SysTick engine leaves the speaker alone */
extern volatile uint8 freestyleAudioActive;

void AudioRestartBackground(void);
void AudioResumeBackgroundOutput(void);
void AudioTick1ms(void);
void AudioDelay(uint16 durationMs);
void StartButtonSound(uint8 buttonNumber);
void StartWrongButtonSound(void);
void AudioInit(void);

#endif /* AUDIO_H */
