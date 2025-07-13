"""BitValue class for representing variable-length bit values."""

from enum import Enum
from typing import Optional, Union


class BitStreamError(Exception):
    """Exception raised for errors in the bit stream operations."""
    pass


class InvalidBitCountError(BitStreamError):
    """Exception raised when an invalid bit count is provided."""
    pass


class EndOfStreamError(BitStreamError):
    """Exception raised when attempting to read beyond the end of a stream."""
    pass


class BitValue:
    """A class representing a variable-length bit value.

    This class can store unsigned and signed integers of various bit lengths
    (up to 128 bits).
    """

    def __init__(self, value: Union[int, 'BitValue'], bit_count: Optional[int] = None):
        """Initialize a BitValue with the given value and bit count.

        Args:
            value: The integer value or another BitValue to initialize from.
            bit_count: The number of bits to use (1-128). If None and value is an int,
                       the minimum number of bits needed to represent the value is used.
                       If None and value is a BitValue, the bit count of that value is used.

        Raises:
            InvalidBitCountError: If bit_count is invalid (0 or > 128).
            ValueError: If the value cannot be represented with the given bit count.
        """
        if isinstance(value, BitValue):
            # Copy constructor
            self._value = value._value
            self._bit_count = value._bit_count
            self._is_signed = value._is_signed
            return

        # Determine if the value is signed
        is_signed = value < 0

        # If bit_count is not provided, calculate the minimum required
        if bit_count is None:
            if is_signed:
                # For signed values, we need one more bit for the sign
                bit_count = (value.bit_length() + 1)
            else:
                bit_count = max(1, value.bit_length())

        # Validate bit count
        if bit_count <= 0 or bit_count > 128:
            raise InvalidBitCountError(f"Bit count must be between 1 and 128, got {bit_count}")

        # Validate value fits in bit_count
        if is_signed:
            min_value = -(1 << (bit_count - 1))
            max_value = (1 << (bit_count - 1)) - 1
            if value < min_value or value > max_value:
                raise ValueError(f"Value {value} cannot be represented with {bit_count} bits as signed")
        else:
            max_value = (1 << bit_count) - 1
            if value < 0 or value > max_value:
                raise ValueError(f"Value {value} cannot be represented with {bit_count} bits as unsigned")

        self._value = value
        self._bit_count = bit_count
        self._is_signed = is_signed

    @property
    def bit_count(self) -> int:
        """Get the number of bits used to represent this value."""
        return self._bit_count

    @property
    def is_signed(self) -> bool:
        """Check if this value is signed."""
        return self._is_signed

    def to_int(self) -> int:
        """Convert the BitValue to a Python integer."""
        return self._value

    def __int__(self) -> int:
        """Convert the BitValue to a Python integer."""
        return self._value

    def __repr__(self) -> str:
        """Return a string representation of the BitValue."""
        sign = "signed" if self._is_signed else "unsigned"
        return f"BitValue({self._value}, {self._bit_count} bits, {sign})"

    def __eq__(self, other) -> bool:
        """Check if this BitValue is equal to another BitValue or integer."""
        if isinstance(other, BitValue):
            return (self._value == other._value and 
                    self._bit_count == other._bit_count and 
                    self._is_signed == other._is_signed)
        elif isinstance(other, int):
            return self._value == other
        return NotImplemented
