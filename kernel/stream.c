#include "stream.h"

#include "../include/intercept.h"

char stream_buf[4096];
size_t stream_len;

DECLARE_WAIT_QUEUE_HEAD(stream_wait);
DEFINE_MUTEX(stream_lock);

void push_stream_data(const char *data, size_t len)
{
    mutex_lock(&stream_lock);
    memcpy(stream_buf, data, len);
    stream_len = len;
    mutex_unlock(&stream_lock);
    wake_up_interruptible(&stream_wait);
}

