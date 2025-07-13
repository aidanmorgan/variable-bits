#include "bit_stream.h"

// Helper function to create a BitStreamResult with an error
static BitStreamResult create_error_result(BitStreamErrorCode code, int io_errno) {
    BitStreamResult result;
    result.success = false;
    result.error.code = code;
    result.error.io_errno = io_errno;
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

// Helper function to create a BitStreamResult with success status
static BitStreamResult create_success_result(void) {
    BitStreamResult result;
    result.success = true;
    result.error.code = BIT_STREAM_ERROR_NONE;
    result.error.io_errno = 0;
    return result;
}

BitStreamReader* bit_stream_reader_new(FILE* file) {
    return bit_stream_reader_with_capacity(file, 4096);
}

BitStreamReader* bit_stream_reader_with_capacity(FILE* file, size_t capacity) {
    BitStreamReader* reader = (BitStreamReader*)malloc(sizeof(BitStreamReader));
    if (reader == NULL) {
        return NULL;
    }
    
    reader->file = file;
    reader->buffer = (uint8_t*)malloc(capacity);
    if (reader->buffer == NULL) {
        free(reader);
        return NULL;
    }
    
    reader->buffer_capacity = capacity;
    reader->buffer_size = 0;
    reader->byte_pos = 0;
    reader->bit_pos = 0;
    reader->eof = false;
    
    return reader;
}

void bit_stream_reader_free(BitStreamReader* reader) {
    if (reader != NULL) {
        free(reader->buffer);
        free(reader);
    }
}

// Fill the internal buffer with data from the underlying file
static BitStreamResult fill_buffer(BitStreamReader* reader) {
    // Reset buffer position
    reader->byte_pos = 0;
    reader->bit_pos = 0;
    
    // Read from the underlying file
    size_t bytes_read = fread(reader->buffer, 1, reader->buffer_capacity, reader->file);
    
    if (bytes_read == 0) {
        if (ferror(reader->file)) {
            return create_error_result(BIT_STREAM_ERROR_IO, errno);
        }
        
        // End of file
        reader->buffer_size = 0;
        reader->eof = true;
    } else {
        // Successfully read bytes
        reader->buffer_size = bytes_read;
    }
    
    return create_success_result();
}

BitStreamResult bit_stream_reader_read_bits(BitStreamReader* reader, uint8_t bit_count) {
    if (bit_count == 0 || bit_count > 64) {
        return create_error_result(BIT_STREAM_ERROR_INVALID_BIT_COUNT, 0);
    }
    
    uint64_t result = 0;
    uint8_t bits_read = 0;
    
    while (bits_read < bit_count) {
        // Check if we need to load more data
        if (reader->byte_pos >= reader->buffer_size) {
            if (reader->eof) {
                if (bits_read == 0) {
                    return create_error_result(BIT_STREAM_ERROR_END_OF_STREAM, 0);
                }
                break;
            }
            
            // Load more data from the underlying file
            BitStreamResult fill_result = fill_buffer(reader);
            if (!fill_result.success) {
                return fill_result;
            }
            
            // Check if we reached the end of the file
            if (reader->buffer_size == 0) {
                reader->eof = true;
                if (bits_read == 0) {
                    return create_error_result(BIT_STREAM_ERROR_END_OF_STREAM, 0);
                }
                break;
            }
        }
        
        uint8_t current_byte = reader->buffer[reader->byte_pos];
        uint8_t bits_left_in_byte = 8 - reader->bit_pos;
        uint8_t bits_to_read = (bit_count - bits_read < bits_left_in_byte) ? 
                               (bit_count - bits_read) : bits_left_in_byte;
        
        // Extract bits from the current byte (LSB first)
        uint8_t mask = ((1U << bits_to_read) - 1);
        uint8_t shift = reader->bit_pos;
        uint8_t extracted_bits = (current_byte >> shift) & mask;

        // Add the extracted bits to the result (LSB first)
        result |= ((uint64_t)extracted_bits) << bits_read;
        bits_read += bits_to_read;
        
        // Update position
        reader->bit_pos += bits_to_read;
        if (reader->bit_pos == 8) {
            reader->byte_pos += 1;
            reader->bit_pos = 0;
        }
    }
    
    return create_u64_result(result);
}

BitStreamResult bit_stream_reader_read_bits_u128(BitStreamReader* reader, uint8_t bit_count) {
    if (bit_count == 0 || bit_count > 128) {
        return create_error_result(BIT_STREAM_ERROR_INVALID_BIT_COUNT, 0);
    }
    // For bit counts up to 64, we can use the existing read_bits method
    if (bit_count <= 64) {
        BitStreamResult result = bit_stream_reader_read_bits(reader, bit_count);
        if (!result.success) {
            return result;
        }
        return create_u128_result(uint128_from_u64(result.value.u64));
    }
    // For bit counts > 64, read the low 64 bits first (LSB), then the high bits
    BitStreamResult low_result = bit_stream_reader_read_bits(reader, 64);
    if (!low_result.success) {
        return low_result;
    }
    uint8_t high_bits = bit_count - 64;
    BitStreamResult high_result = bit_stream_reader_read_bits(reader, high_bits);
    if (!high_result.success) {
        return high_result;
    }
    UInt128 result = uint128_from_parts(high_result.value.u64, low_result.value.u64);
    return create_u128_result(result);
}

BitStreamResult bit_stream_reader_read_bit_value(BitStreamReader* reader, uint8_t bit_count) {
    if (bit_count <= 64) {
        BitStreamResult result = bit_stream_reader_read_bits(reader, bit_count);
        if (!result.success) {
            return result;
        }
        return bit_value_new(result.value.u64, bit_count);
    } else {
        BitStreamResult result = bit_stream_reader_read_bits_u128(reader, bit_count);
        if (!result.success) {
            return result;
        }
        return bit_value_new_u128(result.value.u128, bit_count);
    }
}

bool bit_stream_reader_is_eof(const BitStreamReader* reader) {
    return reader->eof && reader->byte_pos >= reader->buffer_size;
}