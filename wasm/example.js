const VWModule = require('./out/vw-wasm.js');
const { initializeVWModule } = require('./vw.js');

// You must wait for the WASM runtime to be ready before using the module.

// using initialzieVWModule instead of VWModule.["onRuntimeInitialized"] means that both of these "then"'s will run and not just one of them.
initializeVWModule().then(() => {
    try {
        // Create a model with default options
        let model = new VWModule.VWModel("");

        let example_line = "0 | price:.23 sqft:.25 age:.05 2006";
        // For multi_ex learners, the input to parse should have newlines in it.
        // One call to parse should only ever get input for either a single
        // example or a single multi_ex grouping.
        let parsedExample = model.parse(example_line);

        // Prediction returns a normal javascript value which corresponds to the
        // expected prediction type. Here is just a number.
        console.log(`prediction: ${model.predict(parsedExample)}`);

        model.learn(parsedExample);

        // Any examples which were given to either learn and/or predict must be
        // given to finishExample.
        model.finishExample(parsedExample);

        // Every object returned by parse must be deleted manually.
        // Additionally, shallow or deep example copies must be deleted.
        //      let shallowCopyExample = parsedExample;
        //      shallowCopyExample.delete();
        //
        //      let deepCopyExample = parsedExample.clone(model);
        //      deepCopyExample.delete();
        parsedExample.delete();

        // VWModel must also always be manually deleted.
        model.delete();
    }
    catch (e) {
        // Exceptions that are produced by the module must be passed through
        // this transformation function to get the error info.
        console.error(VWModule.getExceptionMessage(e));
    }
});

// Delay test execution until the WASM VWModule is ready
initializeVWModule().then(() => {
    const vw = require('./vw.js');
    try {
        // Create a model with default options
        let model = new vw.VW({ args_str: "" });

        let example_line = "0 | price:.23 sqft:.25 age:.05 2006";
        // For multi_ex learners, the input to parse should have newlines in it.
        // One call to parse should only ever get input for either a single
        // example or a single multi_ex grouping.
        let parsedExample = model.parse(example_line);

        // Prediction returns a normal javascript value which corresponds to the
        // expected prediction type. Here is just a number.
        console.log(`prediction: ${model.predict(parsedExample)}`);

        model.learn(parsedExample);

        // Any examples which were given to either learn and/or predict must be
        // given to finishExample.
        model.finishExample(parsedExample);

        // Every object returned by parse must be deleted manually.
        // Additionally, shallow or deep example copies must be deleted.
        //      let shallowCopyExample = parsedExample;
        //      shallowCopyExample.delete();
        //
        //      let deepCopyExample = parsedExample.clone(model);
        //      deepCopyExample.delete();
        parsedExample.delete();

        // VWModel must also always be manually deleted.
        model.delete();
    }
    catch (e) {
        // Exceptions that are produced by the module must be passed through
        // this transformation function to get the error info.
        console.error(vw.getExceptionMessage(e));
    }
});