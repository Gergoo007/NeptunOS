#pragma once

#define foreach(var, l) for (i64 var = 0; var < (l); var++)

#define offsetof(s, m) __builtin_offsetof(s, m)

#define checkalign(ptr, align) if ((u64)ptr % align) printk("%s not aligned (%d)\n", #ptr, align);
