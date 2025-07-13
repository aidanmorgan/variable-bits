#ifndef BIT_STREAM_H
#define BIT_STREAM_H

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

// Error handling
typedef enum {
    BIT_STREAM_ERROR_NONE = 0,
    BIT_STREAM_ERROR_IO,
    BIT_STREAM_ERROR_INVALID_BIT_COUNT,
    BIT_STREAM_ERROR_END_OF_STREAM
} BitStreamErrorCode;

typedef struct {
    BitStreamErrorCode code;
    int io_errno;  // Set when code is BIT_STREAM_ERROR_IO
} BitStreamError;

// BitValue type
typedef enum {
    BIT_VALUE_TYPE_U8,
    BIT_VALUE_TYPE_U16,
    BIT_VALUE_TYPE_U32,
    BIT_VALUE_TYPE_U64,
    BIT_VALUE_TYPE_U128,
    BIT_VALUE_TYPE_I8,
    BIT_VALUE_TYPE_I16,
    BIT_VALUE_TYPE_I32,
    BIT_VALUE_TYPE_I64,
    BIT_VALUE_TYPE_I128
} BitValueType;

// 128-bit integer support
typedef struct {
    uint64_t high;
    uint64_t low;
} UInt128;

typedef struct {
    int64_t high;
    uint64_t low;
} Int128;

typedef struct {
    BitValueType type;
    union {
        uint8_t u8;
        uint16_t u16;
        uint32_t u32;
        uint64_t u64;
        UInt128 u128;
        int8_t i8;
        int16_t i16;
        int32_t i32;
        int64_t i64;
        Int128 i128;
    } value;
} BitValue;

// BitStream
typedef struct {
    uint8_t* buffer;
    size_t buffer_size;
    size_t buffer_capacity;
    size_t byte_pos;
    uint8_t bit_pos;
    size_t bit_length;
} BitStream;

// BitStreamReader
typedef struct {
    FILE* file;
    uint8_t* buffer;
    size_t buffer_capacity;
    size_t buffer_size;
    size_t byte_pos;
    uint8_t bit_pos;
    bool eof;
} BitStreamReader;

// BitStreamWriter
typedef struct {
    FILE* file;
    uint8_t* buffer;
    size_t buffer_capacity;
    size_t byte_pos;
    uint8_t bit_pos;
} BitStreamWriter;

// Result type for functions that can fail
typedef struct {
    bool success;
    BitStreamError error;
    union {
        uint64_t u64;
        UInt128 u128;
        BitValue bit_value;
    } value;
} BitStreamResult;

// BitStream functions
BitStream* bit_stream_new(void);
BitStream* bit_stream_from_bytes(const uint8_t* bytes, size_t length);
void bit_stream_free(BitStream* stream);
size_t bit_stream_position(const BitStream* stream);
BitStreamResult bit_stream_set_position(BitStream* stream, size_t position);
size_t bit_stream_length(const BitStream* stream);
bool bit_stream_is_empty(const BitStream* stream);
BitStreamResult bit_stream_read_bit_value(BitStream* stream, uint8_t bit_count);
BitStreamResult bit_stream_read_bits(BitStream* stream, uint8_t bit_count);
BitStreamResult bit_stream_read_bits_u128(BitStream* stream, uint8_t bit_count);
BitStreamResult bit_stream_write_bits(BitStream* stream, uint64_t value, uint8_t bit_count);
BitStreamResult bit_stream_write_bits_u128(BitStream* stream, UInt128 value, uint8_t bit_count);
BitStreamResult bit_stream_write_bit_value(BitStream* stream, BitValue value, uint8_t bit_count);
uint8_t* bit_stream_into_bytes(BitStream* stream, size_t* length);
void bit_stream_reset(BitStream* stream);
bool bit_stream_is_eof(const BitStream* stream);

// BitStreamReader functions
BitStreamReader* bit_stream_reader_new(FILE* file);
BitStreamReader* bit_stream_reader_with_capacity(FILE* file, size_t capacity);
void bit_stream_reader_free(BitStreamReader* reader);
BitStreamResult bit_stream_reader_read_bits(BitStreamReader* reader, uint8_t bit_count);
BitStreamResult bit_stream_reader_read_bits_u128(BitStreamReader* reader, uint8_t bit_count);
BitStreamResult bit_stream_reader_read_bit_value(BitStreamReader* reader, uint8_t bit_count);
bool bit_stream_reader_is_eof(const BitStreamReader* reader);

// BitStreamWriter functions
BitStreamWriter* bit_stream_writer_new(FILE* file);
BitStreamWriter* bit_stream_writer_with_capacity(FILE* file, size_t capacity);
void bit_stream_writer_free(BitStreamWriter* writer);
BitStreamResult bit_stream_writer_write_bits(BitStreamWriter* writer, uint64_t value, uint8_t bit_count);
BitStreamResult bit_stream_writer_write_bits_u128(BitStreamWriter* writer, UInt128 value, uint8_t bit_count);
BitStreamResult bit_stream_writer_write_bit_value(BitStreamWriter* writer, BitValue value, uint8_t bit_count);
BitStreamResult bit_stream_writer_flush(BitStreamWriter* writer);

// BitValue functions
BitStreamResult bit_value_new(uint64_t value, uint8_t bit_count);
BitStreamResult bit_value_new_u128(UInt128 value, uint8_t bit_count);
BitStreamResult bit_value_new_signed(int64_t value, uint8_t bit_count);
BitStreamResult bit_value_new_i128(Int128 value, uint8_t bit_count);
uint8_t bit_value_bit_count(const BitValue* value);
uint64_t bit_value_to_u64(const BitValue* value);
UInt128 bit_value_to_u128(const BitValue* value);
int64_t bit_value_to_i64(const BitValue* value);
Int128 bit_value_to_i128(const BitValue* value);
bool bit_value_is_signed(const BitValue* value);

// UInt128/Int128 utility functions
UInt128 uint128_from_u64(uint64_t value);
UInt128 uint128_from_parts(uint64_t high, uint64_t low);
Int128 int128_from_i64(int64_t value);
Int128 int128_from_parts(int64_t high, uint64_t low);
UInt128 uint128_add(UInt128 a, UInt128 b);
UInt128 uint128_subtract(UInt128 a, UInt128 b);
UInt128 uint128_shift_left(UInt128 value, unsigned int shift);
UInt128 uint128_shift_right(UInt128 value, unsigned int shift);
UInt128 uint128_and(UInt128 a, UInt128 b);
UInt128 uint128_or(UInt128 a, UInt128 b);
UInt128 uint128_xor(UInt128 a, UInt128 b);
UInt128 uint128_not(UInt128 value);
bool uint128_equal(UInt128 a, UInt128 b);
int uint128_compare(UInt128 a, UInt128 b);

#endif /* BIT_STREAM_H */