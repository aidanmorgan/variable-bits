using System;
using System.IO;
using Xunit;
using VariableBits;
using System.Numerics;

namespace VariableBits.Tests
{
    public class BitStreamWriterTests
    {
        [Fact]
        public void TestBitStreamWriter()
        {
            // Generate expected bytes using LSB order writer
            byte[] expected;
            using (var expectedStream = new MemoryStream())
            using (var writer = new BitStreamWriter(expectedStream))
            {
                writer.WriteBits(0b1, 1);
                writer.WriteBits(0b010, 3);
                writer.WriteBits(0b1010, 4);
                writer.WriteBits(0b11110000, 8);
                writer.WriteBits(0b00001111, 8);
                writer.Flush();
                expected = expectedStream.ToArray();
            }

            var memoryStream = new MemoryStream();
            using (var writer = new BitStreamWriter(memoryStream))
            {
                writer.WriteBits(0b1, 1);
                writer.WriteBits(0b010, 3);
                writer.WriteBits(0b1010, 4);
                writer.WriteBits(0b11110000, 8);
                writer.WriteBits(0b00001111, 8);
                writer.Flush();
            }
            byte[] data = memoryStream.ToArray();
            Assert.Equal(expected, data);
        }

        [Fact]
        public void TestBitStreamWriterWithLargeData()
        {
            // Generate expected bytes using LSB order writer
            byte[] expected;
            using (var expectedStream = new MemoryStream())
            using (var writer = new BitStreamWriter(expectedStream))
            {
                for (int i = 0; i < 1000; i++)
                {
                    writer.WriteBits((byte)(i % 256), 8);
                }
                writer.WriteBits(0b101, 3);
                writer.WriteBits(0b01010, 5);
                for (int i = 0; i < 100; i++)
                {
                    writer.WriteBits((byte)(i % 256), 8);
                }
                writer.Flush();
                expected = expectedStream.ToArray();
            }

            byte[] data;
            using (var memoryStream = new MemoryStream())
            using (var writer = new BitStreamWriter(memoryStream))
            {
                for (int i = 0; i < 1000; i++)
                {
                    writer.WriteBits((byte)(i % 256), 8);
                }
                writer.WriteBits(0b101, 3);
                writer.WriteBits(0b01010, 5);
                for (int i = 0; i < 100; i++)
                {
                    writer.WriteBits((byte)(i % 256), 8);
                }
                writer.Flush();
                data = memoryStream.ToArray();
            }
            Assert.Equal(expected, data);
        }

        [Fact]
        public void TestBitStreamWriterErrorHandling()
        {
            using var memoryStream = new MemoryStream();
            using var writer = new BitStreamWriter(memoryStream);
            Assert.Throws<BitStreamException>(() => writer.WriteBits(0, 0));
            Assert.Throws<BitStreamException>(() => writer.WriteBits(0, 65));
        }

        [Fact]
        public void TestBitStreamWriterWithBitValue()
        {
            // Generate expected bytes using LSB order writer
            byte[] expected;
            using (var expectedStream = new MemoryStream())
            using (var writer = new BitStreamWriter(expectedStream))
            {
                var expectedU8Value = BitValue.New(0xAA, 8);
                var expectedU16Value = BitValue.New(0xBBCC, 16);
                var expectedU32Value = BitValue.New(0xDDEEFF, 24);
                writer.WriteBitValue(expectedU8Value);
                writer.WriteBitValue(expectedU16Value);
                writer.WriteBitValue(expectedU32Value);
                writer.Flush();
                expected = expectedStream.ToArray();
            }

            byte[] data;
            using (var memoryStream = new MemoryStream())
            using (var writer = new BitStreamWriter(memoryStream))
            {
                var u8Value = BitValue.New(0xAA, 8);
                var u16Value = BitValue.New(0xBBCC, 16);
                var u32Value = BitValue.New(0xDDEEFF, 24);
                writer.WriteBitValue(u8Value);
                writer.WriteBitValue(u16Value);
                writer.WriteBitValue(u32Value);
                writer.Flush();
                data = memoryStream.ToArray();
            }
            Assert.Equal(expected, data);
        }

        [Fact]
        public void TestBitStreamWriterU128Operations()
        {
            // Generate expected bytes using LSB order writer
            byte[] expected;
            using (var expectedStream = new MemoryStream())
            using (var writer = new BitStreamWriter(expectedStream))
            {
                var expectedValue1 = new BigInteger(0x1234567890ABCDEF) << 64 | new BigInteger(0xFEDCBA0987654321);
                var expectedValue2 = new BigInteger(0xABCDEF0123456789) << 64 | new BigInteger(0x9876543210FEDCBA);
                writer.WriteBitsU128(expectedValue1, 128);
                writer.WriteBitsU128(expectedValue2, 128);
                writer.Flush();
                expected = expectedStream.ToArray();
            }

            var memoryStream = new MemoryStream();
            using (var writer = new BitStreamWriter(memoryStream))
            {
                var value1 = new BigInteger(0x1234567890ABCDEF) << 64 | new BigInteger(0xFEDCBA0987654321);
                var value2 = new BigInteger(0xABCDEF0123456789) << 64 | new BigInteger(0x9876543210FEDCBA);
                writer.WriteBitsU128(value1, 128);
                writer.WriteBitsU128(value2, 128);
                writer.Flush();
            }
            byte[] data = memoryStream.ToArray();
            Assert.Equal(expected, data);
        }

        [Fact]
        public void TestBitStreamWriterPartialU128Operations()
        {
            // Generate expected bytes using LSB order writer
            byte[] expected;
            using (var expectedStream = new MemoryStream())
            using (var writer = new BitStreamWriter(expectedStream))
            {
                var expectedValue = new BigInteger(0x1234567890ABCDEF) << 64 | new BigInteger(0xFEDCBA0987654321);
                writer.WriteBitsU128(expectedValue, 64);
                writer.WriteBitsU128(expectedValue, 32);
                writer.WriteBitsU128(expectedValue, 24);
                writer.Flush();
                expected = expectedStream.ToArray();
            }

            var memoryStream = new MemoryStream();
            using (var writer = new BitStreamWriter(memoryStream))
            {
                var value = new BigInteger(0x1234567890ABCDEF) << 64 | new BigInteger(0xFEDCBA0987654321);
                writer.WriteBitsU128(value, 64);
                writer.WriteBitsU128(value, 32);
                writer.WriteBitsU128(value, 24);
                writer.Flush();
            }
            byte[] data = memoryStream.ToArray();
            Assert.Equal(expected, data);
        }

        [Fact]
        public void TestBitStreamReaderWriterIntegration()
        {
            byte[] writtenData;
            using (var memoryStream = new MemoryStream())
            using (var writer = new BitStreamWriter(memoryStream))
            {
                writer.WriteBits(0b101, 3);
                writer.WriteBits(0b11110000, 8);
                writer.WriteBits(0xABCD, 16);
                writer.Flush();
                writtenData = memoryStream.ToArray();
            }

            using (var readMemoryStream = new MemoryStream(writtenData))
            using (var reader = new BitStreamReader(readMemoryStream))
            {
                Assert.Equal(0b101UL, reader.ReadBits(3));
                Assert.Equal(0b11110000UL, reader.ReadBits(8));
                Assert.Equal(0xABCDUL, reader.ReadBits(16));
                Assert.True(reader.IsEof);
            }
        }

        [Fact]
        public void TestBitStreamWriterReader_LSBOrder128Bit()
        {
            var value = (new BigInteger(0x0123456789ABCDEF) << 64) | new BigInteger(0xFEDCBA9876543210);
            
            byte[] writtenData;
            using (var memoryStream = new MemoryStream())
            using (var writer = new BitStreamWriter(memoryStream))
            {
                writer.WriteBitsU128(value, 128);
                writer.Flush();
                writtenData = memoryStream.ToArray();
            }

            using (var readMemoryStream = new MemoryStream(writtenData))
            using (var reader = new BitStreamReader(readMemoryStream))
            {
                var readValue = reader.ReadBitsU128(128);
                Assert.Equal(value, readValue);
            }
        }

        [Fact]
        public void TestLSBOrderAllBitLengths()
        {
            // Test LSB order for all bit lengths from 1 to 64
            for (byte bitCount = 1; bitCount <= 64; bitCount++)
            {
                ulong testValue = (1UL << (bitCount - 1)) | 1UL; // Set MSB and LSB
                
                byte[] writtenData;
                using (var memoryStream = new MemoryStream())
                using (var writer = new BitStreamWriter(memoryStream))
                {
                    writer.WriteBits(testValue, bitCount);
                    writer.Flush();
                    writtenData = memoryStream.ToArray();
                }

                using (var readMemoryStream = new MemoryStream(writtenData))
                using (var reader = new BitStreamReader(readMemoryStream))
                {
                    var readValue = reader.ReadBits(bitCount);
                    Assert.Equal(testValue, readValue);
                }
            }
            // Test 128-bit values
            for (byte bitCount = 65; bitCount <= 128; bitCount++)
            {
                var testValue = (BigInteger.One << (bitCount - 1)) | BigInteger.One; // Set MSB and LSB
                
                byte[] writtenData;
                using (var memoryStream = new MemoryStream())
                using (var writer = new BitStreamWriter(memoryStream))
                {
                    writer.WriteBitsU128(testValue, bitCount);
                    writer.Flush();
                    writtenData = memoryStream.ToArray();
                }

                using (var readMemoryStream = new MemoryStream(writtenData))
                using (var reader = new BitStreamReader(readMemoryStream))
                {
                    var readValue = reader.ReadBitsU128(bitCount);
                    Assert.Equal(testValue, readValue);
                }
            }
        }
    }
}