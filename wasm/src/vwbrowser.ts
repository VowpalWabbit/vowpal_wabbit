import vwModulePromise from './vw.js';

function readSync(model_name: string) {
    console.error("readSync was called in the browser, this behaviour is not supported, do not construct a model with a model file in the browser");
    return [];
}

function writeSync(model_name: string) {
    console.error("writeSync was called in the browser, this behaviour is not supported, do not construct a model with a model file in the browser");
}

export const vwPromise = new Promise((resolve) => {
    vwModulePromise.then((vw: any) => {
        /**
             * ES6 wrapper around the Vowpal Wabbit C++ library.
             * @class
             * @extends vw.CbWorkspace
             */
        class CbWorkspace extends vw.CbWorkspace {
            /**
             * Creates a new Vowpal Wabbit workspace for Contextual Bandit exploration algorithms.
             * Can accept either or both string arguments and a model array.
             * 
             * @constructor
             * @param {string} [args_str] - The arguments that are used to initialize Vowpal Wabbit (optional)
             * @param {tuple} [model_array] - The pre-loaded model's array pointer and length (optional).
             *  The memory must be allocated via the WebAssembly module's _malloc function and should later be freed via the _free function.
             * @throws {Error} Throws an error if:
             * - no argument is provided
             * - both string arguments and a model array are provided, and the string arguments and arguments defined in the model clash
             */

            constructor({ args_str, model_array }:
                { args_str?: string, model_array?: [number | undefined, number | undefined] } = {}) {
                super(readSync, writeSync, { args_str, model_array });
            }
        };

        /**
         * ES6 wrapper around the Vowpal Wabbit C++ library.
         * @class
         * @extends vw.Workspace
         */
        class Workspace extends vw.Workspace {
            /**
             * Creates a new Vowpal Wabbit workspace.
             * Can accept either or both string arguments and a model array.
             * 
             * @constructor
             * @param {string} [args_str] - The arguments that are used to initialize Vowpal Wabbit (optional)
             * @param {tuple} [model_array] - The pre-loaded model's array pointer and length (optional).
             *  The memory must be allocated via the WebAssembly module's _malloc function and should later be freed via the _free function.
             * @throws {Error} Throws an error if:
             * - no argument is provided
             * - both string arguments and a model array are provided, and the string arguments and arguments defined in the model clash
             */
            constructor({ args_str, model_file, model_array }:
                { args_str?: string, model_file?: string, model_array?: [number | undefined, number | undefined] } = {}) {
                super(readSync, writeSync, { args_str, model_file, model_array });
            }
        };

        resolve({ CbWorkspace: CbWorkspace, Workspace: Workspace, Prediction: vw.Prediction, getExceptionMessage: vw.getExceptionMessage, wasmModule: vw.wasmModule });
    })
});