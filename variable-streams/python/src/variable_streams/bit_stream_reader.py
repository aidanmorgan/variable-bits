"""BitStreamReader class for reading bits from an external stream."""

from typing import BinaryIO, Optional, Union
from .bit_value import BitValue, InvalidBitCountError, EndOfStreamError


class BitStreamReader:
    """A reader that allows reading individual bits from an underlying stream.

    This class provides methods for reading bits from any binary stream that
    supports the read method.
    """

    def __init__(self, stream: BinaryIO, buffer_size: int = 4096):
        """Initialize a new BitStreamReader.

        Args:
            stream: The binary stream to read from.
            buffer_size: The size of the internal buffer in bytes.
        """
        self._stream = stream
        self._buffer = bytearray(buffer_size)
        self._byte_pos = 0
        self._bit_pos = 0
        self._buffer_size = 0
        self._eof = False

    def is_eof(self) -> bool:
        """Check if the end of the stream has been reached."""
        import io

        # If we haven't reached the end of the underlying stream, we're not at EOF
        if not self._eof:
            return False

        # If we've read all the data in the buffer, we're at EOF
        if self._byte_pos >= self._buffer_size:
            return True

        # If we're at the last byte in the buffer and have read all bits in that byte, we're at EOF
        if self._byte_pos == self._buffer_size - 1 and self._bit_pos == 8:
            return True

        # Try to read more data to see if we're at EOF
        try:
            self._stream.seek(0, io.SEEK_CUR)  # Get current position
            if not self._stream.read(1):  # Try to read one more byte
                # No more data in the stream
                self._eof = True
                return self._byte_pos >= self._buffer_size
            else:
                # There's more data, so we're not at EOF
                self._stream.seek(-1, io.SEEK_CUR)  # Go back one byte
                return False
        except:
            # If there's an error reading, assume we're at EOF
            return True

    def _fill_buffer(self) -> None:
        """Fill the internal buffer with data from the underlying stream.

        Raises:
            EndOfStreamError: If the end of the stream has been reached.
        """
        # Reset buffer position
        self._byte_pos = 0
        self._bit_pos = 0

        # Read from the underlying stream
        bytes_read = self._stream.read(len(self._buffer))
        if not bytes_read:
            # End of stream
            self._buffer_size = 0
            self._eof = True
        else:
            # Successfully read bytes
            self._buffer_size = len(bytes_read)
            self._buffer[:self._buffer_size] = bytes_read

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

        result = 0
        bits_read = 0

        while bits_read < bit_count:
            # Check if we need to load more data
            if self._byte_pos >= self._buffer_size:
                if self._eof:
                    if bits_read == 0:
                        raise EndOfStreamError("End of stream reached")
                    break

                # Load more data from the underlying stream
                self._fill_buffer()

                # Check if we reached the end of the stream
                if self._buffer_size == 0:
                    self._eof = True
                    if bits_read == 0:
                        raise EndOfStreamError("End of stream reached")
                    break

            current_byte = self._buffer[self._byte_pos]
            bits_left_in_byte = 8 - self._bit_pos
            bits_to_read = min(bit_count - bits_read, bits_left_in_byte)

            # Extract bits from the current byte (LSB first)
            mask = ((1 << bits_to_read) - 1)
            shift = self._bit_pos
            extracted_bits = (current_byte >> shift) & mask

            # Add the extracted bits to the result
            result |= (extracted_bits << bits_read)
            bits_read += bits_to_read

            # Update position
            self._bit_pos += bits_to_read
            if self._bit_pos == 8:
                self._byte_pos += 1
                self._bit_pos = 0

        return result

    def read_bits_u128(self, bit_count: int) -> int:
        """Read up to 128 bits from the stream in LSB order (lower bits first)."""
        if bit_count <= 0 or bit_count > 128:
            raise InvalidBitCountError(f"Bit count must be between 1 and 128, got {bit_count}")
        if bit_count <= 64:
            return self.read_bits(bit_count)
        # For bit counts > 64, read the lower 64 bits first (LSB), then the higher bits
        low_value = self.read_bits(64)
        high_bits = bit_count - 64
        high_value = self.read_bits(high_bits)
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
