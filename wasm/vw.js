const { log } = require('console');
const fs = require('fs');
const VWWasmModule = require('./out/vw-wasm.js');


// internals

const ModelType =
{
    VWType: 'VW',
    VWCBType: 'VWCB',
};

// exported

class VWBase {
    constructor(type, { args_str, model_file } = {}) {
        if (args_str == undefined) {
            console.error("Can not initialize vw object without args_str");
        }

        if (model_file !== undefined) {
            let modelBuffer = fs.readFileSync(model_file);
            let ptr = VWWasmModule._malloc(modelBuffer.byteLength);
            let heapBytes = new Uint8Array(VWWasmModule.HEAPU8.buffer, ptr, modelBuffer.byteLength);
            heapBytes.set(new Uint8Array(modelBuffer));
            if (type == ModelType.VWType) {
                this._instance = new VWWasmModule.VWModel(args_str, ptr, modelBuffer.byteLength);
            } else if (type == ModelType.VWCBType) {
                this._instance = new VWWasmModule.VWCBModel(args_str, ptr, modelBuffer.byteLength);
            }
            else {
                console.error("Unknown model type");
            }
            VWWasmModule._free(ptr);
        }
        else {
            if (type == ModelType.VWType) {
                this._instance = new VWWasmModule.VWModel(args_str);
            } else if (type == ModelType.VWCBType) {
                this._instance = new VWWasmModule.VWCBModel(args_str);
            }
            else {
                console.error("Unknown model type");
            }
        }


        return new Proxy(this, {
            get(target, propertyName, receiver) {
                if (typeof target._instance[propertyName] == 'function') {
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

class VW extends VWBase {
    constructor({ args_str, model_file } = {}) {
        super(ModelType.VWType, { args_str, model_file });
    }
};

class VWCB extends VWBase {
    constructor({ args_str, model_file } = {}) {
        super(ModelType.VWCBType, { args_str, model_file });
    }
};

function getExceptionMessage(exception) {
    VWWasmModule.getExceptionMessage(exception)
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
                VW: VW,
                VWCB: VWCB,
                Prediction: Prediction,
                getExceptionMessage: getExceptionMessage,
                VWWasmModule : VWWasmModule,
            }
        )
    }
})
