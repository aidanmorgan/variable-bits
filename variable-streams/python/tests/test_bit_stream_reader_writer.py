"""Tests for the BitStreamReader and BitStreamWriter classes."""

import io
import pytest
from variable_streams import BitStreamReader, BitStreamWriter, BitValue
from variable_streams.bit_value import InvalidBitCountError, EndOfStreamError


def test_bit_stream_reader_basic():
    """Test basic functionality of BitStreamReader."""
    # Create a binary stream with known data
    data = bytes([0xA5, 0x5A])  # 10100101 01011010
    stream = io.BytesIO(data)

    # Create a reader
    reader = BitStreamReader(stream)

    # Read 1 bit (should be 1)
    assert reader.read_bits(1) == 1

    # Read 3 bits (should be 010 = 2)
    assert reader.read_bits(3) == 2

    # Read 4 bits (should be 0101 = 5)
    assert reader.read_bits(4) == 5

    # Read 8 bits (should be 01011010 = 0x5A = 90)
    assert reader.read_bits(8) == 0x5A

    # Try to read beyond end
    with pytest.raises(EndOfStreamError):
        reader.read_bits(1)

    # Check EOF
    assert reader.is_eof()


def test_bit_stream_writer_basic():
    """Test basic functionality of BitStreamWriter."""
    # Create a binary stream
    stream = io.BytesIO()

    # Create a writer
    writer = BitStreamWriter(stream)

    # Write 8 bits (0xA5 = 10100101)
    writer.write_bits(0xA5, 8)

    # Write 8 more bits (0x5A = 01011010)
    writer.write_bits(0x5A, 8)

    # Flush to ensure all bits are written
    writer.flush()

    # Check the stream content
    stream.seek(0)
    data = stream.read()
    assert len(data) == 2
    assert data[0] == 0xA5
    assert data[1] == 0x5A


def test_bit_stream_reader_writer_integration():
    """Test integration between BitStreamReader and BitStreamWriter."""
    # Create a binary stream
    stream = io.BytesIO()

    # Create a writer
    writer = BitStreamWriter(stream)

    # Write various bit patterns
    writer.write_bits(0x1, 1)  # 1
    writer.write_bits(0x3, 2)  # 11
    writer.write_bits(0x7, 3)  # 111
    writer.write_bits(0xF, 4)  # 1111
    writer.write_bits(0x1F, 5)  # 11111
    writer.write_bits(0x3F, 6)  # 111111
    writer.write_bits(0x7F, 7)  # 1111111
    writer.write_bits(0xFF, 8)  # 11111111

    # Flush to ensure all bits are written
    writer.flush()

    # Reset stream position
    stream.seek(0)

    # Create a reader
    reader = BitStreamReader(stream)

    # Read back the patterns
    assert reader.read_bits(1) == 0x1
    assert reader.read_bits(2) == 0x3
    assert reader.read_bits(3) == 0x7
    assert reader.read_bits(4) == 0xF
    assert reader.read_bits(5) == 0x1F
    assert reader.read_bits(6) == 0x3F
    assert reader.read_bits(7) == 0x7F
    assert reader.read_bits(8) == 0xFF

    # Check that we've read all the expected data
    # The stream might have padding bits, so we'll check the actual content
    stream.seek(0)
    data = stream.read()
    assert len(data) == 5  # We wrote 36 bits, which is 4.5 bytes, rounded up to 5

    # Verify we can't read any more meaningful bits
    try:
        value = reader.read_bits(1)
        # If we get here, there are more bits, but they should be padding
        assert value == 0
    except EndOfStreamError:
        # This is also acceptable
        pass


def test_bit_stream_reader_writer_large_values():
    """Test reading and writing large values with BitStreamReader and BitStreamWriter."""
    # Create a binary stream
    stream = io.BytesIO()

    # Create a writer
    writer = BitStreamWriter(stream)

    # Write a 96-bit value
    value = (1 << 96) - 1  # All 1s in 96 bits
    writer.write_bits_u128(value, 96)

    # Flush to ensure all bits are written
    writer.flush()

    # Reset stream position
    stream.seek(0)

    # Create a reader
    reader = BitStreamReader(stream)

    # Read back the value
    read_value = reader.read_bits_u128(96)
    assert read_value == value


def test_bit_stream_reader_writer_bit_value():
    """Test reading and writing BitValue objects with BitStreamReader and BitStreamWriter."""
    # Create a binary stream
    stream = io.BytesIO()

    # Create a writer
    writer = BitStreamWriter(stream)

    # Create and write BitValue objects
    bv1 = BitValue(42, 8)
    writer.write_bit_value(bv1)

    bv2 = BitValue(-42, 8)
    writer.write_bit_value(bv2)

    # Write a large BitValue
    large_value = (1 << 96) - 1
    bv3 = BitValue(large_value, 96)
    writer.write_bit_value(bv3)

    # Flush to ensure all bits are written
    writer.flush()

    # Reset stream position
    stream.seek(0)

    # Create a reader
    reader = BitStreamReader(stream)

    # Read back the BitValue objects
    read_bv1 = reader.read_bit_value(8)
    assert int(read_bv1) == 42
    assert read_bv1.bit_count == 8

    read_bv2 = reader.read_bit_value(8, is_signed=True)
    assert int(read_bv2) == -42
    assert read_bv2.bit_count == 8

    read_bv3 = reader.read_bit_value(96)
    assert int(read_bv3) == large_value
    assert read_bv3.bit_count == 96


def test_bit_stream_reader_writer_error_handling():
    """Test error handling in BitStreamReader and BitStreamWriter."""
    # Test reader error handling
    stream = io.BytesIO(bytes([0xFF]))
    reader = BitStreamReader(stream)

    # Test invalid bit counts
    with pytest.raises(InvalidBitCountError):
        reader.read_bits(0)

    with pytest.raises(InvalidBitCountError):
        reader.read_bits(65)

    # Read all available bits
    reader.read_bits(8)

    # Try to read beyond end
    with pytest.raises(EndOfStreamError):
        reader.read_bits(1)

    # Test writer error handling
    stream = io.BytesIO()
    writer = BitStreamWriter(stream)

    # Test invalid bit counts
    with pytest.raises(InvalidBitCountError):
        writer.write_bits(0, 0)

    with pytest.raises(InvalidBitCountError):
        writer.write_bits(0, 65)


def test_bit_stream_reader_writer_buffer_handling():
    """Test buffer handling in BitStreamReader and BitStreamWriter."""
    # Create a large amount of data
    data_size = 8192  # 8KB
    data = bytes([0xAA] * data_size)

    # Test reader with large data
    stream = io.BytesIO(data)
    reader = BitStreamReader(stream, buffer_size=1024)

    # Read all data bit by bit
    for _ in range(data_size * 8 // 8):
        value = reader.read_bits(8)
        assert value == 0xAA

    # Check that we've read all the data
    with pytest.raises(EndOfStreamError):
        reader.read_bits(1)

    # Test writer with large data
    stream = io.BytesIO()
    writer = BitStreamWriter(stream, buffer_size=1024)

    # Write a large amount of data
    for _ in range(data_size):
        writer.write_bits(0xAA, 8)

    writer.flush()

    # Verify the data
    stream.seek(0)
    written_data = stream.read()
    assert len(written_data) == data_size
    assert all(b == 0xAA for b in written_data)


def test_lsb_order_all_bit_lengths():
    """Test LSB order for all bit lengths from 1 to 128."""
    from variable_streams.bit_stream_writer import BitStreamWriter
    from variable_streams.bit_stream_reader import BitStreamReader
    import io
    
    # Test LSB order for all bit lengths from 1 to 128
    for bit_count in range(1, 129):
        if bit_count <= 64:
            test_value = (1 << (bit_count - 1)) | 1  # Set MSB and LSB
        else:
            test_value = (1 << (bit_count - 1)) | 1  # Set MSB and LSB
        
        # Write the value
        output = io.BytesIO()
        writer = BitStreamWriter(output)
        if bit_count <= 64:
            writer.write_bits(test_value, bit_count)
        else:
            writer.write_bits_u128(test_value, bit_count)
        writer.flush()
        
        # Read the value back
        output.seek(0)
        reader = BitStreamReader(output)
        if bit_count <= 64:
            read_value = reader.read_bits(bit_count)
        else:
            read_value = reader.read_bits_u128(bit_count)
        
        assert read_value == test_value, f"Failed for {bit_count} bits"
