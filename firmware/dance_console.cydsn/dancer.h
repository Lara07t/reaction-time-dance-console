/*
 * dancer.h
 * Pixel dancer sprite: a 64x96 off-screen buffer, line and rectangle drawing, and the pose animations.
 */

#ifndef DANCER_H
#define DANCER_H

#include <project.h>

/* Sprite size and its position on the screen */
#define SPR_W 64
#define SPR_H 96
#define SPR_X 88
#define SPR_Y 145

void DrawDancerFrame(uint8 pose, uint8 legFrame);

#endif /* DANCER_H */
