#include "config.h"

#if HAVE_NEWLCD
#include "lcd-new.c"
#else
#include "lcd-old.c"
#endif
