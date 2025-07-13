#include "bit_stream.h"
#include "unity.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

// Temporary file paths for testing
#define TEST_FILE_PATH "test_file.bin"
#define TEST_FILE_PATH_2 "test_file_2.bin"

void setUp(void) {
    // This is run before each test
}

void tearDown(void) {
    // This is run after each test
    // Clean up any test files
    remove(TEST_FILE_PATH);
    remove(TEST_FILE_PATH_2);
}

void test_bit_stream_reader(void) {
    // Create test data
    uint8_t data[] = {0b10101010, 0b11110000, 0b00001111};
    
    // Write test data to a file
    FILE* file = fopen(TEST_FILE_PATH, "wb");
    TEST_ASSERT_NOT_NULL(file);
    fwrite(data, 1, sizeof(data), file);
    fclose(file);
    
    // Open the file for reading
    file = fopen(TEST_FILE_PATH, "rb");
    TEST_ASSERT_NOT_NULL(file);
    
    // Create a BitStreamReader with the file
    BitStreamReader* reader = bit_stream_reader_new(file);
    TEST_ASSERT_NOT_NULL(reader);
    
    // Read and verify bits
    BitStreamResult result;
    
    result = bit_stream_reader_read_bits(reader, 1);
    TEST_ASSERT_TRUE(result.success);
    TEST_ASSERT_EQUAL_UINT64(0b1, result.value.u64);
    
    result = bit_stream_reader_read_bits(reader, 3);
    TEST_ASSERT_TRUE(result.success);
    TEST_ASSERT_EQUAL_UINT64(0b010, result.value.u64);
    
    result = bit_stream_reader_read_bits(reader, 4);
    TEST_ASSERT_TRUE(result.success);
    TEST_ASSERT_EQUAL_UINT64(0b1010, result.value.u64);
    
    result = bit_stream_reader_read_bits(reader, 8);
    TEST_ASSERT_TRUE(result.success);
    TEST_ASSERT_EQUAL_UINT64(0b11110000, result.value.u64);
    
    result = bit_stream_reader_read_bits(reader, 8);
    TEST_ASSERT_TRUE(result.success);
    TEST_ASSERT_EQUAL_UINT64(0b00001111, result.value.u64);
    
    // Should be at end of stream now
    result = bit_stream_reader_read_bits(reader, 1);
    TEST_ASSERT_FALSE(result.success);
    TEST_ASSERT_EQUAL(BIT_STREAM_ERROR_END_OF_STREAM, result.error.code);
    TEST_ASSERT_TRUE(bit_stream_reader_is_eof(reader));
    
    // Clean up
    bit_stream_reader_free(reader);
    fclose(file);
}

void test_bit_stream_writer(void) {
    // Open a file for writing
    FILE* file = fopen(TEST_FILE_PATH, "wb");
    TEST_ASSERT_NOT_NULL(file);
    
    // Create a BitStreamWriter with the file
    BitStreamWriter* writer = bit_stream_writer_new(file);
    TEST_ASSERT_NOT_NULL(writer);
    
    // Write bits
    BitStreamResult result;
    
    result = bit_stream_writer_write_bits(writer, 0b1, 1);
    TEST_ASSERT_TRUE(result.success);
    
    result = bit_stream_writer_write_bits(writer, 0b010, 3);
    TEST_ASSERT_TRUE(result.success);
    
    result = bit_stream_writer_write_bits(writer, 0b1010, 4);
    TEST_ASSERT_TRUE(result.success);
    
    result = bit_stream_writer_write_bits(writer, 0b11110000, 8);
    TEST_ASSERT_TRUE(result.success);
    
    result = bit_stream_writer_write_bits(writer, 0b00001111, 8);
    TEST_ASSERT_TRUE(result.success);
    
    // Flush to ensure all bits are written
    result = bit_stream_writer_flush(writer);
    TEST_ASSERT_TRUE(result.success);
    
    // Clean up writer
    bit_stream_writer_free(writer);
    fclose(file);
    
    // Verify the written data
    uint8_t expected_data[] = {0b10101010, 0b11110000, 0b00001111};
    uint8_t read_data[3];
    
    file = fopen(TEST_FILE_PATH, "rb");
    TEST_ASSERT_NOT_NULL(file);
    
    size_t bytes_read = fread(read_data, 1, sizeof(read_data), file);
    TEST_ASSERT_EQUAL_size_t(sizeof(expected_data), bytes_read);
    
    TEST_ASSERT_EQUAL_MEMORY(expected_data, read_data, sizeof(expected_data));
    
    fclose(file);
}

void test_bit_stream_reader_writer_integration(void) {
    // Open a file for writing
    FILE* write_file = fopen(TEST_FILE_PATH, "wb");
    TEST_ASSERT_NOT_NULL(write_file);
    
    // Create a BitStreamWriter with the file
    BitStreamWriter* writer = bit_stream_writer_new(write_file);
    TEST_ASSERT_NOT_NULL(writer);
    
    // Write various bit patterns
    BitStreamResult result;
    
    result = bit_stream_writer_write_bits(writer, 0b101, 3);
    TEST_ASSERT_TRUE(result.success);
    
    result = bit_stream_writer_write_bits(writer, 0b11110000, 8);
    TEST_ASSERT_TRUE(result.success);
    
    result = bit_stream_writer_write_bits(writer, 0xFFFFFFFF, 32);
    TEST_ASSERT_TRUE(result.success);
    
    // Write a 128-bit value
    UInt128 value_128 = uint128_from_parts(0x12345678, 0x9ABCDEF0);
    result = bit_stream_writer_write_bits_u128(writer, value_128, 128);
    TEST_ASSERT_TRUE(result.success);
    
    // Write a BitValue
    BitStreamResult bit_value_result = bit_value_new(0xABCDEF01, 32);
    TEST_ASSERT_TRUE(bit_value_result.success);
    
    result = bit_stream_writer_write_bit_value(writer, bit_value_result.value.bit_value, 0);
    TEST_ASSERT_TRUE(result.success);
    
    // Flush and close
    result = bit_stream_writer_flush(writer);
    TEST_ASSERT_TRUE(result.success);
    
    bit_stream_writer_free(writer);
    fclose(write_file);
    
    // Now read back the data
    FILE* read_file = fopen(TEST_FILE_PATH, "rb");
    TEST_ASSERT_NOT_NULL(read_file);
    
    // Create a BitStreamReader with the file
    BitStreamReader* reader = bit_stream_reader_new(read_file);
    TEST_ASSERT_NOT_NULL(reader);
    
    // Read and verify the data
    result = bit_stream_reader_read_bits(reader, 3);
    TEST_ASSERT_TRUE(result.success);
    TEST_ASSERT_EQUAL_UINT64(0b101, result.value.u64);
    
    result = bit_stream_reader_read_bits(reader, 8);
    TEST_ASSERT_TRUE(result.success);
    TEST_ASSERT_EQUAL_UINT64(0b11110000, result.value.u64);
    
    result = bit_stream_reader_read_bits(reader, 32);
    TEST_ASSERT_TRUE(result.success);
    TEST_ASSERT_EQUAL_UINT64(0xFFFFFFFF, result.value.u64);
    
    // Read the 128-bit value
    result = bit_stream_reader_read_bits_u128(reader, 128);
    TEST_ASSERT_TRUE(result.success);
    TEST_ASSERT_EQUAL_UINT64(0x12345678, result.value.u128.high);
    TEST_ASSERT_EQUAL_UINT64(0x9ABCDEF0, result.value.u128.low);
    
    // Read the BitValue
    result = bit_stream_reader_read_bit_value(reader, 32);
    TEST_ASSERT_TRUE(result.success);
    TEST_ASSERT_EQUAL(BIT_VALUE_TYPE_U32, result.value.bit_value.type);
    TEST_ASSERT_EQUAL_UINT32(0xABCDEF01, result.value.bit_value.value.u32);
    
    // Should be at end of stream now
    TEST_ASSERT_TRUE(bit_stream_reader_is_eof(reader));
    
    // Clean up
    bit_stream_reader_free(reader);
    fclose(read_file);
}

void test_bit_stream_reader_writer_large_data(void) {
    // Test with a larger amount of data to ensure buffer handling works correctly
    
    // Open a file for writing
    FILE* write_file = fopen(TEST_FILE_PATH, "wb");
    TEST_ASSERT_NOT_NULL(write_file);
    
    // Create a BitStreamWriter with a small buffer to force multiple flushes
    BitStreamWriter* writer = bit_stream_writer_with_capacity(write_file, 16);
    TEST_ASSERT_NOT_NULL(writer);
    
    // Write a large amount of data
    const int num_values = 1000;
    uint64_t values[num_values];
    
    // Initialize with some pattern
    for (int i = 0; i < num_values; i++) {
        values[i] = (uint64_t)i * 0x0101010101010101ULL;
    }
    
    // Write all values
    for (int i = 0; i < num_values; i++) {
        BitStreamResult result = bit_stream_writer_write_bits(writer, values[i], 64);
        TEST_ASSERT_TRUE(result.success);
    }
    
    // Flush and close
    BitStreamResult result = bit_stream_writer_flush(writer);
    TEST_ASSERT_TRUE(result.success);
    
    bit_stream_writer_free(writer);
    fclose(write_file);
    
    // Now read back the data
    FILE* read_file = fopen(TEST_FILE_PATH, "rb");
    TEST_ASSERT_NOT_NULL(read_file);
    
    // Create a BitStreamReader with a small buffer to force multiple fills
    BitStreamReader* reader = bit_stream_reader_with_capacity(read_file, 16);
    TEST_ASSERT_NOT_NULL(reader);
    
    // Read and verify all values
    for (int i = 0; i < num_values; i++) {
        result = bit_stream_reader_read_bits(reader, 64);
        TEST_ASSERT_TRUE(result.success);
        TEST_ASSERT_EQUAL_UINT64(values[i], result.value.u64);
    }
    
    // Should be at end of stream now
    TEST_ASSERT_TRUE(bit_stream_reader_is_eof(reader));
    
    // Clean up
    bit_stream_reader_free(reader);
    fclose(read_file);
}

void test_bit_stream_reader_writer_non_byte_aligned(void) {
    // Test reading and writing non-byte-aligned data
    
    // Open a file for writing
    FILE* write_file = fopen(TEST_FILE_PATH, "wb");
    TEST_ASSERT_NOT_NULL(write_file);
    
    // Create a BitStreamWriter
    BitStreamWriter* writer = bit_stream_writer_new(write_file);
    TEST_ASSERT_NOT_NULL(writer);
    
    // Write non-byte-aligned data
    BitStreamResult result;
    
    result = bit_stream_writer_write_bits(writer, 0b1, 1);
    TEST_ASSERT_TRUE(result.success);
    
    result = bit_stream_writer_write_bits(writer, 0b10, 2);
    TEST_ASSERT_TRUE(result.success);
    
    result = bit_stream_writer_write_bits(writer, 0b111, 3);
    TEST_ASSERT_TRUE(result.success);
    
    result = bit_stream_writer_write_bits(writer, 0b1001, 4);
    TEST_ASSERT_TRUE(result.success);
    
    result = bit_stream_writer_write_bits(writer, 0b10110, 5);
    TEST_ASSERT_TRUE(result.success);
    
    // Flush and close
    result = bit_stream_writer_flush(writer);
    TEST_ASSERT_TRUE(result.success);
    
    bit_stream_writer_free(writer);
    fclose(write_file);
    
    // Now read back the data
    FILE* read_file = fopen(TEST_FILE_PATH, "rb");
    TEST_ASSERT_NOT_NULL(read_file);
    
    // Create a BitStreamReader
    BitStreamReader* reader = bit_stream_reader_new(read_file);
    TEST_ASSERT_NOT_NULL(reader);
    
    // Read and verify the data
    result = bit_stream_reader_read_bits(reader, 1);
    TEST_ASSERT_TRUE(result.success);
    TEST_ASSERT_EQUAL_UINT64(0b1, result.value.u64);
    
    result = bit_stream_reader_read_bits(reader, 2);
    TEST_ASSERT_TRUE(result.success);
    TEST_ASSERT_EQUAL_UINT64(0b10, result.value.u64);
    
    result = bit_stream_reader_read_bits(reader, 3);
    TEST_ASSERT_TRUE(result.success);
    TEST_ASSERT_EQUAL_UINT64(0b111, result.value.u64);
    
    result = bit_stream_reader_read_bits(reader, 4);
    TEST_ASSERT_TRUE(result.success);
    TEST_ASSERT_EQUAL_UINT64(0b1001, result.value.u64);
    
    result = bit_stream_reader_read_bits(reader, 5);
    TEST_ASSERT_TRUE(result.success);
    TEST_ASSERT_EQUAL_UINT64(0b10110, result.value.u64);
    
    // Clean up
    bit_stream_reader_free(reader);
    fclose(read_file);
}

void test_bit_stream_lsb_order_128bit(void) {
    // Open a file for writing
    FILE* file = fopen(TEST_FILE_PATH, "wb");
    TEST_ASSERT_NOT_NULL(file);
    BitStreamWriter* writer = bit_stream_writer_new(file);
    TEST_ASSERT_NOT_NULL(writer);
    UInt128 value;
    value.high = 0x0123456789ABCDEFULL;
    value.low  = 0xFEDCBA9876543210ULL;
    BitStreamResult result = bit_stream_writer_write_bits_u128(writer, value, 128);
    TEST_ASSERT_TRUE(result.success);
    result = bit_stream_writer_flush(writer);
    TEST_ASSERT_TRUE(result.success);
    bit_stream_writer_free(writer);
    fclose(file);
    // Now read back
    file = fopen(TEST_FILE_PATH, "rb");
    TEST_ASSERT_NOT_NULL(file);
    BitStreamReader* reader = bit_stream_reader_new(file);
    TEST_ASSERT_NOT_NULL(reader);
    result = bit_stream_reader_read_bits_u128(reader, 128);
    TEST_ASSERT_TRUE(result.success);
    TEST_ASSERT_EQUAL_UINT64(0x0123456789ABCDEFULL, result.value.u128.high);
    TEST_ASSERT_EQUAL_UINT64(0xFEDCBA9876543210ULL, result.value.u128.low);
    bit_stream_reader_free(reader);
    fclose(file);
}

void test_bit_stream_lsb_order_all_bit_lengths(void) {
    // Test LSB order for all bit lengths from 1 to 128
    for (uint8_t bit_count = 1; bit_count <= 64; bit_count++) {
        uint64_t test_value = (1ULL << (bit_count - 1)) | 1ULL; // Set MSB and LSB
        
        // Open a file for writing
        FILE* file = fopen(TEST_FILE_PATH, "wb");
        TEST_ASSERT_NOT_NULL(file);
        BitStreamWriter* writer = bit_stream_writer_new(file);
        TEST_ASSERT_NOT_NULL(writer);
        
        BitStreamResult result = bit_stream_writer_write_bits(writer, test_value, bit_count);
        TEST_ASSERT_TRUE(result.success);
        result = bit_stream_writer_flush(writer);
        TEST_ASSERT_TRUE(result.success);
        bit_stream_writer_free(writer);
        fclose(file);
        
        // Now read back
        file = fopen(TEST_FILE_PATH, "rb");
        TEST_ASSERT_NOT_NULL(file);
        BitStreamReader* reader = bit_stream_reader_new(file);
        TEST_ASSERT_NOT_NULL(reader);
        
        BitStreamResult read_result = bit_stream_reader_read_bits(reader, bit_count);
        TEST_ASSERT_TRUE(read_result.success);
        TEST_ASSERT_EQUAL_UINT64(test_value, read_result.value.u64);
        
        bit_stream_reader_free(reader);
        fclose(file);
    }
    
    // Test 128-bit values
    for (uint8_t bit_count = 65; bit_count <= 128; bit_count++) {
        UInt128 test_value;
        if (bit_count <= 64) {
            test_value.low = (1ULL << (bit_count - 1)) | 1ULL;
            test_value.high = 0;
        } else {
            test_value.low = 1ULL; // Set LSB
            test_value.high = (1ULL << (bit_count - 65)) | 1ULL; // Set MSB and LSB of high part
        }
        
        // Open a file for writing
        FILE* file = fopen(TEST_FILE_PATH, "wb");
        TEST_ASSERT_NOT_NULL(file);
        BitStreamWriter* writer = bit_stream_writer_new(file);
        TEST_ASSERT_NOT_NULL(writer);
        
        BitStreamResult result = bit_stream_writer_write_bits_u128(writer, test_value, bit_count);
        TEST_ASSERT_TRUE(result.success);
        result = bit_stream_writer_flush(writer);
        TEST_ASSERT_TRUE(result.success);
        bit_stream_writer_free(writer);
        fclose(file);
        
        // Now read back
        file = fopen(TEST_FILE_PATH, "rb");
        TEST_ASSERT_NOT_NULL(file);
        BitStreamReader* reader = bit_stream_reader_new(file);
        TEST_ASSERT_NOT_NULL(reader);
        
        BitStreamResult read_result = bit_stream_reader_read_bits_u128(reader, bit_count);
        TEST_ASSERT_TRUE(read_result.success);
        TEST_ASSERT_EQUAL_UINT64(test_value.low, read_result.value.u128.low);
        TEST_ASSERT_EQUAL_UINT64(test_value.high, read_result.value.u128.high);
        
        bit_stream_reader_free(reader);
        fclose(file);
    }
}

int main(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_bit_stream_reader);
    RUN_TEST(test_bit_stream_writer);
    RUN_TEST(test_bit_stream_reader_writer_integration);
    RUN_TEST(test_bit_stream_reader_writer_large_data);
    RUN_TEST(test_bit_stream_reader_writer_non_byte_aligned);
    RUN_TEST(test_bit_stream_lsb_order_128bit);
    RUN_TEST(test_bit_stream_lsb_order_all_bit_lengths);
    
    return UNITY_END();
}