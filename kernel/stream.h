#pragma once
#include <linux/wait.h>
#include <linux/mutex.h>

extern char stream_buf[];
extern size_t stream_len;
extern wait_queue_head_t stream_wait;
extern struct mutex stream_lock;

void push_stream_data(const char *data, size_t len);