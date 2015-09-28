package vw;

import java.io.Closeable;

/**
 * Created by deak on 9/27/15.
 */
public interface VWGeneric<T> extends Closeable {
    T learn(String example);
    T predict(String example);
}
