#ifndef AQ_TERMINAL_H_
#define AQ_TERMINAL_H_

#include <stdbool.h>

bool terminal_supports_colour(void);

#if defined (WIN32)

#define VTSEQ(ID) ("\x1b[1;" #ID "m")

#else // defined (WIN32)

#define VTSEQ(ID) ("\x1b[" #ID "m")

#endif // defined (WIN32)

#endif // AQ_TERMINAL_H_
