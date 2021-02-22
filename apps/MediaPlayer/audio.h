#ifndef __AUDIO_H__
#define __AUDIO_H__

#include "media_player.h"

int audio_open(void *opaque, int64_t wanted_channel_layout, int wanted_nb_channels, int wanted_sample_rate, struct AudioParams *audio_hw_params);
int64_t get_valid_channel_layout(int64_t channel_layout, int channels);
int audio_thread(void *arg);

#endif
