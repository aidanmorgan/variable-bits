#include "bit_stream.h"
#include "unity.h"
#include <stdint.h>
#include <stdbool.h>

void setUp(void) {
    // This is run before each test
}

void tearDown(void) {
    // This is run after each test
}

void test_bit_value_new_valid(void) {
    // Test creating BitValue with valid bit counts
    BitStreamResult result;
    
    // Test 8-bit value
    result = bit_value_new(0x5A, 8);
    TEST_ASSERT_TRUE(result.success);
    TEST_ASSERT_EQUAL(BIT_VALUE_TYPE_U8, result.value.bit_value.type);
    TEST_ASSERT_EQUAL_UINT8(0x5A, result.value.bit_value.value.u8);
    
    // Test 16-bit value
    result = bit_value_new(0x5AA5, 16);
    TEST_ASSERT_TRUE(result.success);
    TEST_ASSERT_EQUAL(BIT_VALUE_TYPE_U16, result.value.bit_value.type);
    TEST_ASSERT_EQUAL_UINT16(0x5AA5, result.value.bit_value.value.u16);
    
    // Test 32-bit value
    result = bit_value_new(0x5AA55AA5, 32);
    TEST_ASSERT_TRUE(result.success);
    TEST_ASSERT_EQUAL(BIT_VALUE_TYPE_U32, result.value.bit_value.type);
    TEST_ASSERT_EQUAL_UINT32(0x5AA55AA5, result.value.bit_value.value.u32);
    
    // Test 64-bit value
    result = bit_value_new(0x5AA55AA55AA55AA5, 64);
    TEST_ASSERT_TRUE(result.success);
    TEST_ASSERT_EQUAL(BIT_VALUE_TYPE_U64, result.value.bit_value.type);
    TEST_ASSERT_EQUAL_UINT64(0x5AA55AA55AA55AA5, result.value.bit_value.value.u64);
}

void test_bit_value_new_invalid(void) {
    // Test creating BitValue with invalid bit counts
    BitStreamResult result;
    
    // Test 0 bits
    result = bit_value_new(0, 0);
    TEST_ASSERT_FALSE(result.success);
    TEST_ASSERT_EQUAL(BIT_STREAM_ERROR_INVALID_BIT_COUNT, result.error.code);
    
    // Test too many bits
    result = bit_value_new(0, 129);
    TEST_ASSERT_FALSE(result.success);
    TEST_ASSERT_EQUAL(BIT_STREAM_ERROR_INVALID_BIT_COUNT, result.error.code);
}

void test_bit_value_new_u128(void) {
    // Test creating 128-bit BitValue
    BitStreamResult result;
    UInt128 value = uint128_from_parts(0x5AA55AA5, 0x5AA55AA5);
    
    result = bit_value_new_u128(value, 128);
    TEST_ASSERT_TRUE(result.success);
    TEST_ASSERT_EQUAL(BIT_VALUE_TYPE_U128, result.value.bit_value.type);
    TEST_ASSERT_EQUAL_UINT64(0x5AA55AA5, result.value.bit_value.value.u128.high);
    TEST_ASSERT_EQUAL_UINT64(0x5AA55AA5, result.value.bit_value.value.u128.low);
}

void test_bit_value_new_signed(void) {
    // Test creating signed BitValue
    BitStreamResult result;
    
    // Test 8-bit signed value
    result = bit_value_new_signed(-42, 8);
    TEST_ASSERT_TRUE(result.success);
    TEST_ASSERT_EQUAL(BIT_VALUE_TYPE_I8, result.value.bit_value.type);
    TEST_ASSERT_EQUAL_INT8(-42, result.value.bit_value.value.i8);
    
    // Test 16-bit signed value
    result = bit_value_new_signed(-12345, 16);
    TEST_ASSERT_TRUE(result.success);
    TEST_ASSERT_EQUAL(BIT_VALUE_TYPE_I16, result.value.bit_value.type);
    TEST_ASSERT_EQUAL_INT16(-12345, result.value.bit_value.value.i16);
    
    // Test 32-bit signed value
    result = bit_value_new_signed(-1234567890, 32);
    TEST_ASSERT_TRUE(result.success);
    TEST_ASSERT_EQUAL(BIT_VALUE_TYPE_I32, result.value.bit_value.type);
    TEST_ASSERT_EQUAL_INT32(-1234567890, result.value.bit_value.value.i32);
    
    // Test 64-bit signed value
    result = bit_value_new_signed(-1234567890123456789, 64);
    TEST_ASSERT_TRUE(result.success);
    TEST_ASSERT_EQUAL(BIT_VALUE_TYPE_I64, result.value.bit_value.type);
    TEST_ASSERT_EQUAL_INT64(-1234567890123456789, result.value.bit_value.value.i64);
}

void test_bit_value_new_i128(void) {
    // Test creating 128-bit signed BitValue
    BitStreamResult result;
    Int128 value = int128_from_parts(-1, 0x5AA55AA5);
    
    result = bit_value_new_i128(value, 128);
    TEST_ASSERT_TRUE(result.success);
    TEST_ASSERT_EQUAL(BIT_VALUE_TYPE_I128, result.value.bit_value.type);
    TEST_ASSERT_EQUAL_INT64(-1, result.value.bit_value.value.i128.high);
    TEST_ASSERT_EQUAL_UINT64(0x5AA55AA5, result.value.bit_value.value.i128.low);
}

void test_bit_value_bit_count(void) {
    // Test getting bit count from BitValue
    BitStreamResult result;
    
    // Test 8-bit value
    result = bit_value_new(0x5A, 8);
    TEST_ASSERT_EQUAL(8, bit_value_bit_count(&result.value.bit_value));
    
    // Test 16-bit value
    result = bit_value_new(0x5AA5, 16);
    TEST_ASSERT_EQUAL(16, bit_value_bit_count(&result.value.bit_value));
    
    // Test 32-bit value
    result = bit_value_new(0x5AA55AA5, 32);
    TEST_ASSERT_EQUAL(32, bit_value_bit_count(&result.value.bit_value));
    
    // Test 64-bit value
    result = bit_value_new(0x5AA55AA55AA55AA5, 64);
    TEST_ASSERT_EQUAL(64, bit_value_bit_count(&result.value.bit_value));
    
    // Test 128-bit value
    UInt128 value = uint128_from_parts(0x5AA55AA5, 0x5AA55AA5);
    result = bit_value_new_u128(value, 128);
    TEST_ASSERT_EQUAL(128, bit_value_bit_count(&result.value.bit_value));
}

void test_bit_value_to_u64(void) {
    // Test converting BitValue to u64
    BitStreamResult result;
    
    // Test 8-bit value
    result = bit_value_new(0x5A, 8);
    TEST_ASSERT_EQUAL_UINT64(0x5A, bit_value_to_u64(&result.value.bit_value));
    
    // Test 16-bit value
    result = bit_value_new(0x5AA5, 16);
    TEST_ASSERT_EQUAL_UINT64(0x5AA5, bit_value_to_u64(&result.value.bit_value));
    
    // Test 32-bit value
    result = bit_value_new(0x5AA55AA5, 32);
    TEST_ASSERT_EQUAL_UINT64(0x5AA55AA5, bit_value_to_u64(&result.value.bit_value));
    
    // Test 64-bit value
    result = bit_value_new(0x5AA55AA55AA55AA5, 64);
    TEST_ASSERT_EQUAL_UINT64(0x5AA55AA55AA55AA5, bit_value_to_u64(&result.value.bit_value));
}

void test_bit_value_to_u128(void) {
    // Test converting BitValue to u128
    BitStreamResult result;
    UInt128 value, expected;
    
    // Test 64-bit value
    result = bit_value_new(0x5AA55AA55AA55AA5, 64);
    value = bit_value_to_u128(&result.value.bit_value);
    expected = uint128_from_u64(0x5AA55AA55AA55AA5);
    TEST_ASSERT_TRUE(uint128_equal(expected, value));
    
    // Test 128-bit value
    expected = uint128_from_parts(0x5AA55AA5, 0x5AA55AA5);
    result = bit_value_new_u128(expected, 128);
    value = bit_value_to_u128(&result.value.bit_value);
    TEST_ASSERT_TRUE(uint128_equal(expected, value));
}

void test_bit_value_to_i64(void) {
    // Test converting BitValue to i64
    BitStreamResult result;
    
    // Test 8-bit signed value
    result = bit_value_new_signed(-42, 8);
    TEST_ASSERT_EQUAL_INT64(-42, bit_value_to_i64(&result.value.bit_value));
    
    // Test 16-bit signed value
    result = bit_value_new_signed(-12345, 16);
    TEST_ASSERT_EQUAL_INT64(-12345, bit_value_to_i64(&result.value.bit_value));
    
    // Test 32-bit signed value
    result = bit_value_new_signed(-1234567890, 32);
    TEST_ASSERT_EQUAL_INT64(-1234567890, bit_value_to_i64(&result.value.bit_value));
    
    // Test 64-bit signed value
    result = bit_value_new_signed(-1234567890123456789, 64);
    TEST_ASSERT_EQUAL_INT64(-1234567890123456789, bit_value_to_i64(&result.value.bit_value));
}

void test_bit_value_is_signed(void) {
    // Test checking if BitValue is signed
    BitStreamResult result;
    
    // Test unsigned value
    result = bit_value_new(0x5A, 8);
    TEST_ASSERT_FALSE(bit_value_is_signed(&result.value.bit_value));
    
    // Test signed value
    result = bit_value_new_signed(-42, 8);
    TEST_ASSERT_TRUE(bit_value_is_signed(&result.value.bit_value));
}

int main(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_bit_value_new_valid);
    RUN_TEST(test_bit_value_new_invalid);
    RUN_TEST(test_bit_value_new_u128);
    RUN_TEST(test_bit_value_new_signed);
    RUN_TEST(test_bit_value_new_i128);
    RUN_TEST(test_bit_value_bit_count);
    RUN_TEST(test_bit_value_to_u64);
    RUN_TEST(test_bit_value_to_u128);
    RUN_TEST(test_bit_value_to_i64);
    RUN_TEST(test_bit_value_is_signed);
    
    return UNITY_END();
}