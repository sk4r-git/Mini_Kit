#pragma once
#include <linux/fs.h>

int hook_file(struct file *file);
void unhook_file(void);