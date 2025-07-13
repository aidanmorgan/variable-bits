#include "bit_stream.h"

UInt128 uint128_from_u64(uint64_t value) {
    UInt128 result;
    result.high = 0;
    result.low = value;
    return result;
}

UInt128 uint128_from_parts(uint64_t high, uint64_t low) {
    UInt128 result;
    result.high = high;
    result.low = low;
    return result;
}

Int128 int128_from_i64(int64_t value) {
    Int128 result;
    result.high = (value < 0) ? -1 : 0;
    result.low = (uint64_t)value;
    return result;
}

Int128 int128_from_parts(int64_t high, uint64_t low) {
    Int128 result;
    result.high = high;
    result.low = low;
    return result;
}

UInt128 uint128_add(UInt128 a, UInt128 b) {
    UInt128 result;
    result.low = a.low + b.low;
    result.high = a.high + b.high + (result.low < a.low); // Carry if overflow
    return result;
}

UInt128 uint128_subtract(UInt128 a, UInt128 b) {
    UInt128 result;
    result.low = a.low - b.low;
    result.high = a.high - b.high - (a.low < b.low); // Borrow if underflow
    return result;
}

UInt128 uint128_shift_left(UInt128 value, unsigned int shift) {
    UInt128 result;
    
    if (shift >= 128) {
        result.high = 0;
        result.low = 0;
        return result;
    }
    
    if (shift >= 64) {
        result.high = value.low << (shift - 64);
        result.low = 0;
    } else if (shift > 0) {
        result.high = (value.high << shift) | (value.low >> (64 - shift));
        result.low = value.low << shift;
    } else {
        result = value;
    }
    
    return result;
}

UInt128 uint128_shift_right(UInt128 value, unsigned int shift) {
    UInt128 result;
    
    if (shift >= 128) {
        result.high = 0;
        result.low = 0;
        return result;
    }
    
    if (shift >= 64) {
        result.low = value.high >> (shift - 64);
        result.high = 0;
    } else if (shift > 0) {
        result.low = (value.low >> shift) | (value.high << (64 - shift));
        result.high = value.high >> shift;
    } else {
        result = value;
    }
    
    return result;
}

UInt128 uint128_and(UInt128 a, UInt128 b) {
    UInt128 result;
    result.high = a.high & b.high;
    result.low = a.low & b.low;
    return result;
}

UInt128 uint128_or(UInt128 a, UInt128 b) {
    UInt128 result;
    result.high = a.high | b.high;
    result.low = a.low | b.low;
    return result;
}

UInt128 uint128_xor(UInt128 a, UInt128 b) {
    UInt128 result;
    result.high = a.high ^ b.high;
    result.low = a.low ^ b.low;
    return result;
}

UInt128 uint128_not(UInt128 value) {
    UInt128 result;
    result.high = ~value.high;
    result.low = ~value.low;
    return result;
}

bool uint128_equal(UInt128 a, UInt128 b) {
    return a.high == b.high && a.low == b.low;
}

int uint128_compare(UInt128 a, UInt128 b) {
    if (a.high > b.high) {
        return 1;
    } else if (a.high < b.high) {
        return -1;
    } else if (a.low > b.low) {
        return 1;
    } else if (a.low < b.low) {
        return -1;
    } else {
        return 0;
    }
}