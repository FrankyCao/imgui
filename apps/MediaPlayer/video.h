#ifndef __VIDEO_H__
#define __VIDEO_H__

#include "media_player.h"
int video_thread(void *arg);
void video_refresh(void *opaque, double *remaining_time);

#endif

