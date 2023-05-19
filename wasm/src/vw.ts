import fs from 'fs';
import crypto from 'crypto';

const VWWasmModule = require('./vw-wasm.js');

// internals

const ProblemType =
{
    All: 'All',
    CB: 'Cb',
};

// exported

/**
 * A class that helps facilitate the stringification of Vowpal Wabbit examples, and the logging of Vowpal Wabbit examples to a file.
 * @class
 */
class VWExampleLogger {
    _outputLogStream: fs.WriteStream | null;
    _log_file: string | null;

    constructor() {
        this._outputLogStream = null;
        this._log_file = null;
    }

    /**
     * 
     * Starts a log stream to the specified file. Any new logs will be appended to the file.
     * 
     * @param {string} log_file the path to the file where the log will be appended to
     * @throws {Error} Throws an error if another logging stream has already been started
     */
    startLogStream(log_file: string) {
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
    logLineToStream(line: string) {
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
     * @throws {Error} Throws an error if another logging stream has already been started
     */
    logLineSync(log_file: string, line: string) {
        if (this._outputLogStream !== null && this._log_file === log_file) {
            throw new Error("Can not call logLineSync on log file while the same file has an async log writer active. Call endLogStream first. Log file: " + log_file);
        }
        fs.appendFileSync(log_file, line);
    }

    /**
     * 
     * Takes a CB example and returns the string representation of it
     * 
     * @param {object} example a CB example that will be stringified
     * @returns {string} the string representation of the CB example
     * @throws {Error} Throws an error if the example is malformed
     */
    CBExampleToString(example: any): string {
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
     * 
     * Takes a CB example, stringifies it by calling CBExampleToString, and appends it to the log file. Line is logged in an asynchronous manner.
     * 
     * @param {object} example a CB example that will be stringified and appended to the log file
     * @throws {Error} Throws an error if no logging stream has been started
     */
    logCBExampleToStream(example: any) {
        let ex_str = this.CBExampleToString(example);
        this.logLineToStream(ex_str);
    }

    /**
     * 
     * Takes a CB example, stringifies it by calling CBExampleToString, and appends it to the log file. Example is logged in a synchronous manner.
     * Every call to this function will open a new file handle, append the line and close the file handle.
     * 
     * @param {string} log_file the path to the file where the log will be appended to
     * @param {object} example a CB example that will be stringified and appended to the log file
     * @throws {Error} Throws an error if another logging stream has already been started
     */
    logCBExampleSync(log_file: string, example: any) {
        let ex_str = this.CBExampleToString(example);
        this.logLineSync(log_file, ex_str);
    }
};

module.exports = new Promise((resolve) => {
    VWWasmModule().then((moduleInstance: any) => {
        class WorkspaceBase {
            _args_str: string | undefined;
            _instance: any;

            constructor(type: string, { args_str, model_file, model_array }:
                { args_str?: string, model_file?: string, model_array?: [number | undefined, number | undefined] } = {}) {

                let vwModelConstructor = null;
                if (type === ProblemType.All) {
                    vwModelConstructor = moduleInstance.VWModel;
                } else if (type === ProblemType.CB) {
                    vwModelConstructor = moduleInstance.VWCBModel;
                }
                else {
                    throw new Error("Unknown model type");
                }

                let model_array_ptr: number | undefined = undefined;
                let model_array_len: number | undefined = undefined;
                if (model_array !== undefined) {
                    [model_array_ptr, model_array_len] = model_array;
                }

                let model_array_defined = model_array_ptr !== undefined && model_array_len !== undefined && model_array_ptr !== null && model_array_len > 0;

                if (args_str === undefined && model_file === undefined && !model_array_defined) {
                    throw new Error("Can not initialize vw object without args_str or a model_file or a model_array");
                }

                if (model_file !== undefined && model_array_defined) {
                    throw new Error("Can not initialize vw object with both model_file and model_array");
                }

                this._args_str = args_str;
                if (args_str === undefined) {
                    this._args_str = "";
                }

                if (model_file !== undefined) {
                    let modelBuffer = fs.readFileSync(model_file);
                    let ptr = moduleInstance._malloc(modelBuffer.byteLength);
                    let heapBytes = new Uint8Array(moduleInstance.HEAPU8.buffer, ptr, modelBuffer.byteLength);
                    heapBytes.set(new Uint8Array(modelBuffer));

                    this._instance = new vwModelConstructor(this._args_str, ptr, modelBuffer.byteLength);

                    moduleInstance._free(ptr);
                }
                else if (model_array_defined) {
                    this._instance = new vwModelConstructor(this._args_str, model_array_ptr, model_array_len);
                }
                else {
                    this._instance = new vwModelConstructor(this._args_str);
                }

                return this;
            }

            /**
             * Returns the enum value of the prediction type corresponding to the problem type of the model
             * @returns enum value of prediction type
             */
            predictionType() {
                return this._instance.predictionType();
            }

            /**
             * The current total sum of the progressive validation loss
             * 
             * @returns {number} the sum of all losses accumulated by the model
             */
            sumLoss(): number {
                return this._instance.sumLoss();
            }

            /**
             * 
             * Takes a file location and stores the VW model in binary format in the file.
             * 
             * @param {string} model_file the path to the file where the model will be saved 
             */
            saveModelToFile(model_file: string) {
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
             * Gets the VW model in binary format as a Uint8Array that can be saved to a file.
             * There is no need to delete or free the array returned by this function. 
             * If the same array is however used to re-load the model into VW, then the array needs to be stored in wasm memory (see loadModelFromArray)
             * 
             * @returns {Uint8Array} the VW model in binary format
             */
            getModelAsArray(): Uint8Array {
                let char_vector = this._instance.getModel();
                const size = char_vector.size();
                const uint8Array = new Uint8Array(size);
                for (let i = 0; i < size; ++i) {
                    uint8Array[i] = char_vector.get(i);
                }
                char_vector.delete();

                return uint8Array;
            }

            /**
             * 
             * Takes a file location and loads the VW model from the file.
             * 
             * @param {string} model_file the path to the file where the model will be loaded from
             */
            loadModelFromFile(model_file: string) {
                let modelBuffer = fs.readFileSync(model_file);
                let ptr = moduleInstance._malloc(modelBuffer.byteLength);
                let heapBytes = new Uint8Array(moduleInstance.HEAPU8.buffer, ptr, modelBuffer.byteLength);
                heapBytes.set(new Uint8Array(modelBuffer));
                this._instance.loadModelFromBuffer(ptr, modelBuffer.byteLength);
                moduleInstance._free(ptr);
            }

            /**
             * Takes a model in an array binary format and loads it into the VW instance.
             * The memory must be allocated via the WebAssembly module's _malloc function and should later be freed via the _free function.
             * 
             * @param {number} model_array_ptr the pre-loaded model's array pointer
             *  The memory must be allocated via the WebAssembly module's _malloc function and should later be freed via the _free function. 
             * @param {number} model_array_len the pre-loaded model's array length
             */
            loadModelFromArray(model_array_ptr: number, model_array_len: number) {
                this._instance.loadModelFromBuffer(model_array_ptr, model_array_len);
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
             * @param {tuple} [model_array] - The pre-loaded model's array pointer and length (optional).
             *  The memory must be allocated via the WebAssembly module's _malloc function and should later be freed via the _free function.
             * @throws {Error} Throws an error if:
             * - no argument is provided
             * - both string arguments and a model file are provided, and the string arguments and arguments defined in the model clash
             * - both string arguments and a model array are provided, and the string arguments and arguments defined in the model clash
             * - both a model file and a model array are provided
             */
            constructor({ args_str, model_file, model_array }:
                { args_str?: string, model_file?: string, model_array?: [number | undefined, number | undefined] } = {}) {
                super(ProblemType.All, { args_str, model_file, model_array });
            }

            /**
             * Parse a line of text into a VW example. 
             * The example can then be used for prediction or learning. 
             * finishExample() must be called and then delete() on the example, when it is no longer needed.
             * 
             * @param {string} line 
             * @returns a parsed vw example that can be used for prediction or learning
             */
            parse(line: string): object {
                return this._instance.parse(line);
            }

            /**
             * Calls vw predict on the example and returns the prediction.
             * 
             * @param {object} example returned from parse()
             * @returns the prediction with a type corresponding to the reduction that was used
             */
            predict(example: object) {
                return this._instance.predict(example);
            }

            /**
             * Calls vw learn on the example and updates the model
             * 
             * @param {object} example returned from parse()
             */
            learn(example: object) {
                return this._instance.learn(example);
            }

            /**
             * Cleans the example and returns it to the pool of available examples. delete() must also be called on the example object
             * 
             * @param {object} example returned from parse()
             */
            finishExample(example: object) {
                return this._instance.finishExample(example);
            }
        };

        /**
         * A Wrapper around the Wowpal Wabbit C++ library for Contextual Bandit exploration algorithms.
         * @class
         * @extends WorkspaceBase
         */
        class CbWorkspace extends WorkspaceBase {
            /**
             * Creates a new Vowpal Wabbit workspace for Contextual Bandit exploration algorithms.
             * Can accept either or both string arguments and a model file.
             * 
             * @constructor
             * @param {string} [args_str] - The arguments that are used to initialize Vowpal Wabbit (optional)
             * @param {string} [model_file] - The path to the file where the model will be loaded from (optional)
             * @param {tuple} [model_array] - The pre-loaded model's array pointer and length (optional).
             *  The memory must be allocated via the WebAssembly module's _malloc function and should later be freed via the _free function.
             * @throws {Error} Throws an error if:
             * - no argument is provided
             * - both string arguments and a model file are provided, and the string arguments and arguments defined in the model clash
             * - both string arguments and a model array are provided, and the string arguments and arguments defined in the model clash
             * - both a model file and a model array are provided
             */

            _ex: string;
            constructor({ args_str, model_file, model_array }:
                { args_str?: string, model_file?: string, model_array?: [number | undefined, number | undefined] } = {}) {
                super(ProblemType.CB, { args_str, model_file, model_array });
                this._ex = "";
            }

            /**
             * Takes a CB example and returns an array of (action, score) pairs, representing the probability mass function over the available actions
             * The returned pmf can be used with samplePmf to sample an action
             * 
             * Example must have the following properties:
             * - text_context: a string representing the context
             * 
             * @param {object} example the example object that will be used for prediction
             * @returns {array} probability mass function, an array of action,score pairs that was returned by predict
             */
            predict(example: object) {
                return this._instance.predict(example);
            }

            /**
             * Takes a CB example and uses it to update the model
             * 
             * Example must have the following properties:
             * - text_context: a string representing the context
             * - labels: an array of label objects (usually one), each label object must have the following properties:
             *  - action: the action index
             *  - cost: the cost of the action
             *  - probability: the probability of the action
             * 
             * A label object should have more than one labels only if a reduction that accepts multiple labels was used (e.g. graph_feedback)
             * 
             * 
             * @param {object} example the example object that will be used for prediction
             */
            learn(example: object) {
                return this._instance.learn(example);
            }

            /**
             * Accepts a CB example (in text format) line by line. Once a full CB example is passed in it will call learnFromString.
             * This is intended to be used with files that have CB examples, that were logged using logCBExampleToStream and are being read line by line.
             * 
             * @param {string} line a string representing a line from a CB example in text Vowpal Wabbit format
             */
            addLine(line: string) {
                if (line.trim() === '') {
                    this.learnFromString(this._ex);
                    this._ex = "";
                }
                else {
                    this._ex = this._ex + line + "\n";
                }

            }

            /**
             * Takes a full multiline CB example in text format and uses it to update the model. This is intended to be used with examples that are logged to a file using logCBExampleToStream.
             * 
             * @param {string} example a string representing the CB example in text Vowpal Wabbit format
             * @throws {Error} Throws an error if the example is an object with a label and/or a text_context
             */
            learnFromString(example: string) {
                if (example.hasOwnProperty("labels") || example.hasOwnProperty("text_context")) {
                    throw new Error("Example should not have a label or a text_context when using learnFromString, the label and context should just be in the string");
                }

                let ex = {
                    text_context: example
                }

                return this._instance.learnFromString(ex);
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
            samplePmf(pmf: Array<number>): object {
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
            samplePmfWithUUID(pmf: Array<number>, uuid: string): object {
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
            predictAndSample(example: object): object {
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
            predictAndSampleWithUUID(example: object, uuid: string): object {
                let ret = this._instance._predictAndSample(example, uuid);
                ret["uuid"] = uuid;
                return ret;
            }
        };

        function getExceptionMessage(exception: number): string {
            return moduleInstance.getExceptionMessage(exception)
        };

        class Prediction {
            static Type = {
                Scalar: moduleInstance.PredictionType.scalar,
                Scalars: moduleInstance.PredictionType.scalars,
                ActionScores: moduleInstance.PredictionType.action_scores,
                Pdf: moduleInstance.PredictionType.pdf,
                ActionProbs: moduleInstance.PredictionType.action_probs,
                MultiClass: moduleInstance.PredictionType.multiclass,
                MultiLabels: moduleInstance.PredictionType.multilabels,
                Prob: moduleInstance.PredictionType.prob,
                MultiClassProb: moduleInstance.PredictionType.multiclassprob,
                DecisionProbs: moduleInstance.PredictionType.decision_probs,
                ActionPdfValue: moduleInstance.PredictionType.ActionPdfValue,
                ActiveMultiClass: moduleInstance.PredictionType.activeMultiClass,
            };
        };

        resolve(
            {
                Workspace: Workspace,
                CbWorkspace: CbWorkspace,
                Prediction: Prediction,
                VWExampleLogger: VWExampleLogger,
                getExceptionMessage: getExceptionMessage,
                wasmModule: moduleInstance
            }
        )
    })
});
