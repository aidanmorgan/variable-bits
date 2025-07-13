# Variable Streams Java Implementation

This is a Java implementation of the Variable Streams library, which provides functionality for reading and writing individual bits to streams.

## Features

- Read and write individual bits to streams
- Support for bit values from 1 to 128 bits
- Support for both signed and unsigned values
- Memory-efficient bit stream implementation
- Integration with Java's InputStream and OutputStream

## Requirements

- Java 17 or higher
- Maven 3.6 or higher

## Installation

Add the following dependency to your Maven project:

```xml
<dependency>
    <groupId>com.aidanjmorgan.variablebits</groupId>
    <artifactId>variable-streams</artifactId>
    <version>1.0.0</version>
</dependency>
```

## Usage

### BitStream

`BitStream` is an in-memory implementation for reading and writing bits:

```java
// Create a new BitStream
BitStream bitStream = new BitStream();

// Write bits
bitStream.writeBits(0b101, (byte)3);
bitStream.writeBits(0b11110000, (byte)8);
bitStream.writeBits(0xABCD, (byte)16);

// Reset the position to the beginning
bitStream.setPosition(0);

// Read bits
long value1 = bitStream.readBits((byte)3);  // 0b101
long value2 = bitStream.readBits((byte)8);  // 0b11110000
long value3 = bitStream.readBits((byte)16); // 0xABCD

// Convert to byte array
byte[] data = bitStream.toByteArray();
```

### BitStreamWriter

`BitStreamWriter` writes bits to an underlying `OutputStream`:

```java
// Create a BitStreamWriter with an OutputStream
try (BitStreamWriter writer = new BitStreamWriter(outputStream)) {
    // Write bits
    writer.writeBits(0b101, (byte)3);
    writer.writeBits(0b11110000, (byte)8);
    writer.writeBits(0xABCD, (byte)16);
    
    // Write a BitValue
    BitValue value = BitValue.newValue(0xDEADBEEF, (byte)32);
    writer.writeBitValue(value, null);
    
    // Write a 128-bit value
    UInt128 value128 = new UInt128(0x1234567890ABCDEFL, 0xFEDCBA0987654321L);
    writer.writeBitsU128(value128, (byte)128);
    
    // Flush to ensure all bits are written
    writer.flush();
}
```

### BitStreamReader

`BitStreamReader` reads bits from an underlying `InputStream`:

```java
// Create a BitStreamReader with an InputStream
try (BitStreamReader reader = new BitStreamReader(inputStream)) {
    // Read bits
    long value1 = reader.readBits((byte)3);  // Read 3 bits
    long value2 = reader.readBits((byte)8);  // Read 8 bits
    long value3 = reader.readBits((byte)16); // Read 16 bits
    
    // Read a BitValue
    BitValue value = reader.readBitValue((byte)32); // Read 32 bits as a BitValue
    
    // Read a 128-bit value
    UInt128 value128 = reader.readBitsU128((byte)128); // Read 128 bits
    
    // Check if we've reached the end of the stream
    boolean eof = reader.isEof();
}
```

### BitValue

`BitValue` represents a value with a specific bit width:

```java
// Create BitValues
BitValue u8Value = BitValue.newValue(0xAA, (byte)8);
BitValue u16Value = BitValue.newValue(0xBBCC, (byte)16);
BitValue u32Value = BitValue.newValue(0xDDEEFF, (byte)24);

// Create signed BitValues
BitValue i8Value = BitValue.newSignedValue(-42, (byte)8);
BitValue i16Value = BitValue.newSignedValue(-1000, (byte)16);

// Create 128-bit BitValues
UInt128 uint128 = new UInt128(0x1234567890ABCDEFL, 0xFEDCBA0987654321L);
BitValue u128Value = BitValue.newUInt128Value(uint128, (byte)128);

Int128 int128 = new Int128(-1L, 0L);
BitValue i128Value = BitValue.newInt128Value(int128, (byte)128);

// Convert to different types
long u64 = u32Value.toUInt64();
UInt128 u128 = u32Value.toUInt128();
long i64 = i16Value.toInt64();
Int128 i128 = i16Value.toInt128();

// Check if signed
boolean isSigned = i16Value.isSigned(); // true
```

### UInt128 and Int128

`UInt128` and `Int128` represent 128-bit unsigned and signed integers, respectively:

```java
// Create UInt128 values
UInt128 value1 = new UInt128(0x1234567890ABCDEFL, 0xFEDCBA0987654321L);
UInt128 value2 = new UInt128(42L); // High bits are 0

// Create Int128 values
Int128 value3 = new Int128(0x1234567890ABCDEFL, 0xFEDCBA0987654321L);
Int128 value4 = new Int128(-42L); // Negative value

// Perform operations
UInt128 sum = value1.add(value2);
UInt128 difference = value1.subtract(value2);
UInt128 and = value1.and(value2);
UInt128 or = value1.or(value2);
UInt128 xor = value1.xor(value2);
UInt128 not = value1.not();
UInt128 leftShift = value1.shiftLeft(10);
UInt128 rightShift = value1.shiftRight(10);

// Compare values
int comparison = value1.compareTo(value2);
boolean equals = value1.equals(value2);

// Convert between UInt128 and Int128
UInt128 uint128 = value3.toUInt128();
Int128 int128 = Int128.fromUInt128(value1);
```

## License

This project is licensed under the MIT License - see the LICENSE file for details.