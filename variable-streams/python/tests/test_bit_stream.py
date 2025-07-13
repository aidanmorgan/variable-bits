"""Tests for the BitStream class."""

import pytest
from variable_streams import BitStream, BitValue
from variable_streams.bit_value import InvalidBitCountError, EndOfStreamError


def test_bit_stream_creation():
    """Test creating BitStream instances."""
    # Test empty stream
    bs = BitStream()
    assert bs.length() == 0
    assert bs.is_empty()
    assert bs.position() == 0

    # Test stream from bytes
    data = bytes([0xA5, 0x5A])  # 10100101 01011010
    bs = BitStream(data)
    assert bs.length() == 16  # 2 bytes = 16 bits
    assert not bs.is_empty()
    assert bs.position() == 0


def test_bit_stream_position():
    """Test position management in BitStream."""
    data = bytes([0xA5, 0x5A])  # 10100101 01011010
    bs = BitStream(data)

    # Test initial position
    assert bs.position() == 0

    # Test setting position
    bs.set_position(8)
    assert bs.position() == 8

    # Test setting position to end
    bs.set_position(16)
    assert bs.position() == 16
    assert bs.is_eof()

    # Test setting position beyond end
    with pytest.raises(EndOfStreamError):
        bs.set_position(17)

    # Test reset
    bs.reset()
    assert bs.position() == 0


def test_bit_stream_read_bits():
    """Test reading bits from BitStream."""
    # Create a stream with known bit pattern
    # 10100101 01011010
    data = bytes([0xA5, 0x5A])
    bs = BitStream(data)

    # Read 1 bit (should be 1)
    assert bs.read_bits(1) == 1
    assert bs.position() == 1

    # Read 3 bits (should be 010 = 2)
    assert bs.read_bits(3) == 2
    assert bs.position() == 4

    # Read 4 bits (should be 0101 = 5)
    assert bs.read_bits(4) == 5
    assert bs.position() == 8

    # Read 8 bits (should be 01011010 = 0x5A = 90)
    assert bs.read_bits(8) == 0x5A
    assert bs.position() == 16

    # Try to read beyond end
    with pytest.raises(EndOfStreamError):
        bs.read_bits(1)


def test_bit_stream_write_bits():
    """Test writing bits to BitStream."""
    bs = BitStream()

    # Write 3 bits (value 5 = 101)
    bs.write_bits(5, 3)
    assert bs.length() == 3
    assert bs.position() == 3

    # Write 5 more bits (value 10 = 01010)
    bs.write_bits(10, 5)
    assert bs.length() == 8
    assert bs.position() == 8

    # Reset and read to verify
    bs.reset()
    # Should read 10101010 = 0xAA = 170
    assert bs.read_bits(8) == 0xAA

    # Test writing at a specific position
    bs.reset()
    bs.set_position(3)
    bs.write_bits(3, 3)  # 011
    assert bs.length() == 8  # Length shouldn't change

    # Verify the write
    bs.reset()
    # Should now read 10101110 = 0xAE = 174
    assert bs.read_bits(8) == 0xAE


def test_bit_stream_read_write_u128():
    """Test reading and writing large values (up to 128 bits)."""
    bs = BitStream()

    # Write a 96-bit value
    value = (1 << 96) - 1  # All 1s in 96 bits
    bs.write_bits_u128(value, 96)
    assert bs.length() == 96

    # Reset and read it back
    bs.reset()
    read_value = bs.read_bits_u128(96)
    assert read_value == value

    # Test with a value that uses all 128 bits
    bs = BitStream()
    value = (1 << 128) - 1  # All 1s in 128 bits
    bs.write_bits_u128(value, 128)

    bs.reset()
    read_value = bs.read_bits_u128(128)
    assert read_value == value


def test_bit_stream_bit_value():
    """Test reading and writing BitValue objects."""
    bs = BitStream()

    # Create and write a BitValue
    bv1 = BitValue(42, 8)
    bs.write_bit_value(bv1)

    # Create and write another BitValue
    bv2 = BitValue(-42, 8)
    bs.write_bit_value(bv2)

    # Reset and read back
    bs.reset()
    read_bv1 = bs.read_bit_value(8)
    assert int(read_bv1) == 42
    assert read_bv1.bit_count == 8

    read_bv2 = bs.read_bit_value(8, is_signed=True)
    assert int(read_bv2) == -42
    assert read_bv2.bit_count == 8

    # Test with a large BitValue
    bs = BitStream()
    large_value = (1 << 96) - 1
    bv3 = BitValue(large_value, 96)
    bs.write_bit_value(bv3)

    bs.reset()
    read_bv3 = bs.read_bit_value(96)
    assert int(read_bv3) == large_value
    assert read_bv3.bit_count == 96


def test_bit_stream_to_bytes():
    """Test converting BitStream to bytes."""
    # Create a stream with some data
    bs = BitStream()
    bs.write_bits(0xA5, 8)  # 10100101
    bs.write_bits(0x5A, 8)  # 01011010

    # Convert to bytes
    data = bs.to_bytes()
    assert len(data) == 2
    assert data[0] == 0xA5
    assert data[1] == 0x5A

    # Test with non-byte-aligned bit count
    bs = BitStream()
    bs.write_bits(0x5, 3)  # 101
    bs.write_bits(0x3, 2)  # 11
    # Should be 10111000 = 0xB8 with 3 unused bits

    data = bs.to_bytes()
    assert len(data) == 1
    assert data[0] == 0xB8


def test_bit_stream_error_handling():
    """Test error handling in BitStream."""
    bs = BitStream()

    # Test invalid bit counts
    with pytest.raises(InvalidBitCountError):
        bs.read_bits(0)

    with pytest.raises(InvalidBitCountError):
        bs.read_bits(65)

    with pytest.raises(InvalidBitCountError):
        bs.write_bits(0, 0)

    with pytest.raises(InvalidBitCountError):
        bs.write_bits(0, 65)

    # Test reading from empty stream
    with pytest.raises(EndOfStreamError):
        bs.read_bits(1)

    # Test reading beyond end
    bs.write_bits(0xFF, 8)
    bs.reset()
    bs.read_bits(8)
    with pytest.raises(EndOfStreamError):
        bs.read_bits(1)
