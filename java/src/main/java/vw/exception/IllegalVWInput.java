package vw.exception;

/**
 * An exception that indicates an unsupported VW input type.
 */
public class IllegalVWInput extends Exception {
    public IllegalVWInput() {}

    //Constructor that accepts a message
    public IllegalVWInput(String message) {
        super(message);
    }
}
