"""BitStream class for in-memory bit stream operations."""

from typing import List, Optional, Union
from .bit_value import BitValue, InvalidBitCountError, EndOfStreamError


class BitStream:
    """A stream that allows reading and writing individual bits.

    This class provides methods for reading and writing bits to an in-memory buffer.
    """

    def __init__(self, data: Optional[bytes] = None):
        """Initialize a new BitStream.

        Args:
            data: Optional bytes to initialize the stream with.
        """
        self._buffer = bytearray(data) if data is not None else bytearray()
        self._byte_pos = 0
        self._bit_pos = 0
        self._bit_length = len(self._buffer) * 8

    def position(self) -> int:
        """Get the current position in bits."""
        return self._byte_pos * 8 + self._bit_pos

    def set_position(self, position: int) -> None:
        """Set the current position in bits.

        Args:
            position: The position in bits to set.

        Raises:
            EndOfStreamError: If the position is beyond the end of the stream.
        """
        if position > self._bit_length:
            raise EndOfStreamError("Position is beyond the end of the stream")

        self._byte_pos = position // 8
        self._bit_pos = position % 8

    def length(self) -> int:
        """Get the total length of the stream in bits."""
        return self._bit_length

    def is_empty(self) -> bool:
        """Check if the stream is empty."""
        return self._bit_length == 0

    def is_eof(self) -> bool:
        """Check if the current position is at the end of the stream."""
        return self.position() >= self._bit_length

    def reset(self) -> None:
        """Reset the stream position to the beginning."""
        self._byte_pos = 0
        self._bit_pos = 0

    def read_bits(self, bit_count: int) -> int:
        """Read up to 64 bits from the stream.

        Args:
            bit_count: The number of bits to read (1-64).

        Returns:
            The read bits as an integer.

        Raises:
            InvalidBitCountError: If bit_count is invalid (0 or > 64).
            EndOfStreamError: If trying to read beyond the end of the stream.
        """
        if bit_count <= 0 or bit_count > 64:
            raise InvalidBitCountError(f"Bit count must be between 1 and 64, got {bit_count}")

        # Check if we're trying to read beyond the bit length of the stream
        if self.position() + bit_count > self._bit_length:
            raise EndOfStreamError("Trying to read beyond the end of the stream")

        result = 0
        bits_read = 0

        while bits_read < bit_count:
            # Check if we've reached the end of the buffer
            if self._byte_pos >= len(self._buffer):
                if bits_read == 0:
                    raise EndOfStreamError("End of stream reached")
                break

            current_byte = self._buffer[self._byte_pos]
            bits_left_in_byte = 8 - self._bit_pos
            bits_to_read = min(bit_count - bits_read, bits_left_in_byte)

            # Extract bits from the current byte
            mask = ((1 << bits_to_read) - 1)
            shift = bits_left_in_byte - bits_to_read
            extracted_bits = (current_byte >> shift) & mask

            # Add the extracted bits to the result
            result |= (extracted_bits << (bit_count - bits_read - bits_to_read))
            bits_read += bits_to_read

            # Update position
            self._bit_pos += bits_to_read
            if self._bit_pos == 8:
                self._byte_pos += 1
                self._bit_pos = 0

        return result

    def read_bits_u128(self, bit_count: int) -> int:
        """Read up to 128 bits from the stream.

        Args:
            bit_count: The number of bits to read (1-128).

        Returns:
            The read bits as an integer.

        Raises:
            InvalidBitCountError: If bit_count is invalid (0 or > 128).
            EndOfStreamError: If trying to read beyond the end of the stream.
        """
        if bit_count <= 0 or bit_count > 128:
            raise InvalidBitCountError(f"Bit count must be between 1 and 128, got {bit_count}")

        # For bit counts up to 64, we can use the existing read_bits method
        if bit_count <= 64:
            return self.read_bits(bit_count)

        # Check if we're trying to read beyond the bit length of the stream
        if self.position() + bit_count > self._bit_length:
            raise EndOfStreamError("Trying to read beyond the end of the stream")

        # For bit counts > 64, we need to read in two parts
        high_bits = bit_count - 64
        high_value = self.read_bits(high_bits)
        low_value = self.read_bits(64)

        # Combine the high and low parts
        result = (high_value << 64) | low_value

        return result

    def read_bit_value(self, bit_count: int, is_signed: bool = False) -> BitValue:
        """Read bits from the stream and return a BitValue.

        Args:
            bit_count: The number of bits to read (1-128).
            is_signed: Whether the value should be interpreted as signed.

        Returns:
            The read bits as a BitValue.

        Raises:
            InvalidBitCountError: If bit_count is invalid (0 or > 128).
            EndOfStreamError: If trying to read beyond the end of the stream.
        """
        if bit_count <= 64:
            value = self.read_bits(bit_count)
            # Convert to signed if necessary
            if is_signed and (value & (1 << (bit_count - 1))):
                # If the sign bit is set, convert to negative
                value = value - (1 << bit_count)
            return BitValue(value, bit_count)
        else:
            value = self.read_bits_u128(bit_count)
            # Convert to signed if necessary
            if is_signed and (value & (1 << (bit_count - 1))):
                # If the sign bit is set, convert to negative
                value = value - (1 << bit_count)
            return BitValue(value, bit_count)

    def write_bits(self, value: int, bit_count: int) -> None:
        """Write up to 64 bits to the stream.

        Args:
            value: The value to write.
            bit_count: The number of bits to write (1-64).

        Raises:
            InvalidBitCountError: If bit_count is invalid (0 or > 64).
        """
        if bit_count <= 0 or bit_count > 64:
            raise InvalidBitCountError(f"Bit count must be between 1 and 64, got {bit_count}")

        bits_written = 0

        # Ensure the buffer has enough space
        required_bytes = (self.position() + bit_count + 7) // 8
        if required_bytes > len(self._buffer):
            self._buffer.extend([0] * (required_bytes - len(self._buffer)))

        while bits_written < bit_count:
            bits_left_in_byte = 8 - self._bit_pos
            bits_to_write = min(bit_count - bits_written, bits_left_in_byte)

            # Extract the bits to write
            shift = bit_count - bits_written - bits_to_write
            mask = ((1 << bits_to_write) - 1) << shift
            extracted_bits = (value & mask) >> shift

            # Write the bits to the current byte
            byte_shift = bits_left_in_byte - bits_to_write
            byte_mask = ~(((1 << bits_to_write) - 1) << byte_shift) & 0xFF
            self._buffer[self._byte_pos] &= byte_mask
            self._buffer[self._byte_pos] |= (extracted_bits << byte_shift)

            # Update position
            bits_written += bits_to_write
            self._bit_pos += bits_to_write
            if self._bit_pos == 8:
                self._byte_pos += 1
                self._bit_pos = 0

        # Update the bit length if we've written beyond the current length
        new_position = self.position()
        if new_position > self._bit_length:
            self._bit_length = new_position

    def write_bits_u128(self, value: int, bit_count: int) -> None:
        """Write up to 128 bits to the stream.

        Args:
            value: The value to write.
            bit_count: The number of bits to write (1-128).

        Raises:
            InvalidBitCountError: If bit_count is invalid (0 or > 128).
        """
        if bit_count <= 0 or bit_count > 128:
            raise InvalidBitCountError(f"Bit count must be between 1 and 128, got {bit_count}")

        # For bit counts up to 64, we can use the existing write_bits method
        if bit_count <= 64:
            return self.write_bits(value, bit_count)

        # For bit counts > 64, we need to write in two parts
        high_bits = bit_count - 64
        high_value = (value >> 64)
        low_value = value & ((1 << 64) - 1)

        # Write the high bits first, then the low bits
        self.write_bits(high_value, high_bits)
        self.write_bits(low_value, 64)

    def write_bit_value(self, value: BitValue, bit_count: Optional[int] = None) -> None:
        """Write a BitValue to the stream.

        Args:
            value: The BitValue to write.
            bit_count: The number of bits to write (1-128). If None, uses the bit count of the BitValue.

        Raises:
            InvalidBitCountError: If bit_count is invalid (0 or > 128).
        """
        bit_count = bit_count if bit_count is not None else value.bit_count

        if bit_count <= 64:
            self.write_bits(int(value), bit_count)
        else:
            self.write_bits_u128(int(value), bit_count)

    def to_bytes(self) -> bytes:
        """Convert the stream to bytes.

        Returns:
            The stream data as bytes.
        """
        return bytes(self._buffer)

    def from_bytes(self, data: bytes) -> None:
        """Initialize the stream from bytes.

        Args:
            data: The bytes to initialize the stream with.
        """
        self._buffer = bytearray(data)
        self._byte_pos = 0
        self._bit_pos = 0
        self._bit_length = len(self._buffer) * 8
