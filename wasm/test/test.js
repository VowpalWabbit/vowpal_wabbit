const assert = require('assert');
const mocha = require('mocha');
const fs = require('fs');
const path = require('path');

const VWModule = require('../out/vw-wasm.js');

// Delay test execution until the WASM VWModule is ready
VWModule['onRuntimeInitialized'] = () => { mocha.run(); }

describe('Call WASM VWModule', () => {
    it('invalid positional argument', () => {
        assert.throws(
            () => new VWModule.VWModel("test"));
    });
    it('Pass argument which is invalid', () => {
        assert.throws(
            () => new VWModule.VWModel("--data data.txt"));
    });
    it('Default construct', () => {
        let model = new VWModule.VWModel("");
        model.delete();
    });

    it('Can clone example', () => {
        let model = new VWModule.VWModel("");
        let ex = model.parse("|a ab");
        let clone = ex.get(0).clone(model);
        model.finishExample(ex);
        ex.delete();
        clone.delete();
        model.delete();
    });

    it('Example toString', () => {
        let model = new VWModule.VWModel("");
        let ex = model.parse("|a ab");
        assert.equal(ex.get(0).toString(), "Example");
        ex.delete();
        model.delete();
    });

    it('Make scalar prediction', () => {
        let model = new VWModule.VWModel("");
        assert.equal(model.predictionType, VWModule.PredictionType.scalar);
        let example = model.parse("|a ab");
        let prediction = model.predict(example);
        assert.equal(typeof prediction, "number");
        model.finishExample(example);
        example.delete();
        model.delete();
    });

    it('Make scalars prediction', () => {
        let model = new VWModule.VWModel("--oaa 2 --probabilities");
        assert.equal(model.predictionType, VWModule.PredictionType.scalars);
        let example = model.parse("|a ab");
        let prediction = model.predict(example);
        assert.equal(typeof prediction, "object");
        assert.equal(prediction.length, 2);
        assert.equal(typeof prediction[0], "number");
        model.finishExample(example);
        example.delete();
        model.delete();
    });

    it('Make action_scores prediction', () => {
        let model = new VWModule.VWModel("--cb_explore_adf");
        assert.equal(model.predictionType, VWModule.PredictionType.action_probs);
        let example = model.parse(`
            shared | s_1 s_2
            0:1.0:0.5 | a_1 b_1 c_1
            | a_2 b_2 c_2
            | a_3 b_3 c_3
        `);
        assert.equal(example.size(), 4);
        let prediction = model.predict(example);
        assert.equal(typeof prediction, "object");
        assert.equal(prediction.length, 3);
        assert(prediction[0].hasOwnProperty('action'));
        assert(prediction[0].hasOwnProperty('score'));
        model.finishExample(example);
        example.delete();
        model.delete();
    });

    it('Make decision_scores prediction', () => {
        let model = new VWModule.VWModel("--ccb_explore_adf");
        assert.equal(model.predictionType, VWModule.PredictionType.decision_probs);
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
        assert.equal(example.size(), 9);
        let prediction = model.predict(example);
        assert.equal(typeof prediction, "object");
        assert.equal(prediction.length, 3);
        assert.equal(prediction[0].length, 5);
        assert(prediction[0][0].hasOwnProperty('action'));
        assert(prediction[0][0].hasOwnProperty('score'));
        model.finishExample(example);
        example.delete();
        model.delete();
    });

    it('Load model', () => {
        let modelBuffer = fs.readFileSync(path.join(__dirname, "model.vw"));
        let ptr = VWModule._malloc(modelBuffer.byteLength);
        let heapBytes = new Uint8Array(VWModule.HEAPU8.buffer, ptr, modelBuffer.byteLength);
        heapBytes.set(new Uint8Array(modelBuffer));
        let model = new VWModule.VWModel("",ptr, modelBuffer.byteLength);
        VWModule._free(ptr);
        assert.equal(model.predictionType, VWModule.PredictionType.scalar);
        let example = model.parse("|f 1470:4.3616120e-02 1494:6.2730476e-02 1511:6.8593867e-02 1556:3.9865732e-02 1577:4.5643266e-02 1821:8.7437980e-02 1862:5.4120179e-02 1888:4.2459391e-02 1910:4.3156520e-02 1967:9.7915463e-02 1972:4.1025959e-02 2008:5.0531935e-02 2018:3.8799949e-02 2088:4.0381286e-02 2128:3.9645299e-02 2168:5.0549522e-02 2175:7.5826041e-02 2183:1.7829052e-01 2234:3.3989199e-02 2270:8.2511209e-02 2281:5.4877985e-02 2316:4.4665784e-02 2322:4.4940550e-02 2477:4.1533679e-02 2533:4.8195656e-02 2588:9.1189362e-02 2701:4.3949336e-02 2861:7.2961919e-02 2932:5.8073092e-02 2992:5.0242696e-02 3077:8.1162862e-02 3110:1.1454716e-01 3170:5.4857526e-02 3263:6.3250236e-02 3325:4.0466305e-02 3491:1.7403087e-01 3690:7.2856687e-02 3858:1.7263067e-01 3973:7.3958009e-02 4037:7.3799074e-02 4492:1.2360445e-01 5166:5.2890390e-02 5483:5.4483801e-02 5484:7.0126176e-02 6086:1.6554411e-01 6171:1.6998538e-01 6858:5.3109396e-02 7157:7.4319251e-02 7502:6.5168351e-02 7626:6.2204096e-02 7904:1.4150701e-01 7905:5.5091526e-02 7909:1.1336020e-01 8224:1.9635071e-01 8376:7.8653499e-02 8568:8.0502190e-02 8665:9.8245032e-02 8954:1.2403890e-01 9111:1.1121018e-01 9636:6.1581194e-02 11789:8.3692431e-02 12145:8.1212714e-02 15171:8.1602275e-02 16066:8.7211892e-02 17940:8.6479917e-02 19892:9.3631372e-02 28774:1.0198968e-01 29080:1.0360513e-01 37508:3.2965177e-01 38026:2.7250558e-01 38027:2.7823427e-01 39870:9.9310391e-02");
        let prediction = model.predict(example);
        console.log(prediction)
        assert.equal(typeof prediction, "number");
        model.finishExample(example);
        example.delete();
        model.delete();
    });
});
