package com.aidanjmorgan.variablebits;

import java.io.IOException;
import java.io.OutputStream;
import java.math.BigInteger;

/**
 * A writer that allows writing individual bits to an underlying stream.
 */
public class BitStreamWriter implements AutoCloseable {

    /**
     * The underlying output stream.
     */
    private final OutputStream outputStream;

    /**
     * Buffer for writing to the underlying stream.
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
     * Default buffer size.
     */
    private static final int DEFAULT_BUFFER_SIZE = 4096;

    /**
     * Creates a new BitStreamWriter with the specified stream.
     *
     * @param outputStream The underlying stream to write to.
     */
    public BitStreamWriter(OutputStream outputStream) {
        this(outputStream, DEFAULT_BUFFER_SIZE);
    }

    /**
     * Creates a new BitStreamWriter with the specified stream and buffer capacity.
     *
     * @param outputStream The underlying stream to write to.
     * @param capacity The buffer capacity.
     */
    public BitStreamWriter(OutputStream outputStream, int capacity) {
        if (outputStream == null) {
            throw new NullPointerException("Output stream cannot be null");
        }
        this.outputStream = outputStream;
        this.buffer = new byte[capacity];
        this.bytePos = 0;
        this.bitPos = 0;
    }

    /**
     * Writes up to 64 bits to the stream.
     *
     * @param value The value to write.
     * @param bitCount The number of bits to write (1-64).
     * @throws BitStreamException If the bit count is invalid or an I/O error occurs.
     */
    public void writeBits(long value, byte bitCount) {
        if (bitCount <= 0 || bitCount > 64) {
            throw BitStreamException.invalidBitCount();
        }

        // Mask the value to ensure it only contains the specified number of bits
        long maskedValue = bitCount == 64 
            ? value 
            : value & ((1L << bitCount) - 1);

        int bitsWritten = 0;

        // Write bits until we have written the requested number
        while (bitsWritten < bitCount) {
            // If we've filled the buffer, flush it
            if (bytePos >= buffer.length) {
                flushBuffer();
            }

            // Calculate how many bits we can write to the current byte
            int bitsAvailable = 8 - bitPos;
            int bitsToWrite = Math.min(bitsAvailable, bitCount - bitsWritten);

            // Create a mask for the bits we want to write (LSB first)
            byte mask = (byte)((1 << bitsToWrite) - 1);

            // Get the bits to write (LSB first) - extract from least significant bit
            byte bitsToWriteValue = (byte)((maskedValue >> bitsWritten) & mask);

            // Shift the bits to the correct position within the byte (LSB first)
            bitsToWriteValue = (byte)(bitsToWriteValue << bitPos);

            // Create a mask for the bits we want to preserve
            byte preserveMask = (byte)~(mask << bitPos);

            // Update the byte in the buffer
            buffer[bytePos] = (byte)((buffer[bytePos] & preserveMask) | bitsToWriteValue);

            // Update the bit position
            bitsWritten += bitsToWrite;
            bitPos += bitsToWrite;

            // If we've filled the current byte, move to the next byte
            if (bitPos == 8) {
                bytePos++;
                bitPos = 0;
            }
        }
    }

    /**
     * Flushes the buffer to the underlying stream.
     *
     * @throws BitStreamException If an I/O error occurs.
     */
    private void flushBuffer() {
        try {
            if (bytePos > 0) {
                // Write the buffer to the underlying stream
                outputStream.write(buffer, 0, bytePos);

                // Reset the buffer
                for (int i = 0; i < buffer.length; i++) {
                    buffer[i] = 0;
                }
                bytePos = 0;
                // Keep the bit position as it is
            }
        } catch (IOException e) {
            throw BitStreamException.fromIOException(e);
        }
    }

    /**
     * Flushes any remaining bits to the underlying stream.
     *
     * @throws BitStreamException If an I/O error occurs.
     */
    public void flush() {
        try {
            // If there are any bits in the current byte, write it
            if (bytePos > 0 || bitPos > 0) {
                // Write the buffer to the stream
                outputStream.write(buffer, 0, bytePos + (bitPos > 0 ? 1 : 0));

                // Reset the buffer
                for (int i = 0; i < buffer.length; i++) {
                    buffer[i] = 0;
                }
                bytePos = 0;
                bitPos = 0;
            }

            // Flush the underlying stream
            outputStream.flush();
        } catch (IOException e) {
            throw BitStreamException.fromIOException(e);
        }
    }

    /**
     * Writes up to 128 bits to the stream.
     *
     * @param value The value to write.
     * @param bitCount The number of bits to write (1-128).
     * @throws BitStreamException If the bit count is invalid or an I/O error occurs.
     */
    public void writeBitsU128(BigInteger value, byte bitCount) {
        if (bitCount <= 0 || bitCount > 128) {
            throw BitStreamException.invalidBitCount();
        }

        // For bit counts up to 64, we can use the existing method
        if (bitCount <= 64) {
            writeBits(value.longValue(), bitCount);
            return;
        }

        // For bit counts > 64, write the low 64 bits first (LSB), then the high bits
        long lowBits = value.and(BigInteger.valueOf(-1L)).longValue();
        writeBits(lowBits, (byte)64);
        long highBits = value.shiftRight(64).longValue();
        writeBits(highBits, (byte)(bitCount - 64));
    }

    /**
     * Writes a BitValue to the stream.
     *
     * @param value The BitValue to write.
     * @param bitCount Optional bit count override. If not provided, uses the BitValue's bit count.
     * @throws BitStreamException If the bit count is invalid or an I/O error occurs.
     */
    public void writeBitValue(BitValue value, Byte bitCount) {
        byte bitsToWrite = bitCount != null ? bitCount : value.getBitCount();

        if (bitsToWrite <= 64) {
            writeBits(value.toUInt64(), bitsToWrite);
        } else {
            writeBitsU128(value.toUnsignedBigInteger(), bitsToWrite);
        }
    }

    /**
     * Closes the BitStreamWriter, flushing any remaining bits and closing the underlying stream.
     *
     * @throws IOException If an I/O error occurs.
     */
    @Override
    public void close() throws IOException {
        flush();
        outputStream.close();
    }
}
