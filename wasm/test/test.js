const assert = require('assert');
const mocha = require('mocha');
const fs = require('fs');
const path = require('path');

const VWModule = require('../out/vw-wasm.js');
const { initializeVWModule, VW, VWCB } = require('../vw.js');

// Delay test execution until the WASM VWModule is ready
initializeVWModule().then(() => { mocha.run(); });

describe('Call WASM VWModule', () => {
    it('invalid positional argument', () => {
        assert.throws(
            () => new VW({ args_str: "test" }));
    });
    it('Pass argument which is invalid', () => {
        assert.throws(
            () => new VW({ args_str: "--data data.txt" }));
    });
    it('Default construct', () => {
        let model = new VW({ args_str: "" });
        model.delete();
    });

    it('Can clone example', () => {
        let model = new VW({ args_str: "" });
        let ex = model.parse("|a ab");
        let clone = ex.get(0).clone(model._instance);
        model.finishExample(ex);
        ex.delete();
        clone.delete();
        model.delete();
    });

    it('Example toString', () => {
        let model = new VW({ args_str: "" });
        let ex = model.parse("|a ab");
        assert.equal(ex.get(0).toString(), "Example");
        ex.delete();
        model.delete();
    });

    it('Make scalar prediction', () => {
        let model = new VW({ args_str: "" });
        assert.equal(model.predictionType(), VWModule.PredictionType.scalar);
        let example = model.parse("|a ab");
        let prediction = model.predict(example);
        assert.equal(typeof prediction, "number");
        model.finishExample(example);
        example.delete();
        model.delete();
    });

    it('Make scalars prediction', () => {
        let model = new VW({ args_str: "--oaa 2 --probabilities" });
        assert.equal(model.predictionType(), VWModule.PredictionType.scalars);
        let example = model.parse("|a ab");
        let prediction = model.predict(example);
        assert.equal(typeof prediction, "object");
        assert.equal(prediction.length, 2);
        assert.equal(typeof prediction[0], "number");
        model.finishExample(example);
        example.delete();
        model.delete();
    });

    it('Make action_scores prediction with generic VW model', () => {
        let model = new VW({ args_str: "--cb_explore_adf" });
        assert.equal(model.predictionType(), VWModule.PredictionType.action_probs);
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

    it('Make action_scores prediction with CBVW', () => {
        let model = new VWCB({ args_str: "--cb_explore_adf" });
        assert.equal(model.predictionType(), VWModule.PredictionType.action_probs);

        let example = {
            text_context: `shared | s_1 s_2
            | a_1 b_1 c_1
            | a_2 b_2 c_2
            | a_3 b_3 c_3
        `};

        let prediction = model.predict(example);
        assert.equal(typeof prediction, "object");
        assert.equal(prediction.length, 3);
        assert(prediction[0].hasOwnProperty('action'));
        assert(prediction[0].hasOwnProperty('score'));

        assert(model.sumLoss() == 0);

        // try learning without setting a label
        try {
            model.learn(example);
        }
        catch (e) {
            console.error('Error caught from C++ during learn:', VWModule.getExceptionMessage(e));
        }

        // try learning with an empty label
        try {
            example.labels = []
            model.learn(example);
        }
        catch (e) {
            console.error('Error caught from C++ during learn:', VWModule.getExceptionMessage(e));
        }

        // try learning with a label that has an action out of bounds
        try {
            example.labels = [{ action: 10, cost: 1.0, probability: 0.5 }]
            model.learn(example);
        }
        catch (e) {
            console.error('Error caught from C++ during learn:', VWModule.getExceptionMessage(e));
        }

        example.labels = [{ action: 0, cost: 1.0, probability: 0.5 }]
        model.learn(example);

        assert(model.sumLoss() > 0);

        model.delete();
    });

    it('Make decision_scores prediction', () => {
        let model = new VW({ args_str: "--ccb_explore_adf" });
        assert.equal(model.predictionType(), VWModule.PredictionType.decision_probs);
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
        let model = new VW({ args_str: "", model_file: path.join(__dirname, "model.vw") });
        assert.equal(model.predictionType(), VWModule.PredictionType.scalar);
        let example = model.parse("|f 6:6.8953723e-02 10:2.4920074e-02 24:1.9822951e-02 50:1.7663756e-02 73:1.6158640e-02 121:5.0723456e-02 157:4.8291773e-02 178:6.0458232e-02 179:2.0943997e-02 188:1.3043258e-02 193:7.4816257e-02 209:4.6250634e-02 233:1.8848978e-02 236:3.4921709e-02 239:2.6007419e-02 264:3.1431116e-02 265:4.3809656e-02 267:3.5724755e-02 276:2.2626529e-02 293:1.9127907e-02 302:2.7269145e-02 307:4.2694855e-02 312:3.1664621e-02 326:2.3426855e-02 368:3.9957818e-02 433:2.1424608e-02 494:3.0670732e-02 506:2.4791485e-02 550:2.8210135e-02 567:4.9445845e-02 617:2.7873196e-02 625:3.2085080e-02 630:3.6478668e-02 631:3.7034698e-02 670:4.1411690e-02 682:4.8788730e-02 702:2.5331981e-02 724:2.4551263e-02 783:4.6463773e-02 817:3.9437063e-02 837:9.0064272e-02 842:1.8598272e-02 848:7.6375939e-02 850:5.0411887e-02 852:7.4332051e-02 855:7.8169920e-02 884:1.1030679e-01 889:9.8633111e-02 894:3.9411522e-02 905:3.7478998e-02 914:5.6504101e-02 949:4.6126790e-02 950:4.5762073e-02 963:3.2610044e-02 979:4.8457999e-02 1000:2.9386828e-02 1045:3.4139425e-02 1059:3.3603869e-02 1061:4.0301725e-02 1066:7.4160680e-02 1071:2.6853660e-02 1073:8.7932266e-02 1081:7.7701092e-02 1117:9.1598287e-02 1123:9.3790986e-02 1131:4.6108399e-02 1132:4.9031150e-02 1162:3.4282148e-02 1170:3.8612958e-02 1177:5.4951586e-02 1178:4.6940666e-02 1188:2.5121527e-02 1189:3.2896131e-02 1191:9.6172296e-02 1211:4.2716283e-02 1237:3.5444438e-02 1240:3.1929389e-02 1247:6.4616486e-02 1311:7.5592339e-02 1342:3.2629944e-02 1366:5.0296519e-02 1416:3.9530758e-02 1417:3.8943492e-02 1470:4.3616120e-02 1494:6.2730476e-02 1511:6.8593867e-02 1556:3.9865732e-02 1577:4.5643266e-02 1821:8.7437980e-02 1862:5.4120179e-02 1888:4.2459391e-02 1910:4.3156520e-02 1967:9.7915463e-02 1972:4.1025959e-02 2008:5.0531935e-02 2018:3.8799949e-02 2088:4.0381286e-02 2128:3.9645299e-02 2168:5.0549522e-02 2175:7.5826041e-02 2183:1.7829052e-01 2234:3.3989199e-02 2270:8.2511209e-02 2281:5.4877985e-02 2316:4.4665784e-02 2322:4.4940550e-02 2477:4.1533679e-02 2533:4.8195656e-02 2588:9.1189362e-02 2701:4.3949336e-02 2861:7.2961919e-02 2932:5.8073092e-02 2992:5.0242696e-02 3077:8.1162862e-02 3110:1.1454716e-01 3170:5.4857526e-02 3263:6.3250236e-02 3325:4.0466305e-02 3491:1.7403087e-01 3690:7.2856687e-02 3858:1.7263067e-01 3973:7.3958009e-02 4037:7.3799074e-02 4492:1.2360445e-01 5166:5.2890390e-02 5483:5.4483801e-02 5484:7.0126176e-02 6086:1.6554411e-01 6171:1.6998538e-01 6858:5.3109396e-02 7157:7.4319251e-02 7502:6.5168351e-02 7626:6.2204096e-02 7904:1.4150701e-01 7905:5.5091526e-02 7909:1.1336020e-01 8224:1.9635071e-01 8376:7.8653499e-02 8568:8.0502190e-02 8665:9.8245032e-02 8954:1.2403890e-01 9111:1.1121018e-01 9636:6.1581194e-02 11789:8.3692431e-02 12145:8.1212714e-02 15171:8.1602275e-02 16066:8.7211892e-02 17940:8.6479917e-02 19892:9.3631372e-02 28774:1.0198968e-01 29080:1.0360513e-01 37508:3.2965177e-01 38026:2.7250558e-01 38027:2.7823427e-01 39870:9.9310391e-02");
        let prediction = model.predict(example);
        assert.equal(typeof prediction, "number");
        model.finishExample(example);
        example.delete();
        model.delete();
    });

    it('Compare save and load model with continuous learning', () => {
        let example = {
            text_context: `shared | s_1 s_2
            | a_1 b_1 c_1
            | a_2 b_2 c_2
            | a_3 b_3 c_3`,
            labels: [{ action: 0, cost: 1.0, probability: 0.5 }]
        };

        let example2 = {
            text_context: `shared | s1_1 s1_2
            | a_1 b_1 c_1
            | a_2 b_2 c_2
            | a_3 b_3 c_3`,
            labels: [{ action: 1, cost: 2.0, probability: 0.8 }]
        };

        let model1 = new VWCB({ args_str: "--cb_explore_adf" });
        assert.equal(model1.predictionType(), VWModule.PredictionType.action_probs);

        model1.learn(example);
        model1.learn(example2);

        // model 2 will save and then load here and continue learning and predicting
        model1.learn(example);
        model1.learn(example2);

        // collect information from model 1
        let model1_sumLoss = model1.sumLoss();
        assert(model1_sumLoss > 0);

        let pred1 = model1.predict(example2);
        assert.equal(pred1.length, 3);
        let model1_action1 = pred1[0]['action'];
        let model1_score1 = pred1[0]['score'];
        let model1_action2 = pred1[1]['action'];
        let model1_score2 = pred1[1]['score'];
        let model1_action3 = pred1[2]['action'];
        let model1_score3 = pred1[2]['score'];

        model1.delete();

        let model2 = new VWCB({ args_str: "--cb_explore_adf" });
        assert.equal(model2.predictionType(), VWModule.PredictionType.action_probs);

        model2.learn(example);
        model2.learn(example2);

        // save load and continue learning and predicting
        let filePath = path.join(__dirname, "save_model.vw");
        model2.saveModel(filePath);
        try {
            model2.loadModel(filePath);
        }
        catch (e) {
            console.error('Error caught while loading saved file:', VWModule.getExceptionMessage(e));
        }
        // DONE save load and continue learning and predicting

        model2.learn(example);
        model2.learn(example2);

        // collect information from model 2
        let model2_sumLoss = model2.sumLoss();
        assert(model2_sumLoss > 0);

        let pred2 = model2.predict(example2);
        assert.equal(pred2.length, 3);
        let model2_action1 = pred2[0]['action'];
        let model2_score1 = pred2[0]['score'];
        let model2_action2 = pred2[1]['action'];
        let model2_score2 = pred2[1]['score'];
        let model2_action3 = pred2[2]['action'];
        let model2_score3 = pred2[2]['score'];

        model2.delete();

        // compare action/scores between the two models
        assert.equal(model1_action1, model2_action1);
        assert.equal(model1_score1, model2_score1);
        assert.equal(model1_action2, model2_action2);
        assert.equal(model1_score2, model2_score2);
        assert.equal(model1_action3, model2_action3);
        assert.equal(model1_score3, model2_score3);

        // cleanup the file

        fs.unlink(filePath, (err) => {
            if (err) {
                console.error('Error removing file:', err);
            } else {
                console.log('File removed successfully:', filePath);
            }
        });
    });

    // it('Check save load of model that clashes with existing arguments', () => {
});
