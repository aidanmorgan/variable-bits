"""BitStreamWriter class for writing bits to an external stream."""

from typing import BinaryIO, Optional, Union
from .bit_value import BitValue, InvalidBitCountError


class BitStreamWriter:
    """A writer that allows writing individual bits to an underlying stream.
    
    This class provides methods for writing bits to any binary stream that
    supports the write method.
    """
    
    def __init__(self, stream: BinaryIO, buffer_size: int = 4096):
        """Initialize a new BitStreamWriter.
        
        Args:
            stream: The binary stream to write to.
            buffer_size: The size of the internal buffer in bytes.
        """
        self._stream = stream
        self._buffer = bytearray(buffer_size)
        self._byte_pos = 0
        self._bit_pos = 0
        self._buffer_size = buffer_size
    
    def _flush_buffer(self) -> None:
        """Flush the internal buffer to the underlying stream."""
        if self._byte_pos > 0:
            # Write the buffer to the underlying stream
            self._stream.write(self._buffer[:self._byte_pos])
            self._byte_pos = 0
            self._buffer[0] = 0  # Clear the first byte for the next write
    
    def flush(self) -> None:
        """Flush any remaining bits to the underlying stream."""
        # If there are any bits in the current byte, write it
        if self._bit_pos > 0:
            self._byte_pos += 1
            self._bit_pos = 0
        
        # Flush the buffer
        self._flush_buffer()
        
        # Flush the underlying stream
        self._stream.flush()
    
    def write_bits(self, value: int, bit_count: int) -> None:
        """Write up to 64 bits to the stream in LSB order (least significant bit first)."""
        if bit_count <= 0 or bit_count > 64:
            raise InvalidBitCountError(f"Bit count must be between 1 and 64, got {bit_count}")
        
        bits_written = 0
        
        # Ensure the buffer has enough space
        if self._byte_pos >= self._buffer_size:
            self._flush_buffer()
        
        while bits_written < bit_count:
            bits_left_in_byte = 8 - self._bit_pos
            bits_to_write = min(bit_count - bits_written, bits_left_in_byte)
            
            # Extract the bits to write (LSB first)
            shift = bits_written
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
                
                # If the buffer is full, flush it
                if self._byte_pos >= self._buffer_size:
                    self._flush_buffer()
    
    def write_bits_u128(self, value: int, bit_count: int) -> None:
        """Write up to 128 bits to the stream in LSB order (lower bits first)."""
        if bit_count <= 0 or bit_count > 128:
            raise InvalidBitCountError(f"Bit count must be between 1 and 128, got {bit_count}")
        if bit_count <= 64:
            return self.write_bits(value, bit_count)
        # For bit counts > 64, write the lower 64 bits first (LSB), then the higher bits
        low_value = value & ((1 << 64) - 1)
        high_bits = bit_count - 64
        high_value = value >> 64
        self.write_bits(low_value, 64)
        self.write_bits(high_value, high_bits)
    
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
    
    def __del__(self) -> None:
        """Ensure the buffer is flushed when the object is garbage collected."""
        try:
            self.flush()
        except:
            # Ignore errors during garbage collection
            pass