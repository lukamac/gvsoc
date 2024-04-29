#include "archi_redmule.h"

#define SRC_FMT FP8
typedef uint8_t src_fmt_t;

#define DST_FMT FP8
typedef uint8_t dst_fmt_t;

#define ARRAY_HEIGHT 16
#define PIPE_REGS    3
#define ARRAY_WIDTH  (PIPE_REGS * ARRAY_HEIGHT)

#define DATA_WIDTH 512

