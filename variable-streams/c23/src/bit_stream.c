#include "bit_stream.h"

// Helper function to create a BitStreamResult with an error
static BitStreamResult create_error_result(BitStreamErrorCode code) {
    BitStreamResult result;
    result.success = false;
    result.error.code = code;
    result.error.io_errno = 0;
    return result;
}

// Helper function to create a BitStreamResult with a u64 value
static BitStreamResult create_u64_result(uint64_t value) {
    BitStreamResult result;
    result.success = true;
    result.error.code = BIT_STREAM_ERROR_NONE;
    result.error.io_errno = 0;
    result.value.u64 = value;
    return result;
}

// Helper function to create a BitStreamResult with a u128 value
static BitStreamResult create_u128_result(UInt128 value) {
    BitStreamResult result;
    result.success = true;
    result.error.code = BIT_STREAM_ERROR_NONE;
    result.error.io_errno = 0;
    result.value.u128 = value;
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

// Helper function to create a BitStreamResult with success status
static BitStreamResult create_success_result(void) {
    BitStreamResult result;
    result.success = true;
    result.error.code = BIT_STREAM_ERROR_NONE;
    result.error.io_errno = 0;
    return result;
}

BitStream* bit_stream_new(void) {
    BitStream* stream = (BitStream*)malloc(sizeof(BitStream));
    if (stream == NULL) {
        return NULL;
    }
    
    stream->buffer = NULL;
    stream->buffer_size = 0;
    stream->buffer_capacity = 0;
    stream->byte_pos = 0;
    stream->bit_pos = 0;
    stream->bit_length = 0;
    
    return stream;
}

BitStream* bit_stream_from_bytes(const uint8_t* bytes, size_t length) {
    BitStream* stream = bit_stream_new();
    if (stream == NULL) {
        return NULL;
    }
    
    if (length > 0) {
        stream->buffer = (uint8_t*)malloc(length);
        if (stream->buffer == NULL) {
            free(stream);
            return NULL;
        }
        
        memcpy(stream->buffer, bytes, length);
        stream->buffer_size = length;
        stream->buffer_capacity = length;
        stream->bit_length = length * 8;
    }
    
    return stream;
}

void bit_stream_free(BitStream* stream) {
    if (stream != NULL) {
        free(stream->buffer);
        free(stream);
    }
}

size_t bit_stream_position(const BitStream* stream) {
    return stream->byte_pos * 8 + stream->bit_pos;
}

BitStreamResult bit_stream_set_position(BitStream* stream, size_t position) {
    if (position > stream->bit_length) {
        return create_error_result(BIT_STREAM_ERROR_END_OF_STREAM);
    }
    
    stream->byte_pos = position / 8;
    stream->bit_pos = position % 8;
    
    return create_success_result();
}

size_t bit_stream_length(const BitStream* stream) {
    return stream->bit_length;
}

bool bit_stream_is_empty(const BitStream* stream) {
    return stream->bit_length == 0;
}

BitStreamResult bit_stream_read_bits(BitStream* stream, uint8_t bit_count) {
    if (bit_count == 0 || bit_count > 64) {
        return create_error_result(BIT_STREAM_ERROR_INVALID_BIT_COUNT);
    }
    
    // Check if we're trying to read beyond the bit length of the stream
    if (bit_stream_position(stream) + bit_count > stream->bit_length) {
        return create_error_result(BIT_STREAM_ERROR_END_OF_STREAM);
    }
    
    uint64_t result = 0;
    uint8_t bits_read = 0;
    
    while (bits_read < bit_count) {
        // Check if we've reached the end of the buffer
        if (stream->byte_pos >= stream->buffer_size) {
            if (bits_read == 0) {
                return create_error_result(BIT_STREAM_ERROR_END_OF_STREAM);
            }
            break;
        }
        
        uint8_t current_byte = stream->buffer[stream->byte_pos];
        uint8_t bits_left_in_byte = 8 - stream->bit_pos;
        uint8_t bits_to_read = (bit_count - bits_read < bits_left_in_byte) ? 
                               (bit_count - bits_read) : bits_left_in_byte;
        
        // Extract bits from the current byte
        uint8_t mask = ((1U << bits_to_read) - 1);
        uint8_t shift = bits_left_in_byte - bits_to_read;
        uint8_t extracted_bits = (current_byte >> shift) & mask;
        
        // Add the extracted bits to the result
        result |= ((uint64_t)extracted_bits) << (bit_count - bits_read - bits_to_read);
        bits_read += bits_to_read;
        
        // Update position
        stream->bit_pos += bits_to_read;
        if (stream->bit_pos == 8) {
            stream->byte_pos += 1;
            stream->bit_pos = 0;
        }
    }
    
    return create_u64_result(result);
}

BitStreamResult bit_stream_read_bits_u128(BitStream* stream, uint8_t bit_count) {
    if (bit_count == 0 || bit_count > 128) {
        return create_error_result(BIT_STREAM_ERROR_INVALID_BIT_COUNT);
    }
    
    // For bit counts up to 64, we can use the existing read_bits method
    if (bit_count <= 64) {
        BitStreamResult result = bit_stream_read_bits(stream, bit_count);
        if (!result.success) {
            return result;
        }
        return create_u128_result(uint128_from_u64(result.value.u64));
    }
    
    // Check if we're trying to read beyond the bit length of the stream
    if (bit_stream_position(stream) + bit_count > stream->bit_length) {
        return create_error_result(BIT_STREAM_ERROR_END_OF_STREAM);
    }
    
    // For bit counts > 64, we need to read in two parts
    uint8_t high_bits = bit_count - 64;
    
    BitStreamResult high_result = bit_stream_read_bits(stream, high_bits);
    if (!high_result.success) {
        return high_result;
    }
    
    BitStreamResult low_result = bit_stream_read_bits(stream, 64);
    if (!low_result.success) {
        return low_result;
    }
    
    // Combine the high and low parts
    UInt128 result = uint128_from_parts(high_result.value.u64, low_result.value.u64);
    
    return create_u128_result(result);
}

BitStreamResult bit_stream_read_bit_value(BitStream* stream, uint8_t bit_count) {
    if (bit_count <= 64) {
        BitStreamResult result = bit_stream_read_bits(stream, bit_count);
        if (!result.success) {
            return result;
        }
        return bit_value_new(result.value.u64, bit_count);
    } else {
        BitStreamResult result = bit_stream_read_bits_u128(stream, bit_count);
        if (!result.success) {
            return result;
        }
        return bit_value_new_u128(result.value.u128, bit_count);
    }
}

BitStreamResult bit_stream_write_bits(BitStream* stream, uint64_t value, uint8_t bit_count) {
    if (bit_count == 0 || bit_count > 64) {
        return create_error_result(BIT_STREAM_ERROR_INVALID_BIT_COUNT);
    }
    
    uint8_t bits_written = 0;
    
    // Ensure the buffer has enough space
    size_t required_bytes = (bit_stream_position(stream) + bit_count + 7) / 8;
    if (required_bytes > stream->buffer_capacity) {
        size_t new_capacity = (required_bytes > stream->buffer_capacity * 2) ? 
                              required_bytes : stream->buffer_capacity * 2;
        if (new_capacity == 0) {
            new_capacity = 1;
        }
        
        uint8_t* new_buffer = (uint8_t*)realloc(stream->buffer, new_capacity);
        if (new_buffer == NULL) {
            return create_error_result(BIT_STREAM_ERROR_IO);
        }
        
        stream->buffer = new_buffer;
        stream->buffer_capacity = new_capacity;
    }
    
    // Ensure buffer_size is large enough
    if (required_bytes > stream->buffer_size) {
        // Zero out any new bytes
        memset(stream->buffer + stream->buffer_size, 0, required_bytes - stream->buffer_size);
        stream->buffer_size = required_bytes;
    }
    
    while (bits_written < bit_count) {
        uint8_t bits_left_in_byte = 8 - stream->bit_pos;
        uint8_t bits_to_write = (bit_count - bits_written < bits_left_in_byte) ? 
                                (bit_count - bits_written) : bits_left_in_byte;
        
        // Extract the bits to write
        uint8_t shift = bit_count - bits_written - bits_to_write;
        uint64_t mask = ((1ULL << bits_to_write) - 1) << shift;
        uint8_t extracted_bits = (uint8_t)((value & mask) >> shift);
        
        // Write the bits to the current byte
        uint8_t byte_shift = bits_left_in_byte - bits_to_write;
        uint8_t byte_mask = ~(((1U << bits_to_write) - 1) << byte_shift);
        stream->buffer[stream->byte_pos] &= byte_mask;
        stream->buffer[stream->byte_pos] |= extracted_bits << byte_shift;
        
        // Update position
        bits_written += bits_to_write;
        stream->bit_pos += bits_to_write;
        if (stream->bit_pos == 8) {
            stream->byte_pos += 1;
            stream->bit_pos = 0;
        }
    }
    
    // Update the bit length if we've written beyond the current length
    size_t new_position = bit_stream_position(stream);
    if (new_position > stream->bit_length) {
        stream->bit_length = new_position;
    }
    
    return create_success_result();
}

BitStreamResult bit_stream_write_bits_u128(BitStream* stream, UInt128 value, uint8_t bit_count) {
    if (bit_count == 0 || bit_count > 128) {
        return create_error_result(BIT_STREAM_ERROR_INVALID_BIT_COUNT);
    }
    
    // For bit counts up to 64, we can use the existing write_bits method
    if (bit_count <= 64) {
        return bit_stream_write_bits(stream, value.low, bit_count);
    }
    
    // For bit counts > 64, we need to write in two parts
    uint8_t high_bits = bit_count - 64;
    
    BitStreamResult high_result = bit_stream_write_bits(stream, value.high, high_bits);
    if (!high_result.success) {
        return high_result;
    }
    
    return bit_stream_write_bits(stream, value.low, 64);
}

BitStreamResult bit_stream_write_bit_value(BitStream* stream, BitValue value, uint8_t bit_count) {
    uint8_t actual_bit_count = bit_count;
    if (actual_bit_count == 0) {
        actual_bit_count = bit_value_bit_count(&value);
    }
    
    if (actual_bit_count <= 64) {
        return bit_stream_write_bits(stream, bit_value_to_u64(&value), actual_bit_count);
    } else {
        return bit_stream_write_bits_u128(stream, bit_value_to_u128(&value), actual_bit_count);
    }
}

uint8_t* bit_stream_into_bytes(BitStream* stream, size_t* length) {
    if (stream == NULL || stream->buffer == NULL) {
        if (length != NULL) {
            *length = 0;
        }
        return NULL;
    }
    
    uint8_t* buffer = stream->buffer;
    *length = stream->buffer_size;
    
    // Detach the buffer from the stream
    stream->buffer = NULL;
    stream->buffer_size = 0;
    stream->buffer_capacity = 0;
    stream->byte_pos = 0;
    stream->bit_pos = 0;
    stream->bit_length = 0;
    
    return buffer;
}

void bit_stream_reset(BitStream* stream) {
    stream->byte_pos = 0;
    stream->bit_pos = 0;
}

bool bit_stream_is_eof(const BitStream* stream) {
    return bit_stream_position(stream) >= stream->bit_length;
}