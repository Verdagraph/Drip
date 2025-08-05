#ifndef UTILS_H
#define UTILS_H

#define NUM_MS_IN_S 1000U
#define NUM_S_IN_MIN 60U

#define MS_TO_S (timeMs) (timeMs / NUM_MS_IN_S)
#define MS_TO_MIN (timeMs) (timeMs / (NUM_MS_IN_S * NUM_S_IN_MIN) )

#endif