package com.aidanjmorgan.variablebits;

import java.io.IOException;
import java.io.InputStream;
import java.math.BigInteger;

/**
 * A reader that allows reading individual bits from an underlying stream.
 */
public class BitStreamReader implements AutoCloseable {

    /**
     * The underlying input stream.
     */
    private final InputStream inputStream;

    /**
     * Buffer for reading from the underlying stream.
     */
    private final byte[] buffer;

    /**
     * Current byte position in the buffer.
     */
    private int bytePos;

    /**
     * Current bit position within the current byte (0-7).
     */
    private byte bitPos;

    /**
     * Number of valid bytes in the buffer.
     */
    private int bufferSize;

    /**
     * Whether the end of the underlying stream has been reached.
     */
    private boolean eof;

    /**
     * Default buffer size.
     */
    private static final int DEFAULT_BUFFER_SIZE = 4096;

    /**
     * Creates a new BitStreamReader with the specified stream.
     *
     * @param inputStream The underlying stream to read from.
     */
    public BitStreamReader(InputStream inputStream) {
        this(inputStream, DEFAULT_BUFFER_SIZE);
    }

    /**
     * Creates a new BitStreamReader with the specified stream and buffer capacity.
     *
     * @param inputStream The underlying stream to read from.
     * @param capacity The buffer capacity.
     */
    public BitStreamReader(InputStream inputStream, int capacity) {
        if (inputStream == null) {
            throw new NullPointerException("Input stream cannot be null");
        }
        this.inputStream = inputStream;
        this.buffer = new byte[capacity];
        this.bytePos = 0;
        this.bitPos = 0;
        this.bufferSize = 0;
        this.eof = false;
    }

    /**
     * Reads up to 64 bits from the stream.
     *
     * @param bitCount The number of bits to read (1-64).
     * @return The read bits as a 64-bit unsigned long.
     * @throws BitStreamException If the bit count is invalid or the end of stream is reached.
     */
    public long readBits(byte bitCount) {
        if (bitCount <= 0 || bitCount > 64) {
            throw BitStreamException.invalidBitCount();
        }

        long result = 0;
        int bitsRead = 0;

        // Read bits until we have read the requested number
        while (bitsRead < bitCount) {
            // If we've reached the end of the buffer, try to fill it
            if (bytePos >= bufferSize) {
                if (!fillBuffer()) {
                    throw BitStreamException.endOfStream();
                }
            }

            // Get the current byte
            byte currentByte = buffer[bytePos];

            // Calculate how many bits we can read from the current byte
            int bitsAvailable = 8 - bitPos;
            int bitsToRead = Math.min(bitsAvailable, bitCount - bitsRead);

            // Create a mask for the bits we want to read (LSB first)
            byte mask = (byte)((1 << bitsToRead) - 1);

            // Shift the mask to the correct position (LSB first)
            mask = (byte)(mask << bitPos);

            // Read the bits (LSB first)
            byte readBits = (byte)((currentByte & mask) >> bitPos);

            // Add the bits to the result (LSB first)
            result |= (long)readBits << bitsRead;

            // Update the bit position
            bitsRead += bitsToRead;
            bitPos += bitsToRead;

            // If we've read all bits in the current byte, move to the next byte
            if (bitPos == 8) {
                bytePos++;
                bitPos = 0;
            }
        }

        return result;
    }

    /**
     * Fills the buffer with data from the underlying stream.
     *
     * @return True if at least one byte was read, false otherwise.
     * @throws BitStreamException If an I/O error occurs.
     */
    private boolean fillBuffer() {
        // If we've already reached the end of the stream, return false
        if (eof) {
            return false;
        }

        try {
            // Reset the buffer position
            bytePos = 0;
            bitPos = 0;

            // Read data into the buffer
            bufferSize = inputStream.read(buffer, 0, buffer.length);

            // If we didn't read any bytes, we've reached the end of the stream
            if (bufferSize == 0 || bufferSize == -1) {
                eof = true;
                return false;
            }

            return true;
        } catch (IOException e) {
            throw BitStreamException.fromIOException(e);
        }
    }

    /**
     * Returns true if the end of the stream has been reached.
     *
     * @return True if at the end of the stream, false otherwise.
     */
    public boolean isEof() {
        return eof && bytePos >= bufferSize;
    }

    /**
     * Reads up to 128 bits from the stream.
     *
     * @param bitCount The number of bits to read (1-128).
     * @return The read bits as a 128-bit unsigned BigInteger.
     * @throws BitStreamException If the bit count is invalid or the end of stream is reached.
     */
    public BigInteger readBitsU128(byte bitCount) {
        if (bitCount <= 0 || bitCount > 128) {
            throw BitStreamException.invalidBitCount();
        }

        // For bit counts up to 64, we can use the existing method
        if (bitCount <= 64) {
            long value = readBits(bitCount);
            if (value < 0) {
                // Convert negative long to unsigned BigInteger
                return BigInteger.valueOf(value & Long.MAX_VALUE).setBit(63);
            } else {
                return BigInteger.valueOf(value);
            }
        }

        // For bit counts > 64, read the low 64 bits first (LSB), then the high bits
        long lowBits = readBits((byte)64);
        byte highBitCount = (byte)(bitCount - 64);
        long highBits = readBits(highBitCount);
        if (highBitCount < 64) {
            highBits = highBits & ((1L << highBitCount) - 1);
        }
        
        // For LSB order: low bits are read first, then high bits
        // Reconstruct as: (high << 64) | low
        BigInteger highValue = BigInteger.valueOf(highBits).shiftLeft(64);
        BigInteger lowValue = BigInteger.valueOf(lowBits);
        return highValue.or(lowValue);
    }

    /**
     * Reads up to 128 bits from the stream and returns a BitValue.
     *
     * @param bitCount The number of bits to read (1-128).
     * @return The read bits as a BitValue.
     * @throws BitStreamException If the bit count is invalid or the end of stream is reached.
     */
    public BitValue readBitValue(byte bitCount) {
        if (bitCount <= 64) {
            long value = readBits(bitCount);
            return BitValue.newValue(value, bitCount);
        } else {
            BigInteger value = readBitsU128(bitCount);
            return BitValue.newBigIntegerValue(value, bitCount, false);
        }
    }

    /**
     * Closes the BitStreamReader and the underlying stream.
     *
     * @throws IOException If an I/O error occurs.
     */
    @Override
    public void close() throws IOException {
        inputStream.close();
    }
}
