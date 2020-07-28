#ifndef PID_CONTROL_H
#define PID_CONTROL_H

#include "pid_config.h"

struct pid_state {
    const float KP;                   /* proportional gain */
    const float KI;                   /* integral gain */
    const float KD;                   /* derivative gain */
    const float T;                    /* controller period */
    const float MINSUM;               /* minimum integral value before gain */
    const float MAXSUM;               /* maximum integral value before gain */
    const int DERIV_INTERVAL;         /* time difference between derivative terms */
    float x[MAX_DERIV_INTERVAL + 1];  /* previous inputs */
    float sum;                        /* current value of sum term */
};

/**
 * Computes the next output from the pid state and current input
 */
float pid_tick(struct pid_state *state, float input);

#endif /* PID_CONTROL_H */