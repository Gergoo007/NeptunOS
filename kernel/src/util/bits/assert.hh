#pragma once

#define assert(c) if (!(c)) fatal("Assert failed: %s (" __FILE__ ":%d)", #c, __LINE__)
#define assert_eq(c1, c2) if ((c1) != (c2)) fatal("Assert failed: expected %s == %s (%lld vs %lld) (" __FILE__ ":%d)", #c1, #c2, c1, c2, __LINE__)
#define assert_neq(c1, c2) if ((c1) == (c2)) fatal("Assert failed: expected %s != %s (%lld vs %lld) (" __FILE__ ":%d)", #c1, #c2, c1, c2, __LINE__)

#define dbg_assert(c) if constexpr (DBG) if (!(c)) fatal("Assert failed: %s (" __FILE__ ":%d)", #c, __LINE__)
