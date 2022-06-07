
// initialize speed handling, register frame alarm
void speed_init(CPU *cpu, int cyclespersec, int msperframe);

// set the target speed of the emulator
void speed_set_percent(unsigned int percent);

