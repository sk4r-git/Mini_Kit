#pragma once
#include <linux/fs.h>

int global_write_hook(void);
int global_read_hook(void);
int global_hook(void);
void global_unhook(void);
