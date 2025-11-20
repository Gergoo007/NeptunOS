#pragma once

#define SERIALPRINTK

#define VECTOR_NULLBYDEFAULT

constexpr bool debug = true;
constexpr bool stacktrace_on_fatal = true;
constexpr bool ioapic_fix = true;
static constexpr unsigned long long default_vec_size = 0;
