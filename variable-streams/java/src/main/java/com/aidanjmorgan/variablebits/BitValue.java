package com.aidanjmorgan.variablebits;

import java.math.BigInteger;
import java.util.Objects;

/**
 * Represents a value with a specific bit width.
 */
public final class BitValue {

    /**
     * The value, stored in the most appropriate type based on bit count.
     */
    private final Object value;

    /**
     * The number of bits in the value (1-128).
     */
    private final byte bitCount;

    /**
     * Private constructor to create a BitValue with the specified value and bit count.
     *
     * @param value The value object.
     * @param bitCount The number of bits (1-128).
     */
    private BitValue(Object value, byte bitCount) {
        this.value = value;
        this.bitCount = bitCount;
    }

    /**
     * Gets the bit count of this value.
     *
     * @return The bit count.
     */
    public byte getBitCount() {
        return bitCount;
    }

    /**
     * Checks if this value is signed.
     *
     * @return True if the value is signed, false otherwise.
     */
    public boolean isSigned() {
        return value instanceof Byte || value instanceof Short || 
               value instanceof Integer || value instanceof Long || 
               (value instanceof BigInteger && ((BigInteger) value).signum() < 0);
    }

    /**
     * Creates a new BitValue with the appropriate type based on the bit count.
     *
     * @param value The unsigned value.
     * @param bitCount The number of bits (1-128).
     * @return A new BitValue.
     * @throws BitStreamException If the bit count is invalid.
     */
    public static BitValue newValue(long value, byte bitCount) {
        if (bitCount <= 0 || bitCount > 64) {
            throw BitStreamException.invalidBitCount();
        }

        // Mask the value to ensure it only contains the specified number of bits
        long maskedValue = bitCount == 64 
            ? value 
            : value & ((1L << bitCount) - 1);

        // Choose the appropriate type based on the bit count
        Object typedValue;
        if (bitCount <= 8) {
            typedValue = (byte) maskedValue;
        } else if (bitCount <= 16) {
            typedValue = (short) maskedValue;
        } else if (bitCount <= 32) {
            typedValue = (int) maskedValue;
        } else {
            typedValue = maskedValue;
        }

        return new BitValue(typedValue, bitCount);
    }

    /**
     * Creates a new BitValue with the appropriate type based on the bit count for 128-bit values.
     *
     * @param value The 128-bit value.
     * @param bitCount The number of bits (1-128).
     * @param signed Whether the value is signed.
     * @return A new BitValue.
     * @throws BitStreamException If the bit count is invalid.
     */
    public static BitValue newBigIntegerValue(BigInteger value, byte bitCount, boolean signed) {
        if (bitCount <= 0 || bitCount > 128) {
            throw BitStreamException.invalidBitCount();
        }

        // Mask the value to ensure it only contains the specified number of bits
        BigInteger maskedValue;
        if (bitCount == 128) {
            maskedValue = value;
        } else if (!signed) {
            // For unsigned values, mask to the specified bit count
            maskedValue = value.and(BigInteger.ONE.shiftLeft(bitCount).subtract(BigInteger.ONE));
        } else {
            // For signed values, we don't mask as we want to preserve the sign bit
            maskedValue = value;
        }

        // Choose the appropriate type based on the bit count
        Object typedValue;
        if (bitCount <= 8) {
            typedValue = maskedValue.byteValue();
        } else if (bitCount <= 16) {
            typedValue = maskedValue.shortValue();
        } else if (bitCount <= 32) {
            typedValue = maskedValue.intValue();
        } else if (bitCount <= 64) {
            typedValue = maskedValue.longValue();
        } else {
            typedValue = maskedValue;
        }

        return new BitValue(typedValue, bitCount);
    }

    /**
     * Creates a new signed BitValue with the appropriate type based on the bit count.
     *
     * @param value The signed value.
     * @param bitCount The number of bits (1-64).
     * @return A new BitValue.
     * @throws BitStreamException If the bit count is invalid.
     */
    public static BitValue newSignedValue(long value, byte bitCount) {
        if (bitCount <= 0 || bitCount > 64) {
            throw BitStreamException.invalidBitCount();
        }

        // Choose the appropriate type based on the bit count
        Object typedValue;
        if (bitCount <= 8) {
            typedValue = (byte) value;
        } else if (bitCount <= 16) {
            typedValue = (short) value;
        } else if (bitCount <= 32) {
            typedValue = (int) value;
        } else {
            typedValue = value;
        }

        return new BitValue(typedValue, bitCount);
    }


    /**
     * Converts the value to a 64-bit unsigned long.
     *
     * @return The value as a 64-bit unsigned long.
     */
    public long toUInt64() {
        if (value instanceof Byte) {
            return ((Byte) value) & 0xFFL;
        } else if (value instanceof Short) {
            return ((Short) value) & 0xFFFFL;
        } else if (value instanceof Integer) {
            return ((Integer) value) & 0xFFFFFFFFL;
        } else if (value instanceof Long) {
            return (Long) value;
        } else if (value instanceof BigInteger) {
            return ((BigInteger) value).and(BigInteger.valueOf(-1L)).longValue();
        } else {
            throw new IllegalStateException("Unexpected value type: " + value.getClass());
        }
    }

    /**
     * Converts the value to a 128-bit unsigned integer.
     *
     * @return The value as a 128-bit unsigned BigInteger.
     */
    public BigInteger toUnsignedBigInteger() {
        if (value instanceof Byte) {
            byte byteValue = (Byte) value;
            return BigInteger.valueOf(byteValue & 0xFFL);
        } else if (value instanceof Short) {
            short shortValue = (Short) value;
            return BigInteger.valueOf(shortValue & 0xFFFFL);
        } else if (value instanceof Integer) {
            int intValue = (Integer) value;
            return BigInteger.valueOf(intValue & 0xFFFFFFFFL);
        } else if (value instanceof Long) {
            long longValue = (Long) value;
            if (longValue < 0) {
                // Convert negative long to unsigned BigInteger
                return BigInteger.valueOf(longValue & Long.MAX_VALUE).setBit(63);
            } else {
                return BigInteger.valueOf(longValue);
            }
        } else if (value instanceof BigInteger) {
            return (BigInteger) value;
        } else {
            throw new IllegalStateException("Unexpected value type: " + value.getClass());
        }
    }

    /**
     * Converts the value to a 64-bit signed long.
     *
     * @return The value as a 64-bit signed long.
     */
    public long toInt64() {
        if (value instanceof Byte) {
            return (Byte) value;
        } else if (value instanceof Short) {
            return (Short) value;
        } else if (value instanceof Integer) {
            return (Integer) value;
        } else if (value instanceof Long) {
            return (Long) value;
        } else if (value instanceof BigInteger) {
            return ((BigInteger) value).longValue();
        } else {
            throw new IllegalStateException("Unexpected value type: " + value.getClass());
        }
    }

    /**
     * Converts the value to a 128-bit signed integer.
     *
     * @return The value as a 128-bit signed BigInteger.
     */
    public BigInteger toBigInteger() {
        if (value instanceof Byte) {
            return BigInteger.valueOf((Byte) value);
        } else if (value instanceof Short) {
            return BigInteger.valueOf((Short) value);
        } else if (value instanceof Integer) {
            return BigInteger.valueOf((Integer) value);
        } else if (value instanceof Long) {
            return BigInteger.valueOf((Long) value);
        } else if (value instanceof BigInteger) {
            return (BigInteger) value;
        } else {
            throw new IllegalStateException("Unexpected value type: " + value.getClass());
        }
    }

    /**
     * Creates a BitValue from a byte.
     *
     * @param value The byte value.
     * @return A new BitValue.
     */
    public static BitValue fromByte(byte value) {
        return new BitValue(value, (byte) 8);
    }

    /**
     * Creates a BitValue from a short.
     *
     * @param value The short value.
     * @return A new BitValue.
     */
    public static BitValue fromShort(short value) {
        return new BitValue(value, (byte) 16);
    }

    /**
     * Creates a BitValue from an int.
     *
     * @param value The int value.
     * @return A new BitValue.
     */
    public static BitValue fromInt(int value) {
        return new BitValue(value, (byte) 32);
    }

    /**
     * Creates a BitValue from a long.
     *
     * @param value The long value.
     * @return A new BitValue.
     */
    public static BitValue fromLong(long value) {
        return new BitValue(value, (byte) 64);
    }

    /**
     * Creates a BitValue from a BigInteger.
     *
     * @param value The BigInteger value.
     * @return A new BitValue.
     */
    public static BitValue fromBigInteger(BigInteger value) {
        return new BitValue(value, (byte) 128);
    }

    /**
     * Checks if this BitValue is equal to another object.
     *
     * @param obj The other object.
     * @return True if the objects are equal, false otherwise.
     */
    @Override
    public boolean equals(Object obj) {
        if (this == obj) {
            return true;
        }
        if (obj == null || getClass() != obj.getClass()) {
            return false;
        }
        BitValue other = (BitValue) obj;
        return bitCount == other.bitCount && Objects.equals(value, other.value);
    }

    /**
     * Computes the hash code for this BitValue.
     *
     * @return The hash code.
     */
    @Override
    public int hashCode() {
        return Objects.hash(value, bitCount);
    }

    /**
     * Returns a string representation of this BitValue.
     *
     * @return The string representation.
     */
    @Override
    public String toString() {
        return value + " (" + bitCount + " bits, " + (isSigned() ? "signed" : "unsigned") + ")";
    }
}
