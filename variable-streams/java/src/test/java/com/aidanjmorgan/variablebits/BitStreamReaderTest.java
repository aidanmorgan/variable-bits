package com.variablebits;

import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.DisplayName;

import java.io.ByteArrayInputStream;
import java.io.IOException;

import static org.junit.jupiter.api.Assertions.*;

/**
 * Tests for the BitStreamReader class.
 */
public class BitStreamReaderTest {
    
    @Test
    @DisplayName("Test basic bit reading functionality")
    public void testBasicBitReading() throws IOException {
        // Create test data
        byte[] data = new byte[] {
            (byte)0b10101010, 
            (byte)0b11110000, 
            (byte)0b00001111
        };
        
        // Create an input stream with the test data
        ByteArrayInputStream inputStream = new ByteArrayInputStream(data);
        
        // Create a BitStreamReader with the input stream
        try (BitStreamReader reader = new BitStreamReader(inputStream)) {
            // Read bits
            assertEquals(0b1, reader.readBits(1));
            assertEquals(0b010, reader.readBits(3));
            assertEquals(0b1010, reader.readBits(4));
            assertEquals(0b11110000, reader.readBits(8));
            assertEquals(0b00001111, reader.readBits(8));
            
            // We've read all the bits, so we should be at the end of the stream
            assertTrue(reader.isEof());
        }
    }
    
    @Test
    @DisplayName("Test reading large amounts of data")
    public void testLargeDataReading() throws IOException {
        // Create test data
        byte[] data = new byte[1101];
        for (int i = 0; i < 1000; i++) {
            data[i] = (byte)(i % 256);
        }
        
        // Set the 1000th byte to contain the 3-bit value 0b101 followed by the first 5 bits of 0b01010
        data[1000] = (byte)0b10101010;
        
        // Set the remaining bytes
        for (int i = 0; i < 100; i++) {
            data[1001 + i] = (byte)(i % 256);
        }
        
        // Create an input stream with the test data
        ByteArrayInputStream inputStream = new ByteArrayInputStream(data);
        
        // Create a BitStreamReader with the input stream and a custom buffer size
        try (BitStreamReader reader = new BitStreamReader(inputStream, 1024)) {
            // Read a large amount of data
            for (int i = 0; i < 1000; i++) {
                assertEquals(i % 256, reader.readBits(8));
            }
            
            // Read some non-byte-aligned bits
            assertEquals(0b101, reader.readBits(3));
            assertEquals(0b01010, reader.readBits(5));
            
            // Read more data
            for (int i = 0; i < 100; i++) {
                assertEquals(i % 256, reader.readBits(8));
            }
            
            // We've read all the bits, so we should be at the end of the stream
            assertTrue(reader.isEof());
        }
    }
    
    @Test
    @DisplayName("Test error handling for invalid bit counts")
    public void testErrorHandling() {
        // Create test data
        byte[] data = new byte[] { 0x00 };
        
        // Create an input stream with the test data
        ByteArrayInputStream inputStream = new ByteArrayInputStream(data);
        
        // Create a BitStreamReader with the input stream
        BitStreamReader reader = new BitStreamReader(inputStream);
        
        // Test invalid bit count
        assertThrows(BitStreamException.class, () -> reader.readBits((byte)0));
        assertThrows(BitStreamException.class, () -> reader.readBits((byte)65));
        
        // Verify the error type
        try {
            reader.readBits((byte)0);
            fail("Expected BitStreamException");
        } catch (BitStreamException e) {
            assertEquals(BitStreamException.BitStreamErrorType.INVALID_BIT_COUNT, e.getErrorType());
        }
    }
    
    @Test
    @DisplayName("Test end of stream handling")
    public void testEndOfStreamHandling() {
        // Create test data
        byte[] data = new byte[] { 0x01 };
        
        // Create an input stream with the test data
        ByteArrayInputStream inputStream = new ByteArrayInputStream(data);
        
        // Create a BitStreamReader with the input stream
        BitStreamReader reader = new BitStreamReader(inputStream);
        
        // Read 8 bits (the entire stream)
        assertEquals(1, reader.readBits(8));
        
        // Trying to read more should throw an exception
        assertThrows(BitStreamException.class, () -> reader.readBits(1));
        
        // Verify the error type
        try {
            reader.readBits(1);
            fail("Expected BitStreamException");
        } catch (BitStreamException e) {
            assertEquals(BitStreamException.BitStreamErrorType.END_OF_STREAM, e.getErrorType());
        }
    }
    
    @Test
    @DisplayName("Test reading BitValue objects")
    public void testBitValueReading() throws IOException {
        // Create test data
        byte[] data = new byte[] {
            (byte)0xAA,
            (byte)0xCC, (byte)0xBB,
            (byte)0xFF, (byte)0xEE, (byte)0xDD
        };
        
        // Create an input stream with the test data
        ByteArrayInputStream inputStream = new ByteArrayInputStream(data);
        
        // Create a BitStreamReader with the input stream
        try (BitStreamReader reader = new BitStreamReader(inputStream)) {
            // Read BitValues
            BitValue u8Value = reader.readBitValue((byte)8);
            BitValue u16Value = reader.readBitValue((byte)16);
            BitValue u24Value = reader.readBitValue((byte)24);
            
            // Verify the BitValues
            assertEquals(0xAA, u8Value.toUInt64());
            assertEquals(0xBBCC, u16Value.toUInt64());
            assertEquals(0xDDEEFF, u24Value.toUInt64());
            
            // Verify the bit counts
            assertEquals(8, u8Value.getBitCount());
            assertEquals(16, u16Value.getBitCount());
            assertEquals(24, u24Value.getBitCount());
        }
    }
    
    @Test
    @DisplayName("Test 128-bit operations")
    public void testUInt128Operations() throws IOException {
        // Create test data for two 128-bit values
        byte[] data = new byte[32];
        
        // First 128-bit value (little-endian)
        data[0] = (byte)0x21;
        data[1] = (byte)0x43;
        data[2] = (byte)0x65;
        data[3] = (byte)0x87;
        data[4] = (byte)0x09;
        data[5] = (byte)0xBA;
        data[6] = (byte)0xDC;
        data[7] = (byte)0xFE;
        data[8] = (byte)0xEF;
        data[9] = (byte)0xCD;
        data[10] = (byte)0xAB;
        data[11] = (byte)0x90;
        data[12] = (byte)0x78;
        data[13] = (byte)0x56;
        data[14] = (byte)0x34;
        data[15] = (byte)0x12;
        
        // Second 128-bit value (little-endian)
        data[16] = (byte)0xBA;
        data[17] = (byte)0xDC;
        data[18] = (byte)0xFE;
        data[19] = (byte)0x10;
        data[20] = (byte)0x32;
        data[21] = (byte)0x54;
        data[22] = (byte)0x76;
        data[23] = (byte)0x98;
        data[24] = (byte)0x89;
        data[25] = (byte)0x67;
        data[26] = (byte)0x45;
        data[27] = (byte)0x23;
        data[28] = (byte)0x01;
        data[29] = (byte)0xEF;
        data[30] = (byte)0xCD;
        data[31] = (byte)0xAB;
        
        // Create an input stream with the test data
        ByteArrayInputStream inputStream = new ByteArrayInputStream(data);
        
        // Create a BitStreamReader with the input stream
        try (BitStreamReader reader = new BitStreamReader(inputStream)) {
            // Read 128-bit values
            UInt128 value1 = reader.readBitsU128((byte)128);
            UInt128 value2 = reader.readBitsU128((byte)128);
            
            // Verify the values
            assertEquals(0x1234567890ABCDEFL, value1.getHigh());
            assertEquals(0xFEDCBA0987654321L, value1.getLow());
            
            assertEquals(0xABCDEF0123456789L, value2.getHigh());
            assertEquals(0x9876543210FEDCBAL, value2.getLow());
        }
    }
    
    @Test
    @DisplayName("Test partial 128-bit operations")
    public void testPartialUInt128Operations() throws IOException {
        // Create test data
        byte[] data = new byte[15];
        
        // First 64 bits
        data[0] = (byte)0x21;
        data[1] = (byte)0x43;
        data[2] = (byte)0x65;
        data[3] = (byte)0x87;
        data[4] = (byte)0x09;
        data[5] = (byte)0xBA;
        data[6] = (byte)0xDC;
        data[7] = (byte)0xFE;
        
        // Next 32 bits
        data[8] = (byte)0x21;
        data[9] = (byte)0x43;
        data[10] = (byte)0x65;
        data[11] = (byte)0x87;
        
        // Last 24 bits
        data[12] = (byte)0x21;
        data[13] = (byte)0x43;
        data[14] = (byte)0x65;
        
        // Create an input stream with the test data
        ByteArrayInputStream inputStream = new ByteArrayInputStream(data);
        
        // Create a BitStreamReader with the input stream
        try (BitStreamReader reader = new BitStreamReader(inputStream)) {
            // Read partial 128-bit values
            UInt128 value1 = reader.readBitsU128((byte)64);
            UInt128 value2 = reader.readBitsU128((byte)32);
            UInt128 value3 = reader.readBitsU128((byte)24);
            
            // Verify the values
            assertEquals(0L, value1.getHigh());
            assertEquals(0xFEDCBA0987654321L, value1.getLow());
            
            assertEquals(0L, value2.getHigh());
            assertEquals(0x87654321L, value2.getLow());
            
            assertEquals(0L, value3.getHigh());
            assertEquals(0x654321L, value3.getLow());
        }
    }
}