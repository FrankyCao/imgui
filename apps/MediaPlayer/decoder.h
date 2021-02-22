#ifndef __DECODER_H__
#define __DECODER_H__

#include "media_player.h"

int decoder_init(Decoder *d, AVCodecContext *avctx, PacketQueue *queue, SDL_cond *empty_queue_cond);
void decoder_abort(Decoder *d, FrameQueue *fq);
void decoder_destroy(Decoder *d);
int decoder_start(Decoder *d, int (*fn)(void *), const char *thread_name, void* arg);
int decoder_decode_frame(Decoder *d, AVFrame *frame, AVSubtitle *sub);

#endif /* __DECODER_H__ */