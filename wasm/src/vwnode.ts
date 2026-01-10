import fs from 'fs';

const vwModule = require('./vw.js').default;

/**
 * A class that helps facilitate the stringification of Vowpal Wabbit examples, and the logging of Vowpal Wabbit examples to a file.
 * Currently available for use in nodejs environments only.
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
     * Closes the logging stream. Logs a warning to the console if there is no logging stream active, but does not throw.
     * Returns a Promise that resolves when the stream is fully closed and flushed.
     * @returns {Promise<void>} A promise that resolves when the stream is closed
     */
    endLogStream(): Promise<void> {
        return new Promise((resolve) => {
            if (this._outputLogStream !== null) {
                const stream = this._outputLogStream;
                stream.end(() => {
                    resolve();
                });
                this._outputLogStream = null;
                this._log_file = null;
            }
            else {
                console.warn("Can not close log, log file is not specified");
                resolve();
            }
        });
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
    CBExampleToString(example: { text_context: string, labels: Array<{ action: number, cost: number, probability: number }> }): string {
        let context = ""
        if (example.hasOwnProperty('text_context')) {
            context = example.text_context;
        }
        else {
            throw new Error("Can not log example, there is no context available");
        }

        const lines = context.trim().split("\n").map((substr) => substr.trim());
        lines.push("");
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
    logCBExampleToStream(example: { text_context: string, labels: Array<{ action: number, cost: number, probability: number }> }) {
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
    logCBExampleSync(log_file: string, example: { text_context: string, labels: Array<{ action: number, cost: number, probability: number }> }) {
        let ex_str = this.CBExampleToString(example);
        this.logLineSync(log_file, ex_str);
    }
};

module.exports = new Promise((resolve) => {
    vwModule.then((vw: any) => {

        /**
         * Nodejs wrapper around the Vowpal Wabbit C++ library.
         * @class
         * @extends vw.CbWorkspace
         */
        class CbWorkspace extends vw.CbWorkspace {
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

            constructor({ args_str, model_file, model_array }:
                { args_str?: string, model_file?: string, model_array?: [number | undefined, number | undefined] } = {}) {
                super(fs.readFileSync, fs.writeFileSync, { args_str, model_file, model_array });
            }
        };

        /**
         * Nodejs wrapper around the Vowpal Wabbit C++ library.
         * @class
         * @extends vw.Workspace
         */
        class Workspace extends vw.Workspace {
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
                super(fs.readFileSync, fs.writeFileSync, { args_str, model_file, model_array });
            }
        };

        resolve(
            {
                Workspace: Workspace,
                CbWorkspace: CbWorkspace,
                Prediction: vw.Prediction,
                VWExampleLogger: VWExampleLogger,
                getExceptionMessage: vw.getExceptionMessage,
                wasmModule: vw.wasmModule,
            }
        )
    })
})