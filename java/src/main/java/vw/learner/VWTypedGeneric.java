package vw.learner;

/**
 * This is the main generic interface to which all VW predictors should adhere.  VW predictors
 * may provided <em>additional methods</em> when the cost of boxing a primitive to an object is
 * too expensive.
 *
 * The recommended way of using this interface is to extend <code>VWGenericBase</code>.
 * See its documentation for more details.
 *
 * @author deak
 */
public interface VWTypedGeneric<T> extends VWGeneric {

    /**
     * Learn from the example then return the prediction given the example, after the internal learner is updated.
     * @param example an example from which to learn.
     * @return a prediction after the model has been updated.
     */
    T learn(String example);

    /**
     * Prediction without learning from the example.
     * @param example an example upon which the prediction is based
     * @return a prediction.
     */
    T predict(String example);
}
