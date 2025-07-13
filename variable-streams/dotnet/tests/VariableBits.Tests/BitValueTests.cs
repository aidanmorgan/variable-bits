// All bit-level operations in BitStreamWriter/Reader are unsigned. If you need to write signed values, convert them to their unsigned bitwise representation first.

using System;
using Xunit;
using VariableBits;
using System.Numerics;

namespace VariableBits.Tests
{
    public class BitValueTests
    {
        [Fact]
        public void TestBitValueCreation()
        {
            // Test BitValue creation
            var u8Value = BitValue.New(0xFF, 8);
            var u16Value = BitValue.New(0xFFFF, 16);
            var u32Value = BitValue.New(0xFFFFFFFF, 32);
            var u64Value = BitValue.New(0xFFFFFFFFFFFFFFFF, 64);

            // Test conversion to ulong
            Assert.Equal(0xFFUL, u8Value.ToUInt64());
            Assert.Equal(0xFFFFUL, u16Value.ToUInt64());
            Assert.Equal(0xFFFFFFFFUL, u32Value.ToUInt64());
            Assert.Equal(0xFFFFFFFFFFFFFFFFUL, u64Value.ToUInt64());

            // Test bit count
            Assert.Equal(8, u8Value.BitCount);
            Assert.Equal(16, u16Value.BitCount);
            Assert.Equal(32, u32Value.BitCount);
            Assert.Equal(64, u64Value.BitCount);
        }

        [Fact]
        public void TestBitValueConversions()
        {
            // Test conversion from primitive types to BitValue
            BitValue byteValue = (byte)0xFF;
            BitValue ushortValue = (ushort)0xFFFF;
            BitValue uintValue = (uint)0xFFFFFFFF;
            BitValue ulongValue = 0xFFFFFFFFFFFFFFFFUL;

            // Test bit count
            Assert.Equal(8, byteValue.BitCount);
            Assert.Equal(16, ushortValue.BitCount);
            Assert.Equal(32, uintValue.BitCount);
            Assert.Equal(64, ulongValue.BitCount);

            // Test conversion from BitValue to primitive types
            byte byteFromBitValue = (byte)byteValue;
            ushort ushortFromBitValue = (ushort)ushortValue;
            uint uintFromBitValue = (uint)uintValue;
            ulong ulongFromBitValue = (ulong)ulongValue;

            Assert.Equal(0xFF, byteFromBitValue);
            Assert.Equal(0xFFFF, ushortFromBitValue);
            Assert.Equal(0xFFFFFFFF, uintFromBitValue);
            Assert.Equal(0xFFFFFFFFFFFFFFFF, ulongFromBitValue);
        }

        [Fact]
        public void TestBitValueInvalidBitCount()
        {
            // Test invalid bit counts
            Assert.Throws<BitStreamException>(() => BitValue.New(0, 0));
            Assert.Throws<BitStreamException>(() => BitValue.New(0, 65));
            Assert.Throws<BitStreamException>(() => BitValue.NewSigned(0, 0));
            Assert.Throws<BitStreamException>(() => BitValue.NewSigned(0, 65));
        }

        [Fact]
        public void TestBigIntegerOperations()
        {
            // Test BigInteger creation and operations
            var value1 = new BigInteger(0x1234567890ABCDEF) << 64 | new BigInteger(0xFEDCBA0987654321);
            var value2 = new BigInteger(1);

            // Test bitwise operations
            var andResult = value1 & value2;
            var orResult = value1 | value2;
            var xorResult = value1 ^ value2;
            var notResult = ~value1;
            var leftShiftResult = value2 << 64;
            var rightShiftResult = value1 >> 64;

            Assert.Equal(new BigInteger(1), andResult);
            Assert.Equal(value1, orResult);
            Assert.Equal(new BigInteger(0x1234567890ABCDEF) << 64 | new BigInteger(0xFEDCBA0987654320), xorResult);
            Assert.Equal(~value1, notResult);
            Assert.Equal(new BigInteger(1) << 64, leftShiftResult);
            Assert.Equal(new BigInteger(0x1234567890ABCDEF), rightShiftResult);

            // Test arithmetic operations
            var addResult = value1 + value2;
            var subResult = value1 - value2;

            Assert.Equal(value1 + 1, addResult);
            Assert.Equal(value1 - 1, subResult);

            // Test comparison operations
            Assert.True(value1 > value2);
            Assert.True(value1 >= value2);
            Assert.False(value1 < value2);
            Assert.False(value1 <= value2);
            Assert.False(value1 == value2);
            Assert.True(value1 != value2);
        }
    }
}