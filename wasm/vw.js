const { log } = require('console');
const fs = require('fs');
const crypto = require('crypto');
const VWWasmModule = require('./out/vw-wasm.js');


// internals

const ProblemType =
{
    All: 'All',
    CB: 'Cb',
};

// exported

class WorkspaceBase {
    constructor(type, { args_str, model_file } = {}) {
        if (args_str === undefined && model_file === undefined) {
            throw new Error("Can not initialize vw object without args_str or a model_file");
        }

        this._args_str = args_str;
        if (args_str === undefined) {
            this._args_str = "";
        }

        this._outputLogStream = null;

        if (model_file !== undefined) {
            let modelBuffer = fs.readFileSync(model_file);
            let ptr = VWWasmModule._malloc(modelBuffer.byteLength);
            let heapBytes = new Uint8Array(VWWasmModule.HEAPU8.buffer, ptr, modelBuffer.byteLength);
            heapBytes.set(new Uint8Array(modelBuffer));
            if (type === ProblemType.All) {
                this._instance = new VWWasmModule.VWModel(this._args_str, ptr, modelBuffer.byteLength);
            } else if (type === ProblemType.CB) {
                this._instance = new VWWasmModule.VWCBModel(this._args_str, ptr, modelBuffer.byteLength);
            }
            else {
                throw new Error("Unknown model type");
            }
            VWWasmModule._free(ptr);
        }
        else {
            if (type === ProblemType.All) {
                this._instance = new VWWasmModule.VWModel(this._args_str);
            } else if (type === ProblemType.CB) {
                this._instance = new VWWasmModule.VWCBModel(this._args_str);
            }
            else {
                throw new Error("Unknown model type");
            }
        }


        return new Proxy(this, {
            get(target, propertyName, receiver) {
                if (typeof target._instance[propertyName] === 'function') {
                    if (propertyName === '_samplePmf') {
                        return undefined;
                    }
                    if (propertyName === '_predictAndSample') {
                        return undefined;
                    }
                    return target._instance[propertyName].bind(target._instance);
                }

                return Reflect.get(target, propertyName, receiver);
            }
        });
    }

    /**
     * 
     * Takes a file location and stores the VW model in binary format in the file.
     * 
     * @param {string} model_file the path to the file where the model will be saved 
     */
    saveModel(model_file) {
        let char_vector = this._instance.getModel();
        const size = char_vector.size();
        const uint8Array = new Uint8Array(size);

        for (let i = 0; i < size; ++i) {
            uint8Array[i] = char_vector.get(i);
        }

        fs.writeFileSync(model_file, Buffer.from(uint8Array));

        char_vector.delete();
    }

    /**
     * 
     * Takes a file location and loads the VW model from the file.
     * 
     * @param {string} model_file the path to the file where the model will be loaded from
     */
    loadModel(model_file) {
        let modelBuffer = fs.readFileSync(model_file);
        let ptr = VWWasmModule._malloc(modelBuffer.byteLength);
        let heapBytes = new Uint8Array(VWWasmModule.HEAPU8.buffer, ptr, modelBuffer.byteLength);
        heapBytes.set(new Uint8Array(modelBuffer));
        this._instance.loadModelFromBuffer(ptr, modelBuffer.byteLength);
        VWWasmModule._free(ptr);
    }

    /**
     * 
     * Starts a log stream to the specified file. Any new logs will be appended to the file.
     * 
     * @param {string} log_file the path to the file where the log will be appended to
     * @throws {Error} Throws an error if another logging stream has already been started
     */
    startLogStream(log_file) {
        if (this._outputLogStream !== null) {
            throw new Error("Can not start log stream, another log stream is currently active. Call endLogStream first if you want to change the log file. Current log file: " + this._log_file);
        }
        else {
            this._log_file = log_file;
            this._outputLogStream = fs.createWriteStream(log_file, { flags: 'a' });
        }
    }

    /**
     * Takes a string and appends it to the log file. Line is logged in an asynchronous manner.
     * 
     * @param {string} line the line to be appended to the log file
     * @throws {Error} Throws an error if no logging stream has been started
     */
    logLineToStream(line) {
        if (this._outputLogStream !== null) {
            this._outputLogStream.write(line);
        }
        else {
            throw new Error("Can not log line, log file is not specified. Call startLogStream first.");
        }
    }

    /**
     * Closes the logging stream. Logs a warning to the console if there is no logging stream active, but does not throw
     */
    endLogStream() {
        if (this._outputLogStream !== null) {
            this._outputLogStream.end();
            this._outputLogStream = null;
            this._log_file = null;
        }
        else {
            console.warn("Can not close log, log file is not specified");
        }
    }

    /**
     * 
     * Takes a string and appends it to the log file. Line is logged in a synchronous manner. 
     * Every call to this function will open a new file handle, append the line and close the file handle.
     * 
     * @param {string} log_file the path to the file where the log will be appended to
     * @param {string} line the line to be appended to the log file
     */
    logLineSync(log_file, line) {
        if (this._outputLogStream !== null && this._log_file === log_file) {
            throw new Error("Can not call logLineSync on log file while the same file has an async log writer active. Call endLogStream first. Log file: " + log_file);
        }
        fs.appendFileSync(log_file, line);
    }

    /**
     * Deletes the underlying VW instance. This function should be called when the instance is no longer needed.
     */
    delete() {
        this._instance.delete();
    }
};

/**
 * A Wrapper around the Wowpal Wabbit C++ library.
 * @class
 * @extends WorkspaceBase
 */
class Workspace extends WorkspaceBase {
    /**
     * Creates a new Vowpal Wabbit workspace.
     * Can accept either or both string arguments and a model file.
     * 
     * @constructor
     * @param {string} [args_str] - The arguments that are used to initialize Vowpal Wabbit (optional)
     * @param {string} [model_file] - The path to the file where the model will be loaded from (optional)
     * @throws {Error} Throws an error if both of the string arguments and the model file are missing,
     *  or throws an error if both string arguments and a model file are provided, and the string arguments
     *  and arguments defined in the model clash
     */
    constructor({ args_str, model_file, log_file } = {}) {
        super(ProblemType.All, { args_str, model_file, log_file });
    }
};

function getExampleString(example) {
    let context = ""
    if (example.hasOwnProperty('text_context')) {
        context = example.text_context;
    }
    else {
        throw new Error("Can not log example, there is no context available");
    }

    const lines = context.split("\n").map((substr) => substr.trim());
    lines.push("");

    if (example.hasOwnProperty("labels") && example["labels"].length > 0) {
        let indexOffset = 0;
        if (context.includes("shared")) {
            indexOffset = 1;
        }

        for (let i = 0; i < example["labels"].length; i++) {
            let label = example["labels"][i];
            if (label.action + indexOffset >= lines.length) {
                throw new Error("action index out of bounds: " + label.action);
            }

            lines[label.action + indexOffset] = label.action + ":" + label.cost + ":" + label.probability + " " + lines[label.action + indexOffset]
        }
    }
    return lines.join("\n");
}

/**
 * A Wrapper around the Wowpal Wabbit C++ library for Contextual Bandit exploration algorithms.
 * @class
 * @extends WorkspaceBase
 * @example
 * 
 * const vwPromise = require('./vw.js');
 * // require returns a promise because we need to wait for the wasm module to be initialized
 * 
 * vwPromise.then((vw) => {    
 *  let model = new vw.CbWorkspace({ args_str: "--cb_explore_adf" });
 *  model.startLogStream("mylogfile.txt");
 * 
 *  let example = {
 *      text_context: `shared | s_1 s_2
 *          | a_1 b_1 c_1
 *          | a_2 b_2 c_2
 *          | a_3 b_3 c_3`,
 *      };
 * 
 *  let prediction = model.predictAndSample(example);
 *  
 *  example.labels = [{ action: prediction["action"], cost: 1.0, probability: prediction["score"] }];
 * 
 *  model.learn(example);
 *  model.logExampleToStream(example);
 *  
 *  model.saveModel("my_model.vw");
 *  model.endLogStream();
 *  model.delete();
 * 
 *  let model2 = new vw.CbWorkspace({ model_file: "my_model.vw" });
 *  console.log(model2.predict(example));
 *  console.log(model2.predictAndSample(example));
 *  model2.delete();
 * });
 */
class CbWorkspace extends WorkspaceBase {
    /**
     * Creates a new Vowpal Wabbit workspace for Contextual Bandit exploration algorithms.
     * Can accept either or both string arguments and a model file.
     * 
     * @constructor
     * @param {string} [args_str] - The arguments that are used to initialize Vowpal Wabbit (optional)
     * @param {string} [model_file] - The path to the file where the model will be loaded from (optional)
     * @throws {Error} Throws an error if both of the string arguments and the model file are missing,
     *  or throws an error if both string arguments and a model file are provided, and the string arguments
     *  and arguments defined in the model clash
     */

    constructor({ args_str, model_file, log_file } = {}) {
        super(ProblemType.CB, { args_str, model_file, log_file });
    }

    /**
     * 
     * Takes an exploration prediction (array of action, score pairs) and returns a single action and score,
     * along with a unique id that was used to seed the sampling and that can be used to track and reproduce the sampling.
     * 
     * @param {array} pmf probability mass function, an array of action,score pairs that was returned by predict
     * @returns {object} an object with the following properties:
     * - action: the action index that was sampled
     * - score: the score of the action that was sampled
     * - uuid: the uuid that was passed to the predict function
     * @throws {Error} Throws an error if the input is not an array of action,score pairs
     */
    samplePmf(pmf) {
        let uuid = crypto.randomUUID();
        let ret = this._instance._samplePmf(pmf, uuid);
        ret["uuid"] = uuid;
        return ret;
    }

    /**
     * 
     * Takes an exploration prediction (array of action, score pairs) and a unique id that is used to seed the sampling,
     * and returns a single action index and the corresponding score.
     * 
     * @param {array} pmf probability mass function, an array of action,score pairs that was returned by predict
     * @param {string} uuid a unique id that can be used to seed the prediction
     * @returns {object} an object with the following properties:
     * - action: the action index that was sampled
     * - score: the score of the action that was sampled
     * - uuid: the uuid that was passed to the predict function
     * @throws {Error} Throws an error if the input is not an array of action,score pairs
     */
    samplePmfWithUUID(pmf, uuid) {
        let ret = this._instance._samplePmf(pmf, uuid);
        ret["uuid"] = uuid;
        return ret;
    }

    /**
     * 
     * Takes an example with a text_context field and calls predict. The prediction (a probability mass function over the available actions)
     * will then be sampled from, and only the chosen action index and the corresponding score will be returned,
     * along with a unique id that was used to seed the sampling and that can be used to track and reproduce the sampling.
     * 
     * @param {object} example an example object containing the context to be used during prediction
     * @returns {object} an object with the following properties:
     * - action: the action index that was sampled
     * - score: the score of the action that was sampled
     * - uuid: the uuid that was passed to the predict function
     * @throws {Error} if there is no text_context field in the example
     */
    predictAndSample(example) {
        let uuid = crypto.randomUUID();
        let ret = this._instance._predictAndSample(example, uuid);
        ret["uuid"] = uuid;
        return ret;
    }

    /**
     * 
     * Takes an example with a text_context field and calls predict, and a unique id that is used to seed the sampling.
     * The prediction (a probability mass function over the available actions) will then be sampled from, and only the chosen action index
     * and the corresponding score will be returned, along with a unique id that was used to seed the sampling and that can be used to track and reproduce the sampling.
     * 
     * @param {object} example an example object containing the context to be used during prediction
     * @returns {object} an object with the following properties:
     * - action: the action index that was sampled
     * - score: the score of the action that was sampled
     * - uuid: the uuid that was passed to the predict function
     * @throws {Error} if there is no text_context field in the example
     */
    predictAndSampleWithUUID(example, uuid) {
        let ret = this._instance._predictAndSample(example, uuid);
        ret["uuid"] = uuid;
        return ret;
    }

    /**
     * 
     * Takes an example, stringifies it and appends it to the log file. Line is logged in an asynchronous manner.
     * 
     * @param {object} example an example that will be stringified and appended to the log file
     * @throws {Error} Throws an error if no logging stream has been started
     */
    logExampleToStream(example) {
        let ex_str = getExampleString(example);
        this.logLineToStream(ex_str);
    }

    /**
     * 
     * Takes an example, stringifies it, and appends it to the log file. Example is logged in a synchronous manner.
     * Every call to this function will open a new file handle, append the line and close the file handle.
     * 
     * @param {string} log_file the path to the file where the log will be appended to
     * @param {object} example an example that will be stringified and appended to the log file
     */
    logExampleSync(log_file, example) {
        let ex_str = getExampleString(example);
        this.logLineSync(log_file, ex_str);
    }

};

function getExceptionMessage(exception) {
    return VWWasmModule.getExceptionMessage(exception)
};

module.exports = new Promise((resolve) => {
    VWWasmModule.onRuntimeInitialized = () => {
        class Prediction {
            static Type = {
                Scalar: VWWasmModule.PredictionType.scalar,
                Scalars: VWWasmModule.PredictionType.scalars,
                ActionScores: VWWasmModule.PredictionType.action_scores,
                Pdf: VWWasmModule.PredictionType.pdf,
                ActionProbs: VWWasmModule.PredictionType.action_probs,
                MultiClass: VWWasmModule.PredictionType.multiclass,
                MultiLabels: VWWasmModule.PredictionType.multilabels,
                Prob: VWWasmModule.PredictionType.prob,
                MultiClassProb: VWWasmModule.PredictionType.multiclassprob,
                DecisionProbs: VWWasmModule.PredictionType.decision_probs,
                ActionPdfValue: VWWasmModule.PredictionType.ActionPdfValue,
                ActiveMultiClass: VWWasmModule.PredictionType.activeMultiClass,
            };
        };

        resolve(
            {
                Workspace: Workspace,
                CbWorkspace: CbWorkspace,
                Prediction: Prediction,
                getExceptionMessage: getExceptionMessage,
            }
        )
    }
})
