using System;
using System.IO;
using System.Numerics;

namespace VariableBits
{
    /// <summary>
    /// A writer that allows writing individual bits to an underlying stream.
    /// </summary>
    public class BitStreamWriter : IDisposable
    {
        /// <summary>
        /// The underlying writer.
        /// </summary>
        private readonly Stream _stream;

        /// <summary>
        /// Buffer for writing to the underlying stream.
        /// </summary>
        private readonly byte[] _buffer;

        /// <summary>
        /// Current byte position in the buffer.
        /// </summary>
        private int _bytePos;

        /// <summary>
        /// Current bit position within the current byte (0-7).
        /// </summary>
        private byte _bitPos;

        /// <summary>
        /// Default buffer size.
        /// </summary>
        private const int DefaultBufferSize = 4096;

        private readonly bool _leaveOpen;

        /// <summary>
        /// Creates a new BitStreamWriter with the specified stream.
        /// </summary>
        /// <param name="stream">The underlying stream to write to.</param>
        public BitStreamWriter(Stream stream, bool leaveOpen = false) : this(stream, DefaultBufferSize, leaveOpen) {}

        /// <summary>
        /// Creates a new BitStreamWriter with the specified stream and buffer capacity.
        /// </summary>
        /// <param name="stream">The underlying stream to write to.</param>
        /// <param name="capacity">The buffer capacity.</param>
        public BitStreamWriter(Stream stream, int capacity, bool leaveOpen = false)
        {
            _stream = stream ?? throw new ArgumentNullException(nameof(stream));
            _buffer = new byte[capacity];
            _bytePos = 0;
            _bitPos = 0;
            _leaveOpen = leaveOpen;
        }

        /// <summary>
        /// Writes up to 64 bits to the stream.
        /// </summary>
        /// <param name="value">The value to write.</param>
        /// <param name="bitCount">The number of bits to write (1-64).</param>
        /// <exception cref="BitStreamException">If the bit count is invalid or an I/O error occurs.</exception>
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
        }

        /// <summary>
        /// Flushes the buffer to the underlying stream.
        /// </summary>
        /// <exception cref="BitStreamException">If an I/O error occurs.</exception>
        private void FlushBuffer()
        {
            try
            {
                // Write the buffer to the stream
                _stream.Write(_buffer, 0, _bytePos);

                // Reset the buffer
                Array.Clear(_buffer, 0, _buffer.Length);
                _bytePos = 0;
                // Keep the bit position as it is
            }
            catch (IOException ex)
            {
                throw BitStreamException.FromIOException(ex);
            }
        }

        /// <summary>
        /// Flushes any remaining bits to the underlying stream.
        /// </summary>
        /// <exception cref="BitStreamException">If an I/O error occurs.</exception>
        public void Flush()
        {
            try
            {
                // If we have any bits in the current byte, write it
                if (_bytePos > 0 || _bitPos > 0)
                {
                    // Write the buffer to the stream
                    _stream.Write(_buffer, 0, _bytePos + (_bitPos > 0 ? 1 : 0));

                    // Reset the buffer
                    Array.Clear(_buffer, 0, _buffer.Length);
                    _bytePos = 0;
                    _bitPos = 0;
                }

                // Flush the underlying stream
                _stream.Flush();
            }
            catch (IOException ex)
            {
                throw BitStreamException.FromIOException(ex);
            }
        }

        /// <summary>
        /// Writes up to 128 bits to the stream.
        /// </summary>
        /// <param name="value">The value to write.</param>
        /// <param name="bitCount">The number of bits to write (1-128).</param>
        /// <exception cref="BitStreamException">If the bit count is invalid or an I/O error occurs.</exception>
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

            if (bitCount <= 64)
            {
                WriteBits((ulong)maskedValue, bitCount);
                return;
            }

            // For bit counts > 64, write the lower bits first (LSB), then the higher bits
            ulong low = (ulong)(maskedValue & ((BigInteger.One << 64) - 1));
            ulong high = (ulong)(maskedValue >> 64);
            WriteBits(low, 64);
            WriteBits(high, (byte)(bitCount - 64));
        }

        private static byte ReverseBits(byte b)
        {
            b = (byte)((b * 0x0202020202 & 0x010884422010) % 1023);
            return b;
        }

        private void WriteBit(bool bit)
        {
            if (_bytePos >= _buffer.Length)
            {
                FlushBuffer();
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
        /// <exception cref="BitStreamException">If the bit count is invalid or an I/O error occurs.</exception>
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
        /// Disposes the BitStreamWriter, flushing any remaining bits and disposing the underlying stream.
        /// </summary>
        public void Dispose()
        {
            Flush();
            if (!_leaveOpen)
            {
                _stream.Dispose();
            }
        }
    }
}
