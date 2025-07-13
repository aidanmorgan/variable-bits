package com.aidanjmorgan.variablebits;

import java.math.BigInteger;
import java.util.ArrayList;
import java.util.List;

/**
 * A stream that allows reading and writing individual bits.
 */
public class BitStream {

    /**
     * The internal buffer storing the data.
     */
    private final List<Byte> buffer;

    /**
     * Current byte position in the buffer.
     */
    private int bytePos;

    /**
     * Current bit position within the current byte (0-7).
     */
    private byte bitPos;

    /**
     * Total number of bits in the stream.
     */
    private int bitLength;

    /**
     * Creates a new, empty BitStream.
     */
    public BitStream() {
        buffer = new ArrayList<>();
        bytePos = 0;
        bitPos = 0;
        bitLength = 0;
    }

    /**
     * Creates a new BitStream from an existing buffer.
     *
     * @param bytes The initial buffer content.
     */
    public BitStream(byte[] bytes) {
        buffer = new ArrayList<>(bytes.length);
        for (byte b : bytes) {
            buffer.add(b);
        }
        bytePos = 0;
        bitPos = 0;
        bitLength = buffer.size() * 8;
    }

    /**
     * Returns the current position in bits.
     *
     * @return The current position in bits.
     */
    public int getPosition() {
        return bytePos * 8 + bitPos;
    }

    /**
     * Sets the current position in bits.
     *
     * @param position The new position in bits.
     * @throws BitStreamException If the position is beyond the end of the stream.
     */
    public void setPosition(int position) {
        if (position > bitLength) {
            throw BitStreamException.endOfStream();
        }

        bytePos = position / 8;
        bitPos = (byte)(position % 8);
    }

    /**
     * Returns the total length of the stream in bits.
     *
     * @return The total length in bits.
     */
    public int getLength() {
        return bitLength;
    }

    /**
     * Returns true if the stream is empty.
     *
     * @return True if the stream is empty, false otherwise.
     */
    public boolean isEmpty() {
        return bitLength == 0;
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

        // Check if we have enough bits left
        if (getPosition() + bitCount > bitLength) {
            throw BitStreamException.endOfStream();
        }

        long result = 0;
        int bitsRead = 0;

        // Read bits until we have read the requested number
        while (bitsRead < bitCount) {
            // Ensure we have a valid byte to read from
            if (bytePos >= buffer.size()) {
                throw BitStreamException.endOfStream();
            }

            // Get the current byte
            byte currentByte = buffer.get(bytePos);

            // Calculate how many bits we can read from the current byte
            int bitsAvailable = 8 - bitPos;
            int bitsToRead = Math.min(bitsAvailable, bitCount - bitsRead);

            // Create a mask for the bits we want to read
            byte mask = (byte)((1 << bitsToRead) - 1);

            // Shift the mask to the correct position
            mask = (byte)(mask << bitPos);

            // Read the bits
            byte readBits = (byte)((currentByte & mask) >> bitPos);

            // Add the bits to the result
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

        // For bit counts > 64, we need to read in two parts
        long lowBits = readBits((byte)64);
        long highBits = readBits((byte)(bitCount - 64));

        // Convert high bits to BigInteger and shift left 64 bits
        BigInteger highValue;
        if (highBits < 0) {
            highValue = BigInteger.valueOf(highBits & Long.MAX_VALUE).setBit(63).shiftLeft(64);
        } else {
            highValue = BigInteger.valueOf(highBits).shiftLeft(64);
        }

        // Convert low bits to BigInteger
        BigInteger lowValue;
        if (lowBits < 0) {
            lowValue = BigInteger.valueOf(lowBits & Long.MAX_VALUE).setBit(63);
        } else {
            lowValue = BigInteger.valueOf(lowBits);
        }

        // Combine high and low bits
        return highValue.or(lowValue);
    }

    /**
     * Writes up to 64 bits to the stream.
     *
     * @param value The value to write.
     * @param bitCount The number of bits to write (1-64).
     * @throws BitStreamException If the bit count is invalid.
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
            // Ensure we have a byte to write to
            if (bytePos >= buffer.size()) {
                buffer.add((byte)0);
            }

            // Calculate how many bits we can write to the current byte
            int bitsAvailable = 8 - bitPos;
            int bitsToWrite = Math.min(bitsAvailable, bitCount - bitsWritten);

            // Create a mask for the bits we want to write
            byte mask = (byte)((1 << bitsToWrite) - 1);

            // Get the bits to write
            byte bitsToWriteValue = (byte)((maskedValue >> bitsWritten) & mask);

            // Shift the bits to the correct position
            bitsToWriteValue = (byte)(bitsToWriteValue << bitPos);

            // Create a mask for the bits we want to preserve
            byte preserveMask = (byte)~(mask << bitPos);

            // Update the byte in the buffer
            byte currentByte = buffer.get(bytePos);
            buffer.set(bytePos, (byte)((currentByte & preserveMask) | bitsToWriteValue));

            // Update the bit position
            bitsWritten += bitsToWrite;
            bitPos += bitsToWrite;

            // If we've filled the current byte, move to the next byte
            if (bitPos == 8) {
                bytePos++;
                bitPos = 0;
            }
        }

        // Update the bit length if we've written beyond the current end
        bitLength = Math.max(bitLength, getPosition());
    }

    /**
     * Writes up to 128 bits to the stream.
     *
     * @param value The value to write.
     * @param bitCount The number of bits to write (1-128).
     * @throws BitStreamException If the bit count is invalid.
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

        // For bit counts > 64, we need to write in two parts
        // First write the low 64 bits
        long lowBits = value.and(BigInteger.valueOf(-1L)).longValue();
        writeBits(lowBits, (byte)64);

        // Then write the high bits
        long highBits = value.shiftRight(64).longValue();
        writeBits(highBits, (byte)(bitCount - 64));
    }

    /**
     * Writes a BitValue to the stream.
     *
     * @param value The BitValue to write.
     * @param bitCount Optional bit count override. If not provided, uses the BitValue's bit count.
     * @throws BitStreamException If the bit count is invalid.
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
     * Converts the BitStream to a byte array.
     *
     * @return The content of the BitStream as a byte array.
     */
    public byte[] toByteArray() {
        byte[] result = new byte[buffer.size()];
        for (int i = 0; i < buffer.size(); i++) {
            result[i] = buffer.get(i);
        }
        return result;
    }

    /**
     * Resets the BitStream to its initial state.
     */
    public void reset() {
        buffer.clear();
        bytePos = 0;
        bitPos = 0;
        bitLength = 0;
    }

    /**
     * Returns true if the current position is at the end of the stream.
     *
     * @return True if at the end of the stream, false otherwise.
     */
    public boolean isEof() {
        return getPosition() >= bitLength;
    }
}
