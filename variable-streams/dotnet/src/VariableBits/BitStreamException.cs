using System;
using System.IO;

namespace VariableBits
{
    /// <summary>
    /// Exception that can occur during bit stream operations.
    /// </summary>
    public class BitStreamException : Exception
    {
        /// <summary>
        /// The type of bit stream error.
        /// </summary>
        public BitStreamErrorType ErrorType { get; }

        /// <summary>
        /// Creates a new BitStreamException with the specified error type.
        /// </summary>
        /// <param name="errorType">The type of bit stream error.</param>
        /// <param name="message">The error message.</param>
        public BitStreamException(BitStreamErrorType errorType, string message) 
            : base(message)
        {
            ErrorType = errorType;
        }

        /// <summary>
        /// Creates a new BitStreamException with the specified error type and inner exception.
        /// </summary>
        /// <param name="errorType">The type of bit stream error.</param>
        /// <param name="message">The error message.</param>
        /// <param name="innerException">The inner exception.</param>
        public BitStreamException(BitStreamErrorType errorType, string message, Exception innerException) 
            : base(message, innerException)
        {
            ErrorType = errorType;
        }

        /// <summary>
        /// Creates a BitStreamException from an IOException.
        /// </summary>
        /// <param name="ioException">The IOException.</param>
        /// <returns>A new BitStreamException.</returns>
        public static BitStreamException FromIOException(IOException ioException) =>
            new(BitStreamErrorType.Io, "An I/O error occurred.", ioException);

        /// <summary>
        /// Creates a BitStreamException for an invalid bit count.
        /// </summary>
        /// <returns>A new BitStreamException.</returns>
        public static BitStreamException InvalidBitCount() =>
            new(BitStreamErrorType.InvalidBitCount, "The requested bit count is invalid (must be between 1 and 128).");

        /// <summary>
        /// Creates a BitStreamException for end of stream reached.
        /// </summary>
        /// <returns>A new BitStreamException.</returns>
        public static BitStreamException EndOfStream() =>
            new(BitStreamErrorType.EndOfStream, "End of stream reached while reading.");
    }

    /// <summary>
    /// Types of errors that can occur during bit stream operations.
    /// </summary>
    public enum BitStreamErrorType
    {
        /// <summary>
        /// An I/O error occurred.
        /// </summary>
        Io,

        /// <summary>
        /// The requested bit count is invalid (must be between 1 and 128).
        /// </summary>
        InvalidBitCount,

        /// <summary>
        /// End of stream reached while reading.
        /// </summary>
        EndOfStream
    }
}