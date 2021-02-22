#ifndef __CLOCK_H__
#define __CLOCK_H__

#include "media_player.h"

void init_clock(Clock *c, int *queue_serial);
int get_master_sync_type(VideoState *is);
double get_master_clock(VideoState *is);
double get_clock(Clock *c);
void set_clock(Clock *c, double pts, int serial);
void set_clock_at(Clock *c, double pts, int serial, double time);
void set_clock_speed(Clock *c, double speed);
void sync_clock_to_slave(Clock *c, Clock *slave);
void check_external_clock_speed(VideoState *is);

#endif /* __CLOCK_H__ */