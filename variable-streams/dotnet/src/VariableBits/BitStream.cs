using System;
using System.Collections.Generic;
using System.IO;
using System.Numerics;

namespace VariableBits
{
    /// <summary>
    /// A stream that allows reading and writing individual bits.
    /// </summary>
    public class BitStream
    {
        /// <summary>
        /// The internal buffer storing the data.
        /// </summary>
        private readonly List<byte> _buffer;

        /// <summary>
        /// Current byte position in the buffer.
        /// </summary>
        private int _bytePos;

        /// <summary>
        /// Current bit position within the current byte (0-7).
        /// </summary>
        private byte _bitPos;

        /// <summary>
        /// Total number of bits in the stream.
        /// </summary>
        private int _bitLength;

        /// <summary>
        /// Creates a new, empty BitStream.
        /// </summary>
        public BitStream()
        {
            _buffer = new List<byte>();
            _bytePos = 0;
            _bitPos = 0;
            _bitLength = 0;
        }

        /// <summary>
        /// Creates a new BitStream from an existing buffer.
        /// </summary>
        /// <param name="bytes">The initial buffer content.</param>
        public BitStream(IEnumerable<byte> bytes)
        {
            _buffer = new List<byte>(bytes);
            _bytePos = 0;
            _bitPos = 0;
            _bitLength = _buffer.Count * 8;
        }

        /// <summary>
        /// Returns the current position in bits.
        /// </summary>
        public int Position => _bytePos * 8 + _bitPos;

        /// <summary>
        /// Sets the current position in bits.
        /// </summary>
        /// <param name="position">The new position in bits.</param>
        /// <exception cref="BitStreamException">If the position is beyond the end of the stream.</exception>
        public void SetPosition(int position)
        {
            if (position > _bitLength)
            {
                throw BitStreamException.EndOfStream();
            }

            _bytePos = position / 8;
            _bitPos = (byte)(position % 8);
        }

        /// <summary>
        /// Returns the total length of the stream in bits.
        /// </summary>
        public int Length => _bitLength;

        /// <summary>
        /// Returns true if the stream is empty.
        /// </summary>
        public bool IsEmpty => _bitLength == 0;

        /// <summary>
        /// Reads up to 128 bits from the stream and returns a BitValue.
        /// </summary>
        /// <param name="bitCount">The number of bits to read (1-128).</param>
        /// <returns>The read bits as a BitValue.</returns>
        /// <exception cref="BitStreamException">If the bit count is invalid or the end of stream is reached.</exception>
        public BitValue ReadBitValue(byte bitCount)
        {
            if (bitCount <= 64)
            {
                var value = ReadBits(bitCount);
                return BitValue.New(value, bitCount);
            }
            else
            {
                var value = ReadBitsU128(bitCount);
                return BitValue.NewUInt128(value, bitCount);
            }
        }

        /// <summary>
        /// Reads up to 64 bits from the stream.
        /// </summary>
        /// <param name="bitCount">The number of bits to read (1-64).</param>
        /// <returns>The read bits as a 64-bit unsigned integer.</returns>
        /// <exception cref="BitStreamException">If the bit count is invalid or the end of stream is reached.</exception>
        public ulong ReadBits(byte bitCount)
        {
            if (bitCount == 0 || bitCount > 64)
            {
                throw BitStreamException.InvalidBitCount();
            }

            // Check if we have enough bits left
            if (Position + bitCount > _bitLength)
            {
                throw BitStreamException.EndOfStream();
            }

            ulong result = 0;
            for (int i = 0; i < bitCount; i++)
            {
                if (_bytePos >= _buffer.Count)
                {
                    throw BitStreamException.EndOfStream();
                }
                byte currentByte = _buffer[_bytePos];
                int bitIndex = _bitPos;
                bool bit = (currentByte & (1 << bitIndex)) != 0;
                if (bit)
                {
                    result |= (1UL << i);
                }
                _bitPos++;
                if (_bitPos == 8)
                {
                    _bytePos++;
                    _bitPos = 0;
                }
            }
            return result;
        }

        /// <summary>
        /// Reads up to 128 bits from the stream.
        /// </summary>
        /// <param name="bitCount">The number of bits to read (1-128).</param>
        /// <returns>The read bits as a 128-bit unsigned integer.</returns>
        /// <exception cref="BitStreamException">If the bit count is invalid or the end of stream is reached.</exception>
        public BigInteger ReadBitsU128(byte bitCount)
        {
            if (bitCount == 0 || bitCount > 128)
            {
                throw BitStreamException.InvalidBitCount();
            }

            // For bit counts up to 64, we can use the existing method
            if (bitCount <= 64)
            {
                return new BigInteger(ReadBits(bitCount));
            }

            // For bit counts > 64, we need to read in two parts
            // Read lower 64 bits first, then higher bits (LSB order)
            ulong lowBits = ReadBits(64);
            ulong highBits = ReadBits((byte)(bitCount - 64));

            return (new BigInteger(highBits) << 64) | new BigInteger(lowBits);
        }

        /// <summary>
        /// Writes up to 64 bits to the stream.
        /// </summary>
        /// <param name="value">The value to write.</param>
        /// <param name="bitCount">The number of bits to write (1-64).</param>
        /// <exception cref="BitStreamException">If the bit count is invalid.</exception>
        public void WriteBits(ulong value, byte bitCount)
        {
            if (bitCount == 0 || bitCount > 64)
            {
                throw BitStreamException.InvalidBitCount();
            }

            // Write bits LSB-first
            for (int i = 0; i < bitCount; i++)
            {
                bool bit = ((value >> i) & 1) != 0;
                WriteBit(bit);
            }

            // Update the bit length if we've written beyond the current end
            _bitLength = Math.Max(_bitLength, Position);
        }

        /// <summary>
        /// Writes up to 128 bits to the stream.
        /// </summary>
        /// <param name="value">The value to write.</param>
        /// <param name="bitCount">The number of bits to write (1-128).</param>
        /// <exception cref="BitStreamException">If the bit count is invalid.</exception>
        public void WriteBitsU128(BigInteger value, byte bitCount)
        {
            if (bitCount == 0 || bitCount > 128)
            {
                throw BitStreamException.InvalidBitCount();
            }

            // Mask the value to ensure it only contains the specified number of bits
            BigInteger maskedValue = bitCount == 128 
                ? value 
                : value & ((BigInteger.One << bitCount) - BigInteger.One);

            // Write bits LSB-first
            for (int i = 0; i < bitCount; i++)
            {
                bool bit = ((maskedValue >> i) & 1) != 0;
                WriteBit(bit);
            }

            // Update the bit length if we've written beyond the current end
            _bitLength = Math.Max(_bitLength, Position);
        }

        private static byte ReverseBits(byte b)
        {
            b = (byte)((b * 0x0202020202 & 0x010884422010) % 1023);
            return b;
        }

        private void WriteBit(bool bit)
        {
            if (_bytePos >= _buffer.Count)
            {
                _buffer.Add(0);
            }

            // Write bits in LSB-first order within each byte
            int bitIndex = _bitPos;
            byte mask = (byte)(1 << bitIndex);
            if (bit)
            {
                _buffer[_bytePos] |= mask;
            }
            else
            {
                _buffer[_bytePos] &= (byte)~mask;
            }
            _bitPos++;
            if (_bitPos == 8)
            {
                _bytePos++;
                _bitPos = 0;
            }
        }

        /// <summary>
        /// Writes a BitValue to the stream.
        /// </summary>
        /// <param name="value">The BitValue to write.</param>
        /// <param name="bitCount">Optional bit count override. If not provided, uses the BitValue's bit count.</param>
        /// <exception cref="BitStreamException">If the bit count is invalid.</exception>
        public void WriteBitValue(BitValue value, byte? bitCount = null)
        {
            byte bitsToWrite = bitCount ?? value.BitCount;

            if (bitsToWrite <= 64 && !value.IsBigInteger)
            {
                WriteBits(value.ToUInt64(), bitsToWrite);
            }
            else
            {
                WriteBitsU128(value.ToUInt128(), bitsToWrite);
            }
        }

        /// <summary>
        /// Converts the BitStream to a byte array.
        /// </summary>
        /// <returns>The content of the BitStream as a byte array.</returns>
        public byte[] ToArray() => _buffer.ToArray();

        /// <summary>
        /// Resets the BitStream to its initial state.
        /// </summary>
        public void Reset()
        {
            _buffer.Clear();
            _bytePos = 0;
            _bitPos = 0;
            _bitLength = 0;
        }

        /// <summary>
        /// Returns true if the current position is at the end of the stream.
        /// </summary>
        public bool IsEof => Position >= _bitLength && _bitLength > 0;
    }
}
