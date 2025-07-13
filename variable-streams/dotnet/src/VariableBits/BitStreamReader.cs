using System;
using System.IO;
using System.Numerics;

namespace VariableBits
{
    /// <summary>
    /// A reader that allows reading individual bits from an underlying stream.
    /// </summary>
    public class BitStreamReader : IDisposable
    {
        /// <summary>
        /// The underlying reader.
        /// </summary>
        private readonly Stream _stream;

        /// <summary>
        /// Buffer for reading from the underlying stream.
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
        /// Number of valid bytes in the buffer.
        /// </summary>
        private int _bufferSize;

        /// <summary>
        /// Whether the end of the underlying stream has been reached.
        /// </summary>
        private bool _eof;

        /// <summary>
        /// Default buffer size.
        /// </summary>
        private const int DefaultBufferSize = 4096;

        /// <summary>
        /// Creates a new BitStreamReader with the specified stream.
        /// </summary>
        /// <param name="stream">The underlying stream to read from.</param>
        public BitStreamReader(Stream stream) : this(stream, DefaultBufferSize)
        {
        }

        /// <summary>
        /// Creates a new BitStreamReader with the specified stream and buffer capacity.
        /// </summary>
        /// <param name="stream">The underlying stream to read from.</param>
        /// <param name="capacity">The buffer capacity.</param>
        public BitStreamReader(Stream stream, int capacity)
        {
            _stream = stream ?? throw new ArgumentNullException(nameof(stream));
            _buffer = new byte[capacity];
            _bytePos = 0;
            _bitPos = 0;
            _bufferSize = 0;
            _eof = false;
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

            ulong result = 0;
            int bitsRead = 0;

            while (bitsRead < bitCount)
            {
                // If we've reached the end of the buffer, load more data
                if (_bytePos >= _bufferSize)
                {
                    LoadMoreData();
                    if (_bufferSize == 0)
                    {
                        if (bitsRead == 0)
                        {
                            throw BitStreamException.EndOfStream();
                        }
                        break;
                    }
                }

                // Calculate how many bits we can read from the current byte
                int bitsAvailable = 8 - _bitPos;
                int bitsToRead = Math.Min(bitsAvailable, bitCount - bitsRead);

                // Read bits LSB-first
                for (int i = 0; i < bitsToRead; i++)
                {
                    int bitIndex = _bitPos + i;
                    byte mask = (byte)(1 << bitIndex);
                    bool bit = (_buffer[_bytePos] & mask) != 0;
                    if (bit)
                    {
                        result |= (ulong)1 << bitsRead;
                    }
                    bitsRead++;
                }

                // Update position
                _bitPos += (byte)bitsToRead;
                if (_bitPos == 8)
                {
                    _bytePos++;
                    _bitPos = 0;
                }
            }

            return result;
        }

        /// <summary>
        /// Fills the buffer with data from the underlying stream.
        /// </summary>
        /// <returns>True if at least one byte was read, false otherwise.</returns>
        /// <exception cref="BitStreamException">If an I/O error occurs.</exception>
        private bool FillBuffer()
        {
            // If we've already reached the end of the stream, return false
            if (_eof)
            {
                return false;
            }

            try
            {
                // Reset the buffer position
                _bytePos = 0;
                _bitPos = 0;

                // Read data into the buffer
                _bufferSize = _stream.Read(_buffer, 0, _buffer.Length);

                // If we didn't read any bytes, we've reached the end of the stream
                if (_bufferSize == 0)
                {
                    _eof = true;
                    return false;
                }

                return true;
            }
            catch (IOException ex)
            {
                throw BitStreamException.FromIOException(ex);
            }
        }

        /// <summary>
        /// Loads more data into the buffer.
        /// </summary>
        /// <exception cref="BitStreamException">If an I/O error occurs.</exception>
        private void LoadMoreData()
        {
            FillBuffer();
        }

        /// <summary>
        /// Returns true if the end of the stream has been reached.
        /// </summary>
        public bool IsEof => _eof && _bytePos >= _bufferSize;

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

            if (bitCount <= 64)
            {
                return new BigInteger(ReadBits(bitCount));
            }

            // For bit counts > 64, read the lower bits first (LSB), then the higher bits
            ulong low = ReadBits(64);
            ulong high = ReadBits((byte)(bitCount - 64));
            return (new BigInteger(high) << 64) | new BigInteger(low);
        }

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
        /// Disposes the BitStreamReader and the underlying stream.
        /// </summary>
        public void Dispose()
        {
            _stream.Dispose();
        }
    }
}
