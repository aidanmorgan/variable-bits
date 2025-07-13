"""Tests for the BitValue class."""

import pytest
from variable_streams import BitValue
from variable_streams.bit_value import InvalidBitCountError


def test_bit_value_creation():
    """Test creating BitValue instances with different values and bit counts."""
    # Test unsigned values
    bv1 = BitValue(42, 8)
    assert int(bv1) == 42
    assert bv1.bit_count == 8
    assert not bv1.is_signed

    # Test signed values
    bv2 = BitValue(-42, 8)
    assert int(bv2) == -42
    assert bv2.bit_count == 8
    assert bv2.is_signed

    # Test auto bit count calculation for unsigned
    bv3 = BitValue(42)
    assert int(bv3) == 42
    assert bv3.bit_count == 6  # 42 needs 6 bits (101010)
    assert not bv3.is_signed

    # Test auto bit count calculation for signed
    bv4 = BitValue(-42)
    assert int(bv4) == -42
    assert bv4.bit_count == 7  # -42 needs 7 bits (1 sign bit + 6 bits)
    assert bv4.is_signed

    # Test copy constructor
    bv5 = BitValue(bv1)
    assert int(bv5) == 42
    assert bv5.bit_count == 8
    assert not bv5.is_signed


def test_bit_value_validation():
    """Test validation of bit counts and value ranges."""
    # Test invalid bit count
    with pytest.raises(InvalidBitCountError):
        BitValue(42, 0)

    with pytest.raises(InvalidBitCountError):
        BitValue(42, 129)

    # Test value out of range for unsigned
    with pytest.raises(ValueError):
        BitValue(256, 8)  # 8 bits can only represent 0-255

    # Test value out of range for unsigned
    with pytest.raises(ValueError):
        BitValue(128, 7)  # 7 bits can only represent 0-127

    with pytest.raises(ValueError):
        BitValue(-129, 8)  # 8 bits signed can only represent -128 to 127


def test_bit_value_equality():
    """Test equality comparison of BitValue instances."""
    bv1 = BitValue(42, 8)
    bv2 = BitValue(42, 8)
    bv3 = BitValue(42, 16)
    bv4 = BitValue(43, 8)
    bv5 = BitValue(-42, 8)

    # Same value and bit count
    assert bv1 == bv2

    # Same value but different bit count
    assert bv1 != bv3

    # Different value but same bit count
    assert bv1 != bv4

    # Different sign
    assert bv1 != bv5

    # Compare with integer
    assert bv1 == 42
    assert bv1 != 43


def test_bit_value_large_values():
    """Test BitValue with large values (near 64-bit and 128-bit limits)."""
    # Test 64-bit values
    max_u64 = (1 << 64) - 1
    bv1 = BitValue(max_u64, 64)
    assert int(bv1) == max_u64
    assert bv1.bit_count == 64

    # Test values near 64-bit signed limit
    max_i64 = (1 << 63) - 1
    bv2 = BitValue(max_i64, 64)
    assert int(bv2) == max_i64
    assert bv2.bit_count == 64

    min_i64 = -(1 << 63)
    bv3 = BitValue(min_i64, 64)
    assert int(bv3) == min_i64
    assert bv3.bit_count == 64

    # Test large values with auto bit count
    large_value = (1 << 32) - 1
    bv4 = BitValue(large_value)
    assert int(bv4) == large_value
    assert bv4.bit_count == 32


def test_bit_value_representation():
    """Test string representation of BitValue."""
    bv1 = BitValue(42, 8)
    repr_str = repr(bv1)
    assert "BitValue" in repr_str
    assert "42" in repr_str
    assert "8 bits" in repr_str
    assert "unsigned" in repr_str

    bv2 = BitValue(-42, 8)
    repr_str = repr(bv2)
    assert "BitValue" in repr_str
    assert "-42" in repr_str
    assert "8 bits" in repr_str
    assert "signed" in repr_str
