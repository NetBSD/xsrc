/*
 * Animation routines
 */

#ifndef _CTWM_ANIMATE_H
#define _CTWM_ANIMATE_H

/* Current code requires these to be leaked */
extern int Animating;
extern bool AnimationActive;
extern bool MaybeAnimate;
extern int AnimationSpeed;
extern struct timeval AnimateTimeout;


void StartAnimation(void);
void StopAnimation(void);
void SetAnimationSpeed(int speed);
void ModifyAnimationSpeed(int incr);
void TryToAnimate(void);

#endif /* _CTWM_ANIMATE_H */
