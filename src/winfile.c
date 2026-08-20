#include "c2_target.h"

#if PLATFORM_WINDOWS

#include <io.h>

// FUNCTION: C2WIN 0x0040f570
int valid_save_file(char *filename)
{
    int handle;

    handle = _open(filename, 0x8000);
    if (handle >= 0) {
        _close(handle);
        return 1;
    }
    return 0;
}

#endif
