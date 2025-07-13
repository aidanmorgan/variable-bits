#include "bit_stream.h"
#include "unity.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

void setUp(void) {
    // This is run before each test
}

void tearDown(void) {
    // This is run after each test
}

void test_bit_stream_new(void) {
    // Test creating a new empty BitStream
    BitStream* stream = bit_stream_new();
    
    TEST_ASSERT_NOT_NULL(stream);
    TEST_ASSERT_NULL(stream->buffer);
    TEST_ASSERT_EQUAL_size_t(0, stream->buffer_size);
    TEST_ASSERT_EQUAL_size_t(0, stream->buffer_capacity);
    TEST_ASSERT_EQUAL_size_t(0, stream->byte_pos);
    TEST_ASSERT_EQUAL_UINT8(0, stream->bit_pos);
    TEST_ASSERT_EQUAL_size_t(0, stream->bit_length);
    
    bit_stream_free(stream);
}

void test_bit_stream_from_bytes(void) {
    // Test creating a BitStream from existing bytes
    uint8_t bytes[] = {0x5A, 0xA5, 0x3C, 0xC3};
    BitStream* stream = bit_stream_from_bytes(bytes, sizeof(bytes));
    
    TEST_ASSERT_NOT_NULL(stream);
    TEST_ASSERT_NOT_NULL(stream->buffer);
    TEST_ASSERT_EQUAL_size_t(sizeof(bytes), stream->buffer_size);
    TEST_ASSERT_EQUAL_size_t(sizeof(bytes), stream->buffer_capacity);
    TEST_ASSERT_EQUAL_size_t(0, stream->byte_pos);
    TEST_ASSERT_EQUAL_UINT8(0, stream->bit_pos);
    TEST_ASSERT_EQUAL_size_t(sizeof(bytes) * 8, stream->bit_length);
    
    // Check buffer contents
    TEST_ASSERT_EQUAL_MEMORY(bytes, stream->buffer, sizeof(bytes));
    
    bit_stream_free(stream);
}

void test_bit_stream_position(void) {
    // Test getting and setting position in a BitStream
    uint8_t bytes[] = {0x5A, 0xA5, 0x3C, 0xC3};
    BitStream* stream = bit_stream_from_bytes(bytes, sizeof(bytes));
    
    // Initial position should be 0
    TEST_ASSERT_EQUAL_size_t(0, bit_stream_position(stream));
    
    // Set position to middle of second byte
    BitStreamResult result = bit_stream_set_position(stream, 12);
    TEST_ASSERT_TRUE(result.success);
    TEST_ASSERT_EQUAL_size_t(1, stream->byte_pos);
    TEST_ASSERT_EQUAL_UINT8(4, stream->bit_pos);
    TEST_ASSERT_EQUAL_size_t(12, bit_stream_position(stream));
    
    // Try to set position beyond end of stream
    result = bit_stream_set_position(stream, 40);
    TEST_ASSERT_FALSE(result.success);
    TEST_ASSERT_EQUAL(BIT_STREAM_ERROR_END_OF_STREAM, result.error.code);
    
    bit_stream_free(stream);
}

void test_bit_stream_length_and_empty(void) {
    // Test length and empty checks
    
    // Empty stream
    BitStream* empty_stream = bit_stream_new();
    TEST_ASSERT_EQUAL_size_t(0, bit_stream_length(empty_stream));
    TEST_ASSERT_TRUE(bit_stream_is_empty(empty_stream));
    bit_stream_free(empty_stream);
    
    // Non-empty stream
    uint8_t bytes[] = {0x5A, 0xA5, 0x3C, 0xC3};
    BitStream* stream = bit_stream_from_bytes(bytes, sizeof(bytes));
    TEST_ASSERT_EQUAL_size_t(sizeof(bytes) * 8, bit_stream_length(stream));
    TEST_ASSERT_FALSE(bit_stream_is_empty(stream));
    bit_stream_free(stream);
}

void test_bit_stream_read_write_bits(void) {
    // Test reading and writing bits
    BitStream* stream = bit_stream_new();
    BitStreamResult result;
    
    // Write different bit lengths
    result = bit_stream_write_bits(stream, 0b101, 3);
    TEST_ASSERT_TRUE(result.success);
    
    result = bit_stream_write_bits(stream, 0b11110000, 8);
    TEST_ASSERT_TRUE(result.success);
    
    result = bit_stream_write_bits(stream, 0xFFFFFFFFFFFFFFFF, 64);
    TEST_ASSERT_TRUE(result.success);
    
    // Reset position to beginning
    bit_stream_reset(stream);
    
    // Read and verify
    result = bit_stream_read_bits(stream, 3);
    TEST_ASSERT_TRUE(result.success);
    TEST_ASSERT_EQUAL_UINT64(0b101, result.value.u64);
    
    result = bit_stream_read_bits(stream, 8);
    TEST_ASSERT_TRUE(result.success);
    TEST_ASSERT_EQUAL_UINT64(0b11110000, result.value.u64);
    
    result = bit_stream_read_bits(stream, 64);
    TEST_ASSERT_TRUE(result.success);
    TEST_ASSERT_EQUAL_UINT64(0xFFFFFFFFFFFFFFFF, result.value.u64);
    
    bit_stream_free(stream);
}

void test_bit_stream_read_write_bits_u128(void) {
    // Test reading and writing 128-bit values
    BitStream* stream = bit_stream_new();
    BitStreamResult result;
    
    // Write 128-bit value
    UInt128 write_value = uint128_from_parts(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF);
    result = bit_stream_write_bits_u128(stream, write_value, 128);
    TEST_ASSERT_TRUE(result.success);
    
    // Reset position to beginning
    bit_stream_reset(stream);
    
    // Read and verify
    result = bit_stream_read_bits_u128(stream, 128);
    TEST_ASSERT_TRUE(result.success);
    TEST_ASSERT_TRUE(uint128_equal(write_value, result.value.u128));
    
    bit_stream_free(stream);
}

void test_bit_stream_read_write_bit_value(void) {
    // Test reading and writing BitValue
    BitStream* stream = bit_stream_new();
    BitStreamResult result;
    
    // Create a BitValue
    result = bit_value_new(0x5AA55AA5, 32);
    TEST_ASSERT_TRUE(result.success);
    BitValue write_value = result.value.bit_value;
    
    // Write the BitValue
    result = bit_stream_write_bit_value(stream, write_value, 0);
    TEST_ASSERT_TRUE(result.success);
    
    // Reset position to beginning
    bit_stream_reset(stream);
    
    // Read and verify
    result = bit_stream_read_bit_value(stream, 32);
    TEST_ASSERT_TRUE(result.success);
    TEST_ASSERT_EQUAL(BIT_VALUE_TYPE_U32, result.value.bit_value.type);
    TEST_ASSERT_EQUAL_UINT32(0x5AA55AA5, result.value.bit_value.value.u32);
    
    bit_stream_free(stream);
}

void test_bit_stream_non_byte_aligned_operations(void) {
    // Test operations that don't align with byte boundaries
    BitStream* stream = bit_stream_new();
    BitStreamResult result;
    
    // Write bits that don't align with byte boundaries
    result = bit_stream_write_bits(stream, 0b1, 1);
    TEST_ASSERT_TRUE(result.success);
    
    result = bit_stream_write_bits(stream, 0b10, 2);
    TEST_ASSERT_TRUE(result.success);
    
    result = bit_stream_write_bits(stream, 0b111, 3);
    TEST_ASSERT_TRUE(result.success);
    
    // Reset position to beginning
    bit_stream_reset(stream);
    
    // Read and verify
    result = bit_stream_read_bits(stream, 1);
    TEST_ASSERT_TRUE(result.success);
    TEST_ASSERT_EQUAL_UINT64(0b1, result.value.u64);
    
    result = bit_stream_read_bits(stream, 2);
    TEST_ASSERT_TRUE(result.success);
    TEST_ASSERT_EQUAL_UINT64(0b10, result.value.u64);
    
    result = bit_stream_read_bits(stream, 3);
    TEST_ASSERT_TRUE(result.success);
    TEST_ASSERT_EQUAL_UINT64(0b111, result.value.u64);
    
    bit_stream_free(stream);
}

void test_bit_stream_error_handling(void) {
    // Test error handling
    BitStream* stream = bit_stream_new();
    BitStreamResult result;
    
    // Test invalid bit count
    result = bit_stream_read_bits(stream, 0);
    TEST_ASSERT_FALSE(result.success);
    TEST_ASSERT_EQUAL(BIT_STREAM_ERROR_INVALID_BIT_COUNT, result.error.code);
    
    result = bit_stream_read_bits(stream, 65);
    TEST_ASSERT_FALSE(result.success);
    TEST_ASSERT_EQUAL(BIT_STREAM_ERROR_INVALID_BIT_COUNT, result.error.code);
    
    result = bit_stream_write_bits(stream, 0, 0);
    TEST_ASSERT_FALSE(result.success);
    TEST_ASSERT_EQUAL(BIT_STREAM_ERROR_INVALID_BIT_COUNT, result.error.code);
    
    result = bit_stream_write_bits(stream, 0, 65);
    TEST_ASSERT_FALSE(result.success);
    TEST_ASSERT_EQUAL(BIT_STREAM_ERROR_INVALID_BIT_COUNT, result.error.code);
    
    // Test end of stream
    result = bit_stream_read_bits(stream, 1);
    TEST_ASSERT_FALSE(result.success);
    TEST_ASSERT_EQUAL(BIT_STREAM_ERROR_END_OF_STREAM, result.error.code);
    
    bit_stream_free(stream);
}

void test_bit_stream_into_bytes(void) {
    // Test converting BitStream to bytes
    BitStream* stream = bit_stream_new();
    
    // Write some data
    bit_stream_write_bits(stream, 0x5AA55AA5, 32);
    
    // Get bytes
    size_t length;
    uint8_t* bytes = bit_stream_into_bytes(stream, &length);
    
    // Check bytes
    TEST_ASSERT_NOT_NULL(bytes);
    TEST_ASSERT_EQUAL_size_t(4, length);
    TEST_ASSERT_EQUAL_UINT8(0x5A, bytes[0]);
    TEST_ASSERT_EQUAL_UINT8(0xA5, bytes[1]);
    TEST_ASSERT_EQUAL_UINT8(0x5A, bytes[2]);
    TEST_ASSERT_EQUAL_UINT8(0xA5, bytes[3]);
    
    // Stream should be empty now
    TEST_ASSERT_NULL(stream->buffer);
    TEST_ASSERT_EQUAL_size_t(0, stream->buffer_size);
    TEST_ASSERT_EQUAL_size_t(0, stream->buffer_capacity);
    TEST_ASSERT_EQUAL_size_t(0, stream->byte_pos);
    TEST_ASSERT_EQUAL_UINT8(0, stream->bit_pos);
    TEST_ASSERT_EQUAL_size_t(0, stream->bit_length);
    
    free(bytes);
    bit_stream_free(stream);
}

void test_bit_stream_reset_and_eof(void) {
    // Test reset and EOF detection
    uint8_t bytes[] = {0x5A, 0xA5};
    BitStream* stream = bit_stream_from_bytes(bytes, sizeof(bytes));
    
    // Initially not at EOF
    TEST_ASSERT_FALSE(bit_stream_is_eof(stream));
    
    // Read all bits
    bit_stream_read_bits(stream, 16);
    
    // Should be at EOF now
    TEST_ASSERT_TRUE(bit_stream_is_eof(stream));
    
    // Reset
    bit_stream_reset(stream);
    
    // Should not be at EOF after reset
    TEST_ASSERT_FALSE(bit_stream_is_eof(stream));
    
    bit_stream_free(stream);
}

int main(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_bit_stream_new);
    RUN_TEST(test_bit_stream_from_bytes);
    RUN_TEST(test_bit_stream_position);
    RUN_TEST(test_bit_stream_length_and_empty);
    RUN_TEST(test_bit_stream_read_write_bits);
    RUN_TEST(test_bit_stream_read_write_bits_u128);
    RUN_TEST(test_bit_stream_read_write_bit_value);
    RUN_TEST(test_bit_stream_non_byte_aligned_operations);
    RUN_TEST(test_bit_stream_error_handling);
    RUN_TEST(test_bit_stream_into_bytes);
    RUN_TEST(test_bit_stream_reset_and_eof);
    
    return UNITY_END();
}