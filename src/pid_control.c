#include "pid_control.h"

float pid_tick(struct pid_state *state, float input)
{
    /* put input into input array */
    float *x = state->x;
    for (int i = state->DERIV_INTERVAL; i > 0; --i)
    {
        x[i] = x[i - 1];
    }
    x[0] = input;

    state->sum = state->sum + state->T * x[0];
    if (state->sum > state->MAXSUM) {
        state->sum = state->MAXSUM;
    } else if (state->sum < state->MINSUM) {
        state->sum = state->MINSUM;
    }

    float deriv = (x[0] - x[state->DERIV_INTERVAL]) / (state->DERIV_INTERVAL * state->T);
    float output = x[0] * state->KP + state->sum * state->KI + deriv * state->KD;

    return output;
}