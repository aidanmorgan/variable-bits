using System;
using System.Numerics;

namespace VariableBits
{
    /// <summary>
    /// Represents a value with a specific bit width.
    /// </summary>
    public readonly struct BitValue : IEquatable<BitValue>
    {
        private readonly object _value;
        private readonly byte _bitCount;

        /// <summary>
        /// Gets the bit count of this value.
        /// </summary>
        public byte BitCount => _bitCount;

        /// <summary>
        /// Gets whether this value is signed.
        /// </summary>
        public bool IsSigned => _value switch
        {
            sbyte or short or int or long or nint => true,
            _ => false
        };

        /// <summary>
        /// Gets whether this value is a BigInteger.
        /// </summary>
        public bool IsBigInteger => _value is BigInteger;

        private BitValue(object value, byte bitCount)
        {
            _value = value;
            _bitCount = bitCount;
        }

        /// <summary>
        /// Creates a new BitValue with the appropriate type based on the bit count.
        /// </summary>
        /// <param name="value">The unsigned value.</param>
        /// <param name="bitCount">The number of bits (1-128).</param>
        /// <returns>A new BitValue.</returns>
        /// <exception cref="BitStreamException">If the bit count is invalid.</exception>
        public static BitValue New(ulong value, byte bitCount)
        {
            if (bitCount is 0 or > 64)
            {
                throw BitStreamException.InvalidBitCount();
            }

            // Mask the value to ensure it only contains the specified number of bits
            var maskedValue = bitCount == 64
                ? value // No need to mask for 64 bits
                : value & ((1UL << bitCount) - 1);

            // Choose the appropriate type based on the bit count
            object typedValue = bitCount switch
            {
                <= 8 => (byte)maskedValue,
                <= 16 => (ushort)maskedValue,
                <= 32 => (uint)maskedValue,
                _ => maskedValue
            };

            return new BitValue(typedValue, bitCount);
        }

        /// <summary>
        /// Creates a new BitValue with the appropriate type based on the bit count for 128-bit values.
        /// </summary>
        /// <param name="value">The unsigned 128-bit value.</param>
        /// <param name="bitCount">The number of bits (1-128).</param>
        /// <returns>A new BitValue.</returns>
        /// <exception cref="BitStreamException">If the bit count is invalid.</exception>
        public static BitValue NewUInt128(BigInteger value, byte bitCount)
        {
            if (bitCount is 0 or > 128)
            {
                throw BitStreamException.InvalidBitCount();
            }

            // Mask the value to ensure it only contains the specified number of bits
            var maskedValue = bitCount == 128
                ? value // No need to mask for 128 bits
                : value & ((BigInteger.One << bitCount) - BigInteger.One);

            // Choose the appropriate type based on the bit count
            object typedValue = bitCount switch
            {
                <= 8 => (byte)maskedValue,
                <= 16 => (ushort)maskedValue,
                <= 32 => (uint)maskedValue,
                <= 64 => (ulong)maskedValue,
                _ => maskedValue
            };

            return new BitValue(typedValue, bitCount);
        }

        /// <summary>
        /// Creates a new signed BitValue with the appropriate type based on the bit count.
        /// </summary>
        /// <param name="value">The signed value.</param>
        /// <param name="bitCount">The number of bits (1-64).</param>
        /// <returns>A new BitValue.</returns>
        /// <exception cref="BitStreamException">If the bit count is invalid.</exception>
        public static BitValue NewSigned(long value, byte bitCount)
        {
            if (bitCount is 0 or > 64)
            {
                throw BitStreamException.InvalidBitCount();
            }

            // Choose the appropriate type based on the bit count
            object typedValue = bitCount switch
            {
                <= 8 => (sbyte)value,
                <= 16 => (short)value,
                <= 32 => (int)value,
                _ => value
            };

            return new BitValue(typedValue, bitCount);
        }

        /// <summary>
        /// Creates a new signed BitValue with the appropriate type based on the bit count for 128-bit values.
        /// </summary>
        /// <param name="value">The signed 128-bit value.</param>
        /// <param name="bitCount">The number of bits (1-128).</param>
        /// <returns>A new BitValue.</returns>
        /// <exception cref="BitStreamException">If the bit count is invalid.</exception>
        public static BitValue NewInt128(BigInteger value, byte bitCount)
        {
            if (bitCount is 0 or > 128)
            {
                throw BitStreamException.InvalidBitCount();
            }

            // Choose the appropriate type based on the bit count
            object typedValue = bitCount switch
            {
                <= 8 => (sbyte)value,
                <= 16 => (short)value,
                <= 32 => (int)value,
                <= 64 => (long)value,
                _ => value
            };

            return new BitValue(typedValue, bitCount);
        }

        /// <summary>
        /// Converts the value to a 64-bit unsigned integer.
        /// </summary>
        /// <returns>The value as a 64-bit unsigned integer.</returns>
        /// <exception cref="InvalidCastException">If the value cannot be converted to ulong.</exception>
        public ulong ToUInt64() => _value switch
        {
            byte b => b,
            ushort us => us,
            uint ui => ui,
            ulong ul => ul,
            BigInteger bi => bi < 0 || bi > ulong.MaxValue 
                ? throw new InvalidCastException("BigInteger value is negative or too large for ulong") 
                : (ulong)bi,
            sbyte sb => (ulong)sb,
            short s => (ulong)s,
            int i => (ulong)i,
            long l => (ulong)l,
            _ => throw new InvalidCastException()
        };

        /// <summary>
        /// Converts the value to a 128-bit unsigned integer.
        /// </summary>
        /// <returns>The value as a 128-bit unsigned integer.</returns>
        public BigInteger ToUInt128() => _value switch
        {
            BigInteger bi => bi,
            sbyte sb => (BigInteger)sb,
            short s => (BigInteger)s,
            int i => (BigInteger)i,
            long l => (BigInteger)l,
            byte b => (BigInteger)b,
            ushort us => (BigInteger)us,
            uint ui => (BigInteger)ui,
            ulong ul => (BigInteger)ul,
            _ => throw new InvalidCastException()
        };

        /// <summary>
        /// Converts the value to a 64-bit signed integer.
        /// </summary>
        /// <returns>The value as a 64-bit signed integer.</returns>
        public long ToInt64() => _value switch
        {
            byte b => b,
            ushort us => us,
            uint ui => (int)ui,
            ulong ul => (long)ul,
            BigInteger bi => (long)bi,
            sbyte sb => sb,
            short s => s,
            int i => i,
            long l => l,
            _ => throw new InvalidCastException()
        };

        /// <summary>
        /// Converts the value to a 128-bit signed integer.
        /// </summary>
        /// <returns>The value as a 128-bit signed integer.</returns>
        public BigInteger ToInt128() => _value switch
        {
            BigInteger bi => bi,
            sbyte sb => (BigInteger)sb,
            short s => (BigInteger)s,
            int i => (BigInteger)i,
            long l => (BigInteger)l,
            byte b => (BigInteger)b,
            ushort us => (BigInteger)us,
            uint ui => (BigInteger)ui,
            ulong ul => (BigInteger)ul,
            _ => throw new InvalidCastException()
        };

        // Implicit conversion operators from primitive types to BitValue
        public static implicit operator BitValue(byte value) => new(value, 8);
        public static implicit operator BitValue(ushort value) => new(value, 16);
        public static implicit operator BitValue(uint value) => new(value, 32);
        public static implicit operator BitValue(ulong value) => new(value, 64);
        public static implicit operator BitValue(sbyte value) => new(value, 8);
        public static implicit operator BitValue(short value) => new(value, 16);
        public static implicit operator BitValue(int value) => new(value, 32);
        public static implicit operator BitValue(long value) => new(value, 64);

        // Explicit conversion operators from BitValue to primitive types
        public static explicit operator byte(BitValue value) => (byte)value._value;
        public static explicit operator ushort(BitValue value) => (ushort)value._value;
        public static explicit operator uint(BitValue value) => (uint)value._value;
        public static explicit operator ulong(BitValue value) => (ulong)value._value;
        public static explicit operator sbyte(BitValue value) => (sbyte)value._value;
        public static explicit operator short(BitValue value) => (short)value._value;
        public static explicit operator int(BitValue value) => (int)value._value;
        public static explicit operator long(BitValue value) => (long)value._value;

        // Equality and comparison
        public bool Equals(BitValue other) => 
            _bitCount == other._bitCount && 
            _value.Equals(other._value);

        public override bool Equals(object? obj) => 
            obj is BitValue other && Equals(other);

        public override int GetHashCode() => 
            HashCode.Combine(_value, _bitCount);

        public static bool operator ==(BitValue left, BitValue right) => 
            left.Equals(right);

        public static bool operator !=(BitValue left, BitValue right) => 
            !left.Equals(right);

        public override string ToString() => 
            $"{_value} ({_bitCount} bits, {(IsSigned ? "signed" : "unsigned")})";
    }
}