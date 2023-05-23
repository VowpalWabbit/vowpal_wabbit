# Vowpal Wabbit

Javascript bindings for [VowpalWabbit](https://vowpalwabbit.org/)

## Documentation

[API documentation](documentation.md)

## Examples and How-To

### How-To include the dependency and initialize a Contextual Bandit ADF model

Full API reference [here](documentation.md#CbWorkspace)

Require returns a promise because we need to wait for the WASM module to be initialized before including and using the VowpalWabbit JS code

```(js)
const vwPromise = require('@vowpalwabbit/vowpalwabbit');

vwPromise.then((vw) => {
    let model = new vw.CbWorkspace({ args_str: "--cb_explore_adf" });
    model.delete()
});
```

A VW model needs to be deleted after we are done with its usage to return the aquired memory back to the WASM runtime.

### How-To call learn and predict on a Contextual Bandit model

```(js)
const vwPromise = require('@vowpalwabbit/vowpalwabbit');

vwPromise.then((vw) => {
    let model = new vw.CbWorkspace({ args_str: "--cb_explore_adf" });

    let example = {
        text_context: `shared | s_1 s_2
            | a_1 b_1 c_1
            | a_2 b_2 c_2
            | a_3 b_3 c_3`,
        };

    let pred = model.predictAndSample(example);

    # user defined cost function
    let action_cost = calculate_cost_from_action(pred["action"])

    example.labels = [{ action: pred["action"], cost: action_cost, probability: pred["score"] }];

    model.learn(example);

    model.delete()
});
```

`predictAndSample` is a convenience function that samples the probability mass function that a call to `vw.predict()` returns. It is possible to first predict, get the entire pmf back, and then sample from it:

```(js)
    let pred = model.predictAndSample(example);
    let uuid = pred["uuid"];
    let pred2 = model.predictAndSampleWithUUID(example, uuid);
```

`predictAndSample` and `samplePmf` generate and use a `uuid` during sampling. That `uuid` is available in the js object returned from both function calls and can be used for reproducability. A user defined `uuid` can also be specified with these calls:

```(js)
    let pred = model.samplePmf(pmf);
    let uuid = pred["uuid"];
    let pred2 = model.samplePmfWithUUID(pmf, uuid);
```

or

```(js)
    let chosen = model.samplePmf(pmf);
    let uuid = chosen["uuid"];
    let chosen2 = model.samplePmfWithUUID(pmf, uuid);
```

### How-To save/load a model

There are two ways to save/load a model

#### Provide a file path where the model will be saved to or loaded from

Node's `fs` will be used to access the file and save/loading is blocking

```(js)
const vwPromise = require('@vowpalwabbit/vowpalwabbit');

vwPromise.then((vw) => {
    let model = new vw.CbWorkspace({ args_str: "--cb_explore_adf" });

    let example = {
        text_context: `shared | s_1 s_2
            | a_1 b_1 c_1
            | a_2 b_2 c_2
            | a_3 b_3 c_3`,
        labels  = [{ action: 0, cost: 1.0, probability: 1}]
        };

    model.learn(example);
    model.saveModelToFile("my_model.vw");
    model.delete();

    let model2 = new vw.CbWorkspace({ model_file: "my_model.vw" });
    console.log(model2.predict(example));
    model2.delete();
});
```

A model can be loaded from a file either during model construction (shown above) or as a separate function call:

```(js)
    model2.loadModelFromFile("my_model.vw");
```

#### Get the model or supply the model as a Uint8Array for user-handled storing

```(js)
    # get model as Uint8Array and store it using fs
    let modelarray = model4.getModelAsArray();
    let filePath = path.join(__dirname, "my_model.vw");
    fs.writeFileSync(filePath, Buffer.from(modelarray));

    # load model as a Uint8Array and call load with it
    {
        let modelBuffer = fs.readFileSync(filePath);
        let ptr = vw.wasmModule._malloc(modelBuffer.byteLength);
        let heapBytes = new Uint8Array(vw.wasmModule.HEAPU8.buffer, ptr, modelBuffer.byteLength);
        heapBytes.set(new Uint8Array(modelBuffer));
        model.loadModelFromArray(ptr, modelBuffer.byteLength);
        vw.wasmModule._free(ptr);
    }

    # load model as Uint8Array and construct a new vw model with it
    {
        let modelBuffer = fs.readFileSync(filePath);
        let ptr = vw.wasmModule._malloc(modelBuffer.byteLength);
        let heapBytes = new Uint8Array(vw.wasmModule.HEAPU8.buffer, ptr, modelBuffer.byteLength);
        heapBytes.set(new Uint8Array(modelBuffer));
        let model = new vw.CbWorkspace({ model_array: [ptr, modelBuffer.byteLength] });
        vw.wasmModule._free(ptr);
        model.delete();
    }
```

### How-To log examples into a file or stringify examples for user-handled logging

A log stream can be started which will create and use a `fs` write stream:

```(js)
const vwPromise = require('@vowpalwabbit/vowpalwabbit');

vwPromise.then((vw) => {

    let example = {
        text_context: `shared | s_1 s_2
            | a_1 b_1 c_1
            | a_2 b_2 c_2
            | a_3 b_3 c_3`,
        labels  = [{ action: 0, cost: 1.0, probability: 1}]
        };

    let model = new vw.CbWorkspace({ args_str: "--cb_explore_adf" });
    let vwLogger = new vw.VWExampleLogger();
    vwLogger.startLogStream("mylogfile.txt");
    vwLogger.logCBExampleToStream(example);
    vwLogger.endLogStream();
    model.delete();
});
```

There is also the option of stringifying an example for user-handled logging:

```(js)
    let cbAsString = CBExampleToString(example);
```

Synchronous logging options are also available [see API documentation](documentation.md#VWExampleLogger)

### How-To train a model with data from a file

```(js)
const vwPromise = require('@vowpalwabbit/vowpalwabbit');

vwPromise.then((vw) => {

    let model = new vw.CbWorkspace({ args_str: "--cb_explore_adf" });
    const fileStream = fs.createReadStream(filePath);
    const rl = readline.createInterface({
        input: fileStream,
        crlfDelay: Infinity,
        output: process.stdout,
        terminal: false,
    });


    rl.on('line', model.addLine.bind(model));

    rl.on('close', () => {
        assert(model.sumLoss() > 0);
        model.delete();
    });
});
```

### How-To handle errors

Some function calls with throw if something went wrong or if they were called incorrectly. There are two type of errors that can be thrown: native JavaScript errors and WebAssembly runtime errors, the latter which are wrapped in a VWError object.

When logging an error to the console there needs to be a check of the error type and the logging needs to be handled accordingly:

```(js)
try {}
catch (e)
{
    if (e.name === 'VWError') {
            console.error(vw.getExceptionMessage(e));
    }
    else {
        console.error(e);
    }
}
```

### How-To use a generic VW model (non Contextual Bandit specific functionality)

Full API reference [here](documentation.md#Workspace)

#### Simple regression example

```(js)
const vwPromise = require('@vowpalwabbit/vowpalwabbit');

vwPromise.then((vw) => {

    let model = new vw.Workspace({ args_str: "" });
    let example = model.parse("|f 6:6.8953723e-02");
    let prediction = model.predict(example);
    model.finishExample(example);
    example.delete();
    model.delete();
});
```

#### CCB example

```(js)
const vwPromise = require('@vowpalwabbit/vowpalwabbit');

vwPromise.then((vw) => {

    let model = new vw.Workspace({ args_str: "--ccb_explore_adf" });

    let example = model.parse(`
        ccb shared |User b
        ccb action |Action d
        ccb action |Action e
        ccb action |Action f
        ccb action |Action ff
        ccb action |Action fff
        ccb slot 0:0:0.2 |Slot h
        ccb slot 1:0:0.25 |Slot i
        ccb slot 2:0:0.333333 |Slot j
    `);

    let prediction = model.predict(example);

    assert(prediction[0][0].hasOwnProperty('action'));
    assert(prediction[0][0].hasOwnProperty('score'));

    model.finishExample(example);
    example.delete();
    model.delete();
});
```
