/* Provides the copter motor control task */

#ifndef COPTER_H
#define COPTER_H

extern uint16_t LastADCValue;

void copter_task(void *param);

#endif /* COPTER_H */