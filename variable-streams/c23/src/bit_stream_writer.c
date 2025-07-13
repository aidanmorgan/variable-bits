#include "bit_stream.h"

// Helper function to create a BitStreamResult with an error
static BitStreamResult create_error_result(BitStreamErrorCode code, int io_errno) {
    BitStreamResult result;
    result.success = false;
    result.error.code = code;
    result.error.io_errno = io_errno;
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

BitStreamWriter* bit_stream_writer_new(FILE* file) {
    return bit_stream_writer_with_capacity(file, 4096);
}

BitStreamWriter* bit_stream_writer_with_capacity(FILE* file, size_t capacity) {
    BitStreamWriter* writer = (BitStreamWriter*)malloc(sizeof(BitStreamWriter));
    if (writer == NULL) {
        return NULL;
    }
    
    writer->file = file;
    writer->buffer = (uint8_t*)malloc(capacity);
    if (writer->buffer == NULL) {
        free(writer);
        return NULL;
    }
    
    writer->buffer_capacity = capacity;
    writer->byte_pos = 0;
    writer->bit_pos = 0;
    
    // Initialize buffer to zeros
    memset(writer->buffer, 0, capacity);
    
    return writer;
}

void bit_stream_writer_free(BitStreamWriter* writer) {
    if (writer != NULL) {
        free(writer->buffer);
        free(writer);
    }
}

// Flushes the internal buffer to the underlying file
static BitStreamResult flush_buffer(BitStreamWriter* writer) {
    if (writer->byte_pos > 0) {
        // Write the buffer to the underlying file
        size_t bytes_written = fwrite(writer->buffer, 1, writer->byte_pos, writer->file);
        
        if (bytes_written != writer->byte_pos) {
            return create_error_result(BIT_STREAM_ERROR_IO, errno);
        }
        
        // Reset buffer position
        writer->byte_pos = 0;
        
        // Clear the first byte for the next write
        writer->buffer[0] = 0;
    }
    
    return create_success_result();
}

BitStreamResult bit_stream_writer_write_bits(BitStreamWriter* writer, uint64_t value, uint8_t bit_count) {
    if (bit_count == 0 || bit_count > 64) {
        return create_error_result(BIT_STREAM_ERROR_INVALID_BIT_COUNT, 0);
    }
    
    uint8_t bits_written = 0;
    
    // Ensure the buffer has enough space
    if (writer->byte_pos >= writer->buffer_capacity) {
        BitStreamResult flush_result = flush_buffer(writer);
        if (!flush_result.success) {
            return flush_result;
        }
    }
    
    while (bits_written < bit_count) {
        uint8_t bits_left_in_byte = 8 - writer->bit_pos;
        uint8_t bits_to_write = (bit_count - bits_written < bits_left_in_byte) ? 
                                (bit_count - bits_written) : bits_left_in_byte;
        
        // Extract the bits to write (LSB first)
        uint8_t shift = bits_written;
        uint64_t mask = ((1ULL << bits_to_write) - 1) << shift;
        uint8_t extracted_bits = (uint8_t)((value & mask) >> shift);
        
        // Write the bits to the current byte
        uint8_t byte_shift = bits_left_in_byte - bits_to_write;
        uint8_t byte_mask = ~(((1U << bits_to_write) - 1) << byte_shift);
        writer->buffer[writer->byte_pos] &= byte_mask;
        writer->buffer[writer->byte_pos] |= extracted_bits << byte_shift;
        
        // Update position
        bits_written += bits_to_write;
        writer->bit_pos += bits_to_write;
        if (writer->bit_pos == 8) {
            writer->byte_pos += 1;
            writer->bit_pos = 0;
            
            // If the buffer is full, flush it
            if (writer->byte_pos >= writer->buffer_capacity) {
                BitStreamResult flush_result = flush_buffer(writer);
                if (!flush_result.success) {
                    return flush_result;
                }
            } else {
                // Initialize the next byte to zero
                writer->buffer[writer->byte_pos] = 0;
            }
        }
    }
    
    return create_success_result();
}

BitStreamResult bit_stream_writer_write_bits_u128(BitStreamWriter* writer, UInt128 value, uint8_t bit_count) {
    if (bit_count == 0 || bit_count > 128) {
        return create_error_result(BIT_STREAM_ERROR_INVALID_BIT_COUNT, 0);
    }
    // For bit counts up to 64, we can use the existing write_bits method
    if (bit_count <= 64) {
        return bit_stream_writer_write_bits(writer, value.low, bit_count);
    }
    // For bit counts > 64, write the low 64 bits first (LSB), then the high bits
    BitStreamResult low_result = bit_stream_writer_write_bits(writer, value.low, 64);
    if (!low_result.success) {
        return low_result;
    }
    return bit_stream_writer_write_bits(writer, value.high, bit_count - 64);
}

BitStreamResult bit_stream_writer_write_bit_value(BitStreamWriter* writer, BitValue value, uint8_t bit_count) {
    uint8_t actual_bit_count = bit_count;
    if (actual_bit_count == 0) {
        actual_bit_count = bit_value_bit_count(&value);
    }
    
    if (actual_bit_count <= 64) {
        return bit_stream_writer_write_bits(writer, bit_value_to_u64(&value), actual_bit_count);
    } else {
        return bit_stream_writer_write_bits_u128(writer, bit_value_to_u128(&value), actual_bit_count);
    }
}

BitStreamResult bit_stream_writer_flush(BitStreamWriter* writer) {
    // If there are any bits in the current byte, write it
    if (writer->bit_pos > 0) {
        writer->byte_pos += 1;
        writer->bit_pos = 0;
    }
    
    // Flush the buffer
    BitStreamResult flush_result = flush_buffer(writer);
    if (!flush_result.success) {
        return flush_result;
    }
    
    // Flush the underlying file
    if (fflush(writer->file) != 0) {
        return create_error_result(BIT_STREAM_ERROR_IO, errno);
    }
    
    return create_success_result();
}