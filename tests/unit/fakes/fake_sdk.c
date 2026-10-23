#include <stdlib.h>

#include "exceptions.h"

static try_context_t* current_context;

try_context_t* try_context_get(void) { return current_context; }

try_context_t* try_context_set(try_context_t* context) {
    try_context_t* previous = current_context;
    current_context = context;
    return previous;
}

// Same semantics as the device: jump to the innermost TRY, or abort
void os_longjmp(unsigned int exception) {
    if (current_context == NULL) {
        abort();
    }
    longjmp(current_context->jmp_buf, exception);
}
