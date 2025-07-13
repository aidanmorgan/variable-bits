using System;
using System.IO;
using System.Linq;
using Xunit;
using VariableBits;
using System.Numerics;

namespace VariableBits.Tests
{
    public class BitStreamTests
    {
        [Fact]
        public void TestWriteAndReadBits()
        {
            var stream = new BitStream();
            stream.WriteBits(0b101, 3);
            stream.WriteBits(0b11110000, 8);
            stream.WriteBits(0xFFFF_FFFF_FFFF_FFFF, 64);
            stream.SetPosition(0);
            Assert.Equal(0b101UL, stream.ReadBits(3));
            Assert.Equal(0b11110000UL, stream.ReadBits(8));
            Assert.Equal(0xFFFF_FFFF_FFFF_FFFFUL, stream.ReadBits(64));
        }

        [Fact]
        public void TestNonByteAlignedOperations()
        {
            var stream = new BitStream();
            stream.WriteBits(0b1, 1);
            stream.WriteBits(0b10, 2);
            stream.WriteBits(0b111, 3);
            stream.SetPosition(0);
            Assert.Equal(0b1UL, stream.ReadBits(1));
            Assert.Equal(0b10UL, stream.ReadBits(2));
            Assert.Equal(0b111UL, stream.ReadBits(3));
        }

        [Fact]
        public void TestErrorHandling()
        {
            var stream = new BitStream();
            Assert.Throws<BitStreamException>(() => stream.ReadBits(0));
            Assert.Throws<BitStreamException>(() => stream.ReadBits(65));
            Assert.Throws<BitStreamException>(() => stream.WriteBits(0, 0));
            Assert.Throws<BitStreamException>(() => stream.WriteBits(0, 65));
            Assert.Throws<BitStreamException>(() => stream.ReadBits(1));
        }

        [Fact]
        public void TestBitValueWithStreams()
        {
            var stream = new BitStream();
            var u8Value = BitValue.New(0xAA, 8);
            var u16Value = BitValue.New(0xBBBB, 16);
            var u32Value = BitValue.New(0xCCCCCCCC, 32);
            stream.WriteBitValue(u8Value);
            stream.WriteBitValue(u16Value);
            stream.WriteBitValue(u32Value);
            stream.SetPosition(0);
            var readU8Value = stream.ReadBitValue(8);
            var readU16Value = stream.ReadBitValue(16);
            var readU32Value = stream.ReadBitValue(32);
            Assert.Equal(0xAAUL, readU8Value.ToUInt64());
            Assert.Equal(0xBBBBUL, readU16Value.ToUInt64());
            Assert.Equal(0xCCCCCCCCUL, readU32Value.ToUInt64());
        }

        [Fact]
        public void TestBitStreamPosition()
        {
            var stream = new BitStream();
            stream.WriteBits(0b101, 3);
            stream.WriteBits(0b11110000, 8);
            Assert.Equal(11, stream.Position);
            stream.SetPosition(3);
            Assert.Equal(3, stream.Position);
            Assert.Equal(0b11110000UL, stream.ReadBits(8));
            Assert.Equal(11, stream.Position);
            Assert.Throws<BitStreamException>(() => stream.SetPosition(12));
        }

        [Fact]
        public void TestBitStreamLength()
        {
            var stream = new BitStream();
            Assert.Equal(0, stream.Length);
            Assert.True(stream.IsEmpty);
            stream.WriteBits(0b101, 3);
            Assert.Equal(3, stream.Length);
            Assert.False(stream.IsEmpty);
            stream.WriteBits(0b11110000, 8);
            Assert.Equal(11, stream.Length);
            stream.SetPosition(0);
            Assert.Equal(11, stream.Length);
        }

        [Fact]
        public void TestBitStreamToArray()
        {
            var stream = new BitStream();
            stream.WriteBits(0b10101010, 8);
            stream.WriteBits(0b11110000, 8);
            byte[] array = stream.ToArray();
            byte[] expected = new byte[] { 85, 15 };
            Assert.Equal(expected, array);
        }

        [Fact]
        public void TestBitStreamReset()
        {
            var stream = new BitStream();
            stream.WriteBits(0b10101010, 8);
            stream.WriteBits(0b11110000, 8);
            stream.Reset();
            Assert.Equal(0, stream.Length);
            Assert.True(stream.IsEmpty);
            Assert.Equal(0, stream.Position);
        }

        [Fact]
        public void TestBitStreamIsEof()
        {
            var stream = new BitStream();
            stream.WriteBits(0b10101010, 8);
            Assert.False(stream.IsEof);
            stream.SetPosition(0);
            stream.ReadBits(8);
            Assert.True(stream.IsEof);
            stream.SetPosition(0);
            Assert.False(stream.IsEof);
        }

        [Fact]
        public void TestBitStreamU128Operations()
        {
            var stream = new BitStream();
            var value1 = new BigInteger(0x1234567890ABCDEF) << 64 | new BigInteger(0xFEDCBA0987654321);
            var value2 = new BigInteger(0xABCDEF0123456789) << 64 | new BigInteger(0x9876543210FEDCBA);
            stream.WriteBitsU128(value1, 128);
            stream.WriteBitsU128(value2, 128);
            stream.SetPosition(0);
            var readValue1 = stream.ReadBitsU128(128);
            var readValue2 = stream.ReadBitsU128(128);
            Assert.Equal(value1, readValue1);
            Assert.Equal(value2, readValue2);
        }

        [Fact]
        public void TestBitStreamPartialU128Operations()
        {
            var stream = new BitStream();
            var value = new BigInteger(0x1234567890ABCDEF) << 64 | new BigInteger(0xFEDCBA0987654321);
            stream.WriteBitsU128(value, 100);
            stream.SetPosition(0);
            var readValue = stream.ReadBitsU128(100);
            var mask = (BigInteger.One << 100) - BigInteger.One;
            var expectedValue = value & mask;
            Assert.Equal(expectedValue, readValue);
        }

        [Fact]
        public void TestLSBOrderAllBitLengths()
        {
            // Test LSB order for all bit lengths from 1 to 64
            for (byte bitCount = 1; bitCount <= 64; bitCount++)
            {
                ulong testValue = (1UL << (bitCount - 1)) | 1UL; // Set MSB and LSB
                var stream = new BitStream();
                stream.WriteBits(testValue, bitCount);
                stream.SetPosition(0);
                var readValue = stream.ReadBits(bitCount);
                Assert.Equal(testValue, readValue);
            }
            
            // Test 128-bit values
            for (byte bitCount = 65; bitCount <= 128; bitCount++)
            {
                var testValue = (BigInteger.One << (bitCount - 1)) | BigInteger.One; // Set MSB and LSB
                var stream = new BitStream();
                stream.WriteBitsU128(testValue, bitCount);
                stream.SetPosition(0);
                var readValue = stream.ReadBitsU128(bitCount);
                Assert.Equal(testValue, readValue);
            }
        }
    }
}