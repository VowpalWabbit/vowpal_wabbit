const { log } = require('console');
const fs = require('fs');
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
        if (args_str === undefined) {
            throw new Error("Can not initialize vw object without args_str");
        }

        if (model_file !== undefined) {
            let modelBuffer = fs.readFileSync(model_file);
            let ptr = VWWasmModule._malloc(modelBuffer.byteLength);
            let heapBytes = new Uint8Array(VWWasmModule.HEAPU8.buffer, ptr, modelBuffer.byteLength);
            heapBytes.set(new Uint8Array(modelBuffer));
            if (type == ProblemType.All) {
                this._instance = new VWWasmModule.VWModel(args_str, ptr, modelBuffer.byteLength);
            } else if (type == ProblemType.CB) {
                this._instance = new VWWasmModule.VWCBModel(args_str, ptr, modelBuffer.byteLength);
            }
            else {
                throw new Error("Unknown model type");
            }
            VWWasmModule._free(ptr);
        }
        else {
            if (type == ProblemType.All) {
                this._instance = new VWWasmModule.VWModel(args_str);
            } else if (type == ProblemType.CB) {
                this._instance = new VWWasmModule.VWCBModel(args_str);
            }
            else {
                throw new Error("Unknown model type");
            }
        }


        return new Proxy(this, {
            get(target, propertyName, receiver) {
                if (typeof target._instance[propertyName] === 'function') {
                    return target._instance[propertyName].bind(target._instance);
                }

                return Reflect.get(target, propertyName, receiver);
            }
        });
    }

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

    loadModel(model_file) {
        let modelBuffer = fs.readFileSync(model_file);
        let ptr = VWWasmModule._malloc(modelBuffer.byteLength);
        let heapBytes = new Uint8Array(VWWasmModule.HEAPU8.buffer, ptr, modelBuffer.byteLength);
        heapBytes.set(new Uint8Array(modelBuffer));
        this._instance.loadModelFromBuffer(ptr, modelBuffer.byteLength);
        VWWasmModule._free(ptr);
    }

    delete() {
        this._instance.delete();
    }
};

class Workspace extends WorkspaceBase {
    constructor({ args_str, model_file } = {}) {
        super(ProblemType.All, { args_str, model_file });
    }

    logLine(log_file, line) {
        fs.appendFile(log_file, line);
    }

    logLineSync(log_file, line) {
        fs.appendFileSync(log_file, line);
    }
};


function getLineFromExample(example) {
    let context = ""
    if (example.hasOwnProperty('text_context')) {
        context = example.text_context;
    }
    else {
        return { success: false, message: "Can not log example, there is no context available" };
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
                return { success: false, message: "action index out of bounds: " + label.action };
            }

            lines[label.action + indexOffset] = label.action + ":" + label.cost + ":" + label.probability + " " + lines[label.action + indexOffset]
        }
    }
    return { success: true, result: lines.join("\n") };
}

class CbWorkspace extends WorkspaceBase {
    constructor({ args_str, model_file } = {}) {
        super(ProblemType.CB, { args_str, model_file });
    }

    logExample(log_file, example) {

        let res = getLineFromExample(example);
        if (res.success) {
            fs.appendFile(log_file, res.result);
        }
        else {
            console.error("Can not log example: " + res.message);
        }

    }

    logExampleSync(log_file, example) {
        let res = getLineFromExample(example);
        if (res.success) {
            fs.appendFileSync(log_file, res.result);
        }
        else {
            console.error("Can not log example: " + res.message);
        }
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
                // todo hide wasm module?
                VWWasmModule: VWWasmModule,
            }
        )
    }
})
