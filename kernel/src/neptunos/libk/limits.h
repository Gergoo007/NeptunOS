#pragma once

#undef  SCHAR_MIN
#undef  SCHAR_MAX
#undef  UCHAR_MAX
#define SCHAR_MAX __SCHAR_MAX__
#define SCHAR_MIN (-__SCHAR_MAX__-1)
#define UCHAR_MAX (__SCHAR_MAX__*2  +1)

#undef  SHRT_MIN
#undef  SHRT_MAX
#undef  USHRT_MAX
#define SHRT_MAX  __SHRT_MAX__
#define SHRT_MIN  (-__SHRT_MAX__ -1)
#define USHRT_MAX (__SHRT_MAX__ *2  +1)

#undef  INT_MIN
#undef  INT_MAX
#undef  UINT_MAX
#define INT_MAX   __INT_MAX__
#define LONG_MAX  __LONG_MAX__
#define INT_MIN   (-__INT_MAX__  -1)
#define UINT_MAX  (__INT_MAX__  *2U +1U)

#undef  LONG_MIN
#undef  LONG_MAX
#undef  ULONG_MAX
#define LONG_MIN  (-__LONG_MAX__ -1L)
#define ULONG_MAX (__LONG_MAX__ *2UL+1UL)

#undef  CHAR_BIT
#undef  CHAR_MIN
#undef  CHAR_MAX
#define CHAR_BIT  __CHAR_BIT__
#define CHAR_MIN SCHAR_MIN
#define CHAR_MAX __SCHAR_MAX__

#define MB_LEN_MAX 1
