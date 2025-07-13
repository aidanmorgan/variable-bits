using System;
using System.IO;
using System.Numerics;
using Xunit;

namespace VariableBits.Tests
{
    public class BitStreamReaderTests
    {
        [Fact]
        public void TestLSBOrderBehavior()
        {
            var value = 0b10101010UL;

            byte[] writtenData;
            
            using (var memoryStream = new MemoryStream())
            using (var writer = new BitStreamWriter(memoryStream))
            {
                writer.WriteBits(value, 8); // 0xAA
                writer.Flush();

                writtenData = memoryStream.ToArray();
            }

            using (var readMemoryStream = new MemoryStream(writtenData))
            using (var reader = new BitStreamReader(readMemoryStream))
            {
                var readValue = reader.ReadBits(8);
                Assert.Equal(value, readValue);
            }
        }

        [Fact]
        public void TestBitStreamReader()
        {
            ulong[] values = { 0b1UL, 0b010UL, 0b1010UL, 0b11110000UL, 0b00001111UL };
            byte[] bitCounts = { 1, 3, 4, 8, 8 };
            
            byte[] writtenData;
            using (var memoryStream = new MemoryStream())
            using (var writer = new BitStreamWriter(memoryStream))
            {
                for (var i = 0; i < values.Length; i++)
                    writer.WriteBits(values[i], bitCounts[i]);
                writer.Flush();
                writtenData = memoryStream.ToArray();
            }

            using (var readMemoryStream = new MemoryStream(writtenData))
            using (var reader = new BitStreamReader(readMemoryStream))
            {
                for (var i = 0; i < values.Length; i++)
                    Assert.Equal(values[i], reader.ReadBits(bitCounts[i]));
                Assert.Throws<BitStreamException>(() => reader.ReadBits(1));
                Assert.True(reader.IsEof);
            }
        }

        [Fact]
        public void TestBitStreamReaderWithLargeBuffer()
        {
            byte[] writtenData;
            using (var memoryStream = new MemoryStream())
            using (var writer = new BitStreamWriter(memoryStream))
            {
                for (var i = 0; i < 1000; i++)
                    writer.WriteBits((byte)(i % 256), 8);
                writer.WriteBits(0b101, 3);
                writer.WriteBits(0b01010, 5);
                for (var i = 1000; i < 1100; i++)
                    writer.WriteBits((byte)(i % 256), 8);
                writer.Flush();
                writtenData = memoryStream.ToArray();
            }

            using (var readMemoryStream = new MemoryStream(writtenData))
            using (var reader = new BitStreamReader(readMemoryStream, 1024))
            {
                for (var i = 0; i < 1000; i++)
                    Assert.Equal((byte)(i % 256), reader.ReadBits(8));
                Assert.Equal(0b101UL, reader.ReadBits(3));
                Assert.Equal(0b01010UL, reader.ReadBits(5));
                for (var i = 1000; i < 1100; i++)
                    Assert.Equal((byte)(i % 256), reader.ReadBits(8));
            }
        }

        [Fact]
        public void TestBitStreamReaderErrorHandling()
        {
            byte[] writtenData;
            using (var memoryStream = new MemoryStream())
            using (var writer = new BitStreamWriter(memoryStream))
            {
                writer.WriteBits(0b10101010, 8);
                writer.Flush();
                writtenData = memoryStream.ToArray();
            }

            using (var readMemoryStream = new MemoryStream(writtenData))
            using (var reader = new BitStreamReader(readMemoryStream))
            {
                Assert.Throws<BitStreamException>(() => reader.ReadBits(0));
                Assert.Throws<BitStreamException>(() => reader.ReadBits(65));
                reader.ReadBits(8);
                Assert.Throws<BitStreamException>(() => reader.ReadBits(1));
                Assert.True(reader.IsEof);
            }
        }

        [Fact]
        public void TestBitStreamReaderWithBitValue()
        {
            var u8Value = BitValue.New(0xAA, 8);
            var u16Value = BitValue.New(0xBBCC, 16);
            var u32Value = BitValue.New(0xDDEEFF, 24);
            
            byte[] writtenData;
            using (var memoryStream = new MemoryStream())
            using (var writer = new BitStreamWriter(memoryStream))
            {
                writer.WriteBitValue(u8Value);
                writer.WriteBitValue(u16Value);
                writer.WriteBitValue(u32Value);
                writer.Flush();
                writtenData = memoryStream.ToArray();
            }

            using (var readMemoryStream = new MemoryStream(writtenData))
            using (var reader = new BitStreamReader(readMemoryStream))
            {
                Assert.Equal(u8Value.ToUInt64(), reader.ReadBitValue(8).ToUInt64());
                Assert.Equal(u16Value.ToUInt64(), reader.ReadBitValue(16).ToUInt64());
                Assert.Equal(u32Value.ToUInt64(), reader.ReadBitValue(24).ToUInt64());
            }
        }

        [Fact]
        public void TestBitStreamReaderU128Operations()
        {
            var value1 = (new BigInteger(0x1234567890ABCDEF) << 64) | new BigInteger(0xFEDCBA0987654321);
            var value2 = (new BigInteger(0xABCDEF0123456789) << 64) | new BigInteger(0x9876543210FEDCBA);
            
            byte[] writtenData;
            using (var memoryStream = new MemoryStream())
            using (var writer = new BitStreamWriter(memoryStream))
            {
                writer.WriteBitsU128(value1, 128);
                writer.WriteBitsU128(value2, 128);
                writer.Flush();
                writtenData = memoryStream.ToArray();
            }

            using (var readMemoryStream = new MemoryStream(writtenData))
            using (var reader = new BitStreamReader(readMemoryStream))
            {
                Assert.Equal(value1, reader.ReadBitsU128(128));
                Assert.Equal(value2, reader.ReadBitsU128(128));
            }
        }

        [Fact]
        public void TestBitStreamReaderPartialU128Operations()
        {
            var value = (new BigInteger(0x1234567890ABCDEF) << 64) | new BigInteger(0xFEDCBA0987654321);
            
            byte[] writtenData;
            using (var memoryStream = new MemoryStream())
            using (var writer = new BitStreamWriter(memoryStream))
            {
                writer.WriteBitsU128(value, 64);
                writer.WriteBitsU128(value, 32);
                writer.WriteBitsU128(value, 24);
                writer.Flush();
                writtenData = memoryStream.ToArray();
            }

            using (var readMemoryStream = new MemoryStream(writtenData))
            using (var reader = new BitStreamReader(readMemoryStream))
            {
                Assert.Equal((ulong)(value & ((BigInteger.One << 64) - 1)), reader.ReadBitsU128(64));
                Assert.Equal((ulong)(value & ((BigInteger.One << 32) - 1)), reader.ReadBitsU128(32));
                Assert.Equal((ulong)(value & ((BigInteger.One << 24) - 1)), reader.ReadBitsU128(24));
            }
        }
    }
}