#include "bit_stream.h"

// Helper function to create a BitStreamResult with an error
static BitStreamResult create_error_result(BitStreamErrorCode code) {
    BitStreamResult result;
    result.success = false;
    result.error.code = code;
    result.error.io_errno = 0;
    return result;
}

// Helper function to create a BitStreamResult with a BitValue
static BitStreamResult create_bit_value_result(BitValue value) {
    BitStreamResult result;
    result.success = true;
    result.error.code = BIT_STREAM_ERROR_NONE;
    result.error.io_errno = 0;
    result.value.bit_value = value;
    return result;
}

BitStreamResult bit_value_new(uint64_t value, uint8_t bit_count) {
    if (bit_count == 0 || bit_count > 128) {
        return create_error_result(BIT_STREAM_ERROR_INVALID_BIT_COUNT);
    }

    BitValue bit_value;

    // For bit counts up to 64, we can use the value directly
    if (bit_count <= 64) {
        // Mask the value to ensure it only contains the specified number of bits
        uint64_t masked_value = (bit_count == 64) ? value : (value & ((1ULL << bit_count) - 1));

        // Choose the appropriate variant based on the bit count
        if (bit_count <= 8) {
            bit_value.type = BIT_VALUE_TYPE_U8;
            bit_value.value.u8 = (uint8_t)masked_value;
        } else if (bit_count <= 16) {
            bit_value.type = BIT_VALUE_TYPE_U16;
            bit_value.value.u16 = (uint16_t)masked_value;
        } else if (bit_count <= 32) {
            bit_value.type = BIT_VALUE_TYPE_U32;
            bit_value.value.u32 = (uint32_t)masked_value;
        } else {
            bit_value.type = BIT_VALUE_TYPE_U64;
            bit_value.value.u64 = masked_value;
        }
    } else {
        // For bit counts > 64, we need to use u128
        // Since the input is u64, we can just convert it to u128
        bit_value.type = BIT_VALUE_TYPE_U128;
        bit_value.value.u128 = uint128_from_u64(value);
    }

    return create_bit_value_result(bit_value);
}

BitStreamResult bit_value_new_u128(UInt128 value, uint8_t bit_count) {
    if (bit_count == 0 || bit_count > 128) {
        return create_error_result(BIT_STREAM_ERROR_INVALID_BIT_COUNT);
    }

    BitValue bit_value;

    // Mask the value to ensure it only contains the specified number of bits
    UInt128 masked_value;
    if (bit_count == 128) {
        masked_value = value; // No need to mask for 128 bits
    } else if (bit_count <= 64) {
        // For bit counts <= 64, we only need to mask the low part
        masked_value.high = 0;
        masked_value.low = (bit_count == 64) ? value.low : (value.low & ((1ULL << bit_count) - 1));
    } else {
        // For bit counts > 64, we need to mask both parts
        masked_value.low = value.low;
        masked_value.high = value.high & ((1ULL << (bit_count - 64)) - 1);
    }

    // Choose the appropriate variant based on the bit count
    if (bit_count <= 8) {
        bit_value.type = BIT_VALUE_TYPE_U8;
        bit_value.value.u8 = (uint8_t)masked_value.low;
    } else if (bit_count <= 16) {
        bit_value.type = BIT_VALUE_TYPE_U16;
        bit_value.value.u16 = (uint16_t)masked_value.low;
    } else if (bit_count <= 32) {
        bit_value.type = BIT_VALUE_TYPE_U32;
        bit_value.value.u32 = (uint32_t)masked_value.low;
    } else if (bit_count <= 64) {
        bit_value.type = BIT_VALUE_TYPE_U64;
        bit_value.value.u64 = masked_value.low;
    } else {
        bit_value.type = BIT_VALUE_TYPE_U128;
        bit_value.value.u128 = masked_value;
    }

    return create_bit_value_result(bit_value);
}

BitStreamResult bit_value_new_signed(int64_t value, uint8_t bit_count) {
    if (bit_count == 0 || bit_count > 64) {
        return create_error_result(BIT_STREAM_ERROR_INVALID_BIT_COUNT);
    }

    BitValue bit_value;

    // Choose the appropriate variant based on the bit count
    if (bit_count <= 8) {
        bit_value.type = BIT_VALUE_TYPE_I8;
        bit_value.value.i8 = (int8_t)value;
    } else if (bit_count <= 16) {
        bit_value.type = BIT_VALUE_TYPE_I16;
        bit_value.value.i16 = (int16_t)value;
    } else if (bit_count <= 32) {
        bit_value.type = BIT_VALUE_TYPE_I32;
        bit_value.value.i32 = (int32_t)value;
    } else {
        bit_value.type = BIT_VALUE_TYPE_I64;
        bit_value.value.i64 = value;
    }

    return create_bit_value_result(bit_value);
}

BitStreamResult bit_value_new_i128(Int128 value, uint8_t bit_count) {
    if (bit_count == 0 || bit_count > 128) {
        return create_error_result(BIT_STREAM_ERROR_INVALID_BIT_COUNT);
    }

    BitValue bit_value;

    // Choose the appropriate variant based on the bit count
    if (bit_count <= 8) {
        bit_value.type = BIT_VALUE_TYPE_I8;
        bit_value.value.i8 = (int8_t)value.low;
    } else if (bit_count <= 16) {
        bit_value.type = BIT_VALUE_TYPE_I16;
        bit_value.value.i16 = (int16_t)value.low;
    } else if (bit_count <= 32) {
        bit_value.type = BIT_VALUE_TYPE_I32;
        bit_value.value.i32 = (int32_t)value.low;
    } else if (bit_count <= 64) {
        bit_value.type = BIT_VALUE_TYPE_I64;
        bit_value.value.i64 = (int64_t)value.low;
    } else {
        bit_value.type = BIT_VALUE_TYPE_I128;
        bit_value.value.i128 = value;
    }

    return create_bit_value_result(bit_value);
}

uint8_t bit_value_bit_count(const BitValue* value) {
    switch (value->type) {
        case BIT_VALUE_TYPE_U8:
        case BIT_VALUE_TYPE_I8:
            return 8;
        case BIT_VALUE_TYPE_U16:
        case BIT_VALUE_TYPE_I16:
            return 16;
        case BIT_VALUE_TYPE_U32:
        case BIT_VALUE_TYPE_I32:
            return 32;
        case BIT_VALUE_TYPE_U64:
        case BIT_VALUE_TYPE_I64:
            return 64;
        case BIT_VALUE_TYPE_U128:
        case BIT_VALUE_TYPE_I128:
            return 128;
        default:
            return 0; // Should never happen
    }
}

uint64_t bit_value_to_u64(const BitValue* value) {
    switch (value->type) {
        case BIT_VALUE_TYPE_U8:
            return (uint64_t)value->value.u8;
        case BIT_VALUE_TYPE_U16:
            return (uint64_t)value->value.u16;
        case BIT_VALUE_TYPE_U32:
            return (uint64_t)value->value.u32;
        case BIT_VALUE_TYPE_U64:
            return value->value.u64;
        case BIT_VALUE_TYPE_U128:
            return value->value.u128.low; // Truncate to low 64 bits
        case BIT_VALUE_TYPE_I8:
            return (uint64_t)(int64_t)value->value.i8;
        case BIT_VALUE_TYPE_I16:
            return (uint64_t)(int64_t)value->value.i16;
        case BIT_VALUE_TYPE_I32:
            return (uint64_t)(int64_t)value->value.i32;
        case BIT_VALUE_TYPE_I64:
            return (uint64_t)value->value.i64;
        case BIT_VALUE_TYPE_I128:
            return (uint64_t)value->value.i128.low; // Truncate to low 64 bits
        default:
            return 0; // Should never happen
    }
}

UInt128 bit_value_to_u128(const BitValue* value) {
    UInt128 result = {0, 0};

    switch (value->type) {
        case BIT_VALUE_TYPE_U8:
            result.low = (uint64_t)value->value.u8;
            break;
        case BIT_VALUE_TYPE_U16:
            result.low = (uint64_t)value->value.u16;
            break;
        case BIT_VALUE_TYPE_U32:
            result.low = (uint64_t)value->value.u32;
            break;
        case BIT_VALUE_TYPE_U64:
            result.low = value->value.u64;
            break;
        case BIT_VALUE_TYPE_U128:
            return value->value.u128;
        case BIT_VALUE_TYPE_I8:
            result.low = (uint64_t)(int64_t)value->value.i8;
            result.high = (value->value.i8 < 0) ? UINT64_MAX : 0;
            break;
        case BIT_VALUE_TYPE_I16:
            result.low = (uint64_t)(int64_t)value->value.i16;
            result.high = (value->value.i16 < 0) ? UINT64_MAX : 0;
            break;
        case BIT_VALUE_TYPE_I32:
            result.low = (uint64_t)(int64_t)value->value.i32;
            result.high = (value->value.i32 < 0) ? UINT64_MAX : 0;
            break;
        case BIT_VALUE_TYPE_I64:
            result.low = (uint64_t)value->value.i64;
            result.high = (value->value.i64 < 0) ? UINT64_MAX : 0;
            break;
        case BIT_VALUE_TYPE_I128:
            result.low = value->value.i128.low;
            result.high = (uint64_t)value->value.i128.high;
            break;
    }

    return result;
}

int64_t bit_value_to_i64(const BitValue* value) {
    switch (value->type) {
        case BIT_VALUE_TYPE_U8:
            return (int64_t)value->value.u8;
        case BIT_VALUE_TYPE_U16:
            return (int64_t)value->value.u16;
        case BIT_VALUE_TYPE_U32:
            return (int64_t)value->value.u32;
        case BIT_VALUE_TYPE_U64:
            return (int64_t)value->value.u64; // May truncate for large values
        case BIT_VALUE_TYPE_U128:
            return (int64_t)value->value.u128.low; // Truncate to low 64 bits
        case BIT_VALUE_TYPE_I8:
            return (int64_t)value->value.i8;
        case BIT_VALUE_TYPE_I16:
            return (int64_t)value->value.i16;
        case BIT_VALUE_TYPE_I32:
            return (int64_t)value->value.i32;
        case BIT_VALUE_TYPE_I64:
            return value->value.i64;
        case BIT_VALUE_TYPE_I128:
            return (int64_t)value->value.i128.low; // Truncate to low 64 bits
        default:
            return 0; // Should never happen
    }
}

Int128 bit_value_to_i128(const BitValue* value) {
    Int128 result = {0, 0};

    switch (value->type) {
        case BIT_VALUE_TYPE_U8:
            result.low = (uint64_t)value->value.u8;
            break;
        case BIT_VALUE_TYPE_U16:
            result.low = (uint64_t)value->value.u16;
            break;
        case BIT_VALUE_TYPE_U32:
            result.low = (uint64_t)value->value.u32;
            break;
        case BIT_VALUE_TYPE_U64:
            result.low = value->value.u64;
            break;
        case BIT_VALUE_TYPE_U128:
            result.low = value->value.u128.low;
            result.high = (int64_t)value->value.u128.high;
            break;
        case BIT_VALUE_TYPE_I8:
            result.low = (uint64_t)(int64_t)value->value.i8;
            result.high = (value->value.i8 < 0) ? -1 : 0;
            break;
        case BIT_VALUE_TYPE_I16:
            result.low = (uint64_t)(int64_t)value->value.i16;
            result.high = (value->value.i16 < 0) ? -1 : 0;
            break;
        case BIT_VALUE_TYPE_I32:
            result.low = (uint64_t)(int64_t)value->value.i32;
            result.high = (value->value.i32 < 0) ? -1 : 0;
            break;
        case BIT_VALUE_TYPE_I64:
            result.low = (uint64_t)value->value.i64;
            result.high = (value->value.i64 < 0) ? -1 : 0;
            break;
        case BIT_VALUE_TYPE_I128:
            return value->value.i128;
    }

    return result;
}

bool bit_value_is_signed(const BitValue* value) {
    switch (value->type) {
        case BIT_VALUE_TYPE_I8:
        case BIT_VALUE_TYPE_I16:
        case BIT_VALUE_TYPE_I32:
        case BIT_VALUE_TYPE_I64:
        case BIT_VALUE_TYPE_I128:
            return true;
        default:
            return false;
    }
}