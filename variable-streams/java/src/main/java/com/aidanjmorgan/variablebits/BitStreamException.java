package com.aidanjmorgan.variablebits;

import java.io.IOException;

/**
 * Exception that can occur during bit stream operations.
 */
public class BitStreamException extends RuntimeException {

    /**
     * The type of bit stream error.
     */
    private final BitStreamErrorType errorType;

    /**
     * Creates a new BitStreamException with the specified error type.
     *
     * @param errorType The type of bit stream error.
     * @param message The error message.
     */
    public BitStreamException(BitStreamErrorType errorType, String message) {
        super(message);
        this.errorType = errorType;
    }

    /**
     * Creates a new BitStreamException with the specified error type and inner exception.
     *
     * @param errorType The type of bit stream error.
     * @param message The error message.
     * @param cause The cause of this exception.
     */
    public BitStreamException(BitStreamErrorType errorType, String message, Throwable cause) {
        super(message, cause);
        this.errorType = errorType;
    }

    /**
     * Gets the type of bit stream error.
     *
     * @return The error type.
     */
    public BitStreamErrorType getErrorType() {
        return errorType;
    }

    /**
     * Creates a BitStreamException from an IOException.
     *
     * @param ioException The IOException.
     * @return A new BitStreamException.
     */
    public static BitStreamException fromIOException(IOException ioException) {
        return new BitStreamException(BitStreamErrorType.IO, "An I/O error occurred.", ioException);
    }

    /**
     * Creates a BitStreamException for an invalid bit count.
     *
     * @return A new BitStreamException.
     */
    public static BitStreamException invalidBitCount() {
        return new BitStreamException(BitStreamErrorType.INVALID_BIT_COUNT, 
                "The requested bit count is invalid (must be between 1 and 128).");
    }

    /**
     * Creates a BitStreamException for end of stream reached.
     *
     * @return A new BitStreamException.
     */
    public static BitStreamException endOfStream() {
        return new BitStreamException(BitStreamErrorType.END_OF_STREAM, 
                "End of stream reached while reading.");
    }

    /**
     * Types of errors that can occur during bit stream operations.
     */
    public enum BitStreamErrorType {
        /**
         * An I/O error occurred.
         */
        IO,

        /**
         * The requested bit count is invalid (must be between 1 and 128).
         */
        INVALID_BIT_COUNT,

        /**
         * End of stream reached while reading.
         */
        END_OF_STREAM
    }
}
