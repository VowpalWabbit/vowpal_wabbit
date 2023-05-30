const vwPromise = require('@vowpalwabbit/vowpalwabbit');
const fs = require('fs');

// Delay test execution until the WASM VWModule is ready
vwPromise.then((vw) => {
    try {
        // Create a model with default options
        let model = new vw.Workspace({ args_str: "" });

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

vwPromise.then((vw) => {
    try {
        let logFile = "mylogfile.txt";
        let modelFile = "my_model.vw";
        // Create a model with default options
        let model = new vw.CbWorkspace({ args_str: "--cb_explore_adf" });
        let vwLogger = new vw.VWExampleLogger();
        vwLogger.startLogStream(logFile);

        let example = {
            text_context: `shared | s_1 s_2
            | a_1 b_1 c_1
            | a_2 b_2 c_2
            | a_3 b_3 c_3`,
        };

        let prediction = model.predictAndSample(example);

        example.labels = [{ action: prediction["action"], cost: 1.0, probability: prediction["score"] }];

        model.learn(example);

        vwLogger.logCBExampleToStream(example);

        model.saveModelToFile(modelFile);

        vwLogger.endLogStream();
        model.delete();

        let model2 = new vw.CbWorkspace({ model_file: modelFile });
        console.log(model2.predict(example));
        console.log(model2.predictAndSample(example));
        model2.delete();

        fs.unlink(logFile, (err) => {
            if (err) {
                console.error('Error removing file:', err);
            } else {
                console.log('File removed successfully:', logFile);
            }
        });

        fs.unlink(modelFile, (err) => {
            if (err) {
                console.error('Error removing file:', err);
            } else {
                console.log('File removed successfully:', modelFile);
            }
        });
    }
    catch (e) {
        // Exceptions that are produced by the module must be passed through
        // this transformation function to get the error info.
        if (e.name === 'VWError') {
            console.error(vw.getExceptionMessage(e.stack));
        }
        else { console.error(e); }
    }
}).catch(err => { console.log(err) });

vwPromise.then((vw) => {
    try {
        let model = new vw.CbWorkspace({ args_str: "--cb_explore_adf" });

        let example = {
            text_context: `shared | s_1 s_2
            | a_1 b_1 c_1
            | a_2 b_2 c_2
            | a_3 b_3 c_3`,
        };
    
        // model learn without a label should throw
        model.learn(example);

        model.delete();
    }
    catch (e) {
        // Exceptions that are produced by the module must be passed through
        // this transformation function to get the error info.
        if (e.name === 'VWError') {
            console.error(vw.getExceptionMessage(e.stack));
        }
        else { console.error(e); }
    }
}).catch(err => { console.log(err) });