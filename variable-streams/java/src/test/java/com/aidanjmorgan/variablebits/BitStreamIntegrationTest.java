package com.variablebits;

import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.DisplayName;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;

import static org.junit.jupiter.api.Assertions.*;

/**
 * Integration tests for BitStreamReader and BitStreamWriter.
 */
public class BitStreamIntegrationTest {
    
    @Test
    @DisplayName("Test basic integration between reader and writer")
    public void testBasicIntegration() throws IOException {
        // Create a ByteArrayOutputStream to write to
        ByteArrayOutputStream outputStream = new ByteArrayOutputStream();
        
        // Write various bit patterns
        try (BitStreamWriter writer = new BitStreamWriter(outputStream)) {
            writer.writeBits(0b101, 3);
            writer.writeBits(0b11110000, 8);
            writer.writeBits(0xABCD, 16);
            
            // Flush to ensure all bits are written
            writer.flush();
        }
        
        // Get the written data
        byte[] data = outputStream.toByteArray();
        
        // Create a ByteArrayInputStream with the written data
        ByteArrayInputStream inputStream = new ByteArrayInputStream(data);
        
        // Read and verify the bit patterns
        try (BitStreamReader reader = new BitStreamReader(inputStream)) {
            assertEquals(0b101, reader.readBits(3));
            assertEquals(0b11110000, reader.readBits(8));
            assertEquals(0xABCD, reader.readBits(16));
            
            // We've read all the bits, so we should be at the end of the stream
            assertTrue(reader.isEof());
        }
    }
    
    @Test
    @DisplayName("Test integration with large data")
    public void testLargeDataIntegration() throws IOException {
        // Create a ByteArrayOutputStream to write to
        ByteArrayOutputStream outputStream = new ByteArrayOutputStream();
        
        // Write a large amount of data
        try (BitStreamWriter writer = new BitStreamWriter(outputStream)) {
            for (int i = 0; i < 1000; i++) {
                writer.writeBits(i % 256, (byte)8);
            }
            
            // Write some non-byte-aligned bits
            writer.writeBits(0b101, 3);
            writer.writeBits(0b01010, 5);
            
            // Write more data
            for (int i = 0; i < 100; i++) {
                writer.writeBits(i % 256, (byte)8);
            }
            
            // Flush to ensure all bits are written
            writer.flush();
        }
        
        // Get the written data
        byte[] data = outputStream.toByteArray();
        
        // Create a ByteArrayInputStream with the written data
        ByteArrayInputStream inputStream = new ByteArrayInputStream(data);
        
        // Read and verify the data
        try (BitStreamReader reader = new BitStreamReader(inputStream)) {
            // Read the first 1000 bytes
            for (int i = 0; i < 1000; i++) {
                assertEquals(i % 256, reader.readBits(8));
            }
            
            // Read the non-byte-aligned bits
            assertEquals(0b101, reader.readBits(3));
            assertEquals(0b01010, reader.readBits(5));
            
            // Read the remaining 100 bytes
            for (int i = 0; i < 100; i++) {
                assertEquals(i % 256, reader.readBits(8));
            }
            
            // We've read all the bits, so we should be at the end of the stream
            assertTrue(reader.isEof());
        }
    }
    
    @Test
    @DisplayName("Test integration with BitValue objects")
    public void testBitValueIntegration() throws IOException {
        // Create a ByteArrayOutputStream to write to
        ByteArrayOutputStream outputStream = new ByteArrayOutputStream();
        
        // Create BitValues
        BitValue u8Value = BitValue.newValue(0xAA, (byte)8);
        BitValue u16Value = BitValue.newValue(0xBBCC, (byte)16);
        BitValue u32Value = BitValue.newValue(0xDDEEFF, (byte)24);
        
        // Write BitValues
        try (BitStreamWriter writer = new BitStreamWriter(outputStream)) {
            writer.writeBitValue(u8Value, null);
            writer.writeBitValue(u16Value, null);
            writer.writeBitValue(u32Value, null);
            
            // Flush to ensure all bits are written
            writer.flush();
        }
        
        // Get the written data
        byte[] data = outputStream.toByteArray();
        
        // Create a ByteArrayInputStream with the written data
        ByteArrayInputStream inputStream = new ByteArrayInputStream(data);
        
        // Read and verify the BitValues
        try (BitStreamReader reader = new BitStreamReader(inputStream)) {
            BitValue readU8Value = reader.readBitValue((byte)8);
            BitValue readU16Value = reader.readBitValue((byte)16);
            BitValue readU32Value = reader.readBitValue((byte)24);
            
            // Verify the values
            assertEquals(u8Value.toUInt64(), readU8Value.toUInt64());
            assertEquals(u16Value.toUInt64(), readU16Value.toUInt64());
            assertEquals(u32Value.toUInt64(), readU32Value.toUInt64());
            
            // Verify the bit counts
            assertEquals(u8Value.getBitCount(), readU8Value.getBitCount());
            assertEquals(u16Value.getBitCount(), readU16Value.getBitCount());
            assertEquals(u32Value.getBitCount(), readU32Value.getBitCount());
            
            // We've read all the bits, so we should be at the end of the stream
            assertTrue(reader.isEof());
        }
    }
    
    @Test
    @DisplayName("Test integration with 128-bit values")
    public void testUInt128Integration() throws IOException {
        // Create a ByteArrayOutputStream to write to
        ByteArrayOutputStream outputStream = new ByteArrayOutputStream();
        
        // Create 128-bit values
        UInt128 value1 = new UInt128(0x1234567890ABCDEFL, 0xFEDCBA0987654321L);
        UInt128 value2 = new UInt128(0xABCDEF0123456789L, 0x9876543210FEDCBAL);
        
        // Write 128-bit values
        try (BitStreamWriter writer = new BitStreamWriter(outputStream)) {
            writer.writeBitsU128(value1, (byte)128);
            writer.writeBitsU128(value2, (byte)128);
            
            // Flush to ensure all bits are written
            writer.flush();
        }
        
        // Get the written data
        byte[] data = outputStream.toByteArray();
        
        // Create a ByteArrayInputStream with the written data
        ByteArrayInputStream inputStream = new ByteArrayInputStream(data);
        
        // Read and verify the 128-bit values
        try (BitStreamReader reader = new BitStreamReader(inputStream)) {
            UInt128 readValue1 = reader.readBitsU128((byte)128);
            UInt128 readValue2 = reader.readBitsU128((byte)128);
            
            // Verify the values
            assertEquals(value1.getHigh(), readValue1.getHigh());
            assertEquals(value1.getLow(), readValue1.getLow());
            assertEquals(value2.getHigh(), readValue2.getHigh());
            assertEquals(value2.getLow(), readValue2.getLow());
            
            // We've read all the bits, so we should be at the end of the stream
            assertTrue(reader.isEof());
        }
    }
    
    @Test
    @DisplayName("Test integration with BitStream")
    public void testBitStreamIntegration() {
        // Create a BitStream
        BitStream bitStream = new BitStream();
        
        // Write various bit patterns
        bitStream.writeBits(0b101, (byte)3);
        bitStream.writeBits(0b11110000, (byte)8);
        bitStream.writeBits(0xABCD, (byte)16);
        
        // Reset the position to the beginning
        bitStream.setPosition(0);
        
        // Read and verify the bit patterns
        assertEquals(0b101, bitStream.readBits((byte)3));
        assertEquals(0b11110000, bitStream.readBits((byte)8));
        assertEquals(0xABCD, bitStream.readBits((byte)16));
        
        // We've read all the bits, so we should be at the end of the stream
        assertTrue(bitStream.isEof());
    }
    
    @Test
    @DisplayName("Test integration with non-byte-aligned operations")
    public void testNonByteAlignedIntegration() throws IOException {
        // Create a ByteArrayOutputStream to write to
        ByteArrayOutputStream outputStream = new ByteArrayOutputStream();
        
        // Write various bit patterns with non-byte-aligned operations
        try (BitStreamWriter writer = new BitStreamWriter(outputStream)) {
            writer.writeBits(0b1, 1);
            writer.writeBits(0b10, 2);
            writer.writeBits(0b100, 3);
            writer.writeBits(0b1000, 4);
            writer.writeBits(0b10000, 5);
            writer.writeBits(0b100000, 6);
            writer.writeBits(0b1000000, 7);
            
            // Flush to ensure all bits are written
            writer.flush();
        }
        
        // Get the written data
        byte[] data = outputStream.toByteArray();
        
        // Create a ByteArrayInputStream with the written data
        ByteArrayInputStream inputStream = new ByteArrayInputStream(data);
        
        // Read and verify the bit patterns
        try (BitStreamReader reader = new BitStreamReader(inputStream)) {
            assertEquals(0b1, reader.readBits(1));
            assertEquals(0b10, reader.readBits(2));
            assertEquals(0b100, reader.readBits(3));
            assertEquals(0b1000, reader.readBits(4));
            assertEquals(0b10000, reader.readBits(5));
            assertEquals(0b100000, reader.readBits(6));
            assertEquals(0b1000000, reader.readBits(7));
            
            // We've read all the bits, so we should be at the end of the stream
            assertTrue(reader.isEof());
        }
    }
}