window.BENCHMARK_DATA = {
  "lastUpdate": 1667970505165,
  "repoUrl": "https://github.com/VowpalWabbit/vowpal_wabbit",
  "entries": {
    "Benchmark": [
      {
        "commit": {
          "author": {
            "email": "byronxu@microsoft.com",
            "name": "Byron Xu",
            "username": "byronxu99"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "ea278c49f059b548725b653e82df1920b3180ffd",
          "message": "refactor: Use github-action-benchmark for running benchmarks (#4152)\n\nhttps://github.com/benchmark-action/github-action-benchmark\r\n\r\nThis change simplifies CI jobs for benchmarking and allows benchmark statistics to be automatically tracked over time.",
          "timestamp": "2022-10-03T12:41:53-04:00",
          "tree_id": "2415aa7a729ce7edf63b0c418fe5fa4942a86f3c",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/ea278c49f059b548725b653e82df1920b3180ffd"
        },
        "date": 1664816672769,
        "tool": "googlecpp",
        "benches": [
          {
            "name": "bench_text/120_string_fts",
            "value": 6664.057055078417,
            "unit": "ns/iter",
            "extra": "iterations: 625273\ncpu: 6661.85985961332 ns\nthreads: 1"
          },
          {
            "name": "bench_text/120_num_fts",
            "value": 5355.95483413266,
            "unit": "ns/iter",
            "extra": "iterations: 820354\ncpu: 5355.266994492621 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/8_features",
            "value": 83.8554524160607,
            "unit": "ns/iter",
            "extra": "iterations: 49980531\ncpu: 83.84380310005108 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/1_feature",
            "value": 184.75517935997004,
            "unit": "ns/iter",
            "extra": "iterations: 22532369\ncpu: 184.72102067918377 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features",
            "value": 7864.989773408609,
            "unit": "ns/iter",
            "extra": "iterations: 540356\ncpu: 7863.6922695408175 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features",
            "value": 13653.76900571444,
            "unit": "ns/iter",
            "extra": "iterations: 306592\ncpu: 13651.532003444323 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features_no_predict",
            "value": 3623.6964660377625,
            "unit": "ns/iter",
            "extra": "iterations: 1175819\ncpu: 3623.1780571669574 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features_no_predic",
            "value": 5327.172492569401,
            "unit": "ns/iter",
            "extra": "iterations: 790979\ncpu: 5326.467453623926 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/few_features",
            "value": 1287.1819784848947,
            "unit": "ns/iter",
            "extra": "iterations: 3176370\ncpu: 1287.0105812609993 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/many_features/min_time:15.000",
            "value": 66562.97366815532,
            "unit": "ns/iter",
            "extra": "iterations: 314714\ncpu: 66544.35964081674 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_no_namespaces/min_time:15.000",
            "value": 301798.4374520031,
            "unit": "ns/iter",
            "extra": "iterations: 75526\ncpu: 301752.3660726108 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 324720.6953370543,
            "unit": "ns/iter",
            "extra": "iterations: 66503\ncpu: 324631.75646211463 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_interactions/min_time:15.000",
            "value": 597494.242094336,
            "unit": "ns/iter",
            "extra": "iterations: 35639\ncpu: 597400.3703807621 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_no_interactions/min_time:15.000",
            "value": 298159.4971043968,
            "unit": "ns/iter",
            "extra": "iterations: 70279\ncpu: 298110.8738029854 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_interactions/min_time:15.000",
            "value": 456433.471992935,
            "unit": "ns/iter",
            "extra": "iterations: 46399\ncpu: 456363.3030884288 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_no_namespaces/min_time:15.000",
            "value": 619615.8197096103,
            "unit": "ns/iter",
            "extra": "iterations: 33679\ncpu: 619436.7706879657 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 818138.0316736612,
            "unit": "ns/iter",
            "extra": "iterations: 25005\ncpu: 818010.0339932004 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_interactions/min_time:15.000",
            "value": 2678820.8184595797,
            "unit": "ns/iter",
            "extra": "iterations: 7855\ncpu: 2678399.223424564 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_no_interactions/min_time:15.000",
            "value": 663877.7406668966,
            "unit": "ns/iter",
            "extra": "iterations: 31099\ncpu: 663700.7717289942 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_interactions/min_time:15.000",
            "value": 2048289.1103995382,
            "unit": "ns/iter",
            "extra": "iterations: 10462\ncpu: 2047963.7736570435 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/simple/min_time:15.000",
            "value": 54819.720832290775,
            "unit": "ns/iter",
            "extra": "iterations: 391786\ncpu: 54805.13239370476 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/quadratic/min_time:15.000",
            "value": 3110920.644157009,
            "unit": "ns/iter",
            "extra": "iterations: 6649\ncpu: 3110471.364114903 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_string_fts",
            "value": 1068.6296247389564,
            "unit": "ns/iter",
            "extra": "iterations: 3915221\ncpu: 1068.4941922818678 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_string_fts",
            "value": 7804.014437133793,
            "unit": "ns/iter",
            "extra": "iterations: 538334\ncpu: 7803.032503984545 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_num_fts",
            "value": 913.0453050708237,
            "unit": "ns/iter",
            "extra": "iterations: 4565383\ncpu: 912.9340079463144 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_num_fts",
            "value": 6071.742136079764,
            "unit": "ns/iter",
            "extra": "iterations: 696313\ncpu: 6070.967366687069 ns\nthreads: 1"
          },
          {
            "name": "benchmark_example_reuse",
            "value": 833.3011306930822,
            "unit": "ns/iter",
            "extra": "iterations: 4861620\ncpu: 832.7582986741046 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_char",
            "value": 55.02283105085443,
            "unit": "ns/iter",
            "extra": "iterations: 73120375\ncpu: 55.01601844903033 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_extent",
            "value": 81.79889397663605,
            "unit": "ns/iter",
            "extra": "iterations: 51459130\ncpu: 81.78871465568824 ns\nthreads: 1"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "byronxu@microsoft.com",
            "name": "Byron Xu",
            "username": "byronxu99"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "ea278c49f059b548725b653e82df1920b3180ffd",
          "message": "refactor: Use github-action-benchmark for running benchmarks (#4152)\n\nhttps://github.com/benchmark-action/github-action-benchmark\r\n\r\nThis change simplifies CI jobs for benchmarking and allows benchmark statistics to be automatically tracked over time.",
          "timestamp": "2022-10-03T12:41:53-04:00",
          "tree_id": "2415aa7a729ce7edf63b0c418fe5fa4942a86f3c",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/ea278c49f059b548725b653e82df1920b3180ffd"
        },
        "date": 1664817818098,
        "tool": "benchmarkdotnet",
        "benches": [
          {
            "name": "BenchmarkText.Benchmark(args: 120_num_features)",
            "value": 6574.919535319011,
            "unit": "ns",
            "range": "± 93.79530775545679"
          },
          {
            "name": "BenchmarkText.Benchmark(args: 120_string_fts)",
            "value": 9385.611179896763,
            "unit": "ns",
            "range": "± 143.37241196798277"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 1_feature)",
            "value": 710.3578834533691,
            "unit": "ns",
            "range": "± 18.6482108552604"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 8_features)",
            "value": 641.152993370505,
            "unit": "ns",
            "range": "± 12.904914214749065"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_interactions)",
            "value": 1099037.3697916667,
            "unit": "ns",
            "range": "± 26063.382319407472"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_no_interactions)",
            "value": 683385.8268229166,
            "unit": "ns",
            "range": "± 9129.353305687768"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_no_namespaces)",
            "value": 672665.7552083334,
            "unit": "ns",
            "range": "± 10895.296152796116"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_interactions)",
            "value": 1075119.7823660714,
            "unit": "ns",
            "range": "± 12263.979786500646"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_no_interactions)",
            "value": 671461.0807291666,
            "unit": "ns",
            "range": "± 10392.732118005928"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_interactions)",
            "value": 4835329.541015625,
            "unit": "ns",
            "range": "± 93745.30218447442"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_no_interactions)",
            "value": 1334014.658203125,
            "unit": "ns",
            "range": "± 30514.80086892214"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_no_namespaces)",
            "value": 1170004.443359375,
            "unit": "ns",
            "range": "± 21738.567949837554"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_interactions)",
            "value": 4503955.625,
            "unit": "ns",
            "range": "± 78020.37972632668"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_no_interactions)",
            "value": 1161263.0598958333,
            "unit": "ns",
            "range": "± 20377.41208332837"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: few_features)",
            "value": 3874.462089538574,
            "unit": "ns",
            "range": "± 87.08666629364397"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: few_features)",
            "value": 13627.952372233072,
            "unit": "ns",
            "range": "± 238.41235278342248"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: many_features)",
            "value": 166510.9119762074,
            "unit": "ns",
            "range": "± 3927.155439388778"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: many_features)",
            "value": 23752.81195389597,
            "unit": "ns",
            "range": "± 498.6880839253683"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: quadratic)",
            "value": 6276986.049107143,
            "unit": "ns",
            "range": "± 76197.28994950146"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: simple)",
            "value": 297574.1341145833,
            "unit": "ns",
            "range": "± 4240.456062710992"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "griffinbassman@gmail.com",
            "name": "Griffin Bassman",
            "username": "bassmang"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "4e58863e2dcf85152dbce6fc1e19c212cab814e9",
          "message": "fix: one_of for loss_option (#4178)",
          "timestamp": "2022-10-03T16:16:57-04:00",
          "tree_id": "124b790b4fdfb296febb11354100887a734f030c",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/4e58863e2dcf85152dbce6fc1e19c212cab814e9"
        },
        "date": 1664829629821,
        "tool": "googlecpp",
        "benches": [
          {
            "name": "bench_text/120_string_fts",
            "value": 7320.13336314848,
            "unit": "ns/iter",
            "extra": "iterations: 572422\ncpu: 7319.435311710592 ns\nthreads: 1"
          },
          {
            "name": "bench_text/120_num_fts",
            "value": 5816.502452499062,
            "unit": "ns/iter",
            "extra": "iterations: 726198\ncpu: 5816.03336831002 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/8_features",
            "value": 89.18590218869623,
            "unit": "ns/iter",
            "extra": "iterations: 47588692\ncpu: 89.17846701901367 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/1_feature",
            "value": 196.7572468352743,
            "unit": "ns/iter",
            "extra": "iterations: 21559204\ncpu: 196.7309507345449 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features",
            "value": 8077.321752951884,
            "unit": "ns/iter",
            "extra": "iterations: 515268\ncpu: 8076.307474945075 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features",
            "value": 14555.592219733504,
            "unit": "ns/iter",
            "extra": "iterations: 286674\ncpu: 14553.763159547074 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features_no_predict",
            "value": 3869.2817299213457,
            "unit": "ns/iter",
            "extra": "iterations: 1097576\ncpu: 3868.9325386123624 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features_no_predic",
            "value": 5877.642401204957,
            "unit": "ns/iter",
            "extra": "iterations: 711093\ncpu: 5877.09315096618 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/few_features",
            "value": 1421.5449695955238,
            "unit": "ns/iter",
            "extra": "iterations: 2991499\ncpu: 1421.4313292433 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/many_features/min_time:15.000",
            "value": 75573.02661670798,
            "unit": "ns/iter",
            "extra": "iterations: 278096\ncpu: 75565.39324549794 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_no_namespaces/min_time:15.000",
            "value": 341612.81387536804,
            "unit": "ns/iter",
            "extra": "iterations: 64301\ncpu: 341575.5804730873 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 367724.56851705624,
            "unit": "ns/iter",
            "extra": "iterations: 57015\ncpu: 367682.5098658248 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_interactions/min_time:15.000",
            "value": 658756.2815937531,
            "unit": "ns/iter",
            "extra": "iterations: 32000\ncpu: 658688.3218750001 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_no_interactions/min_time:15.000",
            "value": 321590.72797098145,
            "unit": "ns/iter",
            "extra": "iterations: 63405\ncpu: 321457.58220960473 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_interactions/min_time:15.000",
            "value": 490058.87978039164,
            "unit": "ns/iter",
            "extra": "iterations: 42622\ncpu: 490009.8493735625 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_no_namespaces/min_time:15.000",
            "value": 673179.8891642861,
            "unit": "ns/iter",
            "extra": "iterations: 31470\ncpu: 673098.646329838 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 890260.4882275334,
            "unit": "ns/iter",
            "extra": "iterations: 23487\ncpu: 890145.7274236825 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_interactions/min_time:15.000",
            "value": 2924207.582399311,
            "unit": "ns/iter",
            "extra": "iterations: 7227\ncpu: 2923878.0130067808 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_no_interactions/min_time:15.000",
            "value": 702658.0276959684,
            "unit": "ns/iter",
            "extra": "iterations: 30221\ncpu: 702574.0776281384 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_interactions/min_time:15.000",
            "value": 2151056.5765965874,
            "unit": "ns/iter",
            "extra": "iterations: 9896\ncpu: 2150817.168552951 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/simple/min_time:15.000",
            "value": 59057.288512590676,
            "unit": "ns/iter",
            "extra": "iterations: 355215\ncpu: 59051.331728671335 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/quadratic/min_time:15.000",
            "value": 3407671.474493121,
            "unit": "ns/iter",
            "extra": "iterations: 6116\ncpu: 3407294.882276 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_string_fts",
            "value": 1249.788939315599,
            "unit": "ns/iter",
            "extra": "iterations: 3356248\ncpu: 1249.6708228950897 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_string_fts",
            "value": 8530.891347916342,
            "unit": "ns/iter",
            "extra": "iterations: 490980\ncpu: 8530.08432115367 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_num_fts",
            "value": 1036.9219797539447,
            "unit": "ns/iter",
            "extra": "iterations: 4079800\ncpu: 1036.8225403206102 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_num_fts",
            "value": 6494.501805232801,
            "unit": "ns/iter",
            "extra": "iterations: 646454\ncpu: 6493.910471588043 ns\nthreads: 1"
          },
          {
            "name": "benchmark_example_reuse",
            "value": 909.0014790111068,
            "unit": "ns/iter",
            "extra": "iterations: 4647024\ncpu: 908.9126288136188 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_char",
            "value": 60.095677950776924,
            "unit": "ns/iter",
            "extra": "iterations: 69533868\ncpu: 60.0903289315072 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_extent",
            "value": 85.12064730520868,
            "unit": "ns/iter",
            "extra": "iterations: 48527093\ncpu: 85.11096883549175 ns\nthreads: 1"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "griffinbassman@gmail.com",
            "name": "Griffin Bassman",
            "username": "bassmang"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "4e58863e2dcf85152dbce6fc1e19c212cab814e9",
          "message": "fix: one_of for loss_option (#4178)",
          "timestamp": "2022-10-03T16:16:57-04:00",
          "tree_id": "124b790b4fdfb296febb11354100887a734f030c",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/4e58863e2dcf85152dbce6fc1e19c212cab814e9"
        },
        "date": 1664830598606,
        "tool": "benchmarkdotnet",
        "benches": [
          {
            "name": "BenchmarkText.Benchmark(args: 120_num_features)",
            "value": 5188.4843826293945,
            "unit": "ns",
            "range": "± 16.478231459295028"
          },
          {
            "name": "BenchmarkText.Benchmark(args: 120_string_fts)",
            "value": 7193.40814443735,
            "unit": "ns",
            "range": "± 35.89625667208664"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 1_feature)",
            "value": 649.9236806233724,
            "unit": "ns",
            "range": "± 4.387319100343567"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 8_features)",
            "value": 546.8599573771158,
            "unit": "ns",
            "range": "± 9.146037631058588"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_interactions)",
            "value": 849652.0052083334,
            "unit": "ns",
            "range": "± 1520.293121983569"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_no_interactions)",
            "value": 463930.1041666667,
            "unit": "ns",
            "range": "± 834.502985190256"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_no_namespaces)",
            "value": 458779.2513020833,
            "unit": "ns",
            "range": "± 1172.113746151115"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_interactions)",
            "value": 848966.3499098558,
            "unit": "ns",
            "range": "± 1246.699611315346"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_no_interactions)",
            "value": 463405.30724158656,
            "unit": "ns",
            "range": "± 818.6499766010093"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_interactions)",
            "value": 3475665.2083333335,
            "unit": "ns",
            "range": "± 5104.071762747823"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_no_interactions)",
            "value": 1036662.5279017857,
            "unit": "ns",
            "range": "± 3255.5378650776365"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_no_namespaces)",
            "value": 865801.5071614584,
            "unit": "ns",
            "range": "± 1297.129996551881"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_interactions)",
            "value": 2768778.4765625,
            "unit": "ns",
            "range": "± 3437.5886815113213"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_no_interactions)",
            "value": 885597.0768229166,
            "unit": "ns",
            "range": "± 2063.19142479935"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: few_features)",
            "value": 3374.2903573172434,
            "unit": "ns",
            "range": "± 2.9801269026365405"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: few_features)",
            "value": 11423.018704927885,
            "unit": "ns",
            "range": "± 21.629627677584367"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: many_features)",
            "value": 125845.59122721355,
            "unit": "ns",
            "range": "± 163.9171255028683"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: many_features)",
            "value": 19645.140729631697,
            "unit": "ns",
            "range": "± 20.09984271615676"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: quadratic)",
            "value": 4851986.614583333,
            "unit": "ns",
            "range": "± 15770.764036006305"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: simple)",
            "value": 204621.3826497396,
            "unit": "ns",
            "range": "± 1948.073958401032"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "byronxu@microsoft.com",
            "name": "Byron Xu",
            "username": "byronxu99"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "a9d2bd04c0d25d4de4af2599b9551ec547902f9b",
          "message": "docs: Update readme for benchmarks (#4181)",
          "timestamp": "2022-10-04T15:09:23Z",
          "tree_id": "da4ae8115bd14a2697b025930d7fc786876e77aa",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/a9d2bd04c0d25d4de4af2599b9551ec547902f9b"
        },
        "date": 1664897979567,
        "tool": "googlecpp",
        "benches": [
          {
            "name": "bench_text/120_string_fts",
            "value": 7388.286077977097,
            "unit": "ns/iter",
            "extra": "iterations: 567985\ncpu: 7387.322200410221 ns\nthreads: 1"
          },
          {
            "name": "bench_text/120_num_fts",
            "value": 5887.008118274982,
            "unit": "ns/iter",
            "extra": "iterations: 709264\ncpu: 5886.461317647589 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/8_features",
            "value": 89.72654283571929,
            "unit": "ns/iter",
            "extra": "iterations: 46874336\ncpu: 89.71954290723178 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/1_feature",
            "value": 199.3896221029398,
            "unit": "ns/iter",
            "extra": "iterations: 21377568\ncpu: 199.37401204851736 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features",
            "value": 8222.56366855956,
            "unit": "ns/iter",
            "extra": "iterations: 506553\ncpu: 8221.654200054089 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features",
            "value": 14755.542776807286,
            "unit": "ns/iter",
            "extra": "iterations: 285587\ncpu: 14753.937329080112 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features_no_predict",
            "value": 3931.4925588171072,
            "unit": "ns/iter",
            "extra": "iterations: 1063339\ncpu: 3928.99743167513 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features_no_predic",
            "value": 5949.482540999007,
            "unit": "ns/iter",
            "extra": "iterations: 713672\ncpu: 5948.990852940845 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/few_features",
            "value": 1426.7264603075164,
            "unit": "ns/iter",
            "extra": "iterations: 2937241\ncpu: 1426.5987026600794 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/many_features/min_time:15.000",
            "value": 73346.79098718676,
            "unit": "ns/iter",
            "extra": "iterations: 285083\ncpu: 73339.81822837559 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_no_namespaces/min_time:15.000",
            "value": 321459.5306190657,
            "unit": "ns/iter",
            "extra": "iterations: 65825\ncpu: 321380.5453854918 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 352963.9031606609,
            "unit": "ns/iter",
            "extra": "iterations: 59418\ncpu: 352915.00555387256 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_interactions/min_time:15.000",
            "value": 649856.280665344,
            "unit": "ns/iter",
            "extra": "iterations: 32284\ncpu: 649790.8375665965 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_no_interactions/min_time:15.000",
            "value": 322790.97341830283,
            "unit": "ns/iter",
            "extra": "iterations: 65120\ncpu: 322691.13175675663 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_interactions/min_time:15.000",
            "value": 504595.80334969366,
            "unit": "ns/iter",
            "extra": "iterations: 41556\ncpu: 504545.44710751757 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_no_namespaces/min_time:15.000",
            "value": 692002.4590326101,
            "unit": "ns/iter",
            "extra": "iterations: 30329\ncpu: 691841.9070856277 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 902975.6705152489,
            "unit": "ns/iter",
            "extra": "iterations: 23212\ncpu: 902875.8271583666 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_interactions/min_time:15.000",
            "value": 2967945.3044948536,
            "unit": "ns/iter",
            "extra": "iterations: 7097\ncpu: 2967221.7979427907 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_no_interactions/min_time:15.000",
            "value": 719600.8446569014,
            "unit": "ns/iter",
            "extra": "iterations: 29496\ncpu: 719525.396663955 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_interactions/min_time:15.000",
            "value": 2260537.625291767,
            "unit": "ns/iter",
            "extra": "iterations: 9426\ncpu: 2259987.470825373 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/simple/min_time:15.000",
            "value": 59738.20402127013,
            "unit": "ns/iter",
            "extra": "iterations: 349591\ncpu: 59732.651870328504 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/quadratic/min_time:15.000",
            "value": 3283415.9187234472,
            "unit": "ns/iter",
            "extra": "iterations: 6361\ncpu: 3282588.570979399 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_string_fts",
            "value": 1265.617104893312,
            "unit": "ns/iter",
            "extra": "iterations: 3319097\ncpu: 1265.25847240981 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_string_fts",
            "value": 8736.273753533409,
            "unit": "ns/iter",
            "extra": "iterations: 480739\ncpu: 8735.46831024736 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_num_fts",
            "value": 1051.9099568340027,
            "unit": "ns/iter",
            "extra": "iterations: 3992263\ncpu: 1051.8241909413334 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_num_fts",
            "value": 6620.862975102661,
            "unit": "ns/iter",
            "extra": "iterations: 632469\ncpu: 6620.370326450806 ns\nthreads: 1"
          },
          {
            "name": "benchmark_example_reuse",
            "value": 925.5769428260336,
            "unit": "ns/iter",
            "extra": "iterations: 4549378\ncpu: 925.4999035032959 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_char",
            "value": 61.21073516004309,
            "unit": "ns/iter",
            "extra": "iterations: 68713759\ncpu: 61.206491410257264 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_extent",
            "value": 86.41075239475954,
            "unit": "ns/iter",
            "extra": "iterations: 48609097\ncpu: 86.40431440230212 ns\nthreads: 1"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "griffinbassman@gmail.com",
            "name": "Griffin Bassman",
            "username": "bassmang"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "befbd6674f31adceb15299f3f581f86ec6eabea2",
          "message": "ci: check model weights for gd-based tests for forward and backward compat (#4172)\n\n* ci: check model weights for gd-based tests\r\n\r\n* pytho lint\r\n\r\n* unkeep options\r\n\r\n* dirs\r\n\r\n* upload paths\r\n\r\n* address comments",
          "timestamp": "2022-10-04T14:39:45-04:00",
          "tree_id": "618ea9f209f5ddafb8352e87a1978a8f3a8492b5",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/befbd6674f31adceb15299f3f581f86ec6eabea2"
        },
        "date": 1664910014619,
        "tool": "googlecpp",
        "benches": [
          {
            "name": "bench_text/120_string_fts",
            "value": 5943.556442678041,
            "unit": "ns/iter",
            "extra": "iterations: 700392\ncpu: 5940.96520234383 ns\nthreads: 1"
          },
          {
            "name": "bench_text/120_num_fts",
            "value": 4121.591773446498,
            "unit": "ns/iter",
            "extra": "iterations: 1026724\ncpu: 4121.391922269277 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/8_features",
            "value": 69.2225753674439,
            "unit": "ns/iter",
            "extra": "iterations: 61165268\ncpu: 69.21981932622286 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/1_feature",
            "value": 160.0017815815918,
            "unit": "ns/iter",
            "extra": "iterations: 26503417\ncpu: 159.994030203728 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features",
            "value": 6866.056729151899,
            "unit": "ns/iter",
            "extra": "iterations: 611132\ncpu: 6865.680245838868 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features",
            "value": 12446.035238566765,
            "unit": "ns/iter",
            "extra": "iterations: 339486\ncpu: 12445.376539827854 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features_no_predict",
            "value": 3313.3483968148107,
            "unit": "ns/iter",
            "extra": "iterations: 1270003\ncpu: 3313.1958743404534 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features_no_predic",
            "value": 5098.447564805639,
            "unit": "ns/iter",
            "extra": "iterations: 827860\ncpu: 5098.195709419467 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/few_features",
            "value": 1211.2996152922735,
            "unit": "ns/iter",
            "extra": "iterations: 3356314\ncpu: 1211.2451040039762 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/many_features/min_time:15.000",
            "value": 54547.16305293449,
            "unit": "ns/iter",
            "extra": "iterations: 393130\ncpu: 54539.843563197916 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_no_namespaces/min_time:15.000",
            "value": 192252.76222424908,
            "unit": "ns/iter",
            "extra": "iterations: 110743\ncpu: 192242.96795282772 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 220004.64589529976,
            "unit": "ns/iter",
            "extra": "iterations: 95147\ncpu: 219993.1232724101 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_interactions/min_time:15.000",
            "value": 454961.5872328393,
            "unit": "ns/iter",
            "extra": "iterations: 45946\ncpu: 454938.42336656107 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_no_interactions/min_time:15.000",
            "value": 191150.89096501668,
            "unit": "ns/iter",
            "extra": "iterations: 108910\ncpu: 191126.7321641722 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_interactions/min_time:15.000",
            "value": 320933.4436454501,
            "unit": "ns/iter",
            "extra": "iterations: 65221\ncpu: 320890.4447953879 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_no_namespaces/min_time:15.000",
            "value": 530340.7781127323,
            "unit": "ns/iter",
            "extra": "iterations: 39475\ncpu: 530306.7941735275 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 672061.644248301,
            "unit": "ns/iter",
            "extra": "iterations: 31269\ncpu: 671962.9537241353 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_interactions/min_time:15.000",
            "value": 2099204.2946178378,
            "unit": "ns/iter",
            "extra": "iterations: 9996\ncpu: 2099093.517406964 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_no_interactions/min_time:15.000",
            "value": 544034.2747648181,
            "unit": "ns/iter",
            "extra": "iterations: 38906\ncpu: 543957.8522592917 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_interactions/min_time:15.000",
            "value": 1622676.936632899,
            "unit": "ns/iter",
            "extra": "iterations: 12830\ncpu: 1622590.5611847257 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/simple/min_time:15.000",
            "value": 47363.54653726878,
            "unit": "ns/iter",
            "extra": "iterations: 442151\ncpu: 47361.79246456532 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/quadratic/min_time:15.000",
            "value": 2177529.2272202997,
            "unit": "ns/iter",
            "extra": "iterations: 9537\ncpu: 2177199.3079584753 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_string_fts",
            "value": 898.5221812583584,
            "unit": "ns/iter",
            "extra": "iterations: 4672954\ncpu: 898.4853264123751 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_string_fts",
            "value": 6996.810744082435,
            "unit": "ns/iter",
            "extra": "iterations: 600256\ncpu: 6996.465674645431 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_num_fts",
            "value": 688.9132950247802,
            "unit": "ns/iter",
            "extra": "iterations: 6109338\ncpu: 688.8854569840437 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_num_fts",
            "value": 4691.777790686081,
            "unit": "ns/iter",
            "extra": "iterations: 895210\ncpu: 4691.592810625423 ns\nthreads: 1"
          },
          {
            "name": "benchmark_example_reuse",
            "value": 767.1411225188582,
            "unit": "ns/iter",
            "extra": "iterations: 5502304\ncpu: 767.1075789342036 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_char",
            "value": 41.18596477820425,
            "unit": "ns/iter",
            "extra": "iterations: 102272071\ncpu: 41.184317075186726 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_extent",
            "value": 54.18934099077354,
            "unit": "ns/iter",
            "extra": "iterations: 77628716\ncpu: 54.18738859470495 ns\nthreads: 1"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "griffinbassman@gmail.com",
            "name": "Griffin Bassman",
            "username": "bassmang"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "befbd6674f31adceb15299f3f581f86ec6eabea2",
          "message": "ci: check model weights for gd-based tests for forward and backward compat (#4172)\n\n* ci: check model weights for gd-based tests\r\n\r\n* pytho lint\r\n\r\n* unkeep options\r\n\r\n* dirs\r\n\r\n* upload paths\r\n\r\n* address comments",
          "timestamp": "2022-10-04T14:39:45-04:00",
          "tree_id": "618ea9f209f5ddafb8352e87a1978a8f3a8492b5",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/befbd6674f31adceb15299f3f581f86ec6eabea2"
        },
        "date": 1664911229430,
        "tool": "benchmarkdotnet",
        "benches": [
          {
            "name": "BenchmarkText.Benchmark(args: 120_num_features)",
            "value": 6434.21396108774,
            "unit": "ns",
            "range": "± 20.103881366843307"
          },
          {
            "name": "BenchmarkText.Benchmark(args: 120_string_fts)",
            "value": 9378.543090820312,
            "unit": "ns",
            "range": "± 24.422060586231186"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 1_feature)",
            "value": 660.5423545837402,
            "unit": "ns",
            "range": "± 3.4651769897038918"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 8_features)",
            "value": 552.727108001709,
            "unit": "ns",
            "range": "± 4.945756734431909"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_interactions)",
            "value": 916719.4921875,
            "unit": "ns",
            "range": "± 3103.718426032815"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_no_interactions)",
            "value": 525248.2356770834,
            "unit": "ns",
            "range": "± 1834.1968484094086"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_no_namespaces)",
            "value": 541102.8515625,
            "unit": "ns",
            "range": "± 2642.1213384441567"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_interactions)",
            "value": 910982.5260416666,
            "unit": "ns",
            "range": "± 2447.8186122870407"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_no_interactions)",
            "value": 554276.7447916666,
            "unit": "ns",
            "range": "± 2603.875827695249"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_interactions)",
            "value": 3489410.6584821427,
            "unit": "ns",
            "range": "± 5296.229255892744"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_no_interactions)",
            "value": 1071597.3407451923,
            "unit": "ns",
            "range": "± 1534.8182665460527"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_no_namespaces)",
            "value": 913576.2369791666,
            "unit": "ns",
            "range": "± 2343.6287853079534"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_interactions)",
            "value": 2849836.3020833335,
            "unit": "ns",
            "range": "± 11231.310557994597"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_no_interactions)",
            "value": 921135.0065104166,
            "unit": "ns",
            "range": "± 1359.4764693483994"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: few_features)",
            "value": 3276.9007001604355,
            "unit": "ns",
            "range": "± 7.674328501705692"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: few_features)",
            "value": 11499.77783203125,
            "unit": "ns",
            "range": "± 14.950774974988413"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: many_features)",
            "value": 108948.30403645833,
            "unit": "ns",
            "range": "± 333.74119363995686"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: many_features)",
            "value": 19559.344278971355,
            "unit": "ns",
            "range": "± 62.31289142543073"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: quadratic)",
            "value": 4345319.0625,
            "unit": "ns",
            "range": "± 37956.82550591852"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: simple)",
            "value": 212935.67068917412,
            "unit": "ns",
            "range": "± 1158.3339292176852"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "jackgerrits@users.noreply.github.com",
            "name": "Jack Gerrits",
            "username": "jackgerrits"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "a6eb7a1518005fbfe929359ef73784775eedb0d5",
          "message": "style: resolve style issues in allreduce project (#4187)",
          "timestamp": "2022-10-05T12:02:57-04:00",
          "tree_id": "4846f1caa404468564ca899189683dc5ec901c01",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/a6eb7a1518005fbfe929359ef73784775eedb0d5"
        },
        "date": 1664987585311,
        "tool": "googlecpp",
        "benches": [
          {
            "name": "bench_text/120_string_fts",
            "value": 7268.485547578826,
            "unit": "ns/iter",
            "extra": "iterations: 572776\ncpu: 7267.629754039973 ns\nthreads: 1"
          },
          {
            "name": "bench_text/120_num_fts",
            "value": 5757.34713889385,
            "unit": "ns/iter",
            "extra": "iterations: 738543\ncpu: 5756.821742268221 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/8_features",
            "value": 88.43264893503115,
            "unit": "ns/iter",
            "extra": "iterations: 47261027\ncpu: 88.3603460415704 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/1_feature",
            "value": 195.50935604952747,
            "unit": "ns/iter",
            "extra": "iterations: 21780293\ncpu: 195.4911028974679 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features",
            "value": 8246.44229717618,
            "unit": "ns/iter",
            "extra": "iterations: 504724\ncpu: 8244.321847187768 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features",
            "value": 14786.909916384011,
            "unit": "ns/iter",
            "extra": "iterations: 282482\ncpu: 14785.183480717353 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features_no_predict",
            "value": 3926.1679705653723,
            "unit": "ns/iter",
            "extra": "iterations: 1093132\ncpu: 3923.8109395754605 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features_no_predic",
            "value": 5921.712222271253,
            "unit": "ns/iter",
            "extra": "iterations: 702709\ncpu: 5921.107456998556 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/few_features",
            "value": 1442.8844839584583,
            "unit": "ns/iter",
            "extra": "iterations: 2942864\ncpu: 1442.7389441034309 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/many_features/min_time:15.000",
            "value": 74479.47096650743,
            "unit": "ns/iter",
            "extra": "iterations: 282243\ncpu: 74471.87211020292 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_no_namespaces/min_time:15.000",
            "value": 328457.1725513315,
            "unit": "ns/iter",
            "extra": "iterations: 64433\ncpu: 328378.444275449 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 361530.25626419496,
            "unit": "ns/iter",
            "extra": "iterations: 58108\ncpu: 361459.1398774697 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_interactions/min_time:15.000",
            "value": 657892.511527199,
            "unit": "ns/iter",
            "extra": "iterations: 32098\ncpu: 657819.826780485 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_no_interactions/min_time:15.000",
            "value": 330429.6881850229,
            "unit": "ns/iter",
            "extra": "iterations: 63775\ncpu: 330393.9098392788 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_interactions/min_time:15.000",
            "value": 512027.4003910551,
            "unit": "ns/iter",
            "extra": "iterations: 40915\ncpu: 511971.9491628987 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_no_namespaces/min_time:15.000",
            "value": 694911.6196300697,
            "unit": "ns/iter",
            "extra": "iterations: 30168\ncpu: 694752.6617608061 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 898535.3804463797,
            "unit": "ns/iter",
            "extra": "iterations: 23433\ncpu: 898434.1484231644 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_interactions/min_time:15.000",
            "value": 2874586.1858839723,
            "unit": "ns/iter",
            "extra": "iterations: 7155\ncpu: 2873179.245283025 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_no_interactions/min_time:15.000",
            "value": 687533.802287372,
            "unit": "ns/iter",
            "extra": "iterations: 31040\ncpu: 687290.5927835057 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_interactions/min_time:15.000",
            "value": 2056473.7942985506,
            "unit": "ns/iter",
            "extra": "iterations: 10243\ncpu: 2056003.1924240934 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/simple/min_time:15.000",
            "value": 56254.05652286636,
            "unit": "ns/iter",
            "extra": "iterations: 378024\ncpu: 56248.92546504985 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/quadratic/min_time:15.000",
            "value": 3248804.657943915,
            "unit": "ns/iter",
            "extra": "iterations: 6420\ncpu: 3247904.8753894065 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_string_fts",
            "value": 1242.6747675256981,
            "unit": "ns/iter",
            "extra": "iterations: 3369190\ncpu: 1242.5759603940337 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_string_fts",
            "value": 8564.499298193248,
            "unit": "ns/iter",
            "extra": "iterations: 484464\ncpu: 8563.826414346622 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_num_fts",
            "value": 1033.5242423493987,
            "unit": "ns/iter",
            "extra": "iterations: 4050680\ncpu: 1033.440582815724 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_num_fts",
            "value": 6485.975904562888,
            "unit": "ns/iter",
            "extra": "iterations: 648048\ncpu: 6485.384107350113 ns\nthreads: 1"
          },
          {
            "name": "benchmark_example_reuse",
            "value": 904.950279937502,
            "unit": "ns/iter",
            "extra": "iterations: 4597098\ncpu: 904.8757063695452 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_char",
            "value": 61.36369518979462,
            "unit": "ns/iter",
            "extra": "iterations: 70452576\ncpu: 61.35872448439622 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_extent",
            "value": 85.21996963297765,
            "unit": "ns/iter",
            "extra": "iterations: 48984716\ncpu: 85.21405125631402 ns\nthreads: 1"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "jackgerrits@users.noreply.github.com",
            "name": "Jack Gerrits",
            "username": "jackgerrits"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "a6eb7a1518005fbfe929359ef73784775eedb0d5",
          "message": "style: resolve style issues in allreduce project (#4187)",
          "timestamp": "2022-10-05T12:02:57-04:00",
          "tree_id": "4846f1caa404468564ca899189683dc5ec901c01",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/a6eb7a1518005fbfe929359ef73784775eedb0d5"
        },
        "date": 1664988605302,
        "tool": "benchmarkdotnet",
        "benches": [
          {
            "name": "BenchmarkText.Benchmark(args: 120_num_features)",
            "value": 6548.132148155799,
            "unit": "ns",
            "range": "± 23.404743646527535"
          },
          {
            "name": "BenchmarkText.Benchmark(args: 120_string_fts)",
            "value": 10282.885335286459,
            "unit": "ns",
            "range": "± 51.52096950565708"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 1_feature)",
            "value": 675.3321520487467,
            "unit": "ns",
            "range": "± 2.3883673246171853"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 8_features)",
            "value": 595.2848307291666,
            "unit": "ns",
            "range": "± 2.729084467291612"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_interactions)",
            "value": 914391.2760416666,
            "unit": "ns",
            "range": "± 4209.216291468806"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_no_interactions)",
            "value": 573590.2018229166,
            "unit": "ns",
            "range": "± 3057.733772516154"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_no_namespaces)",
            "value": 567925.2408854166,
            "unit": "ns",
            "range": "± 3300.060303221013"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_interactions)",
            "value": 914311.4518229166,
            "unit": "ns",
            "range": "± 2974.645811400645"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_no_interactions)",
            "value": 576020.4622395834,
            "unit": "ns",
            "range": "± 4383.59242397916"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_interactions)",
            "value": 3536917.8850446427,
            "unit": "ns",
            "range": "± 6564.906784198728"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_no_interactions)",
            "value": 1107609.8214285714,
            "unit": "ns",
            "range": "± 5864.729815982196"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_no_namespaces)",
            "value": 925329.4503348215,
            "unit": "ns",
            "range": "± 2203.9332521333163"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_interactions)",
            "value": 2828383.8541666665,
            "unit": "ns",
            "range": "± 7776.532999939712"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_no_interactions)",
            "value": 958808.4505208334,
            "unit": "ns",
            "range": "± 4701.6960043396"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: few_features)",
            "value": 3308.482233683268,
            "unit": "ns",
            "range": "± 9.718468308491612"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: few_features)",
            "value": 11573.844604492188,
            "unit": "ns",
            "range": "± 32.42860927769922"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: many_features)",
            "value": 110109.69563802083,
            "unit": "ns",
            "range": "± 369.2384919819246"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: many_features)",
            "value": 19096.664428710938,
            "unit": "ns",
            "range": "± 46.08806581986807"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: quadratic)",
            "value": 4240019.479166667,
            "unit": "ns",
            "range": "± 43406.63024048214"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: simple)",
            "value": 216814.24479166666,
            "unit": "ns",
            "range": "± 669.2078032387736"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "jackgerrits@users.noreply.github.com",
            "name": "Jack Gerrits",
            "username": "jackgerrits"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "5a57cf4feeac429e255c2b0a964f8a3b5ff6d5ab",
          "message": "refactor: cleanup includes (#4188)\n\n* refactor: cleanup includes\r\n\r\n* Update CMakeLists.txt\r\n\r\n* Update conditional_contextual_bandit.h",
          "timestamp": "2022-10-05T14:17:51-04:00",
          "tree_id": "3b50d97d08312c2285cf04c38dc29eae1c024a08",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/5a57cf4feeac429e255c2b0a964f8a3b5ff6d5ab"
        },
        "date": 1664995054332,
        "tool": "googlecpp",
        "benches": [
          {
            "name": "bench_text/120_string_fts",
            "value": 6223.449529401714,
            "unit": "ns/iter",
            "extra": "iterations: 668617\ncpu: 6223.127141547403 ns\nthreads: 1"
          },
          {
            "name": "bench_text/120_num_fts",
            "value": 5005.563798173401,
            "unit": "ns/iter",
            "extra": "iterations: 835886\ncpu: 5005.27284821136 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/8_features",
            "value": 73.03537257998184,
            "unit": "ns/iter",
            "extra": "iterations: 58008350\ncpu: 73.03136531206282 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/1_feature",
            "value": 165.90092239074792,
            "unit": "ns/iter",
            "extra": "iterations: 25155391\ncpu: 165.8917128340402 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features",
            "value": 7008.8403068958905,
            "unit": "ns/iter",
            "extra": "iterations: 598379\ncpu: 7008.187453102463 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features",
            "value": 12624.20358947616,
            "unit": "ns/iter",
            "extra": "iterations: 331859\ncpu: 12623.18002525168 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features_no_predict",
            "value": 3375.0923444843424,
            "unit": "ns/iter",
            "extra": "iterations: 1228108\ncpu: 3374.915561172145 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features_no_predic",
            "value": 5111.233915626558,
            "unit": "ns/iter",
            "extra": "iterations: 810989\ncpu: 5110.959211530605 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/few_features",
            "value": 1204.752870398071,
            "unit": "ns/iter",
            "extra": "iterations: 3527995\ncpu: 1204.7027277533002 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/many_features/min_time:15.000",
            "value": 57864.06088053801,
            "unit": "ns/iter",
            "extra": "iterations: 360575\ncpu: 57860.20467309158 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_no_namespaces/min_time:15.000",
            "value": 279729.79383301095,
            "unit": "ns/iter",
            "extra": "iterations: 74915\ncpu: 279709.58152572904 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 318541.23166281485,
            "unit": "ns/iter",
            "extra": "iterations: 65768\ncpu: 318516.55820459785 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_interactions/min_time:15.000",
            "value": 559945.3164032404,
            "unit": "ns/iter",
            "extra": "iterations: 37645\ncpu: 559904.5264975423 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_no_interactions/min_time:15.000",
            "value": 290313.0610013027,
            "unit": "ns/iter",
            "extra": "iterations: 72146\ncpu: 290291.9177778392 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_interactions/min_time:15.000",
            "value": 432776.30364975554,
            "unit": "ns/iter",
            "extra": "iterations: 48332\ncpu: 432743.4494744678 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_no_namespaces/min_time:15.000",
            "value": 599620.6089272855,
            "unit": "ns/iter",
            "extra": "iterations: 34725\ncpu: 599568.7717782572 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 780616.6313782135,
            "unit": "ns/iter",
            "extra": "iterations: 27006\ncpu: 780554.3064504177 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_interactions/min_time:15.000",
            "value": 2462491.014991287,
            "unit": "ns/iter",
            "extra": "iterations: 8605\ncpu: 2462275.4677513116 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_no_interactions/min_time:15.000",
            "value": 626377.2760070185,
            "unit": "ns/iter",
            "extra": "iterations: 33068\ncpu: 626317.7210596356 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_interactions/min_time:15.000",
            "value": 1664496.7207751719,
            "unit": "ns/iter",
            "extra": "iterations: 11249\ncpu: 1664153.8803449222 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/simple/min_time:15.000",
            "value": 46172.09553298538,
            "unit": "ns/iter",
            "extra": "iterations: 449562\ncpu: 46169.14952776257 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/quadratic/min_time:15.000",
            "value": 2603406.784271922,
            "unit": "ns/iter",
            "extra": "iterations: 7973\ncpu: 2603240.9130816553 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_string_fts",
            "value": 990.3061257709977,
            "unit": "ns/iter",
            "extra": "iterations: 4238895\ncpu: 990.2559983203115 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_string_fts",
            "value": 7302.629075940707,
            "unit": "ns/iter",
            "extra": "iterations: 575266\ncpu: 7302.262953138118 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_num_fts",
            "value": 808.6818127011904,
            "unit": "ns/iter",
            "extra": "iterations: 5191854\ncpu: 808.641248386408 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_num_fts",
            "value": 5598.535226823639,
            "unit": "ns/iter",
            "extra": "iterations: 741523\ncpu: 5598.20976557702 ns\nthreads: 1"
          },
          {
            "name": "benchmark_example_reuse",
            "value": 773.4884190466685,
            "unit": "ns/iter",
            "extra": "iterations: 5434354\ncpu: 773.4457122226472 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_char",
            "value": 51.25069464424433,
            "unit": "ns/iter",
            "extra": "iterations: 82064669\ncpu: 51.24793959748978 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_extent",
            "value": 73.18745769258395,
            "unit": "ns/iter",
            "extra": "iterations: 57408671\ncpu: 73.1833071000722 ns\nthreads: 1"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "jackgerrits@users.noreply.github.com",
            "name": "Jack Gerrits",
            "username": "jackgerrits"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "5a57cf4feeac429e255c2b0a964f8a3b5ff6d5ab",
          "message": "refactor: cleanup includes (#4188)\n\n* refactor: cleanup includes\r\n\r\n* Update CMakeLists.txt\r\n\r\n* Update conditional_contextual_bandit.h",
          "timestamp": "2022-10-05T14:17:51-04:00",
          "tree_id": "3b50d97d08312c2285cf04c38dc29eae1c024a08",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/5a57cf4feeac429e255c2b0a964f8a3b5ff6d5ab"
        },
        "date": 1664996773944,
        "tool": "benchmarkdotnet",
        "benches": [
          {
            "name": "BenchmarkText.Benchmark(args: 120_num_features)",
            "value": 6399.679674421038,
            "unit": "ns",
            "range": "± 43.00702539488709"
          },
          {
            "name": "BenchmarkText.Benchmark(args: 120_string_fts)",
            "value": 9212.284240722656,
            "unit": "ns",
            "range": "± 43.75605919387233"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 1_feature)",
            "value": 633.381290435791,
            "unit": "ns",
            "range": "± 3.552076186310146"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 8_features)",
            "value": 539.3716049194336,
            "unit": "ns",
            "range": "± 4.65874166621139"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_interactions)",
            "value": 881096.30859375,
            "unit": "ns",
            "range": "± 6581.221464615615"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_no_interactions)",
            "value": 567186.7154947916,
            "unit": "ns",
            "range": "± 8109.805476341058"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_no_namespaces)",
            "value": 543356.328125,
            "unit": "ns",
            "range": "± 3588.5234151891896"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_interactions)",
            "value": 872922.36328125,
            "unit": "ns",
            "range": "± 3884.4773982599995"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_no_interactions)",
            "value": 545186.7815290178,
            "unit": "ns",
            "range": "± 3780.761380569831"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_interactions)",
            "value": 3473257.4739583335,
            "unit": "ns",
            "range": "± 12576.573999096321"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_no_interactions)",
            "value": 1054970.3645833333,
            "unit": "ns",
            "range": "± 6524.132218084654"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_no_namespaces)",
            "value": 890047.6106770834,
            "unit": "ns",
            "range": "± 4525.778498573072"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_interactions)",
            "value": 2861885.4631696427,
            "unit": "ns",
            "range": "± 9884.962690778591"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_no_interactions)",
            "value": 910605.21484375,
            "unit": "ns",
            "range": "± 4170.348594283505"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: few_features)",
            "value": 3212.6551491873606,
            "unit": "ns",
            "range": "± 12.360214000298482"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: few_features)",
            "value": 11296.077728271484,
            "unit": "ns",
            "range": "± 28.214360569320615"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: many_features)",
            "value": 108359.11865234375,
            "unit": "ns",
            "range": "± 334.52848820287613"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: many_features)",
            "value": 19196.64786202567,
            "unit": "ns",
            "range": "± 50.529157802078124"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: quadratic)",
            "value": 4192081.25,
            "unit": "ns",
            "range": "± 33056.56129283317"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: simple)",
            "value": 212408.65234375,
            "unit": "ns",
            "range": "± 1182.353323135203"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "jackgerrits@users.noreply.github.com",
            "name": "Jack Gerrits",
            "username": "jackgerrits"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "0b951064228bba9218a2e63e67c02242dedf0e7f",
          "message": "refactor: split sparse and dense parameters (#4190)",
          "timestamp": "2022-10-05T16:29:30-04:00",
          "tree_id": "bb84f25fdde9cec76cbcdaba7961bd0e993b85b0",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/0b951064228bba9218a2e63e67c02242dedf0e7f"
        },
        "date": 1665003296382,
        "tool": "googlecpp",
        "benches": [
          {
            "name": "bench_text/120_string_fts",
            "value": 7284.816767948395,
            "unit": "ns/iter",
            "extra": "iterations: 592571\ncpu: 7278.304371965553 ns\nthreads: 1"
          },
          {
            "name": "bench_text/120_num_fts",
            "value": 5058.2794654548925,
            "unit": "ns/iter",
            "extra": "iterations: 820062\ncpu: 5057.444071301927 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/8_features",
            "value": 88.87697845673394,
            "unit": "ns/iter",
            "extra": "iterations: 48612827\ncpu: 88.86040509431798 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/1_feature",
            "value": 198.3527765909931,
            "unit": "ns/iter",
            "extra": "iterations: 21318084\ncpu: 198.30101523195054 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features",
            "value": 8626.240919752041,
            "unit": "ns/iter",
            "extra": "iterations: 488175\ncpu: 8624.852972806888 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features",
            "value": 16010.351725158815,
            "unit": "ns/iter",
            "extra": "iterations: 263396\ncpu: 16008.325866755751 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features_no_predict",
            "value": 4201.609602533067,
            "unit": "ns/iter",
            "extra": "iterations: 1039684\ncpu: 4201.078981690594 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features_no_predic",
            "value": 6510.067505912391,
            "unit": "ns/iter",
            "extra": "iterations: 644388\ncpu: 6509.242723328185 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/few_features",
            "value": 1531.1985088868885,
            "unit": "ns/iter",
            "extra": "iterations: 2694363\ncpu: 1531.0426620318044 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/many_features/min_time:15.000",
            "value": 96938.5695804649,
            "unit": "ns/iter",
            "extra": "iterations: 217288\ncpu: 96926.71799639192 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_no_namespaces/min_time:15.000",
            "value": 373219.13990998286,
            "unit": "ns/iter",
            "extra": "iterations: 54435\ncpu: 373173.96160558466 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 391277.337129962,
            "unit": "ns/iter",
            "extra": "iterations: 54961\ncpu: 391227.9743818344 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_interactions/min_time:15.000",
            "value": 679904.9566993145,
            "unit": "ns/iter",
            "extra": "iterations: 31339\ncpu: 679824.7391429207 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_no_interactions/min_time:15.000",
            "value": 362572.1394850881,
            "unit": "ns/iter",
            "extra": "iterations: 57913\ncpu: 362526.16165627755 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_interactions/min_time:15.000",
            "value": 562170.8716221625,
            "unit": "ns/iter",
            "extra": "iterations: 37561\ncpu: 562109.730837837 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_no_namespaces/min_time:15.000",
            "value": 787125.991414282,
            "unit": "ns/iter",
            "extra": "iterations: 25391\ncpu: 787018.5026190393 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 942971.9731404034,
            "unit": "ns/iter",
            "extra": "iterations: 22599\ncpu: 942874.3174476733 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_interactions/min_time:15.000",
            "value": 3148188.022958437,
            "unit": "ns/iter",
            "extra": "iterations: 6882\ncpu: 3147845.5826794486 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_no_interactions/min_time:15.000",
            "value": 756927.4243037227,
            "unit": "ns/iter",
            "extra": "iterations: 27683\ncpu: 756844.2220857564 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_interactions/min_time:15.000",
            "value": 2388576.6818077066,
            "unit": "ns/iter",
            "extra": "iterations: 8674\ncpu: 2388301.8100069175 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/simple/min_time:15.000",
            "value": 84424.62840662294,
            "unit": "ns/iter",
            "extra": "iterations: 251899\ncpu: 84415.54432530509 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/quadratic/min_time:15.000",
            "value": 3102401.4146852107,
            "unit": "ns/iter",
            "extra": "iterations: 6687\ncpu: 3102097.1437116726 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_string_fts",
            "value": 1171.546013370208,
            "unit": "ns/iter",
            "extra": "iterations: 3551935\ncpu: 1171.4286438237154 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_string_fts",
            "value": 8471.261670057906,
            "unit": "ns/iter",
            "extra": "iterations: 487037\ncpu: 8470.294248691625 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_num_fts",
            "value": 873.0811848623032,
            "unit": "ns/iter",
            "extra": "iterations: 4812880\ncpu: 872.9969373846894 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_num_fts",
            "value": 5654.040395022071,
            "unit": "ns/iter",
            "extra": "iterations: 698898\ncpu: 5653.237954608586 ns\nthreads: 1"
          },
          {
            "name": "benchmark_example_reuse",
            "value": 1016.9536826858752,
            "unit": "ns/iter",
            "extra": "iterations: 4119086\ncpu: 1016.8002318961109 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_char",
            "value": 53.90368281423205,
            "unit": "ns/iter",
            "extra": "iterations: 77131884\ncpu: 53.89913722320109 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_extent",
            "value": 80.42359467121409,
            "unit": "ns/iter",
            "extra": "iterations: 54858497\ncpu: 80.4150157449637 ns\nthreads: 1"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "jackgerrits@users.noreply.github.com",
            "name": "Jack Gerrits",
            "username": "jackgerrits"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "0b951064228bba9218a2e63e67c02242dedf0e7f",
          "message": "refactor: split sparse and dense parameters (#4190)",
          "timestamp": "2022-10-05T16:29:30-04:00",
          "tree_id": "bb84f25fdde9cec76cbcdaba7961bd0e993b85b0",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/0b951064228bba9218a2e63e67c02242dedf0e7f"
        },
        "date": 1665004714035,
        "tool": "benchmarkdotnet",
        "benches": [
          {
            "name": "BenchmarkText.Benchmark(args: 120_num_features)",
            "value": 6079.863085065569,
            "unit": "ns",
            "range": "± 28.13581548982908"
          },
          {
            "name": "BenchmarkText.Benchmark(args: 120_string_fts)",
            "value": 9518.885192871094,
            "unit": "ns",
            "range": "± 58.76971727551046"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 1_feature)",
            "value": 591.8783315022787,
            "unit": "ns",
            "range": "± 5.949495153582872"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 8_features)",
            "value": 487.8601328531901,
            "unit": "ns",
            "range": "± 3.3576921475402735"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_interactions)",
            "value": 906354.4205729166,
            "unit": "ns",
            "range": "± 1815.2331634404547"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_no_interactions)",
            "value": 557064.0625,
            "unit": "ns",
            "range": "± 1213.0097850535763"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_no_namespaces)",
            "value": 558515.9895833334,
            "unit": "ns",
            "range": "± 1071.234718430383"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_interactions)",
            "value": 902753.6002604166,
            "unit": "ns",
            "range": "± 2423.6631477915967"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_no_interactions)",
            "value": 556033.076171875,
            "unit": "ns",
            "range": "± 1276.7577256403765"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_interactions)",
            "value": 3431028.7630208335,
            "unit": "ns",
            "range": "± 9670.061960974705"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_no_interactions)",
            "value": 1117504.2578125,
            "unit": "ns",
            "range": "± 2879.1445952079403"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_no_namespaces)",
            "value": 908803.2552083334,
            "unit": "ns",
            "range": "± 1482.7441003970291"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_interactions)",
            "value": 2788988.2552083335,
            "unit": "ns",
            "range": "± 8385.439762323544"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_no_interactions)",
            "value": 931162.2200520834,
            "unit": "ns",
            "range": "± 2092.7868440719644"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: few_features)",
            "value": 3238.983799861028,
            "unit": "ns",
            "range": "± 8.500409028629788"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: few_features)",
            "value": 11380.939737955729,
            "unit": "ns",
            "range": "± 38.339210298244616"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: many_features)",
            "value": 109189.267578125,
            "unit": "ns",
            "range": "± 223.9490238728255"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: many_features)",
            "value": 22865.355834960938,
            "unit": "ns",
            "range": "± 54.52010595374669"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: quadratic)",
            "value": 4196724.322916667,
            "unit": "ns",
            "range": "± 8833.77255388176"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: simple)",
            "value": 190006.39973958334,
            "unit": "ns",
            "range": "± 794.673088664096"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "jackgerrits@users.noreply.github.com",
            "name": "Jack Gerrits",
            "username": "jackgerrits"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "14a7c81e6250ff50c269c393302a418a761d71ee",
          "message": "refactor: move open_socket into details namespace (#4189)",
          "timestamp": "2022-10-05T17:22:47-04:00",
          "tree_id": "2926c5e4bbee53503bd91b20c8be1984f893ff62",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/14a7c81e6250ff50c269c393302a418a761d71ee"
        },
        "date": 1665006357026,
        "tool": "googlecpp",
        "benches": [
          {
            "name": "bench_text/120_string_fts",
            "value": 7282.4546984344115,
            "unit": "ns/iter",
            "extra": "iterations: 580587\ncpu: 7281.211256883119 ns\nthreads: 1"
          },
          {
            "name": "bench_text/120_num_fts",
            "value": 5437.837350249728,
            "unit": "ns/iter",
            "extra": "iterations: 756343\ncpu: 5436.618306773515 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/8_features",
            "value": 87.72937377039825,
            "unit": "ns/iter",
            "extra": "iterations: 44348750\ncpu: 87.71580258744608 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/1_feature",
            "value": 200.55415199766225,
            "unit": "ns/iter",
            "extra": "iterations: 20981599\ncpu: 200.52479794318816 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features",
            "value": 8323.064670821619,
            "unit": "ns/iter",
            "extra": "iterations: 506519\ncpu: 8318.719929558418 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features",
            "value": 14752.589434821017,
            "unit": "ns/iter",
            "extra": "iterations: 285750\ncpu: 14745.948556430436 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features_no_predict",
            "value": 3898.5762637401967,
            "unit": "ns/iter",
            "extra": "iterations: 1099415\ncpu: 3897.808925655915 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features_no_predic",
            "value": 5813.735678189157,
            "unit": "ns/iter",
            "extra": "iterations: 732554\ncpu: 5812.642617472569 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/few_features",
            "value": 1396.1607506468533,
            "unit": "ns/iter",
            "extra": "iterations: 2964803\ncpu: 1395.9207407709716 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/many_features/min_time:15.000",
            "value": 74108.45708387936,
            "unit": "ns/iter",
            "extra": "iterations: 287759\ncpu: 74083.66966802078 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_no_namespaces/min_time:15.000",
            "value": 328809.1275642295,
            "unit": "ns/iter",
            "extra": "iterations: 64493\ncpu: 328740.5454855566 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 359872.3363153398,
            "unit": "ns/iter",
            "extra": "iterations: 57791\ncpu: 359799.4653146685 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_interactions/min_time:15.000",
            "value": 644097.200160026,
            "unit": "ns/iter",
            "extra": "iterations: 32494\ncpu: 643885.2311195907 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_no_interactions/min_time:15.000",
            "value": 327883.78223088867,
            "unit": "ns/iter",
            "extra": "iterations: 64100\ncpu: 327818.89079563157 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_interactions/min_time:15.000",
            "value": 506821.96828450967,
            "unit": "ns/iter",
            "extra": "iterations: 42093\ncpu: 506653.1109685697 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_no_namespaces/min_time:15.000",
            "value": 685913.1083748672,
            "unit": "ns/iter",
            "extra": "iterations: 30496\ncpu: 685767.7105194123 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 887786.015915346,
            "unit": "ns/iter",
            "extra": "iterations: 23625\ncpu: 887465.0243386242 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_interactions/min_time:15.000",
            "value": 2843884.27051712,
            "unit": "ns/iter",
            "extra": "iterations: 7445\ncpu: 2843279.610476832 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_no_interactions/min_time:15.000",
            "value": 718625.4906681268,
            "unit": "ns/iter",
            "extra": "iterations: 29201\ncpu: 718387.9079483582 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_interactions/min_time:15.000",
            "value": 2212126.541802175,
            "unit": "ns/iter",
            "extra": "iterations: 9533\ncpu: 2211647.6450225543 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/simple/min_time:15.000",
            "value": 58994.21732127187,
            "unit": "ns/iter",
            "extra": "iterations: 355736\ncpu: 58983.24797040497 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/quadratic/min_time:15.000",
            "value": 3336511.3681407515,
            "unit": "ns/iter",
            "extra": "iterations: 6196\ncpu: 3335621.1265332466 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_string_fts",
            "value": 1139.8163015774028,
            "unit": "ns/iter",
            "extra": "iterations: 3685263\ncpu: 1139.6113113229599 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_string_fts",
            "value": 8613.779237153016,
            "unit": "ns/iter",
            "extra": "iterations: 486205\ncpu: 8612.265402453662 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_num_fts",
            "value": 860.1164875102926,
            "unit": "ns/iter",
            "extra": "iterations: 4877862\ncpu: 859.9698597459335 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_num_fts",
            "value": 6063.655205036356,
            "unit": "ns/iter",
            "extra": "iterations: 684952\ncpu: 6062.555040353171 ns\nthreads: 1"
          },
          {
            "name": "benchmark_example_reuse",
            "value": 977.395005432645,
            "unit": "ns/iter",
            "extra": "iterations: 4325700\ncpu: 977.2197794576584 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_char",
            "value": 62.718109551724496,
            "unit": "ns/iter",
            "extra": "iterations: 67106073\ncpu: 62.70724856750265 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_extent",
            "value": 89.36696807726976,
            "unit": "ns/iter",
            "extra": "iterations: 46797403\ncpu: 89.35189843761263 ns\nthreads: 1"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "jackgerrits@users.noreply.github.com",
            "name": "Jack Gerrits",
            "username": "jackgerrits"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "14a7c81e6250ff50c269c393302a418a761d71ee",
          "message": "refactor: move open_socket into details namespace (#4189)",
          "timestamp": "2022-10-05T17:22:47-04:00",
          "tree_id": "2926c5e4bbee53503bd91b20c8be1984f893ff62",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/14a7c81e6250ff50c269c393302a418a761d71ee"
        },
        "date": 1665007769419,
        "tool": "benchmarkdotnet",
        "benches": [
          {
            "name": "BenchmarkText.Benchmark(args: 120_num_features)",
            "value": 5237.575266911434,
            "unit": "ns",
            "range": "± 10.864853368671943"
          },
          {
            "name": "BenchmarkText.Benchmark(args: 120_string_fts)",
            "value": 7187.960052490234,
            "unit": "ns",
            "range": "± 32.52394936642"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 1_feature)",
            "value": 565.0971539815267,
            "unit": "ns",
            "range": "± 1.8090973656874079"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 8_features)",
            "value": 466.37181917826337,
            "unit": "ns",
            "range": "± 1.3090692894117857"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_interactions)",
            "value": 853266.8607271635,
            "unit": "ns",
            "range": "± 606.8336572340045"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_no_interactions)",
            "value": 470161.572265625,
            "unit": "ns",
            "range": "± 530.7021503469558"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_no_namespaces)",
            "value": 472434.6516927083,
            "unit": "ns",
            "range": "± 391.16950560605005"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_interactions)",
            "value": 850807.40234375,
            "unit": "ns",
            "range": "± 1213.623312057"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_no_interactions)",
            "value": 467006.8246694711,
            "unit": "ns",
            "range": "± 410.6850607322835"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_interactions)",
            "value": 3410232.890625,
            "unit": "ns",
            "range": "± 5073.2424806432955"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_no_interactions)",
            "value": 1054810.80078125,
            "unit": "ns",
            "range": "± 1591.4435396080016"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_no_namespaces)",
            "value": 870906.4453125,
            "unit": "ns",
            "range": "± 1765.4051065428982"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_interactions)",
            "value": 2796851.8619791665,
            "unit": "ns",
            "range": "± 3844.5019687948316"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_no_interactions)",
            "value": 884243.0385044643,
            "unit": "ns",
            "range": "± 1051.0586513680155"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: few_features)",
            "value": 3400.668580191476,
            "unit": "ns",
            "range": "± 2.6662120924352846"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: few_features)",
            "value": 11350.015970865885,
            "unit": "ns",
            "range": "± 30.793796927335734"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: many_features)",
            "value": 125713.74860491071,
            "unit": "ns",
            "range": "± 165.02554777325676"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: many_features)",
            "value": 19610.18798828125,
            "unit": "ns",
            "range": "± 40.76024272997716"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: quadratic)",
            "value": 4840171.484375,
            "unit": "ns",
            "range": "± 2999.8991455340465"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: simple)",
            "value": 189630.47688802084,
            "unit": "ns",
            "range": "± 282.19436494531243"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "jackgerrits@users.noreply.github.com",
            "name": "Jack Gerrits",
            "username": "jackgerrits"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "4ba0474dfe81ad902003fef0b2bffc2557d37378",
          "message": "refactor: use operators for inequality instead of custom compare functions (#4192)",
          "timestamp": "2022-10-05T22:24:25-04:00",
          "tree_id": "af7cd64a28ef76c9f47d85d895bc70ba51fc524b",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/4ba0474dfe81ad902003fef0b2bffc2557d37378"
        },
        "date": 1665024399556,
        "tool": "googlecpp",
        "benches": [
          {
            "name": "bench_text/120_string_fts",
            "value": 7041.868942553747,
            "unit": "ns/iter",
            "extra": "iterations: 595548\ncpu: 7041.5368702438755 ns\nthreads: 1"
          },
          {
            "name": "bench_text/120_num_fts",
            "value": 5734.036498444408,
            "unit": "ns/iter",
            "extra": "iterations: 759868\ncpu: 5733.479235867281 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/8_features",
            "value": 90.73200086193995,
            "unit": "ns/iter",
            "extra": "iterations: 48365316\ncpu: 90.72742748129674 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/1_feature",
            "value": 200.5374727516904,
            "unit": "ns/iter",
            "extra": "iterations: 21021488\ncpu: 200.52821189441954 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features",
            "value": 8236.359029902573,
            "unit": "ns/iter",
            "extra": "iterations: 513804\ncpu: 8235.689873959722 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features",
            "value": 15016.18710867238,
            "unit": "ns/iter",
            "extra": "iterations: 289559\ncpu: 15014.528299931966 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features_no_predict",
            "value": 3880.5606269509553,
            "unit": "ns/iter",
            "extra": "iterations: 1074151\ncpu: 3880.3429871591616 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features_no_predic",
            "value": 5922.043935567935,
            "unit": "ns/iter",
            "extra": "iterations: 702506\ncpu: 5921.7414513185695 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/few_features",
            "value": 1391.0556655784292,
            "unit": "ns/iter",
            "extra": "iterations: 2976148\ncpu: 1390.9715511459776 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/many_features/min_time:15.000",
            "value": 69491.72078947356,
            "unit": "ns/iter",
            "extra": "iterations: 300200\ncpu: 69487.94670219856 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_no_namespaces/min_time:15.000",
            "value": 340704.882482145,
            "unit": "ns/iter",
            "extra": "iterations: 65105\ncpu: 340682.7816603946 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 380572.97336275136,
            "unit": "ns/iter",
            "extra": "iterations: 55749\ncpu: 380545.9846813394 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_interactions/min_time:15.000",
            "value": 659475.9599198996,
            "unit": "ns/iter",
            "extra": "iterations: 31961\ncpu: 659433.997684678 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_no_interactions/min_time:15.000",
            "value": 345177.8441968613,
            "unit": "ns/iter",
            "extra": "iterations: 63009\ncpu: 345155.3444745989 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_interactions/min_time:15.000",
            "value": 509443.13476285996,
            "unit": "ns/iter",
            "extra": "iterations: 40293\ncpu: 509372.36740873096 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_no_namespaces/min_time:15.000",
            "value": 699413.2827902893,
            "unit": "ns/iter",
            "extra": "iterations: 30047\ncpu: 699313.109461843 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 886956.6128990691,
            "unit": "ns/iter",
            "extra": "iterations: 23242\ncpu: 886839.6136304962 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_interactions/min_time:15.000",
            "value": 2843218.4076057165,
            "unit": "ns/iter",
            "extra": "iterations: 7284\ncpu: 2842856.4250411815 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_no_interactions/min_time:15.000",
            "value": 649354.0562248967,
            "unit": "ns/iter",
            "extra": "iterations: 29133\ncpu: 649145.2785500982 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_interactions/min_time:15.000",
            "value": 1818716.1490872179,
            "unit": "ns/iter",
            "extra": "iterations: 10846\ncpu: 1818477.6968467655 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/simple/min_time:15.000",
            "value": 56583.47168345885,
            "unit": "ns/iter",
            "extra": "iterations: 412003\ncpu: 56578.12491656614 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/quadratic/min_time:15.000",
            "value": 3158872.018053866,
            "unit": "ns/iter",
            "extra": "iterations: 6536\ncpu: 3158591.630966953 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_string_fts",
            "value": 1110.9957102213207,
            "unit": "ns/iter",
            "extra": "iterations: 3766861\ncpu: 1110.9122157679694 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_string_fts",
            "value": 9103.291277947515,
            "unit": "ns/iter",
            "extra": "iterations: 454377\ncpu: 9102.470635617523 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_num_fts",
            "value": 877.5432531766527,
            "unit": "ns/iter",
            "extra": "iterations: 4641451\ncpu: 877.471290766618 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_num_fts",
            "value": 6277.459594750925,
            "unit": "ns/iter",
            "extra": "iterations: 639261\ncpu: 6276.9552655331345 ns\nthreads: 1"
          },
          {
            "name": "benchmark_example_reuse",
            "value": 910.2317691845719,
            "unit": "ns/iter",
            "extra": "iterations: 4803145\ncpu: 910.1526603923058 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_char",
            "value": 58.34968252185359,
            "unit": "ns/iter",
            "extra": "iterations: 75021542\ncpu: 58.34502575273678 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_extent",
            "value": 83.28199490710583,
            "unit": "ns/iter",
            "extra": "iterations: 55246402\ncpu: 83.27606022198512 ns\nthreads: 1"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "jackgerrits@users.noreply.github.com",
            "name": "Jack Gerrits",
            "username": "jackgerrits"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "4ba0474dfe81ad902003fef0b2bffc2557d37378",
          "message": "refactor: use operators for inequality instead of custom compare functions (#4192)",
          "timestamp": "2022-10-05T22:24:25-04:00",
          "tree_id": "af7cd64a28ef76c9f47d85d895bc70ba51fc524b",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/4ba0474dfe81ad902003fef0b2bffc2557d37378"
        },
        "date": 1665025743795,
        "tool": "benchmarkdotnet",
        "benches": [
          {
            "name": "BenchmarkText.Benchmark(args: 120_num_features)",
            "value": 6177.409069354718,
            "unit": "ns",
            "range": "± 18.242990787795957"
          },
          {
            "name": "BenchmarkText.Benchmark(args: 120_string_fts)",
            "value": 9598.738403320312,
            "unit": "ns",
            "range": "± 36.86214161660196"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 1_feature)",
            "value": 614.2967160542806,
            "unit": "ns",
            "range": "± 3.766019292873421"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 8_features)",
            "value": 514.4706153869629,
            "unit": "ns",
            "range": "± 7.618382662798201"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_interactions)",
            "value": 896375.8463541666,
            "unit": "ns",
            "range": "± 1840.7035263770301"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_no_interactions)",
            "value": 511387.5260416667,
            "unit": "ns",
            "range": "± 693.4065534151395"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_no_namespaces)",
            "value": 545411.0286458334,
            "unit": "ns",
            "range": "± 1851.0269258580083"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_interactions)",
            "value": 892856.9921875,
            "unit": "ns",
            "range": "± 2057.4688619817684"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_no_interactions)",
            "value": 542391.4388020834,
            "unit": "ns",
            "range": "± 1542.0989605562906"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_interactions)",
            "value": 3447685.4166666665,
            "unit": "ns",
            "range": "± 9275.949973624209"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_no_interactions)",
            "value": 1062053.3723958333,
            "unit": "ns",
            "range": "± 3294.2839212483227"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_no_namespaces)",
            "value": 899148.3072916666,
            "unit": "ns",
            "range": "± 2551.5877529417826"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_interactions)",
            "value": 2814257.3567708335,
            "unit": "ns",
            "range": "± 7978.667508535678"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_no_interactions)",
            "value": 936921.0546875,
            "unit": "ns",
            "range": "± 2117.6002951199393"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: few_features)",
            "value": 3204.990971883138,
            "unit": "ns",
            "range": "± 10.968768042040143"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: few_features)",
            "value": 11367.945760091146,
            "unit": "ns",
            "range": "± 37.127745227191355"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: many_features)",
            "value": 109013.14348493304,
            "unit": "ns",
            "range": "± 273.2049426944971"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: many_features)",
            "value": 19070.216471354168,
            "unit": "ns",
            "range": "± 62.1892521566135"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: quadratic)",
            "value": 4167121.8489583335,
            "unit": "ns",
            "range": "± 8874.835652375643"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: simple)",
            "value": 203896.69596354166,
            "unit": "ns",
            "range": "± 1636.9526777454475"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "jackgerrits@users.noreply.github.com",
            "name": "Jack Gerrits",
            "username": "jackgerrits"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "c3364063fc4332d8aeb4ccf5a965d64100b54e41",
          "message": "refactor: remove unused type in allreduce (#4191)",
          "timestamp": "2022-10-05T23:51:06-04:00",
          "tree_id": "563d8cd48506e4c61d0392d63a962e4c5dc38816",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/c3364063fc4332d8aeb4ccf5a965d64100b54e41"
        },
        "date": 1665029461674,
        "tool": "googlecpp",
        "benches": [
          {
            "name": "bench_text/120_string_fts",
            "value": 6146.426843755034,
            "unit": "ns/iter",
            "extra": "iterations: 685232\ncpu: 6140.242136969668 ns\nthreads: 1"
          },
          {
            "name": "bench_text/120_num_fts",
            "value": 4781.309014963778,
            "unit": "ns/iter",
            "extra": "iterations: 871440\ncpu: 4780.908037271643 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/8_features",
            "value": 78.50965938419056,
            "unit": "ns/iter",
            "extra": "iterations: 54121152\ncpu: 78.50404588579342 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/1_feature",
            "value": 173.49543110600868,
            "unit": "ns/iter",
            "extra": "iterations: 24577064\ncpu: 173.4820196586541 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features",
            "value": 6846.817315911559,
            "unit": "ns/iter",
            "extra": "iterations: 614175\ncpu: 6846.182277038306 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features",
            "value": 12664.381582165088,
            "unit": "ns/iter",
            "extra": "iterations: 343504\ncpu: 12663.157925380781 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features_no_predict",
            "value": 3261.848300826458,
            "unit": "ns/iter",
            "extra": "iterations: 1293982\ncpu: 3261.612139890666 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features_no_predic",
            "value": 4931.648668619061,
            "unit": "ns/iter",
            "extra": "iterations: 853963\ncpu: 4931.22477203345 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/few_features",
            "value": 1176.5144268537695,
            "unit": "ns/iter",
            "extra": "iterations: 3524781\ncpu: 1176.4287199687017 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/many_features/min_time:15.000",
            "value": 57107.25495764843,
            "unit": "ns/iter",
            "extra": "iterations: 360554\ncpu: 57100.42434697715 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_no_namespaces/min_time:15.000",
            "value": 278474.36803210265,
            "unit": "ns/iter",
            "extra": "iterations: 80243\ncpu: 278450.12649078434 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 317493.9616540573,
            "unit": "ns/iter",
            "extra": "iterations: 66213\ncpu: 317460.10753175354 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_interactions/min_time:15.000",
            "value": 560672.3270667689,
            "unit": "ns/iter",
            "extra": "iterations: 37607\ncpu: 560623.7402611212 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_no_interactions/min_time:15.000",
            "value": 289902.3175197042,
            "unit": "ns/iter",
            "extra": "iterations: 72701\ncpu: 289877.10072763776 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_interactions/min_time:15.000",
            "value": 420957.1341776832,
            "unit": "ns/iter",
            "extra": "iterations: 48637\ncpu: 420921.1053313318 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_no_namespaces/min_time:15.000",
            "value": 584557.9109925465,
            "unit": "ns/iter",
            "extra": "iterations: 35424\ncpu: 584491.6610207772 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 760528.1937226276,
            "unit": "ns/iter",
            "extra": "iterations: 27400\ncpu: 760453.4999999999 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_interactions/min_time:15.000",
            "value": 2455459.5660465676,
            "unit": "ns/iter",
            "extra": "iterations: 8547\ncpu: 2455218.462618463 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_no_interactions/min_time:15.000",
            "value": 612205.3271090456,
            "unit": "ns/iter",
            "extra": "iterations: 34435\ncpu: 607934.4213736021 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_interactions/min_time:15.000",
            "value": 1873910.6378945634,
            "unit": "ns/iter",
            "extra": "iterations: 11342\ncpu: 1873738.961382476 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/simple/min_time:15.000",
            "value": 52174.009558694255,
            "unit": "ns/iter",
            "extra": "iterations: 404867\ncpu: 52169.68387149367 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/quadratic/min_time:15.000",
            "value": 2830905.342930897,
            "unit": "ns/iter",
            "extra": "iterations: 7363\ncpu: 2830636.534021458 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_string_fts",
            "value": 967.614802694875,
            "unit": "ns/iter",
            "extra": "iterations: 4357058\ncpu: 967.5390366618899 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_string_fts",
            "value": 7816.632572934273,
            "unit": "ns/iter",
            "extra": "iterations: 536648\ncpu: 7816.0101220911265 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_num_fts",
            "value": 767.2652223277097,
            "unit": "ns/iter",
            "extra": "iterations: 5471338\ncpu: 767.2026111346124 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_num_fts",
            "value": 5764.795869643842,
            "unit": "ns/iter",
            "extra": "iterations: 727976\ncpu: 5761.9777300350925 ns\nthreads: 1"
          },
          {
            "name": "benchmark_example_reuse",
            "value": 791.4967449574663,
            "unit": "ns/iter",
            "extra": "iterations: 5304539\ncpu: 791.4225156983501 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_char",
            "value": 52.60195675912251,
            "unit": "ns/iter",
            "extra": "iterations: 79457915\ncpu: 52.59760843208604 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_extent",
            "value": 74.89619152465472,
            "unit": "ns/iter",
            "extra": "iterations: 56115553\ncpu: 74.89074018392053 ns\nthreads: 1"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "jackgerrits@users.noreply.github.com",
            "name": "Jack Gerrits",
            "username": "jackgerrits"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "c3364063fc4332d8aeb4ccf5a965d64100b54e41",
          "message": "refactor: remove unused type in allreduce (#4191)",
          "timestamp": "2022-10-05T23:51:06-04:00",
          "tree_id": "563d8cd48506e4c61d0392d63a962e4c5dc38816",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/c3364063fc4332d8aeb4ccf5a965d64100b54e41"
        },
        "date": 1665030967231,
        "tool": "benchmarkdotnet",
        "benches": [
          {
            "name": "BenchmarkText.Benchmark(args: 120_num_features)",
            "value": 7552.208760579427,
            "unit": "ns",
            "range": "± 108.41334284797054"
          },
          {
            "name": "BenchmarkText.Benchmark(args: 120_string_fts)",
            "value": 11652.876281738281,
            "unit": "ns",
            "range": "± 65.82849364877225"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 1_feature)",
            "value": 732.3094113667806,
            "unit": "ns",
            "range": "± 6.094664561048364"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 8_features)",
            "value": 617.1408144632975,
            "unit": "ns",
            "range": "± 5.362036466990614"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_interactions)",
            "value": 1059524.4065504808,
            "unit": "ns",
            "range": "± 4023.6394123582863"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_no_interactions)",
            "value": 645374.3239182692,
            "unit": "ns",
            "range": "± 4922.484348173031"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_no_namespaces)",
            "value": 647515.2971540178,
            "unit": "ns",
            "range": "± 5586.6889135958845"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_interactions)",
            "value": 1059315.557391827,
            "unit": "ns",
            "range": "± 8306.68997759511"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_no_interactions)",
            "value": 645645.1939174107,
            "unit": "ns",
            "range": "± 5167.25421675183"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_interactions)",
            "value": 4185735.3365384615,
            "unit": "ns",
            "range": "± 21804.445918475234"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_no_interactions)",
            "value": 1267267.48046875,
            "unit": "ns",
            "range": "± 9510.896900439033"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_no_namespaces)",
            "value": 1054812.2349330357,
            "unit": "ns",
            "range": "± 8203.651870602656"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_interactions)",
            "value": 3391749.9739583335,
            "unit": "ns",
            "range": "± 27792.897283480714"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_no_interactions)",
            "value": 1098305.2083333333,
            "unit": "ns",
            "range": "± 10229.59766226044"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: few_features)",
            "value": 3847.624715169271,
            "unit": "ns",
            "range": "± 37.571075824211"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: few_features)",
            "value": 13483.853963216146,
            "unit": "ns",
            "range": "± 81.80506987874331"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: many_features)",
            "value": 132095.17252604166,
            "unit": "ns",
            "range": "± 1451.8431758772874"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: many_features)",
            "value": 23065.760904947918,
            "unit": "ns",
            "range": "± 195.5961950859554"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: quadratic)",
            "value": 5015619.427083333,
            "unit": "ns",
            "range": "± 43053.45514576472"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: simple)",
            "value": 245211.97713216147,
            "unit": "ns",
            "range": "± 824.9595078114187"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "byronxu@microsoft.com",
            "name": "Byron Xu",
            "username": "byronxu99"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "c2d0f6c4d09e5b50279ae549c6387e8fec5cd39a",
          "message": "feat: Model merging with delta objects (#4177)\n\n* Add a new VW::model_delta object that internally keeps a VW::workspace\r\n\r\n* Define operators + and - such that deltas are created by subtracting two workspaces, and can be added to update a workspace\r\n\r\n* Add a merge_delta function that combines multiple deltas into a single delta\r\n\r\n* Refactor model merging to be implemented via delta merging",
          "timestamp": "2022-10-06T11:08:19-04:00",
          "tree_id": "f32dec9605d2e03b3ea22df6d2b275ee658b3052",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/c2d0f6c4d09e5b50279ae549c6387e8fec5cd39a"
        },
        "date": 1665070261087,
        "tool": "googlecpp",
        "benches": [
          {
            "name": "bench_text/120_string_fts",
            "value": 5943.494034217165,
            "unit": "ns/iter",
            "extra": "iterations: 688426\ncpu: 5941.736366726417 ns\nthreads: 1"
          },
          {
            "name": "bench_text/120_num_fts",
            "value": 4591.096964191792,
            "unit": "ns/iter",
            "extra": "iterations: 907139\ncpu: 4585.58192294676 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/8_features",
            "value": 80.99623813181043,
            "unit": "ns/iter",
            "extra": "iterations: 52123570\ncpu: 80.98741893542595 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/1_feature",
            "value": 176.46622276143472,
            "unit": "ns/iter",
            "extra": "iterations: 23007313\ncpu: 176.36561470694116 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features",
            "value": 6771.421926854758,
            "unit": "ns/iter",
            "extra": "iterations: 596018\ncpu: 6768.043079235863 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features",
            "value": 12474.025581780408,
            "unit": "ns/iter",
            "extra": "iterations: 314208\ncpu: 12473.615248497808 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features_no_predict",
            "value": 3230.5100129431075,
            "unit": "ns/iter",
            "extra": "iterations: 1203742\ncpu: 3229.6774558003303 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features_no_predic",
            "value": 4961.989071620865,
            "unit": "ns/iter",
            "extra": "iterations: 806890\ncpu: 4961.562418669217 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/few_features",
            "value": 1248.4107701785501,
            "unit": "ns/iter",
            "extra": "iterations: 3306389\ncpu: 1248.3387163458378 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/many_features/min_time:15.000",
            "value": 87561.06154209819,
            "unit": "ns/iter",
            "extra": "iterations: 249634\ncpu: 87553.93616254193 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_no_namespaces/min_time:15.000",
            "value": 318154.82427325536,
            "unit": "ns/iter",
            "extra": "iterations: 61129\ncpu: 318127.3372703628 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 324790.2518501303,
            "unit": "ns/iter",
            "extra": "iterations: 64455\ncpu: 324766.46497556433 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_interactions/min_time:15.000",
            "value": 588016.8803861121,
            "unit": "ns/iter",
            "extra": "iterations: 34394\ncpu: 587969.014944467 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_no_interactions/min_time:15.000",
            "value": 316892.72931928065,
            "unit": "ns/iter",
            "extra": "iterations: 67164\ncpu: 316870.73432195827 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_interactions/min_time:15.000",
            "value": 498905.2966027219,
            "unit": "ns/iter",
            "extra": "iterations: 44359\ncpu: 498818.48779278155 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_no_namespaces/min_time:15.000",
            "value": 600413.5799514386,
            "unit": "ns/iter",
            "extra": "iterations: 34596\ncpu: 600158.4258295757 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 807602.1295787559,
            "unit": "ns/iter",
            "extra": "iterations: 28392\ncpu: 807536.3236122855 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_interactions/min_time:15.000",
            "value": 2496368.3504871386,
            "unit": "ns/iter",
            "extra": "iterations: 8006\ncpu: 2496201.6112915305 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_no_interactions/min_time:15.000",
            "value": 659889.1572056186,
            "unit": "ns/iter",
            "extra": "iterations: 34598\ncpu: 659827.1229550854 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_interactions/min_time:15.000",
            "value": 1905894.9292985953,
            "unit": "ns/iter",
            "extra": "iterations: 10707\ncpu: 1905727.869618008 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/simple/min_time:15.000",
            "value": 68248.61461420068,
            "unit": "ns/iter",
            "extra": "iterations: 304731\ncpu: 68243.12590448624 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/quadratic/min_time:15.000",
            "value": 2825747.970727381,
            "unit": "ns/iter",
            "extra": "iterations: 6764\ncpu: 2825453.3264340605 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_string_fts",
            "value": 969.2512812807773,
            "unit": "ns/iter",
            "extra": "iterations: 4193070\ncpu: 969.1725633008681 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_string_fts",
            "value": 7053.66781255087,
            "unit": "ns/iter",
            "extra": "iterations: 599246\ncpu: 7053.137108966909 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_num_fts",
            "value": 723.3093123757294,
            "unit": "ns/iter",
            "extra": "iterations: 5748619\ncpu: 723.2501927854349 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_num_fts",
            "value": 4803.054504595554,
            "unit": "ns/iter",
            "extra": "iterations: 884641\ncpu: 4802.783841128773 ns\nthreads: 1"
          },
          {
            "name": "benchmark_example_reuse",
            "value": 854.5285026342824,
            "unit": "ns/iter",
            "extra": "iterations: 5064374\ncpu: 854.4594257849039 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_char",
            "value": 43.35808336563524,
            "unit": "ns/iter",
            "extra": "iterations: 97471361\ncpu: 43.35481680613829 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_extent",
            "value": 66.91220909145396,
            "unit": "ns/iter",
            "extra": "iterations: 66689172\ncpu: 66.9070850062429 ns\nthreads: 1"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "byronxu@microsoft.com",
            "name": "Byron Xu",
            "username": "byronxu99"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "c2d0f6c4d09e5b50279ae549c6387e8fec5cd39a",
          "message": "feat: Model merging with delta objects (#4177)\n\n* Add a new VW::model_delta object that internally keeps a VW::workspace\r\n\r\n* Define operators + and - such that deltas are created by subtracting two workspaces, and can be added to update a workspace\r\n\r\n* Add a merge_delta function that combines multiple deltas into a single delta\r\n\r\n* Refactor model merging to be implemented via delta merging",
          "timestamp": "2022-10-06T11:08:19-04:00",
          "tree_id": "f32dec9605d2e03b3ea22df6d2b275ee658b3052",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/c2d0f6c4d09e5b50279ae549c6387e8fec5cd39a"
        },
        "date": 1665071802773,
        "tool": "benchmarkdotnet",
        "benches": [
          {
            "name": "BenchmarkText.Benchmark(args: 120_num_features)",
            "value": 7853.301903509325,
            "unit": "ns",
            "range": "± 235.8230909076411"
          },
          {
            "name": "BenchmarkText.Benchmark(args: 120_string_fts)",
            "value": 11848.836878726357,
            "unit": "ns",
            "range": "± 262.8058381144619"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 1_feature)",
            "value": 773.265552520752,
            "unit": "ns",
            "range": "± 14.957908074253266"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 8_features)",
            "value": 644.9487277439663,
            "unit": "ns",
            "range": "± 6.853457096179127"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_interactions)",
            "value": 1022450.078125,
            "unit": "ns",
            "range": "± 13211.274924929883"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_no_interactions)",
            "value": 657220.4676011029,
            "unit": "ns",
            "range": "± 13450.39413408492"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_no_namespaces)",
            "value": 657586.89453125,
            "unit": "ns",
            "range": "± 11066.757317086676"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_interactions)",
            "value": 1063165.0103400736,
            "unit": "ns",
            "range": "± 21690.403393364108"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_no_interactions)",
            "value": 649965.5240885416,
            "unit": "ns",
            "range": "± 11231.212668906373"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_interactions)",
            "value": 4376279.15625,
            "unit": "ns",
            "range": "± 115282.1032302112"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_no_interactions)",
            "value": 1311617.1809895833,
            "unit": "ns",
            "range": "± 39191.86493345908"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_no_namespaces)",
            "value": 1097740.0918496621,
            "unit": "ns",
            "range": "± 36113.68481472599"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_interactions)",
            "value": 3467539.7623697915,
            "unit": "ns",
            "range": "± 87264.57473493923"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_no_interactions)",
            "value": 1086985.15625,
            "unit": "ns",
            "range": "± 12813.989602703703"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: few_features)",
            "value": 3848.3682759602866,
            "unit": "ns",
            "range": "± 64.3447419869392"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: few_features)",
            "value": 13643.115437825521,
            "unit": "ns",
            "range": "± 196.67744716592813"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: many_features)",
            "value": 133144.6533203125,
            "unit": "ns",
            "range": "± 2078.845083698681"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: many_features)",
            "value": 27882.679409450953,
            "unit": "ns",
            "range": "± 589.3488804481708"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: quadratic)",
            "value": 5107019.308035715,
            "unit": "ns",
            "range": "± 86614.52262058885"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: simple)",
            "value": 251625.6924715909,
            "unit": "ns",
            "range": "± 6027.263527248134"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "griffinbassman@gmail.com",
            "name": "Griffin Bassman",
            "username": "bassmang"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "39cc326bea88c193f2b5241a89741ea410c5a1c5",
          "message": "feat: Add ftrl to dump_weights_to_json and compat CIs (#4193)\n\n* feat: Add ftrl to dump_regressor and compat CIs\r\n\r\n* dont allow online state with ftrl",
          "timestamp": "2022-10-06T14:07:19-04:00",
          "tree_id": "d85646332a9479103344e85fa8ec5aef4196f7bf",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/39cc326bea88c193f2b5241a89741ea410c5a1c5"
        },
        "date": 1665080823759,
        "tool": "googlecpp",
        "benches": [
          {
            "name": "bench_text/120_string_fts",
            "value": 5913.072881782599,
            "unit": "ns/iter",
            "extra": "iterations: 710987\ncpu: 5885.065971670368 ns\nthreads: 1"
          },
          {
            "name": "bench_text/120_num_fts",
            "value": 4084.0393129377712,
            "unit": "ns/iter",
            "extra": "iterations: 1028262\ncpu: 4083.280623031873 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/8_features",
            "value": 68.63298588129098,
            "unit": "ns/iter",
            "extra": "iterations: 60760232\ncpu: 68.62628997203302 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/1_feature",
            "value": 164.1322253416795,
            "unit": "ns/iter",
            "extra": "iterations: 25670049\ncpu: 164.12548336000455 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features",
            "value": 6698.753547625066,
            "unit": "ns/iter",
            "extra": "iterations: 626405\ncpu: 6698.3868264142175 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features",
            "value": 12178.598213709494,
            "unit": "ns/iter",
            "extra": "iterations: 341042\ncpu: 12177.9695755948 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features_no_predict",
            "value": 3295.399502209984,
            "unit": "ns/iter",
            "extra": "iterations: 1283272\ncpu: 3294.7382160601946 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features_no_predic",
            "value": 5062.680677280752,
            "unit": "ns/iter",
            "extra": "iterations: 829966\ncpu: 5062.473402524921 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/few_features",
            "value": 1186.5250568787076,
            "unit": "ns/iter",
            "extra": "iterations: 3571634\ncpu: 1186.3999222764699 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/many_features/min_time:15.000",
            "value": 52155.859020331525,
            "unit": "ns/iter",
            "extra": "iterations: 396277\ncpu: 52152.877658809346 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_no_namespaces/min_time:15.000",
            "value": 188915.46147713557,
            "unit": "ns/iter",
            "extra": "iterations: 111635\ncpu: 188902.42844985885 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 215816.56770800307,
            "unit": "ns/iter",
            "extra": "iterations: 94494\ncpu: 215802.62027218658 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_interactions/min_time:15.000",
            "value": 452203.76238009665,
            "unit": "ns/iter",
            "extra": "iterations: 46183\ncpu: 452172.84715154895 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_no_interactions/min_time:15.000",
            "value": 189763.7730041467,
            "unit": "ns/iter",
            "extra": "iterations: 108024\ncpu: 189751.45800933134 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_interactions/min_time:15.000",
            "value": 330491.96366047044,
            "unit": "ns/iter",
            "extra": "iterations: 62604\ncpu: 330469.6952271423 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_no_namespaces/min_time:15.000",
            "value": 540478.4230267328,
            "unit": "ns/iter",
            "extra": "iterations: 39085\ncpu: 540436.8120762437 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 685957.2554239349,
            "unit": "ns/iter",
            "extra": "iterations: 31066\ncpu: 685894.801390587 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_interactions/min_time:15.000",
            "value": 2172182.114114128,
            "unit": "ns/iter",
            "extra": "iterations: 9657\ncpu: 2172027.9693486625 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_no_interactions/min_time:15.000",
            "value": 558524.9218416847,
            "unit": "ns/iter",
            "extra": "iterations: 37987\ncpu: 558476.7736330846 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_interactions/min_time:15.000",
            "value": 1714911.430285722,
            "unit": "ns/iter",
            "extra": "iterations: 12250\ncpu: 1714774.4163265328 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/simple/min_time:15.000",
            "value": 47891.19823270468,
            "unit": "ns/iter",
            "extra": "iterations: 437052\ncpu: 47888.33205202127 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/quadratic/min_time:15.000",
            "value": 2280232.2170715383,
            "unit": "ns/iter",
            "extra": "iterations: 8974\ncpu: 2279939.0461332733 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_string_fts",
            "value": 893.9509260499663,
            "unit": "ns/iter",
            "extra": "iterations: 4694833\ncpu: 893.8985263160511 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_string_fts",
            "value": 6946.610509007254,
            "unit": "ns/iter",
            "extra": "iterations: 609991\ncpu: 6946.209698175915 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_num_fts",
            "value": 678.6150257953785,
            "unit": "ns/iter",
            "extra": "iterations: 6187160\ncpu: 678.5768430103637 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_num_fts",
            "value": 4643.13308162401,
            "unit": "ns/iter",
            "extra": "iterations: 905542\ncpu: 4642.8864702023975 ns\nthreads: 1"
          },
          {
            "name": "benchmark_example_reuse",
            "value": 758.3481431245283,
            "unit": "ns/iter",
            "extra": "iterations: 5529450\ncpu: 758.3118935879739 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_char",
            "value": 43.32720790272534,
            "unit": "ns/iter",
            "extra": "iterations: 96955415\ncpu: 43.324736426531594 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_extent",
            "value": 53.6262542930096,
            "unit": "ns/iter",
            "extra": "iterations: 78066189\ncpu: 53.623250393330125 ns\nthreads: 1"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "griffinbassman@gmail.com",
            "name": "Griffin Bassman",
            "username": "bassmang"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "39cc326bea88c193f2b5241a89741ea410c5a1c5",
          "message": "feat: Add ftrl to dump_weights_to_json and compat CIs (#4193)\n\n* feat: Add ftrl to dump_regressor and compat CIs\r\n\r\n* dont allow online state with ftrl",
          "timestamp": "2022-10-06T14:07:19-04:00",
          "tree_id": "d85646332a9479103344e85fa8ec5aef4196f7bf",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/39cc326bea88c193f2b5241a89741ea410c5a1c5"
        },
        "date": 1665082586736,
        "tool": "benchmarkdotnet",
        "benches": [
          {
            "name": "BenchmarkText.Benchmark(args: 120_num_features)",
            "value": 5218.481063842773,
            "unit": "ns",
            "range": "± 53.62703546354363"
          },
          {
            "name": "BenchmarkText.Benchmark(args: 120_string_fts)",
            "value": 7178.706868489583,
            "unit": "ns",
            "range": "± 90.61207754806448"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 1_feature)",
            "value": 640.7349650065104,
            "unit": "ns",
            "range": "± 5.661301985490011"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 8_features)",
            "value": 536.8497276306152,
            "unit": "ns",
            "range": "± 4.128567480520044"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_interactions)",
            "value": 854599.6940104166,
            "unit": "ns",
            "range": "± 8987.937800381138"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_no_interactions)",
            "value": 469201.8587239583,
            "unit": "ns",
            "range": "± 5948.643123400371"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_no_namespaces)",
            "value": 463020.7726111779,
            "unit": "ns",
            "range": "± 3955.0546541981744"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_interactions)",
            "value": 860528.2389322916,
            "unit": "ns",
            "range": "± 8117.99105151636"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_no_interactions)",
            "value": 462421.78083147324,
            "unit": "ns",
            "range": "± 3470.6342822022266"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_interactions)",
            "value": 3466182.7734375,
            "unit": "ns",
            "range": "± 23707.635746410913"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_no_interactions)",
            "value": 1030036.3839285715,
            "unit": "ns",
            "range": "± 6736.4755103064335"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_no_namespaces)",
            "value": 866222.2981770834,
            "unit": "ns",
            "range": "± 2059.8230535570774"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_interactions)",
            "value": 2810435.1302083335,
            "unit": "ns",
            "range": "± 21379.93098286788"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_no_interactions)",
            "value": 878251.0602678572,
            "unit": "ns",
            "range": "± 4519.698614926095"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: few_features)",
            "value": 3328.4217580159507,
            "unit": "ns",
            "range": "± 14.353577818324187"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: few_features)",
            "value": 11218.996102469308,
            "unit": "ns",
            "range": "± 53.41376404698607"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: many_features)",
            "value": 125372.48883928571,
            "unit": "ns",
            "range": "± 155.60373959968408"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: many_features)",
            "value": 19785.533578055245,
            "unit": "ns",
            "range": "± 88.22546944975475"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: quadratic)",
            "value": 4827386.049107143,
            "unit": "ns",
            "range": "± 17488.541537719593"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: simple)",
            "value": 203973.11197916666,
            "unit": "ns",
            "range": "± 590.9394665951181"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "jackgerrits@users.noreply.github.com",
            "name": "Jack Gerrits",
            "username": "jackgerrits"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "3e17a8973e627acf18356cd6ecb2d824ad75a7a6",
          "message": "ci: settle on consistent style and add warnings for violation (#4183)\n\n* ci: settle on consistent style and add warnings for violation\r\n\r\n* Update rules for enums, macros and constants\r\n\r\n* Change to snake case",
          "timestamp": "2022-10-06T20:49:01Z",
          "tree_id": "7aeeccab0b2ba45727302d6cb873b86c6a7a4fa6",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/3e17a8973e627acf18356cd6ecb2d824ad75a7a6"
        },
        "date": 1665090576713,
        "tool": "googlecpp",
        "benches": [
          {
            "name": "bench_text/120_string_fts",
            "value": 6134.226197807843,
            "unit": "ns/iter",
            "extra": "iterations: 685210\ncpu: 6133.7509668568755 ns\nthreads: 1"
          },
          {
            "name": "bench_text/120_num_fts",
            "value": 4828.490873142814,
            "unit": "ns/iter",
            "extra": "iterations: 868426\ncpu: 4828.110973185971 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/8_features",
            "value": 78.34819065169553,
            "unit": "ns/iter",
            "extra": "iterations: 54099534\ncpu: 78.34024411374781 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/1_feature",
            "value": 165.80896031981715,
            "unit": "ns/iter",
            "extra": "iterations: 25334810\ncpu: 165.79884356740783 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features",
            "value": 6863.330958132388,
            "unit": "ns/iter",
            "extra": "iterations: 610615\ncpu: 6862.652407818351 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features",
            "value": 12268.604058190267,
            "unit": "ns/iter",
            "extra": "iterations: 341088\ncpu: 12267.62653626044 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features_no_predict",
            "value": 3269.7608197195336,
            "unit": "ns/iter",
            "extra": "iterations: 1292247\ncpu: 3269.5537308270013 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features_no_predic",
            "value": 5009.5034812555095,
            "unit": "ns/iter",
            "extra": "iterations: 840645\ncpu: 5009.156778426093 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/few_features",
            "value": 1192.791732307632,
            "unit": "ns/iter",
            "extra": "iterations: 3566751\ncpu: 1192.7183871259879 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/many_features/min_time:15.000",
            "value": 62984.1694218955,
            "unit": "ns/iter",
            "extra": "iterations: 334455\ncpu: 62979.596657248374 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_no_namespaces/min_time:15.000",
            "value": 268445.3980627875,
            "unit": "ns/iter",
            "extra": "iterations: 81354\ncpu: 268423.71364653227 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 259028.32560734864,
            "unit": "ns/iter",
            "extra": "iterations: 69894\ncpu: 258949.7367442126 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_interactions/min_time:15.000",
            "value": 482488.4288610416,
            "unit": "ns/iter",
            "extra": "iterations: 43408\ncpu: 482442.10974935506 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_no_interactions/min_time:15.000",
            "value": 212957.2017655627,
            "unit": "ns/iter",
            "extra": "iterations: 98892\ncpu: 212941.66363305412 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_interactions/min_time:15.000",
            "value": 367978.75589476066,
            "unit": "ns/iter",
            "extra": "iterations: 57127\ncpu: 367950.01487912895 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_no_namespaces/min_time:15.000",
            "value": 551196.6566711934,
            "unit": "ns/iter",
            "extra": "iterations: 38284\ncpu: 551142.4041374986 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 739213.2540909705,
            "unit": "ns/iter",
            "extra": "iterations: 28844\ncpu: 739151.1579531272 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_interactions/min_time:15.000",
            "value": 2341258.988492899,
            "unit": "ns/iter",
            "extra": "iterations: 8951\ncpu: 2341054.373812984 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_no_interactions/min_time:15.000",
            "value": 577572.9604166676,
            "unit": "ns/iter",
            "extra": "iterations: 36960\ncpu: 577513.6607142846 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_interactions/min_time:15.000",
            "value": 1755500.0505412247,
            "unit": "ns/iter",
            "extra": "iterations: 12010\ncpu: 1755329.0174854288 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/simple/min_time:15.000",
            "value": 48321.658032352294,
            "unit": "ns/iter",
            "extra": "iterations: 430453\ncpu: 48317.72713861905 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/quadratic/min_time:15.000",
            "value": 2899380.531433344,
            "unit": "ns/iter",
            "extra": "iterations: 7158\ncpu: 2899166.0938809766 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_string_fts",
            "value": 958.5577576569361,
            "unit": "ns/iter",
            "extra": "iterations: 4380311\ncpu: 958.4892031638782 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_string_fts",
            "value": 7767.419737279642,
            "unit": "ns/iter",
            "extra": "iterations: 541260\ncpu: 7762.209658943966 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_num_fts",
            "value": 746.9521409905102,
            "unit": "ns/iter",
            "extra": "iterations: 5642365\ncpu: 746.903594503372 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_num_fts",
            "value": 5795.223116745832,
            "unit": "ns/iter",
            "extra": "iterations: 725526\ncpu: 5794.8549052686985 ns\nthreads: 1"
          },
          {
            "name": "benchmark_example_reuse",
            "value": 789.5033381907158,
            "unit": "ns/iter",
            "extra": "iterations: 5317701\ncpu: 789.4480904435965 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_char",
            "value": 50.401652330716686,
            "unit": "ns/iter",
            "extra": "iterations: 83397106\ncpu: 50.39810853868283 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_extent",
            "value": 71.38939778363532,
            "unit": "ns/iter",
            "extra": "iterations: 58812533\ncpu: 71.3842081924956 ns\nthreads: 1"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "jackgerrits@users.noreply.github.com",
            "name": "Jack Gerrits",
            "username": "jackgerrits"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "4d7f37107172ba0d6cf95bb5c7be6520c83a1630",
          "message": "fix: Fix test 67 (#4194)\n\n* fix: Fix test 67\r\n\r\n* formatting",
          "timestamp": "2022-10-06T18:00:44-04:00",
          "tree_id": "df9763e8dbb0d0ca511f942422ef3f82cb1688e9",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/4d7f37107172ba0d6cf95bb5c7be6520c83a1630"
        },
        "date": 1665094805950,
        "tool": "googlecpp",
        "benches": [
          {
            "name": "bench_text/120_string_fts",
            "value": 5784.007393112209,
            "unit": "ns/iter",
            "extra": "iterations: 726487\ncpu: 5773.534970343585 ns\nthreads: 1"
          },
          {
            "name": "bench_text/120_num_fts",
            "value": 3888.098341497089,
            "unit": "ns/iter",
            "extra": "iterations: 1078141\ncpu: 3887.9090026258164 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/8_features",
            "value": 70.02300273185419,
            "unit": "ns/iter",
            "extra": "iterations: 59435810\ncpu: 70.01833574742234 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/1_feature",
            "value": 161.97538741822441,
            "unit": "ns/iter",
            "extra": "iterations: 26275545\ncpu: 161.96269573095444 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features",
            "value": 6719.984605904536,
            "unit": "ns/iter",
            "extra": "iterations: 624785\ncpu: 6719.48782381139 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features",
            "value": 12307.54497140586,
            "unit": "ns/iter",
            "extra": "iterations: 341328\ncpu: 12306.802547696063 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features_no_predict",
            "value": 3421.98957919475,
            "unit": "ns/iter",
            "extra": "iterations: 1263530\ncpu: 3421.860897643901 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features_no_predic",
            "value": 5125.897828039026,
            "unit": "ns/iter",
            "extra": "iterations: 827409\ncpu: 5125.6285585484275 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/few_features",
            "value": 1185.7942137756932,
            "unit": "ns/iter",
            "extra": "iterations: 3551919\ncpu: 1185.7371747497607 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/many_features/min_time:15.000",
            "value": 52529.343075155375,
            "unit": "ns/iter",
            "extra": "iterations: 400370\ncpu: 52522.00314708895 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_no_namespaces/min_time:15.000",
            "value": 197516.6276537208,
            "unit": "ns/iter",
            "extra": "iterations: 110835\ncpu: 197506.04682636348 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 225516.2583048876,
            "unit": "ns/iter",
            "extra": "iterations: 92867\ncpu: 225496.89125308237 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_interactions/min_time:15.000",
            "value": 457257.57815227413,
            "unit": "ns/iter",
            "extra": "iterations: 46403\ncpu: 457232.5065189751 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_no_interactions/min_time:15.000",
            "value": 195166.38097084878,
            "unit": "ns/iter",
            "extra": "iterations: 108297\ncpu: 195156.0200190216 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_interactions/min_time:15.000",
            "value": 335609.09085990716,
            "unit": "ns/iter",
            "extra": "iterations: 62844\ncpu: 335561.2548532879 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_no_namespaces/min_time:15.000",
            "value": 561740.7484807795,
            "unit": "ns/iter",
            "extra": "iterations: 38013\ncpu: 561709.68089864 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 683273.3065897913,
            "unit": "ns/iter",
            "extra": "iterations: 30699\ncpu: 683172.6440600676 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_interactions/min_time:15.000",
            "value": 2141803.4584863335,
            "unit": "ns/iter",
            "extra": "iterations: 9804\ncpu: 2141677.4173806617 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_no_interactions/min_time:15.000",
            "value": 547111.8755314931,
            "unit": "ns/iter",
            "extra": "iterations: 37865\ncpu: 546978.9488974 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_interactions/min_time:15.000",
            "value": 1619529.101225821,
            "unit": "ns/iter",
            "extra": "iterations: 12971\ncpu: 1619347.9916737317 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/simple/min_time:15.000",
            "value": 47751.460260322376,
            "unit": "ns/iter",
            "extra": "iterations: 439762\ncpu: 47749.25118586872 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/quadratic/min_time:15.000",
            "value": 2195607.8737597703,
            "unit": "ns/iter",
            "extra": "iterations: 9474\ncpu: 2195245.9362465674 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_string_fts",
            "value": 894.2510282154643,
            "unit": "ns/iter",
            "extra": "iterations: 4713020\ncpu: 894.213773758648 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_string_fts",
            "value": 6785.90675512479,
            "unit": "ns/iter",
            "extra": "iterations: 618597\ncpu: 6785.583667557373 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_num_fts",
            "value": 680.7320701615672,
            "unit": "ns/iter",
            "extra": "iterations: 6141313\ncpu: 680.7008045347985 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_num_fts",
            "value": 4490.291565884041,
            "unit": "ns/iter",
            "extra": "iterations: 936008\ncpu: 4490.122627157011 ns\nthreads: 1"
          },
          {
            "name": "benchmark_example_reuse",
            "value": 766.8201837216085,
            "unit": "ns/iter",
            "extra": "iterations: 5495270\ncpu: 766.7891841529076 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_char",
            "value": 41.2307692133564,
            "unit": "ns/iter",
            "extra": "iterations: 101602071\ncpu: 41.229053293608594 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_extent",
            "value": 54.96548776567207,
            "unit": "ns/iter",
            "extra": "iterations: 78601257\ncpu: 54.96246580382311 ns\nthreads: 1"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "jackgerrits@users.noreply.github.com",
            "name": "Jack Gerrits",
            "username": "jackgerrits"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "4d7f37107172ba0d6cf95bb5c7be6520c83a1630",
          "message": "fix: Fix test 67 (#4194)\n\n* fix: Fix test 67\r\n\r\n* formatting",
          "timestamp": "2022-10-06T18:00:44-04:00",
          "tree_id": "df9763e8dbb0d0ca511f942422ef3f82cb1688e9",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/4d7f37107172ba0d6cf95bb5c7be6520c83a1630"
        },
        "date": 1665096412686,
        "tool": "benchmarkdotnet",
        "benches": [
          {
            "name": "BenchmarkText.Benchmark(args: 120_num_features)",
            "value": 7681.588472638811,
            "unit": "ns",
            "range": "± 122.72890876985873"
          },
          {
            "name": "BenchmarkText.Benchmark(args: 120_string_fts)",
            "value": 11762.12148030599,
            "unit": "ns",
            "range": "± 178.90477952121807"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 1_feature)",
            "value": 788.1720860799154,
            "unit": "ns",
            "range": "± 13.002490723002778"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 8_features)",
            "value": 692.4556218660795,
            "unit": "ns",
            "range": "± 6.466370510239465"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_interactions)",
            "value": 1087935.44921875,
            "unit": "ns",
            "range": "± 16085.138046733278"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_no_interactions)",
            "value": 667721.435546875,
            "unit": "ns",
            "range": "± 6338.979150287742"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_no_namespaces)",
            "value": 668808.5880055147,
            "unit": "ns",
            "range": "± 13518.705561149534"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_interactions)",
            "value": 1100945.4905790442,
            "unit": "ns",
            "range": "± 22226.138737887297"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_no_interactions)",
            "value": 667324.7395833334,
            "unit": "ns",
            "range": "± 9674.427569129935"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_interactions)",
            "value": 4218233.020833333,
            "unit": "ns",
            "range": "± 55965.81667232523"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_no_interactions)",
            "value": 1303285.9765625,
            "unit": "ns",
            "range": "± 15469.191105124833"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_no_namespaces)",
            "value": 1084422.7957589286,
            "unit": "ns",
            "range": "± 10262.422292518571"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_interactions)",
            "value": 3400975.78125,
            "unit": "ns",
            "range": "± 41785.300042676005"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_no_interactions)",
            "value": 1089110.2957589286,
            "unit": "ns",
            "range": "± 17099.365209452815"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: few_features)",
            "value": 3907.625732421875,
            "unit": "ns",
            "range": "± 55.811557447442816"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: few_features)",
            "value": 13764.046834309896,
            "unit": "ns",
            "range": "± 172.28024124556524"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: many_features)",
            "value": 132734.80130709134,
            "unit": "ns",
            "range": "± 1354.6967022701467"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: many_features)",
            "value": 23005.608927408855,
            "unit": "ns",
            "range": "± 246.0166252542381"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: quadratic)",
            "value": 5143272.5,
            "unit": "ns",
            "range": "± 59034.37564780706"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: simple)",
            "value": 253230.3187779018,
            "unit": "ns",
            "range": "± 3212.836395324843"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "jackgerrits@users.noreply.github.com",
            "name": "Jack Gerrits",
            "username": "jackgerrits"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "7e6de3ee87d8daa5e5be49c0fdee5332d73fd3a6",
          "message": "style: fix style issues in config (#4198)\n\n* style: fix style issues in config\r\n\r\n* formatting",
          "timestamp": "2022-10-07T09:29:17-04:00",
          "tree_id": "4cb751cb19e40e1ddf193f81537446b9f28b0c11",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/7e6de3ee87d8daa5e5be49c0fdee5332d73fd3a6"
        },
        "date": 1665150865371,
        "tool": "googlecpp",
        "benches": [
          {
            "name": "bench_text/120_string_fts",
            "value": 7023.6110030201635,
            "unit": "ns/iter",
            "extra": "iterations: 595691\ncpu: 7019.324112669153 ns\nthreads: 1"
          },
          {
            "name": "bench_text/120_num_fts",
            "value": 5012.694716650216,
            "unit": "ns/iter",
            "extra": "iterations: 849745\ncpu: 5012.199424533243 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/8_features",
            "value": 92.32935218740758,
            "unit": "ns/iter",
            "extra": "iterations: 44814149\ncpu: 92.32230874226796 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/1_feature",
            "value": 208.83376966963786,
            "unit": "ns/iter",
            "extra": "iterations: 20580456\ncpu: 208.81819625376605 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features",
            "value": 8685.314892071367,
            "unit": "ns/iter",
            "extra": "iterations: 487313\ncpu: 8684.513854545234 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features",
            "value": 16004.854073956709,
            "unit": "ns/iter",
            "extra": "iterations: 265614\ncpu: 16003.289736233786 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features_no_predict",
            "value": 4053.392745140267,
            "unit": "ns/iter",
            "extra": "iterations: 995195\ncpu: 4053.103261169923 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features_no_predic",
            "value": 6348.222798089096,
            "unit": "ns/iter",
            "extra": "iterations: 673188\ncpu: 6345.924466865127 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/few_features",
            "value": 1534.82738715804,
            "unit": "ns/iter",
            "extra": "iterations: 2787394\ncpu: 1534.7209615863426 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/many_features/min_time:15.000",
            "value": 100680.6071579006,
            "unit": "ns/iter",
            "extra": "iterations: 211291\ncpu: 100665.5195914639 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_no_namespaces/min_time:15.000",
            "value": 332730.05438179657,
            "unit": "ns/iter",
            "extra": "iterations: 63992\ncpu: 332705.9679334916 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 369458.57389808894,
            "unit": "ns/iter",
            "extra": "iterations: 56402\ncpu: 369426.3767242297 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_interactions/min_time:15.000",
            "value": 664960.3601560065,
            "unit": "ns/iter",
            "extra": "iterations: 32050\ncpu: 664910.8954758196 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_no_interactions/min_time:15.000",
            "value": 353825.6807429873,
            "unit": "ns/iter",
            "extra": "iterations: 59059\ncpu: 353797.6277959329 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_interactions/min_time:15.000",
            "value": 545457.3260380311,
            "unit": "ns/iter",
            "extra": "iterations: 38655\ncpu: 545415.9875824603 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_no_namespaces/min_time:15.000",
            "value": 751010.8286558895,
            "unit": "ns/iter",
            "extra": "iterations: 27401\ncpu: 750931.1229517171 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 937996.7806344482,
            "unit": "ns/iter",
            "extra": "iterations: 22287\ncpu: 937918.5489298701 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_interactions/min_time:15.000",
            "value": 3352406.430881651,
            "unit": "ns/iter",
            "extra": "iterations: 6431\ncpu: 3352121.3030632855 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_no_interactions/min_time:15.000",
            "value": 797425.5400965482,
            "unit": "ns/iter",
            "extra": "iterations: 26723\ncpu: 797349.2384837033 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_interactions/min_time:15.000",
            "value": 2642769.9766778327,
            "unit": "ns/iter",
            "extra": "iterations: 8061\ncpu: 2642497.1839722083 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/simple/min_time:15.000",
            "value": 84251.35588938367,
            "unit": "ns/iter",
            "extra": "iterations: 250162\ncpu: 84245.72117268005 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/quadratic/min_time:15.000",
            "value": 3450631.594248684,
            "unit": "ns/iter",
            "extra": "iterations: 6016\ncpu: 3450355.6349734045 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_string_fts",
            "value": 1170.3034057095551,
            "unit": "ns/iter",
            "extra": "iterations: 3552916\ncpu: 1170.2274413467685 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_string_fts",
            "value": 8486.972708591438,
            "unit": "ns/iter",
            "extra": "iterations: 497336\ncpu: 8486.396520662116 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_num_fts",
            "value": 909.5219435310091,
            "unit": "ns/iter",
            "extra": "iterations: 4521173\ncpu: 909.4538519096725 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_num_fts",
            "value": 5854.238573181729,
            "unit": "ns/iter",
            "extra": "iterations: 730212\ncpu: 5853.857509873788 ns\nthreads: 1"
          },
          {
            "name": "benchmark_example_reuse",
            "value": 1017.9135275841099,
            "unit": "ns/iter",
            "extra": "iterations: 4137435\ncpu: 1017.850697352341 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_char",
            "value": 55.62873363327272,
            "unit": "ns/iter",
            "extra": "iterations: 73304903\ncpu: 55.6250568942165 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_extent",
            "value": 79.401257274013,
            "unit": "ns/iter",
            "extra": "iterations: 53163749\ncpu: 79.39594703902517 ns\nthreads: 1"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "jackgerrits@users.noreply.github.com",
            "name": "Jack Gerrits",
            "username": "jackgerrits"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "7e6de3ee87d8daa5e5be49c0fdee5332d73fd3a6",
          "message": "style: fix style issues in config (#4198)\n\n* style: fix style issues in config\r\n\r\n* formatting",
          "timestamp": "2022-10-07T09:29:17-04:00",
          "tree_id": "4cb751cb19e40e1ddf193f81537446b9f28b0c11",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/7e6de3ee87d8daa5e5be49c0fdee5332d73fd3a6"
        },
        "date": 1665152266975,
        "tool": "benchmarkdotnet",
        "benches": [
          {
            "name": "BenchmarkText.Benchmark(args: 120_num_features)",
            "value": 7490.427398681641,
            "unit": "ns",
            "range": "± 129.63419616535114"
          },
          {
            "name": "BenchmarkText.Benchmark(args: 120_string_fts)",
            "value": 11506.835174560547,
            "unit": "ns",
            "range": "± 180.38839954060506"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 1_feature)",
            "value": 734.9504979451498,
            "unit": "ns",
            "range": "± 10.616155901073666"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 8_features)",
            "value": 595.0436557422985,
            "unit": "ns",
            "range": "± 25.15711653623824"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_interactions)",
            "value": 1061844.8697916667,
            "unit": "ns",
            "range": "± 9550.429062621528"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_no_interactions)",
            "value": 728896.142578125,
            "unit": "ns",
            "range": "± 7751.2161176071895"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_no_namespaces)",
            "value": 666726.5567555147,
            "unit": "ns",
            "range": "± 13266.359163979723"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_interactions)",
            "value": 1021635.7722355769,
            "unit": "ns",
            "range": "± 12411.031676216715"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_no_interactions)",
            "value": 640710.9993489584,
            "unit": "ns",
            "range": "± 10483.12371456002"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_interactions)",
            "value": 4189090.625,
            "unit": "ns",
            "range": "± 73932.50331534779"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_no_interactions)",
            "value": 1259765.7942708333,
            "unit": "ns",
            "range": "± 16019.80661649795"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_no_namespaces)",
            "value": 1323545.7589285714,
            "unit": "ns",
            "range": "± 15854.971454565622"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_interactions)",
            "value": 3358194.110576923,
            "unit": "ns",
            "range": "± 53312.93618145469"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_no_interactions)",
            "value": 1102214.2838541667,
            "unit": "ns",
            "range": "± 16576.885116167265"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: few_features)",
            "value": 3871.1763509114585,
            "unit": "ns",
            "range": "± 69.25768583529951"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: few_features)",
            "value": 13576.205851236979,
            "unit": "ns",
            "range": "± 236.71279023223087"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: many_features)",
            "value": 125143.27204777644,
            "unit": "ns",
            "range": "± 3324.1752928222068"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: many_features)",
            "value": 22560.565403529577,
            "unit": "ns",
            "range": "± 396.73755815566244"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: quadratic)",
            "value": 4875294.299768519,
            "unit": "ns",
            "range": "± 133906.05577670093"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: simple)",
            "value": 246931.5638950893,
            "unit": "ns",
            "range": "± 8014.145145520024"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "jackgerrits@users.noreply.github.com",
            "name": "Jack Gerrits",
            "username": "jackgerrits"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "691419a21a0c6eb428584ebd0b0c8c0868e309d2",
          "message": "style: don't warn on short variable names, add static constant rule (#4196)\n\n* style: don't warn on short variable names\r\n\r\n* Update .clang-tidy",
          "timestamp": "2022-10-07T10:22:10-04:00",
          "tree_id": "4631a98e51a391e36276c42bc6f0d37f10f50030",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/691419a21a0c6eb428584ebd0b0c8c0868e309d2"
        },
        "date": 1665153985265,
        "tool": "googlecpp",
        "benches": [
          {
            "name": "bench_text/120_string_fts",
            "value": 6681.726819257628,
            "unit": "ns/iter",
            "extra": "iterations: 625681\ncpu: 6672.392321326683 ns\nthreads: 1"
          },
          {
            "name": "bench_text/120_num_fts",
            "value": 4753.233087401386,
            "unit": "ns/iter",
            "extra": "iterations: 874496\ncpu: 4752.32385282494 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/8_features",
            "value": 85.70256112515355,
            "unit": "ns/iter",
            "extra": "iterations: 48632883\ncpu: 85.69381749381378 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/1_feature",
            "value": 192.19538201331014,
            "unit": "ns/iter",
            "extra": "iterations: 22246188\ncpu: 192.17661470810188 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features",
            "value": 8249.171127550222,
            "unit": "ns/iter",
            "extra": "iterations: 497734\ncpu: 8248.354341877388 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features",
            "value": 14546.554103643744,
            "unit": "ns/iter",
            "extra": "iterations: 288585\ncpu: 14545.095898955235 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features_no_predict",
            "value": 3944.3187312071464,
            "unit": "ns/iter",
            "extra": "iterations: 1065895\ncpu: 3943.891846757885 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features_no_predic",
            "value": 5845.37965015924,
            "unit": "ns/iter",
            "extra": "iterations: 725244\ncpu: 5844.838978330049 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/few_features",
            "value": 1470.3372255131783,
            "unit": "ns/iter",
            "extra": "iterations: 2819671\ncpu: 1470.1985799052438 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/many_features/min_time:15.000",
            "value": 98305.71301317321,
            "unit": "ns/iter",
            "extra": "iterations: 210041\ncpu: 98296.8363319542 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_no_namespaces/min_time:15.000",
            "value": 348546.6508832527,
            "unit": "ns/iter",
            "extra": "iterations: 61647\ncpu: 348514.35106331215 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 368357.5467584067,
            "unit": "ns/iter",
            "extra": "iterations: 58428\ncpu: 368327.1000205381 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_interactions/min_time:15.000",
            "value": 641163.1977047808,
            "unit": "ns/iter",
            "extra": "iterations: 32938\ncpu: 641112.7239055198 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_no_interactions/min_time:15.000",
            "value": 344192.1065541585,
            "unit": "ns/iter",
            "extra": "iterations: 61152\ncpu: 344114.76811878616 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_interactions/min_time:15.000",
            "value": 487198.71224165335,
            "unit": "ns/iter",
            "extra": "iterations: 41514\ncpu: 487126.5308088838 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_no_namespaces/min_time:15.000",
            "value": 712685.5911936912,
            "unit": "ns/iter",
            "extra": "iterations: 29547\ncpu: 712619.6669712662 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 888717.105602466,
            "unit": "ns/iter",
            "extra": "iterations: 24043\ncpu: 888642.9563698362 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_interactions/min_time:15.000",
            "value": 2836308.3609898062,
            "unit": "ns/iter",
            "extra": "iterations: 7557\ncpu: 2836086.581976974 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_no_interactions/min_time:15.000",
            "value": 725716.520188282,
            "unit": "ns/iter",
            "extra": "iterations: 28680\ncpu: 725651.5585774077 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_interactions/min_time:15.000",
            "value": 2467710.017313432,
            "unit": "ns/iter",
            "extra": "iterations: 8375\ncpu: 2467495.7014925377 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/simple/min_time:15.000",
            "value": 73150.8952586358,
            "unit": "ns/iter",
            "extra": "iterations: 285150\ncpu: 73145.9950903033 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/quadratic/min_time:15.000",
            "value": 2701475.7607469186,
            "unit": "ns/iter",
            "extra": "iterations: 7444\ncpu: 2701260.330467495 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_string_fts",
            "value": 1114.1588002795702,
            "unit": "ns/iter",
            "extra": "iterations: 3626845\ncpu: 1114.0958050316392 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_string_fts",
            "value": 7871.17490500791,
            "unit": "ns/iter",
            "extra": "iterations: 525045\ncpu: 7870.576998161998 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_num_fts",
            "value": 887.9945841520387,
            "unit": "ns/iter",
            "extra": "iterations: 4845040\ncpu: 887.9171482588383 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_num_fts",
            "value": 5430.579812452928,
            "unit": "ns/iter",
            "extra": "iterations: 761836\ncpu: 5430.1416577846185 ns\nthreads: 1"
          },
          {
            "name": "benchmark_example_reuse",
            "value": 960.3514565099297,
            "unit": "ns/iter",
            "extra": "iterations: 4481775\ncpu: 960.2779032860947 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_char",
            "value": 51.64804129730697,
            "unit": "ns/iter",
            "extra": "iterations: 78187568\ncpu: 51.64384189568343 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_extent",
            "value": 73.8993341953676,
            "unit": "ns/iter",
            "extra": "iterations: 55880356\ncpu: 73.89397447646887 ns\nthreads: 1"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "jackgerrits@users.noreply.github.com",
            "name": "Jack Gerrits",
            "username": "jackgerrits"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "691419a21a0c6eb428584ebd0b0c8c0868e309d2",
          "message": "style: don't warn on short variable names, add static constant rule (#4196)\n\n* style: don't warn on short variable names\r\n\r\n* Update .clang-tidy",
          "timestamp": "2022-10-07T10:22:10-04:00",
          "tree_id": "4631a98e51a391e36276c42bc6f0d37f10f50030",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/691419a21a0c6eb428584ebd0b0c8c0868e309d2"
        },
        "date": 1665155267016,
        "tool": "benchmarkdotnet",
        "benches": [
          {
            "name": "BenchmarkText.Benchmark(args: 120_num_features)",
            "value": 7052.438672383626,
            "unit": "ns",
            "range": "± 272.359619123009"
          },
          {
            "name": "BenchmarkText.Benchmark(args: 120_string_fts)",
            "value": 10444.515845889136,
            "unit": "ns",
            "range": "± 225.74230341821743"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 1_feature)",
            "value": 803.5908997058868,
            "unit": "ns",
            "range": "± 13.209867936017304"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 8_features)",
            "value": 650.4302151997884,
            "unit": "ns",
            "range": "± 19.416288991573776"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_interactions)",
            "value": 1093153.125,
            "unit": "ns",
            "range": "± 15932.1804546922"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_no_interactions)",
            "value": 708379.4487847222,
            "unit": "ns",
            "range": "± 14843.91708476403"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_no_namespaces)",
            "value": 698734.5108695652,
            "unit": "ns",
            "range": "± 17330.040557543"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_interactions)",
            "value": 1126970.46875,
            "unit": "ns",
            "range": "± 29913.679917111425"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_no_interactions)",
            "value": 665311.8815104166,
            "unit": "ns",
            "range": "± 11235.32015146404"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_interactions)",
            "value": 4979992.083333333,
            "unit": "ns",
            "range": "± 65551.75970624111"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_no_interactions)",
            "value": 1385833.140625,
            "unit": "ns",
            "range": "± 35736.84209728482"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_no_namespaces)",
            "value": 1113008.677455357,
            "unit": "ns",
            "range": "± 18704.5676257288"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_interactions)",
            "value": 4003532.65625,
            "unit": "ns",
            "range": "± 152048.3970324363"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_no_interactions)",
            "value": 1207736.6536458333,
            "unit": "ns",
            "range": "± 18837.07758162616"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: few_features)",
            "value": 4138.523991902669,
            "unit": "ns",
            "range": "± 68.85145978585157"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: few_features)",
            "value": 14038.698120117188,
            "unit": "ns",
            "range": "± 198.8767342718267"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: many_features)",
            "value": 144455.65773292823,
            "unit": "ns",
            "range": "± 3915.199225662568"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: many_features)",
            "value": 24900.447252061633,
            "unit": "ns",
            "range": "± 516.6430340608174"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: quadratic)",
            "value": 5576023.149671053,
            "unit": "ns",
            "range": "± 121603.95860759515"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: simple)",
            "value": 306717.27120535716,
            "unit": "ns",
            "range": "± 4502.734253477453"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "jackgerrits@users.noreply.github.com",
            "name": "Jack Gerrits",
            "username": "jackgerrits"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "fdd97d2679bf6bf9ab14bb979453740f5b64fdf6",
          "message": "style: move action scores into VW namespace (#4199)",
          "timestamp": "2022-10-07T11:54:27-04:00",
          "tree_id": "2460f48ac910b3e0cd5f8bd2933aced0e2d66009",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/fdd97d2679bf6bf9ab14bb979453740f5b64fdf6"
        },
        "date": 1665159273906,
        "tool": "googlecpp",
        "benches": [
          {
            "name": "bench_text/120_string_fts",
            "value": 5781.938420492488,
            "unit": "ns/iter",
            "extra": "iterations: 723390\ncpu: 5781.390674463291 ns\nthreads: 1"
          },
          {
            "name": "bench_text/120_num_fts",
            "value": 4494.711504553378,
            "unit": "ns/iter",
            "extra": "iterations: 935786\ncpu: 4494.3745685445165 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/8_features",
            "value": 76.16166010831128,
            "unit": "ns/iter",
            "extra": "iterations: 52832601\ncpu: 76.15724995254351 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/1_feature",
            "value": 162.62615968093692,
            "unit": "ns/iter",
            "extra": "iterations: 25917258\ncpu: 162.6173301203391 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features",
            "value": 6842.560269438413,
            "unit": "ns/iter",
            "extra": "iterations: 612682\ncpu: 6842.0129855292 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features",
            "value": 12243.891612730215,
            "unit": "ns/iter",
            "extra": "iterations: 343306\ncpu: 12242.286764577375 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features_no_predict",
            "value": 3325.57461797359,
            "unit": "ns/iter",
            "extra": "iterations: 1282372\ncpu: 3325.209299641598 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features_no_predic",
            "value": 4933.337768729408,
            "unit": "ns/iter",
            "extra": "iterations: 844344\ncpu: 4933.059155983817 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/few_features",
            "value": 1177.1181573550377,
            "unit": "ns/iter",
            "extra": "iterations: 3568631\ncpu: 1176.4899481061489 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/many_features/min_time:15.000",
            "value": 57404.77598969962,
            "unit": "ns/iter",
            "extra": "iterations: 359604\ncpu: 57400.833972925786 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_no_namespaces/min_time:15.000",
            "value": 269540.44521496206,
            "unit": "ns/iter",
            "extra": "iterations: 87387\ncpu: 269488.9102498083 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 296448.2237950345,
            "unit": "ns/iter",
            "extra": "iterations: 70998\ncpu: 296388.45178737416 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_interactions/min_time:15.000",
            "value": 538231.4704546074,
            "unit": "ns/iter",
            "extra": "iterations: 38737\ncpu: 538187.1053514731 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_no_interactions/min_time:15.000",
            "value": 233677.78345715185,
            "unit": "ns/iter",
            "extra": "iterations: 77532\ncpu: 233570.86106381885 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_interactions/min_time:15.000",
            "value": 350059.20379462646,
            "unit": "ns/iter",
            "extra": "iterations: 60085\ncpu: 350033.1214113341 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_no_namespaces/min_time:15.000",
            "value": 542605.5798012937,
            "unit": "ns/iter",
            "extra": "iterations: 38953\ncpu: 542502.2565656042 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 720976.5180990832,
            "unit": "ns/iter",
            "extra": "iterations: 29228\ncpu: 720912.8438483644 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_interactions/min_time:15.000",
            "value": 2267080.1404556637,
            "unit": "ns/iter",
            "extra": "iterations: 9042\ncpu: 2266873.468259233 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_no_interactions/min_time:15.000",
            "value": 558815.6779409789,
            "unit": "ns/iter",
            "extra": "iterations: 37785\ncpu: 558766.2908561612 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_interactions/min_time:15.000",
            "value": 1647161.6594845138,
            "unit": "ns/iter",
            "extra": "iterations: 12299\ncpu: 1647030.498414506 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/simple/min_time:15.000",
            "value": 48299.52394889929,
            "unit": "ns/iter",
            "extra": "iterations: 438350\ncpu: 48291.56153758415 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/quadratic/min_time:15.000",
            "value": 2832734.3741311226,
            "unit": "ns/iter",
            "extra": "iterations: 7337\ncpu: 2832509.4861660055 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_string_fts",
            "value": 966.4698395734696,
            "unit": "ns/iter",
            "extra": "iterations: 4345479\ncpu: 966.410055140063 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_string_fts",
            "value": 7429.987724969797,
            "unit": "ns/iter",
            "extra": "iterations: 563909\ncpu: 7429.566472604658 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_num_fts",
            "value": 742.5417262792814,
            "unit": "ns/iter",
            "extra": "iterations: 5629402\ncpu: 742.4923819617068 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_num_fts",
            "value": 5393.739454805247,
            "unit": "ns/iter",
            "extra": "iterations: 774879\ncpu: 5393.432523013307 ns\nthreads: 1"
          },
          {
            "name": "benchmark_example_reuse",
            "value": 846.461748991293,
            "unit": "ns/iter",
            "extra": "iterations: 4961477\ncpu: 846.0041233689075 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_char",
            "value": 51.28357891510179,
            "unit": "ns/iter",
            "extra": "iterations: 82012060\ncpu: 51.280727493005145 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_extent",
            "value": 72.59507539393546,
            "unit": "ns/iter",
            "extra": "iterations: 57866192\ncpu: 72.59043242382334 ns\nthreads: 1"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "jackgerrits@users.noreply.github.com",
            "name": "Jack Gerrits",
            "username": "jackgerrits"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "fdd97d2679bf6bf9ab14bb979453740f5b64fdf6",
          "message": "style: move action scores into VW namespace (#4199)",
          "timestamp": "2022-10-07T11:54:27-04:00",
          "tree_id": "2460f48ac910b3e0cd5f8bd2933aced0e2d66009",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/fdd97d2679bf6bf9ab14bb979453740f5b64fdf6"
        },
        "date": 1665161241433,
        "tool": "benchmarkdotnet",
        "benches": [
          {
            "name": "BenchmarkText.Benchmark(args: 120_num_features)",
            "value": 7640.9836832682295,
            "unit": "ns",
            "range": "± 138.16552317150212"
          },
          {
            "name": "BenchmarkText.Benchmark(args: 120_string_fts)",
            "value": 11654.613622029623,
            "unit": "ns",
            "range": "± 297.15850455589936"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 1_feature)",
            "value": 767.2277859279087,
            "unit": "ns",
            "range": "± 21.055239423735422"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 8_features)",
            "value": 638.9860303778397,
            "unit": "ns",
            "range": "± 13.994619230740675"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_interactions)",
            "value": 1061976.07421875,
            "unit": "ns",
            "range": "± 13604.426160136416"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_no_interactions)",
            "value": 611816.3783482143,
            "unit": "ns",
            "range": "± 16531.666839860125"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_no_namespaces)",
            "value": 649078.3353365385,
            "unit": "ns",
            "range": "± 10590.296767798585"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_interactions)",
            "value": 1046783.0078125,
            "unit": "ns",
            "range": "± 21612.524752169"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_no_interactions)",
            "value": 618029.8828125,
            "unit": "ns",
            "range": "± 9838.57725175703"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_interactions)",
            "value": 4209277.228860294,
            "unit": "ns",
            "range": "± 135498.1090514814"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_no_interactions)",
            "value": 1292671.346507353,
            "unit": "ns",
            "range": "± 52618.1092642427"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_no_namespaces)",
            "value": 1106194.935825893,
            "unit": "ns",
            "range": "± 15834.489393219083"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_interactions)",
            "value": 3466368.107358871,
            "unit": "ns",
            "range": "± 104004.72038357743"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_no_interactions)",
            "value": 1111567.9236778845,
            "unit": "ns",
            "range": "± 15839.646063928745"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: few_features)",
            "value": 3837.150319417318,
            "unit": "ns",
            "range": "± 37.93425427904112"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: few_features)",
            "value": 13608.05695851644,
            "unit": "ns",
            "range": "± 345.07348653460014"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: many_features)",
            "value": 135218.26428865132,
            "unit": "ns",
            "range": "± 2881.481624377197"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: many_features)",
            "value": 22776.1351449149,
            "unit": "ns",
            "range": "± 652.2746002577238"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: quadratic)",
            "value": 5073435.044642857,
            "unit": "ns",
            "range": "± 73669.16560389323"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: simple)",
            "value": 256918.43813004033,
            "unit": "ns",
            "range": "± 11645.310193543635"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "jackgerrits@users.noreply.github.com",
            "name": "Jack Gerrits",
            "username": "jackgerrits"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "18508a9d8d9522e385621ab936ea43d0e21993ae",
          "message": "style: update allreduce to snake_case (#4197)\n\n* style: update allreduce to snake_case\r\n\r\n* fix naming collisions\r\n\r\n* fix c#\r\n\r\n* fmt",
          "timestamp": "2022-10-07T13:10:09-04:00",
          "tree_id": "fa296580aefde99a14cd5e45760384a7772650fa",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/18508a9d8d9522e385621ab936ea43d0e21993ae"
        },
        "date": 1665163973573,
        "tool": "googlecpp",
        "benches": [
          {
            "name": "bench_text/120_string_fts",
            "value": 7178.037801588425,
            "unit": "ns/iter",
            "extra": "iterations: 583494\ncpu: 7175.448762112377 ns\nthreads: 1"
          },
          {
            "name": "bench_text/120_num_fts",
            "value": 5674.564005965164,
            "unit": "ns/iter",
            "extra": "iterations: 744290\ncpu: 5663.8908221257825 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/8_features",
            "value": 88.40091140863302,
            "unit": "ns/iter",
            "extra": "iterations: 43694122\ncpu: 88.38415382279568 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/1_feature",
            "value": 197.5961239313357,
            "unit": "ns/iter",
            "extra": "iterations: 21076510\ncpu: 197.56355297912236 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features",
            "value": 8201.537724914855,
            "unit": "ns/iter",
            "extra": "iterations: 506469\ncpu: 8199.962485364362 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features",
            "value": 15053.248767327213,
            "unit": "ns/iter",
            "extra": "iterations: 279474\ncpu: 15050.662315635802 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features_no_predict",
            "value": 3929.973640320682,
            "unit": "ns/iter",
            "extra": "iterations: 1072661\ncpu: 3929.2846481786933 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features_no_predic",
            "value": 6009.455953179078,
            "unit": "ns/iter",
            "extra": "iterations: 693705\ncpu: 6008.423032845375 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/few_features",
            "value": 1441.7321151185004,
            "unit": "ns/iter",
            "extra": "iterations: 2835132\ncpu: 1441.4745415733735 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/many_features/min_time:15.000",
            "value": 71112.6382929197,
            "unit": "ns/iter",
            "extra": "iterations: 296436\ncpu: 71090.52173150363 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_no_namespaces/min_time:15.000",
            "value": 306833.31755171187,
            "unit": "ns/iter",
            "extra": "iterations: 68603\ncpu: 306776.7954754164 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 345573.624023055,
            "unit": "ns/iter",
            "extra": "iterations: 61416\ncpu: 345504.2057444314 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_interactions/min_time:15.000",
            "value": 638469.8794142716,
            "unit": "ns/iter",
            "extra": "iterations: 32848\ncpu: 638355.8390160742 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_no_interactions/min_time:15.000",
            "value": 308030.97909361636,
            "unit": "ns/iter",
            "extra": "iterations: 68161\ncpu: 307972.89212306164 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_interactions/min_time:15.000",
            "value": 484198.5001155262,
            "unit": "ns/iter",
            "extra": "iterations: 43280\ncpu: 484046.7097966729 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_no_namespaces/min_time:15.000",
            "value": 708543.834084455,
            "unit": "ns/iter",
            "extra": "iterations: 29955\ncpu: 708406.83692205 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 876103.2884144094,
            "unit": "ns/iter",
            "extra": "iterations: 24073\ncpu: 875829.1239147602 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_interactions/min_time:15.000",
            "value": 2904773.9504132112,
            "unit": "ns/iter",
            "extra": "iterations: 7260\ncpu: 2904214.0220385664 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_no_interactions/min_time:15.000",
            "value": 722839.0349535232,
            "unit": "ns/iter",
            "extra": "iterations: 29153\ncpu: 722604.9909100267 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_interactions/min_time:15.000",
            "value": 2212356.4278035327,
            "unit": "ns/iter",
            "extra": "iterations: 9488\ncpu: 2211934.664839798 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/simple/min_time:15.000",
            "value": 64636.83175710691,
            "unit": "ns/iter",
            "extra": "iterations: 332228\ncpu: 64617.15869824327 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/quadratic/min_time:15.000",
            "value": 3421471.83445782,
            "unit": "ns/iter",
            "extra": "iterations: 6077\ncpu: 3420853.9575448474 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_string_fts",
            "value": 1133.0633800373425,
            "unit": "ns/iter",
            "extra": "iterations: 3689332\ncpu: 1132.8692836535263 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_string_fts",
            "value": 9437.060414892967,
            "unit": "ns/iter",
            "extra": "iterations: 447489\ncpu: 9435.334723311684 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_num_fts",
            "value": 858.1859551810281,
            "unit": "ns/iter",
            "extra": "iterations: 4851782\ncpu: 858.0505059790391 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_num_fts",
            "value": 6755.311548059368,
            "unit": "ns/iter",
            "extra": "iterations: 618736\ncpu: 6754.192256471293 ns\nthreads: 1"
          },
          {
            "name": "benchmark_example_reuse",
            "value": 966.7639644269034,
            "unit": "ns/iter",
            "extra": "iterations: 4354869\ncpu: 966.5969975216292 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_char",
            "value": 61.41610783073668,
            "unit": "ns/iter",
            "extra": "iterations: 68209346\ncpu: 61.406749157219394 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_extent",
            "value": 86.74463066865631,
            "unit": "ns/iter",
            "extra": "iterations: 48216013\ncpu: 86.72966385669356 ns\nthreads: 1"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "jackgerrits@users.noreply.github.com",
            "name": "Jack Gerrits",
            "username": "jackgerrits"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "18508a9d8d9522e385621ab936ea43d0e21993ae",
          "message": "style: update allreduce to snake_case (#4197)\n\n* style: update allreduce to snake_case\r\n\r\n* fix naming collisions\r\n\r\n* fix c#\r\n\r\n* fmt",
          "timestamp": "2022-10-07T13:10:09-04:00",
          "tree_id": "fa296580aefde99a14cd5e45760384a7772650fa",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/18508a9d8d9522e385621ab936ea43d0e21993ae"
        },
        "date": 1665165845305,
        "tool": "benchmarkdotnet",
        "benches": [
          {
            "name": "BenchmarkText.Benchmark(args: 120_num_features)",
            "value": 6404.062398274739,
            "unit": "ns",
            "range": "± 100.64543338317799"
          },
          {
            "name": "BenchmarkText.Benchmark(args: 120_string_fts)",
            "value": 9808.463832310268,
            "unit": "ns",
            "range": "± 54.83266521915785"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 1_feature)",
            "value": 650.5150159200033,
            "unit": "ns",
            "range": "± 1.846907831553896"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 8_features)",
            "value": 565.8921559651693,
            "unit": "ns",
            "range": "± 2.4357868413284995"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_interactions)",
            "value": 911135.64453125,
            "unit": "ns",
            "range": "± 3316.16055429319"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_no_interactions)",
            "value": 566180.029296875,
            "unit": "ns",
            "range": "± 2925.536067515484"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_no_namespaces)",
            "value": 572792.8571428572,
            "unit": "ns",
            "range": "± 3168.96463785877"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_interactions)",
            "value": 906179.0234375,
            "unit": "ns",
            "range": "± 3865.14993790246"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_no_interactions)",
            "value": 572699.619140625,
            "unit": "ns",
            "range": "± 3695.2740638367695"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_interactions)",
            "value": 3546781.015625,
            "unit": "ns",
            "range": "± 20902.39742022303"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_no_interactions)",
            "value": 1071887.3046875,
            "unit": "ns",
            "range": "± 2791.359915510266"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_no_namespaces)",
            "value": 923996.5401785715,
            "unit": "ns",
            "range": "± 2380.0437759468955"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_interactions)",
            "value": 2864028.5714285714,
            "unit": "ns",
            "range": "± 12991.182023716212"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_no_interactions)",
            "value": 939725.6685697115,
            "unit": "ns",
            "range": "± 2488.0727718244293"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: few_features)",
            "value": 3242.471640450614,
            "unit": "ns",
            "range": "± 10.888148484559208"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: few_features)",
            "value": 12733.824920654297,
            "unit": "ns",
            "range": "± 19.437858437913587"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: many_features)",
            "value": 109719.19294084821,
            "unit": "ns",
            "range": "± 250.85221272840928"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: many_features)",
            "value": 18917.921956380207,
            "unit": "ns",
            "range": "± 71.51563790193727"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: quadratic)",
            "value": 4395542.473958333,
            "unit": "ns",
            "range": "± 60183.10058947968"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: simple)",
            "value": 215237.57847377233,
            "unit": "ns",
            "range": "± 923.0277604692825"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "jackgerrits@users.noreply.github.com",
            "name": "Jack Gerrits",
            "username": "jackgerrits"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "c3607b45c47e2f8be2951aab954d0f2ed2ffe5cb",
          "message": "fix: small build fixes for LAS on MacOS (#4202)",
          "timestamp": "2022-10-07T14:51:31-04:00",
          "tree_id": "df2284fadfbbd4f86db1221eab72807cbd12ac3e",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/c3607b45c47e2f8be2951aab954d0f2ed2ffe5cb"
        },
        "date": 1665170093047,
        "tool": "googlecpp",
        "benches": [
          {
            "name": "bench_text/120_string_fts",
            "value": 7308.129139414214,
            "unit": "ns/iter",
            "extra": "iterations: 572273\ncpu: 7307.163539080125 ns\nthreads: 1"
          },
          {
            "name": "bench_text/120_num_fts",
            "value": 5670.262444188847,
            "unit": "ns/iter",
            "extra": "iterations: 742234\ncpu: 5669.330022607426 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/8_features",
            "value": 88.6583604206968,
            "unit": "ns/iter",
            "extra": "iterations: 42158034\ncpu: 88.64139632317767 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/1_feature",
            "value": 199.08598408305312,
            "unit": "ns/iter",
            "extra": "iterations: 21084821\ncpu: 199.04369593652217 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features",
            "value": 8286.349144042939,
            "unit": "ns/iter",
            "extra": "iterations: 502128\ncpu: 8284.436040212851 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features",
            "value": 15036.068856790347,
            "unit": "ns/iter",
            "extra": "iterations: 277823\ncpu: 15032.717593575771 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features_no_predict",
            "value": 4005.8917441345025,
            "unit": "ns/iter",
            "extra": "iterations: 1051943\ncpu: 4005.2716734651926 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features_no_predic",
            "value": 6100.163502253009,
            "unit": "ns/iter",
            "extra": "iterations: 691966\ncpu: 6099.301844310267 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/few_features",
            "value": 1452.7996045868667,
            "unit": "ns/iter",
            "extra": "iterations: 2913914\ncpu: 1451.7846442962978 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/many_features/min_time:15.000",
            "value": 70327.91810628484,
            "unit": "ns/iter",
            "extra": "iterations: 299327\ncpu: 70316.67306992022 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_no_namespaces/min_time:15.000",
            "value": 321955.79791375657,
            "unit": "ns/iter",
            "extra": "iterations: 67298\ncpu: 321894.29700734053 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 356492.4682464596,
            "unit": "ns/iter",
            "extra": "iterations: 59206\ncpu: 356437.9944600211 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_interactions/min_time:15.000",
            "value": 650225.8952223602,
            "unit": "ns/iter",
            "extra": "iterations: 32087\ncpu: 650124.3244927854 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_no_interactions/min_time:15.000",
            "value": 324220.84115769604,
            "unit": "ns/iter",
            "extra": "iterations: 64542\ncpu: 324172.24288060475 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_interactions/min_time:15.000",
            "value": 496984.8381583395,
            "unit": "ns/iter",
            "extra": "iterations: 41658\ncpu: 496865.22636708396 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_no_namespaces/min_time:15.000",
            "value": 719282.451757233,
            "unit": "ns/iter",
            "extra": "iterations: 29279\ncpu: 718973.462208409 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 881299.310120289,
            "unit": "ns/iter",
            "extra": "iterations: 24110\ncpu: 880922.7623392775 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_interactions/min_time:15.000",
            "value": 2816262.7819630112,
            "unit": "ns/iter",
            "extra": "iterations: 7407\ncpu: 2815821.5606858376 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_no_interactions/min_time:15.000",
            "value": 698372.6401449622,
            "unit": "ns/iter",
            "extra": "iterations: 30629\ncpu: 698250.6023703032 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_interactions/min_time:15.000",
            "value": 2087725.2996768947,
            "unit": "ns/iter",
            "extra": "iterations: 9904\ncpu: 2087354.038772218 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/simple/min_time:15.000",
            "value": 57097.9565771263,
            "unit": "ns/iter",
            "extra": "iterations: 362666\ncpu: 57090.140239228436 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/quadratic/min_time:15.000",
            "value": 3120681.021100225,
            "unit": "ns/iter",
            "extra": "iterations: 6635\ncpu: 3120227.7618688787 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_string_fts",
            "value": 1127.885469426171,
            "unit": "ns/iter",
            "extra": "iterations: 3764724\ncpu: 1127.7361102699626 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_string_fts",
            "value": 9443.288073506432,
            "unit": "ns/iter",
            "extra": "iterations: 443609\ncpu: 9441.876742807359 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_num_fts",
            "value": 857.8109187857589,
            "unit": "ns/iter",
            "extra": "iterations: 4935329\ncpu: 857.6967817140523 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_num_fts",
            "value": 6765.777131708213,
            "unit": "ns/iter",
            "extra": "iterations: 621192\ncpu: 6764.907307241521 ns\nthreads: 1"
          },
          {
            "name": "benchmark_example_reuse",
            "value": 964.503226088402,
            "unit": "ns/iter",
            "extra": "iterations: 4347215\ncpu: 964.3774462500667 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_char",
            "value": 61.25115751103705,
            "unit": "ns/iter",
            "extra": "iterations: 68354424\ncpu: 61.24312451232128 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_extent",
            "value": 86.38652305489218,
            "unit": "ns/iter",
            "extra": "iterations: 48511528\ncpu: 86.37535597724325 ns\nthreads: 1"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "jackgerrits@users.noreply.github.com",
            "name": "Jack Gerrits",
            "username": "jackgerrits"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "c3607b45c47e2f8be2951aab954d0f2ed2ffe5cb",
          "message": "fix: small build fixes for LAS on MacOS (#4202)",
          "timestamp": "2022-10-07T14:51:31-04:00",
          "tree_id": "df2284fadfbbd4f86db1221eab72807cbd12ac3e",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/c3607b45c47e2f8be2951aab954d0f2ed2ffe5cb"
        },
        "date": 1665171500259,
        "tool": "benchmarkdotnet",
        "benches": [
          {
            "name": "BenchmarkText.Benchmark(args: 120_num_features)",
            "value": 6425.741032191685,
            "unit": "ns",
            "range": "± 43.76757089744049"
          },
          {
            "name": "BenchmarkText.Benchmark(args: 120_string_fts)",
            "value": 9883.498273577008,
            "unit": "ns",
            "range": "± 31.35830251822219"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 1_feature)",
            "value": 665.81755956014,
            "unit": "ns",
            "range": "± 2.9252155795221517"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 8_features)",
            "value": 550.283997853597,
            "unit": "ns",
            "range": "± 2.857363762632159"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_interactions)",
            "value": 882461.4908854166,
            "unit": "ns",
            "range": "± 3708.5329247814466"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_no_interactions)",
            "value": 551075.0906808035,
            "unit": "ns",
            "range": "± 2375.470895907742"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_no_namespaces)",
            "value": 554275.8091517857,
            "unit": "ns",
            "range": "± 2110.8824982875126"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_interactions)",
            "value": 895644.140625,
            "unit": "ns",
            "range": "± 1507.8533840401499"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_no_interactions)",
            "value": 551298.9388020834,
            "unit": "ns",
            "range": "± 1960.1463129059978"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_interactions)",
            "value": 3602414.7916666665,
            "unit": "ns",
            "range": "± 24097.756732794278"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_no_interactions)",
            "value": 1085912.3828125,
            "unit": "ns",
            "range": "± 4755.194485001904"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_no_namespaces)",
            "value": 907546.6517857143,
            "unit": "ns",
            "range": "± 2453.2811554444634"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_interactions)",
            "value": 2822363.28125,
            "unit": "ns",
            "range": "± 10412.31066787296"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_no_interactions)",
            "value": 920501.2565104166,
            "unit": "ns",
            "range": "± 3517.4226593264243"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: few_features)",
            "value": 3302.135442097982,
            "unit": "ns",
            "range": "± 12.643886915966569"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: few_features)",
            "value": 11419.026387532553,
            "unit": "ns",
            "range": "± 44.96512912071879"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: many_features)",
            "value": 109598.35989815848,
            "unit": "ns",
            "range": "± 378.5729324714562"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: many_features)",
            "value": 19233.86404854911,
            "unit": "ns",
            "range": "± 28.571858965610136"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: quadratic)",
            "value": 4271506.927083333,
            "unit": "ns",
            "range": "± 51630.9010566659"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: simple)",
            "value": 219688.79568917412,
            "unit": "ns",
            "range": "± 824.0652578489331"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "griffinbassman@gmail.com",
            "name": "Griffin Bassman",
            "username": "bassmang"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "29b2ee5f503a27f8aa6ca2b23f41a0702caf4338",
          "message": "refactor: replace classes with structs for consistency (#4205)\n\n* refact: replace classes with structs for consistency\r\n\r\n* clang\r\n\r\n* move learner",
          "timestamp": "2022-10-07T17:02:28-04:00",
          "tree_id": "3340e686588da81ac3e64792d02089ba234bf8cf",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/29b2ee5f503a27f8aa6ca2b23f41a0702caf4338"
        },
        "date": 1665177742083,
        "tool": "googlecpp",
        "benches": [
          {
            "name": "bench_text/120_string_fts",
            "value": 5430.613841014362,
            "unit": "ns/iter",
            "extra": "iterations: 786373\ncpu: 5430.306101557403 ns\nthreads: 1"
          },
          {
            "name": "bench_text/120_num_fts",
            "value": 4129.458239677634,
            "unit": "ns/iter",
            "extra": "iterations: 1021472\ncpu: 4127.515487453401 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/8_features",
            "value": 68.90288350556216,
            "unit": "ns/iter",
            "extra": "iterations: 60843094\ncpu: 68.89925749009413 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/1_feature",
            "value": 160.9469617694219,
            "unit": "ns/iter",
            "extra": "iterations: 26080904\ncpu: 160.93956329121104 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features",
            "value": 6679.250554362695,
            "unit": "ns/iter",
            "extra": "iterations: 626846\ncpu: 6678.8080325949295 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features",
            "value": 12351.623036862897,
            "unit": "ns/iter",
            "extra": "iterations: 342946\ncpu: 12350.848238498187 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features_no_predict",
            "value": 3216.0593453001156,
            "unit": "ns/iter",
            "extra": "iterations: 1304720\ncpu: 3215.2662640259955 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features_no_predic",
            "value": 5058.301341428353,
            "unit": "ns/iter",
            "extra": "iterations: 833440\ncpu: 5058.0518093683995 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/few_features",
            "value": 1164.9867861174723,
            "unit": "ns/iter",
            "extra": "iterations: 3557168\ncpu: 1164.9221515542702 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/many_features/min_time:15.000",
            "value": 52238.86040431474,
            "unit": "ns/iter",
            "extra": "iterations: 404289\ncpu: 52231.59719903337 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_no_namespaces/min_time:15.000",
            "value": 190705.44785561512,
            "unit": "ns/iter",
            "extra": "iterations: 111757\ncpu: 190685.5597412242 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 217324.4145805009,
            "unit": "ns/iter",
            "extra": "iterations: 96389\ncpu: 217310.43272572596 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_interactions/min_time:15.000",
            "value": 450730.2198071218,
            "unit": "ns/iter",
            "extra": "iterations: 47387\ncpu: 450663.6630299448 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_no_interactions/min_time:15.000",
            "value": 192632.79011178535,
            "unit": "ns/iter",
            "extra": "iterations: 108958\ncpu: 192621.11455790297 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_interactions/min_time:15.000",
            "value": 330975.1844384517,
            "unit": "ns/iter",
            "extra": "iterations: 63040\ncpu: 330926.0485406092 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_no_namespaces/min_time:15.000",
            "value": 546349.6864559242,
            "unit": "ns/iter",
            "extra": "iterations: 38910\ncpu: 546311.0819840658 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 688292.7605976869,
            "unit": "ns/iter",
            "extra": "iterations: 30384\ncpu: 688181.6976040021 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_interactions/min_time:15.000",
            "value": 2135710.7116279006,
            "unit": "ns/iter",
            "extra": "iterations: 9890\ncpu: 2135538.766430738 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_no_interactions/min_time:15.000",
            "value": 540533.8677499682,
            "unit": "ns/iter",
            "extra": "iterations: 38155\ncpu: 540358.7943912975 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_interactions/min_time:15.000",
            "value": 1610824.4386973341,
            "unit": "ns/iter",
            "extra": "iterations: 13050\ncpu: 1610359.4176245208 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/simple/min_time:15.000",
            "value": 47332.74109929278,
            "unit": "ns/iter",
            "extra": "iterations: 442830\ncpu: 47330.15242869728 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/quadratic/min_time:15.000",
            "value": 2218347.392948974,
            "unit": "ns/iter",
            "extra": "iterations: 9332\ncpu: 2217980.604372053 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_string_fts",
            "value": 891.3353902638595,
            "unit": "ns/iter",
            "extra": "iterations: 4692915\ncpu: 891.2938972898537 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_string_fts",
            "value": 6451.7780312054365,
            "unit": "ns/iter",
            "extra": "iterations: 630467\ncpu: 6451.455825602294 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_num_fts",
            "value": 690.0682762335646,
            "unit": "ns/iter",
            "extra": "iterations: 6090831\ncpu: 690.0348573125776 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_num_fts",
            "value": 4756.754572505893,
            "unit": "ns/iter",
            "extra": "iterations: 885182\ncpu: 4756.466805696442 ns\nthreads: 1"
          },
          {
            "name": "benchmark_example_reuse",
            "value": 732.3133487767902,
            "unit": "ns/iter",
            "extra": "iterations: 5714179\ncpu: 732.2770427737715 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_char",
            "value": 40.55464805909488,
            "unit": "ns/iter",
            "extra": "iterations: 103515433\ncpu: 40.552244997130074 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_extent",
            "value": 55.78864123365642,
            "unit": "ns/iter",
            "extra": "iterations: 78757831\ncpu: 55.78624429105984 ns\nthreads: 1"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "griffinbassman@gmail.com",
            "name": "Griffin Bassman",
            "username": "bassmang"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "29b2ee5f503a27f8aa6ca2b23f41a0702caf4338",
          "message": "refactor: replace classes with structs for consistency (#4205)\n\n* refact: replace classes with structs for consistency\r\n\r\n* clang\r\n\r\n* move learner",
          "timestamp": "2022-10-07T17:02:28-04:00",
          "tree_id": "3340e686588da81ac3e64792d02089ba234bf8cf",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/29b2ee5f503a27f8aa6ca2b23f41a0702caf4338"
        },
        "date": 1665179154352,
        "tool": "benchmarkdotnet",
        "benches": [
          {
            "name": "BenchmarkText.Benchmark(args: 120_num_features)",
            "value": 6009.2270914713545,
            "unit": "ns",
            "range": "± 88.8955181718005"
          },
          {
            "name": "BenchmarkText.Benchmark(args: 120_string_fts)",
            "value": 9532.169233049664,
            "unit": "ns",
            "range": "± 26.78976995085065"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 1_feature)",
            "value": 653.3151944478353,
            "unit": "ns",
            "range": "± 2.2834416157669493"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 8_features)",
            "value": 545.9301471710205,
            "unit": "ns",
            "range": "± 3.7222807029408926"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_interactions)",
            "value": 867436.3932291666,
            "unit": "ns",
            "range": "± 2807.850979121491"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_no_interactions)",
            "value": 527122.2028459822,
            "unit": "ns",
            "range": "± 1752.9935291295658"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_no_namespaces)",
            "value": 555864.3229166666,
            "unit": "ns",
            "range": "± 3167.7168181367224"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_interactions)",
            "value": 885983.28125,
            "unit": "ns",
            "range": "± 5439.802821842378"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_no_interactions)",
            "value": 534726.2109375,
            "unit": "ns",
            "range": "± 4449.868914049623"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_interactions)",
            "value": 3462114.425223214,
            "unit": "ns",
            "range": "± 5539.431020547773"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_no_interactions)",
            "value": 1067415.6770833333,
            "unit": "ns",
            "range": "± 2437.435453312523"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_no_namespaces)",
            "value": 918734.609375,
            "unit": "ns",
            "range": "± 4789.687910320025"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_interactions)",
            "value": 2852174.9441964286,
            "unit": "ns",
            "range": "± 8954.870367311962"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_no_interactions)",
            "value": 920230.56640625,
            "unit": "ns",
            "range": "± 978.4275549812974"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: few_features)",
            "value": 3238.8445172991073,
            "unit": "ns",
            "range": "± 4.656975998616584"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: few_features)",
            "value": 11408.583286830357,
            "unit": "ns",
            "range": "± 15.474393667155045"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: many_features)",
            "value": 107893.02734375,
            "unit": "ns",
            "range": "± 451.4893032455006"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: many_features)",
            "value": 19583.96728515625,
            "unit": "ns",
            "range": "± 34.943379257585036"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: quadratic)",
            "value": 4217610.625,
            "unit": "ns",
            "range": "± 45429.665617893326"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: simple)",
            "value": 210665.576171875,
            "unit": "ns",
            "range": "± 666.3114736840648"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "jackgerrits@users.noreply.github.com",
            "name": "Jack Gerrits",
            "username": "jackgerrits"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "3580418977e6b80a47f8c044a6847585f096957c",
          "message": "refactor: move ccb items into VW namespace (#4204)\n\n* refactor: move ccb items into VW namespace\r\n\r\n* dont use deprecated types\r\n\r\n* formatting",
          "timestamp": "2022-10-07T21:31:20-04:00",
          "tree_id": "c907dc3c8c118c7da837ad664429840be2020c71",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/3580418977e6b80a47f8c044a6847585f096957c"
        },
        "date": 1665194002458,
        "tool": "googlecpp",
        "benches": [
          {
            "name": "bench_text/120_string_fts",
            "value": 6982.00263247723,
            "unit": "ns/iter",
            "extra": "iterations: 597536\ncpu: 6981.475425748407 ns\nthreads: 1"
          },
          {
            "name": "bench_text/120_num_fts",
            "value": 5189.691604475323,
            "unit": "ns/iter",
            "extra": "iterations: 811873\ncpu: 5189.374323323968 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/8_features",
            "value": 89.14713478837815,
            "unit": "ns/iter",
            "extra": "iterations: 46128146\ncpu: 89.12621374377369 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/1_feature",
            "value": 192.84139972395423,
            "unit": "ns/iter",
            "extra": "iterations: 21572825\ncpu: 192.82859338079277 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features",
            "value": 7781.775605271935,
            "unit": "ns/iter",
            "extra": "iterations: 529803\ncpu: 7781.075984847196 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features",
            "value": 13697.279516236504,
            "unit": "ns/iter",
            "extra": "iterations: 303206\ncpu: 13688.63215107881 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features_no_predict",
            "value": 3749.647877651979,
            "unit": "ns/iter",
            "extra": "iterations: 1108183\ncpu: 3749.4355174190546 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features_no_predic",
            "value": 5615.15048045932,
            "unit": "ns/iter",
            "extra": "iterations: 724619\ncpu: 5614.799225524038 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/few_features",
            "value": 1374.7747900646898,
            "unit": "ns/iter",
            "extra": "iterations: 3027957\ncpu: 1374.692441141007 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/many_features/min_time:15.000",
            "value": 69031.26920286105,
            "unit": "ns/iter",
            "extra": "iterations: 308704\ncpu: 69025.63717995233 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_no_namespaces/min_time:15.000",
            "value": 280347.7644940169,
            "unit": "ns/iter",
            "extra": "iterations: 77463\ncpu: 280272.10668319074 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 272634.4869427602,
            "unit": "ns/iter",
            "extra": "iterations: 72029\ncpu: 272613.27104360756 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_interactions/min_time:15.000",
            "value": 551353.7171952162,
            "unit": "ns/iter",
            "extra": "iterations: 38691\ncpu: 551242.7928975726 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_no_interactions/min_time:15.000",
            "value": 239780.94897281355,
            "unit": "ns/iter",
            "extra": "iterations: 87326\ncpu: 239724.63756498645 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_interactions/min_time:15.000",
            "value": 408109.08001627197,
            "unit": "ns/iter",
            "extra": "iterations: 51627\ncpu: 408070.2365041546 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_no_namespaces/min_time:15.000",
            "value": 644135.9097073508,
            "unit": "ns/iter",
            "extra": "iterations: 33624\ncpu: 644048.6200333094 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 849472.1552524128,
            "unit": "ns/iter",
            "extra": "iterations: 25217\ncpu: 849372.6414720233 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_interactions/min_time:15.000",
            "value": 2826241.0343341874,
            "unit": "ns/iter",
            "extra": "iterations: 7427\ncpu: 2825965.2349535436 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_no_interactions/min_time:15.000",
            "value": 670185.411023146,
            "unit": "ns/iter",
            "extra": "iterations: 30064\ncpu: 670125.5421766905 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_interactions/min_time:15.000",
            "value": 1960293.8874874315,
            "unit": "ns/iter",
            "extra": "iterations: 10941\ncpu: 1960116.652956767 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/simple/min_time:15.000",
            "value": 57889.18467032109,
            "unit": "ns/iter",
            "extra": "iterations: 363778\ncpu: 57885.10575130998 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/quadratic/min_time:15.000",
            "value": 3049455.3256772496,
            "unit": "ns/iter",
            "extra": "iterations: 6792\ncpu: 3049218.1537102438 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_string_fts",
            "value": 1098.8922060513892,
            "unit": "ns/iter",
            "extra": "iterations: 3903939\ncpu: 1098.826416088975 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_string_fts",
            "value": 8129.687938740309,
            "unit": "ns/iter",
            "extra": "iterations: 509698\ncpu: 8129.186891061048 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_num_fts",
            "value": 826.5607645606991,
            "unit": "ns/iter",
            "extra": "iterations: 5016057\ncpu: 826.5157074570783 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_num_fts",
            "value": 5897.515922138234,
            "unit": "ns/iter",
            "extra": "iterations: 722359\ncpu: 5897.180487818361 ns\nthreads: 1"
          },
          {
            "name": "benchmark_example_reuse",
            "value": 955.9250973677388,
            "unit": "ns/iter",
            "extra": "iterations: 4401354\ncpu: 955.8671036231127 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_char",
            "value": 61.12306222668844,
            "unit": "ns/iter",
            "extra": "iterations: 68541170\ncpu: 61.1193754060514 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_extent",
            "value": 97.4973351946395,
            "unit": "ns/iter",
            "extra": "iterations: 42456947\ncpu: 97.49195579229945 ns\nthreads: 1"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "jackgerrits@users.noreply.github.com",
            "name": "Jack Gerrits",
            "username": "jackgerrits"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "3580418977e6b80a47f8c044a6847585f096957c",
          "message": "refactor: move ccb items into VW namespace (#4204)\n\n* refactor: move ccb items into VW namespace\r\n\r\n* dont use deprecated types\r\n\r\n* formatting",
          "timestamp": "2022-10-07T21:31:20-04:00",
          "tree_id": "c907dc3c8c118c7da837ad664429840be2020c71",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/3580418977e6b80a47f8c044a6847585f096957c"
        },
        "date": 1665195501520,
        "tool": "benchmarkdotnet",
        "benches": [
          {
            "name": "BenchmarkText.Benchmark(args: 120_num_features)",
            "value": 6649.529088338216,
            "unit": "ns",
            "range": "± 122.19652779515431"
          },
          {
            "name": "BenchmarkText.Benchmark(args: 120_string_fts)",
            "value": 10871.324586868286,
            "unit": "ns",
            "range": "± 332.05394816045794"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 1_feature)",
            "value": 737.3435709211561,
            "unit": "ns",
            "range": "± 15.46225403433123"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 8_features)",
            "value": 628.0942409269271,
            "unit": "ns",
            "range": "± 18.1366878053704"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_interactions)",
            "value": 969280.7421875,
            "unit": "ns",
            "range": "± 12046.415358347887"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_no_interactions)",
            "value": 605839.2708333334,
            "unit": "ns",
            "range": "± 9057.148876324281"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_no_namespaces)",
            "value": 614181.0128348215,
            "unit": "ns",
            "range": "± 9149.629543522227"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_interactions)",
            "value": 968305.9993489584,
            "unit": "ns",
            "range": "± 13364.518361181346"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_no_interactions)",
            "value": 604500.1220703125,
            "unit": "ns",
            "range": "± 13760.757586067048"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_interactions)",
            "value": 3986693.5677083335,
            "unit": "ns",
            "range": "± 108949.4872464326"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_no_interactions)",
            "value": 1220422.40234375,
            "unit": "ns",
            "range": "± 23268.15416881897"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_no_namespaces)",
            "value": 985069.9908088235,
            "unit": "ns",
            "range": "± 19128.881217585404"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_interactions)",
            "value": 3135711.2413194445,
            "unit": "ns",
            "range": "± 54827.27386163265"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_no_interactions)",
            "value": 984962.8743489584,
            "unit": "ns",
            "range": "± 8568.437268070626"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: few_features)",
            "value": 3796.610019825123,
            "unit": "ns",
            "range": "± 106.17122859806827"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: few_features)",
            "value": 12699.305953979492,
            "unit": "ns",
            "range": "± 286.10940230484306"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: many_features)",
            "value": 126310.02854567308,
            "unit": "ns",
            "range": "± 2071.626597952167"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: many_features)",
            "value": 21590.103149414062,
            "unit": "ns",
            "range": "± 390.72078012018795"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: quadratic)",
            "value": 4860223.177083333,
            "unit": "ns",
            "range": "± 143239.17707912636"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: simple)",
            "value": 241236.24624857088,
            "unit": "ns",
            "range": "± 8530.431944577149"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "jackgerrits@users.noreply.github.com",
            "name": "Jack Gerrits",
            "username": "jackgerrits"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "39ce23f10144b15485eb1856eb08b973b8d0199f",
          "message": "style: rename label_data to VW::simple_label (#4200)\n\n* style: rename label_data to VW::simple_label\r\n\r\n* add operator !=\r\n\r\n* move << into VW namespace in test",
          "timestamp": "2022-10-07T23:46:27-04:00",
          "tree_id": "f30d1ea68bb6dcf584cd5cd42442c5aa89defd73",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/39ce23f10144b15485eb1856eb08b973b8d0199f"
        },
        "date": 1665202160482,
        "tool": "googlecpp",
        "benches": [
          {
            "name": "bench_text/120_string_fts",
            "value": 6409.864613907012,
            "unit": "ns/iter",
            "extra": "iterations: 668466\ncpu: 6409.389407987842 ns\nthreads: 1"
          },
          {
            "name": "bench_text/120_num_fts",
            "value": 4751.8626971183285,
            "unit": "ns/iter",
            "extra": "iterations: 901933\ncpu: 4746.557116770314 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/8_features",
            "value": 85.69409434347659,
            "unit": "ns/iter",
            "extra": "iterations: 49408207\ncpu: 85.6877724787706 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/1_feature",
            "value": 181.03778630235254,
            "unit": "ns/iter",
            "extra": "iterations: 22637039\ncpu: 181.02500949881295 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features",
            "value": 7519.082452638462,
            "unit": "ns/iter",
            "extra": "iterations: 561110\ncpu: 7513.685195416229 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features",
            "value": 13508.992655476026,
            "unit": "ns/iter",
            "extra": "iterations: 306623\ncpu: 13507.833071883055 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features_no_predict",
            "value": 3702.697524243341,
            "unit": "ns/iter",
            "extra": "iterations: 1175681\ncpu: 3702.447347537299 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features_no_predic",
            "value": 6111.724442248371,
            "unit": "ns/iter",
            "extra": "iterations: 621289\ncpu: 6111.157287510322 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/few_features",
            "value": 1374.2959295377048,
            "unit": "ns/iter",
            "extra": "iterations: 2946938\ncpu: 1374.1913131528381 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/many_features/min_time:15.000",
            "value": 85493.14634571546,
            "unit": "ns/iter",
            "extra": "iterations: 258108\ncpu: 85485.48708292654 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_no_namespaces/min_time:15.000",
            "value": 327187.5047930116,
            "unit": "ns/iter",
            "extra": "iterations: 65825\ncpu: 327104.8568173187 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 354554.14716054406,
            "unit": "ns/iter",
            "extra": "iterations: 61878\ncpu: 354521.2159410453 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_interactions/min_time:15.000",
            "value": 626311.7842754469,
            "unit": "ns/iter",
            "extra": "iterations: 33807\ncpu: 626164.7646937025 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_no_interactions/min_time:15.000",
            "value": 327413.0939910905,
            "unit": "ns/iter",
            "extra": "iterations: 64421\ncpu: 327376.336908772 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_interactions/min_time:15.000",
            "value": 516126.19304878206,
            "unit": "ns/iter",
            "extra": "iterations: 41000\ncpu: 516005.8756097561 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_no_namespaces/min_time:15.000",
            "value": 719381.8304844789,
            "unit": "ns/iter",
            "extra": "iterations: 28959\ncpu: 719194.5371041818 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 883954.5905575387,
            "unit": "ns/iter",
            "extra": "iterations: 23532\ncpu: 883557.0329763737 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_interactions/min_time:15.000",
            "value": 3009235.6863708626,
            "unit": "ns/iter",
            "extra": "iterations: 6919\ncpu: 3008962.4656742304 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_no_interactions/min_time:15.000",
            "value": 732940.7290616909,
            "unit": "ns/iter",
            "extra": "iterations: 29372\ncpu: 732870.6352989239 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_interactions/min_time:15.000",
            "value": 2217237.218606634,
            "unit": "ns/iter",
            "extra": "iterations: 9373\ncpu: 2217039.613784273 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/simple/min_time:15.000",
            "value": 81805.42436281967,
            "unit": "ns/iter",
            "extra": "iterations: 260758\ncpu: 81798.88632371786 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/quadratic/min_time:15.000",
            "value": 2852119.361038273,
            "unit": "ns/iter",
            "extra": "iterations: 7243\ncpu: 2851525.0448709135 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_string_fts",
            "value": 1152.9428558723112,
            "unit": "ns/iter",
            "extra": "iterations: 3665451\ncpu: 1152.8541235444177 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_string_fts",
            "value": 8236.122579588156,
            "unit": "ns/iter",
            "extra": "iterations: 512826\ncpu: 8235.519649939786 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_num_fts",
            "value": 898.690900912418,
            "unit": "ns/iter",
            "extra": "iterations: 4844178\ncpu: 898.6140476258269 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_num_fts",
            "value": 5966.012850588034,
            "unit": "ns/iter",
            "extra": "iterations: 715298\ncpu: 5955.181896216733 ns\nthreads: 1"
          },
          {
            "name": "benchmark_example_reuse",
            "value": 996.1019420358737,
            "unit": "ns/iter",
            "extra": "iterations: 4276646\ncpu: 996.0341351610507 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_char",
            "value": 55.73937953315661,
            "unit": "ns/iter",
            "extra": "iterations: 77401235\ncpu: 55.73487684014334 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_extent",
            "value": 77.78734517909541,
            "unit": "ns/iter",
            "extra": "iterations: 54748258\ncpu: 77.78131132501107 ns\nthreads: 1"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "jackgerrits@users.noreply.github.com",
            "name": "Jack Gerrits",
            "username": "jackgerrits"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "39ce23f10144b15485eb1856eb08b973b8d0199f",
          "message": "style: rename label_data to VW::simple_label (#4200)\n\n* style: rename label_data to VW::simple_label\r\n\r\n* add operator !=\r\n\r\n* move << into VW namespace in test",
          "timestamp": "2022-10-07T23:46:27-04:00",
          "tree_id": "f30d1ea68bb6dcf584cd5cd42442c5aa89defd73",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/39ce23f10144b15485eb1856eb08b973b8d0199f"
        },
        "date": 1665203507992,
        "tool": "benchmarkdotnet",
        "benches": [
          {
            "name": "BenchmarkText.Benchmark(args: 120_num_features)",
            "value": 5828.251139322917,
            "unit": "ns",
            "range": "± 75.00193532136313"
          },
          {
            "name": "BenchmarkText.Benchmark(args: 120_string_fts)",
            "value": 9381.337629045758,
            "unit": "ns",
            "range": "± 55.70021635010238"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 1_feature)",
            "value": 639.4078254699707,
            "unit": "ns",
            "range": "± 2.774787716507572"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 8_features)",
            "value": 558.7504577636719,
            "unit": "ns",
            "range": "± 2.6380273132307392"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_interactions)",
            "value": 874715.2213541666,
            "unit": "ns",
            "range": "± 956.9424554706303"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_no_interactions)",
            "value": 525628.4440104166,
            "unit": "ns",
            "range": "± 1343.643106900261"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_no_namespaces)",
            "value": 554821.9921875,
            "unit": "ns",
            "range": "± 800.1475565932362"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_interactions)",
            "value": 875948.8509114584,
            "unit": "ns",
            "range": "± 1388.660309772851"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_no_interactions)",
            "value": 529592.1805245535,
            "unit": "ns",
            "range": "± 1423.9423801162393"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_interactions)",
            "value": 3555313.7239583335,
            "unit": "ns",
            "range": "± 8802.625117029065"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_no_interactions)",
            "value": 1076727.1614583333,
            "unit": "ns",
            "range": "± 2316.2248747831454"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_no_namespaces)",
            "value": 893514.1015625,
            "unit": "ns",
            "range": "± 1894.0299952743408"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_interactions)",
            "value": 2775624.8197115385,
            "unit": "ns",
            "range": "± 4302.087870175244"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_no_interactions)",
            "value": 921796.6861979166,
            "unit": "ns",
            "range": "± 1597.0489008268423"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: few_features)",
            "value": 3258.126585824149,
            "unit": "ns",
            "range": "± 6.784726190174566"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: few_features)",
            "value": 11438.004557291666,
            "unit": "ns",
            "range": "± 25.186438371084993"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: many_features)",
            "value": 108137.45361328125,
            "unit": "ns",
            "range": "± 226.18390138635252"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: many_features)",
            "value": 19239.047241210938,
            "unit": "ns",
            "range": "± 35.283048749410035"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: quadratic)",
            "value": 4248010.611979167,
            "unit": "ns",
            "range": "± 3833.2308606510105"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: simple)",
            "value": 207377.42396763392,
            "unit": "ns",
            "range": "± 2094.1287384987313"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "byronxu@microsoft.com",
            "name": "Byron Xu",
            "username": "byronxu99"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "d4074a6244ba8007c0be2c9d6a1fbcee9dc6ae42",
          "message": "ci: Fix randomly failing .NET benchmarks (#4209)",
          "timestamp": "2022-10-10T11:55:54-04:00",
          "tree_id": "2cf3ef0fbe20e07a17f2dcfc684a0b1a15f9bc06",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/d4074a6244ba8007c0be2c9d6a1fbcee9dc6ae42"
        },
        "date": 1665418545223,
        "tool": "googlecpp",
        "benches": [
          {
            "name": "bench_text/120_string_fts",
            "value": 5332.748312062401,
            "unit": "ns/iter",
            "extra": "iterations: 789573\ncpu: 5332.458430062831 ns\nthreads: 1"
          },
          {
            "name": "bench_text/120_num_fts",
            "value": 4020.8970229291635,
            "unit": "ns/iter",
            "extra": "iterations: 1048010\ncpu: 4018.6514441656086 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/8_features",
            "value": 73.84569968423114,
            "unit": "ns/iter",
            "extra": "iterations: 56824362\ncpu: 73.84144673722864 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/1_feature",
            "value": 162.3314707331666,
            "unit": "ns/iter",
            "extra": "iterations: 26030099\ncpu: 162.26109243764307 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features",
            "value": 6650.570310339147,
            "unit": "ns/iter",
            "extra": "iterations: 629054\ncpu: 6650.302199811145 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features",
            "value": 12189.897630072417,
            "unit": "ns/iter",
            "extra": "iterations: 343597\ncpu: 12184.207952921593 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features_no_predict",
            "value": 3298.0651227820504,
            "unit": "ns/iter",
            "extra": "iterations: 1281702\ncpu: 3297.8881986608426 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features_no_predic",
            "value": 5027.184893376845,
            "unit": "ns/iter",
            "extra": "iterations: 839592\ncpu: 5027.014192607838 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/few_features",
            "value": 1195.311428721282,
            "unit": "ns/iter",
            "extra": "iterations: 3488763\ncpu: 1195.251956065802 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/many_features/min_time:15.000",
            "value": 51887.1322584249,
            "unit": "ns/iter",
            "extra": "iterations: 402825\ncpu: 51884.71097871284 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_no_namespaces/min_time:15.000",
            "value": 187865.65109520743,
            "unit": "ns/iter",
            "extra": "iterations: 115275\ncpu: 187838.5790500976 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 215259.67681496558,
            "unit": "ns/iter",
            "extra": "iterations: 98046\ncpu: 215248.18044591314 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_interactions/min_time:15.000",
            "value": 449566.0085789074,
            "unit": "ns/iter",
            "extra": "iterations: 46626\ncpu: 449540.26294342196 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_no_interactions/min_time:15.000",
            "value": 186201.13046680315,
            "unit": "ns/iter",
            "extra": "iterations: 112703\ncpu: 186174.25179453974 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_interactions/min_time:15.000",
            "value": 329629.4050601066,
            "unit": "ns/iter",
            "extra": "iterations: 63635\ncpu: 329612.02325764124 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_no_namespaces/min_time:15.000",
            "value": 520886.06548695255,
            "unit": "ns/iter",
            "extra": "iterations: 39275\ncpu: 520606.46976448124 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 658650.1656508858,
            "unit": "ns/iter",
            "extra": "iterations: 31995\ncpu: 658576.0962650413 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_interactions/min_time:15.000",
            "value": 2081696.8188052787,
            "unit": "ns/iter",
            "extra": "iterations: 10061\ncpu: 2081320.584434945 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_no_interactions/min_time:15.000",
            "value": 534376.4436681238,
            "unit": "ns/iter",
            "extra": "iterations: 40075\ncpu: 534198.0311915163 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_interactions/min_time:15.000",
            "value": 1610873.542767696,
            "unit": "ns/iter",
            "extra": "iterations: 13094\ncpu: 1610783.9086604563 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/simple/min_time:15.000",
            "value": 47716.95139193839,
            "unit": "ns/iter",
            "extra": "iterations: 439495\ncpu: 47714.76581076004 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/quadratic/min_time:15.000",
            "value": 2197915.7873915737,
            "unit": "ns/iter",
            "extra": "iterations: 9454\ncpu: 2197732.864396026 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_string_fts",
            "value": 894.951717978303,
            "unit": "ns/iter",
            "extra": "iterations: 4694646\ncpu: 894.9099037499376 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_string_fts",
            "value": 6362.972990916978,
            "unit": "ns/iter",
            "extra": "iterations: 656705\ncpu: 6362.676848813386 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_num_fts",
            "value": 695.1057873895613,
            "unit": "ns/iter",
            "extra": "iterations: 6110095\ncpu: 694.8017174855669 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_num_fts",
            "value": 4617.881467602998,
            "unit": "ns/iter",
            "extra": "iterations: 906267\ncpu: 4617.667751335959 ns\nthreads: 1"
          },
          {
            "name": "benchmark_example_reuse",
            "value": 761.394508542562,
            "unit": "ns/iter",
            "extra": "iterations: 5455783\ncpu: 761.3657104763898 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_char",
            "value": 42.03154559241884,
            "unit": "ns/iter",
            "extra": "iterations: 99973174\ncpu: 42.02956185026217 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_extent",
            "value": 56.9356729450626,
            "unit": "ns/iter",
            "extra": "iterations: 73472134\ncpu: 56.93326669945362 ns\nthreads: 1"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "byronxu@microsoft.com",
            "name": "Byron Xu",
            "username": "byronxu99"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "d4074a6244ba8007c0be2c9d6a1fbcee9dc6ae42",
          "message": "ci: Fix randomly failing .NET benchmarks (#4209)",
          "timestamp": "2022-10-10T11:55:54-04:00",
          "tree_id": "2cf3ef0fbe20e07a17f2dcfc684a0b1a15f9bc06",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/d4074a6244ba8007c0be2c9d6a1fbcee9dc6ae42"
        },
        "date": 1665420014413,
        "tool": "benchmarkdotnet",
        "benches": [
          {
            "name": "BenchmarkText.Benchmark(args: 120_num_features)",
            "value": 6003.341850867639,
            "unit": "ns",
            "range": "± 48.0992619011055"
          },
          {
            "name": "BenchmarkText.Benchmark(args: 120_string_fts)",
            "value": 9731.231994628906,
            "unit": "ns",
            "range": "± 131.9999143305759"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 1_feature)",
            "value": 657.7292169843402,
            "unit": "ns",
            "range": "± 4.882643141988658"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 8_features)",
            "value": 601.1587651570638,
            "unit": "ns",
            "range": "± 3.897614614364102"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_interactions)",
            "value": 899502.7734375,
            "unit": "ns",
            "range": "± 9173.354027490836"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_no_interactions)",
            "value": 563538.0403645834,
            "unit": "ns",
            "range": "± 6073.503093300551"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_no_namespaces)",
            "value": 560190.0390625,
            "unit": "ns",
            "range": "± 5679.900398004736"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_interactions)",
            "value": 876637.451171875,
            "unit": "ns",
            "range": "± 6217.71197358232"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_no_interactions)",
            "value": 551388.4728064904,
            "unit": "ns",
            "range": "± 786.702574421644"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_interactions)",
            "value": 3535726.8229166665,
            "unit": "ns",
            "range": "± 10785.58926031382"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_no_interactions)",
            "value": 1062200.15625,
            "unit": "ns",
            "range": "± 6427.374417981117"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_no_namespaces)",
            "value": 913188.1184895834,
            "unit": "ns",
            "range": "± 4040.0311624942906"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_interactions)",
            "value": 2804037.3177083335,
            "unit": "ns",
            "range": "± 15304.574630427558"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_no_interactions)",
            "value": 926909.8563058035,
            "unit": "ns",
            "range": "± 2580.3247125816306"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: few_features)",
            "value": 3284.1067250569663,
            "unit": "ns",
            "range": "± 16.519531095375516"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: few_features)",
            "value": 11421.812540690104,
            "unit": "ns",
            "range": "± 35.26999268491418"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: many_features)",
            "value": 108044.140625,
            "unit": "ns",
            "range": "± 215.0414685202878"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: many_features)",
            "value": 19293.592122395832,
            "unit": "ns",
            "range": "± 76.20804355454648"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: quadratic)",
            "value": 4198932.589285715,
            "unit": "ns",
            "range": "± 23384.644661019633"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: simple)",
            "value": 223119.17643229166,
            "unit": "ns",
            "range": "± 1020.7853100067607"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "jackgerrits@users.noreply.github.com",
            "name": "Jack Gerrits",
            "username": "jackgerrits"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "dc6e9c40e3cfc9bb92ec94a407b8d2ebd5349014",
          "message": "refactor: move several labels into VW namespace, style updates (#4206)\n\n* move no label to VW namespace\r\n\r\n* move multiclass label\r\n\r\n* rename cs label and move to vw namespace\r\n\r\n* change wclass to cs_class\r\n\r\n* move cb label in VW namespace\r\n\r\n* cut out cb for now\r\n\r\n* formatting and deprecated usage\r\n\r\n* small fixes\r\n\r\n* fixes\r\n\r\n* remove unused function\r\n\r\n* formatting",
          "timestamp": "2022-10-10T15:53:51-04:00",
          "tree_id": "97690a55d1e7eb3c930c5ee1b0925687a6e4800b",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/dc6e9c40e3cfc9bb92ec94a407b8d2ebd5349014"
        },
        "date": 1665432994239,
        "tool": "googlecpp",
        "benches": [
          {
            "name": "bench_text/120_string_fts",
            "value": 7175.755469421087,
            "unit": "ns/iter",
            "extra": "iterations: 581003\ncpu: 7169.270210308726 ns\nthreads: 1"
          },
          {
            "name": "bench_text/120_num_fts",
            "value": 5423.781691274134,
            "unit": "ns/iter",
            "extra": "iterations: 770496\ncpu: 5417.861221862279 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/8_features",
            "value": 89.44418947472165,
            "unit": "ns/iter",
            "extra": "iterations: 46929320\ncpu: 89.38590842569207 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/1_feature",
            "value": 201.23105407473992,
            "unit": "ns/iter",
            "extra": "iterations: 20858113\ncpu: 201.20644182913384 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features",
            "value": 8195.58866273354,
            "unit": "ns/iter",
            "extra": "iterations: 509188\ncpu: 8194.118086050727 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features",
            "value": 14314.242006554523,
            "unit": "ns/iter",
            "extra": "iterations: 290488\ncpu: 14311.863484894393 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features_no_predict",
            "value": 3902.532347149139,
            "unit": "ns/iter",
            "extra": "iterations: 1081069\ncpu: 3902.0378902734283 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features_no_predic",
            "value": 5858.890931944132,
            "unit": "ns/iter",
            "extra": "iterations: 709667\ncpu: 5858.216741091251 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/few_features",
            "value": 1390.2803274789949,
            "unit": "ns/iter",
            "extra": "iterations: 3027980\ncpu: 1390.1091486733724 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/many_features/min_time:15.000",
            "value": 68303.84744863275,
            "unit": "ns/iter",
            "extra": "iterations: 309050\ncpu: 68294.66202879792 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_no_namespaces/min_time:15.000",
            "value": 278298.78090530785,
            "unit": "ns/iter",
            "extra": "iterations: 76880\ncpu: 278257.51951092604 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 312839.40469767334,
            "unit": "ns/iter",
            "extra": "iterations: 67097\ncpu: 312794.94463239785 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_interactions/min_time:15.000",
            "value": 615116.7769931981,
            "unit": "ns/iter",
            "extra": "iterations: 34555\ncpu: 615027.2319490665 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_no_interactions/min_time:15.000",
            "value": 277075.6329597888,
            "unit": "ns/iter",
            "extra": "iterations: 75850\ncpu: 277036.3836519447 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_interactions/min_time:15.000",
            "value": 461550.6835270574,
            "unit": "ns/iter",
            "extra": "iterations: 45511\ncpu: 461488.22043022583 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_no_namespaces/min_time:15.000",
            "value": 683680.5343364457,
            "unit": "ns/iter",
            "extra": "iterations: 30638\ncpu: 683571.6495854822 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 896104.0096480737,
            "unit": "ns/iter",
            "extra": "iterations: 23528\ncpu: 895970.248214894 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_interactions/min_time:15.000",
            "value": 2911168.3231138336,
            "unit": "ns/iter",
            "extra": "iterations: 7502\ncpu: 2910721.1676886147 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_no_interactions/min_time:15.000",
            "value": 718542.5356596742,
            "unit": "ns/iter",
            "extra": "iterations: 29431\ncpu: 718437.4129319433 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_interactions/min_time:15.000",
            "value": 2074651.1750590121,
            "unit": "ns/iter",
            "extra": "iterations: 10168\ncpu: 2074350.8851298187 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/simple/min_time:15.000",
            "value": 61879.969506626105,
            "unit": "ns/iter",
            "extra": "iterations: 340533\ncpu: 61871.86322617775 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/quadratic/min_time:15.000",
            "value": 3147799.7938284427,
            "unit": "ns/iter",
            "extra": "iterations: 6611\ncpu: 3147367.4935713233 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_string_fts",
            "value": 1131.0510731780855,
            "unit": "ns/iter",
            "extra": "iterations: 3562829\ncpu: 1130.915572989885 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_string_fts",
            "value": 8434.179809660087,
            "unit": "ns/iter",
            "extra": "iterations: 503310\ncpu: 8433.183723748763 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_num_fts",
            "value": 849.7611193222593,
            "unit": "ns/iter",
            "extra": "iterations: 4907291\ncpu: 849.6685238352502 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_num_fts",
            "value": 6172.256605245383,
            "unit": "ns/iter",
            "extra": "iterations: 676962\ncpu: 6171.562953312019 ns\nthreads: 1"
          },
          {
            "name": "benchmark_example_reuse",
            "value": 972.7913844449378,
            "unit": "ns/iter",
            "extra": "iterations: 4335391\ncpu: 972.6796498862568 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_char",
            "value": 63.25409155226251,
            "unit": "ns/iter",
            "extra": "iterations: 66151483\ncpu: 63.247062805832634 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_extent",
            "value": 88.8259030534284,
            "unit": "ns/iter",
            "extra": "iterations: 47263787\ncpu: 88.816423025941 ns\nthreads: 1"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "jackgerrits@users.noreply.github.com",
            "name": "Jack Gerrits",
            "username": "jackgerrits"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "dc6e9c40e3cfc9bb92ec94a407b8d2ebd5349014",
          "message": "refactor: move several labels into VW namespace, style updates (#4206)\n\n* move no label to VW namespace\r\n\r\n* move multiclass label\r\n\r\n* rename cs label and move to vw namespace\r\n\r\n* change wclass to cs_class\r\n\r\n* move cb label in VW namespace\r\n\r\n* cut out cb for now\r\n\r\n* formatting and deprecated usage\r\n\r\n* small fixes\r\n\r\n* fixes\r\n\r\n* remove unused function\r\n\r\n* formatting",
          "timestamp": "2022-10-10T15:53:51-04:00",
          "tree_id": "97690a55d1e7eb3c930c5ee1b0925687a6e4800b",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/dc6e9c40e3cfc9bb92ec94a407b8d2ebd5349014"
        },
        "date": 1665434105462,
        "tool": "benchmarkdotnet",
        "benches": [
          {
            "name": "BenchmarkText.Benchmark(args: 120_num_features)",
            "value": 5752.742055257161,
            "unit": "ns",
            "range": "± 58.694148393801406"
          },
          {
            "name": "BenchmarkText.Benchmark(args: 120_string_fts)",
            "value": 9327.713826497396,
            "unit": "ns",
            "range": "± 51.583864628778784"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 1_feature)",
            "value": 636.1480304173061,
            "unit": "ns",
            "range": "± 2.1577572800434672"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 8_features)",
            "value": 518.3318546840122,
            "unit": "ns",
            "range": "± 2.5242542287607828"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_interactions)",
            "value": 871603.5853794643,
            "unit": "ns",
            "range": "± 2755.537744229051"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_no_interactions)",
            "value": 550192.4609375,
            "unit": "ns",
            "range": "± 4479.330222096427"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_no_namespaces)",
            "value": 552780.5363581731,
            "unit": "ns",
            "range": "± 1942.2960698108554"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_interactions)",
            "value": 875771.6029575893,
            "unit": "ns",
            "range": "± 2455.33760588778"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_no_interactions)",
            "value": 552297.421875,
            "unit": "ns",
            "range": "± 2647.9537211569523"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_interactions)",
            "value": 3432334.933035714,
            "unit": "ns",
            "range": "± 7586.292209818813"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_no_interactions)",
            "value": 1073238.5463169643,
            "unit": "ns",
            "range": "± 3914.5998968096455"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_no_namespaces)",
            "value": 906424.7977120535,
            "unit": "ns",
            "range": "± 1804.6269070943238"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_interactions)",
            "value": 2798125.1953125,
            "unit": "ns",
            "range": "± 9229.588278553305"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_no_interactions)",
            "value": 925755.8244977678,
            "unit": "ns",
            "range": "± 1927.8516314690594"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: few_features)",
            "value": 3239.4383748372397,
            "unit": "ns",
            "range": "± 12.50857829320766"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: few_features)",
            "value": 11459.64365641276,
            "unit": "ns",
            "range": "± 32.29688054327406"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: many_features)",
            "value": 109215.791015625,
            "unit": "ns",
            "range": "± 249.6747834906047"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: many_features)",
            "value": 19118.641967773438,
            "unit": "ns",
            "range": "± 51.58933964073389"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: quadratic)",
            "value": 4179821.40625,
            "unit": "ns",
            "range": "± 21381.59372260596"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: simple)",
            "value": 203001.36067708334,
            "unit": "ns",
            "range": "± 721.7315998187419"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "jackgerrits@users.noreply.github.com",
            "name": "Jack Gerrits",
            "username": "jackgerrits"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "9ab3f6974200a57f2550385c48cf5965393810ed",
          "message": "style: apply more style fixes per clang-tidy (#4208)\n\n* style: apply more style fixes per clang-tidy\r\n\r\n* win fix\r\n\r\n* fixes\r\n\r\n* ignore unused function",
          "timestamp": "2022-10-10T17:12:56-04:00",
          "tree_id": "9a08a328d2940f957b29528e5ee09d0761a081d3",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/9ab3f6974200a57f2550385c48cf5965393810ed"
        },
        "date": 1665437867351,
        "tool": "googlecpp",
        "benches": [
          {
            "name": "bench_text/120_string_fts",
            "value": 7339.918220302666,
            "unit": "ns/iter",
            "extra": "iterations: 574727\ncpu: 7327.721161525384 ns\nthreads: 1"
          },
          {
            "name": "bench_text/120_num_fts",
            "value": 5142.142235814107,
            "unit": "ns/iter",
            "extra": "iterations: 819442\ncpu: 5141.633208939742 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/8_features",
            "value": 92.56530994361607,
            "unit": "ns/iter",
            "extra": "iterations: 44676497\ncpu: 92.55668366300071 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/1_feature",
            "value": 218.4132351316083,
            "unit": "ns/iter",
            "extra": "iterations: 19338697\ncpu: 218.39446059887075 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features",
            "value": 8555.087460979654,
            "unit": "ns/iter",
            "extra": "iterations: 497822\ncpu: 8554.04281048246 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features",
            "value": 15640.456512624834,
            "unit": "ns/iter",
            "extra": "iterations: 271918\ncpu: 15638.858405842937 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features_no_predict",
            "value": 4056.3224301274336,
            "unit": "ns/iter",
            "extra": "iterations: 1036851\ncpu: 4055.9149771760835 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features_no_predic",
            "value": 6168.112330429536,
            "unit": "ns/iter",
            "extra": "iterations: 667495\ncpu: 6167.34672169829 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/few_features",
            "value": 1489.1902277428937,
            "unit": "ns/iter",
            "extra": "iterations: 2824588\ncpu: 1489.0211953035273 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/many_features/min_time:15.000",
            "value": 88447.25514206951,
            "unit": "ns/iter",
            "extra": "iterations: 236821\ncpu: 88438.93278045446 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_no_namespaces/min_time:15.000",
            "value": 357395.6396017104,
            "unit": "ns/iter",
            "extra": "iterations: 59856\ncpu: 357359.6180834002 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 384770.3060175483,
            "unit": "ns/iter",
            "extra": "iterations: 54474\ncpu: 384738.4036421043 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_interactions/min_time:15.000",
            "value": 671044.8169009539,
            "unit": "ns/iter",
            "extra": "iterations: 30661\ncpu: 670983.741560941 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_no_interactions/min_time:15.000",
            "value": 355786.00925862475,
            "unit": "ns/iter",
            "extra": "iterations: 58216\ncpu: 355753.330699464 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_interactions/min_time:15.000",
            "value": 555896.2308431116,
            "unit": "ns/iter",
            "extra": "iterations: 37480\ncpu: 555846.2833511207 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_no_namespaces/min_time:15.000",
            "value": 760278.0349029265,
            "unit": "ns/iter",
            "extra": "iterations: 27247\ncpu: 760196.6124710975 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 942018.8560905533,
            "unit": "ns/iter",
            "extra": "iterations: 22264\ncpu: 941926.6708587848 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_interactions/min_time:15.000",
            "value": 3175988.7432493716,
            "unit": "ns/iter",
            "extra": "iterations: 6629\ncpu: 3175678.428118876 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_no_interactions/min_time:15.000",
            "value": 803527.3649257108,
            "unit": "ns/iter",
            "extra": "iterations: 26652\ncpu: 803422.8875881715 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_interactions/min_time:15.000",
            "value": 2471893.2316297665,
            "unit": "ns/iter",
            "extra": "iterations: 8492\ncpu: 2471633.0781912403 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/simple/min_time:15.000",
            "value": 80610.0236440733,
            "unit": "ns/iter",
            "extra": "iterations: 258458\ncpu: 80602.58030318288 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/quadratic/min_time:15.000",
            "value": 3067632.5896162344,
            "unit": "ns/iter",
            "extra": "iterations: 6645\ncpu: 3067339.443190365 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_string_fts",
            "value": 1219.8251049546989,
            "unit": "ns/iter",
            "extra": "iterations: 3416712\ncpu: 1219.7128408832725 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_string_fts",
            "value": 8734.760516040997,
            "unit": "ns/iter",
            "extra": "iterations: 485853\ncpu: 8734.016050122229 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_num_fts",
            "value": 883.6686044659436,
            "unit": "ns/iter",
            "extra": "iterations: 4802355\ncpu: 883.5837833729533 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_num_fts",
            "value": 5893.628121639907,
            "unit": "ns/iter",
            "extra": "iterations: 707761\ncpu: 5893.0754873467 ns\nthreads: 1"
          },
          {
            "name": "benchmark_example_reuse",
            "value": 1050.4070441711847,
            "unit": "ns/iter",
            "extra": "iterations: 4067448\ncpu: 1050.3130464114176 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_char",
            "value": 56.651665538951654,
            "unit": "ns/iter",
            "extra": "iterations: 72945427\ncpu: 56.64658457616575 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_extent",
            "value": 80.32100337167019,
            "unit": "ns/iter",
            "extra": "iterations: 51755338\ncpu: 80.31401321347711 ns\nthreads: 1"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "jackgerrits@users.noreply.github.com",
            "name": "Jack Gerrits",
            "username": "jackgerrits"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "9ab3f6974200a57f2550385c48cf5965393810ed",
          "message": "style: apply more style fixes per clang-tidy (#4208)\n\n* style: apply more style fixes per clang-tidy\r\n\r\n* win fix\r\n\r\n* fixes\r\n\r\n* ignore unused function",
          "timestamp": "2022-10-10T17:12:56-04:00",
          "tree_id": "9a08a328d2940f957b29528e5ee09d0761a081d3",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/9ab3f6974200a57f2550385c48cf5965393810ed"
        },
        "date": 1665438929490,
        "tool": "benchmarkdotnet",
        "benches": [
          {
            "name": "BenchmarkText.Benchmark(args: 120_num_features)",
            "value": 6150.066630045573,
            "unit": "ns",
            "range": "± 63.08699368142197"
          },
          {
            "name": "BenchmarkText.Benchmark(args: 120_string_fts)",
            "value": 9931.697184244791,
            "unit": "ns",
            "range": "± 150.04414437257515"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 1_feature)",
            "value": 665.9999370574951,
            "unit": "ns",
            "range": "± 3.598967845359109"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 8_features)",
            "value": 570.6881904602051,
            "unit": "ns",
            "range": "± 4.187224133852162"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_interactions)",
            "value": 923234.8828125,
            "unit": "ns",
            "range": "± 6713.0047415015015"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_no_interactions)",
            "value": 575689.43359375,
            "unit": "ns",
            "range": "± 4554.985829684675"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_no_namespaces)",
            "value": 574771.5169270834,
            "unit": "ns",
            "range": "± 4587.2507789639885"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_interactions)",
            "value": 917912.890625,
            "unit": "ns",
            "range": "± 5696.754849048671"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_no_interactions)",
            "value": 571826.3741629465,
            "unit": "ns",
            "range": "± 4272.283852694158"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_interactions)",
            "value": 3546440.0948660714,
            "unit": "ns",
            "range": "± 24988.249727263235"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_no_interactions)",
            "value": 1086344.321986607,
            "unit": "ns",
            "range": "± 5293.222488542809"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_no_namespaces)",
            "value": 902268.2896205357,
            "unit": "ns",
            "range": "± 4748.908029891432"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_interactions)",
            "value": 2847793.0524553573,
            "unit": "ns",
            "range": "± 11415.289524114274"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_no_interactions)",
            "value": 926360.8473557692,
            "unit": "ns",
            "range": "± 2916.1763536936396"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: few_features)",
            "value": 3237.9544576009116,
            "unit": "ns",
            "range": "± 17.081481203543007"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: few_features)",
            "value": 11510.741271972656,
            "unit": "ns",
            "range": "± 56.38291822363453"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: many_features)",
            "value": 110546.72770182292,
            "unit": "ns",
            "range": "± 629.5323096381393"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: many_features)",
            "value": 19061.341203962053,
            "unit": "ns",
            "range": "± 105.30075478834603"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: quadratic)",
            "value": 4367721.71875,
            "unit": "ns",
            "range": "± 61579.00660884065"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: simple)",
            "value": 218244.95279947916,
            "unit": "ns",
            "range": "± 1838.2509229289658"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "griffinbassman@gmail.com",
            "name": "Griffin Bassman",
            "username": "bassmang"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "1529e7437e51cf0f3712970879ace730640a8e01",
          "message": "fix: LAS unit test bug (#4210)",
          "timestamp": "2022-10-11T10:10:48-04:00",
          "tree_id": "b77031dd2cdf04ae175af723c295cf32726ec905",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/1529e7437e51cf0f3712970879ace730640a8e01"
        },
        "date": 1665498665361,
        "tool": "googlecpp",
        "benches": [
          {
            "name": "bench_text/120_string_fts",
            "value": 5982.85839134361,
            "unit": "ns/iter",
            "extra": "iterations: 700162\ncpu: 5982.345514323828 ns\nthreads: 1"
          },
          {
            "name": "bench_text/120_num_fts",
            "value": 4731.157221863068,
            "unit": "ns/iter",
            "extra": "iterations: 880043\ncpu: 4724.854921861774 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/8_features",
            "value": 77.91452534333905,
            "unit": "ns/iter",
            "extra": "iterations: 54583770\ncpu: 77.90717094110573 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/1_feature",
            "value": 166.34623950239293,
            "unit": "ns/iter",
            "extra": "iterations: 25258386\ncpu: 166.33691479732704 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features",
            "value": 6925.971459444719,
            "unit": "ns/iter",
            "extra": "iterations: 603387\ncpu: 6925.167264127339 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features",
            "value": 12233.13166382246,
            "unit": "ns/iter",
            "extra": "iterations: 341058\ncpu: 12232.054078778396 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features_no_predict",
            "value": 3269.459388243107,
            "unit": "ns/iter",
            "extra": "iterations: 1286524\ncpu: 3269.2176749131763 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features_no_predic",
            "value": 4934.34768049205,
            "unit": "ns/iter",
            "extra": "iterations: 856755\ncpu: 4933.653027995164 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/few_features",
            "value": 1183.8016575553513,
            "unit": "ns/iter",
            "extra": "iterations: 3527122\ncpu: 1183.717773300725 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/many_features/min_time:15.000",
            "value": 57596.13429212968,
            "unit": "ns/iter",
            "extra": "iterations: 362955\ncpu: 57591.601713711076 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_no_namespaces/min_time:15.000",
            "value": 273886.29161290306,
            "unit": "ns/iter",
            "extra": "iterations: 79825\ncpu: 273861.18634513 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 305866.6774442665,
            "unit": "ns/iter",
            "extra": "iterations: 68630\ncpu: 305838.71630482306 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_interactions/min_time:15.000",
            "value": 549040.9430928892,
            "unit": "ns/iter",
            "extra": "iterations: 37570\ncpu: 548989.5315411232 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_no_interactions/min_time:15.000",
            "value": 273909.5280719777,
            "unit": "ns/iter",
            "extra": "iterations: 75912\ncpu: 273885.07087153563 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_interactions/min_time:15.000",
            "value": 419767.5500527086,
            "unit": "ns/iter",
            "extra": "iterations: 50277\ncpu: 419731.64071046375 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_no_namespaces/min_time:15.000",
            "value": 597049.7077949928,
            "unit": "ns/iter",
            "extra": "iterations: 34907\ncpu: 596989.1225255677 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 772726.7399075219,
            "unit": "ns/iter",
            "extra": "iterations: 27248\ncpu: 772651.2294480336 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_interactions/min_time:15.000",
            "value": 2478131.9904210186,
            "unit": "ns/iter",
            "extra": "iterations: 8456\ncpu: 2477897.457426684 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_no_interactions/min_time:15.000",
            "value": 625117.1186455607,
            "unit": "ns/iter",
            "extra": "iterations: 34051\ncpu: 625055.9073154965 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_interactions/min_time:15.000",
            "value": 1785972.7500431116,
            "unit": "ns/iter",
            "extra": "iterations: 11598\ncpu: 1785575.9613726514 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/simple/min_time:15.000",
            "value": 49875.13837140106,
            "unit": "ns/iter",
            "extra": "iterations: 421749\ncpu: 49870.98843150791 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/quadratic/min_time:15.000",
            "value": 2817895.7349152323,
            "unit": "ns/iter",
            "extra": "iterations: 7375\ncpu: 2817642.8067796626 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_string_fts",
            "value": 951.7053864000547,
            "unit": "ns/iter",
            "extra": "iterations: 4427484\ncpu: 951.6342690340615 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_string_fts",
            "value": 7056.567967322843,
            "unit": "ns/iter",
            "extra": "iterations: 595161\ncpu: 7056.059452820328 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_num_fts",
            "value": 715.2022751458911,
            "unit": "ns/iter",
            "extra": "iterations: 5875843\ncpu: 715.1476477502881 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_num_fts",
            "value": 5103.18818897654,
            "unit": "ns/iter",
            "extra": "iterations: 821690\ncpu: 5102.853874332191 ns\nthreads: 1"
          },
          {
            "name": "benchmark_example_reuse",
            "value": 811.443932377565,
            "unit": "ns/iter",
            "extra": "iterations: 5179228\ncpu: 811.3756335886272 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_char",
            "value": 54.61069514285293,
            "unit": "ns/iter",
            "extra": "iterations: 76413416\ncpu: 54.60639267847946 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_extent",
            "value": 76.37827527966532,
            "unit": "ns/iter",
            "extra": "iterations: 54941835\ncpu: 76.37273673149075 ns\nthreads: 1"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "griffinbassman@gmail.com",
            "name": "Griffin Bassman",
            "username": "bassmang"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "1529e7437e51cf0f3712970879ace730640a8e01",
          "message": "fix: LAS unit test bug (#4210)",
          "timestamp": "2022-10-11T10:10:48-04:00",
          "tree_id": "b77031dd2cdf04ae175af723c295cf32726ec905",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/1529e7437e51cf0f3712970879ace730640a8e01"
        },
        "date": 1665500381672,
        "tool": "benchmarkdotnet",
        "benches": [
          {
            "name": "BenchmarkText.Benchmark(args: 120_num_features)",
            "value": 5997.61723836263,
            "unit": "ns",
            "range": "± 52.605152246809304"
          },
          {
            "name": "BenchmarkText.Benchmark(args: 120_string_fts)",
            "value": 9633.13692533053,
            "unit": "ns",
            "range": "± 39.56874114158999"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 1_feature)",
            "value": 653.7752723693848,
            "unit": "ns",
            "range": "± 4.655159990270967"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 8_features)",
            "value": 589.9337496076312,
            "unit": "ns",
            "range": "± 3.8205339804367813"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_interactions)",
            "value": 909758.9973958334,
            "unit": "ns",
            "range": "± 10302.835187616822"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_no_interactions)",
            "value": 550955.1627604166,
            "unit": "ns",
            "range": "± 4255.196451059262"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_no_namespaces)",
            "value": 568244.7786458334,
            "unit": "ns",
            "range": "± 7298.857165335333"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_interactions)",
            "value": 874474.19921875,
            "unit": "ns",
            "range": "± 5120.033279123462"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_no_interactions)",
            "value": 560969.189453125,
            "unit": "ns",
            "range": "± 2710.7014750566887"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_interactions)",
            "value": 3432039.0234375,
            "unit": "ns",
            "range": "± 8110.965421625269"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_no_interactions)",
            "value": 1055473.69140625,
            "unit": "ns",
            "range": "± 4786.422134740879"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_no_namespaces)",
            "value": 909961.5234375,
            "unit": "ns",
            "range": "± 4161.340265061566"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_interactions)",
            "value": 2753234.296875,
            "unit": "ns",
            "range": "± 4823.189286124685"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_no_interactions)",
            "value": 919393.5407366072,
            "unit": "ns",
            "range": "± 3193.670971187293"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: few_features)",
            "value": 3273.6190250941686,
            "unit": "ns",
            "range": "± 4.791819512784536"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: few_features)",
            "value": 11448.011670793805,
            "unit": "ns",
            "range": "± 10.02352460398693"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: many_features)",
            "value": 107782.41838727679,
            "unit": "ns",
            "range": "± 281.0893818493322"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: many_features)",
            "value": 19031.62862141927,
            "unit": "ns",
            "range": "± 30.027781623657766"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: quadratic)",
            "value": 4329397.03125,
            "unit": "ns",
            "range": "± 52661.94928305858"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: simple)",
            "value": 215664.68912760416,
            "unit": "ns",
            "range": "± 869.4404757212181"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "jackgerrits@users.noreply.github.com",
            "name": "Jack Gerrits",
            "username": "jackgerrits"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "a38769d8522b1e6536b10f118f3c56870b2e16fa",
          "message": "docs: only document public includes with doxygen (#4212)\n\n* doc: only document public includes\r\n\r\n* fix workflow",
          "timestamp": "2022-10-12T13:16:12-04:00",
          "tree_id": "5ce6055a06cc3980c476fc7dff99e2fc92647db9",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/a38769d8522b1e6536b10f118f3c56870b2e16fa"
        },
        "date": 1665596393269,
        "tool": "googlecpp",
        "benches": [
          {
            "name": "bench_text/120_string_fts",
            "value": 7187.723754432937,
            "unit": "ns/iter",
            "extra": "iterations: 562876\ncpu: 7178.5419879333995 ns\nthreads: 1"
          },
          {
            "name": "bench_text/120_num_fts",
            "value": 5666.7560283611665,
            "unit": "ns/iter",
            "extra": "iterations: 736643\ncpu: 5666.179003940851 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/8_features",
            "value": 93.01913606248327,
            "unit": "ns/iter",
            "extra": "iterations: 46267721\ncpu: 93.0103732578486 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/1_feature",
            "value": 199.69196351163893,
            "unit": "ns/iter",
            "extra": "iterations: 21056249\ncpu: 199.55716234168787 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features",
            "value": 8252.998077062439,
            "unit": "ns/iter",
            "extra": "iterations: 511717\ncpu: 8252.025045093284 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features",
            "value": 14950.232817437874,
            "unit": "ns/iter",
            "extra": "iterations: 278480\ncpu: 14948.408503303663 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features_no_predict",
            "value": 3930.2119791202654,
            "unit": "ns/iter",
            "extra": "iterations: 1079328\ncpu: 3929.8041929793335 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features_no_predic",
            "value": 5954.494817119565,
            "unit": "ns/iter",
            "extra": "iterations: 699418\ncpu: 5953.870789713726 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/few_features",
            "value": 1424.4680498082264,
            "unit": "ns/iter",
            "extra": "iterations: 2965616\ncpu: 1424.3392940960662 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/many_features/min_time:15.000",
            "value": 70654.54395972037,
            "unit": "ns/iter",
            "extra": "iterations: 298910\ncpu: 70647.23662640929 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_no_namespaces/min_time:15.000",
            "value": 338857.0768463403,
            "unit": "ns/iter",
            "extra": "iterations: 64154\ncpu: 338817.6949215949 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 375290.66426434234,
            "unit": "ns/iter",
            "extra": "iterations: 56473\ncpu: 375248.15221433266 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_interactions/min_time:15.000",
            "value": 666394.4028384619,
            "unit": "ns/iter",
            "extra": "iterations: 31355\ncpu: 666183.8080051033 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_no_interactions/min_time:15.000",
            "value": 342275.15090435155,
            "unit": "ns/iter",
            "extra": "iterations: 61370\ncpu: 342234.33599478577 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_interactions/min_time:15.000",
            "value": 518747.3259861092,
            "unit": "ns/iter",
            "extra": "iterations: 40741\ncpu: 518614.4571807269 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_no_namespaces/min_time:15.000",
            "value": 710538.8369670195,
            "unit": "ns/iter",
            "extra": "iterations: 28988\ncpu: 710450.0034497031 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 929284.8046491068,
            "unit": "ns/iter",
            "extra": "iterations: 22585\ncpu: 929045.2291343809 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_interactions/min_time:15.000",
            "value": 2963828.7217354346,
            "unit": "ns/iter",
            "extra": "iterations: 7076\ncpu: 2963468.273035618 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_no_interactions/min_time:15.000",
            "value": 739283.3921885985,
            "unit": "ns/iter",
            "extra": "iterations: 28420\ncpu: 738906.8719211824 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_interactions/min_time:15.000",
            "value": 2243236.2413573,
            "unit": "ns/iter",
            "extra": "iterations: 9401\ncpu: 2242962.1316881143 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/simple/min_time:15.000",
            "value": 61427.978345167794,
            "unit": "ns/iter",
            "extra": "iterations: 340663\ncpu: 61421.45463405198 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/quadratic/min_time:15.000",
            "value": 3493674.918973305,
            "unit": "ns/iter",
            "extra": "iterations: 5961\ncpu: 3492938.9028686513 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_string_fts",
            "value": 1138.3671762488252,
            "unit": "ns/iter",
            "extra": "iterations: 3688512\ncpu: 1138.2729946384936 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_string_fts",
            "value": 8485.770367166462,
            "unit": "ns/iter",
            "extra": "iterations: 494816\ncpu: 8485.00210179137 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_num_fts",
            "value": 858.6886347250752,
            "unit": "ns/iter",
            "extra": "iterations: 4889235\ncpu: 858.6146503491765 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_num_fts",
            "value": 6117.81626009826,
            "unit": "ns/iter",
            "extra": "iterations: 684756\ncpu: 6117.261038968598 ns\nthreads: 1"
          },
          {
            "name": "benchmark_example_reuse",
            "value": 976.0315175576092,
            "unit": "ns/iter",
            "extra": "iterations: 4309820\ncpu: 975.9309901573489 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_char",
            "value": 65.56887720949318,
            "unit": "ns/iter",
            "extra": "iterations: 64095795\ncpu: 65.56331347477631 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_extent",
            "value": 91.77044108862874,
            "unit": "ns/iter",
            "extra": "iterations: 45836661\ncpu: 91.7155374821042 ns\nthreads: 1"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "jackgerrits@users.noreply.github.com",
            "name": "Jack Gerrits",
            "username": "jackgerrits"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "a38769d8522b1e6536b10f118f3c56870b2e16fa",
          "message": "docs: only document public includes with doxygen (#4212)\n\n* doc: only document public includes\r\n\r\n* fix workflow",
          "timestamp": "2022-10-12T13:16:12-04:00",
          "tree_id": "5ce6055a06cc3980c476fc7dff99e2fc92647db9",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/a38769d8522b1e6536b10f118f3c56870b2e16fa"
        },
        "date": 1665597478019,
        "tool": "benchmarkdotnet",
        "benches": [
          {
            "name": "BenchmarkText.Benchmark(args: 120_num_features)",
            "value": 5095.55778503418,
            "unit": "ns",
            "range": "± 15.631768104605666"
          },
          {
            "name": "BenchmarkText.Benchmark(args: 120_string_fts)",
            "value": 7382.742091587612,
            "unit": "ns",
            "range": "± 28.80224829359624"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 1_feature)",
            "value": 662.0953687032064,
            "unit": "ns",
            "range": "± 1.4885944627032255"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 8_features)",
            "value": 563.1497446695963,
            "unit": "ns",
            "range": "± 1.5391220198265763"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_interactions)",
            "value": 844036.9596354166,
            "unit": "ns",
            "range": "± 1320.5000311899869"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_no_interactions)",
            "value": 453973.69559151784,
            "unit": "ns",
            "range": "± 853.7607767350088"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_no_namespaces)",
            "value": 452423.9225260417,
            "unit": "ns",
            "range": "± 1609.5625670427637"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_interactions)",
            "value": 839214.0625,
            "unit": "ns",
            "range": "± 1211.0832643833448"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_no_interactions)",
            "value": 455320.546875,
            "unit": "ns",
            "range": "± 1984.9968351233895"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_interactions)",
            "value": 3364041.9270833335,
            "unit": "ns",
            "range": "± 5590.425727686424"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_no_interactions)",
            "value": 1020575.4231770834,
            "unit": "ns",
            "range": "± 3079.7668855263705"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_no_namespaces)",
            "value": 843042.9268973215,
            "unit": "ns",
            "range": "± 1563.7834124336755"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_interactions)",
            "value": 2805829.2317708335,
            "unit": "ns",
            "range": "± 5852.167236541985"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_no_interactions)",
            "value": 858110.7979910715,
            "unit": "ns",
            "range": "± 1279.7680422678538"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: few_features)",
            "value": 3335.4726246425084,
            "unit": "ns",
            "range": "± 4.6111971350278615"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: few_features)",
            "value": 11243.933512369791,
            "unit": "ns",
            "range": "± 7.532796378393065"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: many_features)",
            "value": 124931.57435825893,
            "unit": "ns",
            "range": "± 124.83436520133027"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: many_features)",
            "value": 19595.206862229566,
            "unit": "ns",
            "range": "± 11.93552250699554"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: quadratic)",
            "value": 4798325.279017857,
            "unit": "ns",
            "range": "± 11988.901491693628"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: simple)",
            "value": 211559.8876953125,
            "unit": "ns",
            "range": "± 404.17264676289585"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "griffinbassman@gmail.com",
            "name": "Griffin Bassman",
            "username": "bassmang"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "14b59e8249edaeb74251e9ac0d2d25966767ed0f",
          "message": "refactor: update structs to classes with public (#4215)\n\n* src\r\n\r\n* include without learner\r\n\r\n* add learner\r\n\r\n* rest\r\n\r\n* clang\r\n\r\n* more fixes\r\n\r\n* zlib fix\r\n\r\n* fix test",
          "timestamp": "2022-10-12T15:06:16-04:00",
          "tree_id": "7c3744c390388af37c519cd86d2dcf9df75aeb8a",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/14b59e8249edaeb74251e9ac0d2d25966767ed0f"
        },
        "date": 1665602979693,
        "tool": "googlecpp",
        "benches": [
          {
            "name": "bench_text/120_string_fts",
            "value": 7482.527634239383,
            "unit": "ns/iter",
            "extra": "iterations: 560826\ncpu: 7467.82228356032 ns\nthreads: 1"
          },
          {
            "name": "bench_text/120_num_fts",
            "value": 5837.669281205727,
            "unit": "ns/iter",
            "extra": "iterations: 711024\ncpu: 5836.51437926146 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/8_features",
            "value": 93.1049101306522,
            "unit": "ns/iter",
            "extra": "iterations: 45114604\ncpu: 93.09340274825418 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/1_feature",
            "value": 213.0653257277868,
            "unit": "ns/iter",
            "extra": "iterations: 19931060\ncpu: 213.0370135858303 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features",
            "value": 8690.821890119845,
            "unit": "ns/iter",
            "extra": "iterations: 491281\ncpu: 8689.505395079395 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features",
            "value": 15625.263660140366,
            "unit": "ns/iter",
            "extra": "iterations: 267640\ncpu: 15622.798535346004 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features_no_predict",
            "value": 4057.2842726435183,
            "unit": "ns/iter",
            "extra": "iterations: 1022346\ncpu: 4056.75691008719 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features_no_predic",
            "value": 6256.1523954853665,
            "unit": "ns/iter",
            "extra": "iterations: 682179\ncpu: 6255.304839345677 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/few_features",
            "value": 1492.520680672098,
            "unit": "ns/iter",
            "extra": "iterations: 2842896\ncpu: 1492.3262053905603 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/many_features/min_time:15.000",
            "value": 72749.77749871436,
            "unit": "ns/iter",
            "extra": "iterations: 289859\ncpu: 72739.43020572068 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_no_namespaces/min_time:15.000",
            "value": 339434.6977072642,
            "unit": "ns/iter",
            "extra": "iterations: 63723\ncpu: 339382.83037521783 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 369922.3969262067,
            "unit": "ns/iter",
            "extra": "iterations: 56998\ncpu: 369866.518123443 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_interactions/min_time:15.000",
            "value": 667294.1184927139,
            "unit": "ns/iter",
            "extra": "iterations: 31580\ncpu: 667185.9088030396 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_no_interactions/min_time:15.000",
            "value": 332783.1351133204,
            "unit": "ns/iter",
            "extra": "iterations: 63184\ncpu: 332719.30235502624 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_interactions/min_time:15.000",
            "value": 513197.3989530797,
            "unit": "ns/iter",
            "extra": "iterations: 40882\ncpu: 513120.82579130173 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_no_namespaces/min_time:15.000",
            "value": 712418.9233025305,
            "unit": "ns/iter",
            "extra": "iterations: 29323\ncpu: 712049.292364356 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 936491.3451670422,
            "unit": "ns/iter",
            "extra": "iterations: 22450\ncpu: 936334.2093541189 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_interactions/min_time:15.000",
            "value": 3048379.19471946,
            "unit": "ns/iter",
            "extra": "iterations: 6969\ncpu: 3047867.4989238004 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_no_interactions/min_time:15.000",
            "value": 742688.2406868694,
            "unit": "ns/iter",
            "extra": "iterations: 28186\ncpu: 742560.2852479947 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_interactions/min_time:15.000",
            "value": 2269737.908340471,
            "unit": "ns/iter",
            "extra": "iterations: 9328\ncpu: 2269347.266295022 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/simple/min_time:15.000",
            "value": 60649.12449516922,
            "unit": "ns/iter",
            "extra": "iterations: 347146\ncpu: 60640.72436381221 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/quadratic/min_time:15.000",
            "value": 3367233.054996752,
            "unit": "ns/iter",
            "extra": "iterations: 6164\ncpu: 3366720.165476963 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_string_fts",
            "value": 1185.750625658312,
            "unit": "ns/iter",
            "extra": "iterations: 3542269\ncpu: 1185.5986657139865 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_string_fts",
            "value": 8830.30691935416,
            "unit": "ns/iter",
            "extra": "iterations: 475001\ncpu: 8829.107096616692 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_num_fts",
            "value": 893.5623832391175,
            "unit": "ns/iter",
            "extra": "iterations: 4681041\ncpu: 893.44387284794 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_num_fts",
            "value": 6359.542844210177,
            "unit": "ns/iter",
            "extra": "iterations: 660556\ncpu: 6358.69116320192 ns\nthreads: 1"
          },
          {
            "name": "benchmark_example_reuse",
            "value": 1014.8780535068463,
            "unit": "ns/iter",
            "extra": "iterations: 4138684\ncpu: 1014.7350703750378 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_char",
            "value": 68.1817433282908,
            "unit": "ns/iter",
            "extra": "iterations: 61658599\ncpu: 68.17237933025348 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_extent",
            "value": 95.46051482907784,
            "unit": "ns/iter",
            "extra": "iterations: 43998214\ncpu: 95.44805841437167 ns\nthreads: 1"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "griffinbassman@gmail.com",
            "name": "Griffin Bassman",
            "username": "bassmang"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "14b59e8249edaeb74251e9ac0d2d25966767ed0f",
          "message": "refactor: update structs to classes with public (#4215)\n\n* src\r\n\r\n* include without learner\r\n\r\n* add learner\r\n\r\n* rest\r\n\r\n* clang\r\n\r\n* more fixes\r\n\r\n* zlib fix\r\n\r\n* fix test",
          "timestamp": "2022-10-12T15:06:16-04:00",
          "tree_id": "7c3744c390388af37c519cd86d2dcf9df75aeb8a",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/14b59e8249edaeb74251e9ac0d2d25966767ed0f"
        },
        "date": 1665604231140,
        "tool": "benchmarkdotnet",
        "benches": [
          {
            "name": "BenchmarkText.Benchmark(args: 120_num_features)",
            "value": 5092.892328898112,
            "unit": "ns",
            "range": "± 8.623737609836326"
          },
          {
            "name": "BenchmarkText.Benchmark(args: 120_string_fts)",
            "value": 7506.648744855608,
            "unit": "ns",
            "range": "± 45.19768968534892"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 1_feature)",
            "value": 658.1539217631022,
            "unit": "ns",
            "range": "± 1.4363120017552446"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 8_features)",
            "value": 554.3468602498373,
            "unit": "ns",
            "range": "± 1.4732217499904707"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_interactions)",
            "value": 847263.1477864584,
            "unit": "ns",
            "range": "± 835.3825501190416"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_no_interactions)",
            "value": 460082.0475260417,
            "unit": "ns",
            "range": "± 2085.8902572798265"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_no_namespaces)",
            "value": 456676.54296875,
            "unit": "ns",
            "range": "± 1123.2492417331162"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_interactions)",
            "value": 845172.3046875,
            "unit": "ns",
            "range": "± 1058.3143824141816"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_no_interactions)",
            "value": 463046.7317708333,
            "unit": "ns",
            "range": "± 1197.5498146077844"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_interactions)",
            "value": 3489591.1197916665,
            "unit": "ns",
            "range": "± 7752.472835057412"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_no_interactions)",
            "value": 1042024.1768973215,
            "unit": "ns",
            "range": "± 1833.743259618684"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_no_namespaces)",
            "value": 887203.2376802885,
            "unit": "ns",
            "range": "± 1698.9551843595623"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_interactions)",
            "value": 2780548.5416666665,
            "unit": "ns",
            "range": "± 6100.030352169285"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_no_interactions)",
            "value": 871954.0597098215,
            "unit": "ns",
            "range": "± 1232.9640174201618"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: few_features)",
            "value": 3351.7540795462473,
            "unit": "ns",
            "range": "± 4.345017934854603"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: few_features)",
            "value": 11194.664588341346,
            "unit": "ns",
            "range": "± 15.93516993205479"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: many_features)",
            "value": 125862.5341796875,
            "unit": "ns",
            "range": "± 266.1052906921553"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: many_features)",
            "value": 19810.48801967076,
            "unit": "ns",
            "range": "± 18.578533318100657"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: quadratic)",
            "value": 4934587.005208333,
            "unit": "ns",
            "range": "± 50805.31329923208"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: simple)",
            "value": 213400.3564453125,
            "unit": "ns",
            "range": "± 316.9469660664937"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "griffinbassman@gmail.com",
            "name": "Griffin Bassman",
            "username": "bassmang"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "6d83ea37a4c9f44e60cbcd8e7a8d4d95aa9c2b37",
          "message": "fix: quake_inv_sqrt func for aarch64 test failure (#4217)\n\n* fix: quake_inv_sqrt func for aarch64 test failure\r\n\r\n* comment back",
          "timestamp": "2022-10-12T16:29:49-04:00",
          "tree_id": "71eff9fdd0f274b21d534d56f1693242bc28167e",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/6d83ea37a4c9f44e60cbcd8e7a8d4d95aa9c2b37"
        },
        "date": 1665607799034,
        "tool": "googlecpp",
        "benches": [
          {
            "name": "bench_text/120_string_fts",
            "value": 5973.688725745495,
            "unit": "ns/iter",
            "extra": "iterations: 700951\ncpu: 5973.317535747863 ns\nthreads: 1"
          },
          {
            "name": "bench_text/120_num_fts",
            "value": 4573.502406108909,
            "unit": "ns/iter",
            "extra": "iterations: 909352\ncpu: 4573.251502168579 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/8_features",
            "value": 75.16760976274944,
            "unit": "ns/iter",
            "extra": "iterations: 56442464\ncpu: 75.1191585115774 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/1_feature",
            "value": 166.2586711835984,
            "unit": "ns/iter",
            "extra": "iterations: 25021094\ncpu: 166.24436165740784 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features",
            "value": 6949.921627427302,
            "unit": "ns/iter",
            "extra": "iterations: 606577\ncpu: 6948.758030719924 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features",
            "value": 12515.016031821508,
            "unit": "ns/iter",
            "extra": "iterations: 330343\ncpu: 12507.26517589294 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features_no_predict",
            "value": 3276.968083447183,
            "unit": "ns/iter",
            "extra": "iterations: 1288767\ncpu: 3276.713090884542 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features_no_predic",
            "value": 4953.413499129583,
            "unit": "ns/iter",
            "extra": "iterations: 843847\ncpu: 4953.040420834589 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/few_features",
            "value": 1173.9187314056376,
            "unit": "ns/iter",
            "extra": "iterations: 3584755\ncpu: 1173.8186849589442 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/many_features/min_time:15.000",
            "value": 58848.38549152274,
            "unit": "ns/iter",
            "extra": "iterations: 357074\ncpu: 58842.64886270074 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_no_namespaces/min_time:15.000",
            "value": 279233.73853885935,
            "unit": "ns/iter",
            "extra": "iterations: 76803\ncpu: 279172.4789396246 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 309846.4141705069,
            "unit": "ns/iter",
            "extra": "iterations: 67704\ncpu: 309809.66560321406 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_interactions/min_time:15.000",
            "value": 552305.6258376471,
            "unit": "ns/iter",
            "extra": "iterations: 38053\ncpu: 552245.1344177859 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_no_interactions/min_time:15.000",
            "value": 280338.0041555917,
            "unit": "ns/iter",
            "extra": "iterations: 74117\ncpu: 280310.3093757165 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_interactions/min_time:15.000",
            "value": 428089.06844013464,
            "unit": "ns/iter",
            "extra": "iterations: 49094\ncpu: 428046.8814926472 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_no_namespaces/min_time:15.000",
            "value": 591165.7672579892,
            "unit": "ns/iter",
            "extra": "iterations: 35288\ncpu: 591033.2917705729 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 768900.6118215532,
            "unit": "ns/iter",
            "extra": "iterations: 27526\ncpu: 768815.2619341714 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_interactions/min_time:15.000",
            "value": 2459098.435562816,
            "unit": "ns/iter",
            "extra": "iterations: 8582\ncpu: 2458575.0524353324 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_no_interactions/min_time:15.000",
            "value": 614049.2489168019,
            "unit": "ns/iter",
            "extra": "iterations: 34389\ncpu: 613983.3987612313 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_interactions/min_time:15.000",
            "value": 1858678.6002478537,
            "unit": "ns/iter",
            "extra": "iterations: 11297\ncpu: 1858271.7889705237 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/simple/min_time:15.000",
            "value": 50114.46847025418,
            "unit": "ns/iter",
            "extra": "iterations: 418795\ncpu: 50110.12428515139 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/quadratic/min_time:15.000",
            "value": 2794677.3895701566,
            "unit": "ns/iter",
            "extra": "iterations: 7421\ncpu: 2794387.6431747708 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_string_fts",
            "value": 949.84771713357,
            "unit": "ns/iter",
            "extra": "iterations: 4427701\ncpu: 949.3292568762057 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_string_fts",
            "value": 7059.053444046543,
            "unit": "ns/iter",
            "extra": "iterations: 594809\ncpu: 7058.511723931559 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_num_fts",
            "value": 767.4603863723655,
            "unit": "ns/iter",
            "extra": "iterations: 5850108\ncpu: 767.3988411837937 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_num_fts",
            "value": 5097.10054154994,
            "unit": "ns/iter",
            "extra": "iterations: 825778\ncpu: 5095.738200823934 ns\nthreads: 1"
          },
          {
            "name": "benchmark_example_reuse",
            "value": 811.4067981042143,
            "unit": "ns/iter",
            "extra": "iterations: 5178532\ncpu: 811.3303538531766 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_char",
            "value": 54.67360953614822,
            "unit": "ns/iter",
            "extra": "iterations: 76985892\ncpu: 54.66923861842123 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_extent",
            "value": 76.63419813333358,
            "unit": "ns/iter",
            "extra": "iterations: 54824969\ncpu: 76.62861970792906 ns\nthreads: 1"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "griffinbassman@gmail.com",
            "name": "Griffin Bassman",
            "username": "bassmang"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "6d83ea37a4c9f44e60cbcd8e7a8d4d95aa9c2b37",
          "message": "fix: quake_inv_sqrt func for aarch64 test failure (#4217)\n\n* fix: quake_inv_sqrt func for aarch64 test failure\r\n\r\n* comment back",
          "timestamp": "2022-10-12T16:29:49-04:00",
          "tree_id": "71eff9fdd0f274b21d534d56f1693242bc28167e",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/6d83ea37a4c9f44e60cbcd8e7a8d4d95aa9c2b37"
        },
        "date": 1665609612678,
        "tool": "benchmarkdotnet",
        "benches": [
          {
            "name": "BenchmarkText.Benchmark(args: 120_num_features)",
            "value": 5213.239860534668,
            "unit": "ns",
            "range": "± 15.287496452777805"
          },
          {
            "name": "BenchmarkText.Benchmark(args: 120_string_fts)",
            "value": 7506.570180257161,
            "unit": "ns",
            "range": "± 7.77632002652002"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 1_feature)",
            "value": 663.091869354248,
            "unit": "ns",
            "range": "± 1.2552246210126936"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 8_features)",
            "value": 564.7235171000162,
            "unit": "ns",
            "range": "± 1.2229786043326691"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_interactions)",
            "value": 851904.443359375,
            "unit": "ns",
            "range": "± 897.2215198274763"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_no_interactions)",
            "value": 463498.2649739583,
            "unit": "ns",
            "range": "± 1606.0459440839434"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_no_namespaces)",
            "value": 466803.1087239583,
            "unit": "ns",
            "range": "± 1109.9392813486036"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_interactions)",
            "value": 847582.5962611607,
            "unit": "ns",
            "range": "± 1166.4835779819407"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_no_interactions)",
            "value": 464713.55329241074,
            "unit": "ns",
            "range": "± 1087.0386285781515"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_interactions)",
            "value": 3528761.8024553573,
            "unit": "ns",
            "range": "± 4815.037333095491"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_no_interactions)",
            "value": 1027237.3177083334,
            "unit": "ns",
            "range": "± 2039.4166742146358"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_no_namespaces)",
            "value": 861512.6046316965,
            "unit": "ns",
            "range": "± 1521.0575458729536"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_interactions)",
            "value": 2802590.1822916665,
            "unit": "ns",
            "range": "± 4340.105749821222"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_no_interactions)",
            "value": 876421.0416666666,
            "unit": "ns",
            "range": "± 1415.880873103921"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: few_features)",
            "value": 3360.152212778727,
            "unit": "ns",
            "range": "± 2.8970727520272095"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: few_features)",
            "value": 11253.833923339844,
            "unit": "ns",
            "range": "± 14.48613452240136"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: many_features)",
            "value": 125956.78187779018,
            "unit": "ns",
            "range": "± 277.9083371073065"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: many_features)",
            "value": 19463.472420828683,
            "unit": "ns",
            "range": "± 18.575404744465043"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: quadratic)",
            "value": 4873327.34375,
            "unit": "ns",
            "range": "± 10883.37826861069"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: simple)",
            "value": 211693.7898763021,
            "unit": "ns",
            "range": "± 233.63685226048779"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "jackgerrits@users.noreply.github.com",
            "name": "Jack Gerrits",
            "username": "jackgerrits"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "f5821240bc4cd3f3e3decdde5aa5969b0d7c0be0",
          "message": "style: style fixes in io project (#4201)\n\n* style: style fixes in io project\r\n\r\n* rename to avoid windows macro conflict\r\n\r\n* try and fix macro conflicts\r\n\r\n* fix win issue",
          "timestamp": "2022-10-13T09:06:54-04:00",
          "tree_id": "3d089a167cf6cab7e732359a956498e4b7a71c05",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/f5821240bc4cd3f3e3decdde5aa5969b0d7c0be0"
        },
        "date": 1665667694106,
        "tool": "googlecpp",
        "benches": [
          {
            "name": "bench_text/120_string_fts",
            "value": 5973.308358635808,
            "unit": "ns/iter",
            "extra": "iterations: 700856\ncpu: 5972.495491227871 ns\nthreads: 1"
          },
          {
            "name": "bench_text/120_num_fts",
            "value": 4794.80315214842,
            "unit": "ns/iter",
            "extra": "iterations: 882763\ncpu: 4794.386375505091 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/8_features",
            "value": 76.55671562617356,
            "unit": "ns/iter",
            "extra": "iterations: 56486408\ncpu: 76.55105631783138 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/1_feature",
            "value": 166.25238961041842,
            "unit": "ns/iter",
            "extra": "iterations: 25274622\ncpu: 166.2414140160039 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features",
            "value": 6948.98874098296,
            "unit": "ns/iter",
            "extra": "iterations: 620214\ncpu: 6948.296072000951 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features",
            "value": 12356.59253514262,
            "unit": "ns/iter",
            "extra": "iterations: 341038\ncpu: 12355.443088453481 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features_no_predict",
            "value": 3266.171989189321,
            "unit": "ns/iter",
            "extra": "iterations: 1280220\ncpu: 3265.929137179547 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features_no_predic",
            "value": 4903.965688982089,
            "unit": "ns/iter",
            "extra": "iterations: 854536\ncpu: 4903.605582444737 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/few_features",
            "value": 1188.8306971835855,
            "unit": "ns/iter",
            "extra": "iterations: 3583762\ncpu: 1188.7298319475456 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/many_features/min_time:15.000",
            "value": 58038.80763876685,
            "unit": "ns/iter",
            "extra": "iterations: 358461\ncpu: 58033.71552274863 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_no_namespaces/min_time:15.000",
            "value": 263907.45764545404,
            "unit": "ns/iter",
            "extra": "iterations: 81349\ncpu: 263882.74225866323 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 295348.6534724177,
            "unit": "ns/iter",
            "extra": "iterations: 71420\ncpu: 295322.9263511621 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_interactions/min_time:15.000",
            "value": 540616.2103547308,
            "unit": "ns/iter",
            "extra": "iterations: 38649\ncpu: 540568.5114750703 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_no_interactions/min_time:15.000",
            "value": 265941.338415753,
            "unit": "ns/iter",
            "extra": "iterations: 78965\ncpu: 265916.76059013465 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_interactions/min_time:15.000",
            "value": 412350.78686812945,
            "unit": "ns/iter",
            "extra": "iterations: 50762\ncpu: 412310.2517631302 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_no_namespaces/min_time:15.000",
            "value": 593896.5874183469,
            "unit": "ns/iter",
            "extra": "iterations: 35210\ncpu: 593830.4373757462 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 764279.1387519607,
            "unit": "ns/iter",
            "extra": "iterations: 27387\ncpu: 764208.5332457006 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_interactions/min_time:15.000",
            "value": 2399997.373249846,
            "unit": "ns/iter",
            "extra": "iterations: 8785\ncpu: 2399764.894706888 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_no_interactions/min_time:15.000",
            "value": 608136.1915501129,
            "unit": "ns/iter",
            "extra": "iterations: 34320\ncpu: 608080.186480186 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_interactions/min_time:15.000",
            "value": 1783251.2364206456,
            "unit": "ns/iter",
            "extra": "iterations: 11801\ncpu: 1783039.3441233765 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/simple/min_time:15.000",
            "value": 48057.21441648396,
            "unit": "ns/iter",
            "extra": "iterations: 426595\ncpu: 48047.79123055834 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/quadratic/min_time:15.000",
            "value": 2753230.272105252,
            "unit": "ns/iter",
            "extra": "iterations: 7600\ncpu: 2752934.2763157906 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_string_fts",
            "value": 950.3966973167626,
            "unit": "ns/iter",
            "extra": "iterations: 4420103\ncpu: 950.3293701526866 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_string_fts",
            "value": 7065.825105588211,
            "unit": "ns/iter",
            "extra": "iterations: 594999\ncpu: 7065.288681157453 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_num_fts",
            "value": 719.6603977425327,
            "unit": "ns/iter",
            "extra": "iterations: 5851775\ncpu: 719.6099644979497 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_num_fts",
            "value": 5101.3600533021845,
            "unit": "ns/iter",
            "extra": "iterations: 823978\ncpu: 5101.008522072224 ns\nthreads: 1"
          },
          {
            "name": "benchmark_example_reuse",
            "value": 814.6128083564106,
            "unit": "ns/iter",
            "extra": "iterations: 5162614\ncpu: 814.5512525243997 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_char",
            "value": 54.576293619937026,
            "unit": "ns/iter",
            "extra": "iterations: 76967989\ncpu: 54.572484672816785 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_extent",
            "value": 76.30987389430099,
            "unit": "ns/iter",
            "extra": "iterations: 54972059\ncpu: 76.30491883158308 ns\nthreads: 1"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "jackgerrits@users.noreply.github.com",
            "name": "Jack Gerrits",
            "username": "jackgerrits"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "f5821240bc4cd3f3e3decdde5aa5969b0d7c0be0",
          "message": "style: style fixes in io project (#4201)\n\n* style: style fixes in io project\r\n\r\n* rename to avoid windows macro conflict\r\n\r\n* try and fix macro conflicts\r\n\r\n* fix win issue",
          "timestamp": "2022-10-13T09:06:54-04:00",
          "tree_id": "3d089a167cf6cab7e732359a956498e4b7a71c05",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/f5821240bc4cd3f3e3decdde5aa5969b0d7c0be0"
        },
        "date": 1665669740049,
        "tool": "benchmarkdotnet",
        "benches": [
          {
            "name": "BenchmarkText.Benchmark(args: 120_num_features)",
            "value": 5184.93532034067,
            "unit": "ns",
            "range": "± 9.153431142573522"
          },
          {
            "name": "BenchmarkText.Benchmark(args: 120_string_fts)",
            "value": 7515.115737915039,
            "unit": "ns",
            "range": "± 8.360098479469551"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 1_feature)",
            "value": 663.4260177612305,
            "unit": "ns",
            "range": "± 1.8341679254743213"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 8_features)",
            "value": 559.1928545633951,
            "unit": "ns",
            "range": "± 0.7754570857673178"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_interactions)",
            "value": 848430.908203125,
            "unit": "ns",
            "range": "± 987.8101272337286"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_no_interactions)",
            "value": 461439.3717447917,
            "unit": "ns",
            "range": "± 1596.859849135059"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_no_namespaces)",
            "value": 462362.0735677083,
            "unit": "ns",
            "range": "± 1178.9395137005067"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_interactions)",
            "value": 847951.953125,
            "unit": "ns",
            "range": "± 5092.647212549761"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_no_interactions)",
            "value": 464997.1907552083,
            "unit": "ns",
            "range": "± 962.4719636663841"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_interactions)",
            "value": 3474091.9596354165,
            "unit": "ns",
            "range": "± 3339.421783793358"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_no_interactions)",
            "value": 1015875.0911458334,
            "unit": "ns",
            "range": "± 1641.0310650002707"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_no_namespaces)",
            "value": 843005.6380208334,
            "unit": "ns",
            "range": "± 1697.6874925487466"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_interactions)",
            "value": 2822107.8125,
            "unit": "ns",
            "range": "± 18137.59025573057"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_no_interactions)",
            "value": 874314.0764508928,
            "unit": "ns",
            "range": "± 2529.618014856345"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: few_features)",
            "value": 3337.3882802327475,
            "unit": "ns",
            "range": "± 3.905029981442435"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: few_features)",
            "value": 11222.074672154018,
            "unit": "ns",
            "range": "± 7.8178514633538265"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: many_features)",
            "value": 125824.44661458333,
            "unit": "ns",
            "range": "± 181.45755540955176"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: many_features)",
            "value": 19676.855686732702,
            "unit": "ns",
            "range": "± 10.51603836223997"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: quadratic)",
            "value": 4847795.758928572,
            "unit": "ns",
            "range": "± 8488.89722962261"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: simple)",
            "value": 213323.798828125,
            "unit": "ns",
            "range": "± 343.50557805170956"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "byronxu@microsoft.com",
            "name": "Byron Xu",
            "username": "byronxu99"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "2522e9e9731d692751204db1f24aaa49a3a41098",
          "message": "refactor: No RapidJSON in header files (#4219)\n\n* Move implementation of functions using RapidJSON to .cc source files\r\n* Replace the header files with just a declaration of the function\r\n* Explicitly instantiate templates for the two possible values of <bool audit>\r\n* Simplify CMake files now that RapidJSON no longer needs to be exported",
          "timestamp": "2022-10-13T14:26:33Z",
          "tree_id": "68f2d8d29f0cd9419eda06c7a096ff5f3da1466e",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/2522e9e9731d692751204db1f24aaa49a3a41098"
        },
        "date": 1665672423390,
        "tool": "googlecpp",
        "benches": [
          {
            "name": "bench_text/120_string_fts",
            "value": 5974.677372131667,
            "unit": "ns/iter",
            "extra": "iterations: 701542\ncpu: 5968.42826231359 ns\nthreads: 1"
          },
          {
            "name": "bench_text/120_num_fts",
            "value": 4470.35396670193,
            "unit": "ns/iter",
            "extra": "iterations: 938916\ncpu: 4467.717133375085 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/8_features",
            "value": 75.52875246981161,
            "unit": "ns/iter",
            "extra": "iterations: 54957644\ncpu: 75.47395772642653 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/1_feature",
            "value": 175.36768703950474,
            "unit": "ns/iter",
            "extra": "iterations: 24304384\ncpu: 175.3435717605515 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features",
            "value": 7039.2982183371905,
            "unit": "ns/iter",
            "extra": "iterations: 600731\ncpu: 7038.5715070472415 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features",
            "value": 12577.61294215847,
            "unit": "ns/iter",
            "extra": "iterations: 333082\ncpu: 12576.466155481232 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features_no_predict",
            "value": 3385.36890458327,
            "unit": "ns/iter",
            "extra": "iterations: 1232709\ncpu: 3385.105811671693 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features_no_predic",
            "value": 5090.799522761916,
            "unit": "ns/iter",
            "extra": "iterations: 831870\ncpu: 5090.263382499675 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/few_features",
            "value": 1180.078830627804,
            "unit": "ns/iter",
            "extra": "iterations: 3575628\ncpu: 1179.9885222959438 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/many_features/min_time:15.000",
            "value": 60373.4887493352,
            "unit": "ns/iter",
            "extra": "iterations: 347935\ncpu: 60360.3397186256 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_no_namespaces/min_time:15.000",
            "value": 294020.30975741526,
            "unit": "ns/iter",
            "extra": "iterations: 79601\ncpu: 293993.11189557926 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 317300.54879932216,
            "unit": "ns/iter",
            "extra": "iterations: 66046\ncpu: 317233.0845168519 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_interactions/min_time:15.000",
            "value": 558896.4787888535,
            "unit": "ns/iter",
            "extra": "iterations: 37386\ncpu: 558843.3852244152 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_no_interactions/min_time:15.000",
            "value": 291586.62850590545,
            "unit": "ns/iter",
            "extra": "iterations: 71950\ncpu: 291557.1785962471 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_interactions/min_time:15.000",
            "value": 435906.3092755827,
            "unit": "ns/iter",
            "extra": "iterations: 48439\ncpu: 435817.9194450753 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_no_namespaces/min_time:15.000",
            "value": 610487.1166258233,
            "unit": "ns/iter",
            "extra": "iterations: 33869\ncpu: 610422.3655850482 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 791533.7682903536,
            "unit": "ns/iter",
            "extra": "iterations: 26175\ncpu: 791352.7793696278 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_interactions/min_time:15.000",
            "value": 2522639.9466998954,
            "unit": "ns/iter",
            "extra": "iterations: 8424\ncpu: 2522367.996201335 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_no_interactions/min_time:15.000",
            "value": 637533.7863870232,
            "unit": "ns/iter",
            "extra": "iterations: 33233\ncpu: 637381.401017062 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_interactions/min_time:15.000",
            "value": 1882759.8989013103,
            "unit": "ns/iter",
            "extra": "iterations: 11286\ncpu: 1882472.718412189 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/simple/min_time:15.000",
            "value": 50809.40644219153,
            "unit": "ns/iter",
            "extra": "iterations: 401292\ncpu: 50799.38423890828 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/quadratic/min_time:15.000",
            "value": 2850921.4410677585,
            "unit": "ns/iter",
            "extra": "iterations: 7305\ncpu: 2850652.4161533206 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_string_fts",
            "value": 952.260433265368,
            "unit": "ns/iter",
            "extra": "iterations: 4413000\ncpu: 952.1957625198252 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_string_fts",
            "value": 6979.005007314994,
            "unit": "ns/iter",
            "extra": "iterations: 601520\ncpu: 6978.546515494138 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_num_fts",
            "value": 728.5596177718602,
            "unit": "ns/iter",
            "extra": "iterations: 5833898\ncpu: 728.5119143324064 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_num_fts",
            "value": 5041.584956856422,
            "unit": "ns/iter",
            "extra": "iterations: 824801\ncpu: 5041.206424337509 ns\nthreads: 1"
          },
          {
            "name": "benchmark_example_reuse",
            "value": 818.819958516323,
            "unit": "ns/iter",
            "extra": "iterations: 5131173\ncpu: 818.7533727668101 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_char",
            "value": 52.061212728197695,
            "unit": "ns/iter",
            "extra": "iterations: 80663763\ncpu: 52.05768171266675 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_extent",
            "value": 73.86636103028599,
            "unit": "ns/iter",
            "extra": "iterations: 56972057\ncpu: 73.82716583324398 ns\nthreads: 1"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "byronxu@microsoft.com",
            "name": "Byron Xu",
            "username": "byronxu99"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "2522e9e9731d692751204db1f24aaa49a3a41098",
          "message": "refactor: No RapidJSON in header files (#4219)\n\n* Move implementation of functions using RapidJSON to .cc source files\r\n* Replace the header files with just a declaration of the function\r\n* Explicitly instantiate templates for the two possible values of <bool audit>\r\n* Simplify CMake files now that RapidJSON no longer needs to be exported",
          "timestamp": "2022-10-13T14:26:33Z",
          "tree_id": "68f2d8d29f0cd9419eda06c7a096ff5f3da1466e",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/2522e9e9731d692751204db1f24aaa49a3a41098"
        },
        "date": 1665674037243,
        "tool": "benchmarkdotnet",
        "benches": [
          {
            "name": "BenchmarkText.Benchmark(args: 120_num_features)",
            "value": 6334.383850097656,
            "unit": "ns",
            "range": "± 97.82899935057097"
          },
          {
            "name": "BenchmarkText.Benchmark(args: 120_string_fts)",
            "value": 9379.078020368304,
            "unit": "ns",
            "range": "± 140.22553875220282"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 1_feature)",
            "value": 687.9974047342936,
            "unit": "ns",
            "range": "± 12.876866959603117"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 8_features)",
            "value": 585.7988357543945,
            "unit": "ns",
            "range": "± 10.51262464726024"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_interactions)",
            "value": 1072139.6223958333,
            "unit": "ns",
            "range": "± 16401.38681856113"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_no_interactions)",
            "value": 672118.5872395834,
            "unit": "ns",
            "range": "± 10930.251002053248"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_no_namespaces)",
            "value": 671412.9231770834,
            "unit": "ns",
            "range": "± 11286.85364466076"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_interactions)",
            "value": 1047014.3092105263,
            "unit": "ns",
            "range": "± 22131.884132077557"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_no_interactions)",
            "value": 637283.73046875,
            "unit": "ns",
            "range": "± 7886.28864091683"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_interactions)",
            "value": 6110025,
            "unit": "ns",
            "range": "± 59416.827847878834"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_no_interactions)",
            "value": 1324765.9244791667,
            "unit": "ns",
            "range": "± 22328.872083058774"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_no_namespaces)",
            "value": 1136014.2057291667,
            "unit": "ns",
            "range": "± 20432.511008476376"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_interactions)",
            "value": 3885369.84375,
            "unit": "ns",
            "range": "± 60656.035262999234"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_no_interactions)",
            "value": 1180856.54296875,
            "unit": "ns",
            "range": "± 17591.262144964025"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: few_features)",
            "value": 3801.3957096980166,
            "unit": "ns",
            "range": "± 45.62896670987905"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: few_features)",
            "value": 13667.839813232422,
            "unit": "ns",
            "range": "± 194.4078785398737"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: many_features)",
            "value": 163964.7900390625,
            "unit": "ns",
            "range": "± 2510.715363261866"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: many_features)",
            "value": 23371.137288411457,
            "unit": "ns",
            "range": "± 343.1445591169316"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: quadratic)",
            "value": 6161161.495535715,
            "unit": "ns",
            "range": "± 64387.268697956046"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: simple)",
            "value": 278249.6895926339,
            "unit": "ns",
            "range": "± 4624.590525269857"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "griffinbassman@gmail.com",
            "name": "Griffin Bassman",
            "username": "bassmang"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "a853d1f4e60d38ef331924fa09fbb13f0b40c2ba",
          "message": "refactor: remove empty public: (#4220)\n\n* refactor: remove empty public:\r\n\r\n* clang\r\n\r\nCo-authored-by: Griffin Bassman <griffinbassman@MacBook-Pro.local>",
          "timestamp": "2022-10-13T12:42:47-04:00",
          "tree_id": "4243bad7aa01e19a88e41c39f965a78504ff197d",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/a853d1f4e60d38ef331924fa09fbb13f0b40c2ba"
        },
        "date": 1665681176537,
        "tool": "googlecpp",
        "benches": [
          {
            "name": "bench_text/120_string_fts",
            "value": 7029.805947680083,
            "unit": "ns/iter",
            "extra": "iterations: 588532\ncpu: 7028.915674933564 ns\nthreads: 1"
          },
          {
            "name": "bench_text/120_num_fts",
            "value": 5296.613958609961,
            "unit": "ns/iter",
            "extra": "iterations: 791884\ncpu: 5295.892453945276 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/8_features",
            "value": 90.71871780828413,
            "unit": "ns/iter",
            "extra": "iterations: 46529828\ncpu: 90.70745544127095 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/1_feature",
            "value": 205.4101067939806,
            "unit": "ns/iter",
            "extra": "iterations: 20681969\ncpu: 205.38104471581022 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features",
            "value": 7961.933971245059,
            "unit": "ns/iter",
            "extra": "iterations: 530481\ncpu: 7960.711128202521 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features",
            "value": 14795.937669094816,
            "unit": "ns/iter",
            "extra": "iterations: 288669\ncpu: 14792.919225826117 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features_no_predict",
            "value": 3910.703254279203,
            "unit": "ns/iter",
            "extra": "iterations: 1092838\ncpu: 3910.1646355635494 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features_no_predic",
            "value": 5828.597802153809,
            "unit": "ns/iter",
            "extra": "iterations: 701687\ncpu: 5827.796866694121 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/few_features",
            "value": 1370.5621476306953,
            "unit": "ns/iter",
            "extra": "iterations: 3031266\ncpu: 1370.3827707631087 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/many_features/min_time:15.000",
            "value": 64851.81441678604,
            "unit": "ns/iter",
            "extra": "iterations: 325121\ncpu: 64829.16790979357 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_no_namespaces/min_time:15.000",
            "value": 314792.8127252168,
            "unit": "ns/iter",
            "extra": "iterations: 66325\ncpu: 314741.91632114584 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 355814.8044383051,
            "unit": "ns/iter",
            "extra": "iterations: 60068\ncpu: 355712.52413930884 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_interactions/min_time:15.000",
            "value": 643880.3797733667,
            "unit": "ns/iter",
            "extra": "iterations: 32917\ncpu: 643777.6680742471 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_no_interactions/min_time:15.000",
            "value": 326386.3036755869,
            "unit": "ns/iter",
            "extra": "iterations: 64289\ncpu: 326290.44626607985 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_interactions/min_time:15.000",
            "value": 497267.00635297043,
            "unit": "ns/iter",
            "extra": "iterations: 42185\ncpu: 497163.66006874543 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_no_namespaces/min_time:15.000",
            "value": 692428.0145854763,
            "unit": "ns/iter",
            "extra": "iterations: 30167\ncpu: 692226.5323035107 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 889869.6346736032,
            "unit": "ns/iter",
            "extra": "iterations: 23453\ncpu: 889728.3929561244 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_interactions/min_time:15.000",
            "value": 2925031.0008332427,
            "unit": "ns/iter",
            "extra": "iterations: 7201\ncpu: 2924504.5826968444 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_no_interactions/min_time:15.000",
            "value": 718225.4561860218,
            "unit": "ns/iter",
            "extra": "iterations: 28427\ncpu: 718106.8702290065 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_interactions/min_time:15.000",
            "value": 2162741.383003122,
            "unit": "ns/iter",
            "extra": "iterations: 9590\ncpu: 2162416.8196037537 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/simple/min_time:15.000",
            "value": 58518.56597539906,
            "unit": "ns/iter",
            "extra": "iterations: 359581\ncpu: 58504.24021291445 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/quadratic/min_time:15.000",
            "value": 3076591.4951800397,
            "unit": "ns/iter",
            "extra": "iterations: 7054\ncpu: 3076148.8800680474 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_string_fts",
            "value": 1121.6663845402904,
            "unit": "ns/iter",
            "extra": "iterations: 3708740\ncpu: 1121.5258551421803 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_string_fts",
            "value": 8137.221551664421,
            "unit": "ns/iter",
            "extra": "iterations: 503395\ncpu: 8136.266748775801 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_num_fts",
            "value": 831.858402170481,
            "unit": "ns/iter",
            "extra": "iterations: 5047422\ncpu: 831.3419603116241 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_num_fts",
            "value": 5760.097079540493,
            "unit": "ns/iter",
            "extra": "iterations: 715949\ncpu: 5759.435378777004 ns\nthreads: 1"
          },
          {
            "name": "benchmark_example_reuse",
            "value": 929.488393311395,
            "unit": "ns/iter",
            "extra": "iterations: 4724948\ncpu: 929.3838366051817 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_char",
            "value": 59.61829995183764,
            "unit": "ns/iter",
            "extra": "iterations: 73442084\ncpu: 59.61173296770897 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_extent",
            "value": 82.0153883831031,
            "unit": "ns/iter",
            "extra": "iterations: 51318582\ncpu: 82.00615714596294 ns\nthreads: 1"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "griffinbassman@gmail.com",
            "name": "Griffin Bassman",
            "username": "bassmang"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "a853d1f4e60d38ef331924fa09fbb13f0b40c2ba",
          "message": "refactor: remove empty public: (#4220)\n\n* refactor: remove empty public:\r\n\r\n* clang\r\n\r\nCo-authored-by: Griffin Bassman <griffinbassman@MacBook-Pro.local>",
          "timestamp": "2022-10-13T12:42:47-04:00",
          "tree_id": "4243bad7aa01e19a88e41c39f965a78504ff197d",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/a853d1f4e60d38ef331924fa09fbb13f0b40c2ba"
        },
        "date": 1665682170833,
        "tool": "benchmarkdotnet",
        "benches": [
          {
            "name": "BenchmarkText.Benchmark(args: 120_num_features)",
            "value": 7409.068764580621,
            "unit": "ns",
            "range": "± 242.02249252354682"
          },
          {
            "name": "BenchmarkText.Benchmark(args: 120_string_fts)",
            "value": 12208.455027704653,
            "unit": "ns",
            "range": "± 295.23878679385166"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 1_feature)",
            "value": 728.9236409323556,
            "unit": "ns",
            "range": "± 5.171214741887778"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 8_features)",
            "value": 633.0716995965867,
            "unit": "ns",
            "range": "± 15.022082012360311"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_interactions)",
            "value": 1097857.8369140625,
            "unit": "ns",
            "range": "± 33198.24350934291"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_no_interactions)",
            "value": 742636.0871550324,
            "unit": "ns",
            "range": "± 38042.14758918636"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_no_namespaces)",
            "value": 681011.6471354166,
            "unit": "ns",
            "range": "± 20233.8705547462"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_interactions)",
            "value": 1107955.17578125,
            "unit": "ns",
            "range": "± 35248.23425810007"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_no_interactions)",
            "value": 693809.0787760416,
            "unit": "ns",
            "range": "± 20315.939057143198"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_interactions)",
            "value": 4468949.259868421,
            "unit": "ns",
            "range": "± 98069.45528792587"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_no_interactions)",
            "value": 1347008.9613970588,
            "unit": "ns",
            "range": "± 27295.25229326106"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_no_namespaces)",
            "value": 1111304.4150904606,
            "unit": "ns",
            "range": "± 24524.21497639209"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_interactions)",
            "value": 3710901.8323863638,
            "unit": "ns",
            "range": "± 156576.26928523026"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_no_interactions)",
            "value": 1129867.7835398708,
            "unit": "ns",
            "range": "± 32124.71116473504"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: few_features)",
            "value": 3736.297403971354,
            "unit": "ns",
            "range": "± 34.2557698988403"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: few_features)",
            "value": 13493.430189652876,
            "unit": "ns",
            "range": "± 326.5549986342138"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: many_features)",
            "value": 135139.04832409273,
            "unit": "ns",
            "range": "± 4046.467722228201"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: many_features)",
            "value": 23473.1684366862,
            "unit": "ns",
            "range": "± 588.6962877017434"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: quadratic)",
            "value": 5117979.8828125,
            "unit": "ns",
            "range": "± 62720.179340096416"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: simple)",
            "value": 249176.62071814903,
            "unit": "ns",
            "range": "± 3117.880666948926"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "griffinbassman@gmail.com",
            "name": "Griffin Bassman",
            "username": "bassmang"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "1c5cefb6197065dc98dd523d6ed34c22a0e5b0ed",
          "message": "fix: only remove ksvm dump_weights (#4195)\n\n* fix: only remove ksvm dump_weights\r\n\r\n* typo",
          "timestamp": "2022-10-13T14:07:48-04:00",
          "tree_id": "a6ce6f28f5ee04215181ce55bca2bfb8a69a36f3",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/1c5cefb6197065dc98dd523d6ed34c22a0e5b0ed"
        },
        "date": 1665685657393,
        "tool": "googlecpp",
        "benches": [
          {
            "name": "bench_text/120_string_fts",
            "value": 5357.3740592669465,
            "unit": "ns/iter",
            "extra": "iterations: 785159\ncpu: 5356.6906830336275 ns\nthreads: 1"
          },
          {
            "name": "bench_text/120_num_fts",
            "value": 4153.357580726046,
            "unit": "ns/iter",
            "extra": "iterations: 1006335\ncpu: 4151.142412814817 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/8_features",
            "value": 68.7618680696562,
            "unit": "ns/iter",
            "extra": "iterations: 59263850\ncpu: 68.75617935723041 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/1_feature",
            "value": 162.59622613686537,
            "unit": "ns/iter",
            "extra": "iterations: 26311765\ncpu: 162.5898718691049 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features",
            "value": 6854.753140181021,
            "unit": "ns/iter",
            "extra": "iterations: 611987\ncpu: 6854.365370506234 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features",
            "value": 12332.36542303961,
            "unit": "ns/iter",
            "extra": "iterations: 339330\ncpu: 12331.817110187718 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features_no_predict",
            "value": 3637.885550546074,
            "unit": "ns/iter",
            "extra": "iterations: 1152701\ncpu: 3637.725047518824 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features_no_predic",
            "value": 5308.919045964584,
            "unit": "ns/iter",
            "extra": "iterations: 782864\ncpu: 5308.723354247989 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/few_features",
            "value": 1159.6554784373677,
            "unit": "ns/iter",
            "extra": "iterations: 3612636\ncpu: 1159.5962338857266 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/many_features/min_time:15.000",
            "value": 52694.749489499576,
            "unit": "ns/iter",
            "extra": "iterations: 399608\ncpu: 52692.83397729776 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_no_namespaces/min_time:15.000",
            "value": 187485.4301885125,
            "unit": "ns/iter",
            "extra": "iterations: 112725\ncpu: 187477.33688179203 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 213307.75415402316,
            "unit": "ns/iter",
            "extra": "iterations: 96894\ncpu: 213300.56556649538 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_interactions/min_time:15.000",
            "value": 446585.96364756516,
            "unit": "ns/iter",
            "extra": "iterations: 47067\ncpu: 446558.13202456065 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_no_interactions/min_time:15.000",
            "value": 187091.89137311358,
            "unit": "ns/iter",
            "extra": "iterations: 111593\ncpu: 187084.5814701638 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_interactions/min_time:15.000",
            "value": 325249.55429853615,
            "unit": "ns/iter",
            "extra": "iterations: 64615\ncpu: 325236.58438443055 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_no_namespaces/min_time:15.000",
            "value": 546855.0461088396,
            "unit": "ns/iter",
            "extra": "iterations: 38626\ncpu: 546826.8679128054 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 689643.6782924009,
            "unit": "ns/iter",
            "extra": "iterations: 30335\ncpu: 689608.2511949885 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_interactions/min_time:15.000",
            "value": 2124854.8434214527,
            "unit": "ns/iter",
            "extra": "iterations: 9797\ncpu: 2124445.738491373 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_no_interactions/min_time:15.000",
            "value": 538682.4757835501,
            "unit": "ns/iter",
            "extra": "iterations: 38734\ncpu: 538486.5983373784 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_interactions/min_time:15.000",
            "value": 1619535.7089443358,
            "unit": "ns/iter",
            "extra": "iterations: 13025\ncpu: 1619393.1055662192 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/simple/min_time:15.000",
            "value": 47354.27538270058,
            "unit": "ns/iter",
            "extra": "iterations: 444407\ncpu: 47352.61955819783 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/quadratic/min_time:15.000",
            "value": 2218733.2573843985,
            "unit": "ns/iter",
            "extra": "iterations: 9344\ncpu: 2218653.767123285 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_string_fts",
            "value": 900.2708214307604,
            "unit": "ns/iter",
            "extra": "iterations: 4687358\ncpu: 900.2441247286832 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_string_fts",
            "value": 7351.596434580449,
            "unit": "ns/iter",
            "extra": "iterations: 572948\ncpu: 7351.328916411238 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_num_fts",
            "value": 686.4131673423382,
            "unit": "ns/iter",
            "extra": "iterations: 6129088\ncpu: 686.3882685319597 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_num_fts",
            "value": 5266.961554687919,
            "unit": "ns/iter",
            "extra": "iterations: 796430\ncpu: 5266.755019273534 ns\nthreads: 1"
          },
          {
            "name": "benchmark_example_reuse",
            "value": 791.4821036565153,
            "unit": "ns/iter",
            "extra": "iterations: 5305581\ncpu: 791.4560158444505 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_char",
            "value": 41.84927755200447,
            "unit": "ns/iter",
            "extra": "iterations: 100271162\ncpu: 41.84751643747807 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_extent",
            "value": 54.032370970885,
            "unit": "ns/iter",
            "extra": "iterations: 78081254\ncpu: 54.03072804133013 ns\nthreads: 1"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "griffinbassman@gmail.com",
            "name": "Griffin Bassman",
            "username": "bassmang"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "1c5cefb6197065dc98dd523d6ed34c22a0e5b0ed",
          "message": "fix: only remove ksvm dump_weights (#4195)\n\n* fix: only remove ksvm dump_weights\r\n\r\n* typo",
          "timestamp": "2022-10-13T14:07:48-04:00",
          "tree_id": "a6ce6f28f5ee04215181ce55bca2bfb8a69a36f3",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/1c5cefb6197065dc98dd523d6ed34c22a0e5b0ed"
        },
        "date": 1665687429727,
        "tool": "benchmarkdotnet",
        "benches": [
          {
            "name": "BenchmarkText.Benchmark(args: 120_num_features)",
            "value": 5650.521194458008,
            "unit": "ns",
            "range": "± 138.61251541893859"
          },
          {
            "name": "BenchmarkText.Benchmark(args: 120_string_fts)",
            "value": 8260.01481276292,
            "unit": "ns",
            "range": "± 89.23106134684548"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 1_feature)",
            "value": 755.7214609781901,
            "unit": "ns",
            "range": "± 13.438396793372364"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 8_features)",
            "value": 564.1310316143614,
            "unit": "ns",
            "range": "± 17.858091356795132"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_interactions)",
            "value": 970730.3125,
            "unit": "ns",
            "range": "± 17042.659365630476"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_no_interactions)",
            "value": 568152.4739583334,
            "unit": "ns",
            "range": "± 9108.959536037255"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_no_namespaces)",
            "value": 622089.8615056818,
            "unit": "ns",
            "range": "± 25901.59175718029"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_interactions)",
            "value": 921142.7358774039,
            "unit": "ns",
            "range": "± 5181.194279434134"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_no_interactions)",
            "value": 541020.4361979166,
            "unit": "ns",
            "range": "± 7973.766749640102"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_interactions)",
            "value": 4326597.935267857,
            "unit": "ns",
            "range": "± 36217.43491465879"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_no_interactions)",
            "value": 1189925.5859375,
            "unit": "ns",
            "range": "± 21056.83963048584"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_no_namespaces)",
            "value": 1041276.6967773438,
            "unit": "ns",
            "range": "± 51367.94108915475"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_interactions)",
            "value": 3685783.6197916665,
            "unit": "ns",
            "range": "± 61587.33652071511"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_no_interactions)",
            "value": 1067682.5927734375,
            "unit": "ns",
            "range": "± 27611.35031115615"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: few_features)",
            "value": 3815.0127156575522,
            "unit": "ns",
            "range": "± 50.42647496293263"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: few_features)",
            "value": 12990.885598318917,
            "unit": "ns",
            "range": "± 167.37822431453634"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: many_features)",
            "value": 153257.0174153646,
            "unit": "ns",
            "range": "± 2701.728956694281"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: many_features)",
            "value": 22796.81854248047,
            "unit": "ns",
            "range": "± 374.8523710028905"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: quadratic)",
            "value": 5746787.79296875,
            "unit": "ns",
            "range": "± 331323.0874256805"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: simple)",
            "value": 244474.908203125,
            "unit": "ns",
            "range": "± 6299.626450753039"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "jacob.alber@microsoft.com",
            "name": "Jacob Alber",
            "username": "lokitoth"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "28129aff63512771956525a524a327b92a5a396d",
          "message": "fix: Add native runtime dependencies to nuspec (#4216)\n\nAdd native runtime dependencies to nuspec\r\n\r\nThis removes the need to manually reference the native runtime NuGets",
          "timestamp": "2022-10-13T15:27:05-04:00",
          "tree_id": "386ba469b4a1a1e5cd0817d6ebcd6e362b58e688",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/28129aff63512771956525a524a327b92a5a396d"
        },
        "date": 1665690603091,
        "tool": "googlecpp",
        "benches": [
          {
            "name": "bench_text/120_string_fts",
            "value": 7031.089877337865,
            "unit": "ns/iter",
            "extra": "iterations: 595456\ncpu: 7029.238264456147 ns\nthreads: 1"
          },
          {
            "name": "bench_text/120_num_fts",
            "value": 5285.226927013308,
            "unit": "ns/iter",
            "extra": "iterations: 801240\ncpu: 5284.8399980030945 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/8_features",
            "value": 85.18501029951831,
            "unit": "ns/iter",
            "extra": "iterations: 50148954\ncpu: 85.17825715766674 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/1_feature",
            "value": 205.99206865678542,
            "unit": "ns/iter",
            "extra": "iterations: 21204605\ncpu: 205.97510776550664 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features",
            "value": 8458.584816636303,
            "unit": "ns/iter",
            "extra": "iterations: 499336\ncpu: 8457.569051700655 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features",
            "value": 14924.230268502728,
            "unit": "ns/iter",
            "extra": "iterations: 281897\ncpu: 14922.466361827188 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features_no_predict",
            "value": 3845.039058211123,
            "unit": "ns/iter",
            "extra": "iterations: 1082026\ncpu: 3844.6932883313343 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features_no_predic",
            "value": 5934.864180521684,
            "unit": "ns/iter",
            "extra": "iterations: 719293\ncpu: 5934.404477730213 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/few_features",
            "value": 1414.747067199089,
            "unit": "ns/iter",
            "extra": "iterations: 2981365\ncpu: 1414.6325927888724 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/many_features/min_time:15.000",
            "value": 72165.66397470102,
            "unit": "ns/iter",
            "extra": "iterations: 291237\ncpu: 72157.70180299893 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_no_namespaces/min_time:15.000",
            "value": 329628.6853024205,
            "unit": "ns/iter",
            "extra": "iterations: 67290\ncpu: 329550.8500520137 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 355646.7743215627,
            "unit": "ns/iter",
            "extra": "iterations: 59217\ncpu: 355608.0787611666 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_interactions/min_time:15.000",
            "value": 643419.6420359227,
            "unit": "ns/iter",
            "extra": "iterations: 32791\ncpu: 643219.5206001643 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_no_interactions/min_time:15.000",
            "value": 323265.1186759417,
            "unit": "ns/iter",
            "extra": "iterations: 65405\ncpu: 323232.39507682936 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_interactions/min_time:15.000",
            "value": 493122.12726679,
            "unit": "ns/iter",
            "extra": "iterations: 42847\ncpu: 493004.75879291433 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_no_namespaces/min_time:15.000",
            "value": 703619.8243144286,
            "unit": "ns/iter",
            "extra": "iterations: 30048\ncpu: 703535.2036741218 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 908617.2897015112,
            "unit": "ns/iter",
            "extra": "iterations: 23217\ncpu: 908396.3130464754 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_interactions/min_time:15.000",
            "value": 2897945.7185489205,
            "unit": "ns/iter",
            "extra": "iterations: 7305\ncpu: 2897608.596851466 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_no_interactions/min_time:15.000",
            "value": 748263.7314481456,
            "unit": "ns/iter",
            "extra": "iterations: 28609\ncpu: 748081.6351497774 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_interactions/min_time:15.000",
            "value": 2303986.7945070704,
            "unit": "ns/iter",
            "extra": "iterations: 9139\ncpu: 2303725.265346322 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/simple/min_time:15.000",
            "value": 61201.39471793307,
            "unit": "ns/iter",
            "extra": "iterations: 335399\ncpu: 61195.63296253121 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/quadratic/min_time:15.000",
            "value": 3158118.8356979364,
            "unit": "ns/iter",
            "extra": "iterations: 6555\ncpu: 3157312.814645311 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_string_fts",
            "value": 1129.423790012129,
            "unit": "ns/iter",
            "extra": "iterations: 3704252\ncpu: 1129.3387706883825 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_string_fts",
            "value": 8996.665835496839,
            "unit": "ns/iter",
            "extra": "iterations: 467614\ncpu: 8995.932756504295 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_num_fts",
            "value": 843.4823022031225,
            "unit": "ns/iter",
            "extra": "iterations: 4672672\ncpu: 843.4133403756922 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_num_fts",
            "value": 6462.241027820584,
            "unit": "ns/iter",
            "extra": "iterations: 656418\ncpu: 6461.736424046829 ns\nthreads: 1"
          },
          {
            "name": "benchmark_example_reuse",
            "value": 959.5860049713924,
            "unit": "ns/iter",
            "extra": "iterations: 4443435\ncpu: 959.4845879370365 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_char",
            "value": 63.117133851116435,
            "unit": "ns/iter",
            "extra": "iterations: 66980219\ncpu: 63.11180917458629 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_extent",
            "value": 88.27381204688994,
            "unit": "ns/iter",
            "extra": "iterations: 47650555\ncpu: 88.2666613222039 ns\nthreads: 1"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "jacob.alber@microsoft.com",
            "name": "Jacob Alber",
            "username": "lokitoth"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "28129aff63512771956525a524a327b92a5a396d",
          "message": "fix: Add native runtime dependencies to nuspec (#4216)\n\nAdd native runtime dependencies to nuspec\r\n\r\nThis removes the need to manually reference the native runtime NuGets",
          "timestamp": "2022-10-13T15:27:05-04:00",
          "tree_id": "386ba469b4a1a1e5cd0817d6ebcd6e362b58e688",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/28129aff63512771956525a524a327b92a5a396d"
        },
        "date": 1665691690133,
        "tool": "benchmarkdotnet",
        "benches": [
          {
            "name": "BenchmarkText.Benchmark(args: 120_num_features)",
            "value": 5971.330668131511,
            "unit": "ns",
            "range": "± 98.61456965524528"
          },
          {
            "name": "BenchmarkText.Benchmark(args: 120_string_fts)",
            "value": 9561.168009440104,
            "unit": "ns",
            "range": "± 50.514681206448145"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 1_feature)",
            "value": 611.692860921224,
            "unit": "ns",
            "range": "± 5.069281754972343"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 8_features)",
            "value": 505.111083984375,
            "unit": "ns",
            "range": "± 1.565623105320085"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_interactions)",
            "value": 913716.2337239584,
            "unit": "ns",
            "range": "± 6305.640954410755"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_no_interactions)",
            "value": 557014.0852864584,
            "unit": "ns",
            "range": "± 2039.642112787079"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_no_namespaces)",
            "value": 565479.9609375,
            "unit": "ns",
            "range": "± 3180.996552509549"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_interactions)",
            "value": 892129.8665364584,
            "unit": "ns",
            "range": "± 4001.2378050693965"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_no_interactions)",
            "value": 564680.7747395834,
            "unit": "ns",
            "range": "± 2098.880928603711"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_interactions)",
            "value": 3508592.4739583335,
            "unit": "ns",
            "range": "± 8651.052853438903"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_no_interactions)",
            "value": 1060946.7447916667,
            "unit": "ns",
            "range": "± 1442.408616540834"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_no_namespaces)",
            "value": 897273.9192708334,
            "unit": "ns",
            "range": "± 1141.8917599587812"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_interactions)",
            "value": 2781006.4603365385,
            "unit": "ns",
            "range": "± 3408.039165317241"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_no_interactions)",
            "value": 928194.7126116072,
            "unit": "ns",
            "range": "± 1766.2695755734892"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: few_features)",
            "value": 3155.0462086995444,
            "unit": "ns",
            "range": "± 4.446601951633358"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: few_features)",
            "value": 11278.170340401786,
            "unit": "ns",
            "range": "± 17.391332644157274"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: many_features)",
            "value": 108026.24918619792,
            "unit": "ns",
            "range": "± 120.43433961797125"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: many_features)",
            "value": 18772.722516741072,
            "unit": "ns",
            "range": "± 30.963015248244993"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: quadratic)",
            "value": 4290644.21875,
            "unit": "ns",
            "range": "± 29978.606114777267"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: simple)",
            "value": 203094.58984375,
            "unit": "ns",
            "range": "± 1495.7817157981963"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "griffinbassman@gmail.com",
            "name": "Griffin Bassman",
            "username": "bassmang"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "9496a6dd5610910a495ca004a93c8ab6913293e4",
          "message": "chore: Update Version to 9.5.0 (#4221)\n\n* chore: Update Version to 9.5.0\r\n\r\n* change test files",
          "timestamp": "2022-10-14T06:36:33-04:00",
          "tree_id": "af95d1857805d0fca4b93a0f98fd65be920d838b",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/9496a6dd5610910a495ca004a93c8ab6913293e4"
        },
        "date": 1665745289057,
        "tool": "googlecpp",
        "benches": [
          {
            "name": "bench_text/120_string_fts",
            "value": 6799.300507251791,
            "unit": "ns/iter",
            "extra": "iterations: 577031\ncpu: 6798.457795161785 ns\nthreads: 1"
          },
          {
            "name": "bench_text/120_num_fts",
            "value": 4947.577966362003,
            "unit": "ns/iter",
            "extra": "iterations: 853259\ncpu: 4946.807710202881 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/8_features",
            "value": 86.61215737656306,
            "unit": "ns/iter",
            "extra": "iterations: 48686869\ncpu: 86.58972299081297 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/1_feature",
            "value": 198.31310974521185,
            "unit": "ns/iter",
            "extra": "iterations: 20960804\ncpu: 198.29070010864095 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features",
            "value": 8146.328729957753,
            "unit": "ns/iter",
            "extra": "iterations: 523022\ncpu: 8145.583742175277 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features",
            "value": 15325.466758033594,
            "unit": "ns/iter",
            "extra": "iterations: 271434\ncpu: 15324.407406588709 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features_no_predict",
            "value": 3999.2547887289825,
            "unit": "ns/iter",
            "extra": "iterations: 1079514\ncpu: 3998.888481298068 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features_no_predic",
            "value": 6290.077456039714,
            "unit": "ns/iter",
            "extra": "iterations: 701882\ncpu: 6281.245565493918 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/few_features",
            "value": 1501.030693512757,
            "unit": "ns/iter",
            "extra": "iterations: 2757195\ncpu: 1500.9077341283453 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/many_features/min_time:15.000",
            "value": 103241.56251626018,
            "unit": "ns/iter",
            "extra": "iterations: 196037\ncpu: 103231.81389227544 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_no_namespaces/min_time:15.000",
            "value": 352866.8190940505,
            "unit": "ns/iter",
            "extra": "iterations: 59893\ncpu: 352839.13812966476 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 370771.1370769245,
            "unit": "ns/iter",
            "extra": "iterations: 58500\ncpu: 370675.57264957245 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_interactions/min_time:15.000",
            "value": 635521.1668101619,
            "unit": "ns/iter",
            "extra": "iterations: 32522\ncpu: 635344.3791894716 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_no_interactions/min_time:15.000",
            "value": 331107.5034588537,
            "unit": "ns/iter",
            "extra": "iterations: 62593\ncpu: 331000.88029012847 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_interactions/min_time:15.000",
            "value": 503501.3068640999,
            "unit": "ns/iter",
            "extra": "iterations: 41331\ncpu: 503462.01640415203 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_no_namespaces/min_time:15.000",
            "value": 725085.9515273146,
            "unit": "ns/iter",
            "extra": "iterations: 27892\ncpu: 724908.5329126632 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 925042.8793703255,
            "unit": "ns/iter",
            "extra": "iterations: 22996\ncpu: 924924.425987127 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_interactions/min_time:15.000",
            "value": 3011259.2129774387,
            "unit": "ns/iter",
            "extra": "iterations: 7043\ncpu: 3010476.7996592387 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_no_interactions/min_time:15.000",
            "value": 775309.911163901,
            "unit": "ns/iter",
            "extra": "iterations: 27365\ncpu: 775246.31098118 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_interactions/min_time:15.000",
            "value": 2370449.7923042094,
            "unit": "ns/iter",
            "extra": "iterations: 8888\ncpu: 2370229.556705671 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/simple/min_time:15.000",
            "value": 82745.49861623633,
            "unit": "ns/iter",
            "extra": "iterations: 253656\ncpu: 82738.47927902362 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/quadratic/min_time:15.000",
            "value": 2964927.2759161647,
            "unit": "ns/iter",
            "extra": "iterations: 7013\ncpu: 2964668.7009838833 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_string_fts",
            "value": 1279.3126986552609,
            "unit": "ns/iter",
            "extra": "iterations: 3619021\ncpu: 1278.5898451542614 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_string_fts",
            "value": 8285.779748651998,
            "unit": "ns/iter",
            "extra": "iterations: 487770\ncpu: 8285.027369456893 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_num_fts",
            "value": 862.1882717055067,
            "unit": "ns/iter",
            "extra": "iterations: 4880505\ncpu: 862.1185717461661 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_num_fts",
            "value": 5753.025044933634,
            "unit": "ns/iter",
            "extra": "iterations: 721064\ncpu: 5752.4875739185545 ns\nthreads: 1"
          },
          {
            "name": "benchmark_example_reuse",
            "value": 1011.653446882809,
            "unit": "ns/iter",
            "extra": "iterations: 4238975\ncpu: 1011.5810543822427 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_char",
            "value": 54.96357409337023,
            "unit": "ns/iter",
            "extra": "iterations: 76173231\ncpu: 54.9594594457989 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_extent",
            "value": 78.01182556728529,
            "unit": "ns/iter",
            "extra": "iterations: 53889846\ncpu: 78.0059438284527 ns\nthreads: 1"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "jackgerrits@users.noreply.github.com",
            "name": "Jack Gerrits",
            "username": "jackgerrits"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "62b98239c32c5e44d403eaf5b10e60ab9cd223a4",
          "message": "build: remove FORCE_COLORED_OUTPUT (#4213)",
          "timestamp": "2022-10-14T09:43:32-04:00",
          "tree_id": "f26cb28630b16fb5e2b5700d9d708a42b4286694",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/62b98239c32c5e44d403eaf5b10e60ab9cd223a4"
        },
        "date": 1665756227512,
        "tool": "googlecpp",
        "benches": [
          {
            "name": "bench_text/120_string_fts",
            "value": 6029.274372276598,
            "unit": "ns/iter",
            "extra": "iterations: 696047\ncpu: 6028.856959371995 ns\nthreads: 1"
          },
          {
            "name": "bench_text/120_num_fts",
            "value": 4557.863703057873,
            "unit": "ns/iter",
            "extra": "iterations: 922119\ncpu: 4557.604278840367 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/8_features",
            "value": 72.17614795526298,
            "unit": "ns/iter",
            "extra": "iterations: 57380263\ncpu: 72.17097802427291 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/1_feature",
            "value": 172.06109258616382,
            "unit": "ns/iter",
            "extra": "iterations: 24297269\ncpu: 172.0519001538815 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features",
            "value": 7169.879166356496,
            "unit": "ns/iter",
            "extra": "iterations: 591044\ncpu: 7168.190354694405 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features",
            "value": 12649.06688551017,
            "unit": "ns/iter",
            "extra": "iterations: 330221\ncpu: 12648.082042026403 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features_no_predict",
            "value": 3307.383704752305,
            "unit": "ns/iter",
            "extra": "iterations: 1275783\ncpu: 3307.1846074136442 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features_no_predic",
            "value": 5011.511778256396,
            "unit": "ns/iter",
            "extra": "iterations: 839131\ncpu: 5011.162500253235 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/few_features",
            "value": 1220.5162613305213,
            "unit": "ns/iter",
            "extra": "iterations: 3497961\ncpu: 1220.4315885740295 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/many_features/min_time:15.000",
            "value": 63614.97360503927,
            "unit": "ns/iter",
            "extra": "iterations: 330063\ncpu: 63608.8486137495 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_no_namespaces/min_time:15.000",
            "value": 293516.13493385667,
            "unit": "ns/iter",
            "extra": "iterations: 71435\ncpu: 293479.2944634981 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 319405.9125618767,
            "unit": "ns/iter",
            "extra": "iterations: 65452\ncpu: 319370.66247020726 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_interactions/min_time:15.000",
            "value": 568622.2547075801,
            "unit": "ns/iter",
            "extra": "iterations: 37121\ncpu: 568557.2964090407 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_no_interactions/min_time:15.000",
            "value": 292067.29493240576,
            "unit": "ns/iter",
            "extra": "iterations: 72342\ncpu: 292035.2810262367 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_interactions/min_time:15.000",
            "value": 440539.5102271759,
            "unit": "ns/iter",
            "extra": "iterations: 47716\ncpu: 440493.4382597028 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_no_namespaces/min_time:15.000",
            "value": 610420.8519032341,
            "unit": "ns/iter",
            "extra": "iterations: 34599\ncpu: 610346.8770773719 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 783192.9053689565,
            "unit": "ns/iter",
            "extra": "iterations: 27063\ncpu: 783099.9926098364 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_interactions/min_time:15.000",
            "value": 2464894.453807237,
            "unit": "ns/iter",
            "extra": "iterations: 8497\ncpu: 2464620.7131928927 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_no_interactions/min_time:15.000",
            "value": 623496.8216203406,
            "unit": "ns/iter",
            "extra": "iterations: 33894\ncpu: 623432.5898389096 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_interactions/min_time:15.000",
            "value": 1878223.7658272842,
            "unit": "ns/iter",
            "extra": "iterations: 11278\ncpu: 1878028.8703670881 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/simple/min_time:15.000",
            "value": 50655.1891338551,
            "unit": "ns/iter",
            "extra": "iterations: 417609\ncpu: 50650.95148811455 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/quadratic/min_time:15.000",
            "value": 2953619.177828122,
            "unit": "ns/iter",
            "extra": "iterations: 7063\ncpu: 2953332.2950587533 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_string_fts",
            "value": 956.8706630829827,
            "unit": "ns/iter",
            "extra": "iterations: 4394337\ncpu: 956.5718104915485 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_string_fts",
            "value": 7727.836670856823,
            "unit": "ns/iter",
            "extra": "iterations: 542518\ncpu: 7727.348585669037 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_num_fts",
            "value": 726.1089682583009,
            "unit": "ns/iter",
            "extra": "iterations: 5378759\ncpu: 725.8448835502795 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_num_fts",
            "value": 5455.476590410174,
            "unit": "ns/iter",
            "extra": "iterations: 769770\ncpu: 5455.066188601784 ns\nthreads: 1"
          },
          {
            "name": "benchmark_example_reuse",
            "value": 811.1765617389831,
            "unit": "ns/iter",
            "extra": "iterations: 5176297\ncpu: 811.1122680943523 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_char",
            "value": 53.33852051875454,
            "unit": "ns/iter",
            "extra": "iterations: 78718862\ncpu: 53.33486274229931 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_extent",
            "value": 74.21978566810074,
            "unit": "ns/iter",
            "extra": "iterations: 56591670\ncpu: 74.21450011989421 ns\nthreads: 1"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "jackgerrits@users.noreply.github.com",
            "name": "Jack Gerrits",
            "username": "jackgerrits"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "62b98239c32c5e44d403eaf5b10e60ab9cd223a4",
          "message": "build: remove FORCE_COLORED_OUTPUT (#4213)",
          "timestamp": "2022-10-14T09:43:32-04:00",
          "tree_id": "f26cb28630b16fb5e2b5700d9d708a42b4286694",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/62b98239c32c5e44d403eaf5b10e60ab9cd223a4"
        },
        "date": 1665757721714,
        "tool": "benchmarkdotnet",
        "benches": [
          {
            "name": "BenchmarkText.Benchmark(args: 120_num_features)",
            "value": 7520.544942220052,
            "unit": "ns",
            "range": "± 128.5977450604212"
          },
          {
            "name": "BenchmarkText.Benchmark(args: 120_string_fts)",
            "value": 12002.05820719401,
            "unit": "ns",
            "range": "± 200.041175616698"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 1_feature)",
            "value": 756.6520827157157,
            "unit": "ns",
            "range": "± 7.3832775949338085"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 8_features)",
            "value": 618.437582651774,
            "unit": "ns",
            "range": "± 9.04291613932744"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_interactions)",
            "value": 1102051.6479492188,
            "unit": "ns",
            "range": "± 21327.182961725866"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_no_interactions)",
            "value": 663111.9201660156,
            "unit": "ns",
            "range": "± 12695.787713435022"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_no_namespaces)",
            "value": 674590.4817708334,
            "unit": "ns",
            "range": "± 10579.097946436883"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_interactions)",
            "value": 1088707.9380580357,
            "unit": "ns",
            "range": "± 17058.50984871094"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_no_interactions)",
            "value": 680405.0611413043,
            "unit": "ns",
            "range": "± 16689.019963278115"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_interactions)",
            "value": 4591156.380208333,
            "unit": "ns",
            "range": "± 60305.922746405544"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_no_interactions)",
            "value": 1342051.5950520833,
            "unit": "ns",
            "range": "± 23410.54460381784"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_no_namespaces)",
            "value": 1113113.96484375,
            "unit": "ns",
            "range": "± 16420.412433131725"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_interactions)",
            "value": 3567474.3451286764,
            "unit": "ns",
            "range": "± 68288.86620116392"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_no_interactions)",
            "value": 1133971.622242647,
            "unit": "ns",
            "range": "± 21027.6536702211"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: few_features)",
            "value": 3896.978302001953,
            "unit": "ns",
            "range": "± 60.66857246883044"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: few_features)",
            "value": 13649.272664388021,
            "unit": "ns",
            "range": "± 277.2057620329264"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: many_features)",
            "value": 137492.03776041666,
            "unit": "ns",
            "range": "± 2424.979874296108"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: many_features)",
            "value": 23795.688883463543,
            "unit": "ns",
            "range": "± 334.9512430530799"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: quadratic)",
            "value": 5317919.308035715,
            "unit": "ns",
            "range": "± 125904.9947854444"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: simple)",
            "value": 259821.23046875,
            "unit": "ns",
            "range": "± 3042.5610591203485"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "jackgerrits@users.noreply.github.com",
            "name": "Jack Gerrits",
            "username": "jackgerrits"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "57fedd09ff6878499b25b69b91952cdf59d90bce",
          "message": "style: fix some more style issues (#4211)\n\n* style: fix remainder of style issues\r\n\r\n* formatting\r\n\r\n* make label type and pred type adhere to guide\r\n\r\n* Revert \"make label type and pred type adhere to guide\"\r\n\r\nThis reverts commit c23d610b52728893d371a2357b2c62932916d2d2.",
          "timestamp": "2022-10-14T10:41:01-04:00",
          "tree_id": "fd67fc2e4ce6a403764ba3450969efee96cdd035",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/57fedd09ff6878499b25b69b91952cdf59d90bce"
        },
        "date": 1665760069667,
        "tool": "googlecpp",
        "benches": [
          {
            "name": "bench_text/120_string_fts",
            "value": 7955.151266019069,
            "unit": "ns/iter",
            "extra": "iterations: 537867\ncpu: 7953.970963081952 ns\nthreads: 1"
          },
          {
            "name": "bench_text/120_num_fts",
            "value": 6089.782900204176,
            "unit": "ns/iter",
            "extra": "iterations: 709950\ncpu: 6080.384674977113 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/8_features",
            "value": 105.5226140787174,
            "unit": "ns/iter",
            "extra": "iterations: 43225263\ncpu: 105.51082592603314 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/1_feature",
            "value": 209.47002522924228,
            "unit": "ns/iter",
            "extra": "iterations: 18886016\ncpu: 209.43507090113656 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features",
            "value": 8734.15865146608,
            "unit": "ns/iter",
            "extra": "iterations: 464534\ncpu: 8732.14533274206 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features",
            "value": 15950.624611577226,
            "unit": "ns/iter",
            "extra": "iterations: 252946\ncpu: 15948.404402520706 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features_no_predict",
            "value": 4260.715705820872,
            "unit": "ns/iter",
            "extra": "iterations: 930413\ncpu: 4260.269901645822 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features_no_predic",
            "value": 6495.990903145327,
            "unit": "ns/iter",
            "extra": "iterations: 622303\ncpu: 6495.249741685324 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/few_features",
            "value": 1608.4280656403369,
            "unit": "ns/iter",
            "extra": "iterations: 2590299\ncpu: 1608.2650304076872 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/many_features/min_time:15.000",
            "value": 100295.0768215227,
            "unit": "ns/iter",
            "extra": "iterations: 208301\ncpu: 100282.4686391328 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_no_namespaces/min_time:15.000",
            "value": 404381.40215773275,
            "unit": "ns/iter",
            "extra": "iterations: 51721\ncpu: 404323.88971597626 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 423956.4244697885,
            "unit": "ns/iter",
            "extra": "iterations: 49980\ncpu: 423898.45738295343 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_interactions/min_time:15.000",
            "value": 734296.7418871295,
            "unit": "ns/iter",
            "extra": "iterations: 28689\ncpu: 734211.6839206661 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_no_interactions/min_time:15.000",
            "value": 414593.8028996118,
            "unit": "ns/iter",
            "extra": "iterations: 51938\ncpu: 414539.835958258 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_interactions/min_time:15.000",
            "value": 615562.4367435782,
            "unit": "ns/iter",
            "extra": "iterations: 33546\ncpu: 615481.9829487868 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_no_namespaces/min_time:15.000",
            "value": 838018.8669162219,
            "unit": "ns/iter",
            "extra": "iterations: 25112\ncpu: 837905.7621854098 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 1037448.3402448224,
            "unit": "ns/iter",
            "extra": "iterations: 21076\ncpu: 1037290.7809831089 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_interactions/min_time:15.000",
            "value": 3532207.314892195,
            "unit": "ns/iter",
            "extra": "iterations: 5983\ncpu: 3531679.14089921 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_no_interactions/min_time:15.000",
            "value": 871957.3666653063,
            "unit": "ns/iter",
            "extra": "iterations: 24491\ncpu: 871822.0611653264 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_interactions/min_time:15.000",
            "value": 2671829.9321595016,
            "unit": "ns/iter",
            "extra": "iterations: 7724\ncpu: 2671410.693940964 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/simple/min_time:15.000",
            "value": 86713.04700761143,
            "unit": "ns/iter",
            "extra": "iterations: 241897\ncpu: 86703.39235294369 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/quadratic/min_time:15.000",
            "value": 3329859.883931471,
            "unit": "ns/iter",
            "extra": "iterations: 6186\ncpu: 3329487.9728419012 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_string_fts",
            "value": 1249.46642657312,
            "unit": "ns/iter",
            "extra": "iterations: 3348973\ncpu: 1249.3043091120787 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_string_fts",
            "value": 8695.33414611119,
            "unit": "ns/iter",
            "extra": "iterations: 476145\ncpu: 8694.145900933496 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_num_fts",
            "value": 924.3358866449587,
            "unit": "ns/iter",
            "extra": "iterations: 4673669\ncpu: 924.224822083033 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_num_fts",
            "value": 6258.132600163214,
            "unit": "ns/iter",
            "extra": "iterations: 683061\ncpu: 6257.471001857747 ns\nthreads: 1"
          },
          {
            "name": "benchmark_example_reuse",
            "value": 1126.4924162900925,
            "unit": "ns/iter",
            "extra": "iterations: 3839150\ncpu: 1126.353515752184 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_char",
            "value": 58.833357717201274,
            "unit": "ns/iter",
            "extra": "iterations: 73299836\ncpu: 58.825735981182135 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_extent",
            "value": 81.48242260896247,
            "unit": "ns/iter",
            "extra": "iterations: 50155680\ncpu: 81.47165784613028 ns\nthreads: 1"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "jackgerrits@users.noreply.github.com",
            "name": "Jack Gerrits",
            "username": "jackgerrits"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "57fedd09ff6878499b25b69b91952cdf59d90bce",
          "message": "style: fix some more style issues (#4211)\n\n* style: fix remainder of style issues\r\n\r\n* formatting\r\n\r\n* make label type and pred type adhere to guide\r\n\r\n* Revert \"make label type and pred type adhere to guide\"\r\n\r\nThis reverts commit c23d610b52728893d371a2357b2c62932916d2d2.",
          "timestamp": "2022-10-14T10:41:01-04:00",
          "tree_id": "fd67fc2e4ce6a403764ba3450969efee96cdd035",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/57fedd09ff6878499b25b69b91952cdf59d90bce"
        },
        "date": 1665761410883,
        "tool": "benchmarkdotnet",
        "benches": [
          {
            "name": "BenchmarkText.Benchmark(args: 120_num_features)",
            "value": 6987.8035227457685,
            "unit": "ns",
            "range": "± 121.51168538246657"
          },
          {
            "name": "BenchmarkText.Benchmark(args: 120_string_fts)",
            "value": 9999.972299429086,
            "unit": "ns",
            "range": "± 100.95210835673647"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 1_feature)",
            "value": 771.6443570454916,
            "unit": "ns",
            "range": "± 7.993040943726507"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 8_features)",
            "value": 641.5487925211588,
            "unit": "ns",
            "range": "± 11.059116736750147"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_interactions)",
            "value": 1120696.0546875,
            "unit": "ns",
            "range": "± 12095.76341919847"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_no_interactions)",
            "value": 693426.1848958334,
            "unit": "ns",
            "range": "± 12102.575474684589"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_no_namespaces)",
            "value": 679539.8367745535,
            "unit": "ns",
            "range": "± 9958.29526824747"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_interactions)",
            "value": 1136775.3515625,
            "unit": "ns",
            "range": "± 13876.292897711775"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_no_interactions)",
            "value": 696862.4479166666,
            "unit": "ns",
            "range": "± 5714.6806744263595"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_interactions)",
            "value": 4539820.989583333,
            "unit": "ns",
            "range": "± 45899.737477664385"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_no_interactions)",
            "value": 1364216.9140625,
            "unit": "ns",
            "range": "± 21711.938348799074"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_no_namespaces)",
            "value": 1128291.5318080357,
            "unit": "ns",
            "range": "± 12297.027906727464"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_interactions)",
            "value": 3742056.340144231,
            "unit": "ns",
            "range": "± 46433.69569378828"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_no_interactions)",
            "value": 1147208.3854166667,
            "unit": "ns",
            "range": "± 12058.93759652447"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: few_features)",
            "value": 4051.546314784459,
            "unit": "ns",
            "range": "± 37.925065118978814"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: few_features)",
            "value": 14272.054646809896,
            "unit": "ns",
            "range": "± 163.81000278351533"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: many_features)",
            "value": 144722.29817708334,
            "unit": "ns",
            "range": "± 1192.533250587735"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: many_features)",
            "value": 24599.777425130207,
            "unit": "ns",
            "range": "± 323.7946028359475"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: quadratic)",
            "value": 5283708.020833333,
            "unit": "ns",
            "range": "± 84393.17760669022"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: simple)",
            "value": 281886.298828125,
            "unit": "ns",
            "range": "± 6392.376004658656"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "jackgerrits@users.noreply.github.com",
            "name": "Jack Gerrits",
            "username": "jackgerrits"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "f0d45780cfc509a0e17558bc074b4b3f250fb945",
          "message": "feat: implement serialization and deserialization for model deltas (#4222)\n\n* feat: implement serialization and deserialization for model deltas\r\n\r\n* Update merge.cc\r\n\r\n* Update merge.cc",
          "timestamp": "2022-10-14T15:23:23-04:00",
          "tree_id": "61e1a333c045ce0da4c180805761c43cc4a88de3",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/f0d45780cfc509a0e17558bc074b4b3f250fb945"
        },
        "date": 1665776627685,
        "tool": "googlecpp",
        "benches": [
          {
            "name": "bench_text/120_string_fts",
            "value": 6043.352466338104,
            "unit": "ns/iter",
            "extra": "iterations: 696336\ncpu: 6028.701086831644 ns\nthreads: 1"
          },
          {
            "name": "bench_text/120_num_fts",
            "value": 4573.690786361958,
            "unit": "ns/iter",
            "extra": "iterations: 919691\ncpu: 4572.759872609387 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/8_features",
            "value": 75.13116417859543,
            "unit": "ns/iter",
            "extra": "iterations: 55680675\ncpu: 75.12230051090435 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/1_feature",
            "value": 174.3530641885303,
            "unit": "ns/iter",
            "extra": "iterations: 24305603\ncpu: 174.34141831412293 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features",
            "value": 7112.364045114522,
            "unit": "ns/iter",
            "extra": "iterations: 592270\ncpu: 7110.681445962144 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features",
            "value": 12594.062511025202,
            "unit": "ns/iter",
            "extra": "iterations: 334469\ncpu: 12592.207648541422 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features_no_predict",
            "value": 3419.8015367181274,
            "unit": "ns/iter",
            "extra": "iterations: 1238093\ncpu: 3419.3499195940844 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features_no_predic",
            "value": 5065.9492358917405,
            "unit": "ns/iter",
            "extra": "iterations: 825603\ncpu: 5065.566501090726 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/few_features",
            "value": 1201.124031736889,
            "unit": "ns/iter",
            "extra": "iterations: 3508473\ncpu: 1201.0190758201627 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/many_features/min_time:15.000",
            "value": 64012.60336074039,
            "unit": "ns/iter",
            "extra": "iterations: 328142\ncpu: 63999.49290246294 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_no_namespaces/min_time:15.000",
            "value": 294343.3311179382,
            "unit": "ns/iter",
            "extra": "iterations: 75532\ncpu: 294314.3118148598 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 325996.63747724134,
            "unit": "ns/iter",
            "extra": "iterations: 64818\ncpu: 325930.5918109167 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_interactions/min_time:15.000",
            "value": 568257.9018123625,
            "unit": "ns/iter",
            "extra": "iterations: 36858\ncpu: 568166.5825600957 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_no_interactions/min_time:15.000",
            "value": 303335.51221260225,
            "unit": "ns/iter",
            "extra": "iterations: 69764\ncpu: 303305.3867324121 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_interactions/min_time:15.000",
            "value": 445449.06165206706,
            "unit": "ns/iter",
            "extra": "iterations: 46584\ncpu: 445358.17448050814 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_no_namespaces/min_time:15.000",
            "value": 624354.5213596565,
            "unit": "ns/iter",
            "extra": "iterations: 33685\ncpu: 624290.0430458661 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 793994.7590111372,
            "unit": "ns/iter",
            "extra": "iterations: 26495\ncpu: 793661.3549726356 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_interactions/min_time:15.000",
            "value": 2336242.883676654,
            "unit": "ns/iter",
            "extra": "iterations: 8932\ncpu: 2335979.6462158547 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_no_interactions/min_time:15.000",
            "value": 620575.0703535173,
            "unit": "ns/iter",
            "extra": "iterations: 34284\ncpu: 620178.4214210715 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_interactions/min_time:15.000",
            "value": 1881699.1636250839,
            "unit": "ns/iter",
            "extra": "iterations: 11288\ncpu: 1881502.5513819954 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/simple/min_time:15.000",
            "value": 50271.88215722929,
            "unit": "ns/iter",
            "extra": "iterations: 417480\ncpu: 50261.58498610711 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/quadratic/min_time:15.000",
            "value": 2937558.5977938133,
            "unit": "ns/iter",
            "extra": "iterations: 7071\ncpu: 2937251.9304200253 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_string_fts",
            "value": 953.085526954822,
            "unit": "ns/iter",
            "extra": "iterations: 4406260\ncpu: 953.0179562713041 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_string_fts",
            "value": 7733.281731127055,
            "unit": "ns/iter",
            "extra": "iterations: 545009\ncpu: 7732.745881260665 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_num_fts",
            "value": 725.3801841108243,
            "unit": "ns/iter",
            "extra": "iterations: 5797378\ncpu: 725.3283294620392 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_num_fts",
            "value": 5454.819783093923,
            "unit": "ns/iter",
            "extra": "iterations: 770011\ncpu: 5454.501948673433 ns\nthreads: 1"
          },
          {
            "name": "benchmark_example_reuse",
            "value": 812.5907957504705,
            "unit": "ns/iter",
            "extra": "iterations: 5170264\ncpu: 812.5225713812694 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_char",
            "value": 53.53380653506026,
            "unit": "ns/iter",
            "extra": "iterations: 78457730\ncpu: 53.53029204388117 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_extent",
            "value": 74.51619463710912,
            "unit": "ns/iter",
            "extra": "iterations: 56391878\ncpu: 74.4771578630527 ns\nthreads: 1"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "jackgerrits@users.noreply.github.com",
            "name": "Jack Gerrits",
            "username": "jackgerrits"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "f0d45780cfc509a0e17558bc074b4b3f250fb945",
          "message": "feat: implement serialization and deserialization for model deltas (#4222)\n\n* feat: implement serialization and deserialization for model deltas\r\n\r\n* Update merge.cc\r\n\r\n* Update merge.cc",
          "timestamp": "2022-10-14T15:23:23-04:00",
          "tree_id": "61e1a333c045ce0da4c180805761c43cc4a88de3",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/f0d45780cfc509a0e17558bc074b4b3f250fb945"
        },
        "date": 1665778323104,
        "tool": "benchmarkdotnet",
        "benches": [
          {
            "name": "BenchmarkText.Benchmark(args: 120_num_features)",
            "value": 6019.989667619978,
            "unit": "ns",
            "range": "± 76.98823061290186"
          },
          {
            "name": "BenchmarkText.Benchmark(args: 120_string_fts)",
            "value": 9645.783284505209,
            "unit": "ns",
            "range": "± 114.68334582867875"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 1_feature)",
            "value": 618.1991797227126,
            "unit": "ns",
            "range": "± 4.089135375114231"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 8_features)",
            "value": 491.70724868774414,
            "unit": "ns",
            "range": "± 3.5067281798738605"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_interactions)",
            "value": 916334.6749441965,
            "unit": "ns",
            "range": "± 14323.824538433113"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_no_interactions)",
            "value": 565597.4609375,
            "unit": "ns",
            "range": "± 9691.842747605626"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_no_namespaces)",
            "value": 560005.0130208334,
            "unit": "ns",
            "range": "± 3664.0973331348223"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_interactions)",
            "value": 913083.5269325658,
            "unit": "ns",
            "range": "± 19967.659492334176"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_no_interactions)",
            "value": 566887.4755859375,
            "unit": "ns",
            "range": "± 10935.292715769261"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_interactions)",
            "value": 3586086.9270833335,
            "unit": "ns",
            "range": "± 10567.968352797816"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_no_interactions)",
            "value": 1325707.1484375,
            "unit": "ns",
            "range": "± 3887.8506635392287"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_no_namespaces)",
            "value": 928376.1125837053,
            "unit": "ns",
            "range": "± 25266.99274835083"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_interactions)",
            "value": 2826499.639423077,
            "unit": "ns",
            "range": "± 6829.718621131162"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_no_interactions)",
            "value": 925130.4827008928,
            "unit": "ns",
            "range": "± 16256.030869751376"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: few_features)",
            "value": 3267.3401387532554,
            "unit": "ns",
            "range": "± 46.552438684451744"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: few_features)",
            "value": 11542.430005754743,
            "unit": "ns",
            "range": "± 37.1266404832354"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: many_features)",
            "value": 110840.94050480769,
            "unit": "ns",
            "range": "± 1387.0731301116543"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: many_features)",
            "value": 19195.966045673078,
            "unit": "ns",
            "range": "± 53.4027523950755"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: quadratic)",
            "value": 4246424.088541667,
            "unit": "ns",
            "range": "± 15486.547966802023"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: simple)",
            "value": 205381.201171875,
            "unit": "ns",
            "range": "± 4063.341586477853"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "griffinbassman@gmail.com",
            "name": "Griffin Bassman",
            "username": "bassmang"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "d8604844d9d05cc7d037d2b005dcbdfac2579b5f",
          "message": "chore: update boost_math, fmt, vcpkg, zlib (#4223)",
          "timestamp": "2022-10-14T16:37:47-04:00",
          "tree_id": "20873dacb43e0d659db465f671f51e24481b5a96",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/d8604844d9d05cc7d037d2b005dcbdfac2579b5f"
        },
        "date": 1665781444389,
        "tool": "googlecpp",
        "benches": [
          {
            "name": "bench_text/120_string_fts",
            "value": 5938.621963691043,
            "unit": "ns/iter",
            "extra": "iterations: 706985\ncpu: 5938.087936802053 ns\nthreads: 1"
          },
          {
            "name": "bench_text/120_num_fts",
            "value": 4492.561289501209,
            "unit": "ns/iter",
            "extra": "iterations: 942690\ncpu: 4492.197541079251 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/8_features",
            "value": 77.01613524149967,
            "unit": "ns/iter",
            "extra": "iterations: 54637608\ncpu: 77.01008982677278 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/1_feature",
            "value": 168.43797154312728,
            "unit": "ns/iter",
            "extra": "iterations: 24938997\ncpu: 168.42535407498545 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features",
            "value": 6950.558915445391,
            "unit": "ns/iter",
            "extra": "iterations: 602533\ncpu: 6949.893034904314 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features",
            "value": 12659.877369342745,
            "unit": "ns/iter",
            "extra": "iterations: 329469\ncpu: 12658.608245388798 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features_no_predict",
            "value": 3265.268439009742,
            "unit": "ns/iter",
            "extra": "iterations: 1300463\ncpu: 3265.046064363231 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features_no_predic",
            "value": 4968.383795933735,
            "unit": "ns/iter",
            "extra": "iterations: 850194\ncpu: 4967.98671832546 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/few_features",
            "value": 1216.678753000741,
            "unit": "ns/iter",
            "extra": "iterations: 3474613\ncpu: 1216.0172945879171 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/many_features/min_time:15.000",
            "value": 62936.48520233003,
            "unit": "ns/iter",
            "extra": "iterations: 334850\ncpu: 62930.79886516348 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_no_namespaces/min_time:15.000",
            "value": 276441.63894676516,
            "unit": "ns/iter",
            "extra": "iterations: 78235\ncpu: 276413.17441043 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 302568.8782752892,
            "unit": "ns/iter",
            "extra": "iterations: 69345\ncpu: 302505.2217174995 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_interactions/min_time:15.000",
            "value": 551889.076642142,
            "unit": "ns/iter",
            "extra": "iterations: 38060\ncpu: 551839.4823962166 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_no_interactions/min_time:15.000",
            "value": 277052.8694494019,
            "unit": "ns/iter",
            "extra": "iterations: 76208\ncpu: 276984.13814822613 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_interactions/min_time:15.000",
            "value": 428987.04394100327,
            "unit": "ns/iter",
            "extra": "iterations: 48952\ncpu: 428951.7098382086 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_no_namespaces/min_time:15.000",
            "value": 591554.9737959536,
            "unit": "ns/iter",
            "extra": "iterations: 35796\ncpu: 591507.2438261264 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 767372.1560234575,
            "unit": "ns/iter",
            "extra": "iterations: 27451\ncpu: 767308.7792794433 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_interactions/min_time:15.000",
            "value": 2642188.6531639425,
            "unit": "ns/iter",
            "extra": "iterations: 7949\ncpu: 2642002.3399169757 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_no_interactions/min_time:15.000",
            "value": 613829.3118585137,
            "unit": "ns/iter",
            "extra": "iterations: 34490\ncpu: 613785.8538706873 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_interactions/min_time:15.000",
            "value": 1869804.907492879,
            "unit": "ns/iter",
            "extra": "iterations: 11264\ncpu: 1869662.1182528408 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/simple/min_time:15.000",
            "value": 50990.04924982289,
            "unit": "ns/iter",
            "extra": "iterations: 411636\ncpu: 50987.11507253989 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/quadratic/min_time:15.000",
            "value": 2786871.492229378,
            "unit": "ns/iter",
            "extra": "iterations: 7464\ncpu: 2786692.604501609 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_string_fts",
            "value": 947.4675805268655,
            "unit": "ns/iter",
            "extra": "iterations: 4417160\ncpu: 947.421397459008 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_string_fts",
            "value": 6978.589105231938,
            "unit": "ns/iter",
            "extra": "iterations: 601463\ncpu: 6978.271315110029 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_num_fts",
            "value": 754.6683819302775,
            "unit": "ns/iter",
            "extra": "iterations: 5570378\ncpu: 754.6322350116963 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_num_fts",
            "value": 5124.578373228446,
            "unit": "ns/iter",
            "extra": "iterations: 820816\ncpu: 5124.307152882043 ns\nthreads: 1"
          },
          {
            "name": "benchmark_example_reuse",
            "value": 784.0951792011786,
            "unit": "ns/iter",
            "extra": "iterations: 5370417\ncpu: 784.0555956827966 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_char",
            "value": 54.45190897112161,
            "unit": "ns/iter",
            "extra": "iterations: 76910540\ncpu: 54.449126218590656 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_extent",
            "value": 78.65429745806456,
            "unit": "ns/iter",
            "extra": "iterations: 53425245\ncpu: 78.65031971308721 ns\nthreads: 1"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "griffinbassman@gmail.com",
            "name": "Griffin Bassman",
            "username": "bassmang"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "d8604844d9d05cc7d037d2b005dcbdfac2579b5f",
          "message": "chore: update boost_math, fmt, vcpkg, zlib (#4223)",
          "timestamp": "2022-10-14T16:37:47-04:00",
          "tree_id": "20873dacb43e0d659db465f671f51e24481b5a96",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/d8604844d9d05cc7d037d2b005dcbdfac2579b5f"
        },
        "date": 1665782375321,
        "tool": "benchmarkdotnet",
        "benches": [
          {
            "name": "BenchmarkText.Benchmark(args: 120_num_features)",
            "value": 7927.825837976792,
            "unit": "ns",
            "range": "± 157.88895715342576"
          },
          {
            "name": "BenchmarkText.Benchmark(args: 120_string_fts)",
            "value": 11517.743530273438,
            "unit": "ns",
            "range": "± 133.4161328765653"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 1_feature)",
            "value": 716.4462906973703,
            "unit": "ns",
            "range": "± 4.906452669271442"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 8_features)",
            "value": 600.6331076988807,
            "unit": "ns",
            "range": "± 9.229656587278894"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_interactions)",
            "value": 1049826.3392857143,
            "unit": "ns",
            "range": "± 4297.667805368969"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_no_interactions)",
            "value": 641628.5993303572,
            "unit": "ns",
            "range": "± 7251.268556938131"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_no_namespaces)",
            "value": 670053.3813476562,
            "unit": "ns",
            "range": "± 17169.266395963143"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_interactions)",
            "value": 1027830.46875,
            "unit": "ns",
            "range": "± 8055.483626461409"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_no_interactions)",
            "value": 630986.7885044643,
            "unit": "ns",
            "range": "± 7136.599946286041"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_interactions)",
            "value": 4195529.622395833,
            "unit": "ns",
            "range": "± 92374.14020500057"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_no_interactions)",
            "value": 1276457.75390625,
            "unit": "ns",
            "range": "± 23269.762342610087"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_no_namespaces)",
            "value": 1036418.7779017857,
            "unit": "ns",
            "range": "± 3568.422034523004"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_interactions)",
            "value": 3408795.5767463236,
            "unit": "ns",
            "range": "± 69722.21140781647"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_no_interactions)",
            "value": 1079962.8348214286,
            "unit": "ns",
            "range": "± 11331.154609650877"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: few_features)",
            "value": 3869.2168644496373,
            "unit": "ns",
            "range": "± 54.306497720014505"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: few_features)",
            "value": 13726.248982747396,
            "unit": "ns",
            "range": "± 234.06486694894673"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: many_features)",
            "value": 131024.77722167969,
            "unit": "ns",
            "range": "± 2397.4072693183152"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: many_features)",
            "value": 22606.302693684895,
            "unit": "ns",
            "range": "± 287.7546455530587"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: quadratic)",
            "value": 5393404.1015625,
            "unit": "ns",
            "range": "± 152476.76383737402"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: simple)",
            "value": 257764.86497961957,
            "unit": "ns",
            "range": "± 6472.779048405515"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "jackgerrits@users.noreply.github.com",
            "name": "Jack Gerrits",
            "username": "jackgerrits"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "4d181afdfa5a82967833bc64be24e92b94cb8ae6",
          "message": "style: another round of style updates (#4224)\n\n* style: another round of style updates\r\n\r\n* fix name\r\n\r\n* ignore deprecated\r\n\r\n* formatting\r\n\r\n* formatting",
          "timestamp": "2022-10-14T18:25:28-04:00",
          "tree_id": "c767f307e799a45c6450fc5d631e4b834b7d76f1",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/4d181afdfa5a82967833bc64be24e92b94cb8ae6"
        },
        "date": 1665787621902,
        "tool": "googlecpp",
        "benches": [
          {
            "name": "bench_text/120_string_fts",
            "value": 6182.887605226443,
            "unit": "ns/iter",
            "extra": "iterations: 678181\ncpu: 6182.542713523381 ns\nthreads: 1"
          },
          {
            "name": "bench_text/120_num_fts",
            "value": 4636.872097426516,
            "unit": "ns/iter",
            "extra": "iterations: 895197\ncpu: 4636.615404207118 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/8_features",
            "value": 79.2895772851795,
            "unit": "ns/iter",
            "extra": "iterations: 55782856\ncpu: 79.28564109374395 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/1_feature",
            "value": 169.0582969007716,
            "unit": "ns/iter",
            "extra": "iterations: 24699289\ncpu: 169.04867180589687 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features",
            "value": 6816.033863027629,
            "unit": "ns/iter",
            "extra": "iterations: 613058\ncpu: 6815.560517928155 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features",
            "value": 12355.193017003121,
            "unit": "ns/iter",
            "extra": "iterations: 333639\ncpu: 12354.081207532692 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features_no_predict",
            "value": 3247.6558168381016,
            "unit": "ns/iter",
            "extra": "iterations: 1286687\ncpu: 3247.496399668294 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features_no_predic",
            "value": 4901.784930703096,
            "unit": "ns/iter",
            "extra": "iterations: 855594\ncpu: 4901.503867488548 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/few_features",
            "value": 1174.257334823223,
            "unit": "ns/iter",
            "extra": "iterations: 3555539\ncpu: 1174.1947704693987 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/many_features/min_time:15.000",
            "value": 56584.129008379365,
            "unit": "ns/iter",
            "extra": "iterations: 371503\ncpu: 56580.05830370146 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_no_namespaces/min_time:15.000",
            "value": 269312.58575043787,
            "unit": "ns/iter",
            "extra": "iterations: 82192\ncpu: 269261.4877360326 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 298747.6046934455,
            "unit": "ns/iter",
            "extra": "iterations: 70950\ncpu: 298720.73009161366 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_interactions/min_time:15.000",
            "value": 540920.2636628574,
            "unit": "ns/iter",
            "extra": "iterations: 38773\ncpu: 540821.2106362676 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_no_interactions/min_time:15.000",
            "value": 267343.16357225244,
            "unit": "ns/iter",
            "extra": "iterations: 77990\ncpu: 267321.78227977944 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_interactions/min_time:15.000",
            "value": 420197.8141406131,
            "unit": "ns/iter",
            "extra": "iterations: 50210\ncpu: 420162.85202150967 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_no_namespaces/min_time:15.000",
            "value": 575828.598817168,
            "unit": "ns/iter",
            "extra": "iterations: 35677\ncpu: 575776.2648204727 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 782561.2291286037,
            "unit": "ns/iter",
            "extra": "iterations: 26819\ncpu: 782490.547746002 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_interactions/min_time:15.000",
            "value": 2408891.013722123,
            "unit": "ns/iter",
            "extra": "iterations: 8745\ncpu: 2408422.6300743287 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_no_interactions/min_time:15.000",
            "value": 622187.6712064552,
            "unit": "ns/iter",
            "extra": "iterations: 34216\ncpu: 622107.7332242228 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_interactions/min_time:15.000",
            "value": 1801171.9759025911,
            "unit": "ns/iter",
            "extra": "iterations: 11744\ncpu: 1800828.6273841946 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/simple/min_time:15.000",
            "value": 53799.146642110594,
            "unit": "ns/iter",
            "extra": "iterations: 390945\ncpu: 53792.23394595145 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/quadratic/min_time:15.000",
            "value": 2616707.2670123973,
            "unit": "ns/iter",
            "extra": "iterations: 7906\ncpu: 2615958.5884138625 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_string_fts",
            "value": 954.1168022445596,
            "unit": "ns/iter",
            "extra": "iterations: 4383212\ncpu: 954.0599222670457 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_string_fts",
            "value": 7928.397459276,
            "unit": "ns/iter",
            "extra": "iterations: 523709\ncpu: 7927.849053577403 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_num_fts",
            "value": 731.1709911014011,
            "unit": "ns/iter",
            "extra": "iterations: 5782143\ncpu: 731.1280955867029 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_num_fts",
            "value": 5600.226057927108,
            "unit": "ns/iter",
            "extra": "iterations: 752462\ncpu: 5599.898333736426 ns\nthreads: 1"
          },
          {
            "name": "benchmark_example_reuse",
            "value": 816.973663689343,
            "unit": "ns/iter",
            "extra": "iterations: 5146165\ncpu: 816.9268766158915 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_char",
            "value": 53.53200867231825,
            "unit": "ns/iter",
            "extra": "iterations: 78568629\ncpu: 53.529346935658005 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_extent",
            "value": 74.01355148699514,
            "unit": "ns/iter",
            "extra": "iterations: 56749418\ncpu: 74.00963125295905 ns\nthreads: 1"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "jackgerrits@users.noreply.github.com",
            "name": "Jack Gerrits",
            "username": "jackgerrits"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "4d181afdfa5a82967833bc64be24e92b94cb8ae6",
          "message": "style: another round of style updates (#4224)\n\n* style: another round of style updates\r\n\r\n* fix name\r\n\r\n* ignore deprecated\r\n\r\n* formatting\r\n\r\n* formatting",
          "timestamp": "2022-10-14T18:25:28-04:00",
          "tree_id": "c767f307e799a45c6450fc5d631e4b834b7d76f1",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/4d181afdfa5a82967833bc64be24e92b94cb8ae6"
        },
        "date": 1665788962171,
        "tool": "benchmarkdotnet",
        "benches": [
          {
            "name": "BenchmarkText.Benchmark(args: 120_num_features)",
            "value": 6535.402118458467,
            "unit": "ns",
            "range": "± 126.18799032908335"
          },
          {
            "name": "BenchmarkText.Benchmark(args: 120_string_fts)",
            "value": 9581.880235671997,
            "unit": "ns",
            "range": "± 292.289531694241"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 1_feature)",
            "value": 708.961918774773,
            "unit": "ns",
            "range": "± 14.421399572280672"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 8_features)",
            "value": 626.4825948079427,
            "unit": "ns",
            "range": "± 11.27938299867541"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_interactions)",
            "value": 1079833.2293610075,
            "unit": "ns",
            "range": "± 48612.63898506707"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_no_interactions)",
            "value": 686934.7200520834,
            "unit": "ns",
            "range": "± 9604.016290673288"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_no_namespaces)",
            "value": 665850.615234375,
            "unit": "ns",
            "range": "± 11880.484318326624"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_interactions)",
            "value": 1074540.3515625,
            "unit": "ns",
            "range": "± 19427.354414124202"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_no_interactions)",
            "value": 663799.8104319853,
            "unit": "ns",
            "range": "± 12731.072940141576"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_interactions)",
            "value": 4588805.483217592,
            "unit": "ns",
            "range": "± 126455.93876696075"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_no_interactions)",
            "value": 1292991.7410714286,
            "unit": "ns",
            "range": "± 20616.606360291"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_no_namespaces)",
            "value": 1116156.54296875,
            "unit": "ns",
            "range": "± 21311.087458750662"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_interactions)",
            "value": 3627472.3697916665,
            "unit": "ns",
            "range": "± 44590.869422870746"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_no_interactions)",
            "value": 1146321.1263020833,
            "unit": "ns",
            "range": "± 26904.74875197123"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: few_features)",
            "value": 3943.1900746113547,
            "unit": "ns",
            "range": "± 133.88770181997245"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: few_features)",
            "value": 12975.966389973959,
            "unit": "ns",
            "range": "± 329.80854144830187"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: many_features)",
            "value": 137771.53198242188,
            "unit": "ns",
            "range": "± 3110.9817491598874"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: many_features)",
            "value": 22782.22151536208,
            "unit": "ns",
            "range": "± 375.53757771763446"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: quadratic)",
            "value": 4984644.088541667,
            "unit": "ns",
            "range": "± 87554.27544563157"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: simple)",
            "value": 287795.41459517047,
            "unit": "ns",
            "range": "± 6910.465669264364"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "jackgerrits@users.noreply.github.com",
            "name": "Jack Gerrits",
            "username": "jackgerrits"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "464e2a2f18147ec1c69cb3e971922074edc4dcb3",
          "message": "style: update style and namespacing for constants (#4226)\n\n* style: update style and namespacing for constants\r\n\r\n* test fix\r\n\r\n* test fix\r\n\r\n* fix test",
          "timestamp": "2022-10-17T09:34:01-04:00",
          "tree_id": "ba1b324d25ff13772bce675bc191726ed2ee1f9d",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/464e2a2f18147ec1c69cb3e971922074edc4dcb3"
        },
        "date": 1666015253255,
        "tool": "googlecpp",
        "benches": [
          {
            "name": "bench_text/120_string_fts",
            "value": 6997.707446261301,
            "unit": "ns/iter",
            "extra": "iterations: 602450\ncpu: 6992.876587268654 ns\nthreads: 1"
          },
          {
            "name": "bench_text/120_num_fts",
            "value": 4869.80181706705,
            "unit": "ns/iter",
            "extra": "iterations: 875807\ncpu: 4864.920125096053 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/8_features",
            "value": 90.29595959539067,
            "unit": "ns/iter",
            "extra": "iterations: 45899512\ncpu: 90.28485749478122 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/1_feature",
            "value": 196.15631039716328,
            "unit": "ns/iter",
            "extra": "iterations: 21404667\ncpu: 196.12978328511255 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features",
            "value": 8239.487905985095,
            "unit": "ns/iter",
            "extra": "iterations: 508516\ncpu: 8237.443462939222 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features",
            "value": 15521.231073607289,
            "unit": "ns/iter",
            "extra": "iterations: 270667\ncpu: 15518.779164065081 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features_no_predict",
            "value": 4085.588776205566,
            "unit": "ns/iter",
            "extra": "iterations: 1047578\ncpu: 4084.9471829305257 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features_no_predic",
            "value": 6031.905745835725,
            "unit": "ns/iter",
            "extra": "iterations: 650210\ncpu: 6030.992756186465 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/few_features",
            "value": 1446.9093354283175,
            "unit": "ns/iter",
            "extra": "iterations: 2923733\ncpu: 1446.7176038304453 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/many_features/min_time:15.000",
            "value": 99636.96698262525,
            "unit": "ns/iter",
            "extra": "iterations: 211737\ncpu: 99607.44177918833 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_no_namespaces/min_time:15.000",
            "value": 342930.93359231396,
            "unit": "ns/iter",
            "extra": "iterations: 62553\ncpu: 342884.10947516514 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 358301.84815302823,
            "unit": "ns/iter",
            "extra": "iterations: 58447\ncpu: 358270.63493421394 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_interactions/min_time:15.000",
            "value": 633716.8594349398,
            "unit": "ns/iter",
            "extra": "iterations: 32846\ncpu: 633656.2899592036 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_no_interactions/min_time:15.000",
            "value": 338196.8809481603,
            "unit": "ns/iter",
            "extra": "iterations: 62099\ncpu: 338168.33604405844 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_interactions/min_time:15.000",
            "value": 526262.6927941216,
            "unit": "ns/iter",
            "extra": "iterations: 40009\ncpu: 526216.7562298486 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_no_namespaces/min_time:15.000",
            "value": 729429.4807692312,
            "unit": "ns/iter",
            "extra": "iterations: 28548\ncpu: 729353.029984588 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 898712.6256997576,
            "unit": "ns/iter",
            "extra": "iterations: 23401\ncpu: 898640.2375966844 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_interactions/min_time:15.000",
            "value": 3143676.0583651694,
            "unit": "ns/iter",
            "extra": "iterations: 6802\ncpu: 3143392.3404880962 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_no_interactions/min_time:15.000",
            "value": 743591.8560940906,
            "unit": "ns/iter",
            "extra": "iterations: 27379\ncpu: 743525.9943752498 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_interactions/min_time:15.000",
            "value": 2474616.6214537094,
            "unit": "ns/iter",
            "extra": "iterations: 8530\ncpu: 2474395.8147713924 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/simple/min_time:15.000",
            "value": 84673.83369053982,
            "unit": "ns/iter",
            "extra": "iterations: 249156\ncpu: 84666.71563197343 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/quadratic/min_time:15.000",
            "value": 3252149.4404440685,
            "unit": "ns/iter",
            "extra": "iterations: 6305\ncpu: 3251851.435368749 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_string_fts",
            "value": 1162.615218465446,
            "unit": "ns/iter",
            "extra": "iterations: 3638974\ncpu: 1162.5271024195265 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_string_fts",
            "value": 8264.791517018577,
            "unit": "ns/iter",
            "extra": "iterations: 498669\ncpu: 8264.047895497766 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_num_fts",
            "value": 868.4824820305946,
            "unit": "ns/iter",
            "extra": "iterations: 4822648\ncpu: 868.4082686524105 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_num_fts",
            "value": 5694.246532924408,
            "unit": "ns/iter",
            "extra": "iterations: 718545\ncpu: 5693.756549694139 ns\nthreads: 1"
          },
          {
            "name": "benchmark_example_reuse",
            "value": 1007.9558280235184,
            "unit": "ns/iter",
            "extra": "iterations: 4259420\ncpu: 1007.8650849176573 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_char",
            "value": 57.90255954906127,
            "unit": "ns/iter",
            "extra": "iterations: 76505742\ncpu: 57.896892235879186 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_extent",
            "value": 80.24501678848134,
            "unit": "ns/iter",
            "extra": "iterations: 49181336\ncpu: 80.23859701574521 ns\nthreads: 1"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "jackgerrits@users.noreply.github.com",
            "name": "Jack Gerrits",
            "username": "jackgerrits"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "464e2a2f18147ec1c69cb3e971922074edc4dcb3",
          "message": "style: update style and namespacing for constants (#4226)\n\n* style: update style and namespacing for constants\r\n\r\n* test fix\r\n\r\n* test fix\r\n\r\n* fix test",
          "timestamp": "2022-10-17T09:34:01-04:00",
          "tree_id": "ba1b324d25ff13772bce675bc191726ed2ee1f9d",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/464e2a2f18147ec1c69cb3e971922074edc4dcb3"
        },
        "date": 1666016467974,
        "tool": "benchmarkdotnet",
        "benches": [
          {
            "name": "BenchmarkText.Benchmark(args: 120_num_features)",
            "value": 6522.857259114583,
            "unit": "ns",
            "range": "± 32.263693519994085"
          },
          {
            "name": "BenchmarkText.Benchmark(args: 120_string_fts)",
            "value": 9683.404541015625,
            "unit": "ns",
            "range": "± 27.069452568033956"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 1_feature)",
            "value": 626.6759936014811,
            "unit": "ns",
            "range": "± 1.420002461349195"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 8_features)",
            "value": 519.3457730611166,
            "unit": "ns",
            "range": "± 1.708998107910732"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_interactions)",
            "value": 930411.5513392857,
            "unit": "ns",
            "range": "± 3908.547170741615"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_no_interactions)",
            "value": 572943.49609375,
            "unit": "ns",
            "range": "± 3176.6080454700873"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_no_namespaces)",
            "value": 567559.7395833334,
            "unit": "ns",
            "range": "± 3330.895231535691"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_interactions)",
            "value": 931116.25,
            "unit": "ns",
            "range": "± 2352.0620788684823"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_no_interactions)",
            "value": 567542.2135416666,
            "unit": "ns",
            "range": "± 1844.9249659733402"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_interactions)",
            "value": 3566092.3270089286,
            "unit": "ns",
            "range": "± 9582.750094315154"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_no_interactions)",
            "value": 1073906.171875,
            "unit": "ns",
            "range": "± 1865.2188840395436"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_no_namespaces)",
            "value": 905654.0950520834,
            "unit": "ns",
            "range": "± 3339.4504276059884"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_interactions)",
            "value": 2855091.0416666665,
            "unit": "ns",
            "range": "± 8527.012288067208"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_no_interactions)",
            "value": 937222.7734375,
            "unit": "ns",
            "range": "± 3635.692538994807"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: few_features)",
            "value": 3189.5130666097007,
            "unit": "ns",
            "range": "± 11.248802022356832"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: few_features)",
            "value": 11466.531982421875,
            "unit": "ns",
            "range": "± 36.51872437092875"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: many_features)",
            "value": 109438.14371744792,
            "unit": "ns",
            "range": "± 297.7169888686123"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: many_features)",
            "value": 19355.797119140625,
            "unit": "ns",
            "range": "± 93.34741831785479"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: quadratic)",
            "value": 4295024.051339285,
            "unit": "ns",
            "range": "± 48259.465168642026"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: simple)",
            "value": 208656.20930989584,
            "unit": "ns",
            "range": "± 937.04200954847"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "olgavrou@gmail.com",
            "name": "olgavrou",
            "username": "olgavrou"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "cc0e987b2737e2cc637095dd7e103063bea5fce3",
          "message": "fix: [LAS] don't use shared features during SVD calculation (#4225)",
          "timestamp": "2022-10-17T14:27:30Z",
          "tree_id": "6da88a182f8607433f36c9e548f182ae27969df4",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/cc0e987b2737e2cc637095dd7e103063bea5fce3"
        },
        "date": 1666018157212,
        "tool": "googlecpp",
        "benches": [
          {
            "name": "bench_text/120_string_fts",
            "value": 5371.969323429419,
            "unit": "ns/iter",
            "extra": "iterations: 779357\ncpu: 5371.31712424473 ns\nthreads: 1"
          },
          {
            "name": "bench_text/120_num_fts",
            "value": 3990.076025313094,
            "unit": "ns/iter",
            "extra": "iterations: 1056214\ncpu: 3989.0003351593527 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/8_features",
            "value": 70.89358430357598,
            "unit": "ns/iter",
            "extra": "iterations: 59210579\ncpu: 70.88493763926881 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/1_feature",
            "value": 159.02236535982757,
            "unit": "ns/iter",
            "extra": "iterations: 26938802\ncpu: 159.0025681171717 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features",
            "value": 6639.315469744749,
            "unit": "ns/iter",
            "extra": "iterations: 631620\ncpu: 6635.292897628322 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features",
            "value": 12032.75609663885,
            "unit": "ns/iter",
            "extra": "iterations: 344001\ncpu: 12031.065316670592 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features_no_predict",
            "value": 3334.1679460464134,
            "unit": "ns/iter",
            "extra": "iterations: 1274576\ncpu: 3333.7756242075807 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features_no_predic",
            "value": 5044.390448734605,
            "unit": "ns/iter",
            "extra": "iterations: 834235\ncpu: 5043.726527896811 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/few_features",
            "value": 1176.4957035821433,
            "unit": "ns/iter",
            "extra": "iterations: 3586127\ncpu: 1176.3589800361233 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/many_features/min_time:15.000",
            "value": 52148.48860218792,
            "unit": "ns/iter",
            "extra": "iterations: 402665\ncpu: 52141.92045496878 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_no_namespaces/min_time:15.000",
            "value": 190560.44141184824,
            "unit": "ns/iter",
            "extra": "iterations: 112335\ncpu: 190515.83567009395 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 222163.60261869128,
            "unit": "ns/iter",
            "extra": "iterations: 93940\ncpu: 222133.8439429423 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_interactions/min_time:15.000",
            "value": 448354.9119190994,
            "unit": "ns/iter",
            "extra": "iterations: 46673\ncpu: 448220.9671544573 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_no_interactions/min_time:15.000",
            "value": 177583.81475862625,
            "unit": "ns/iter",
            "extra": "iterations: 114035\ncpu: 177524.9598807384 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_interactions/min_time:15.000",
            "value": 311024.26120680774,
            "unit": "ns/iter",
            "extra": "iterations: 67169\ncpu: 310981.37831440114 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_no_namespaces/min_time:15.000",
            "value": 533810.987780356,
            "unit": "ns/iter",
            "extra": "iterations: 39281\ncpu: 533680.4867493188 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 676889.3056988097,
            "unit": "ns/iter",
            "extra": "iterations: 31024\ncpu: 676788.8795771014 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_interactions/min_time:15.000",
            "value": 2094980.5292770418,
            "unit": "ns/iter",
            "extra": "iterations: 10042\ncpu: 2094471.5295757833 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_no_interactions/min_time:15.000",
            "value": 539343.3609038158,
            "unit": "ns/iter",
            "extra": "iterations: 38459\ncpu: 539266.8478119554 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_interactions/min_time:15.000",
            "value": 1621744.8239480206,
            "unit": "ns/iter",
            "extra": "iterations: 12928\ncpu: 1621386.5872524742 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/simple/min_time:15.000",
            "value": 47654.754900937594,
            "unit": "ns/iter",
            "extra": "iterations: 440834\ncpu: 47648.775275954315 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/quadratic/min_time:15.000",
            "value": 2182874.97027624,
            "unit": "ns/iter",
            "extra": "iterations: 9521\ncpu: 2182593.3620418035 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_string_fts",
            "value": 894.892686170689,
            "unit": "ns/iter",
            "extra": "iterations: 4670768\ncpu: 894.7770687818337 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_string_fts",
            "value": 6329.738115611905,
            "unit": "ns/iter",
            "extra": "iterations: 666126\ncpu: 6326.459408580328 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_num_fts",
            "value": 682.3956597338223,
            "unit": "ns/iter",
            "extra": "iterations: 6175013\ncpu: 682.3053327984873 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_num_fts",
            "value": 4614.408930721472,
            "unit": "ns/iter",
            "extra": "iterations: 909445\ncpu: 4613.802373975304 ns\nthreads: 1"
          },
          {
            "name": "benchmark_example_reuse",
            "value": 780.0410893729701,
            "unit": "ns/iter",
            "extra": "iterations: 5372338\ncpu: 779.9380455957922 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_char",
            "value": 40.663798304605876,
            "unit": "ns/iter",
            "extra": "iterations: 103183278\ncpu: 40.658930219294085 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_extent",
            "value": 54.124986054507204,
            "unit": "ns/iter",
            "extra": "iterations: 77569862\ncpu: 54.11846162624326 ns\nthreads: 1"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "olgavrou@gmail.com",
            "name": "olgavrou",
            "username": "olgavrou"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "cc0e987b2737e2cc637095dd7e103063bea5fce3",
          "message": "fix: [LAS] don't use shared features during SVD calculation (#4225)",
          "timestamp": "2022-10-17T14:27:30Z",
          "tree_id": "6da88a182f8607433f36c9e548f182ae27969df4",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/cc0e987b2737e2cc637095dd7e103063bea5fce3"
        },
        "date": 1666019310358,
        "tool": "benchmarkdotnet",
        "benches": [
          {
            "name": "BenchmarkText.Benchmark(args: 120_num_features)",
            "value": 7639.489847819011,
            "unit": "ns",
            "range": "± 115.67547099636468"
          },
          {
            "name": "BenchmarkText.Benchmark(args: 120_string_fts)",
            "value": 11475.515638078961,
            "unit": "ns",
            "range": "± 118.70031393839758"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 1_feature)",
            "value": 728.5069847106934,
            "unit": "ns",
            "range": "± 8.273529245026927"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 8_features)",
            "value": 612.1228218078613,
            "unit": "ns",
            "range": "± 5.079529464789501"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_interactions)",
            "value": 1058159.4308035714,
            "unit": "ns",
            "range": "± 7064.0201587483125"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_no_interactions)",
            "value": 662477.34375,
            "unit": "ns",
            "range": "± 11044.862274922678"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_no_namespaces)",
            "value": 641883.28125,
            "unit": "ns",
            "range": "± 8709.616150227339"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_interactions)",
            "value": 1047515.2994791666,
            "unit": "ns",
            "range": "± 11977.136944647502"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_no_interactions)",
            "value": 649446.7145647322,
            "unit": "ns",
            "range": "± 6039.1902935474845"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_interactions)",
            "value": 4229020.390625,
            "unit": "ns",
            "range": "± 76735.58269454802"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_no_interactions)",
            "value": 1254497.0424107143,
            "unit": "ns",
            "range": "± 5882.270641406605"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_no_namespaces)",
            "value": 1040078.7890625,
            "unit": "ns",
            "range": "± 11945.29380651888"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_interactions)",
            "value": 3297007.578125,
            "unit": "ns",
            "range": "± 28303.717191569584"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_no_interactions)",
            "value": 1066954.4921875,
            "unit": "ns",
            "range": "± 11363.991030461779"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: few_features)",
            "value": 3736.297938028971,
            "unit": "ns",
            "range": "± 44.257303768874614"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: few_features)",
            "value": 13323.792215983072,
            "unit": "ns",
            "range": "± 141.77590716507797"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: many_features)",
            "value": 129841.2451171875,
            "unit": "ns",
            "range": "± 1348.3003395590579"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: many_features)",
            "value": 22345.98214285714,
            "unit": "ns",
            "range": "± 184.8793693434234"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: quadratic)",
            "value": 5044079.073660715,
            "unit": "ns",
            "range": "± 36947.96989908771"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: simple)",
            "value": 244851.93033854166,
            "unit": "ns",
            "range": "± 2548.1842495466894"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "olgavrou@gmail.com",
            "name": "olgavrou",
            "username": "olgavrou"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "3fdc288d7ccd52de6c3e8f3a33152e40a51b4c4a",
          "message": "fix: [LAS] ensure vw prediction makes it to exploration (#4227)",
          "timestamp": "2022-10-17T17:49:25Z",
          "tree_id": "5946f5e613d6e5e7385bf1818ac7939c6f8d6c83",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/3fdc288d7ccd52de6c3e8f3a33152e40a51b4c4a"
        },
        "date": 1666030732259,
        "tool": "googlecpp",
        "benches": [
          {
            "name": "bench_text/120_string_fts",
            "value": 7717.339696852298,
            "unit": "ns/iter",
            "extra": "iterations: 535448\ncpu: 7716.317924429635 ns\nthreads: 1"
          },
          {
            "name": "bench_text/120_num_fts",
            "value": 5312.947667906762,
            "unit": "ns/iter",
            "extra": "iterations: 777649\ncpu: 5312.553607090088 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/8_features",
            "value": 98.47079909359822,
            "unit": "ns/iter",
            "extra": "iterations: 43792442\ncpu: 98.46257945606233 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/1_feature",
            "value": 214.64053222064672,
            "unit": "ns/iter",
            "extra": "iterations: 19571131\ncpu: 214.62045806141705 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features",
            "value": 9085.615057670151,
            "unit": "ns/iter",
            "extra": "iterations: 453616\ncpu: 9084.685064018908 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features",
            "value": 16599.474489963344,
            "unit": "ns/iter",
            "extra": "iterations: 244100\ncpu: 16597.591151167555 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features_no_predict",
            "value": 4372.915743427733,
            "unit": "ns/iter",
            "extra": "iterations: 958311\ncpu: 4372.4888893062935 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features_no_predic",
            "value": 6649.355698320858,
            "unit": "ns/iter",
            "extra": "iterations: 612505\ncpu: 6648.825234079721 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/few_features",
            "value": 1708.546022680172,
            "unit": "ns/iter",
            "extra": "iterations: 2472205\ncpu: 1708.362170612874 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/many_features/min_time:15.000",
            "value": 103898.1932255121,
            "unit": "ns/iter",
            "extra": "iterations: 201816\ncpu: 103887.21855551594 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_no_namespaces/min_time:15.000",
            "value": 403961.2768182418,
            "unit": "ns/iter",
            "extra": "iterations: 52977\ncpu: 403923.21384751867 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 421701.2789970558,
            "unit": "ns/iter",
            "extra": "iterations: 49574\ncpu: 421655.00262234197 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_interactions/min_time:15.000",
            "value": 733182.2537562579,
            "unit": "ns/iter",
            "extra": "iterations: 28153\ncpu: 733115.7389976195 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_no_interactions/min_time:15.000",
            "value": 407487.9960367117,
            "unit": "ns/iter",
            "extra": "iterations: 52734\ncpu: 407437.7744908409 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_interactions/min_time:15.000",
            "value": 603091.1511276439,
            "unit": "ns/iter",
            "extra": "iterations: 34408\ncpu: 603033.442803999 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_no_namespaces/min_time:15.000",
            "value": 835891.6384176229,
            "unit": "ns/iter",
            "extra": "iterations: 25051\ncpu: 835737.0763642169 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 1038167.986806864,
            "unit": "ns/iter",
            "extra": "iterations: 20162\ncpu: 1038063.2179347281 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_interactions/min_time:15.000",
            "value": 3527330.4008956854,
            "unit": "ns/iter",
            "extra": "iterations: 6029\ncpu: 3527024.0835959525 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_no_interactions/min_time:15.000",
            "value": 878304.4504557506,
            "unit": "ns/iter",
            "extra": "iterations: 23807\ncpu: 878220.9308186661 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_interactions/min_time:15.000",
            "value": 2740926.3829646464,
            "unit": "ns/iter",
            "extra": "iterations: 7549\ncpu: 2740675.149026356 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/simple/min_time:15.000",
            "value": 86491.97985503804,
            "unit": "ns/iter",
            "extra": "iterations: 235096\ncpu: 86484.4238098479 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/quadratic/min_time:15.000",
            "value": 3452166.354645336,
            "unit": "ns/iter",
            "extra": "iterations: 6006\ncpu: 3451745.9873459893 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_string_fts",
            "value": 1296.44175940152,
            "unit": "ns/iter",
            "extra": "iterations: 3316355\ncpu: 1296.333353938276 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_string_fts",
            "value": 9259.570156318729,
            "unit": "ns/iter",
            "extra": "iterations: 443645\ncpu: 9258.542077562095 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_num_fts",
            "value": 942.4121434271326,
            "unit": "ns/iter",
            "extra": "iterations: 4593942\ncpu: 942.3136164975584 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_num_fts",
            "value": 6347.174066912582,
            "unit": "ns/iter",
            "extra": "iterations: 649055\ncpu: 6346.628251843054 ns\nthreads: 1"
          },
          {
            "name": "benchmark_example_reuse",
            "value": 1150.836704217954,
            "unit": "ns/iter",
            "extra": "iterations: 3533741\ncpu: 1150.724147581839 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_char",
            "value": 58.42948747445868,
            "unit": "ns/iter",
            "extra": "iterations: 70889462\ncpu: 58.425003987193385 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_extent",
            "value": 84.92992683802291,
            "unit": "ns/iter",
            "extra": "iterations: 51529500\ncpu: 84.92276074869731 ns\nthreads: 1"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "olgavrou@gmail.com",
            "name": "olgavrou",
            "username": "olgavrou"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "3fdc288d7ccd52de6c3e8f3a33152e40a51b4c4a",
          "message": "fix: [LAS] ensure vw prediction makes it to exploration (#4227)",
          "timestamp": "2022-10-17T17:49:25Z",
          "tree_id": "5946f5e613d6e5e7385bf1818ac7939c6f8d6c83",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/3fdc288d7ccd52de6c3e8f3a33152e40a51b4c4a"
        },
        "date": 1666031612613,
        "tool": "benchmarkdotnet",
        "benches": [
          {
            "name": "BenchmarkText.Benchmark(args: 120_num_features)",
            "value": 6633.4944407145185,
            "unit": "ns",
            "range": "± 73.5297407100298"
          },
          {
            "name": "BenchmarkText.Benchmark(args: 120_string_fts)",
            "value": 10077.798990102914,
            "unit": "ns",
            "range": "± 275.80877184239864"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 1_feature)",
            "value": 712.5412559509277,
            "unit": "ns",
            "range": "± 12.554073097400082"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 8_features)",
            "value": 630.5555152893066,
            "unit": "ns",
            "range": "± 11.277489681300002"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_interactions)",
            "value": 1188342.8645833333,
            "unit": "ns",
            "range": "± 16226.342689812838"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_no_interactions)",
            "value": 708908.7439903846,
            "unit": "ns",
            "range": "± 5952.1788588816635"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_no_namespaces)",
            "value": 690254.55078125,
            "unit": "ns",
            "range": "± 8931.77440337463"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_interactions)",
            "value": 1205725.8056640625,
            "unit": "ns",
            "range": "± 22540.31736964237"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_no_interactions)",
            "value": 674623.73046875,
            "unit": "ns",
            "range": "± 6183.438374311766"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_interactions)",
            "value": 4640223.515625,
            "unit": "ns",
            "range": "± 56407.3821679759"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_no_interactions)",
            "value": 1359382.6953125,
            "unit": "ns",
            "range": "± 23791.6211937928"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_no_namespaces)",
            "value": 1157371.9168526786,
            "unit": "ns",
            "range": "± 15758.812317329295"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_interactions)",
            "value": 3833980.78125,
            "unit": "ns",
            "range": "± 45964.88654311979"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_no_interactions)",
            "value": 1199416.5364583333,
            "unit": "ns",
            "range": "± 20455.149430504873"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: few_features)",
            "value": 4024.343197162335,
            "unit": "ns",
            "range": "± 105.55795741768124"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: few_features)",
            "value": 13925.170440673828,
            "unit": "ns",
            "range": "± 251.81480341687242"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: many_features)",
            "value": 146040.3426106771,
            "unit": "ns",
            "range": "± 1712.8221763302183"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: many_features)",
            "value": 23706.694946289062,
            "unit": "ns",
            "range": "± 430.7138306345241"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: quadratic)",
            "value": 6480394.767992424,
            "unit": "ns",
            "range": "± 385787.7046925187"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: simple)",
            "value": 289507.7213541667,
            "unit": "ns",
            "range": "± 4526.589692483989"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "jackgerrits@users.noreply.github.com",
            "name": "Jack Gerrits",
            "username": "jackgerrits"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "0aa74c1465627e04c45428c46b1546926e388938",
          "message": "fix: VW should not add anything to namespace std (#4230)\n\n* fix: VW should not add anything to namespace std\r\n\r\n* formatting",
          "timestamp": "2022-10-17T18:59:06-04:00",
          "tree_id": "b47bffff3122fbe43cadeca9785e0a037dd11289",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/0aa74c1465627e04c45428c46b1546926e388938"
        },
        "date": 1666048806634,
        "tool": "googlecpp",
        "benches": [
          {
            "name": "bench_text/120_string_fts",
            "value": 5302.5195177047435,
            "unit": "ns/iter",
            "extra": "iterations: 789309\ncpu: 5300.24312404901 ns\nthreads: 1"
          },
          {
            "name": "bench_text/120_num_fts",
            "value": 4754.563360133601,
            "unit": "ns/iter",
            "extra": "iterations: 1007329\ncpu: 4752.782358097504 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/8_features",
            "value": 78.70824971223196,
            "unit": "ns/iter",
            "extra": "iterations: 56711663\ncpu: 78.70155385850703 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/1_feature",
            "value": 165.00364112531136,
            "unit": "ns/iter",
            "extra": "iterations: 25586870\ncpu: 164.99414348062112 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features",
            "value": 6857.378558846767,
            "unit": "ns/iter",
            "extra": "iterations: 605182\ncpu: 6856.754992712934 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features",
            "value": 12436.508095469453,
            "unit": "ns/iter",
            "extra": "iterations: 335311\ncpu: 12431.053857463658 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features_no_predict",
            "value": 3274.420334674168,
            "unit": "ns/iter",
            "extra": "iterations: 1277601\ncpu: 3274.2268517322686 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features_no_predic",
            "value": 4954.95509792997,
            "unit": "ns/iter",
            "extra": "iterations: 843970\ncpu: 4954.664502292736 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/few_features",
            "value": 1198.9108434378163,
            "unit": "ns/iter",
            "extra": "iterations: 3484264\ncpu: 1198.8400419715604 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/many_features/min_time:15.000",
            "value": 57883.63259431807,
            "unit": "ns/iter",
            "extra": "iterations: 360615\ncpu: 57878.653134229025 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_no_namespaces/min_time:15.000",
            "value": 280940.20025403856,
            "unit": "ns/iter",
            "extra": "iterations: 75579\ncpu: 280912.8395453763 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 309351.3961760252,
            "unit": "ns/iter",
            "extra": "iterations: 68149\ncpu: 309322.1852118153 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_interactions/min_time:15.000",
            "value": 551257.397442994,
            "unit": "ns/iter",
            "extra": "iterations: 37935\ncpu: 551207.1701594833 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_no_interactions/min_time:15.000",
            "value": 267295.4879411805,
            "unit": "ns/iter",
            "extra": "iterations: 74261\ncpu: 267233.5061472373 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_interactions/min_time:15.000",
            "value": 386315.72736924636,
            "unit": "ns/iter",
            "extra": "iterations: 53688\ncpu: 386284.9109670693 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_no_namespaces/min_time:15.000",
            "value": 529094.3515828357,
            "unit": "ns/iter",
            "extra": "iterations: 39581\ncpu: 529002.2788711757 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 683852.5386632514,
            "unit": "ns/iter",
            "extra": "iterations: 30507\ncpu: 683796.92857377 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_interactions/min_time:15.000",
            "value": 2156619.6896986254,
            "unit": "ns/iter",
            "extra": "iterations: 9523\ncpu: 2156224.782106475 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_no_interactions/min_time:15.000",
            "value": 555105.2125775296,
            "unit": "ns/iter",
            "extra": "iterations: 38052\ncpu: 555044.7781982559 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_interactions/min_time:15.000",
            "value": 1634276.3954935912,
            "unit": "ns/iter",
            "extra": "iterations: 12693\ncpu: 1633964.4213345926 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/simple/min_time:15.000",
            "value": 45827.948200898696,
            "unit": "ns/iter",
            "extra": "iterations: 468734\ncpu: 45820.180955509975 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/quadratic/min_time:15.000",
            "value": 2572328.99556158,
            "unit": "ns/iter",
            "extra": "iterations: 8111\ncpu: 2572118.382443589 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_string_fts",
            "value": 843.0590091426785,
            "unit": "ns/iter",
            "extra": "iterations: 4984431\ncpu: 843.0058716832542 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_string_fts",
            "value": 6888.138552349076,
            "unit": "ns/iter",
            "extra": "iterations: 609553\ncpu: 6887.761195499034 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_num_fts",
            "value": 636.4055098541859,
            "unit": "ns/iter",
            "extra": "iterations: 6599122\ncpu: 636.369292763495 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_num_fts",
            "value": 4919.893393854883,
            "unit": "ns/iter",
            "extra": "iterations: 855673\ncpu: 4917.533917746637 ns\nthreads: 1"
          },
          {
            "name": "benchmark_example_reuse",
            "value": 731.3196518821643,
            "unit": "ns/iter",
            "extra": "iterations: 5739206\ncpu: 731.2753366929146 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_char",
            "value": 46.56853993147063,
            "unit": "ns/iter",
            "extra": "iterations: 90172836\ncpu: 46.565135203244196 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_extent",
            "value": 65.62413640400928,
            "unit": "ns/iter",
            "extra": "iterations: 64685195\ncpu: 65.61967077628789 ns\nthreads: 1"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "jackgerrits@users.noreply.github.com",
            "name": "Jack Gerrits",
            "username": "jackgerrits"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "0aa74c1465627e04c45428c46b1546926e388938",
          "message": "fix: VW should not add anything to namespace std (#4230)\n\n* fix: VW should not add anything to namespace std\r\n\r\n* formatting",
          "timestamp": "2022-10-17T18:59:06-04:00",
          "tree_id": "b47bffff3122fbe43cadeca9785e0a037dd11289",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/0aa74c1465627e04c45428c46b1546926e388938"
        },
        "date": 1666050205832,
        "tool": "benchmarkdotnet",
        "benches": [
          {
            "name": "BenchmarkText.Benchmark(args: 120_num_features)",
            "value": 6415.560404459636,
            "unit": "ns",
            "range": "± 53.22274484654107"
          },
          {
            "name": "BenchmarkText.Benchmark(args: 120_string_fts)",
            "value": 9793.57393704928,
            "unit": "ns",
            "range": "± 36.933283360625744"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 1_feature)",
            "value": 605.1077270507812,
            "unit": "ns",
            "range": "± 1.1322132079484357"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 8_features)",
            "value": 498.2477442423503,
            "unit": "ns",
            "range": "± 2.863170093165768"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_interactions)",
            "value": 918052.9557291666,
            "unit": "ns",
            "range": "± 1807.2066173725098"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_no_interactions)",
            "value": 549126.7903645834,
            "unit": "ns",
            "range": "± 2214.5145993955202"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_no_namespaces)",
            "value": 542060.5794270834,
            "unit": "ns",
            "range": "± 2216.2326240631924"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_interactions)",
            "value": 892334.1080729166,
            "unit": "ns",
            "range": "± 1644.1887296308937"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_no_interactions)",
            "value": 536768.3138020834,
            "unit": "ns",
            "range": "± 2327.559287480133"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_interactions)",
            "value": 3472719.8177083335,
            "unit": "ns",
            "range": "± 5599.041114118454"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_no_interactions)",
            "value": 1072471.6796875,
            "unit": "ns",
            "range": "± 2412.562169185838"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_no_namespaces)",
            "value": 898384.9609375,
            "unit": "ns",
            "range": "± 1832.5725394154726"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_interactions)",
            "value": 2799201.339285714,
            "unit": "ns",
            "range": "± 4174.752502876969"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_no_interactions)",
            "value": 920589.2252604166,
            "unit": "ns",
            "range": "± 1122.4045612390773"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: few_features)",
            "value": 3185.041936238607,
            "unit": "ns",
            "range": "± 8.611023609442631"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: few_features)",
            "value": 11458.45947265625,
            "unit": "ns",
            "range": "± 16.76518857751724"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: many_features)",
            "value": 108371.45100911458,
            "unit": "ns",
            "range": "± 261.45952594005985"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: many_features)",
            "value": 19039.441789899553,
            "unit": "ns",
            "range": "± 24.52291651484934"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: quadratic)",
            "value": 4289844.661458333,
            "unit": "ns",
            "range": "± 19101.133744975046"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: simple)",
            "value": 358884.3961588542,
            "unit": "ns",
            "range": "± 729.4898717360423"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "jackgerrits@users.noreply.github.com",
            "name": "Jack Gerrits",
            "username": "jackgerrits"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "873c0de517bb2d6bfef2b2ffc71e6a7bc0f681f0",
          "message": "style: update style of label_type_t (#4229)\n\n* style: update style of label_type_t\r\n\r\n* formatting\r\n\r\n* fix test",
          "timestamp": "2022-10-18T10:13:01-04:00",
          "tree_id": "181657f338961a393e0acde3d443afd5274e5b53",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/873c0de517bb2d6bfef2b2ffc71e6a7bc0f681f0"
        },
        "date": 1666103874124,
        "tool": "googlecpp",
        "benches": [
          {
            "name": "bench_text/120_string_fts",
            "value": 7377.896190124613,
            "unit": "ns/iter",
            "extra": "iterations: 550333\ncpu: 7376.904710420781 ns\nthreads: 1"
          },
          {
            "name": "bench_text/120_num_fts",
            "value": 5668.521839983216,
            "unit": "ns/iter",
            "extra": "iterations: 738485\ncpu: 5667.609226998517 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/8_features",
            "value": 92.46598574588945,
            "unit": "ns/iter",
            "extra": "iterations: 44313819\ncpu: 92.45082216903943 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/1_feature",
            "value": 196.74882827413464,
            "unit": "ns/iter",
            "extra": "iterations: 21249211\ncpu: 196.7237983565601 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features",
            "value": 8243.970727217107,
            "unit": "ns/iter",
            "extra": "iterations: 504086\ncpu: 8242.624076050515 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features",
            "value": 14923.463565117096,
            "unit": "ns/iter",
            "extra": "iterations: 278291\ncpu: 14921.027629352004 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features_no_predict",
            "value": 3951.129557084854,
            "unit": "ns/iter",
            "extra": "iterations: 1064990\ncpu: 3950.5571883304065 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features_no_predic",
            "value": 6053.7252543143695,
            "unit": "ns/iter",
            "extra": "iterations: 684291\ncpu: 6052.929382382641 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/few_features",
            "value": 1452.8933138331206,
            "unit": "ns/iter",
            "extra": "iterations: 2864242\ncpu: 1452.7166698903222 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/many_features/min_time:15.000",
            "value": 69058.20330486081,
            "unit": "ns/iter",
            "extra": "iterations: 305792\ncpu: 69048.548686689 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_no_namespaces/min_time:15.000",
            "value": 340341.39787626953,
            "unit": "ns/iter",
            "extra": "iterations: 61778\ncpu: 340291.07934863545 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 370157.77395486407,
            "unit": "ns/iter",
            "extra": "iterations: 56763\ncpu: 370080.95942779636 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_interactions/min_time:15.000",
            "value": 639967.4123348229,
            "unit": "ns/iter",
            "extra": "iterations: 32088\ncpu: 639876.4522562948 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_no_interactions/min_time:15.000",
            "value": 315213.32093562564,
            "unit": "ns/iter",
            "extra": "iterations: 66437\ncpu: 315168.2541354968 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_interactions/min_time:15.000",
            "value": 486675.52472603816,
            "unit": "ns/iter",
            "extra": "iterations: 43072\ncpu: 486570.4843053491 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_no_namespaces/min_time:15.000",
            "value": 728169.2410829205,
            "unit": "ns/iter",
            "extra": "iterations: 28737\ncpu: 728028.2005776517 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 938214.4245780576,
            "unit": "ns/iter",
            "extra": "iterations: 22752\ncpu: 937981.2763713077 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_interactions/min_time:15.000",
            "value": 2972599.934505252,
            "unit": "ns/iter",
            "extra": "iterations: 7054\ncpu: 2972061.7947263927 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_no_interactions/min_time:15.000",
            "value": 753250.9219860662,
            "unit": "ns/iter",
            "extra": "iterations: 27854\ncpu: 753118.1553816333 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_interactions/min_time:15.000",
            "value": 2240205.7515004687,
            "unit": "ns/iter",
            "extra": "iterations: 9497\ncpu: 2239801.326734756 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/simple/min_time:15.000",
            "value": 61559.12458829865,
            "unit": "ns/iter",
            "extra": "iterations: 340963\ncpu: 61550.14649683411 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/quadratic/min_time:15.000",
            "value": 3198699.9223345695,
            "unit": "ns/iter",
            "extra": "iterations: 6528\ncpu: 3198207.3376225433 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_string_fts",
            "value": 1147.9621623331414,
            "unit": "ns/iter",
            "extra": "iterations: 3666558\ncpu: 1147.8134261069947 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_string_fts",
            "value": 9399.325539733252,
            "unit": "ns/iter",
            "extra": "iterations: 447119\ncpu: 9398.069865069465 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_num_fts",
            "value": 867.809493247659,
            "unit": "ns/iter",
            "extra": "iterations: 4834847\ncpu: 867.698005748682 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_num_fts",
            "value": 6683.454614963773,
            "unit": "ns/iter",
            "extra": "iterations: 626473\ncpu: 6682.057167667199 ns\nthreads: 1"
          },
          {
            "name": "benchmark_example_reuse",
            "value": 993.3938586481514,
            "unit": "ns/iter",
            "extra": "iterations: 4221253\ncpu: 993.2508428184663 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_char",
            "value": 63.44204378588615,
            "unit": "ns/iter",
            "extra": "iterations: 63104536\ncpu: 63.433337026675865 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_extent",
            "value": 88.39097069485739,
            "unit": "ns/iter",
            "extra": "iterations: 47504187\ncpu: 88.379685352788 ns\nthreads: 1"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "jackgerrits@users.noreply.github.com",
            "name": "Jack Gerrits",
            "username": "jackgerrits"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "873c0de517bb2d6bfef2b2ffc71e6a7bc0f681f0",
          "message": "style: update style of label_type_t (#4229)\n\n* style: update style of label_type_t\r\n\r\n* formatting\r\n\r\n* fix test",
          "timestamp": "2022-10-18T10:13:01-04:00",
          "tree_id": "181657f338961a393e0acde3d443afd5274e5b53",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/873c0de517bb2d6bfef2b2ffc71e6a7bc0f681f0"
        },
        "date": 1666105132097,
        "tool": "benchmarkdotnet",
        "benches": [
          {
            "name": "BenchmarkText.Benchmark(args: 120_num_features)",
            "value": 5264.946855817522,
            "unit": "ns",
            "range": "± 63.41374401891477"
          },
          {
            "name": "BenchmarkText.Benchmark(args: 120_string_fts)",
            "value": 7335.890361240932,
            "unit": "ns",
            "range": "± 50.041304435394714"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 1_feature)",
            "value": 571.0577901204427,
            "unit": "ns",
            "range": "± 5.578293051911935"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 8_features)",
            "value": 489.19716614943286,
            "unit": "ns",
            "range": "± 2.585722525494634"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_interactions)",
            "value": 874851.6322544643,
            "unit": "ns",
            "range": "± 4734.857505881613"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_no_interactions)",
            "value": 462465.1790364583,
            "unit": "ns",
            "range": "± 1085.802405500719"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_no_namespaces)",
            "value": 463323.14453125,
            "unit": "ns",
            "range": "± 559.0602407210094"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_interactions)",
            "value": 859642.6081730769,
            "unit": "ns",
            "range": "± 1711.241332592325"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_no_interactions)",
            "value": 461319.10226004466,
            "unit": "ns",
            "range": "± 1388.694991459135"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_interactions)",
            "value": 3447386.104910714,
            "unit": "ns",
            "range": "± 8477.67748929057"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_no_interactions)",
            "value": 1043940.0111607143,
            "unit": "ns",
            "range": "± 1412.7391923906525"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_no_namespaces)",
            "value": 863872.8816105769,
            "unit": "ns",
            "range": "± 2511.1141989742487"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_interactions)",
            "value": 2769129.717548077,
            "unit": "ns",
            "range": "± 6220.90231901926"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_no_interactions)",
            "value": 874615.5123197115,
            "unit": "ns",
            "range": "± 940.7180612063561"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: few_features)",
            "value": 3320.3978474934897,
            "unit": "ns",
            "range": "± 14.726981797536522"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: few_features)",
            "value": 11338.140462239584,
            "unit": "ns",
            "range": "± 33.052596224283505"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: many_features)",
            "value": 126497.67194475446,
            "unit": "ns",
            "range": "± 417.723185733758"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: many_features)",
            "value": 19600.772298177082,
            "unit": "ns",
            "range": "± 16.517771891118638"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: quadratic)",
            "value": 4880217.7734375,
            "unit": "ns",
            "range": "± 8986.927435879896"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: simple)",
            "value": 189014.30288461538,
            "unit": "ns",
            "range": "± 337.73989420417564"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "griffinbassman@gmail.com",
            "name": "Griffin Bassman",
            "username": "bassmang"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "28e7a79c231c3736f87da05921dbeedc08d6f8de",
          "message": "test: [epsilon decay] find champ change in simulator (#4228)\n\n* test: [epsilon decay] find champ change in simulator\r\n\r\n* clang\r\n\r\n* fix audit string\r\n\r\n* help string\r\n\r\n* clang",
          "timestamp": "2022-10-18T13:23:22-04:00",
          "tree_id": "a2121675475dc7c2ac2376faa10ab061ccb50133",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/28e7a79c231c3736f87da05921dbeedc08d6f8de"
        },
        "date": 1666115130271,
        "tool": "googlecpp",
        "benches": [
          {
            "name": "bench_text/120_string_fts",
            "value": 5938.012647335195,
            "unit": "ns/iter",
            "extra": "iterations: 706552\ncpu: 5934.244896341671 ns\nthreads: 1"
          },
          {
            "name": "bench_text/120_num_fts",
            "value": 4529.368383936526,
            "unit": "ns/iter",
            "extra": "iterations: 917464\ncpu: 4528.602103188789 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/8_features",
            "value": 77.27372589533907,
            "unit": "ns/iter",
            "extra": "iterations: 55213596\ncpu: 77.2686676665653 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/1_feature",
            "value": 165.49434882456876,
            "unit": "ns/iter",
            "extra": "iterations: 25380649\ncpu: 165.48472420858909 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features",
            "value": 6899.765001179134,
            "unit": "ns/iter",
            "extra": "iterations: 606369\ncpu: 6898.696833116467 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features",
            "value": 13082.648295886594,
            "unit": "ns/iter",
            "extra": "iterations: 328910\ncpu: 13081.611991122189 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features_no_predict",
            "value": 3256.0256754067054,
            "unit": "ns/iter",
            "extra": "iterations: 1276046\ncpu: 3255.618449491633 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features_no_predic",
            "value": 5000.375343401205,
            "unit": "ns/iter",
            "extra": "iterations: 846313\ncpu: 4996.942620519831 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/few_features",
            "value": 1192.3209852624268,
            "unit": "ns/iter",
            "extra": "iterations: 3535586\ncpu: 1192.2417387103576 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/many_features/min_time:15.000",
            "value": 61688.39784300736,
            "unit": "ns/iter",
            "extra": "iterations: 339825\ncpu: 61684.09298903849 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_no_namespaces/min_time:15.000",
            "value": 290452.3418116026,
            "unit": "ns/iter",
            "extra": "iterations: 75723\ncpu: 290397.93325673835 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 316987.2903917688,
            "unit": "ns/iter",
            "extra": "iterations: 66672\ncpu: 316962.9094672426 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_interactions/min_time:15.000",
            "value": 556755.1797014714,
            "unit": "ns/iter",
            "extra": "iterations: 37451\ncpu: 556708.055859657 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_no_interactions/min_time:15.000",
            "value": 293842.049039051,
            "unit": "ns/iter",
            "extra": "iterations: 71596\ncpu: 293784.14715905936 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_interactions/min_time:15.000",
            "value": 435804.3992381173,
            "unit": "ns/iter",
            "extra": "iterations: 48039\ncpu: 435767.9385499286 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_no_namespaces/min_time:15.000",
            "value": 598692.6285060302,
            "unit": "ns/iter",
            "extra": "iterations: 34512\ncpu: 598574.6407046823 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 784843.9276028157,
            "unit": "ns/iter",
            "extra": "iterations: 26990\ncpu: 784777.7399036676 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_interactions/min_time:15.000",
            "value": 2470905.3267989336,
            "unit": "ns/iter",
            "extra": "iterations: 8519\ncpu: 2470423.6060570497 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_no_interactions/min_time:15.000",
            "value": 607602.4752865017,
            "unit": "ns/iter",
            "extra": "iterations: 34293\ncpu: 607384.6440964639 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_interactions/min_time:15.000",
            "value": 1778010.6955842646,
            "unit": "ns/iter",
            "extra": "iterations: 11527\ncpu: 1777800.9022295468 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/simple/min_time:15.000",
            "value": 52458.116755570925,
            "unit": "ns/iter",
            "extra": "iterations: 404743\ncpu: 52454.43478948361 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/quadratic/min_time:15.000",
            "value": 2661536.7853323235,
            "unit": "ns/iter",
            "extra": "iterations: 7854\ncpu: 2661309.192768015 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_string_fts",
            "value": 962.1461611538058,
            "unit": "ns/iter",
            "extra": "iterations: 4366338\ncpu: 962.0814513214464 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_string_fts",
            "value": 6978.017731803893,
            "unit": "ns/iter",
            "extra": "iterations: 602082\ncpu: 6976.635408465928 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_num_fts",
            "value": 756.1688541158269,
            "unit": "ns/iter",
            "extra": "iterations: 5558834\ncpu: 755.7707785481684 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_num_fts",
            "value": 5174.3504346149775,
            "unit": "ns/iter",
            "extra": "iterations: 814169\ncpu: 5174.020872816331 ns\nthreads: 1"
          },
          {
            "name": "benchmark_example_reuse",
            "value": 787.8016176377989,
            "unit": "ns/iter",
            "extra": "iterations: 5333085\ncpu: 787.7478795106437 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_char",
            "value": 52.28485391657412,
            "unit": "ns/iter",
            "extra": "iterations: 80295933\ncpu: 52.28197049531741 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_extent",
            "value": 76.30017161071098,
            "unit": "ns/iter",
            "extra": "iterations: 55011718\ncpu: 76.29560851017158 ns\nthreads: 1"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "griffinbassman@gmail.com",
            "name": "Griffin Bassman",
            "username": "bassmang"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "28e7a79c231c3736f87da05921dbeedc08d6f8de",
          "message": "test: [epsilon decay] find champ change in simulator (#4228)\n\n* test: [epsilon decay] find champ change in simulator\r\n\r\n* clang\r\n\r\n* fix audit string\r\n\r\n* help string\r\n\r\n* clang",
          "timestamp": "2022-10-18T13:23:22-04:00",
          "tree_id": "a2121675475dc7c2ac2376faa10ab061ccb50133",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/28e7a79c231c3736f87da05921dbeedc08d6f8de"
        },
        "date": 1666116413772,
        "tool": "benchmarkdotnet",
        "benches": [
          {
            "name": "BenchmarkText.Benchmark(args: 120_num_features)",
            "value": 6695.519722832574,
            "unit": "ns",
            "range": "± 142.14619791442743"
          },
          {
            "name": "BenchmarkText.Benchmark(args: 120_string_fts)",
            "value": 9776.47226969401,
            "unit": "ns",
            "range": "± 176.12689491946827"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 1_feature)",
            "value": 717.697823432184,
            "unit": "ns",
            "range": "± 21.970256042981624"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 8_features)",
            "value": 614.3906422150441,
            "unit": "ns",
            "range": "± 21.206761741757365"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_interactions)",
            "value": 1109411.379076087,
            "unit": "ns",
            "range": "± 27889.45733625845"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_no_interactions)",
            "value": 700990.1227678572,
            "unit": "ns",
            "range": "± 10731.91170721628"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_no_namespaces)",
            "value": 657244.5999710648,
            "unit": "ns",
            "range": "± 18417.043582995975"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_interactions)",
            "value": 1056197.972470238,
            "unit": "ns",
            "range": "± 23883.462005736124"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_no_interactions)",
            "value": 664736.9698660715,
            "unit": "ns",
            "range": "± 6914.851561900878"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_interactions)",
            "value": 4530686.1083984375,
            "unit": "ns",
            "range": "± 136570.03346342812"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_no_interactions)",
            "value": 1334797.2265625,
            "unit": "ns",
            "range": "± 15214.22376366532"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_no_namespaces)",
            "value": 1142550.9765625,
            "unit": "ns",
            "range": "± 12254.36112916377"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_interactions)",
            "value": 3989282.9358552634,
            "unit": "ns",
            "range": "± 84805.76012780044"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_no_interactions)",
            "value": 1179875.5859375,
            "unit": "ns",
            "range": "± 14070.50178972392"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: few_features)",
            "value": 4072.2020975748696,
            "unit": "ns",
            "range": "± 70.61074168184898"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: few_features)",
            "value": 14017.060307094029,
            "unit": "ns",
            "range": "± 130.82321976069554"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: many_features)",
            "value": 164257.9345703125,
            "unit": "ns",
            "range": "± 3060.3962897637857"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: many_features)",
            "value": 24199.418470594617,
            "unit": "ns",
            "range": "± 669.11991278904"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: quadratic)",
            "value": 5998541.40625,
            "unit": "ns",
            "range": "± 80953.7602712454"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: simple)",
            "value": 307990.65946691175,
            "unit": "ns",
            "range": "± 6122.167292466214"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "jackgerrits@users.noreply.github.com",
            "name": "Jack Gerrits",
            "username": "jackgerrits"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "018067389f21ff46df66fa68a3062ac17b84ad98",
          "message": "refactor: reduce build time (#4232)\n\n* refactor: reduce build time\r\n\r\n* add back as private\r\n\r\n* update header\r\n\r\n* add missing include\r\n\r\n* add include\r\n\r\n* change format signatuer\r\n\r\n* build fix\r\n\r\n* add header\r\n\r\n* include winsock2 first\r\n\r\n* add include\r\n\r\n* include headers\r\n\r\n* fixes\r\n\r\n* las fixes\r\n\r\n* add header\r\n\r\n* Update vowpalwabbit.cpp\r\n\r\n* #define NOMINMAX\r\n\r\n* header\r\n\r\n* includes\r\n\r\n* includes\r\n\r\n* Update gen_cs_example.h",
          "timestamp": "2022-10-19T10:44:58-04:00",
          "tree_id": "8d43e2c3d31be81d98fd8dc23d8e36a076e809e1",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/018067389f21ff46df66fa68a3062ac17b84ad98"
        },
        "date": 1666191557501,
        "tool": "googlecpp",
        "benches": [
          {
            "name": "bench_text/120_string_fts",
            "value": 8312.280874354265,
            "unit": "ns/iter",
            "extra": "iterations: 497144\ncpu: 8311.669455932286 ns\nthreads: 1"
          },
          {
            "name": "bench_text/120_num_fts",
            "value": 5630.804442691099,
            "unit": "ns/iter",
            "extra": "iterations: 742703\ncpu: 5621.8380698610345 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/8_features",
            "value": 95.81124392584556,
            "unit": "ns/iter",
            "extra": "iterations: 45514779\ncpu: 95.79624675316998 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/1_feature",
            "value": 203.39835878266965,
            "unit": "ns/iter",
            "extra": "iterations: 21029756\ncpu: 203.3828542756274 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features",
            "value": 8393.685424014107,
            "unit": "ns/iter",
            "extra": "iterations: 501599\ncpu: 8392.900703550044 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features",
            "value": 14774.862521371528,
            "unit": "ns/iter",
            "extra": "iterations: 281331\ncpu: 14773.49954324266 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features_no_predict",
            "value": 4001.04972173118,
            "unit": "ns/iter",
            "extra": "iterations: 1055454\ncpu: 4000.7500090008625 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features_no_predic",
            "value": 5974.308311349968,
            "unit": "ns/iter",
            "extra": "iterations: 696361\ncpu: 5973.913961293072 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/few_features",
            "value": 1480.0235615994718,
            "unit": "ns/iter",
            "extra": "iterations: 2853117\ncpu: 1479.9144935170912 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/many_features/min_time:15.000",
            "value": 71324.60090539596,
            "unit": "ns/iter",
            "extra": "iterations: 294236\ncpu: 71319.25631126037 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_no_namespaces/min_time:15.000",
            "value": 297470.1857903106,
            "unit": "ns/iter",
            "extra": "iterations: 72345\ncpu: 297444.7425530444 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 331621.8556479695,
            "unit": "ns/iter",
            "extra": "iterations: 62881\ncpu: 331592.1645648128 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_interactions/min_time:15.000",
            "value": 615268.1136799271,
            "unit": "ns/iter",
            "extra": "iterations: 33911\ncpu: 615215.7824894572 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_no_interactions/min_time:15.000",
            "value": 298433.9481961024,
            "unit": "ns/iter",
            "extra": "iterations: 70902\ncpu: 298399.57970155997 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_interactions/min_time:15.000",
            "value": 472074.3997565232,
            "unit": "ns/iter",
            "extra": "iterations: 44357\ncpu: 472035.060982483 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_no_namespaces/min_time:15.000",
            "value": 715751.1733287742,
            "unit": "ns/iter",
            "extra": "iterations: 29245\ncpu: 715680.6838775857 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 938865.0515677298,
            "unit": "ns/iter",
            "extra": "iterations: 22708\ncpu: 938768.3767835131 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_interactions/min_time:15.000",
            "value": 2920937.753262988,
            "unit": "ns/iter",
            "extra": "iterations: 7202\ncpu: 2920522.0633157473 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_no_interactions/min_time:15.000",
            "value": 748177.1398428068,
            "unit": "ns/iter",
            "extra": "iterations: 28246\ncpu: 748088.7559300433 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_interactions/min_time:15.000",
            "value": 2270742.5096928873,
            "unit": "ns/iter",
            "extra": "iterations: 9182\ncpu: 2270484.4478327185 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/simple/min_time:15.000",
            "value": 61591.005747702584,
            "unit": "ns/iter",
            "extra": "iterations: 339092\ncpu: 61585.164793035496 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/quadratic/min_time:15.000",
            "value": 3208242.1818322213,
            "unit": "ns/iter",
            "extra": "iterations: 6473\ncpu: 3207902.332766878 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_string_fts",
            "value": 1176.5123480358811,
            "unit": "ns/iter",
            "extra": "iterations: 3569839\ncpu: 1176.4138942960733 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_string_fts",
            "value": 10336.511691718584,
            "unit": "ns/iter",
            "extra": "iterations: 406356\ncpu: 10335.733445550146 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_num_fts",
            "value": 1003.6401004478614,
            "unit": "ns/iter",
            "extra": "iterations: 4185654\ncpu: 1003.5531364991022 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_num_fts",
            "value": 6770.3512490366475,
            "unit": "ns/iter",
            "extra": "iterations: 618917\ncpu: 6769.778984904204 ns\nthreads: 1"
          },
          {
            "name": "benchmark_example_reuse",
            "value": 940.0400040721876,
            "unit": "ns/iter",
            "extra": "iterations: 4449697\ncpu: 939.9563610735762 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_char",
            "value": 63.762867810113704,
            "unit": "ns/iter",
            "extra": "iterations: 65985373\ncpu: 63.75724814043262 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_extent",
            "value": 92.35457381835283,
            "unit": "ns/iter",
            "extra": "iterations: 45471843\ncpu: 92.34703330586356 ns\nthreads: 1"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "jackgerrits@users.noreply.github.com",
            "name": "Jack Gerrits",
            "username": "jackgerrits"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "018067389f21ff46df66fa68a3062ac17b84ad98",
          "message": "refactor: reduce build time (#4232)\n\n* refactor: reduce build time\r\n\r\n* add back as private\r\n\r\n* update header\r\n\r\n* add missing include\r\n\r\n* add include\r\n\r\n* change format signatuer\r\n\r\n* build fix\r\n\r\n* add header\r\n\r\n* include winsock2 first\r\n\r\n* add include\r\n\r\n* include headers\r\n\r\n* fixes\r\n\r\n* las fixes\r\n\r\n* add header\r\n\r\n* Update vowpalwabbit.cpp\r\n\r\n* #define NOMINMAX\r\n\r\n* header\r\n\r\n* includes\r\n\r\n* includes\r\n\r\n* Update gen_cs_example.h",
          "timestamp": "2022-10-19T10:44:58-04:00",
          "tree_id": "8d43e2c3d31be81d98fd8dc23d8e36a076e809e1",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/018067389f21ff46df66fa68a3062ac17b84ad98"
        },
        "date": 1666193041224,
        "tool": "benchmarkdotnet",
        "benches": [
          {
            "name": "BenchmarkText.Benchmark(args: 120_num_features)",
            "value": 5132.270685831706,
            "unit": "ns",
            "range": "± 16.445362580545993"
          },
          {
            "name": "BenchmarkText.Benchmark(args: 120_string_fts)",
            "value": 7322.75648850661,
            "unit": "ns",
            "range": "± 6.504594130417045"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 1_feature)",
            "value": 686.1515744527181,
            "unit": "ns",
            "range": "± 1.1688920581075393"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 8_features)",
            "value": 471.74271436838,
            "unit": "ns",
            "range": "± 1.2311099174101041"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_interactions)",
            "value": 854585.99609375,
            "unit": "ns",
            "range": "± 1624.5013131201476"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_no_interactions)",
            "value": 473392.3940805289,
            "unit": "ns",
            "range": "± 1181.7701493249938"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_no_namespaces)",
            "value": 465138.3430989583,
            "unit": "ns",
            "range": "± 1044.9728863462967"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_interactions)",
            "value": 908337.2395833334,
            "unit": "ns",
            "range": "± 1917.0192773650745"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_no_interactions)",
            "value": 462677.5130208333,
            "unit": "ns",
            "range": "± 1138.340043049022"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_interactions)",
            "value": 3523548.046875,
            "unit": "ns",
            "range": "± 4794.192304818553"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_no_interactions)",
            "value": 1058811.11328125,
            "unit": "ns",
            "range": "± 1297.6765605268488"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_no_namespaces)",
            "value": 880978.2649739584,
            "unit": "ns",
            "range": "± 1036.3423107646302"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_interactions)",
            "value": 2783690.2864583335,
            "unit": "ns",
            "range": "± 5226.857610121126"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_no_interactions)",
            "value": 900620.4241071428,
            "unit": "ns",
            "range": "± 920.8286175706596"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: few_features)",
            "value": 3300.579503377279,
            "unit": "ns",
            "range": "± 6.149856794090146"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: few_features)",
            "value": 10940.908432006836,
            "unit": "ns",
            "range": "± 9.439845049512204"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: many_features)",
            "value": 129079.45963541667,
            "unit": "ns",
            "range": "± 177.17461407868862"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: many_features)",
            "value": 19286.259024483817,
            "unit": "ns",
            "range": "± 26.697951382414633"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: quadratic)",
            "value": 4815133.385416667,
            "unit": "ns",
            "range": "± 18064.22093797393"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: simple)",
            "value": 202234.29036458334,
            "unit": "ns",
            "range": "± 813.9726512784137"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "olgavrou@gmail.com",
            "name": "olgavrou",
            "username": "olgavrou"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "74340888de68ab53a5ad10e176c454aa12ca678c",
          "message": "feat: [LAS] filter out (potentially) more actions than d based on singular values (#4234)",
          "timestamp": "2022-10-20T09:33:48-04:00",
          "tree_id": "8dfabbe6d601d63de40a1a784ae306407028fbca",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/74340888de68ab53a5ad10e176c454aa12ca678c"
        },
        "date": 1666273919995,
        "tool": "googlecpp",
        "benches": [
          {
            "name": "bench_text/120_string_fts",
            "value": 6934.822069090203,
            "unit": "ns/iter",
            "extra": "iterations: 608045\ncpu: 6933.44653767402 ns\nthreads: 1"
          },
          {
            "name": "bench_text/120_num_fts",
            "value": 4681.5510823665245,
            "unit": "ns/iter",
            "extra": "iterations: 899880\ncpu: 4681.141041027692 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/8_features",
            "value": 77.65291560564138,
            "unit": "ns/iter",
            "extra": "iterations: 55941962\ncpu: 77.64836707014317 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/1_feature",
            "value": 166.93898613987312,
            "unit": "ns/iter",
            "extra": "iterations: 24844650\ncpu: 166.92823605886989 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features",
            "value": 7015.4406684209,
            "unit": "ns/iter",
            "extra": "iterations: 596630\ncpu: 7014.854432395288 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features",
            "value": 12445.046057886644,
            "unit": "ns/iter",
            "extra": "iterations: 334275\ncpu: 12438.498840774811 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features_no_predict",
            "value": 3343.3764956452533,
            "unit": "ns/iter",
            "extra": "iterations: 1247037\ncpu: 3343.1603873822514 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features_no_predic",
            "value": 5036.222230662361,
            "unit": "ns/iter",
            "extra": "iterations: 829368\ncpu: 5035.92928591409 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/few_features",
            "value": 1229.1526325666475,
            "unit": "ns/iter",
            "extra": "iterations: 3410360\ncpu: 1229.0816805263967 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/many_features/min_time:15.000",
            "value": 61955.812156640015,
            "unit": "ns/iter",
            "extra": "iterations: 338202\ncpu: 61950.39916972697 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_no_namespaces/min_time:15.000",
            "value": 257467.0444519412,
            "unit": "ns/iter",
            "extra": "iterations: 85958\ncpu: 257418.67307289605 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 290993.81571142783,
            "unit": "ns/iter",
            "extra": "iterations: 72495\ncpu: 290967.2253258846 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_interactions/min_time:15.000",
            "value": 529039.0963273923,
            "unit": "ns/iter",
            "extra": "iterations: 39781\ncpu: 528942.9828309998 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_no_interactions/min_time:15.000",
            "value": 258479.65858797502,
            "unit": "ns/iter",
            "extra": "iterations: 81160\ncpu: 258455.945046821 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_interactions/min_time:15.000",
            "value": 409035.5734948186,
            "unit": "ns/iter",
            "extra": "iterations: 51439\ncpu: 408997.7954470345 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_no_namespaces/min_time:15.000",
            "value": 634978.6443088802,
            "unit": "ns/iter",
            "extra": "iterations: 33113\ncpu: 634857.1316401415 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 771719.1041858773,
            "unit": "ns/iter",
            "extra": "iterations: 27115\ncpu: 771639.8746081498 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_interactions/min_time:15.000",
            "value": 2493466.7050112532,
            "unit": "ns/iter",
            "extra": "iterations: 8441\ncpu: 2493000.5804999415 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_no_interactions/min_time:15.000",
            "value": 619015.8315666922,
            "unit": "ns/iter",
            "extra": "iterations: 33491\ncpu: 618957.015317549 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_interactions/min_time:15.000",
            "value": 1905394.4441431647,
            "unit": "ns/iter",
            "extra": "iterations: 11064\ncpu: 1905032.6193058572 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/simple/min_time:15.000",
            "value": 49040.565817189265,
            "unit": "ns/iter",
            "extra": "iterations: 428695\ncpu: 49036.84274367554 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/quadratic/min_time:15.000",
            "value": 2921219.368746485,
            "unit": "ns/iter",
            "extra": "iterations: 7116\ncpu: 2920972.8499156754 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_string_fts",
            "value": 967.1970129973002,
            "unit": "ns/iter",
            "extra": "iterations: 4350515\ncpu: 966.7561426635791 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_string_fts",
            "value": 8614.157438899709,
            "unit": "ns/iter",
            "extra": "iterations: 487478\ncpu: 8613.551175642766 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_num_fts",
            "value": 814.7770708694636,
            "unit": "ns/iter",
            "extra": "iterations: 5161914\ncpu: 814.720237493302 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_num_fts",
            "value": 5636.339279693071,
            "unit": "ns/iter",
            "extra": "iterations: 747348\ncpu: 5635.929981748855 ns\nthreads: 1"
          },
          {
            "name": "benchmark_example_reuse",
            "value": 788.4744795109567,
            "unit": "ns/iter",
            "extra": "iterations: 5323311\ncpu: 788.4120240203922 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_char",
            "value": 53.088868860690745,
            "unit": "ns/iter",
            "extra": "iterations: 78918723\ncpu: 53.08507589510788 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_extent",
            "value": 77.07084662555255,
            "unit": "ns/iter",
            "extra": "iterations: 54487394\ncpu: 77.06565669116019 ns\nthreads: 1"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "olgavrou@gmail.com",
            "name": "olgavrou",
            "username": "olgavrou"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "74340888de68ab53a5ad10e176c454aa12ca678c",
          "message": "feat: [LAS] filter out (potentially) more actions than d based on singular values (#4234)",
          "timestamp": "2022-10-20T09:33:48-04:00",
          "tree_id": "8dfabbe6d601d63de40a1a784ae306407028fbca",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/74340888de68ab53a5ad10e176c454aa12ca678c"
        },
        "date": 1666275517483,
        "tool": "benchmarkdotnet",
        "benches": [
          {
            "name": "BenchmarkText.Benchmark(args: 120_num_features)",
            "value": 7305.542958577474,
            "unit": "ns",
            "range": "± 136.17390212044688"
          },
          {
            "name": "BenchmarkText.Benchmark(args: 120_string_fts)",
            "value": 11007.54508972168,
            "unit": "ns",
            "range": "± 248.76467219793182"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 1_feature)",
            "value": 778.2607841491699,
            "unit": "ns",
            "range": "± 11.529818695425632"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 8_features)",
            "value": 593.1885583060129,
            "unit": "ns",
            "range": "± 7.669283687899501"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_interactions)",
            "value": 1042290.6467013889,
            "unit": "ns",
            "range": "± 21890.18966887236"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_no_interactions)",
            "value": 644755.44921875,
            "unit": "ns",
            "range": "± 9260.43306106652"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_no_namespaces)",
            "value": 633443.9518229166,
            "unit": "ns",
            "range": "± 7234.52846426394"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_interactions)",
            "value": 1051830.5245535714,
            "unit": "ns",
            "range": "± 10771.348055146294"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_no_interactions)",
            "value": 650572.3567708334,
            "unit": "ns",
            "range": "± 7674.382336825421"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_interactions)",
            "value": 4157097.6041666665,
            "unit": "ns",
            "range": "± 76756.77754804057"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_no_interactions)",
            "value": 1279230.1060267857,
            "unit": "ns",
            "range": "± 13302.359689231082"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_no_namespaces)",
            "value": 1067705.678013393,
            "unit": "ns",
            "range": "± 18555.289640954587"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_interactions)",
            "value": 3351703.3854166665,
            "unit": "ns",
            "range": "± 47329.29957798565"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_no_interactions)",
            "value": 1090342.9947916667,
            "unit": "ns",
            "range": "± 13884.945937081731"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: few_features)",
            "value": 3795.1809692382812,
            "unit": "ns",
            "range": "± 63.3814929468256"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: few_features)",
            "value": 13052.962384905133,
            "unit": "ns",
            "range": "± 119.20716474701054"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: many_features)",
            "value": 129652.38444010417,
            "unit": "ns",
            "range": "± 2107.165870560075"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: many_features)",
            "value": 22620.050252278645,
            "unit": "ns",
            "range": "± 248.74105432962887"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: quadratic)",
            "value": 5030628.489583333,
            "unit": "ns",
            "range": "± 74240.91495853667"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: simple)",
            "value": 235660.40201822916,
            "unit": "ns",
            "range": "± 3793.300200366207"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "jackgerrits@users.noreply.github.com",
            "name": "Jack Gerrits",
            "username": "jackgerrits"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "1666d3d7ed4a5448b05527b35c6aa619998cc3e3",
          "message": "build: do not add sse flags when doing MacOS arm cross build (#4235)\n\n* build: do not add sse flags when doing MacOS arm cross build\r\n\r\n* fix var\r\n\r\n* fix\r\n\r\n* try new fix",
          "timestamp": "2022-10-20T12:23:26-04:00",
          "tree_id": "545828e1c96ee859ccd8cbfae5f3e81083379a2c",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/1666d3d7ed4a5448b05527b35c6aa619998cc3e3"
        },
        "date": 1666283743338,
        "tool": "googlecpp",
        "benches": [
          {
            "name": "bench_text/120_string_fts",
            "value": 5953.399909550949,
            "unit": "ns/iter",
            "extra": "iterations: 714214\ncpu: 5950.12335238458 ns\nthreads: 1"
          },
          {
            "name": "bench_text/120_num_fts",
            "value": 4079.4087217993247,
            "unit": "ns/iter",
            "extra": "iterations: 1056227\ncpu: 4077.8736957112437 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/8_features",
            "value": 69.36723037624685,
            "unit": "ns/iter",
            "extra": "iterations: 60961237\ncpu: 69.36380900538485 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/1_feature",
            "value": 163.84859455772224,
            "unit": "ns/iter",
            "extra": "iterations: 25686078\ncpu: 163.83980847523708 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features",
            "value": 7371.681019770081,
            "unit": "ns/iter",
            "extra": "iterations: 567775\ncpu: 7370.872440667519 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features",
            "value": 12907.666091306915,
            "unit": "ns/iter",
            "extra": "iterations: 325014\ncpu: 12906.863704332733 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features_no_predict",
            "value": 3326.8818447325607,
            "unit": "ns/iter",
            "extra": "iterations: 1268238\ncpu: 3326.7047667709057 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features_no_predic",
            "value": 5108.69907049674,
            "unit": "ns/iter",
            "extra": "iterations: 832165\ncpu: 5108.42429085578 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/few_features",
            "value": 1239.3959013787646,
            "unit": "ns/iter",
            "extra": "iterations: 3366010\ncpu: 1239.3318201669042 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/many_features/min_time:15.000",
            "value": 52215.090856037394,
            "unit": "ns/iter",
            "extra": "iterations: 402681\ncpu: 52206.40904338672 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_no_namespaces/min_time:15.000",
            "value": 195413.76871552103,
            "unit": "ns/iter",
            "extra": "iterations: 107491\ncpu: 195402.14436557476 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 226361.12733916868,
            "unit": "ns/iter",
            "extra": "iterations: 92768\ncpu: 226345.5006036565 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_interactions/min_time:15.000",
            "value": 469021.6949818947,
            "unit": "ns/iter",
            "extra": "iterations: 45017\ncpu: 468956.66970255674 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_no_interactions/min_time:15.000",
            "value": 198169.8188836666,
            "unit": "ns/iter",
            "extra": "iterations: 105954\ncpu: 198157.51741321714 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_interactions/min_time:15.000",
            "value": 340695.9652537839,
            "unit": "ns/iter",
            "extra": "iterations: 61647\ncpu: 340650.2230441058 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_no_namespaces/min_time:15.000",
            "value": 605512.9167289248,
            "unit": "ns/iter",
            "extra": "iterations: 34802\ncpu: 605473.8377104762 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 742667.3187974097,
            "unit": "ns/iter",
            "extra": "iterations: 28871\ncpu: 742565.041044647 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_interactions/min_time:15.000",
            "value": 2291667.705758809,
            "unit": "ns/iter",
            "extra": "iterations: 9047\ncpu: 2291513.595667069 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_no_interactions/min_time:15.000",
            "value": 618411.5073792685,
            "unit": "ns/iter",
            "extra": "iterations: 34353\ncpu: 618372.8640875607 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_interactions/min_time:15.000",
            "value": 1809791.2701504272,
            "unit": "ns/iter",
            "extra": "iterations: 11501\ncpu: 1809668.6896791593 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/simple/min_time:15.000",
            "value": 50255.7350299294,
            "unit": "ns/iter",
            "extra": "iterations: 418485\ncpu: 50253.112297931904 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/quadratic/min_time:15.000",
            "value": 2403507.3542174324,
            "unit": "ns/iter",
            "extra": "iterations: 8619\ncpu: 2403038.7283907663 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_string_fts",
            "value": 900.2673888992738,
            "unit": "ns/iter",
            "extra": "iterations: 4663133\ncpu: 900.2228330180601 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_string_fts",
            "value": 7842.103408561828,
            "unit": "ns/iter",
            "extra": "iterations: 534859\ncpu: 7841.677713191741 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_num_fts",
            "value": 731.0567951986354,
            "unit": "ns/iter",
            "extra": "iterations: 6103773\ncpu: 731.017847485484 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_num_fts",
            "value": 5091.168511058467,
            "unit": "ns/iter",
            "extra": "iterations: 824391\ncpu: 5090.582381418502 ns\nthreads: 1"
          },
          {
            "name": "benchmark_example_reuse",
            "value": 799.472572775448,
            "unit": "ns/iter",
            "extra": "iterations: 5249401\ncpu: 799.4274013358813 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_char",
            "value": 39.81842092723018,
            "unit": "ns/iter",
            "extra": "iterations: 105472226\ncpu: 39.816173027389944 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_extent",
            "value": 56.693102504782495,
            "unit": "ns/iter",
            "extra": "iterations: 74725996\ncpu: 56.69026880551678 ns\nthreads: 1"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "jackgerrits@users.noreply.github.com",
            "name": "Jack Gerrits",
            "username": "jackgerrits"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "1666d3d7ed4a5448b05527b35c6aa619998cc3e3",
          "message": "build: do not add sse flags when doing MacOS arm cross build (#4235)\n\n* build: do not add sse flags when doing MacOS arm cross build\r\n\r\n* fix var\r\n\r\n* fix\r\n\r\n* try new fix",
          "timestamp": "2022-10-20T12:23:26-04:00",
          "tree_id": "545828e1c96ee859ccd8cbfae5f3e81083379a2c",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/1666d3d7ed4a5448b05527b35c6aa619998cc3e3"
        },
        "date": 1666285703733,
        "tool": "benchmarkdotnet",
        "benches": [
          {
            "name": "BenchmarkText.Benchmark(args: 120_num_features)",
            "value": 6238.404897054036,
            "unit": "ns",
            "range": "± 66.55123096999063"
          },
          {
            "name": "BenchmarkText.Benchmark(args: 120_string_fts)",
            "value": 9289.717211042132,
            "unit": "ns",
            "range": "± 69.66595415823633"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 1_feature)",
            "value": 647.726624806722,
            "unit": "ns",
            "range": "± 4.184057822779692"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 8_features)",
            "value": 498.7176554543631,
            "unit": "ns",
            "range": "± 2.5551332139015495"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_interactions)",
            "value": 912694.9068509615,
            "unit": "ns",
            "range": "± 2996.6887141618627"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_no_interactions)",
            "value": 577170.0823102678,
            "unit": "ns",
            "range": "± 2882.043561775919"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_no_namespaces)",
            "value": 572575.5598958334,
            "unit": "ns",
            "range": "± 3883.3482361801284"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_interactions)",
            "value": 910845.9049479166,
            "unit": "ns",
            "range": "± 5033.877968610248"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_no_interactions)",
            "value": 566509.6749441965,
            "unit": "ns",
            "range": "± 3196.8246595893856"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_interactions)",
            "value": 3550922.9166666665,
            "unit": "ns",
            "range": "± 15109.748699450834"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_no_interactions)",
            "value": 1124118.1039663462,
            "unit": "ns",
            "range": "± 7703.709977016078"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_no_namespaces)",
            "value": 937942.28515625,
            "unit": "ns",
            "range": "± 5048.565843809504"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_interactions)",
            "value": 2851199.90234375,
            "unit": "ns",
            "range": "± 5253.5942559151845"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_no_interactions)",
            "value": 971998.3528645834,
            "unit": "ns",
            "range": "± 3909.5114329859985"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: few_features)",
            "value": 3297.3262786865234,
            "unit": "ns",
            "range": "± 14.877922288914192"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: few_features)",
            "value": 11555.053202311197,
            "unit": "ns",
            "range": "± 63.446928780175035"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: many_features)",
            "value": 109376.05631510417,
            "unit": "ns",
            "range": "± 475.9006557534494"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: many_features)",
            "value": 19238.128226143974,
            "unit": "ns",
            "range": "± 67.17676060103081"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: quadratic)",
            "value": 4222239.732142857,
            "unit": "ns",
            "range": "± 26557.434112616957"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: simple)",
            "value": 206776.7236328125,
            "unit": "ns",
            "range": "± 1244.1918700966355"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "griffinbassman@gmail.com",
            "name": "Griffin Bassman",
            "username": "bassmang"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "1ef6de2982884ba387ea7738f878742a5fa38991",
          "message": "style: update label type to all caps (#4236)\n\n* style: update label type to all caps\r\n\r\n* fix break\r\n\r\n* change stderr\r\n\r\n* cb_eval fix",
          "timestamp": "2022-10-20T15:06:02-04:00",
          "tree_id": "3b91bfbb4d8a5ac27eeb567ad924e95e1ae2075d",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/1ef6de2982884ba387ea7738f878742a5fa38991"
        },
        "date": 1666293579864,
        "tool": "googlecpp",
        "benches": [
          {
            "name": "bench_text/120_string_fts",
            "value": 8260.637347761947,
            "unit": "ns/iter",
            "extra": "iterations: 507511\ncpu: 8257.112850755944 ns\nthreads: 1"
          },
          {
            "name": "bench_text/120_num_fts",
            "value": 5555.033455012866,
            "unit": "ns/iter",
            "extra": "iterations: 754715\ncpu: 5554.518460610959 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/8_features",
            "value": 88.10083017517285,
            "unit": "ns/iter",
            "extra": "iterations: 47958312\ncpu: 88.09439331392645 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/1_feature",
            "value": 201.954143797806,
            "unit": "ns/iter",
            "extra": "iterations: 21286041\ncpu: 201.9404078005863 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features",
            "value": 8257.598909096709,
            "unit": "ns/iter",
            "extra": "iterations: 502886\ncpu: 8256.670895590654 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features",
            "value": 14159.779112779803,
            "unit": "ns/iter",
            "extra": "iterations: 288722\ncpu: 14158.480129674916 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features_no_predict",
            "value": 3948.0224692131333,
            "unit": "ns/iter",
            "extra": "iterations: 1066526\ncpu: 3947.609903556032 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features_no_predic",
            "value": 5933.859913990824,
            "unit": "ns/iter",
            "extra": "iterations: 697600\ncpu: 5933.3608084862335 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/few_features",
            "value": 1456.9920171060435,
            "unit": "ns/iter",
            "extra": "iterations: 2888802\ncpu: 1456.876829910807 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/many_features/min_time:15.000",
            "value": 71839.61860073137,
            "unit": "ns/iter",
            "extra": "iterations: 296655\ncpu: 71824.53590871552 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_no_namespaces/min_time:15.000",
            "value": 275967.18024137715,
            "unit": "ns/iter",
            "extra": "iterations: 75815\ncpu: 275940.07122601086 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 315014.63326360757,
            "unit": "ns/iter",
            "extra": "iterations: 66451\ncpu: 314951.3069780739 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_interactions/min_time:15.000",
            "value": 604805.0159533189,
            "unit": "ns/iter",
            "extra": "iterations: 34789\ncpu: 604747.3023082012 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_no_interactions/min_time:15.000",
            "value": 278800.75915135903,
            "unit": "ns/iter",
            "extra": "iterations: 74142\ncpu: 278773.27965255873 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_interactions/min_time:15.000",
            "value": 460327.2374573911,
            "unit": "ns/iter",
            "extra": "iterations: 45764\ncpu: 460238.7575386766 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_no_namespaces/min_time:15.000",
            "value": 695929.5482795391,
            "unit": "ns/iter",
            "extra": "iterations: 29992\ncpu: 695856.181648439 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 917040.4118793028,
            "unit": "ns/iter",
            "extra": "iterations: 23099\ncpu: 916853.7599030259 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_interactions/min_time:15.000",
            "value": 2836952.244578321,
            "unit": "ns/iter",
            "extra": "iterations: 7470\ncpu: 2836560.78982597 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_no_interactions/min_time:15.000",
            "value": 722398.6969739724,
            "unit": "ns/iter",
            "extra": "iterations: 28354\ncpu: 722248.0884531278 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_interactions/min_time:15.000",
            "value": 2114168.5494516413,
            "unit": "ns/iter",
            "extra": "iterations: 10030\ncpu: 2113936.939182453 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/simple/min_time:15.000",
            "value": 61511.71264841518,
            "unit": "ns/iter",
            "extra": "iterations: 344557\ncpu: 61499.89871051823 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/quadratic/min_time:15.000",
            "value": 3136513.242798975,
            "unit": "ns/iter",
            "extra": "iterations: 6631\ncpu: 3136213.708339616 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_string_fts",
            "value": 1138.3117624903807,
            "unit": "ns/iter",
            "extra": "iterations: 3679952\ncpu: 1138.2214224533352 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_string_fts",
            "value": 10159.502645935023,
            "unit": "ns/iter",
            "extra": "iterations: 415921\ncpu: 10158.670997617433 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_num_fts",
            "value": 973.2313400987761,
            "unit": "ns/iter",
            "extra": "iterations: 4323241\ncpu: 973.1491721141545 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_num_fts",
            "value": 6695.891063503616,
            "unit": "ns/iter",
            "extra": "iterations: 626420\ncpu: 6695.363174866616 ns\nthreads: 1"
          },
          {
            "name": "benchmark_example_reuse",
            "value": 926.9896895578853,
            "unit": "ns/iter",
            "extra": "iterations: 4491272\ncpu: 926.9134222999693 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_char",
            "value": 62.927073356325955,
            "unit": "ns/iter",
            "extra": "iterations: 66597923\ncpu: 62.89351245984083 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_extent",
            "value": 92.45055710186634,
            "unit": "ns/iter",
            "extra": "iterations: 45940521\ncpu: 92.44247360625307 ns\nthreads: 1"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "griffinbassman@gmail.com",
            "name": "Griffin Bassman",
            "username": "bassmang"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "1ef6de2982884ba387ea7738f878742a5fa38991",
          "message": "style: update label type to all caps (#4236)\n\n* style: update label type to all caps\r\n\r\n* fix break\r\n\r\n* change stderr\r\n\r\n* cb_eval fix",
          "timestamp": "2022-10-20T15:06:02-04:00",
          "tree_id": "3b91bfbb4d8a5ac27eeb567ad924e95e1ae2075d",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/1ef6de2982884ba387ea7738f878742a5fa38991"
        },
        "date": 1666295161168,
        "tool": "benchmarkdotnet",
        "benches": [
          {
            "name": "BenchmarkText.Benchmark(args: 120_num_features)",
            "value": 6689.19803074428,
            "unit": "ns",
            "range": "± 116.57493585122542"
          },
          {
            "name": "BenchmarkText.Benchmark(args: 120_string_fts)",
            "value": 9847.784146395597,
            "unit": "ns",
            "range": "± 238.20094763854516"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 1_feature)",
            "value": 775.2570152282715,
            "unit": "ns",
            "range": "± 6.920836467088953"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 8_features)",
            "value": 619.5578030177525,
            "unit": "ns",
            "range": "± 4.716421569488011"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_interactions)",
            "value": 1113015.2213541667,
            "unit": "ns",
            "range": "± 14523.255784819034"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_no_interactions)",
            "value": 676486.8229166666,
            "unit": "ns",
            "range": "± 9033.363387172309"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_no_namespaces)",
            "value": 690492.9752604166,
            "unit": "ns",
            "range": "± 7425.475036583633"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_interactions)",
            "value": 1126757.0833333333,
            "unit": "ns",
            "range": "± 12966.963643406521"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_no_interactions)",
            "value": 689570.8463541666,
            "unit": "ns",
            "range": "± 7780.810065107467"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_interactions)",
            "value": 4893449.789663462,
            "unit": "ns",
            "range": "± 68400.2359933882"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_no_interactions)",
            "value": 1376257.2265625,
            "unit": "ns",
            "range": "± 18409.925411893517"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_no_namespaces)",
            "value": 1131885.119047619,
            "unit": "ns",
            "range": "± 22995.49844854123"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_interactions)",
            "value": 4109922.235576923,
            "unit": "ns",
            "range": "± 64815.38079505018"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_no_interactions)",
            "value": 1204292.138671875,
            "unit": "ns",
            "range": "± 20059.113990522557"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: few_features)",
            "value": 3868.5613778921274,
            "unit": "ns",
            "range": "± 34.35735676475064"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: few_features)",
            "value": 13764.475631713867,
            "unit": "ns",
            "range": "± 113.43082888357729"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: many_features)",
            "value": 165554.56217447916,
            "unit": "ns",
            "range": "± 2907.890130335223"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: many_features)",
            "value": 23905.62479654948,
            "unit": "ns",
            "range": "± 321.069450116645"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: quadratic)",
            "value": 6105120.479910715,
            "unit": "ns",
            "range": "± 64931.66827920582"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: simple)",
            "value": 290640.21538628475,
            "unit": "ns",
            "range": "± 6014.455627302117"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "griffinbassman@gmail.com",
            "name": "Griffin Bassman",
            "username": "bassmang"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "5a600e9ca642935f52f9df10a1d7c3eecd7b2048",
          "message": "style: update prediction type to all caps (#4237)\n\n* style: update prediction type to all caps\r\n\r\n* clang\r\n\r\n* nolint\r\n\r\n* clang\r\n\r\n* stderr\r\n\r\n* type\r\n\r\n* fix merge",
          "timestamp": "2022-10-20T16:40:32-04:00",
          "tree_id": "415bd3fc8ccd15f5dea7572925c19e4a1718e438",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/5a600e9ca642935f52f9df10a1d7c3eecd7b2048"
        },
        "date": 1666299311697,
        "tool": "googlecpp",
        "benches": [
          {
            "name": "bench_text/120_string_fts",
            "value": 7469.171058515332,
            "unit": "ns/iter",
            "extra": "iterations: 563544\ncpu: 7468.488884630126 ns\nthreads: 1"
          },
          {
            "name": "bench_text/120_num_fts",
            "value": 4996.521385477827,
            "unit": "ns/iter",
            "extra": "iterations: 823456\ncpu: 4996.046661873859 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/8_features",
            "value": 88.0589219624489,
            "unit": "ns/iter",
            "extra": "iterations: 47001795\ncpu: 88.05048190180823 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/1_feature",
            "value": 207.25947605871974,
            "unit": "ns/iter",
            "extra": "iterations: 20174131\ncpu: 207.15343823235807 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features",
            "value": 8454.49076522932,
            "unit": "ns/iter",
            "extra": "iterations: 501366\ncpu: 8453.736192721484 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features",
            "value": 15040.453559537254,
            "unit": "ns/iter",
            "extra": "iterations: 270906\ncpu: 15038.758093213142 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features_no_predict",
            "value": 3985.13829872966,
            "unit": "ns/iter",
            "extra": "iterations: 1029901\ncpu: 3984.744747310666 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features_no_predic",
            "value": 5912.4506040116785,
            "unit": "ns/iter",
            "extra": "iterations: 700897\ncpu: 5911.938844081226 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/few_features",
            "value": 1496.735366374903,
            "unit": "ns/iter",
            "extra": "iterations: 2765446\ncpu: 1496.6134938089544 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/many_features/min_time:15.000",
            "value": 83576.03876557742,
            "unit": "ns/iter",
            "extra": "iterations: 251486\ncpu: 83568.28968610578 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_no_namespaces/min_time:15.000",
            "value": 333975.11667933874,
            "unit": "ns/iter",
            "extra": "iterations: 68384\ncpu: 333946.2081773515 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 358748.7544489948,
            "unit": "ns/iter",
            "extra": "iterations: 59845\ncpu: 358689.2288411728 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_interactions/min_time:15.000",
            "value": 634428.1669205172,
            "unit": "ns/iter",
            "extra": "iterations: 32171\ncpu: 634292.2103758046 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_no_interactions/min_time:15.000",
            "value": 322461.6786602804,
            "unit": "ns/iter",
            "extra": "iterations: 64312\ncpu: 322435.3184475678 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_interactions/min_time:15.000",
            "value": 533206.4171151774,
            "unit": "ns/iter",
            "extra": "iterations: 40876\ncpu: 533087.8853116746 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_no_namespaces/min_time:15.000",
            "value": 790696.4488704846,
            "unit": "ns/iter",
            "extra": "iterations: 26560\ncpu: 790616.8298192773 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 938317.7457062493,
            "unit": "ns/iter",
            "extra": "iterations: 23115\ncpu: 938048.0337443217 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_interactions/min_time:15.000",
            "value": 3008330.4903765717,
            "unit": "ns/iter",
            "extra": "iterations: 7170\ncpu: 3007962.133891215 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_no_interactions/min_time:15.000",
            "value": 764847.2589738881,
            "unit": "ns/iter",
            "extra": "iterations: 26995\ncpu: 764619.5184293392 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_interactions/min_time:15.000",
            "value": 2281445.929272668,
            "unit": "ns/iter",
            "extra": "iterations: 9473\ncpu: 2281218.0724163437 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/simple/min_time:15.000",
            "value": 73792.53702098377,
            "unit": "ns/iter",
            "extra": "iterations: 284933\ncpu: 73760.97363239776 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/quadratic/min_time:15.000",
            "value": 2804175.1597212856,
            "unit": "ns/iter",
            "extra": "iterations: 7463\ncpu: 2803936.0310866884 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_string_fts",
            "value": 1158.2883281872048,
            "unit": "ns/iter",
            "extra": "iterations: 3610476\ncpu: 1158.1019787972557 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_string_fts",
            "value": 8660.302200210372,
            "unit": "ns/iter",
            "extra": "iterations: 485999\ncpu: 8659.185512727356 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_num_fts",
            "value": 886.9460008859429,
            "unit": "ns/iter",
            "extra": "iterations: 4661595\ncpu: 886.8719397545248 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_num_fts",
            "value": 5691.429948305543,
            "unit": "ns/iter",
            "extra": "iterations: 707813\ncpu: 5690.554143537903 ns\nthreads: 1"
          },
          {
            "name": "benchmark_example_reuse",
            "value": 1028.2296039867917,
            "unit": "ns/iter",
            "extra": "iterations: 4250414\ncpu: 1027.9281971121031 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_char",
            "value": 55.42433311690593,
            "unit": "ns/iter",
            "extra": "iterations: 72201223\ncpu: 55.41875793433548 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_extent",
            "value": 79.5170172392867,
            "unit": "ns/iter",
            "extra": "iterations: 52446433\ncpu: 79.46566165901105 ns\nthreads: 1"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "griffinbassman@gmail.com",
            "name": "Griffin Bassman",
            "username": "bassmang"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "5a600e9ca642935f52f9df10a1d7c3eecd7b2048",
          "message": "style: update prediction type to all caps (#4237)\n\n* style: update prediction type to all caps\r\n\r\n* clang\r\n\r\n* nolint\r\n\r\n* clang\r\n\r\n* stderr\r\n\r\n* type\r\n\r\n* fix merge",
          "timestamp": "2022-10-20T16:40:32-04:00",
          "tree_id": "415bd3fc8ccd15f5dea7572925c19e4a1718e438",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/5a600e9ca642935f52f9df10a1d7c3eecd7b2048"
        },
        "date": 1666300865213,
        "tool": "benchmarkdotnet",
        "benches": [
          {
            "name": "BenchmarkText.Benchmark(args: 120_num_features)",
            "value": 7638.005300668569,
            "unit": "ns",
            "range": "± 114.89440133690178"
          },
          {
            "name": "BenchmarkText.Benchmark(args: 120_string_fts)",
            "value": 11378.633371988932,
            "unit": "ns",
            "range": "± 112.7170150577499"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 1_feature)",
            "value": 818.7958971659342,
            "unit": "ns",
            "range": "± 12.148742640455763"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 8_features)",
            "value": 606.5210069928851,
            "unit": "ns",
            "range": "± 7.639478881855162"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_interactions)",
            "value": 1107321.8331473214,
            "unit": "ns",
            "range": "± 17442.399983632342"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_no_interactions)",
            "value": 684907.3529411765,
            "unit": "ns",
            "range": "± 13209.01626038256"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_no_namespaces)",
            "value": 652389.3205915178,
            "unit": "ns",
            "range": "± 4508.042599613121"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_interactions)",
            "value": 1091779.017857143,
            "unit": "ns",
            "range": "± 11239.175118293992"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_no_interactions)",
            "value": 677625.41015625,
            "unit": "ns",
            "range": "± 12112.349831832526"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_interactions)",
            "value": 4245688.783482143,
            "unit": "ns",
            "range": "± 53913.03085997412"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_no_interactions)",
            "value": 1301599.151141827,
            "unit": "ns",
            "range": "± 12783.757650729518"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_no_namespaces)",
            "value": 1129788.33203125,
            "unit": "ns",
            "range": "± 29460.461672341968"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_interactions)",
            "value": 3354710.205078125,
            "unit": "ns",
            "range": "± 84974.19110746133"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_no_interactions)",
            "value": 1070742.3828125,
            "unit": "ns",
            "range": "± 13218.18318446393"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: few_features)",
            "value": 3967.082268851144,
            "unit": "ns",
            "range": "± 43.23378157467257"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: few_features)",
            "value": 13805.206909179688,
            "unit": "ns",
            "range": "± 186.93832195649023"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: many_features)",
            "value": 131180.04807692306,
            "unit": "ns",
            "range": "± 1047.2860183804685"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: many_features)",
            "value": 23195.093383789062,
            "unit": "ns",
            "range": "± 356.3656418110905"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: quadratic)",
            "value": 5069200.948660715,
            "unit": "ns",
            "range": "± 75037.84265590072"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: simple)",
            "value": 256935.9178331163,
            "unit": "ns",
            "range": "± 8541.890112902209"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "jackgerrits@users.noreply.github.com",
            "name": "Jack Gerrits",
            "username": "jackgerrits"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "a84f3a3e7681cfc1f5ca42c91d70d248ce7f856c",
          "message": "build: allow VW_CXX_STANDARD to be provided by consumer of VW (#4238)\n\n* build: allow VW_CXX_STANDARD to be provided by consumer of VW\r\n\r\n* Update CMakeLists.txt",
          "timestamp": "2022-10-21T11:11:27-04:00",
          "tree_id": "63f96c412125eb3730ce38579201fae508931c17",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/a84f3a3e7681cfc1f5ca42c91d70d248ce7f856c"
        },
        "date": 1666365971618,
        "tool": "googlecpp",
        "benches": [
          {
            "name": "bench_text/120_string_fts",
            "value": 7574.083844577009,
            "unit": "ns/iter",
            "extra": "iterations: 554204\ncpu: 7572.660067412 ns\nthreads: 1"
          },
          {
            "name": "bench_text/120_num_fts",
            "value": 5108.634511234623,
            "unit": "ns/iter",
            "extra": "iterations: 829979\ncpu: 5107.715857871104 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/8_features",
            "value": 91.77321688123045,
            "unit": "ns/iter",
            "extra": "iterations: 47259373\ncpu: 91.76094232143116 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/1_feature",
            "value": 209.41579939539002,
            "unit": "ns/iter",
            "extra": "iterations: 20384337\ncpu: 209.364758834197 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features",
            "value": 8847.456518158826,
            "unit": "ns/iter",
            "extra": "iterations: 473600\ncpu: 8846.249155405403 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features",
            "value": 15767.026086521772,
            "unit": "ns/iter",
            "extra": "iterations: 259981\ncpu: 15764.818198252955 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features_no_predict",
            "value": 4190.341598615482,
            "unit": "ns/iter",
            "extra": "iterations: 1022147\ncpu: 4189.759985598941 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features_no_predic",
            "value": 6275.080820354661,
            "unit": "ns/iter",
            "extra": "iterations: 661665\ncpu: 6274.096106035529 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/few_features",
            "value": 1603.6228383731184,
            "unit": "ns/iter",
            "extra": "iterations: 2670558\ncpu: 1603.3876066350183 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/many_features/min_time:15.000",
            "value": 102407.45351933752,
            "unit": "ns/iter",
            "extra": "iterations: 204655\ncpu: 102392.48686814395 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_no_namespaces/min_time:15.000",
            "value": 350782.59280278964,
            "unit": "ns/iter",
            "extra": "iterations: 63080\ncpu: 350729.56563094485 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 369564.0200085889,
            "unit": "ns/iter",
            "extra": "iterations: 55876\ncpu: 369512.29687164415 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_interactions/min_time:15.000",
            "value": 662091.8854078216,
            "unit": "ns/iter",
            "extra": "iterations: 31791\ncpu: 661994.2814003966 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_no_interactions/min_time:15.000",
            "value": 350197.0486722678,
            "unit": "ns/iter",
            "extra": "iterations: 59048\ncpu: 350124.88992006535 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_interactions/min_time:15.000",
            "value": 544347.1349841667,
            "unit": "ns/iter",
            "extra": "iterations: 38849\ncpu: 544162.0813920561 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_no_namespaces/min_time:15.000",
            "value": 807865.5132699981,
            "unit": "ns/iter",
            "extra": "iterations: 26526\ncpu: 807739.2068159543 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 971348.657513706,
            "unit": "ns/iter",
            "extra": "iterations: 21341\ncpu: 971193.3601986797 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_interactions/min_time:15.000",
            "value": 3150089.127870319,
            "unit": "ns/iter",
            "extra": "iterations: 6663\ncpu: 3149653.1742458357 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_no_interactions/min_time:15.000",
            "value": 823505.609146178,
            "unit": "ns/iter",
            "extra": "iterations: 26153\ncpu: 823370.8217030556 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_interactions/min_time:15.000",
            "value": 2420428.9853025074,
            "unit": "ns/iter",
            "extra": "iterations: 8777\ncpu: 2420094.815996353 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/simple/min_time:15.000",
            "value": 90168.12425424127,
            "unit": "ns/iter",
            "extra": "iterations: 231308\ncpu: 90155.19350822274 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/quadratic/min_time:15.000",
            "value": 2868013.3172197156,
            "unit": "ns/iter",
            "extra": "iterations: 7323\ncpu: 2867599.576676226 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_string_fts",
            "value": 1147.0076968375138,
            "unit": "ns/iter",
            "extra": "iterations: 3642535\ncpu: 1146.8559670668885 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_string_fts",
            "value": 9024.269387087303,
            "unit": "ns/iter",
            "extra": "iterations: 470687\ncpu: 9022.911616424442 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_num_fts",
            "value": 892.5403498736138,
            "unit": "ns/iter",
            "extra": "iterations: 4719990\ncpu: 892.4070389979669 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_num_fts",
            "value": 5875.813680432296,
            "unit": "ns/iter",
            "extra": "iterations: 723252\ncpu: 5875.0369165934135 ns\nthreads: 1"
          },
          {
            "name": "benchmark_example_reuse",
            "value": 1007.6863575936907,
            "unit": "ns/iter",
            "extra": "iterations: 4126706\ncpu: 1007.5333449971967 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_char",
            "value": 56.04760350625832,
            "unit": "ns/iter",
            "extra": "iterations: 76842575\ncpu: 56.039487224367484 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_extent",
            "value": 81.66253113083167,
            "unit": "ns/iter",
            "extra": "iterations: 49644111\ncpu: 81.65200500820814 ns\nthreads: 1"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "jackgerrits@users.noreply.github.com",
            "name": "Jack Gerrits",
            "username": "jackgerrits"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "a84f3a3e7681cfc1f5ca42c91d70d248ce7f856c",
          "message": "build: allow VW_CXX_STANDARD to be provided by consumer of VW (#4238)\n\n* build: allow VW_CXX_STANDARD to be provided by consumer of VW\r\n\r\n* Update CMakeLists.txt",
          "timestamp": "2022-10-21T11:11:27-04:00",
          "tree_id": "63f96c412125eb3730ce38579201fae508931c17",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/a84f3a3e7681cfc1f5ca42c91d70d248ce7f856c"
        },
        "date": 1666367282551,
        "tool": "benchmarkdotnet",
        "benches": [
          {
            "name": "BenchmarkText.Benchmark(args: 120_num_features)",
            "value": 6252.97840663365,
            "unit": "ns",
            "range": "± 38.915392691770684"
          },
          {
            "name": "BenchmarkText.Benchmark(args: 120_string_fts)",
            "value": 9411.744144984654,
            "unit": "ns",
            "range": "± 69.13321790265884"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 1_feature)",
            "value": 696.2356499263218,
            "unit": "ns",
            "range": "± 2.320007172811842"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 8_features)",
            "value": 511.43273671468097,
            "unit": "ns",
            "range": "± 3.96238640169644"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_interactions)",
            "value": 921794.3033854166,
            "unit": "ns",
            "range": "± 3030.762291365134"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_no_interactions)",
            "value": 563341.1263020834,
            "unit": "ns",
            "range": "± 2239.9732718851797"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_no_namespaces)",
            "value": 569716.6536458334,
            "unit": "ns",
            "range": "± 2077.1513748812613"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_interactions)",
            "value": 917310.234375,
            "unit": "ns",
            "range": "± 3233.335588180063"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_no_interactions)",
            "value": 567257.7864583334,
            "unit": "ns",
            "range": "± 1687.2047236489211"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_interactions)",
            "value": 3549332.1875,
            "unit": "ns",
            "range": "± 10403.733203642792"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_no_interactions)",
            "value": 1113613.1380208333,
            "unit": "ns",
            "range": "± 3583.470192189323"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_no_namespaces)",
            "value": 932440.4752604166,
            "unit": "ns",
            "range": "± 2090.7657449219932"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_interactions)",
            "value": 2812697.5651041665,
            "unit": "ns",
            "range": "± 10391.561059041142"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_no_interactions)",
            "value": 952054.35546875,
            "unit": "ns",
            "range": "± 2213.939340440538"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: few_features)",
            "value": 3237.512181599935,
            "unit": "ns",
            "range": "± 18.15764636854053"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: few_features)",
            "value": 11301.126200358072,
            "unit": "ns",
            "range": "± 22.702452420375586"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: many_features)",
            "value": 109388.29833984375,
            "unit": "ns",
            "range": "± 368.4567752170096"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: many_features)",
            "value": 19204.499104817707,
            "unit": "ns",
            "range": "± 76.3010815097501"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: quadratic)",
            "value": 4259811.495535715,
            "unit": "ns",
            "range": "± 28392.102849704417"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: simple)",
            "value": 205694.990234375,
            "unit": "ns",
            "range": "± 1188.979669782743"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "jackgerrits@users.noreply.github.com",
            "name": "Jack Gerrits",
            "username": "jackgerrits"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "d277f3739d8cb7a163187871345e6fe44c394b50",
          "message": "refactor: remove beam.h (#4241)",
          "timestamp": "2022-10-21T13:19:38-04:00",
          "tree_id": "aa7b550132d3fdd8f675efe5216094b13e8c1304",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/d277f3739d8cb7a163187871345e6fe44c394b50"
        },
        "date": 1666373525227,
        "tool": "googlecpp",
        "benches": [
          {
            "name": "bench_text/120_string_fts",
            "value": 6933.0498922524375,
            "unit": "ns/iter",
            "extra": "iterations: 606046\ncpu: 6932.226926668933 ns\nthreads: 1"
          },
          {
            "name": "bench_text/120_num_fts",
            "value": 4696.877428065717,
            "unit": "ns/iter",
            "extra": "iterations: 890987\ncpu: 4696.588390178533 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/8_features",
            "value": 76.9398552513088,
            "unit": "ns/iter",
            "extra": "iterations: 54354205\ncpu: 76.93465482569381 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/1_feature",
            "value": 166.76265305851672,
            "unit": "ns/iter",
            "extra": "iterations: 25250555\ncpu: 166.75218029861125 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features",
            "value": 6981.022376717952,
            "unit": "ns/iter",
            "extra": "iterations: 600088\ncpu: 6980.303888762982 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features",
            "value": 11985.49552559447,
            "unit": "ns/iter",
            "extra": "iterations: 337922\ncpu: 11984.409419925307 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features_no_predict",
            "value": 3328.849734983323,
            "unit": "ns/iter",
            "extra": "iterations: 1263694\ncpu: 3328.6145221865413 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features_no_predic",
            "value": 5030.025501927534,
            "unit": "ns/iter",
            "extra": "iterations: 840211\ncpu: 5029.6680238654335 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/few_features",
            "value": 1232.6211762983116,
            "unit": "ns/iter",
            "extra": "iterations: 3401416\ncpu: 1232.5259538968487 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/many_features/min_time:15.000",
            "value": 61197.97627704334,
            "unit": "ns/iter",
            "extra": "iterations: 343802\ncpu: 61193.73912891722 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_no_namespaces/min_time:15.000",
            "value": 246294.78557629694,
            "unit": "ns/iter",
            "extra": "iterations: 92140\ncpu: 246258.3427393097 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 275648.92216479895,
            "unit": "ns/iter",
            "extra": "iterations: 76238\ncpu: 275582.5205278208 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_interactions/min_time:15.000",
            "value": 519458.97121186135,
            "unit": "ns/iter",
            "extra": "iterations: 40607\ncpu: 519360.3910655797 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_no_interactions/min_time:15.000",
            "value": 245507.50928141252,
            "unit": "ns/iter",
            "extra": "iterations: 84847\ncpu: 245484.49444293848 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_interactions/min_time:15.000",
            "value": 394624.75355540286,
            "unit": "ns/iter",
            "extra": "iterations: 52596\ncpu: 394591.7122975132 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_no_namespaces/min_time:15.000",
            "value": 606914.0338244287,
            "unit": "ns/iter",
            "extra": "iterations: 34413\ncpu: 606852.0500973468 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 764691.0590923949,
            "unit": "ns/iter",
            "extra": "iterations: 27567\ncpu: 764615.2791380989 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_interactions/min_time:15.000",
            "value": 2393655.4364917,
            "unit": "ns/iter",
            "extra": "iterations: 8802\ncpu: 2393205.805498751 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_no_interactions/min_time:15.000",
            "value": 631230.0344932004,
            "unit": "ns/iter",
            "extra": "iterations: 33021\ncpu: 631166.6696950425 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_interactions/min_time:15.000",
            "value": 1810832.6338004041,
            "unit": "ns/iter",
            "extra": "iterations: 11704\ncpu: 1810492.9169514691 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/simple/min_time:15.000",
            "value": 50961.49213610281,
            "unit": "ns/iter",
            "extra": "iterations: 413599\ncpu: 50957.077749220865 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/quadratic/min_time:15.000",
            "value": 2720085.009687129,
            "unit": "ns/iter",
            "extra": "iterations: 7639\ncpu: 2719571.828773399 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_string_fts",
            "value": 978.0872555371146,
            "unit": "ns/iter",
            "extra": "iterations: 4291040\ncpu: 978.0168444013646 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_string_fts",
            "value": 8624.187946758982,
            "unit": "ns/iter",
            "extra": "iterations: 487819\ncpu: 8623.529013835105 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_num_fts",
            "value": 839.8864623747294,
            "unit": "ns/iter",
            "extra": "iterations: 5009335\ncpu: 839.8273024263716 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_num_fts",
            "value": 5652.995553623644,
            "unit": "ns/iter",
            "extra": "iterations: 743077\ncpu: 5652.557810294215 ns\nthreads: 1"
          },
          {
            "name": "benchmark_example_reuse",
            "value": 785.0415116389446,
            "unit": "ns/iter",
            "extra": "iterations: 5341249\ncpu: 784.9849726159597 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_char",
            "value": 53.15341484839992,
            "unit": "ns/iter",
            "extra": "iterations: 79003800\ncpu: 53.13959961419549 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_extent",
            "value": 77.16742833938059,
            "unit": "ns/iter",
            "extra": "iterations: 54464119\ncpu: 77.16190910937152 ns\nthreads: 1"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "jackgerrits@users.noreply.github.com",
            "name": "Jack Gerrits",
            "username": "jackgerrits"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "023c563ad80105a2ae15508f5bbcbe1461f8b55a",
          "message": "style: more style fixes (#4240)\n\n* style: more style fixes\r\n\r\n* build fixes",
          "timestamp": "2022-10-21T15:10:07-04:00",
          "tree_id": "1f652effe9f0526a6f201c7965c9c0c2a67568da",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/023c563ad80105a2ae15508f5bbcbe1461f8b55a"
        },
        "date": 1666380137809,
        "tool": "googlecpp",
        "benches": [
          {
            "name": "bench_text/120_string_fts",
            "value": 5894.897144602525,
            "unit": "ns/iter",
            "extra": "iterations: 710444\ncpu: 5894.618858066223 ns\nthreads: 1"
          },
          {
            "name": "bench_text/120_num_fts",
            "value": 3989.4013139227877,
            "unit": "ns/iter",
            "extra": "iterations: 1053791\ncpu: 3988.9718169921734 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/8_features",
            "value": 67.74934850599146,
            "unit": "ns/iter",
            "extra": "iterations: 61022050\ncpu: 67.74551002465503 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/1_feature",
            "value": 161.2797199843625,
            "unit": "ns/iter",
            "extra": "iterations: 25996691\ncpu: 161.27323281259146 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features",
            "value": 7143.341257156199,
            "unit": "ns/iter",
            "extra": "iterations: 580612\ncpu: 7142.978960131724 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features",
            "value": 12642.025088366903,
            "unit": "ns/iter",
            "extra": "iterations: 332704\ncpu: 12641.261301336926 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features_no_predict",
            "value": 3305.2446885270842,
            "unit": "ns/iter",
            "extra": "iterations: 1270881\ncpu: 3305.1161359718158 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features_no_predic",
            "value": 5106.978199864729,
            "unit": "ns/iter",
            "extra": "iterations: 823527\ncpu: 5106.497661886008 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/few_features",
            "value": 1234.6720614725264,
            "unit": "ns/iter",
            "extra": "iterations: 3389742\ncpu: 1234.6249360570805 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/many_features/min_time:15.000",
            "value": 52421.81555700733,
            "unit": "ns/iter",
            "extra": "iterations: 398020\ncpu: 52415.55625345461 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_no_namespaces/min_time:15.000",
            "value": 197275.83622063327,
            "unit": "ns/iter",
            "extra": "iterations: 107309\ncpu: 197265.24243073745 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 223035.68124251245,
            "unit": "ns/iter",
            "extra": "iterations: 93488\ncpu: 223006.64149409553 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_interactions/min_time:15.000",
            "value": 486844.93794899597,
            "unit": "ns/iter",
            "extra": "iterations: 46075\ncpu: 486811.2642430822 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_no_interactions/min_time:15.000",
            "value": 195798.15218122664,
            "unit": "ns/iter",
            "extra": "iterations: 107004\ncpu: 195784.62580838118 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_interactions/min_time:15.000",
            "value": 341195.480506634,
            "unit": "ns/iter",
            "extra": "iterations: 61662\ncpu: 341149.6756511304 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_no_namespaces/min_time:15.000",
            "value": 572538.2560166655,
            "unit": "ns/iter",
            "extra": "iterations: 36482\ncpu: 572499.5038649194 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 716618.7838039404,
            "unit": "ns/iter",
            "extra": "iterations: 29501\ncpu: 716519.5417104498 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_interactions/min_time:15.000",
            "value": 2200807.446647773,
            "unit": "ns/iter",
            "extra": "iterations: 9531\ncpu: 2200661.68292939 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_no_interactions/min_time:15.000",
            "value": 601698.3379185909,
            "unit": "ns/iter",
            "extra": "iterations: 35476\ncpu: 601614.0517532979 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_interactions/min_time:15.000",
            "value": 1729169.4485950638,
            "unit": "ns/iter",
            "extra": "iterations: 12207\ncpu: 1729058.6794462223 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/simple/min_time:15.000",
            "value": 49026.41744657172,
            "unit": "ns/iter",
            "extra": "iterations: 429219\ncpu: 49024.079316153264 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/quadratic/min_time:15.000",
            "value": 2314166.642217241,
            "unit": "ns/iter",
            "extra": "iterations: 8930\ncpu: 2314037.122060471 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_string_fts",
            "value": 899.6832249599776,
            "unit": "ns/iter",
            "extra": "iterations: 4668275\ncpu: 899.646336173424 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_string_fts",
            "value": 7838.052211351624,
            "unit": "ns/iter",
            "extra": "iterations: 534673\ncpu: 7837.69500236601 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_num_fts",
            "value": 691.9069041660133,
            "unit": "ns/iter",
            "extra": "iterations: 6062108\ncpu: 691.8794254407825 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_num_fts",
            "value": 5092.949346998116,
            "unit": "ns/iter",
            "extra": "iterations: 826644\ncpu: 5092.744639772372 ns\nthreads: 1"
          },
          {
            "name": "benchmark_example_reuse",
            "value": 797.2127836538285,
            "unit": "ns/iter",
            "extra": "iterations: 5220448\ncpu: 797.1767748668324 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_char",
            "value": 39.92135705418587,
            "unit": "ns/iter",
            "extra": "iterations: 105157658\ncpu: 39.91938181050021 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_extent",
            "value": 58.3371695079029,
            "unit": "ns/iter",
            "extra": "iterations: 71016146\ncpu: 58.33451311198956 ns\nthreads: 1"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "jackgerrits@users.noreply.github.com",
            "name": "Jack Gerrits",
            "username": "jackgerrits"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "023c563ad80105a2ae15508f5bbcbe1461f8b55a",
          "message": "style: more style fixes (#4240)\n\n* style: more style fixes\r\n\r\n* build fixes",
          "timestamp": "2022-10-21T15:10:07-04:00",
          "tree_id": "1f652effe9f0526a6f201c7965c9c0c2a67568da",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/023c563ad80105a2ae15508f5bbcbe1461f8b55a"
        },
        "date": 1666381637708,
        "tool": "benchmarkdotnet",
        "benches": [
          {
            "name": "BenchmarkText.Benchmark(args: 120_num_features)",
            "value": 5174.928563435872,
            "unit": "ns",
            "range": "± 8.676865276467817"
          },
          {
            "name": "BenchmarkText.Benchmark(args: 120_string_fts)",
            "value": 7290.3582436697825,
            "unit": "ns",
            "range": "± 12.402672212396702"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 1_feature)",
            "value": 688.9857496534075,
            "unit": "ns",
            "range": "± 1.8208158333528255"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 8_features)",
            "value": 474.64982179495007,
            "unit": "ns",
            "range": "± 1.3986753218538572"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_interactions)",
            "value": 846935.0864955357,
            "unit": "ns",
            "range": "± 1171.6279558741114"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_no_interactions)",
            "value": 458868.388671875,
            "unit": "ns",
            "range": "± 908.268627480187"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_no_namespaces)",
            "value": 460495.5045572917,
            "unit": "ns",
            "range": "± 1157.9904219112598"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_interactions)",
            "value": 859519.990234375,
            "unit": "ns",
            "range": "± 1669.7217395703192"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_no_interactions)",
            "value": 465763.8053385417,
            "unit": "ns",
            "range": "± 1429.644642942193"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_interactions)",
            "value": 3529524.6354166665,
            "unit": "ns",
            "range": "± 5165.611638746475"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_no_interactions)",
            "value": 1048948.2942708333,
            "unit": "ns",
            "range": "± 1822.2681048579884"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_no_namespaces)",
            "value": 880811.9986979166,
            "unit": "ns",
            "range": "± 1593.775576374692"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_interactions)",
            "value": 2803710.078125,
            "unit": "ns",
            "range": "± 5161.023447850435"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_no_interactions)",
            "value": 897986.71875,
            "unit": "ns",
            "range": "± 1386.6337250901115"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: few_features)",
            "value": 3287.260284423828,
            "unit": "ns",
            "range": "± 4.869167586926398"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: few_features)",
            "value": 10923.232914851262,
            "unit": "ns",
            "range": "± 11.241717535220904"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: many_features)",
            "value": 125589.18131510417,
            "unit": "ns",
            "range": "± 246.98816724757344"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: many_features)",
            "value": 19689.554443359375,
            "unit": "ns",
            "range": "± 16.77041084672846"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: quadratic)",
            "value": 4861648.125,
            "unit": "ns",
            "range": "± 22354.58773592553"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: simple)",
            "value": 197558.12825520834,
            "unit": "ns",
            "range": "± 1009.9726895896207"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "jackgerrits@users.noreply.github.com",
            "name": "Jack Gerrits",
            "username": "jackgerrits"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "7eb962cf846ab1ed0454e3269a3a61b51454e05c",
          "message": "style: another round of style updates (#4242)",
          "timestamp": "2022-10-24T09:41:31-04:00",
          "tree_id": "1316c870f348ddc5f19a076bd3140b6ea41de91b",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/7eb962cf846ab1ed0454e3269a3a61b51454e05c"
        },
        "date": 1666619665617,
        "tool": "googlecpp",
        "benches": [
          {
            "name": "bench_text/120_string_fts",
            "value": 6944.175185502022,
            "unit": "ns/iter",
            "extra": "iterations: 606597\ncpu: 6942.106538608004 ns\nthreads: 1"
          },
          {
            "name": "bench_text/120_num_fts",
            "value": 4681.367954755041,
            "unit": "ns/iter",
            "extra": "iterations: 890441\ncpu: 4680.9850399970355 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/8_features",
            "value": 75.82042984214907,
            "unit": "ns/iter",
            "extra": "iterations: 56393446\ncpu: 75.81377630301225 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/1_feature",
            "value": 168.79978283977226,
            "unit": "ns/iter",
            "extra": "iterations: 24688683\ncpu: 168.78739947367794 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features",
            "value": 7009.287307655334,
            "unit": "ns/iter",
            "extra": "iterations: 590931\ncpu: 7008.070485386622 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features",
            "value": 12462.177363240186,
            "unit": "ns/iter",
            "extra": "iterations: 340031\ncpu: 12460.900035584993 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features_no_predict",
            "value": 3332.1751729045222,
            "unit": "ns/iter",
            "extra": "iterations: 1253727\ncpu: 3331.8386698220606 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features_no_predic",
            "value": 4953.344190696492,
            "unit": "ns/iter",
            "extra": "iterations: 841211\ncpu: 4952.914548193025 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/few_features",
            "value": 1232.4493085335803,
            "unit": "ns/iter",
            "extra": "iterations: 3422292\ncpu: 1231.1030443924703 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/many_features/min_time:15.000",
            "value": 60101.59225613753,
            "unit": "ns/iter",
            "extra": "iterations: 348844\ncpu: 60095.33258419238 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_no_namespaces/min_time:15.000",
            "value": 252574.58774107214,
            "unit": "ns/iter",
            "extra": "iterations: 89029\ncpu: 252503.90659223395 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 285027.1157571206,
            "unit": "ns/iter",
            "extra": "iterations: 73205\ncpu: 284989.8695444301 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_interactions/min_time:15.000",
            "value": 520394.4637591512,
            "unit": "ns/iter",
            "extra": "iterations: 40424\ncpu: 520290.898970908 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_no_interactions/min_time:15.000",
            "value": 257634.1952503085,
            "unit": "ns/iter",
            "extra": "iterations: 82658\ncpu: 257606.54262140373 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_interactions/min_time:15.000",
            "value": 403290.28687647945,
            "unit": "ns/iter",
            "extra": "iterations: 52364\ncpu: 403247.639599725 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_no_namespaces/min_time:15.000",
            "value": 618352.1462715924,
            "unit": "ns/iter",
            "extra": "iterations: 34559\ncpu: 618280.5289504905 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 787242.0072579724,
            "unit": "ns/iter",
            "extra": "iterations: 26867\ncpu: 787151.393903301 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_interactions/min_time:15.000",
            "value": 2446950.2243998614,
            "unit": "ns/iter",
            "extra": "iterations: 8623\ncpu: 2446668.9551200303 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_no_interactions/min_time:15.000",
            "value": 629445.8758354079,
            "unit": "ns/iter",
            "extra": "iterations: 33367\ncpu: 629375.191057034 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_interactions/min_time:15.000",
            "value": 1845266.0612979713,
            "unit": "ns/iter",
            "extra": "iterations: 11387\ncpu: 1844883.0332835647 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/simple/min_time:15.000",
            "value": 50738.30724459099,
            "unit": "ns/iter",
            "extra": "iterations: 407366\ncpu: 50733.28947433015 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/quadratic/min_time:15.000",
            "value": 2714711.067031094,
            "unit": "ns/iter",
            "extra": "iterations: 7683\ncpu: 2714136.769491078 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_string_fts",
            "value": 968.441926782485,
            "unit": "ns/iter",
            "extra": "iterations: 4337843\ncpu: 968.3639541587819 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_string_fts",
            "value": 8609.592228333986,
            "unit": "ns/iter",
            "extra": "iterations: 487952\ncpu: 8608.72483359021 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_num_fts",
            "value": 822.3944667366076,
            "unit": "ns/iter",
            "extra": "iterations: 5108920\ncpu: 822.3181611769239 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_num_fts",
            "value": 5660.280354909382,
            "unit": "ns/iter",
            "extra": "iterations: 740358\ncpu: 5659.761223624237 ns\nthreads: 1"
          },
          {
            "name": "benchmark_example_reuse",
            "value": 788.5545386747878,
            "unit": "ns/iter",
            "extra": "iterations: 5324671\ncpu: 788.3628678654546 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_char",
            "value": 53.07356155528963,
            "unit": "ns/iter",
            "extra": "iterations: 79090579\ncpu: 53.068957808488804 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_extent",
            "value": 76.80418504475989,
            "unit": "ns/iter",
            "extra": "iterations: 54784886\ncpu: 76.79802235967053 ns\nthreads: 1"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "jackgerrits@users.noreply.github.com",
            "name": "Jack Gerrits",
            "username": "jackgerrits"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "7eb962cf846ab1ed0454e3269a3a61b51454e05c",
          "message": "style: another round of style updates (#4242)",
          "timestamp": "2022-10-24T09:41:31-04:00",
          "tree_id": "1316c870f348ddc5f19a076bd3140b6ea41de91b",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/7eb962cf846ab1ed0454e3269a3a61b51454e05c"
        },
        "date": 1666622662908,
        "tool": "benchmarkdotnet",
        "benches": [
          {
            "name": "BenchmarkText.Benchmark(args: 120_num_features)",
            "value": 6238.381324930394,
            "unit": "ns",
            "range": "± 241.57330020958256"
          },
          {
            "name": "BenchmarkText.Benchmark(args: 120_string_fts)",
            "value": 9166.79623413086,
            "unit": "ns",
            "range": "± 366.21370865624397"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 1_feature)",
            "value": 660.2767524719238,
            "unit": "ns",
            "range": "± 26.55716393210455"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 8_features)",
            "value": 506.138170560201,
            "unit": "ns",
            "range": "± 22.610350089917983"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_interactions)",
            "value": 864998.2036389803,
            "unit": "ns",
            "range": "± 44024.545071841145"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_no_interactions)",
            "value": 541967.4787554825,
            "unit": "ns",
            "range": "± 22936.782764934702"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_no_namespaces)",
            "value": 553914.3229166666,
            "unit": "ns",
            "range": "± 13002.143217593837"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_interactions)",
            "value": 877376.5625,
            "unit": "ns",
            "range": "± 35654.85190369744"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_no_interactions)",
            "value": 563745.1031410531,
            "unit": "ns",
            "range": "± 28067.796257936847"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_interactions)",
            "value": 3508804.482256356,
            "unit": "ns",
            "range": "± 154869.78613526275"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_no_interactions)",
            "value": 1076848.1471011513,
            "unit": "ns",
            "range": "± 51234.99804273421"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_no_namespaces)",
            "value": 903796.38671875,
            "unit": "ns",
            "range": "± 32476.244808475723"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_interactions)",
            "value": 2855709.006382042,
            "unit": "ns",
            "range": "± 131669.33623352784"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_no_interactions)",
            "value": 925432.0703125,
            "unit": "ns",
            "range": "± 40933.77083279234"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: few_features)",
            "value": 3175.0707467397056,
            "unit": "ns",
            "range": "± 125.33693544745461"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: few_features)",
            "value": 10919.732406616211,
            "unit": "ns",
            "range": "± 642.6157930028842"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: many_features)",
            "value": 106848.92639160156,
            "unit": "ns",
            "range": "± 3740.7480480953554"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: many_features)",
            "value": 18597.584080051733,
            "unit": "ns",
            "range": "± 928.3960428829254"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: quadratic)",
            "value": 4283900.8870442705,
            "unit": "ns",
            "range": "± 257741.2398558292"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: simple)",
            "value": 207976.98115596065,
            "unit": "ns",
            "range": "± 8739.976091084294"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "olgavrou@gmail.com",
            "name": "olgavrou",
            "username": "olgavrou"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "e717b5660eab7d5c759ec2c7b274e34b1f0d3d37",
          "message": "feat: [LAS] sparse Rademacher (#4243)",
          "timestamp": "2022-10-24T11:54:21-04:00",
          "tree_id": "afe5893a3fd182e83d66d7d40c391eeef215b7d1",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/e717b5660eab7d5c759ec2c7b274e34b1f0d3d37"
        },
        "date": 1666627607064,
        "tool": "googlecpp",
        "benches": [
          {
            "name": "bench_text/120_string_fts",
            "value": 5878.070623012669,
            "unit": "ns/iter",
            "extra": "iterations: 715475\ncpu: 5864.4827562109085 ns\nthreads: 1"
          },
          {
            "name": "bench_text/120_num_fts",
            "value": 3983.6913617973905,
            "unit": "ns/iter",
            "extra": "iterations: 1053703\ncpu: 3983.199630256344 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/8_features",
            "value": 70.02037872035164,
            "unit": "ns/iter",
            "extra": "iterations: 59718470\ncpu: 70.01574554739932 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/1_feature",
            "value": 165.00422635563575,
            "unit": "ns/iter",
            "extra": "iterations: 25261717\ncpu: 164.99412530035067 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features",
            "value": 7057.747434588936,
            "unit": "ns/iter",
            "extra": "iterations: 590841\ncpu: 7057.1950829410935 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features",
            "value": 12571.074368270345,
            "unit": "ns/iter",
            "extra": "iterations: 330719\ncpu: 12570.150490295387 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features_no_predict",
            "value": 3296.1209854307626,
            "unit": "ns/iter",
            "extra": "iterations: 1269252\ncpu: 3294.8392439011295 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features_no_predic",
            "value": 5099.684677928062,
            "unit": "ns/iter",
            "extra": "iterations: 830684\ncpu: 5099.304789787694 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/few_features",
            "value": 1226.0217978264345,
            "unit": "ns/iter",
            "extra": "iterations: 3409239\ncpu: 1225.9543552094765 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/many_features/min_time:15.000",
            "value": 52012.86708755565,
            "unit": "ns/iter",
            "extra": "iterations: 403115\ncpu: 52009.53077905808 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_no_namespaces/min_time:15.000",
            "value": 198895.83600485104,
            "unit": "ns/iter",
            "extra": "iterations: 106369\ncpu: 198867.16242514265 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 226269.04345479657,
            "unit": "ns/iter",
            "extra": "iterations: 92648\ncpu: 226252.4414990069 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_interactions/min_time:15.000",
            "value": 465096.71605101484,
            "unit": "ns/iter",
            "extra": "iterations: 45399\ncpu: 465062.4463093902 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_no_interactions/min_time:15.000",
            "value": 198084.4164036373,
            "unit": "ns/iter",
            "extra": "iterations: 106452\ncpu: 198055.13940555367 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_interactions/min_time:15.000",
            "value": 339319.4201719114,
            "unit": "ns/iter",
            "extra": "iterations: 61194\ncpu: 339296.3199006439 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_no_namespaces/min_time:15.000",
            "value": 595654.8937876867,
            "unit": "ns/iter",
            "extra": "iterations: 35156\ncpu: 595569.376493344 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 742191.1248863388,
            "unit": "ns/iter",
            "extra": "iterations: 28594\ncpu: 742133.8147863192 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_interactions/min_time:15.000",
            "value": 2280196.468985884,
            "unit": "ns/iter",
            "extra": "iterations: 9141\ncpu: 2279861.306202822 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_no_interactions/min_time:15.000",
            "value": 592977.0865704131,
            "unit": "ns/iter",
            "extra": "iterations: 35809\ncpu: 592933.4161802909 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_interactions/min_time:15.000",
            "value": 1806127.0998032982,
            "unit": "ns/iter",
            "extra": "iterations: 11693\ncpu: 1805947.0024801176 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/simple/min_time:15.000",
            "value": 51044.00447797538,
            "unit": "ns/iter",
            "extra": "iterations: 418716\ncpu: 51041.05336313862 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/quadratic/min_time:15.000",
            "value": 2351512.7000679257,
            "unit": "ns/iter",
            "extra": "iterations: 8832\ncpu: 2351355.649909418 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_string_fts",
            "value": 899.5858232068359,
            "unit": "ns/iter",
            "extra": "iterations: 4667205\ncpu: 899.5428527352102 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_string_fts",
            "value": 7813.497292661633,
            "unit": "ns/iter",
            "extra": "iterations: 535212\ncpu: 7813.064355806694 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_num_fts",
            "value": 688.1330182756595,
            "unit": "ns/iter",
            "extra": "iterations: 6111897\ncpu: 687.8770208333004 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_num_fts",
            "value": 5085.396820851981,
            "unit": "ns/iter",
            "extra": "iterations: 824246\ncpu: 5085.136476246184 ns\nthreads: 1"
          },
          {
            "name": "benchmark_example_reuse",
            "value": 799.4671978857817,
            "unit": "ns/iter",
            "extra": "iterations: 5224084\ncpu: 799.430981584513 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_char",
            "value": 39.83679227501766,
            "unit": "ns/iter",
            "extra": "iterations: 105331399\ncpu: 39.83471917998549 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_extent",
            "value": 59.558933917286765,
            "unit": "ns/iter",
            "extra": "iterations: 78669393\ncpu: 59.55630419062705 ns\nthreads: 1"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "olgavrou@gmail.com",
            "name": "olgavrou",
            "username": "olgavrou"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "e717b5660eab7d5c759ec2c7b274e34b1f0d3d37",
          "message": "feat: [LAS] sparse Rademacher (#4243)",
          "timestamp": "2022-10-24T11:54:21-04:00",
          "tree_id": "afe5893a3fd182e83d66d7d40c391eeef215b7d1",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/e717b5660eab7d5c759ec2c7b274e34b1f0d3d37"
        },
        "date": 1666629232809,
        "tool": "benchmarkdotnet",
        "benches": [
          {
            "name": "BenchmarkText.Benchmark(args: 120_num_features)",
            "value": 6255.649871826172,
            "unit": "ns",
            "range": "± 87.63383106096951"
          },
          {
            "name": "BenchmarkText.Benchmark(args: 120_string_fts)",
            "value": 9334.891001383463,
            "unit": "ns",
            "range": "± 38.013154867669996"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 1_feature)",
            "value": 664.035841623942,
            "unit": "ns",
            "range": "± 4.8006200529107055"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 8_features)",
            "value": 512.2425651550293,
            "unit": "ns",
            "range": "± 3.5959976421674282"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_interactions)",
            "value": 901027.65625,
            "unit": "ns",
            "range": "± 3022.127749997168"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_no_interactions)",
            "value": 530138.76953125,
            "unit": "ns",
            "range": "± 1531.8404355579519"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_no_namespaces)",
            "value": 541816.0221354166,
            "unit": "ns",
            "range": "± 4994.659539810912"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_interactions)",
            "value": 892807.2721354166,
            "unit": "ns",
            "range": "± 3448.7485285940697"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_no_interactions)",
            "value": 560701.93359375,
            "unit": "ns",
            "range": "± 3639.1784969950627"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_interactions)",
            "value": 3536218.5546875,
            "unit": "ns",
            "range": "± 8697.794348804482"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_no_interactions)",
            "value": 1080431.2918526786,
            "unit": "ns",
            "range": "± 1787.8465833427176"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_no_namespaces)",
            "value": 926424.4921875,
            "unit": "ns",
            "range": "± 1779.940465280528"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_interactions)",
            "value": 2864566.824776786,
            "unit": "ns",
            "range": "± 4691.270042077253"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_no_interactions)",
            "value": 936176.0184151785,
            "unit": "ns",
            "range": "± 932.7484653205263"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: few_features)",
            "value": 3245.4317474365234,
            "unit": "ns",
            "range": "± 8.1661878323447"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: few_features)",
            "value": 11217.279663085938,
            "unit": "ns",
            "range": "± 18.76001566656934"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: many_features)",
            "value": 107720.28895786831,
            "unit": "ns",
            "range": "± 200.73660197188832"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: many_features)",
            "value": 18948.00524030413,
            "unit": "ns",
            "range": "± 17.5025242452458"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: quadratic)",
            "value": 4202949.21875,
            "unit": "ns",
            "range": "± 19580.495521802426"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: simple)",
            "value": 203978.05350167412,
            "unit": "ns",
            "range": "± 997.3105070757556"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "olgavrou@gmail.com",
            "name": "olgavrou",
            "username": "olgavrou"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "598c5d61e966f15c83c0c5ae47db20410b028fa1",
          "message": "chore: [LAS] remove unused implementations and set max actions default (#4247)",
          "timestamp": "2022-10-24T14:37:24-04:00",
          "tree_id": "de7c0f221e77db49c5baaf84f23915806b1bd947",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/598c5d61e966f15c83c0c5ae47db20410b028fa1"
        },
        "date": 1666637410469,
        "tool": "googlecpp",
        "benches": [
          {
            "name": "bench_text/120_string_fts",
            "value": 6936.998030474768,
            "unit": "ns/iter",
            "extra": "iterations: 607253\ncpu: 6936.2002328518765 ns\nthreads: 1"
          },
          {
            "name": "bench_text/120_num_fts",
            "value": 4686.280553165347,
            "unit": "ns/iter",
            "extra": "iterations: 889065\ncpu: 4685.909466686912 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/8_features",
            "value": 74.84373971994341,
            "unit": "ns/iter",
            "extra": "iterations: 54349781\ncpu: 74.8385757064964 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/1_feature",
            "value": 169.0612563342216,
            "unit": "ns/iter",
            "extra": "iterations: 25204169\ncpu: 169.04952510039107 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features",
            "value": 7135.715086596054,
            "unit": "ns/iter",
            "extra": "iterations: 589172\ncpu: 7134.866897951698 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features",
            "value": 12272.95190557793,
            "unit": "ns/iter",
            "extra": "iterations: 321451\ncpu: 12271.846720028865 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features_no_predict",
            "value": 3313.892227748058,
            "unit": "ns/iter",
            "extra": "iterations: 1254720\ncpu: 3313.660338561591 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features_no_predic",
            "value": 4978.645193427309,
            "unit": "ns/iter",
            "extra": "iterations: 838144\ncpu: 4978.281774969455 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/few_features",
            "value": 1229.6659368875817,
            "unit": "ns/iter",
            "extra": "iterations: 3361748\ncpu: 1229.568605380297 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/many_features/min_time:15.000",
            "value": 62598.80953830116,
            "unit": "ns/iter",
            "extra": "iterations: 335175\ncpu: 62593.249794883246 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_no_namespaces/min_time:15.000",
            "value": 247666.9549583005,
            "unit": "ns/iter",
            "extra": "iterations: 88851\ncpu: 247643.77440884185 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 277061.45818167273,
            "unit": "ns/iter",
            "extra": "iterations: 75003\ncpu: 277019.1059024306 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_interactions/min_time:15.000",
            "value": 523188.2448049952,
            "unit": "ns/iter",
            "extra": "iterations: 39076\ncpu: 523137.68809499376 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_no_interactions/min_time:15.000",
            "value": 247814.35715220319,
            "unit": "ns/iter",
            "extra": "iterations: 84065\ncpu: 247766.92321417938 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_interactions/min_time:15.000",
            "value": 395985.8561454629,
            "unit": "ns/iter",
            "extra": "iterations: 52852\ncpu: 395951.09361991996 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_no_namespaces/min_time:15.000",
            "value": 610124.7674186946,
            "unit": "ns/iter",
            "extra": "iterations: 34130\ncpu: 610009.9179607383 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 794466.9905837373,
            "unit": "ns/iter",
            "extra": "iterations: 26656\ncpu: 794384.7163865545 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_interactions/min_time:15.000",
            "value": 2496585.1950988495,
            "unit": "ns/iter",
            "extra": "iterations: 8447\ncpu: 2496075.754705813 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_no_interactions/min_time:15.000",
            "value": 607511.7314710587,
            "unit": "ns/iter",
            "extra": "iterations: 33758\ncpu: 607373.4107470814 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_interactions/min_time:15.000",
            "value": 1899656.2786885304,
            "unit": "ns/iter",
            "extra": "iterations: 11163\ncpu: 1899479.6112156261 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/simple/min_time:15.000",
            "value": 48343.57051387943,
            "unit": "ns/iter",
            "extra": "iterations: 429809\ncpu: 48339.706706932586 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/quadratic/min_time:15.000",
            "value": 2790282.1884408565,
            "unit": "ns/iter",
            "extra": "iterations: 7440\ncpu: 2790034.0725806467 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_string_fts",
            "value": 961.7646083818307,
            "unit": "ns/iter",
            "extra": "iterations: 4371018\ncpu: 961.6962913444808 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_string_fts",
            "value": 8608.861452310422,
            "unit": "ns/iter",
            "extra": "iterations: 488229\ncpu: 8608.371481415405 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_num_fts",
            "value": 787.3438981963761,
            "unit": "ns/iter",
            "extra": "iterations: 5334349\ncpu: 787.2934822974651 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_num_fts",
            "value": 5645.78113261483,
            "unit": "ns/iter",
            "extra": "iterations: 741682\ncpu: 5645.432004551838 ns\nthreads: 1"
          },
          {
            "name": "benchmark_example_reuse",
            "value": 785.1890913606347,
            "unit": "ns/iter",
            "extra": "iterations: 5282822\ncpu: 785.1294440736431 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_char",
            "value": 53.026554025720124,
            "unit": "ns/iter",
            "extra": "iterations: 79161029\ncpu: 53.02323950336741 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_extent",
            "value": 76.90103482973183,
            "unit": "ns/iter",
            "extra": "iterations: 54480460\ncpu: 76.89597150978473 ns\nthreads: 1"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "olgavrou@gmail.com",
            "name": "olgavrou",
            "username": "olgavrou"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "598c5d61e966f15c83c0c5ae47db20410b028fa1",
          "message": "chore: [LAS] remove unused implementations and set max actions default (#4247)",
          "timestamp": "2022-10-24T14:37:24-04:00",
          "tree_id": "de7c0f221e77db49c5baaf84f23915806b1bd947",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/598c5d61e966f15c83c0c5ae47db20410b028fa1"
        },
        "date": 1666638721795,
        "tool": "benchmarkdotnet",
        "benches": [
          {
            "name": "BenchmarkText.Benchmark(args: 120_num_features)",
            "value": 5251.214790344238,
            "unit": "ns",
            "range": "± 131.3463607777938"
          },
          {
            "name": "BenchmarkText.Benchmark(args: 120_string_fts)",
            "value": 7365.9418546236475,
            "unit": "ns",
            "range": "± 49.135489441106806"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 1_feature)",
            "value": 679.6851084782527,
            "unit": "ns",
            "range": "± 2.024329666412283"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 8_features)",
            "value": 470.22669996534074,
            "unit": "ns",
            "range": "± 4.2745723405595974"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_interactions)",
            "value": 857080.4329427084,
            "unit": "ns",
            "range": "± 10466.224183177128"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_no_interactions)",
            "value": 461841.2272135417,
            "unit": "ns",
            "range": "± 4087.9182694733486"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_no_namespaces)",
            "value": 467817.75390625,
            "unit": "ns",
            "range": "± 4249.282469757258"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_interactions)",
            "value": 851433.6067708334,
            "unit": "ns",
            "range": "± 11537.489832676974"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_no_interactions)",
            "value": 461113.4239783654,
            "unit": "ns",
            "range": "± 4456.608539809256"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_interactions)",
            "value": 3478351.6145833335,
            "unit": "ns",
            "range": "± 47933.43864002885"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_no_interactions)",
            "value": 1067072.87109375,
            "unit": "ns",
            "range": "± 13809.75752070996"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_no_namespaces)",
            "value": 1178886.8131510417,
            "unit": "ns",
            "range": "± 16785.5240925682"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_interactions)",
            "value": 2839113.5416666665,
            "unit": "ns",
            "range": "± 35983.20381004769"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_no_interactions)",
            "value": 906503.8461538461,
            "unit": "ns",
            "range": "± 5309.629794189723"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: few_features)",
            "value": 3297.305488586426,
            "unit": "ns",
            "range": "± 8.860057395558934"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: few_features)",
            "value": 11152.773946126303,
            "unit": "ns",
            "range": "± 170.572404419897"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: many_features)",
            "value": 126735.10579427083,
            "unit": "ns",
            "range": "± 963.9497132100278"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: many_features)",
            "value": 19560.728571965145,
            "unit": "ns",
            "range": "± 120.02901071789093"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: quadratic)",
            "value": 4865707.533482143,
            "unit": "ns",
            "range": "± 55301.70061809062"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: simple)",
            "value": 193882.685546875,
            "unit": "ns",
            "range": "± 758.2134489093719"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "jackgerrits@users.noreply.github.com",
            "name": "Jack Gerrits",
            "username": "jackgerrits"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "d41c1833c88864463d9e5d072255a9a19e42a0b7",
          "message": "refactor: move LabelDict namespace items into other namespaces, add const (#4245)\n\n* refactor: move LabelDict namespace items\r\n\r\n* address comments",
          "timestamp": "2022-10-26T10:19:10-04:00",
          "tree_id": "a4d9f72ec73afa552ba4690b6d88eb08f39cb11b",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/d41c1833c88864463d9e5d072255a9a19e42a0b7"
        },
        "date": 1666794723670,
        "tool": "googlecpp",
        "benches": [
          {
            "name": "bench_text/120_string_fts",
            "value": 6268.596216579515,
            "unit": "ns/iter",
            "extra": "iterations: 669500\ncpu: 6266.520985810307 ns\nthreads: 1"
          },
          {
            "name": "bench_text/120_num_fts",
            "value": 4833.12089910678,
            "unit": "ns/iter",
            "extra": "iterations: 868195\ncpu: 4832.735042242814 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/8_features",
            "value": 78.90128165201455,
            "unit": "ns/iter",
            "extra": "iterations: 53058708\ncpu: 78.89461047562634 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/1_feature",
            "value": 170.1333145118888,
            "unit": "ns/iter",
            "extra": "iterations: 24737742\ncpu: 170.12118163412003 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features",
            "value": 7012.755934443637,
            "unit": "ns/iter",
            "extra": "iterations: 594664\ncpu: 7011.976847429813 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features",
            "value": 12343.00478515911,
            "unit": "ns/iter",
            "extra": "iterations: 335830\ncpu: 12341.730041985533 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features_no_predict",
            "value": 3348.3370106539205,
            "unit": "ns/iter",
            "extra": "iterations: 1264417\ncpu: 3348.0434856538627 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features_no_predic",
            "value": 4943.420838623517,
            "unit": "ns/iter",
            "extra": "iterations: 847484\ncpu: 4943.031490860008 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/few_features",
            "value": 1252.6091371704283,
            "unit": "ns/iter",
            "extra": "iterations: 3234196\ncpu: 1252.5104229922983 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/many_features/min_time:15.000",
            "value": 57928.42537973174,
            "unit": "ns/iter",
            "extra": "iterations: 361966\ncpu: 57923.73427338481 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_no_namespaces/min_time:15.000",
            "value": 265423.8964747371,
            "unit": "ns/iter",
            "extra": "iterations: 87341\ncpu: 265398.7966705214 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 290835.668526388,
            "unit": "ns/iter",
            "extra": "iterations: 72950\ncpu: 290808.5181631254 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_interactions/min_time:15.000",
            "value": 537988.7302020636,
            "unit": "ns/iter",
            "extra": "iterations: 39196\ncpu: 537936.445045413 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_no_interactions/min_time:15.000",
            "value": 260418.8045689725,
            "unit": "ns/iter",
            "extra": "iterations: 80018\ncpu: 260394.30128221132 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_interactions/min_time:15.000",
            "value": 408439.18045214994,
            "unit": "ns/iter",
            "extra": "iterations: 51443\ncpu: 408399.4809789478 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_no_namespaces/min_time:15.000",
            "value": 635929.1122023427,
            "unit": "ns/iter",
            "extra": "iterations: 33092\ncpu: 635849.3593617794 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 816528.6201954903,
            "unit": "ns/iter",
            "extra": "iterations: 25679\ncpu: 816427.4504458902 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_interactions/min_time:15.000",
            "value": 2555883.8096573595,
            "unit": "ns/iter",
            "extra": "iterations: 8201\ncpu: 2555588.452627729 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_no_interactions/min_time:15.000",
            "value": 659482.7105116965,
            "unit": "ns/iter",
            "extra": "iterations: 32402\ncpu: 659404.7157582861 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_interactions/min_time:15.000",
            "value": 1965293.294599317,
            "unit": "ns/iter",
            "extra": "iterations: 10869\ncpu: 1965076.8055938887 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/simple/min_time:15.000",
            "value": 49515.560150698606,
            "unit": "ns/iter",
            "extra": "iterations: 419646\ncpu: 49511.122469891336 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/quadratic/min_time:15.000",
            "value": 3017980.893822105,
            "unit": "ns/iter",
            "extra": "iterations: 6847\ncpu: 3017664.568424128 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_string_fts",
            "value": 948.3353977874951,
            "unit": "ns/iter",
            "extra": "iterations: 4431034\ncpu: 948.2603834680574 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_string_fts",
            "value": 7325.58422418026,
            "unit": "ns/iter",
            "extra": "iterations: 568186\ncpu: 7325.033703751923 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_num_fts",
            "value": 715.8063429073984,
            "unit": "ns/iter",
            "extra": "iterations: 5875003\ncpu: 715.7465451506959 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_num_fts",
            "value": 5399.98157264905,
            "unit": "ns/iter",
            "extra": "iterations: 777974\ncpu: 5399.6255658929385 ns\nthreads: 1"
          },
          {
            "name": "benchmark_example_reuse",
            "value": 781.6708449541095,
            "unit": "ns/iter",
            "extra": "iterations: 5362883\ncpu: 781.4136351660167 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_char",
            "value": 51.462260666315004,
            "unit": "ns/iter",
            "extra": "iterations: 81525141\ncpu: 51.458392448533075 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_extent",
            "value": 73.70718614739849,
            "unit": "ns/iter",
            "extra": "iterations: 57374345\ncpu: 73.70191851427646 ns\nthreads: 1"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "jackgerrits@users.noreply.github.com",
            "name": "Jack Gerrits",
            "username": "jackgerrits"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "d41c1833c88864463d9e5d072255a9a19e42a0b7",
          "message": "refactor: move LabelDict namespace items into other namespaces, add const (#4245)\n\n* refactor: move LabelDict namespace items\r\n\r\n* address comments",
          "timestamp": "2022-10-26T10:19:10-04:00",
          "tree_id": "a4d9f72ec73afa552ba4690b6d88eb08f39cb11b",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/d41c1833c88864463d9e5d072255a9a19e42a0b7"
        },
        "date": 1666796186169,
        "tool": "benchmarkdotnet",
        "benches": [
          {
            "name": "BenchmarkText.Benchmark(args: 120_num_features)",
            "value": 5116.516215006511,
            "unit": "ns",
            "range": "± 14.928416134422804"
          },
          {
            "name": "BenchmarkText.Benchmark(args: 120_string_fts)",
            "value": 7171.238109043667,
            "unit": "ns",
            "range": "± 8.290080087760558"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 1_feature)",
            "value": 668.9941883087158,
            "unit": "ns",
            "range": "± 1.1424883817733757"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 8_features)",
            "value": 501.10787285698785,
            "unit": "ns",
            "range": "± 10.309191761409018"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_interactions)",
            "value": 838479.4140625,
            "unit": "ns",
            "range": "± 906.4612443638003"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_no_interactions)",
            "value": 454313.1282552083,
            "unit": "ns",
            "range": "± 571.2390421462011"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_no_namespaces)",
            "value": 458760.0065104167,
            "unit": "ns",
            "range": "± 878.1506885106161"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_interactions)",
            "value": 838758.75,
            "unit": "ns",
            "range": "± 921.5216952482157"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_no_interactions)",
            "value": 455664.2220052083,
            "unit": "ns",
            "range": "± 522.4665652424844"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_interactions)",
            "value": 3408454.1276041665,
            "unit": "ns",
            "range": "± 5763.65134898154"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_no_interactions)",
            "value": 1064123.1166294643,
            "unit": "ns",
            "range": "± 1594.1049947083154"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_no_namespaces)",
            "value": 884097.7994791666,
            "unit": "ns",
            "range": "± 1285.016616804323"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_interactions)",
            "value": 2796718.7109375,
            "unit": "ns",
            "range": "± 4691.072754423844"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_no_interactions)",
            "value": 880337.9947916666,
            "unit": "ns",
            "range": "± 1437.686989731938"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: few_features)",
            "value": 3302.4046090932993,
            "unit": "ns",
            "range": "± 2.790984233465268"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: few_features)",
            "value": 11229.219709123883,
            "unit": "ns",
            "range": "± 10.051967656872856"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: many_features)",
            "value": 125721.42252604167,
            "unit": "ns",
            "range": "± 159.68590681649735"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: many_features)",
            "value": 19859.423014322918,
            "unit": "ns",
            "range": "± 12.35311101850027"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: quadratic)",
            "value": 4849924.010416667,
            "unit": "ns",
            "range": "± 6590.466527241375"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: simple)",
            "value": 200475.9716796875,
            "unit": "ns",
            "range": "± 1912.5088011025339"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "olgavrou@gmail.com",
            "name": "olgavrou",
            "username": "olgavrou"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "f9ded18b16cedec00ce03daf8e3ee4364133c443",
          "message": "fix: don't run tests with iterations (and a simulator) with valgrind … (#4251)",
          "timestamp": "2022-10-26T11:36:22-04:00",
          "tree_id": "daf52d98e8b421d1946b29af5dd6f669ff7c8502",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/f9ded18b16cedec00ce03daf8e3ee4364133c443"
        },
        "date": 1666799358766,
        "tool": "googlecpp",
        "benches": [
          {
            "name": "bench_text/120_string_fts",
            "value": 6264.10073621173,
            "unit": "ns/iter",
            "extra": "iterations: 671546\ncpu: 6261.545448859795 ns\nthreads: 1"
          },
          {
            "name": "bench_text/120_num_fts",
            "value": 4830.658304282956,
            "unit": "ns/iter",
            "extra": "iterations: 867456\ncpu: 4830.2309281392945 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/8_features",
            "value": 81.19767062999269,
            "unit": "ns/iter",
            "extra": "iterations: 51755453\ncpu: 81.18997238803028 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/1_feature",
            "value": 168.65240464350128,
            "unit": "ns/iter",
            "extra": "iterations: 25058413\ncpu: 168.6381894974753 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features",
            "value": 7074.720586177044,
            "unit": "ns/iter",
            "extra": "iterations: 592995\ncpu: 7074.018667948297 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features",
            "value": 12280.339823754766,
            "unit": "ns/iter",
            "extra": "iterations: 334647\ncpu: 12279.087814921399 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features_no_predict",
            "value": 3328.4544808860396,
            "unit": "ns/iter",
            "extra": "iterations: 1264337\ncpu: 3328.183941464974 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features_no_predic",
            "value": 4960.245779553092,
            "unit": "ns/iter",
            "extra": "iterations: 848903\ncpu: 4959.825563109092 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/few_features",
            "value": 1252.761220546285,
            "unit": "ns/iter",
            "extra": "iterations: 3384550\ncpu: 1252.6550649273897 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/many_features/min_time:15.000",
            "value": 59149.740029776505,
            "unit": "ns/iter",
            "extra": "iterations: 352625\ncpu: 59138.724140375765 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_no_namespaces/min_time:15.000",
            "value": 247614.35145849318,
            "unit": "ns/iter",
            "extra": "iterations: 94344\ncpu: 247583.96612397177 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 278150.3396324134,
            "unit": "ns/iter",
            "extra": "iterations: 75302\ncpu: 278091.88998964155 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_interactions/min_time:15.000",
            "value": 523565.0717635309,
            "unit": "ns/iter",
            "extra": "iterations: 40090\ncpu: 523509.8902469439 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_no_interactions/min_time:15.000",
            "value": 248646.42534151455,
            "unit": "ns/iter",
            "extra": "iterations: 84257\ncpu: 248586.00828417795 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_interactions/min_time:15.000",
            "value": 396093.1944234251,
            "unit": "ns/iter",
            "extra": "iterations: 52864\ncpu: 396053.8967917679 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_no_namespaces/min_time:15.000",
            "value": 610035.5986961146,
            "unit": "ns/iter",
            "extra": "iterations: 34819\ncpu: 609904.4458485305 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 789457.006129007,
            "unit": "ns/iter",
            "extra": "iterations: 26758\ncpu: 789373.1968009559 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_interactions/min_time:15.000",
            "value": 2510244.4743314255,
            "unit": "ns/iter",
            "extra": "iterations: 8376\ncpu: 2509985.398758357 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_no_interactions/min_time:15.000",
            "value": 624920.1737380343,
            "unit": "ns/iter",
            "extra": "iterations: 33539\ncpu: 624858.7763499205 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_interactions/min_time:15.000",
            "value": 1896809.130618476,
            "unit": "ns/iter",
            "extra": "iterations: 11124\ncpu: 1896625.4674577503 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/simple/min_time:15.000",
            "value": 49055.895366423036,
            "unit": "ns/iter",
            "extra": "iterations: 423841\ncpu: 49047.338978532025 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/quadratic/min_time:15.000",
            "value": 2921012.4447246874,
            "unit": "ns/iter",
            "extra": "iterations: 7137\ncpu: 2920739.0219980394 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_string_fts",
            "value": 948.073968270462,
            "unit": "ns/iter",
            "extra": "iterations: 4429940\ncpu: 947.9400849672895 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_string_fts",
            "value": 7397.170782083329,
            "unit": "ns/iter",
            "extra": "iterations: 573251\ncpu: 7396.6060242372405 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_num_fts",
            "value": 716.7801884476598,
            "unit": "ns/iter",
            "extra": "iterations: 5878449\ncpu: 716.7183384596822 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_num_fts",
            "value": 5410.259259879854,
            "unit": "ns/iter",
            "extra": "iterations: 775820\ncpu: 5407.707715707249 ns\nthreads: 1"
          },
          {
            "name": "benchmark_example_reuse",
            "value": 778.9584202040372,
            "unit": "ns/iter",
            "extra": "iterations: 5374846\ncpu: 778.892455709425 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_char",
            "value": 51.410254023344756,
            "unit": "ns/iter",
            "extra": "iterations: 81684776\ncpu: 51.40661951499987 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_extent",
            "value": 73.3873995077561,
            "unit": "ns/iter",
            "extra": "iterations: 57247827\ncpu: 73.3817809364185 ns\nthreads: 1"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "olgavrou@gmail.com",
            "name": "olgavrou",
            "username": "olgavrou"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "f9ded18b16cedec00ce03daf8e3ee4364133c443",
          "message": "fix: don't run tests with iterations (and a simulator) with valgrind … (#4251)",
          "timestamp": "2022-10-26T11:36:22-04:00",
          "tree_id": "daf52d98e8b421d1946b29af5dd6f669ff7c8502",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/f9ded18b16cedec00ce03daf8e3ee4364133c443"
        },
        "date": 1666800760759,
        "tool": "benchmarkdotnet",
        "benches": [
          {
            "name": "BenchmarkText.Benchmark(args: 120_num_features)",
            "value": 6577.178985595703,
            "unit": "ns",
            "range": "± 156.03448404694927"
          },
          {
            "name": "BenchmarkText.Benchmark(args: 120_string_fts)",
            "value": 9546.61113194057,
            "unit": "ns",
            "range": "± 131.4897566267569"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 1_feature)",
            "value": 803.9083140236991,
            "unit": "ns",
            "range": "± 8.05656234607655"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 8_features)",
            "value": 618.0272164552108,
            "unit": "ns",
            "range": "± 15.036142144901817"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_interactions)",
            "value": 1087288.002232143,
            "unit": "ns",
            "range": "± 13283.937017565242"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_no_interactions)",
            "value": 681311.2434895834,
            "unit": "ns",
            "range": "± 12470.934642225908"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_no_namespaces)",
            "value": 671809.2708333334,
            "unit": "ns",
            "range": "± 12120.341941340483"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_interactions)",
            "value": 1087339.5703125,
            "unit": "ns",
            "range": "± 15398.50333727947"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_no_interactions)",
            "value": 662873.2877604166,
            "unit": "ns",
            "range": "± 6380.769111067033"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_interactions)",
            "value": 4667299.051339285,
            "unit": "ns",
            "range": "± 58000.03778061065"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_no_interactions)",
            "value": 1361750.4324776786,
            "unit": "ns",
            "range": "± 19982.933096822562"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_no_namespaces)",
            "value": 1154955.4398148148,
            "unit": "ns",
            "range": "± 29458.685727103137"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_interactions)",
            "value": 3997878.515625,
            "unit": "ns",
            "range": "± 88924.63964441279"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_no_interactions)",
            "value": 1154188.9016544118,
            "unit": "ns",
            "range": "± 23034.658183067455"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: few_features)",
            "value": 3903.6303758621216,
            "unit": "ns",
            "range": "± 72.24253633204145"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: few_features)",
            "value": 13834.661458333334,
            "unit": "ns",
            "range": "± 134.58573956119614"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: many_features)",
            "value": 163685.85902622767,
            "unit": "ns",
            "range": "± 2834.5581339138184"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: many_features)",
            "value": 24737.06082430753,
            "unit": "ns",
            "range": "± 898.7371725962603"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: quadratic)",
            "value": 6138901.025390625,
            "unit": "ns",
            "range": "± 114098.6472160235"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: simple)",
            "value": 285265.6282552083,
            "unit": "ns",
            "range": "± 4935.539397936387"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "olgavrou@gmail.com",
            "name": "olgavrou",
            "username": "olgavrou"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "93cf30869139d4459fd0207fb78552864863d049",
          "message": "chore: [LAS] remove compile time flag and its own custom CI (#4249)",
          "timestamp": "2022-10-26T12:40:54-04:00",
          "tree_id": "8835ef7b03fd40ba5aa926683135d901f36bc014",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/93cf30869139d4459fd0207fb78552864863d049"
        },
        "date": 1666803446312,
        "tool": "googlecpp",
        "benches": [
          {
            "name": "bench_text/120_string_fts",
            "value": 6656.435933258666,
            "unit": "ns/iter",
            "extra": "iterations: 601248\ncpu: 6655.537482037363 ns\nthreads: 1"
          },
          {
            "name": "bench_text/120_num_fts",
            "value": 4964.277310132716,
            "unit": "ns/iter",
            "extra": "iterations: 806642\ncpu: 4961.256046672501 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/8_features",
            "value": 79.27869989898556,
            "unit": "ns/iter",
            "extra": "iterations: 49838712\ncpu: 79.26689798885654 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/1_feature",
            "value": 175.15390926818947,
            "unit": "ns/iter",
            "extra": "iterations: 25795672\ncpu: 175.12424564864995 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features",
            "value": 7287.403481185134,
            "unit": "ns/iter",
            "extra": "iterations: 578079\ncpu: 7286.169364394833 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features",
            "value": 12833.014546201379,
            "unit": "ns/iter",
            "extra": "iterations: 326202\ncpu: 12830.93451297049 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features_no_predict",
            "value": 3458.515206158063,
            "unit": "ns/iter",
            "extra": "iterations: 1200632\ncpu: 3457.9862938852216 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features_no_predic",
            "value": 5180.76325754087,
            "unit": "ns/iter",
            "extra": "iterations: 816309\ncpu: 5180.037951315001 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/few_features",
            "value": 1282.435807746772,
            "unit": "ns/iter",
            "extra": "iterations: 3262551\ncpu: 1282.2586987912227 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/many_features/min_time:15.000",
            "value": 63244.52063847821,
            "unit": "ns/iter",
            "extra": "iterations: 331977\ncpu: 63227.160616548725 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_no_namespaces/min_time:15.000",
            "value": 287948.68763092055,
            "unit": "ns/iter",
            "extra": "iterations: 79246\ncpu: 287895.2880902506 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 320555.9330164803,
            "unit": "ns/iter",
            "extra": "iterations: 65225\ncpu: 320466.60176312755 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_interactions/min_time:15.000",
            "value": 572977.9153835676,
            "unit": "ns/iter",
            "extra": "iterations: 36695\ncpu: 572860.460553209 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_no_interactions/min_time:15.000",
            "value": 290793.09820996656,
            "unit": "ns/iter",
            "extra": "iterations: 70278\ncpu: 290744.19448476046 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_interactions/min_time:15.000",
            "value": 439699.42057511,
            "unit": "ns/iter",
            "extra": "iterations: 47504\ncpu: 439579.0270293029 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_no_namespaces/min_time:15.000",
            "value": 645941.3970892072,
            "unit": "ns/iter",
            "extra": "iterations: 32912\ncpu: 645834.6499756925 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 836145.8325340068,
            "unit": "ns/iter",
            "extra": "iterations: 25438\ncpu: 835911.4788898493 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_interactions/min_time:15.000",
            "value": 2596385.91813445,
            "unit": "ns/iter",
            "extra": "iterations: 8062\ncpu: 2595951.848176632 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_no_interactions/min_time:15.000",
            "value": 664314.3024402742,
            "unit": "ns/iter",
            "extra": "iterations: 31226\ncpu: 664132.4441170818 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_interactions/min_time:15.000",
            "value": 1988751.3076923091,
            "unit": "ns/iter",
            "extra": "iterations: 10868\ncpu: 1988419.7644460788 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi_predict/cb_las_small_300_onestep/min_time:15.000/real_time",
            "value": 43.96784028316832,
            "unit": "ms/iter",
            "extra": "iterations: 505\ncpu: 43.95702514851487 ms\nthreads: 1"
          },
          {
            "name": "benchmark_multi_predict/cb_las_small_300_onestep_max_threads/min_time:15.000/real_time",
            "value": 24.550477373467032,
            "unit": "ms/iter",
            "extra": "iterations: 897\ncpu: 2.194174916387969 ms\nthreads: 1"
          },
          {
            "name": "benchmark_multi_predict/cb_las_small_300_plaincb/min_time:15.000/real_time",
            "value": 1.3357210681080374,
            "unit": "ms/iter",
            "extra": "iterations: 15402\ncpu: 1.3355133943643676 ms\nthreads: 1"
          },
          {
            "name": "benchmark_multi_predict/cb_las_small_500_onestep/min_time:15.000/real_time",
            "value": 80.309008146718,
            "unit": "ms/iter",
            "extra": "iterations: 259\ncpu: 80.28275482625476 ms\nthreads: 1"
          },
          {
            "name": "benchmark_multi_predict/cb_las_small_500_onestep_max_threads/min_time:15.000/real_time",
            "value": 42.61051388142312,
            "unit": "ms/iter",
            "extra": "iterations: 506\ncpu: 3.871529644268811 ms\nthreads: 1"
          },
          {
            "name": "benchmark_multi_predict/cb_las_small_500_plaincb/min_time:15.000/real_time",
            "value": 2.3955879173973176,
            "unit": "ms/iter",
            "extra": "iterations: 8668\ncpu: 2.3951452122750343 ms\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/simple/min_time:15.000",
            "value": 58561.287446797316,
            "unit": "ns/iter",
            "extra": "iterations: 371220\ncpu: 58551.66855234084 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/quadratic/min_time:15.000",
            "value": 3292480.958537724,
            "unit": "ns/iter",
            "extra": "iterations: 6319\ncpu: 3291485.614812467 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_string_fts",
            "value": 1096.278047990531,
            "unit": "ns/iter",
            "extra": "iterations: 3951825\ncpu: 1096.112808639041 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_string_fts",
            "value": 8267.137890329757,
            "unit": "ns/iter",
            "extra": "iterations: 536245\ncpu: 8265.894507174946 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_num_fts",
            "value": 823.7431309983422,
            "unit": "ns/iter",
            "extra": "iterations: 5142232\ncpu: 823.618751546015 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_num_fts",
            "value": 6107.205025485544,
            "unit": "ns/iter",
            "extra": "iterations: 712170\ncpu: 6106.331072637096 ns\nthreads: 1"
          },
          {
            "name": "benchmark_example_reuse",
            "value": 866.8954958309408,
            "unit": "ns/iter",
            "extra": "iterations: 4882188\ncpu: 866.7616240914924 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_char",
            "value": 59.43231084221674,
            "unit": "ns/iter",
            "extra": "iterations: 72698105\ncpu: 59.4234636514932 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_extent",
            "value": 85.95488416619818,
            "unit": "ns/iter",
            "extra": "iterations: 51754668\ncpu: 85.94081407304279 ns\nthreads: 1"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "olgavrou@gmail.com",
            "name": "olgavrou",
            "username": "olgavrou"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "93cf30869139d4459fd0207fb78552864863d049",
          "message": "chore: [LAS] remove compile time flag and its own custom CI (#4249)",
          "timestamp": "2022-10-26T12:40:54-04:00",
          "tree_id": "8835ef7b03fd40ba5aa926683135d901f36bc014",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/93cf30869139d4459fd0207fb78552864863d049"
        },
        "date": 1666804720670,
        "tool": "benchmarkdotnet",
        "benches": [
          {
            "name": "BenchmarkText.Benchmark(args: 120_num_features)",
            "value": 6005.173110961914,
            "unit": "ns",
            "range": "± 20.589406005901587"
          },
          {
            "name": "BenchmarkText.Benchmark(args: 120_string_fts)",
            "value": 9824.700273786273,
            "unit": "ns",
            "range": "± 54.63082081079674"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 1_feature)",
            "value": 674.8812675476074,
            "unit": "ns",
            "range": "± 2.111218452493523"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 8_features)",
            "value": 520.2445920308431,
            "unit": "ns",
            "range": "± 3.0250589138997355"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_interactions)",
            "value": 877145.4622395834,
            "unit": "ns",
            "range": "± 2182.0614432802813"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_no_interactions)",
            "value": 546127.265625,
            "unit": "ns",
            "range": "± 3315.3523397247245"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_no_namespaces)",
            "value": 560763.5221354166,
            "unit": "ns",
            "range": "± 3787.2320456939788"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_interactions)",
            "value": 882888.8444010416,
            "unit": "ns",
            "range": "± 3412.1429570907603"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_no_interactions)",
            "value": 543121.8489583334,
            "unit": "ns",
            "range": "± 2764.9461583267953"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_interactions)",
            "value": 3530759.6614583335,
            "unit": "ns",
            "range": "± 19919.262499103337"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_no_interactions)",
            "value": 1088729.19921875,
            "unit": "ns",
            "range": "± 3721.698542206569"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_no_namespaces)",
            "value": 927699.1471354166,
            "unit": "ns",
            "range": "± 5014.39402224709"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_interactions)",
            "value": 2923064.2838541665,
            "unit": "ns",
            "range": "± 12508.76068687381"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_no_interactions)",
            "value": 944313.65234375,
            "unit": "ns",
            "range": "± 2788.846624097316"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: few_features)",
            "value": 3243.791936238607,
            "unit": "ns",
            "range": "± 11.777701448995906"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: few_features)",
            "value": 11329.538370768229,
            "unit": "ns",
            "range": "± 21.119151911060733"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: many_features)",
            "value": 109526.37044270833,
            "unit": "ns",
            "range": "± 447.4209980818607"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: many_features)",
            "value": 19128.850911458332,
            "unit": "ns",
            "range": "± 76.91978580490407"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: quadratic)",
            "value": 4284065.46875,
            "unit": "ns",
            "range": "± 40014.32655795047"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: simple)",
            "value": 211006.572265625,
            "unit": "ns",
            "range": "± 1009.3462617352274"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "olgavrou@gmail.com",
            "name": "olgavrou",
            "username": "olgavrou"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "d263997870d0dbcc083324c1d88236c692b960c1",
          "message": "fix: don't run tests with iterations with asan and ubsan (#4253)",
          "timestamp": "2022-10-26T13:42:27-04:00",
          "tree_id": "28253379bb3025d2485aebf1226d151f4334aa7f",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/d263997870d0dbcc083324c1d88236c692b960c1"
        },
        "date": 1666807057265,
        "tool": "googlecpp",
        "benches": [
          {
            "name": "bench_text/120_string_fts",
            "value": 5220.9150923182815,
            "unit": "ns/iter",
            "extra": "iterations: 802766\ncpu: 5220.618337099478 ns\nthreads: 1"
          },
          {
            "name": "bench_text/120_num_fts",
            "value": 3955.7468453230817,
            "unit": "ns/iter",
            "extra": "iterations: 1012148\ncpu: 3955.495243778578 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/8_features",
            "value": 69.04156930379882,
            "unit": "ns/iter",
            "extra": "iterations: 60524444\ncpu: 68.99229838443456 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/1_feature",
            "value": 156.243582825014,
            "unit": "ns/iter",
            "extra": "iterations: 26800773\ncpu: 156.23413548556974 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features",
            "value": 7196.789772215739,
            "unit": "ns/iter",
            "extra": "iterations: 577564\ncpu: 7196.333739637511 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features",
            "value": 12896.79406304064,
            "unit": "ns/iter",
            "extra": "iterations: 331786\ncpu: 12890.760007956946 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features_no_predict",
            "value": 3354.4289115866,
            "unit": "ns/iter",
            "extra": "iterations: 1235560\ncpu: 3354.1747871410516 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features_no_predic",
            "value": 5049.535885894226,
            "unit": "ns/iter",
            "extra": "iterations: 829546\ncpu: 5049.239945705242 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/few_features",
            "value": 1251.6099033731136,
            "unit": "ns/iter",
            "extra": "iterations: 3261515\ncpu: 1251.5445736107306 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/many_features/min_time:15.000",
            "value": 51786.701919241204,
            "unit": "ns/iter",
            "extra": "iterations: 406098\ncpu: 51772.99691207538 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_no_namespaces/min_time:15.000",
            "value": 198054.06600391836,
            "unit": "ns/iter",
            "extra": "iterations: 104615\ncpu: 198037.04248912673 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 223522.1949668063,
            "unit": "ns/iter",
            "extra": "iterations: 93539\ncpu: 223511.1525673782 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_interactions/min_time:15.000",
            "value": 455688.88757115667,
            "unit": "ns/iter",
            "extra": "iterations: 46376\ncpu: 455657.62032085535 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_no_interactions/min_time:15.000",
            "value": 199110.00311633348,
            "unit": "ns/iter",
            "extra": "iterations: 104931\ncpu: 199099.44916183 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_interactions/min_time:15.000",
            "value": 344615.1441840753,
            "unit": "ns/iter",
            "extra": "iterations: 61366\ncpu: 344599.35143238935 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_no_namespaces/min_time:15.000",
            "value": 579826.7161075532,
            "unit": "ns/iter",
            "extra": "iterations: 35443\ncpu: 579755.1053804705 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 721927.9024123533,
            "unit": "ns/iter",
            "extra": "iterations: 29266\ncpu: 721884.1932618052 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_interactions/min_time:15.000",
            "value": 2207686.4719444155,
            "unit": "ns/iter",
            "extra": "iterations: 9499\ncpu: 2207560.595852197 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_no_interactions/min_time:15.000",
            "value": 596329.002931078,
            "unit": "ns/iter",
            "extra": "iterations: 35823\ncpu: 596293.9703542427 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_interactions/min_time:15.000",
            "value": 1745581.920937527,
            "unit": "ns/iter",
            "extra": "iterations: 12117\ncpu: 1745489.378559052 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi_predict/cb_las_small_300_onestep/min_time:15.000/real_time",
            "value": 46.634228748888894,
            "unit": "ms/iter",
            "extra": "iterations: 450\ncpu: 46.63181266666659 ms\nthreads: 1"
          },
          {
            "name": "benchmark_multi_predict/cb_las_small_300_onestep_max_threads/min_time:15.000/real_time",
            "value": 24.555455267523456,
            "unit": "ms/iter",
            "extra": "iterations: 856\ncpu: 1.9546918224298553 ms\nthreads: 1"
          },
          {
            "name": "benchmark_multi_predict/cb_las_small_300_plaincb/min_time:15.000/real_time",
            "value": 1.2021733082956434,
            "unit": "ms/iter",
            "extra": "iterations: 17467\ncpu: 1.2020967252533354 ms\nthreads: 1"
          },
          {
            "name": "benchmark_multi_predict/cb_las_small_500_onestep/min_time:15.000/real_time",
            "value": 77.81872456296284,
            "unit": "ms/iter",
            "extra": "iterations: 270\ncpu: 77.81303444444431 ms\nthreads: 1"
          },
          {
            "name": "benchmark_multi_predict/cb_las_small_500_onestep_max_threads/min_time:15.000/real_time",
            "value": 40.79696106311645,
            "unit": "ms/iter",
            "extra": "iterations: 507\ncpu: 3.1204378698223882 ms\nthreads: 1"
          },
          {
            "name": "benchmark_multi_predict/cb_las_small_500_plaincb/min_time:15.000/real_time",
            "value": 2.0252075807510375,
            "unit": "ms/iter",
            "extra": "iterations: 10359\ncpu: 2.025110647745924 ms\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/simple/min_time:15.000",
            "value": 48465.56572042319,
            "unit": "ns/iter",
            "extra": "iterations: 431989\ncpu: 48463.94607270103 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/quadratic/min_time:15.000",
            "value": 2188029.885890361,
            "unit": "ns/iter",
            "extra": "iterations: 9412\ncpu: 2187923.6294092634 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_string_fts",
            "value": 886.7939194264711,
            "unit": "ns/iter",
            "extra": "iterations: 4712812\ncpu: 886.754765520042 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_string_fts",
            "value": 6249.34022316003,
            "unit": "ns/iter",
            "extra": "iterations: 669296\ncpu: 6248.940528555337 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_num_fts",
            "value": 679.291530649959,
            "unit": "ns/iter",
            "extra": "iterations: 6162539\ncpu: 679.2617133944291 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_num_fts",
            "value": 4561.143824581684,
            "unit": "ns/iter",
            "extra": "iterations: 924237\ncpu: 4560.946272438791 ns\nthreads: 1"
          },
          {
            "name": "benchmark_example_reuse",
            "value": 779.7548528913438,
            "unit": "ns/iter",
            "extra": "iterations: 5386325\ncpu: 779.7178967106416 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_char",
            "value": 43.847791678929795,
            "unit": "ns/iter",
            "extra": "iterations: 95819400\ncpu: 43.84582140985953 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_extent",
            "value": 56.35718486566325,
            "unit": "ns/iter",
            "extra": "iterations: 74991590\ncpu: 56.354737911277596 ns\nthreads: 1"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "olgavrou@gmail.com",
            "name": "olgavrou",
            "username": "olgavrou"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "d263997870d0dbcc083324c1d88236c692b960c1",
          "message": "fix: don't run tests with iterations with asan and ubsan (#4253)",
          "timestamp": "2022-10-26T13:42:27-04:00",
          "tree_id": "28253379bb3025d2485aebf1226d151f4334aa7f",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/d263997870d0dbcc083324c1d88236c692b960c1"
        },
        "date": 1666808430894,
        "tool": "benchmarkdotnet",
        "benches": [
          {
            "name": "BenchmarkText.Benchmark(args: 120_num_features)",
            "value": 6077.68793741862,
            "unit": "ns",
            "range": "± 51.3703654930759"
          },
          {
            "name": "BenchmarkText.Benchmark(args: 120_string_fts)",
            "value": 9824.154154459635,
            "unit": "ns",
            "range": "± 32.99527733211896"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 1_feature)",
            "value": 681.726271311442,
            "unit": "ns",
            "range": "± 1.5727800030963703"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 8_features)",
            "value": 541.3680585225424,
            "unit": "ns",
            "range": "± 4.524154321245391"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_interactions)",
            "value": 903862.3502604166,
            "unit": "ns",
            "range": "± 1926.6479483374146"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_no_interactions)",
            "value": 564270.8919270834,
            "unit": "ns",
            "range": "± 1731.9490617743527"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_no_namespaces)",
            "value": 564521.0309709822,
            "unit": "ns",
            "range": "± 2161.547694665388"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_interactions)",
            "value": 898202.0808293269,
            "unit": "ns",
            "range": "± 2027.7976229375122"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_no_interactions)",
            "value": 563097.0377604166,
            "unit": "ns",
            "range": "± 1844.2757656906476"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_interactions)",
            "value": 3662930.234375,
            "unit": "ns",
            "range": "± 12067.61216182495"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_no_interactions)",
            "value": 1098484.3880208333,
            "unit": "ns",
            "range": "± 2045.3531166070084"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_no_namespaces)",
            "value": 930810.3125,
            "unit": "ns",
            "range": "± 3309.4970797524084"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_interactions)",
            "value": 2847653.3333333335,
            "unit": "ns",
            "range": "± 12473.829736971034"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_no_interactions)",
            "value": 941028.2486979166,
            "unit": "ns",
            "range": "± 2329.046589620447"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: few_features)",
            "value": 3256.6959635416665,
            "unit": "ns",
            "range": "± 12.611419698522024"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: few_features)",
            "value": 11346.498413085938,
            "unit": "ns",
            "range": "± 17.713828677756815"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: many_features)",
            "value": 109807.93701171875,
            "unit": "ns",
            "range": "± 562.4310153561136"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: many_features)",
            "value": 19019.85819498698,
            "unit": "ns",
            "range": "± 59.275805415285475"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: quadratic)",
            "value": 4241917.608173077,
            "unit": "ns",
            "range": "± 10399.31495374089"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: simple)",
            "value": 210233.984375,
            "unit": "ns",
            "range": "± 925.0874902338908"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "olgavrou@gmail.com",
            "name": "olgavrou",
            "username": "olgavrou"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "db77288403613b3ca2f0b07bcf3a0fdaa8799b43",
          "message": "fix: [LAS] block size should never be zero (#4252)",
          "timestamp": "2022-10-26T14:28:20-04:00",
          "tree_id": "c674fc8728aa8fb207d2fb4d93664b185cad66ca",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/db77288403613b3ca2f0b07bcf3a0fdaa8799b43"
        },
        "date": 1666809836357,
        "tool": "googlecpp",
        "benches": [
          {
            "name": "bench_text/120_string_fts",
            "value": 6244.864175810553,
            "unit": "ns/iter",
            "extra": "iterations: 672406\ncpu: 6241.799299827782 ns\nthreads: 1"
          },
          {
            "name": "bench_text/120_num_fts",
            "value": 4822.392562394658,
            "unit": "ns/iter",
            "extra": "iterations: 874233\ncpu: 4821.862821467503 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/8_features",
            "value": 75.67523519366013,
            "unit": "ns/iter",
            "extra": "iterations: 56457623\ncpu: 75.65563466956445 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/1_feature",
            "value": 165.0110840427561,
            "unit": "ns/iter",
            "extra": "iterations: 25548079\ncpu: 164.99620186707583 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features",
            "value": 7017.322760116537,
            "unit": "ns/iter",
            "extra": "iterations: 595656\ncpu: 7016.316128772311 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features",
            "value": 12145.327913922853,
            "unit": "ns/iter",
            "extra": "iterations: 336814\ncpu: 12143.35241409205 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features_no_predict",
            "value": 3361.8699695890514,
            "unit": "ns/iter",
            "extra": "iterations: 1263689\ncpu: 3359.853650700449 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features_no_predic",
            "value": 4987.693650614802,
            "unit": "ns/iter",
            "extra": "iterations: 843625\ncpu: 4987.206400948281 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/few_features",
            "value": 1256.6197727017595,
            "unit": "ns/iter",
            "extra": "iterations: 3377677\ncpu: 1256.5091333481578 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/many_features/min_time:15.000",
            "value": 56314.17010614658,
            "unit": "ns/iter",
            "extra": "iterations: 371374\ncpu: 56309.723082391334 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_no_namespaces/min_time:15.000",
            "value": 250350.15657833562,
            "unit": "ns/iter",
            "extra": "iterations: 86040\ncpu: 250328.64481636454 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 280103.92012885865,
            "unit": "ns/iter",
            "extra": "iterations: 75121\ncpu: 280079.50639634725 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_interactions/min_time:15.000",
            "value": 519764.01806378085,
            "unit": "ns/iter",
            "extra": "iterations: 40357\ncpu: 519713.7250043365 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_no_interactions/min_time:15.000",
            "value": 247383.8050859401,
            "unit": "ns/iter",
            "extra": "iterations: 85176\ncpu: 247361.63473278857 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_interactions/min_time:15.000",
            "value": 426782.2239331779,
            "unit": "ns/iter",
            "extra": "iterations: 52797\ncpu: 426737.6176676703 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_no_namespaces/min_time:15.000",
            "value": 616448.250232775,
            "unit": "ns/iter",
            "extra": "iterations: 34368\ncpu: 616382.655377095 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 765734.5768637529,
            "unit": "ns/iter",
            "extra": "iterations: 27230\ncpu: 765658.9864120451 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_interactions/min_time:15.000",
            "value": 2403479.5537733654,
            "unit": "ns/iter",
            "extra": "iterations: 8666\ncpu: 2403246.480498498 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_no_interactions/min_time:15.000",
            "value": 644278.95199546,
            "unit": "ns/iter",
            "extra": "iterations: 33476\ncpu: 644215.0645238379 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_interactions/min_time:15.000",
            "value": 1820146.0568687273,
            "unit": "ns/iter",
            "extra": "iterations: 11465\ncpu: 1819952.603576101 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi_predict/cb_las_small_300_onestep/min_time:15.000/real_time",
            "value": 40.88143557894734,
            "unit": "ms/iter",
            "extra": "iterations: 513\ncpu: 40.877561403508786 ms\nthreads: 1"
          },
          {
            "name": "benchmark_multi_predict/cb_las_small_300_onestep_max_threads/min_time:15.000/real_time",
            "value": 21.879186920207268,
            "unit": "ms/iter",
            "extra": "iterations: 965\ncpu: 2.04577471502588 ms\nthreads: 1"
          },
          {
            "name": "benchmark_multi_predict/cb_las_small_300_plaincb/min_time:15.000/real_time",
            "value": 1.1647832459134122,
            "unit": "ms/iter",
            "extra": "iterations: 18108\ncpu: 1.1645273912083063 ms\nthreads: 1"
          },
          {
            "name": "benchmark_multi_predict/cb_las_small_500_onestep/min_time:15.000/real_time",
            "value": 68.29499494498397,
            "unit": "ms/iter",
            "extra": "iterations: 309\ncpu: 68.28768317152105 ms\nthreads: 1"
          },
          {
            "name": "benchmark_multi_predict/cb_las_small_500_onestep_max_threads/min_time:15.000/real_time",
            "value": 36.109928953448126,
            "unit": "ms/iter",
            "extra": "iterations: 580\ncpu: 3.2543218965517537 ms\nthreads: 1"
          },
          {
            "name": "benchmark_multi_predict/cb_las_small_500_plaincb/min_time:15.000/real_time",
            "value": 1.9551281901151287,
            "unit": "ms/iter",
            "extra": "iterations: 10683\ncpu: 1.9549602265281307 ms\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/simple/min_time:15.000",
            "value": 50966.83709756442,
            "unit": "ns/iter",
            "extra": "iterations: 406059\ncpu: 50962.671434446645 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/quadratic/min_time:15.000",
            "value": 2664332.1891683554,
            "unit": "ns/iter",
            "extra": "iterations: 7792\ncpu: 2664103.272587267 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_string_fts",
            "value": 948.0852058180749,
            "unit": "ns/iter",
            "extra": "iterations: 4444896\ncpu: 948.0219334715573 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_string_fts",
            "value": 7346.8733962458045,
            "unit": "ns/iter",
            "extra": "iterations: 572329\ncpu: 7346.364590995764 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_num_fts",
            "value": 718.9836409850866,
            "unit": "ns/iter",
            "extra": "iterations: 5854509\ncpu: 718.9335091977701 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_num_fts",
            "value": 5423.368452960938,
            "unit": "ns/iter",
            "extra": "iterations: 771952\ncpu: 5423.002077849372 ns\nthreads: 1"
          },
          {
            "name": "benchmark_example_reuse",
            "value": 785.5193635187647,
            "unit": "ns/iter",
            "extra": "iterations: 5374927\ncpu: 785.4649746871021 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_char",
            "value": 53.93821768050259,
            "unit": "ns/iter",
            "extra": "iterations: 77883107\ncpu: 53.93455862001026 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_extent",
            "value": 74.6315987560193,
            "unit": "ns/iter",
            "extra": "iterations: 56373561\ncpu: 74.62692839290423 ns\nthreads: 1"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "olgavrou@gmail.com",
            "name": "olgavrou",
            "username": "olgavrou"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "db77288403613b3ca2f0b07bcf3a0fdaa8799b43",
          "message": "fix: [LAS] block size should never be zero (#4252)",
          "timestamp": "2022-10-26T14:28:20-04:00",
          "tree_id": "c674fc8728aa8fb207d2fb4d93664b185cad66ca",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/db77288403613b3ca2f0b07bcf3a0fdaa8799b43"
        },
        "date": 1666811562859,
        "tool": "benchmarkdotnet",
        "benches": [
          {
            "name": "BenchmarkText.Benchmark(args: 120_num_features)",
            "value": 6130.573763166155,
            "unit": "ns",
            "range": "± 54.77073552804192"
          },
          {
            "name": "BenchmarkText.Benchmark(args: 120_string_fts)",
            "value": 9735.629381452289,
            "unit": "ns",
            "range": "± 33.32205108866609"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 1_feature)",
            "value": 683.6581548055013,
            "unit": "ns",
            "range": "± 2.3978659357201955"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 8_features)",
            "value": 552.2875990186419,
            "unit": "ns",
            "range": "± 3.237077495890704"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_interactions)",
            "value": 870902.44140625,
            "unit": "ns",
            "range": "± 2021.2385028559645"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_no_interactions)",
            "value": 529843.125,
            "unit": "ns",
            "range": "± 3346.405029435073"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_no_namespaces)",
            "value": 547826.6252790178,
            "unit": "ns",
            "range": "± 2078.2711342089947"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_interactions)",
            "value": 879964.3815104166,
            "unit": "ns",
            "range": "± 3200.358079661644"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_no_interactions)",
            "value": 540270.712890625,
            "unit": "ns",
            "range": "± 1756.5720642684175"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_interactions)",
            "value": 3533519.296875,
            "unit": "ns",
            "range": "± 12735.475975529029"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_no_interactions)",
            "value": 1093799.2578125,
            "unit": "ns",
            "range": "± 3728.788283148679"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_no_namespaces)",
            "value": 932465.9895833334,
            "unit": "ns",
            "range": "± 6742.733115697164"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_interactions)",
            "value": 2896157.7604166665,
            "unit": "ns",
            "range": "± 12376.02847879369"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_no_interactions)",
            "value": 945444.55078125,
            "unit": "ns",
            "range": "± 2642.6970565723996"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: few_features)",
            "value": 3231.037063598633,
            "unit": "ns",
            "range": "± 15.477418752018705"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: few_features)",
            "value": 11195.77113560268,
            "unit": "ns",
            "range": "± 17.810849106842426"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: many_features)",
            "value": 109563.18882533482,
            "unit": "ns",
            "range": "± 367.0434023695491"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: many_features)",
            "value": 18873.496907552082,
            "unit": "ns",
            "range": "± 54.231393018778256"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: quadratic)",
            "value": 4262382.03125,
            "unit": "ns",
            "range": "± 31963.66354166467"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: simple)",
            "value": 211221.474609375,
            "unit": "ns",
            "range": "± 997.8188441553903"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "olgavrou@gmail.com",
            "name": "olgavrou",
            "username": "olgavrou"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "b9b9b7e6a49cdc02f0e0d21c7c671c91d9088111",
          "message": "fix: [LAS] always return full predictions (#4255)",
          "timestamp": "2022-10-28T11:50:00-04:00",
          "tree_id": "c5f8c7a9cee3fa813ad7cbeff6d71de30eb19698",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/b9b9b7e6a49cdc02f0e0d21c7c671c91d9088111"
        },
        "date": 1666973116882,
        "tool": "googlecpp",
        "benches": [
          {
            "name": "bench_text/120_string_fts",
            "value": 5747.169041030928,
            "unit": "ns/iter",
            "extra": "iterations: 729669\ncpu: 5746.081168310563 ns\nthreads: 1"
          },
          {
            "name": "bench_text/120_num_fts",
            "value": 3932.69394059992,
            "unit": "ns/iter",
            "extra": "iterations: 1070436\ncpu: 3931.7087616634717 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/8_features",
            "value": 70.99617685785493,
            "unit": "ns/iter",
            "extra": "iterations: 57466082\ncpu: 70.99415617024316 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/1_feature",
            "value": 160.65138466842046,
            "unit": "ns/iter",
            "extra": "iterations: 26153626\ncpu: 160.6458890251012 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features",
            "value": 6975.180370216795,
            "unit": "ns/iter",
            "extra": "iterations: 602350\ncpu: 6974.51564704906 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features",
            "value": 12307.47943546954,
            "unit": "ns/iter",
            "extra": "iterations: 336988\ncpu: 12306.925172409696 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features_no_predict",
            "value": 3283.4873555810805,
            "unit": "ns/iter",
            "extra": "iterations: 1272340\ncpu: 3283.3933539777117 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features_no_predic",
            "value": 5052.196534896592,
            "unit": "ns/iter",
            "extra": "iterations: 828489\ncpu: 5051.966773246234 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/few_features",
            "value": 1249.9053517777072,
            "unit": "ns/iter",
            "extra": "iterations: 3340348\ncpu: 1249.870642220512 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/many_features/min_time:15.000",
            "value": 52236.98150951162,
            "unit": "ns/iter",
            "extra": "iterations: 403667\ncpu: 52229.805755734305 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_no_namespaces/min_time:15.000",
            "value": 198823.71361297744,
            "unit": "ns/iter",
            "extra": "iterations: 105539\ncpu: 198815.17827532953 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 225425.97651261013,
            "unit": "ns/iter",
            "extra": "iterations: 92390\ncpu: 225398.5279792186 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_interactions/min_time:15.000",
            "value": 459943.0858254554,
            "unit": "ns/iter",
            "extra": "iterations: 44206\ncpu: 459923.53979097854 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_no_interactions/min_time:15.000",
            "value": 198821.3962128792,
            "unit": "ns/iter",
            "extra": "iterations: 105938\ncpu: 198812.95852290952 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_interactions/min_time:15.000",
            "value": 342313.18742653716,
            "unit": "ns/iter",
            "extra": "iterations: 61256\ncpu: 342274.851443124 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_no_namespaces/min_time:15.000",
            "value": 578140.3744783493,
            "unit": "ns/iter",
            "extra": "iterations: 36902\ncpu: 578112.3489241772 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 714296.6052497535,
            "unit": "ns/iter",
            "extra": "iterations: 29449\ncpu: 714211.9936160827 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_interactions/min_time:15.000",
            "value": 2207640.2790307407,
            "unit": "ns/iter",
            "extra": "iterations: 9533\ncpu: 2207521.242001468 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_no_interactions/min_time:15.000",
            "value": 589025.0299788233,
            "unit": "ns/iter",
            "extra": "iterations: 35892\ncpu: 588956.522344812 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_interactions/min_time:15.000",
            "value": 1753256.2014894225,
            "unit": "ns/iter",
            "extra": "iterations: 11951\ncpu: 1753177.2989707962 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi_predict/cb_las_small_300_onestep/min_time:15.000/real_time",
            "value": 46.44460551106196,
            "unit": "ms/iter",
            "extra": "iterations: 452\ncpu: 46.44265331858407 ms\nthreads: 1"
          },
          {
            "name": "benchmark_multi_predict/cb_las_small_300_onestep_max_threads/min_time:15.000/real_time",
            "value": 24.369283291421883,
            "unit": "ms/iter",
            "extra": "iterations: 851\ncpu: 1.8589002350176036 ms\nthreads: 1"
          },
          {
            "name": "benchmark_multi_predict/cb_las_small_300_plaincb/min_time:15.000/real_time",
            "value": 1.231050482249913,
            "unit": "ms/iter",
            "extra": "iterations: 17014\ncpu: 1.2310004290584218 ms\nthreads: 1"
          },
          {
            "name": "benchmark_multi_predict/cb_las_small_500_onestep/min_time:15.000/real_time",
            "value": 78.3290793358209,
            "unit": "ms/iter",
            "extra": "iterations: 268\ncpu: 78.32546343283573 ms\nthreads: 1"
          },
          {
            "name": "benchmark_multi_predict/cb_las_small_500_onestep_max_threads/min_time:15.000/real_time",
            "value": 40.56857713513504,
            "unit": "ms/iter",
            "extra": "iterations: 518\ncpu: 3.085375096525148 ms\nthreads: 1"
          },
          {
            "name": "benchmark_multi_predict/cb_las_small_500_plaincb/min_time:15.000/real_time",
            "value": 2.038718257015685,
            "unit": "ms/iter",
            "extra": "iterations: 10334\ncpu: 2.0384129185213813 ms\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/simple/min_time:15.000",
            "value": 47513.16009127044,
            "unit": "ns/iter",
            "extra": "iterations: 437819\ncpu: 47513.802050619066 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/quadratic/min_time:15.000",
            "value": 2197135.1971174236,
            "unit": "ns/iter",
            "extra": "iterations: 9436\ncpu: 2196867.062314545 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_string_fts",
            "value": 882.143542657013,
            "unit": "ns/iter",
            "extra": "iterations: 4738466\ncpu: 882.124911310964 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_string_fts",
            "value": 6779.116670273362,
            "unit": "ns/iter",
            "extra": "iterations: 619198\ncpu: 6779.037884489353 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_num_fts",
            "value": 702.0312751336626,
            "unit": "ns/iter",
            "extra": "iterations: 5933276\ncpu: 702.0180082638992 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_num_fts",
            "value": 4523.425515339416,
            "unit": "ns/iter",
            "extra": "iterations: 926768\ncpu: 4522.674606805529 ns\nthreads: 1"
          },
          {
            "name": "benchmark_example_reuse",
            "value": 741.5498772160147,
            "unit": "ns/iter",
            "extra": "iterations: 5678264\ncpu: 741.5355115577584 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_char",
            "value": 40.608377340117755,
            "unit": "ns/iter",
            "extra": "iterations: 103409052\ncpu: 40.60778547703938 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_extent",
            "value": 53.69150217344182,
            "unit": "ns/iter",
            "extra": "iterations: 78021856\ncpu: 53.689871207370196 ns\nthreads: 1"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "olgavrou@gmail.com",
            "name": "olgavrou",
            "username": "olgavrou"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "b9b9b7e6a49cdc02f0e0d21c7c671c91d9088111",
          "message": "fix: [LAS] always return full predictions (#4255)",
          "timestamp": "2022-10-28T11:50:00-04:00",
          "tree_id": "c5f8c7a9cee3fa813ad7cbeff6d71de30eb19698",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/b9b9b7e6a49cdc02f0e0d21c7c671c91d9088111"
        },
        "date": 1666974444676,
        "tool": "benchmarkdotnet",
        "benches": [
          {
            "name": "BenchmarkText.Benchmark(args: 120_num_features)",
            "value": 6074.623325892857,
            "unit": "ns",
            "range": "± 48.30339145101482"
          },
          {
            "name": "BenchmarkText.Benchmark(args: 120_string_fts)",
            "value": 9706.061335972377,
            "unit": "ns",
            "range": "± 43.49554324001472"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 1_feature)",
            "value": 666.207586924235,
            "unit": "ns",
            "range": "± 3.975199977627215"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 8_features)",
            "value": 529.9323972066244,
            "unit": "ns",
            "range": "± 2.794844461080339"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_interactions)",
            "value": 917960.87890625,
            "unit": "ns",
            "range": "± 1537.5215011847756"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_no_interactions)",
            "value": 565594.5703125,
            "unit": "ns",
            "range": "± 1588.8329529091525"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_no_namespaces)",
            "value": 569235.8119419643,
            "unit": "ns",
            "range": "± 1268.0185435899582"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_interactions)",
            "value": 911147.724609375,
            "unit": "ns",
            "range": "± 1800.0327316214618"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_no_interactions)",
            "value": 563021.6569010416,
            "unit": "ns",
            "range": "± 1300.2864000614177"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_interactions)",
            "value": 3534059.0625,
            "unit": "ns",
            "range": "± 8598.407609596032"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_no_interactions)",
            "value": 1102256.0546875,
            "unit": "ns",
            "range": "± 2641.073093932889"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_no_namespaces)",
            "value": 945402.8374565972,
            "unit": "ns",
            "range": "± 19807.309549230395"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_interactions)",
            "value": 2874835.847355769,
            "unit": "ns",
            "range": "± 17208.536135475042"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_no_interactions)",
            "value": 940353.8802083334,
            "unit": "ns",
            "range": "± 2572.614986256507"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: few_features)",
            "value": 3232.2949273245677,
            "unit": "ns",
            "range": "± 14.274212479649925"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: few_features)",
            "value": 11314.202335902623,
            "unit": "ns",
            "range": "± 26.55195198715676"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: many_features)",
            "value": 109363.271484375,
            "unit": "ns",
            "range": "± 256.57424023385477"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: many_features)",
            "value": 18828.82080078125,
            "unit": "ns",
            "range": "± 40.05673000820326"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: quadratic)",
            "value": 4327359.166666667,
            "unit": "ns",
            "range": "± 40959.33647400283"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: simple)",
            "value": 206495.1708984375,
            "unit": "ns",
            "range": "± 1329.8423862973725"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "jackgerrits@users.noreply.github.com",
            "name": "Jack Gerrits",
            "username": "jackgerrits"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "c4781776cc559bdbe918d715112e3d6137a8ad19",
          "message": "refactor: use model_utils for save_load in las (#4263)",
          "timestamp": "2022-11-02T12:33:14-04:00",
          "tree_id": "549b8acc18cc70947ace2535269a892a799e6824",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/c4781776cc559bdbe918d715112e3d6137a8ad19"
        },
        "date": 1667407745072,
        "tool": "googlecpp",
        "benches": [
          {
            "name": "bench_text/120_string_fts",
            "value": 6979.63971011943,
            "unit": "ns/iter",
            "extra": "iterations: 602179\ncpu: 6978.6073576129365 ns\nthreads: 1"
          },
          {
            "name": "bench_text/120_num_fts",
            "value": 4770.2243947194775,
            "unit": "ns/iter",
            "extra": "iterations: 883929\ncpu: 4769.731392453466 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/8_features",
            "value": 77.46552645823749,
            "unit": "ns/iter",
            "extra": "iterations: 56195749\ncpu: 77.4288816757296 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/1_feature",
            "value": 163.02038114239787,
            "unit": "ns/iter",
            "extra": "iterations: 25760136\ncpu: 163.00981485501478 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features",
            "value": 6951.805193759545,
            "unit": "ns/iter",
            "extra": "iterations: 596177\ncpu: 6951.157626007043 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features",
            "value": 12212.029346718866,
            "unit": "ns/iter",
            "extra": "iterations: 338675\ncpu: 12210.947368421055 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features_no_predict",
            "value": 3330.064700745,
            "unit": "ns/iter",
            "extra": "iterations: 1262953\ncpu: 3329.836423049788 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features_no_predic",
            "value": 5012.626336897618,
            "unit": "ns/iter",
            "extra": "iterations: 839537\ncpu: 5012.247822311587 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/few_features",
            "value": 1227.544636861965,
            "unit": "ns/iter",
            "extra": "iterations: 3419416\ncpu: 1227.4533721547753 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/many_features/min_time:15.000",
            "value": 62162.81655925226,
            "unit": "ns/iter",
            "extra": "iterations: 336392\ncpu: 62148.366191823836 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_no_namespaces/min_time:15.000",
            "value": 262824.10608592856,
            "unit": "ns/iter",
            "extra": "iterations: 87957\ncpu: 262799.1780074355 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 283488.1107740024,
            "unit": "ns/iter",
            "extra": "iterations: 74160\ncpu: 283461.3942826321 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_interactions/min_time:15.000",
            "value": 532370.7556160645,
            "unit": "ns/iter",
            "extra": "iterations: 39663\ncpu: 532272.0293472504 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_no_interactions/min_time:15.000",
            "value": 251232.78895314652,
            "unit": "ns/iter",
            "extra": "iterations: 83517\ncpu: 251208.76707736164 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_interactions/min_time:15.000",
            "value": 406391.19734548865,
            "unit": "ns/iter",
            "extra": "iterations: 52213\ncpu: 406311.916572501 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_no_namespaces/min_time:15.000",
            "value": 640964.0040681642,
            "unit": "ns/iter",
            "extra": "iterations: 35151\ncpu: 640892.7740320333 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 772437.1389394008,
            "unit": "ns/iter",
            "extra": "iterations: 26947\ncpu: 772280.0460162534 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_interactions/min_time:15.000",
            "value": 2533069.399683079,
            "unit": "ns/iter",
            "extra": "iterations: 8204\ncpu: 2532798.598244753 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_no_interactions/min_time:15.000",
            "value": 618679.2462793905,
            "unit": "ns/iter",
            "extra": "iterations: 34067\ncpu: 618561.6960695104 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_interactions/min_time:15.000",
            "value": 1910661.4704818646,
            "unit": "ns/iter",
            "extra": "iterations: 11061\ncpu: 1910332.311725886 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi_predict/cb_las_small_300_onestep/min_time:15.000/real_time",
            "value": 40.95632487937749,
            "unit": "ms/iter",
            "extra": "iterations: 514\ncpu: 40.94993424124514 ms\nthreads: 1"
          },
          {
            "name": "benchmark_multi_predict/cb_las_small_300_onestep_max_threads/min_time:15.000/real_time",
            "value": 21.995482549163153,
            "unit": "ms/iter",
            "extra": "iterations: 956\ncpu: 2.192736087866139 ms\nthreads: 1"
          },
          {
            "name": "benchmark_multi_predict/cb_las_small_300_plaincb/min_time:15.000/real_time",
            "value": 1.3123793451808319,
            "unit": "ms/iter",
            "extra": "iterations: 16009\ncpu: 1.3122548066712492 ms\nthreads: 1"
          },
          {
            "name": "benchmark_multi_predict/cb_las_small_500_onestep/min_time:15.000/real_time",
            "value": 68.57431202941154,
            "unit": "ms/iter",
            "extra": "iterations: 306\ncpu: 68.56759477124174 ms\nthreads: 1"
          },
          {
            "name": "benchmark_multi_predict/cb_las_small_500_onestep_max_threads/min_time:15.000/real_time",
            "value": 36.44830140172413,
            "unit": "ms/iter",
            "extra": "iterations: 580\ncpu: 3.6128072413793357 ms\nthreads: 1"
          },
          {
            "name": "benchmark_multi_predict/cb_las_small_500_plaincb/min_time:15.000/real_time",
            "value": 2.254673693185489,
            "unit": "ms/iter",
            "extra": "iterations: 9289\ncpu: 2.254470911831199 ms\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/simple/min_time:15.000",
            "value": 51102.43556809587,
            "unit": "ns/iter",
            "extra": "iterations: 424937\ncpu: 51098.46400760589 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/quadratic/min_time:15.000",
            "value": 2859190.0102541335,
            "unit": "ns/iter",
            "extra": "iterations: 6729\ncpu: 2858938.802199433 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_string_fts",
            "value": 950.0100205424878,
            "unit": "ns/iter",
            "extra": "iterations: 4415729\ncpu: 949.9035153651911 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_string_fts",
            "value": 8761.044694664442,
            "unit": "ns/iter",
            "extra": "iterations: 472562\ncpu: 8760.550784870515 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_num_fts",
            "value": 753.6410883890514,
            "unit": "ns/iter",
            "extra": "iterations: 5573779\ncpu: 753.5903917252512 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_num_fts",
            "value": 5708.294180161601,
            "unit": "ns/iter",
            "extra": "iterations: 732065\ncpu: 5707.8359162096285 ns\nthreads: 1"
          },
          {
            "name": "benchmark_example_reuse",
            "value": 816.2186383551124,
            "unit": "ns/iter",
            "extra": "iterations: 5164254\ncpu: 816.1592748923466 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_char",
            "value": 51.98959455717615,
            "unit": "ns/iter",
            "extra": "iterations: 80750720\ncpu: 51.98625349718276 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_extent",
            "value": 74.08012801909118,
            "unit": "ns/iter",
            "extra": "iterations: 56336910\ncpu: 74.07522883310578 ns\nthreads: 1"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "jackgerrits@users.noreply.github.com",
            "name": "Jack Gerrits",
            "username": "jackgerrits"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "c4781776cc559bdbe918d715112e3d6137a8ad19",
          "message": "refactor: use model_utils for save_load in las (#4263)",
          "timestamp": "2022-11-02T12:33:14-04:00",
          "tree_id": "549b8acc18cc70947ace2535269a892a799e6824",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/c4781776cc559bdbe918d715112e3d6137a8ad19"
        },
        "date": 1667409419863,
        "tool": "benchmarkdotnet",
        "benches": [
          {
            "name": "BenchmarkText.Benchmark(args: 120_num_features)",
            "value": 5992.33404306265,
            "unit": "ns",
            "range": "± 25.009300993869633"
          },
          {
            "name": "BenchmarkText.Benchmark(args: 120_string_fts)",
            "value": 9707.998439243862,
            "unit": "ns",
            "range": "± 46.86546512158321"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 1_feature)",
            "value": 684.6727879842123,
            "unit": "ns",
            "range": "± 3.0175313630187532"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 8_features)",
            "value": 521.1437020983014,
            "unit": "ns",
            "range": "± 2.004085843021427"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_interactions)",
            "value": 903942.3958333334,
            "unit": "ns",
            "range": "± 3884.1928640574874"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_no_interactions)",
            "value": 554991.8870192308,
            "unit": "ns",
            "range": "± 1280.6054464718395"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_no_namespaces)",
            "value": 562045.5705915178,
            "unit": "ns",
            "range": "± 3072.9302826611306"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_interactions)",
            "value": 906085.1449819711,
            "unit": "ns",
            "range": "± 3629.0615229053233"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_no_interactions)",
            "value": 551711.9075520834,
            "unit": "ns",
            "range": "± 1768.228465149613"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_interactions)",
            "value": 3523966.1197916665,
            "unit": "ns",
            "range": "± 13222.323704725331"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_no_interactions)",
            "value": 1106278.2291666667,
            "unit": "ns",
            "range": "± 6402.641184752049"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_no_namespaces)",
            "value": 937086.9140625,
            "unit": "ns",
            "range": "± 4033.5332495728076"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_interactions)",
            "value": 2914548.515625,
            "unit": "ns",
            "range": "± 10980.373448957846"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_no_interactions)",
            "value": 932634.1947115385,
            "unit": "ns",
            "range": "± 1644.226237228035"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: few_features)",
            "value": 3180.194625854492,
            "unit": "ns",
            "range": "± 14.072522116067978"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: few_features)",
            "value": 11287.346590482271,
            "unit": "ns",
            "range": "± 39.42809221362609"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: many_features)",
            "value": 109935.31168619792,
            "unit": "ns",
            "range": "± 454.9865169803702"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: many_features)",
            "value": 18994.213256835938,
            "unit": "ns",
            "range": "± 45.220087510024946"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: quadratic)",
            "value": 4250818.470982143,
            "unit": "ns",
            "range": "± 13278.35619494572"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: simple)",
            "value": 204706.14624023438,
            "unit": "ns",
            "range": "± 485.1734538763278"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "jackgerrits@users.noreply.github.com",
            "name": "Jack Gerrits",
            "username": "jackgerrits"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "96448d34d332b0a527e11697ba39110c34a8f667",
          "message": "refactor: remove unused field in sparse_iterator (#4259)",
          "timestamp": "2022-11-04T09:29:19-04:00",
          "tree_id": "a54e754614e584236b71e98768c273a52a64f359",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/96448d34d332b0a527e11697ba39110c34a8f667"
        },
        "date": 1667570059820,
        "tool": "googlecpp",
        "benches": [
          {
            "name": "bench_text/120_string_fts",
            "value": 7285.1345331739685,
            "unit": "ns/iter",
            "extra": "iterations: 593690\ncpu: 7276.728427293706 ns\nthreads: 1"
          },
          {
            "name": "bench_text/120_num_fts",
            "value": 4807.969960497025,
            "unit": "ns/iter",
            "extra": "iterations: 830573\ncpu: 4807.46460576012 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/8_features",
            "value": 85.37005178877334,
            "unit": "ns/iter",
            "extra": "iterations: 50382348\ncpu: 85.36108916559425 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/1_feature",
            "value": 195.1276475269144,
            "unit": "ns/iter",
            "extra": "iterations: 22068567\ncpu: 195.1082369779606 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features",
            "value": 8316.955270962086,
            "unit": "ns/iter",
            "extra": "iterations: 481857\ncpu: 8316.102910199495 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features",
            "value": 14524.11108401295,
            "unit": "ns/iter",
            "extra": "iterations: 270624\ncpu: 14522.86752098854 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features_no_predict",
            "value": 3768.015208849274,
            "unit": "ns/iter",
            "extra": "iterations: 1142690\ncpu: 3767.7384067419803 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features_no_predic",
            "value": 5742.953844032654,
            "unit": "ns/iter",
            "extra": "iterations: 754247\ncpu: 5739.668967858017 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/few_features",
            "value": 1478.92355812744,
            "unit": "ns/iter",
            "extra": "iterations: 2922155\ncpu: 1478.8107407033494 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/many_features/min_time:15.000",
            "value": 120895.09988996363,
            "unit": "ns/iter",
            "extra": "iterations: 263550\ncpu: 120882.4951622083 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_no_namespaces/min_time:15.000",
            "value": 335006.70021396957,
            "unit": "ns/iter",
            "extra": "iterations: 65430\ncpu: 334926.49854806665 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 325861.2294754897,
            "unit": "ns/iter",
            "extra": "iterations: 64155\ncpu: 325833.22110513615 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_interactions/min_time:15.000",
            "value": 596996.0884546125,
            "unit": "ns/iter",
            "extra": "iterations: 35001\ncpu: 596854.5298705751 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_no_interactions/min_time:15.000",
            "value": 317656.6162210802,
            "unit": "ns/iter",
            "extra": "iterations: 65569\ncpu: 317627.8073479848 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_interactions/min_time:15.000",
            "value": 488623.656167978,
            "unit": "ns/iter",
            "extra": "iterations: 43053\ncpu: 488580.96764453116 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_no_namespaces/min_time:15.000",
            "value": 743264.0245690938,
            "unit": "ns/iter",
            "extra": "iterations: 29183\ncpu: 743189.1340849126 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 892276.0126512614,
            "unit": "ns/iter",
            "extra": "iterations: 23634\ncpu: 892066.7343657457 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_interactions/min_time:15.000",
            "value": 3257786.8698987085,
            "unit": "ns/iter",
            "extra": "iterations: 6910\ncpu: 3257364.037626624 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_no_interactions/min_time:15.000",
            "value": 749548.1072661775,
            "unit": "ns/iter",
            "extra": "iterations: 26644\ncpu: 749470.5712355507 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_interactions/min_time:15.000",
            "value": 2215636.6129704933,
            "unit": "ns/iter",
            "extra": "iterations: 9591\ncpu: 2215172.161401314 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi_predict/cb_las_small_300_onestep/min_time:15.000/real_time",
            "value": 48.40622382798162,
            "unit": "ms/iter",
            "extra": "iterations: 436\ncpu: 48.401475688073376 ms\nthreads: 1"
          },
          {
            "name": "benchmark_multi_predict/cb_las_small_300_onestep_max_threads/min_time:15.000/real_time",
            "value": 25.822680717821825,
            "unit": "ms/iter",
            "extra": "iterations: 808\ncpu: 2.0815242574257 ms\nthreads: 1"
          },
          {
            "name": "benchmark_multi_predict/cb_las_small_300_plaincb/min_time:15.000/real_time",
            "value": 1.3327028040775049,
            "unit": "ms/iter",
            "extra": "iterations: 15843\ncpu: 1.332592899072144 ms\nthreads: 1"
          },
          {
            "name": "benchmark_multi_predict/cb_las_small_500_onestep/min_time:15.000/real_time",
            "value": 79.15055466542765,
            "unit": "ms/iter",
            "extra": "iterations: 269\ncpu: 79.13698215613378 ms\nthreads: 1"
          },
          {
            "name": "benchmark_multi_predict/cb_las_small_500_onestep_max_threads/min_time:15.000/real_time",
            "value": 43.63210164791696,
            "unit": "ms/iter",
            "extra": "iterations: 480\ncpu: 3.511494791666673 ms\nthreads: 1"
          },
          {
            "name": "benchmark_multi_predict/cb_las_small_500_plaincb/min_time:15.000/real_time",
            "value": 2.188899267381972,
            "unit": "ms/iter",
            "extra": "iterations: 9320\ncpu: 2.188688798283262 ms\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/simple/min_time:15.000",
            "value": 66217.29217504029,
            "unit": "ns/iter",
            "extra": "iterations: 325548\ncpu: 66206.80729109068 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/quadratic/min_time:15.000",
            "value": 2724369.0553568895,
            "unit": "ns/iter",
            "extra": "iterations: 7551\ncpu: 2724096.437557948 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_string_fts",
            "value": 1129.7494509735955,
            "unit": "ns/iter",
            "extra": "iterations: 3777596\ncpu: 1129.651026737632 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_string_fts",
            "value": 8145.670200954379,
            "unit": "ns/iter",
            "extra": "iterations: 489962\ncpu: 8145.1194174240545 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_num_fts",
            "value": 794.7675886363298,
            "unit": "ns/iter",
            "extra": "iterations: 4697877\ncpu: 794.7132289755373 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_num_fts",
            "value": 5371.842885874846,
            "unit": "ns/iter",
            "extra": "iterations: 775640\ncpu: 5371.400391934463 ns\nthreads: 1"
          },
          {
            "name": "benchmark_example_reuse",
            "value": 923.2222473129456,
            "unit": "ns/iter",
            "extra": "iterations: 4636506\ncpu: 923.1602849214441 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_char",
            "value": 51.73391247758417,
            "unit": "ns/iter",
            "extra": "iterations: 78327521\ncpu: 51.73011922591124 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_extent",
            "value": 72.903479803503,
            "unit": "ns/iter",
            "extra": "iterations: 58438702\ncpu: 72.89758934070882 ns\nthreads: 1"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "jackgerrits@users.noreply.github.com",
            "name": "Jack Gerrits",
            "username": "jackgerrits"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "96448d34d332b0a527e11697ba39110c34a8f667",
          "message": "refactor: remove unused field in sparse_iterator (#4259)",
          "timestamp": "2022-11-04T09:29:19-04:00",
          "tree_id": "a54e754614e584236b71e98768c273a52a64f359",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/96448d34d332b0a527e11697ba39110c34a8f667"
        },
        "date": 1667570617462,
        "tool": "benchmarkdotnet",
        "benches": [
          {
            "name": "BenchmarkText.Benchmark(args: 120_num_features)",
            "value": 5901.5617915562225,
            "unit": "ns",
            "range": "± 67.7882223082512"
          },
          {
            "name": "BenchmarkText.Benchmark(args: 120_string_fts)",
            "value": 9606.505643404447,
            "unit": "ns",
            "range": "± 124.47676588867533"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 1_feature)",
            "value": 697.9771041870117,
            "unit": "ns",
            "range": "± 5.367106485381342"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 8_features)",
            "value": 550.3402837117513,
            "unit": "ns",
            "range": "± 4.615722396012738"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_interactions)",
            "value": 912622.9296875,
            "unit": "ns",
            "range": "± 3116.7642269159733"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_no_interactions)",
            "value": 555019.2034040178,
            "unit": "ns",
            "range": "± 3082.4653976165023"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_no_namespaces)",
            "value": 558805.4387019231,
            "unit": "ns",
            "range": "± 1358.2918143516638"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_interactions)",
            "value": 897910.4427083334,
            "unit": "ns",
            "range": "± 4024.736576796699"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_no_interactions)",
            "value": 553738.6493389423,
            "unit": "ns",
            "range": "± 2160.7465651497264"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_interactions)",
            "value": 3548352.9854910714,
            "unit": "ns",
            "range": "± 7608.575612230923"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_no_interactions)",
            "value": 1110315.8653846155,
            "unit": "ns",
            "range": "± 2925.255721511557"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_no_namespaces)",
            "value": 939013.3440290178,
            "unit": "ns",
            "range": "± 3512.677555438208"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_interactions)",
            "value": 2894189.871651786,
            "unit": "ns",
            "range": "± 10864.294220000887"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_no_interactions)",
            "value": 942216.6276041666,
            "unit": "ns",
            "range": "± 4901.236873958492"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: few_features)",
            "value": 3228.0980682373047,
            "unit": "ns",
            "range": "± 10.342775903776651"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: few_features)",
            "value": 11104.762980143229,
            "unit": "ns",
            "range": "± 41.85994129407133"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: many_features)",
            "value": 109304.8876953125,
            "unit": "ns",
            "range": "± 483.10901803467124"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: many_features)",
            "value": 18787.46555873326,
            "unit": "ns",
            "range": "± 52.426739395995256"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: quadratic)",
            "value": 4236410.881696428,
            "unit": "ns",
            "range": "± 12867.374885756524"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: simple)",
            "value": 211788.82882254463,
            "unit": "ns",
            "range": "± 1590.3063061157702"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "jackgerrits@users.noreply.github.com",
            "name": "Jack Gerrits",
            "username": "jackgerrits"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "ce26aefba2f1a043f119bcb27df10767119a2932",
          "message": "test: add make_args for easier workspace creation in tests (#4267)\n\n* test: add make_args for easier workspace creation in tests\r\n\r\n* format",
          "timestamp": "2022-11-04T10:48:55-04:00",
          "tree_id": "fbede101e5bce678130c6c6825c517db8d3474bf",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/ce26aefba2f1a043f119bcb27df10767119a2932"
        },
        "date": 1667574594725,
        "tool": "googlecpp",
        "benches": [
          {
            "name": "bench_text/120_string_fts",
            "value": 6982.7423190024965,
            "unit": "ns/iter",
            "extra": "iterations: 601224\ncpu: 6982.25653001211 ns\nthreads: 1"
          },
          {
            "name": "bench_text/120_num_fts",
            "value": 4727.681900561951,
            "unit": "ns/iter",
            "extra": "iterations: 888895\ncpu: 4725.5093121234795 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/8_features",
            "value": 76.31366927741705,
            "unit": "ns/iter",
            "extra": "iterations: 55938714\ncpu: 76.30847573649976 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/1_feature",
            "value": 162.92349017152927,
            "unit": "ns/iter",
            "extra": "iterations: 25682073\ncpu: 162.91324302364535 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features",
            "value": 6924.66298032859,
            "unit": "ns/iter",
            "extra": "iterations: 595894\ncpu: 6924.0901905372475 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features",
            "value": 12550.580896113546,
            "unit": "ns/iter",
            "extra": "iterations: 340381\ncpu: 12549.402581225158 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features_no_predict",
            "value": 3339.3172816626707,
            "unit": "ns/iter",
            "extra": "iterations: 1250794\ncpu: 3339.0804560942865 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features_no_predic",
            "value": 5002.576052906008,
            "unit": "ns/iter",
            "extra": "iterations: 842098\ncpu: 5002.186087605011 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/few_features",
            "value": 1247.271488039677,
            "unit": "ns/iter",
            "extra": "iterations: 3381637\ncpu: 1247.1812320482643 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/many_features/min_time:15.000",
            "value": 60832.45684274225,
            "unit": "ns/iter",
            "extra": "iterations: 347010\ncpu: 60817.76058326849 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_no_namespaces/min_time:15.000",
            "value": 256208.14971206905,
            "unit": "ns/iter",
            "extra": "iterations: 83527\ncpu: 256173.31641265698 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 288511.96452767245,
            "unit": "ns/iter",
            "extra": "iterations: 73212\ncpu: 288484.21706824005 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_interactions/min_time:15.000",
            "value": 527850.2915422384,
            "unit": "ns/iter",
            "extra": "iterations: 39514\ncpu: 527797.3806752037 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_no_interactions/min_time:15.000",
            "value": 254914.969046656,
            "unit": "ns/iter",
            "extra": "iterations: 81639\ncpu: 254892.41049008426 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_interactions/min_time:15.000",
            "value": 406426.06549557624,
            "unit": "ns/iter",
            "extra": "iterations: 52202\ncpu: 406351.21068158257 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_no_namespaces/min_time:15.000",
            "value": 624733.0783941575,
            "unit": "ns/iter",
            "extra": "iterations: 34250\ncpu: 624622.0613138684 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 799264.864625179,
            "unit": "ns/iter",
            "extra": "iterations: 26386\ncpu: 799102.5733343445 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_interactions/min_time:15.000",
            "value": 2497815.4699440557,
            "unit": "ns/iter",
            "extra": "iterations: 8401\ncpu: 2497561.5640995097 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_no_interactions/min_time:15.000",
            "value": 640496.6691747416,
            "unit": "ns/iter",
            "extra": "iterations: 33226\ncpu: 640370.7608499355 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_interactions/min_time:15.000",
            "value": 1895706.372186491,
            "unit": "ns/iter",
            "extra": "iterations: 11196\ncpu: 1895524.6516613055 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi_predict/cb_las_small_300_onestep/min_time:15.000/real_time",
            "value": 41.070354695312616,
            "unit": "ms/iter",
            "extra": "iterations: 512\ncpu: 41.063186718749975 ms\nthreads: 1"
          },
          {
            "name": "benchmark_multi_predict/cb_las_small_300_onestep_max_threads/min_time:15.000/real_time",
            "value": 21.989520539748916,
            "unit": "ms/iter",
            "extra": "iterations: 956\ncpu: 2.190696234309607 ms\nthreads: 1"
          },
          {
            "name": "benchmark_multi_predict/cb_las_small_300_plaincb/min_time:15.000/real_time",
            "value": 1.2967674310039186,
            "unit": "ms/iter",
            "extra": "iterations: 15798\ncpu: 1.296536738827698 ms\nthreads: 1"
          },
          {
            "name": "benchmark_multi_predict/cb_las_small_500_onestep/min_time:15.000/real_time",
            "value": 68.30278612703536,
            "unit": "ms/iter",
            "extra": "iterations: 307\ncpu: 68.29744951140059 ms\nthreads: 1"
          },
          {
            "name": "benchmark_multi_predict/cb_las_small_500_onestep_max_threads/min_time:15.000/real_time",
            "value": 36.181185077452746,
            "unit": "ms/iter",
            "extra": "iterations: 581\ncpu: 3.4369039586919206 ms\nthreads: 1"
          },
          {
            "name": "benchmark_multi_predict/cb_las_small_500_plaincb/min_time:15.000/real_time",
            "value": 2.16315281339221,
            "unit": "ms/iter",
            "extra": "iterations: 9737\ncpu: 2.16276429084934 ms\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/simple/min_time:15.000",
            "value": 46573.21971728919,
            "unit": "ns/iter",
            "extra": "iterations: 455235\ncpu: 46569.550012630876 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/quadratic/min_time:15.000",
            "value": 2684971.5567742027,
            "unit": "ns/iter",
            "extra": "iterations: 7750\ncpu: 2684099.9354838664 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_string_fts",
            "value": 954.6394610419432,
            "unit": "ns/iter",
            "extra": "iterations: 4397522\ncpu: 954.5685501971357 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_string_fts",
            "value": 8704.41438888796,
            "unit": "ns/iter",
            "extra": "iterations: 480329\ncpu: 8703.157627376151 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_num_fts",
            "value": 754.5527183094388,
            "unit": "ns/iter",
            "extra": "iterations: 5566548\ncpu: 754.4769397479541 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_num_fts",
            "value": 5707.815665635343,
            "unit": "ns/iter",
            "extra": "iterations: 735891\ncpu: 5707.255965897018 ns\nthreads: 1"
          },
          {
            "name": "benchmark_example_reuse",
            "value": 851.1136519224801,
            "unit": "ns/iter",
            "extra": "iterations: 4900753\ncpu: 851.0222204628581 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_char",
            "value": 52.00785535432295,
            "unit": "ns/iter",
            "extra": "iterations: 80822325\ncpu: 52.00268118988089 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_extent",
            "value": 73.97851200987874,
            "unit": "ns/iter",
            "extra": "iterations: 56819181\ncpu: 73.97092013698725 ns\nthreads: 1"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "jackgerrits@users.noreply.github.com",
            "name": "Jack Gerrits",
            "username": "jackgerrits"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "ce26aefba2f1a043f119bcb27df10767119a2932",
          "message": "test: add make_args for easier workspace creation in tests (#4267)\n\n* test: add make_args for easier workspace creation in tests\r\n\r\n* format",
          "timestamp": "2022-11-04T10:48:55-04:00",
          "tree_id": "fbede101e5bce678130c6c6825c517db8d3474bf",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/ce26aefba2f1a043f119bcb27df10767119a2932"
        },
        "date": 1667575639703,
        "tool": "benchmarkdotnet",
        "benches": [
          {
            "name": "BenchmarkText.Benchmark(args: 120_num_features)",
            "value": 5137.608555385044,
            "unit": "ns",
            "range": "± 8.162166997141066"
          },
          {
            "name": "BenchmarkText.Benchmark(args: 120_string_fts)",
            "value": 7433.492279052734,
            "unit": "ns",
            "range": "± 5.463265778158466"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 1_feature)",
            "value": 670.1428731282552,
            "unit": "ns",
            "range": "± 4.96432714502773"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 8_features)",
            "value": 482.6246198018392,
            "unit": "ns",
            "range": "± 5.5507227865222735"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_interactions)",
            "value": 850411.15234375,
            "unit": "ns",
            "range": "± 2789.6196208897086"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_no_interactions)",
            "value": 455232.9571063702,
            "unit": "ns",
            "range": "± 849.2926729754419"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_no_namespaces)",
            "value": 460029.74384014425,
            "unit": "ns",
            "range": "± 1074.4166365743115"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_interactions)",
            "value": 843579.7325721154,
            "unit": "ns",
            "range": "± 3407.584228196563"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_no_interactions)",
            "value": 459552.16936383926,
            "unit": "ns",
            "range": "± 801.0041353385577"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_interactions)",
            "value": 3437609.4308035714,
            "unit": "ns",
            "range": "± 6693.551292581366"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_no_interactions)",
            "value": 1053495.591517857,
            "unit": "ns",
            "range": "± 1200.814613053841"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_no_namespaces)",
            "value": 874772.1609933035,
            "unit": "ns",
            "range": "± 1649.0772140968124"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_interactions)",
            "value": 2776702.8515625,
            "unit": "ns",
            "range": "± 5854.607098817906"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_no_interactions)",
            "value": 897060.78125,
            "unit": "ns",
            "range": "± 1299.4813255352556"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: few_features)",
            "value": 3319.507573445638,
            "unit": "ns",
            "range": "± 9.169848368143972"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: few_features)",
            "value": 10741.194534301758,
            "unit": "ns",
            "range": "± 9.00786710959309"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: many_features)",
            "value": 125986.46240234375,
            "unit": "ns",
            "range": "± 337.9481080561576"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: many_features)",
            "value": 19384.977068219865,
            "unit": "ns",
            "range": "± 171.3234939990523"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: quadratic)",
            "value": 4867673.772321428,
            "unit": "ns",
            "range": "± 12708.969572475766"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: simple)",
            "value": 194205.93424479166,
            "unit": "ns",
            "range": "± 1808.2857379787865"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "jackgerrits@users.noreply.github.com",
            "name": "Jack Gerrits",
            "username": "jackgerrits"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "bfacaf310bbb8e041e57d54066b310b92c09c953",
          "message": "ci: change caching for benchmark job (#4269)\n\n* ci: change caching for benchmark job\r\n\r\n* add comment\r\n\r\n* Update run_benchmarks.yml",
          "timestamp": "2022-11-04T12:22:00-04:00",
          "tree_id": "aeebfbfc680fec2b705ebf48f299563611ec64cc",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/bfacaf310bbb8e041e57d54066b310b92c09c953"
        },
        "date": 1667580155173,
        "tool": "googlecpp",
        "benches": [
          {
            "name": "bench_text/120_string_fts",
            "value": 5758.92323413939,
            "unit": "ns/iter",
            "extra": "iterations: 728540\ncpu: 5758.470777170779 ns\nthreads: 1"
          },
          {
            "name": "bench_text/120_num_fts",
            "value": 3935.8912007821004,
            "unit": "ns/iter",
            "extra": "iterations: 1071993\ncpu: 3935.6903449929255 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/8_features",
            "value": 68.27867658145904,
            "unit": "ns/iter",
            "extra": "iterations: 61518316\ncpu: 68.27628701669923 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/1_feature",
            "value": 158.3559105760943,
            "unit": "ns/iter",
            "extra": "iterations: 26461380\ncpu: 158.34689649594995 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features",
            "value": 6834.705951896395,
            "unit": "ns/iter",
            "extra": "iterations: 614090\ncpu: 6834.367600840268 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features",
            "value": 12215.084512819274,
            "unit": "ns/iter",
            "extra": "iterations: 341321\ncpu: 12214.555799379466 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features_no_predict",
            "value": 3301.555551627765,
            "unit": "ns/iter",
            "extra": "iterations: 1272987\ncpu: 3301.465765164925 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features_no_predic",
            "value": 5064.059340584505,
            "unit": "ns/iter",
            "extra": "iterations: 823214\ncpu: 5063.909748862382 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/few_features",
            "value": 1223.4828452876975,
            "unit": "ns/iter",
            "extra": "iterations: 3426114\ncpu: 1223.44994941791 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/many_features/min_time:15.000",
            "value": 51861.14879106184,
            "unit": "ns/iter",
            "extra": "iterations: 405273\ncpu: 51859.39527182913 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_no_namespaces/min_time:15.000",
            "value": 195722.06522699594,
            "unit": "ns/iter",
            "extra": "iterations: 108636\ncpu: 195712.99201001518 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 225382.90845844403,
            "unit": "ns/iter",
            "extra": "iterations: 93717\ncpu: 225373.49680420832 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_interactions/min_time:15.000",
            "value": 503042.1996062974,
            "unit": "ns/iter",
            "extra": "iterations: 43180\ncpu: 503019.7429365444 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_no_interactions/min_time:15.000",
            "value": 195501.35286379964,
            "unit": "ns/iter",
            "extra": "iterations: 107951\ncpu: 195493.77124806616 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_interactions/min_time:15.000",
            "value": 339279.97836892674,
            "unit": "ns/iter",
            "extra": "iterations: 61763\ncpu: 339267.184236517 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_no_namespaces/min_time:15.000",
            "value": 580391.7616455768,
            "unit": "ns/iter",
            "extra": "iterations: 36194\ncpu: 580357.3630988566 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 728096.032699042,
            "unit": "ns/iter",
            "extra": "iterations: 29114\ncpu: 728054.9769870167 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_interactions/min_time:15.000",
            "value": 2243405.106816966,
            "unit": "ns/iter",
            "extra": "iterations: 9315\ncpu: 2243298.1320450907 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_no_interactions/min_time:15.000",
            "value": 594261.5945329648,
            "unit": "ns/iter",
            "extra": "iterations: 35522\ncpu: 594147.1876583522 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_interactions/min_time:15.000",
            "value": 1786048.331636977,
            "unit": "ns/iter",
            "extra": "iterations: 11790\ncpu: 1785970.4240882136 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi_predict/cb_las_small_300_onestep/min_time:15.000/real_time",
            "value": 46.569525405764836,
            "unit": "ms/iter",
            "extra": "iterations: 451\ncpu: 46.56402949002223 ms\nthreads: 1"
          },
          {
            "name": "benchmark_multi_predict/cb_las_small_300_onestep_max_threads/min_time:15.000/real_time",
            "value": 24.714216759150062,
            "unit": "ms/iter",
            "extra": "iterations: 847\ncpu: 1.916600826446308 ms\nthreads: 1"
          },
          {
            "name": "benchmark_multi_predict/cb_las_small_300_plaincb/min_time:15.000/real_time",
            "value": 1.2149042657318334,
            "unit": "ms/iter",
            "extra": "iterations: 17258\ncpu: 1.2148563101170484 ms\nthreads: 1"
          },
          {
            "name": "benchmark_multi_predict/cb_las_small_500_onestep/min_time:15.000/real_time",
            "value": 77.84347957407404,
            "unit": "ms/iter",
            "extra": "iterations: 270\ncpu: 77.83950444444434 ms\nthreads: 1"
          },
          {
            "name": "benchmark_multi_predict/cb_las_small_500_onestep_max_threads/min_time:15.000/real_time",
            "value": 40.69586765697665,
            "unit": "ms/iter",
            "extra": "iterations: 516\ncpu: 3.114169379844964 ms\nthreads: 1"
          },
          {
            "name": "benchmark_multi_predict/cb_las_small_500_plaincb/min_time:15.000/real_time",
            "value": 2.0587383811154507,
            "unit": "ms/iter",
            "extra": "iterations: 10220\ncpu: 2.0584886497064585 ms\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/simple/min_time:15.000",
            "value": 49925.13846826937,
            "unit": "ns/iter",
            "extra": "iterations: 418298\ncpu: 49923.2317630015 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/quadratic/min_time:15.000",
            "value": 2155623.226809651,
            "unit": "ns/iter",
            "extra": "iterations: 9325\ncpu: 2154877.8552278783 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_string_fts",
            "value": 894.961279963317,
            "unit": "ns/iter",
            "extra": "iterations: 4731323\ncpu: 894.8924434032431 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_string_fts",
            "value": 6779.743879167647,
            "unit": "ns/iter",
            "extra": "iterations: 617318\ncpu: 6779.546522213871 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_num_fts",
            "value": 752.2195723954949,
            "unit": "ns/iter",
            "extra": "iterations: 6078888\ncpu: 752.1944803062719 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_num_fts",
            "value": 4535.278949228729,
            "unit": "ns/iter",
            "extra": "iterations: 916622\ncpu: 4535.122765982125 ns\nthreads: 1"
          },
          {
            "name": "benchmark_example_reuse",
            "value": 755.0478151451722,
            "unit": "ns/iter",
            "extra": "iterations: 5560644\ncpu: 755.0226376657167 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_char",
            "value": 40.040827190629976,
            "unit": "ns/iter",
            "extra": "iterations: 104806893\ncpu: 40.03927966837068 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_extent",
            "value": 56.27390794501408,
            "unit": "ns/iter",
            "extra": "iterations: 78460999\ncpu: 56.272021211455865 ns\nthreads: 1"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "jackgerrits@users.noreply.github.com",
            "name": "Jack Gerrits",
            "username": "jackgerrits"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "bfacaf310bbb8e041e57d54066b310b92c09c953",
          "message": "ci: change caching for benchmark job (#4269)\n\n* ci: change caching for benchmark job\r\n\r\n* add comment\r\n\r\n* Update run_benchmarks.yml",
          "timestamp": "2022-11-04T12:22:00-04:00",
          "tree_id": "aeebfbfc680fec2b705ebf48f299563611ec64cc",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/bfacaf310bbb8e041e57d54066b310b92c09c953"
        },
        "date": 1667581352908,
        "tool": "benchmarkdotnet",
        "benches": [
          {
            "name": "BenchmarkText.Benchmark(args: 120_num_features)",
            "value": 5207.586847941081,
            "unit": "ns",
            "range": "± 66.59513970210074"
          },
          {
            "name": "BenchmarkText.Benchmark(args: 120_string_fts)",
            "value": 7476.3157145182295,
            "unit": "ns",
            "range": "± 60.51321537186087"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 1_feature)",
            "value": 670.6856659480503,
            "unit": "ns",
            "range": "± 1.540921522052318"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 8_features)",
            "value": 483.58689035688127,
            "unit": "ns",
            "range": "± 4.3380889333380965"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_interactions)",
            "value": 847774.1341145834,
            "unit": "ns",
            "range": "± 2985.7215455615137"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_no_interactions)",
            "value": 461081.5410907452,
            "unit": "ns",
            "range": "± 849.2557110773108"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_no_namespaces)",
            "value": 459721.88546316966,
            "unit": "ns",
            "range": "± 1547.3699152541099"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_interactions)",
            "value": 842020.1627604166,
            "unit": "ns",
            "range": "± 1370.041111192368"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_no_interactions)",
            "value": 458068.154296875,
            "unit": "ns",
            "range": "± 2288.1334853464323"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_interactions)",
            "value": 3429310.7291666665,
            "unit": "ns",
            "range": "± 7246.3671776282035"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_no_interactions)",
            "value": 1051188.3713942308,
            "unit": "ns",
            "range": "± 1415.47489589335"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_no_namespaces)",
            "value": 872555.1897321428,
            "unit": "ns",
            "range": "± 1100.211132472815"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_interactions)",
            "value": 2797615.234375,
            "unit": "ns",
            "range": "± 4830.801003579183"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_no_interactions)",
            "value": 902335.7682291666,
            "unit": "ns",
            "range": "± 2080.9492912583382"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: few_features)",
            "value": 3295.863596598307,
            "unit": "ns",
            "range": "± 4.214876302626564"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: few_features)",
            "value": 10797.191401890346,
            "unit": "ns",
            "range": "± 15.303398719282697"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: many_features)",
            "value": 125533.48039899554,
            "unit": "ns",
            "range": "± 116.69337777594677"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: many_features)",
            "value": 19428.57382638114,
            "unit": "ns",
            "range": "± 15.579005801322783"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: quadratic)",
            "value": 4874931.458333333,
            "unit": "ns",
            "range": "± 17467.064055481245"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: simple)",
            "value": 196159.23177083334,
            "unit": "ns",
            "range": "± 1883.5717540714215"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "jackgerrits@users.noreply.github.com",
            "name": "Jack Gerrits",
            "username": "jackgerrits"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "54895842a03a7ae178dd9f1d6b5ca98656560190",
          "message": "build: resolve cmake version check TODO in DetectCXXStandard.cmake (#4268)",
          "timestamp": "2022-11-04T13:28:19-04:00",
          "tree_id": "9be165bebf99484ccdde39592428c233fc591617",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/54895842a03a7ae178dd9f1d6b5ca98656560190"
        },
        "date": 1667583891528,
        "tool": "googlecpp",
        "benches": [
          {
            "name": "bench_text/120_string_fts",
            "value": 7702.0008569402025,
            "unit": "ns/iter",
            "extra": "iterations: 548463\ncpu: 7699.842651190692 ns\nthreads: 1"
          },
          {
            "name": "bench_text/120_num_fts",
            "value": 5111.4048591846795,
            "unit": "ns/iter",
            "extra": "iterations: 823348\ncpu: 5103.719933733973 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/8_features",
            "value": 87.35709285878657,
            "unit": "ns/iter",
            "extra": "iterations: 48968753\ncpu: 87.3381582741141 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/1_feature",
            "value": 197.91222459157652,
            "unit": "ns/iter",
            "extra": "iterations: 21318021\ncpu: 197.88437679088494 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features",
            "value": 8441.920831362882,
            "unit": "ns/iter",
            "extra": "iterations: 499036\ncpu: 8440.430149327905 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features",
            "value": 15226.968194139277,
            "unit": "ns/iter",
            "extra": "iterations: 273000\ncpu: 15224.206593406583 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features_no_predict",
            "value": 4226.161076076414,
            "unit": "ns/iter",
            "extra": "iterations: 1013757\ncpu: 4225.017731073617 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features_no_predic",
            "value": 6181.14943864466,
            "unit": "ns/iter",
            "extra": "iterations: 683257\ncpu: 6176.460980275358 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/few_features",
            "value": 1645.5669925316145,
            "unit": "ns/iter",
            "extra": "iterations: 2567219\ncpu: 1645.3797280247613 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/many_features/min_time:15.000",
            "value": 90572.96463004504,
            "unit": "ns/iter",
            "extra": "iterations: 232231\ncpu: 90550.67971114969 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_no_namespaces/min_time:15.000",
            "value": 315111.7642974916,
            "unit": "ns/iter",
            "extra": "iterations: 69138\ncpu: 315074.31369145756 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 332532.591949261,
            "unit": "ns/iter",
            "extra": "iterations: 62752\ncpu: 332408.8682432433 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_interactions/min_time:15.000",
            "value": 632668.1136109377,
            "unit": "ns/iter",
            "extra": "iterations: 33069\ncpu: 632582.9326559615 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_no_interactions/min_time:15.000",
            "value": 311362.60945303325,
            "unit": "ns/iter",
            "extra": "iterations: 66878\ncpu: 311326.89524208294 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_interactions/min_time:15.000",
            "value": 515275.9917585093,
            "unit": "ns/iter",
            "extra": "iterations: 41012\ncpu: 515141.2952306642 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_no_namespaces/min_time:15.000",
            "value": 758158.4937777617,
            "unit": "ns/iter",
            "extra": "iterations: 27241\ncpu: 758073.1838038255 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 950008.3637145294,
            "unit": "ns/iter",
            "extra": "iterations: 22097\ncpu: 949782.5994478887 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_interactions/min_time:15.000",
            "value": 3267493.3643192537,
            "unit": "ns/iter",
            "extra": "iterations: 6390\ncpu: 3267128.5289514824 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_no_interactions/min_time:15.000",
            "value": 796689.6753372612,
            "unit": "ns/iter",
            "extra": "iterations: 26834\ncpu: 796493.124394424 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_interactions/min_time:15.000",
            "value": 2513271.62602114,
            "unit": "ns/iter",
            "extra": "iterations: 8324\ncpu: 2512994.581931765 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi_predict/cb_las_small_300_onestep/min_time:15.000/real_time",
            "value": 51.528369651960894,
            "unit": "ms/iter",
            "extra": "iterations: 408\ncpu: 51.517225490196054 ms\nthreads: 1"
          },
          {
            "name": "benchmark_multi_predict/cb_las_small_300_onestep_max_threads/min_time:15.000/real_time",
            "value": 27.490063421875096,
            "unit": "ms/iter",
            "extra": "iterations: 768\ncpu: 2.7464454427083376 ms\nthreads: 1"
          },
          {
            "name": "benchmark_multi_predict/cb_las_small_300_plaincb/min_time:15.000/real_time",
            "value": 1.5154180228484972,
            "unit": "ms/iter",
            "extra": "iterations: 13874\ncpu: 1.51523226899236 ms\nthreads: 1"
          },
          {
            "name": "benchmark_multi_predict/cb_las_small_500_onestep/min_time:15.000/real_time",
            "value": 85.65663119105693,
            "unit": "ms/iter",
            "extra": "iterations: 246\ncpu: 85.64651138211374 ms\nthreads: 1"
          },
          {
            "name": "benchmark_multi_predict/cb_las_small_500_onestep_max_threads/min_time:15.000/real_time",
            "value": 45.33506983585312,
            "unit": "ms/iter",
            "extra": "iterations: 463\ncpu: 4.417925701943904 ms\nthreads: 1"
          },
          {
            "name": "benchmark_multi_predict/cb_las_small_500_plaincb/min_time:15.000/real_time",
            "value": 2.5973472464961853,
            "unit": "ms/iter",
            "extra": "iterations: 8134\ncpu: 2.596577858372266 ms\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/simple/min_time:15.000",
            "value": 76700.41888122032,
            "unit": "ns/iter",
            "extra": "iterations: 268489\ncpu: 76691.81716941835 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/quadratic/min_time:15.000",
            "value": 3157142.277561407,
            "unit": "ns/iter",
            "extra": "iterations: 6676\ncpu: 3156295.101857399 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_string_fts",
            "value": 1156.8796261169673,
            "unit": "ns/iter",
            "extra": "iterations: 3632366\ncpu: 1156.7663060385328 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_string_fts",
            "value": 9185.009891967156,
            "unit": "ns/iter",
            "extra": "iterations: 460879\ncpu: 9183.758860785483 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_num_fts",
            "value": 896.238023724824,
            "unit": "ns/iter",
            "extra": "iterations: 4630509\ncpu: 896.1153730615955 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_num_fts",
            "value": 5887.014747026857,
            "unit": "ns/iter",
            "extra": "iterations: 713771\ncpu: 5886.332311063246 ns\nthreads: 1"
          },
          {
            "name": "benchmark_example_reuse",
            "value": 1018.9009319756373,
            "unit": "ns/iter",
            "extra": "iterations: 4099678\ncpu: 1018.7686935412905 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_char",
            "value": 54.1403870731546,
            "unit": "ns/iter",
            "extra": "iterations: 77931057\ncpu: 54.13356449149623 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_extent",
            "value": 79.42866171012878,
            "unit": "ns/iter",
            "extra": "iterations: 52181199\ncpu: 79.4186235544328 ns\nthreads: 1"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "griffinbassman@gmail.com",
            "name": "Griffin Bassman",
            "username": "bassmang"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "2b1874f5d9947f44c11993d5712adce44c3cfd85",
          "message": "fix!: save/load entire tag in flat_example + bump version to 9.6 (#4266)\n\n* fix\\!: save/load entire tag in flat_example\r\n\r\n* clang\r\n\r\n* update flat_exampl tag to v_array\r\n\r\n* remove unused\r\n\r\n* bump version\r\n\r\n* clang\r\n\r\nCo-authored-by: Jack Gerrits <jackgerrits@users.noreply.github.com>",
          "timestamp": "2022-11-04T15:01:37-04:00",
          "tree_id": "aa874f5be2a86073d0f52e43d6ccb122034777d6",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/2b1874f5d9947f44c11993d5712adce44c3cfd85"
        },
        "date": 1667589734061,
        "tool": "googlecpp",
        "benches": [
          {
            "name": "bench_text/120_string_fts",
            "value": 7028.533331548358,
            "unit": "ns/iter",
            "extra": "iterations: 597557\ncpu: 7023.9163795253 ns\nthreads: 1"
          },
          {
            "name": "bench_text/120_num_fts",
            "value": 4688.666365862517,
            "unit": "ns/iter",
            "extra": "iterations: 897594\ncpu: 4688.355648544887 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/8_features",
            "value": 78.27874805575165,
            "unit": "ns/iter",
            "extra": "iterations: 53612643\ncpu: 78.27299989668481 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/1_feature",
            "value": 163.46181806993292,
            "unit": "ns/iter",
            "extra": "iterations: 25383303\ncpu: 163.45070221948654 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features",
            "value": 6965.666482448468,
            "unit": "ns/iter",
            "extra": "iterations: 598928\ncpu: 6965.037533726924 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features",
            "value": 11980.14671088951,
            "unit": "ns/iter",
            "extra": "iterations: 336778\ncpu: 11979.104929656929 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features_no_predict",
            "value": 3363.822306617668,
            "unit": "ns/iter",
            "extra": "iterations: 1245460\ncpu: 3363.5703274292187 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features_no_predic",
            "value": 4993.176812449199,
            "unit": "ns/iter",
            "extra": "iterations: 840727\ncpu: 4992.82466246475 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/few_features",
            "value": 1231.3981947125849,
            "unit": "ns/iter",
            "extra": "iterations: 3409651\ncpu: 1231.3066938522456 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/many_features/min_time:15.000",
            "value": 60622.34154127664,
            "unit": "ns/iter",
            "extra": "iterations: 346453\ncpu: 60617.57669871527 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_no_namespaces/min_time:15.000",
            "value": 242804.22759174492,
            "unit": "ns/iter",
            "extra": "iterations: 90904\ncpu: 242758.32966646127 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 274226.8004593486,
            "unit": "ns/iter",
            "extra": "iterations: 76195\ncpu: 274200.8281383293 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_interactions/min_time:15.000",
            "value": 519689.0761975299,
            "unit": "ns/iter",
            "extra": "iterations: 40500\ncpu: 519642.1925925924 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_no_interactions/min_time:15.000",
            "value": 246235.99419001295,
            "unit": "ns/iter",
            "extra": "iterations: 85026\ncpu: 246213.91927175235 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_interactions/min_time:15.000",
            "value": 396372.95893340505,
            "unit": "ns/iter",
            "extra": "iterations: 52841\ncpu: 396335.06557408074 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_no_namespaces/min_time:15.000",
            "value": 590969.155690292,
            "unit": "ns/iter",
            "extra": "iterations: 35956\ncpu: 590861.7337857381 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 761349.9190885624,
            "unit": "ns/iter",
            "extra": "iterations: 27561\ncpu: 761278.2409927084 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_interactions/min_time:15.000",
            "value": 2491202.9343039677,
            "unit": "ns/iter",
            "extra": "iterations: 8448\ncpu: 2490737.92613636 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_no_interactions/min_time:15.000",
            "value": 601915.6811316974,
            "unit": "ns/iter",
            "extra": "iterations: 35027\ncpu: 601858.3035943694 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_interactions/min_time:15.000",
            "value": 1869025.506116615,
            "unit": "ns/iter",
            "extra": "iterations: 11199\ncpu: 1868670.86346995 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi_predict/cb_las_small_300_onestep/min_time:15.000/real_time",
            "value": 41.034525714843895,
            "unit": "ms/iter",
            "extra": "iterations: 512\ncpu: 41.03069179687502 ms\nthreads: 1"
          },
          {
            "name": "benchmark_multi_predict/cb_las_small_300_onestep_max_threads/min_time:15.000/real_time",
            "value": 22.817791772082828,
            "unit": "ms/iter",
            "extra": "iterations: 917\ncpu: 2.070347982551813 ms\nthreads: 1"
          },
          {
            "name": "benchmark_multi_predict/cb_las_small_300_plaincb/min_time:15.000/real_time",
            "value": 1.2855148452718637,
            "unit": "ms/iter",
            "extra": "iterations: 16222\ncpu: 1.2853934718283833 ms\nthreads: 1"
          },
          {
            "name": "benchmark_multi_predict/cb_las_small_500_onestep/min_time:15.000/real_time",
            "value": 68.43283835620862,
            "unit": "ms/iter",
            "extra": "iterations: 306\ncpu: 68.425930392157 ms\nthreads: 1"
          },
          {
            "name": "benchmark_multi_predict/cb_las_small_500_onestep_max_threads/min_time:15.000/real_time",
            "value": 37.602135189530884,
            "unit": "ms/iter",
            "extra": "iterations: 554\ncpu: 3.3368747292418477 ms\nthreads: 1"
          },
          {
            "name": "benchmark_multi_predict/cb_las_small_500_plaincb/min_time:15.000/real_time",
            "value": 2.1614742722142464,
            "unit": "ms/iter",
            "extra": "iterations: 9746\ncpu: 2.161287533347018 ms\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/simple/min_time:15.000",
            "value": 47371.069159940664,
            "unit": "ns/iter",
            "extra": "iterations: 450492\ncpu: 47363.4011258802 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/quadratic/min_time:15.000",
            "value": 2570961.6749129663,
            "unit": "ns/iter",
            "extra": "iterations: 8044\ncpu: 2570728.2073595235 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_string_fts",
            "value": 949.367076673467,
            "unit": "ns/iter",
            "extra": "iterations: 4419845\ncpu: 949.2975658648669 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_string_fts",
            "value": 7988.525234577571,
            "unit": "ns/iter",
            "extra": "iterations: 525093\ncpu: 7987.946516141066 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_num_fts",
            "value": 723.3992418708132,
            "unit": "ns/iter",
            "extra": "iterations: 5805343\ncpu: 723.3493524844238 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_num_fts",
            "value": 5236.786667066027,
            "unit": "ns/iter",
            "extra": "iterations: 801714\ncpu: 5236.3806793943895 ns\nthreads: 1"
          },
          {
            "name": "benchmark_example_reuse",
            "value": 860.464115515435,
            "unit": "ns/iter",
            "extra": "iterations: 4869514\ncpu: 860.0559111237746 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_char",
            "value": 51.771016138186425,
            "unit": "ns/iter",
            "extra": "iterations: 81078264\ncpu: 51.7672147001075 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_extent",
            "value": 73.40801249643859,
            "unit": "ns/iter",
            "extra": "iterations: 56821941\ncpu: 73.4027547563028 ns\nthreads: 1"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "griffinbassman@gmail.com",
            "name": "Griffin Bassman",
            "username": "bassmang"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "2b1874f5d9947f44c11993d5712adce44c3cfd85",
          "message": "fix!: save/load entire tag in flat_example + bump version to 9.6 (#4266)\n\n* fix\\!: save/load entire tag in flat_example\r\n\r\n* clang\r\n\r\n* update flat_exampl tag to v_array\r\n\r\n* remove unused\r\n\r\n* bump version\r\n\r\n* clang\r\n\r\nCo-authored-by: Jack Gerrits <jackgerrits@users.noreply.github.com>",
          "timestamp": "2022-11-04T15:01:37-04:00",
          "tree_id": "aa874f5be2a86073d0f52e43d6ccb122034777d6",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/2b1874f5d9947f44c11993d5712adce44c3cfd85"
        },
        "date": 1667590547420,
        "tool": "benchmarkdotnet",
        "benches": [
          {
            "name": "BenchmarkText.Benchmark(args: 120_num_features)",
            "value": 5098.7155033991885,
            "unit": "ns",
            "range": "± 16.842649850992323"
          },
          {
            "name": "BenchmarkText.Benchmark(args: 120_string_fts)",
            "value": 7402.64641898019,
            "unit": "ns",
            "range": "± 27.934557516229546"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 1_feature)",
            "value": 671.5964698791504,
            "unit": "ns",
            "range": "± 2.01212329706004"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 8_features)",
            "value": 482.70793642316545,
            "unit": "ns",
            "range": "± 2.7530203148476375"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_interactions)",
            "value": 849496.7075892857,
            "unit": "ns",
            "range": "± 855.1150187011614"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_no_interactions)",
            "value": 459328.0831473214,
            "unit": "ns",
            "range": "± 1224.404206396195"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_no_namespaces)",
            "value": 456166.1686197917,
            "unit": "ns",
            "range": "± 590.2220440230757"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_interactions)",
            "value": 846327.4489182692,
            "unit": "ns",
            "range": "± 787.9247382452132"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_no_interactions)",
            "value": 457432.64911358175,
            "unit": "ns",
            "range": "± 658.740509697788"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_interactions)",
            "value": 3454278.3984375,
            "unit": "ns",
            "range": "± 3979.8468727509453"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_no_interactions)",
            "value": 1065074.9088541667,
            "unit": "ns",
            "range": "± 977.3593285587283"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_no_namespaces)",
            "value": 873619.1861979166,
            "unit": "ns",
            "range": "± 1151.2123037519277"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_interactions)",
            "value": 2793876.0416666665,
            "unit": "ns",
            "range": "± 4025.948575114517"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_no_interactions)",
            "value": 907654.7591145834,
            "unit": "ns",
            "range": "± 2221.5971277912895"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: few_features)",
            "value": 3284.6586100260415,
            "unit": "ns",
            "range": "± 6.978283265705603"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: few_features)",
            "value": 10873.199768066406,
            "unit": "ns",
            "range": "± 9.045189104087395"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: many_features)",
            "value": 125727.92643229167,
            "unit": "ns",
            "range": "± 206.25186561522366"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: many_features)",
            "value": 19571.943781926082,
            "unit": "ns",
            "range": "± 11.92922979162716"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: quadratic)",
            "value": 4822431.473214285,
            "unit": "ns",
            "range": "± 6459.330475985524"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: simple)",
            "value": 195107.48860677084,
            "unit": "ns",
            "range": "± 2345.425854064458"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "jackgerrits@users.noreply.github.com",
            "name": "Jack Gerrits",
            "username": "jackgerrits"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "963fa67966f068fd5c412937d2e98ef07606e5c7",
          "message": "test: apply make_args across test projects (#4272)",
          "timestamp": "2022-11-04T17:42:03-04:00",
          "tree_id": "a93831864054ea676f2013ba9f9814e70ad0e2ed",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/963fa67966f068fd5c412937d2e98ef07606e5c7"
        },
        "date": 1667599091597,
        "tool": "googlecpp",
        "benches": [
          {
            "name": "bench_text/120_string_fts",
            "value": 6962.925750944667,
            "unit": "ns/iter",
            "extra": "iterations: 609166\ncpu: 6961.40838457826 ns\nthreads: 1"
          },
          {
            "name": "bench_text/120_num_fts",
            "value": 4673.940396243984,
            "unit": "ns/iter",
            "extra": "iterations: 897326\ncpu: 4673.039341331913 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/8_features",
            "value": 81.72217831702342,
            "unit": "ns/iter",
            "extra": "iterations: 50849458\ncpu: 81.71349633657846 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/1_feature",
            "value": 186.72747975926552,
            "unit": "ns/iter",
            "extra": "iterations: 22972734\ncpu: 186.7056746489121 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features",
            "value": 7940.357460752524,
            "unit": "ns/iter",
            "extra": "iterations: 548698\ncpu: 7938.710912013528 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features",
            "value": 13778.319398719683,
            "unit": "ns/iter",
            "extra": "iterations: 294638\ncpu: 13768.880796095555 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features_no_predict",
            "value": 3875.530845911158,
            "unit": "ns/iter",
            "extra": "iterations: 1097763\ncpu: 3875.005260698349 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features_no_predic",
            "value": 5753.857175298245,
            "unit": "ns/iter",
            "extra": "iterations: 744206\ncpu: 5753.170224373352 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/few_features",
            "value": 1516.5957125091813,
            "unit": "ns/iter",
            "extra": "iterations: 2759796\ncpu: 1516.3973713999146 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/many_features/min_time:15.000",
            "value": 80344.39277462564,
            "unit": "ns/iter",
            "extra": "iterations: 264014\ncpu: 80328.46364207959 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_no_namespaces/min_time:15.000",
            "value": 307597.1699985938,
            "unit": "ns/iter",
            "extra": "iterations: 71130\ncpu: 307517.60860396456 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 323009.3019694673,
            "unit": "ns/iter",
            "extra": "iterations: 64129\ncpu: 322969.64867688576 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_interactions/min_time:15.000",
            "value": 592105.0481309202,
            "unit": "ns/iter",
            "extra": "iterations: 35258\ncpu: 591960.2473197571 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_no_interactions/min_time:15.000",
            "value": 307929.11317171383,
            "unit": "ns/iter",
            "extra": "iterations: 69505\ncpu: 307890.0208618083 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_interactions/min_time:15.000",
            "value": 482387.93541171745,
            "unit": "ns/iter",
            "extra": "iterations: 43367\ncpu: 482321.0044503889 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_no_namespaces/min_time:15.000",
            "value": 703840.2233744302,
            "unit": "ns/iter",
            "extra": "iterations: 29605\ncpu: 703740.1249788888 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 860628.0441206953,
            "unit": "ns/iter",
            "extra": "iterations: 24093\ncpu: 860510.666998713 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_interactions/min_time:15.000",
            "value": 3046919.1157678794,
            "unit": "ns/iter",
            "extra": "iterations: 6824\ncpu: 3046507.6641266085 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_no_interactions/min_time:15.000",
            "value": 716363.1907185427,
            "unit": "ns/iter",
            "extra": "iterations: 29866\ncpu: 716256.080492869 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_interactions/min_time:15.000",
            "value": 2405527.594021434,
            "unit": "ns/iter",
            "extra": "iterations: 8865\ncpu: 2404876.0744500835 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi_predict/cb_las_small_300_onestep/min_time:15.000/real_time",
            "value": 48.806113549425255,
            "unit": "ms/iter",
            "extra": "iterations: 435\ncpu: 48.79985011494256 ms\nthreads: 1"
          },
          {
            "name": "benchmark_multi_predict/cb_las_small_300_onestep_max_threads/min_time:15.000/real_time",
            "value": 26.918226748704626,
            "unit": "ms/iter",
            "extra": "iterations: 772\ncpu: 2.24556230569948 ms\nthreads: 1"
          },
          {
            "name": "benchmark_multi_predict/cb_las_small_300_plaincb/min_time:15.000/real_time",
            "value": 1.411456714753436,
            "unit": "ms/iter",
            "extra": "iterations: 14966\ncpu: 1.4112793598824005 ms\nthreads: 1"
          },
          {
            "name": "benchmark_multi_predict/cb_las_small_500_onestep/min_time:15.000/real_time",
            "value": 80.1629917325583,
            "unit": "ms/iter",
            "extra": "iterations: 258\ncpu: 80.14494224806207 ms\nthreads: 1"
          },
          {
            "name": "benchmark_multi_predict/cb_las_small_500_onestep_max_threads/min_time:15.000/real_time",
            "value": 41.757861237903214,
            "unit": "ms/iter",
            "extra": "iterations: 496\ncpu: 3.4162657258064195 ms\nthreads: 1"
          },
          {
            "name": "benchmark_multi_predict/cb_las_small_500_plaincb/min_time:15.000/real_time",
            "value": 2.2718825123712367,
            "unit": "ms/iter",
            "extra": "iterations: 9417\ncpu: 2.271361049166402 ms\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/simple/min_time:15.000",
            "value": 69148.62127986173,
            "unit": "ns/iter",
            "extra": "iterations: 312690\ncpu: 69139.60248169114 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/quadratic/min_time:15.000",
            "value": 2951710.507475821,
            "unit": "ns/iter",
            "extra": "iterations: 6822\ncpu: 2951373.4242157745 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_string_fts",
            "value": 1050.4010356504164,
            "unit": "ns/iter",
            "extra": "iterations: 3842030\ncpu: 1049.7068216541695 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_string_fts",
            "value": 7988.30600186495,
            "unit": "ns/iter",
            "extra": "iterations: 513657\ncpu: 7987.35810083397 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_num_fts",
            "value": 810.3956414040929,
            "unit": "ns/iter",
            "extra": "iterations: 5182357\ncpu: 810.3185288084148 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_num_fts",
            "value": 5128.665952424395,
            "unit": "ns/iter",
            "extra": "iterations: 821850\ncpu: 5128.186043681947 ns\nthreads: 1"
          },
          {
            "name": "benchmark_example_reuse",
            "value": 877.414653582333,
            "unit": "ns/iter",
            "extra": "iterations: 4777412\ncpu: 877.1337284705693 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_char",
            "value": 53.79200569243047,
            "unit": "ns/iter",
            "extra": "iterations: 79521734\ncpu: 53.785464486980075 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_extent",
            "value": 72.62002094190336,
            "unit": "ns/iter",
            "extra": "iterations: 57384473\ncpu: 72.61242601287083 ns\nthreads: 1"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "jackgerrits@users.noreply.github.com",
            "name": "Jack Gerrits",
            "username": "jackgerrits"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "963fa67966f068fd5c412937d2e98ef07606e5c7",
          "message": "test: apply make_args across test projects (#4272)",
          "timestamp": "2022-11-04T17:42:03-04:00",
          "tree_id": "a93831864054ea676f2013ba9f9814e70ad0e2ed",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/963fa67966f068fd5c412937d2e98ef07606e5c7"
        },
        "date": 1667600843029,
        "tool": "benchmarkdotnet",
        "benches": [
          {
            "name": "BenchmarkText.Benchmark(args: 120_num_features)",
            "value": 7517.790291526101,
            "unit": "ns",
            "range": "± 156.96409485918457"
          },
          {
            "name": "BenchmarkText.Benchmark(args: 120_string_fts)",
            "value": 12121.1782282049,
            "unit": "ns",
            "range": "± 289.2142993859672"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 1_feature)",
            "value": 826.5718207639807,
            "unit": "ns",
            "range": "± 25.189183953047085"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 8_features)",
            "value": 644.9718392413596,
            "unit": "ns",
            "range": "± 24.729190315085745"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_interactions)",
            "value": 1126886.6373697917,
            "unit": "ns",
            "range": "± 43549.91881853773"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_no_interactions)",
            "value": 676736.8208451704,
            "unit": "ns",
            "range": "± 16349.841275977986"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_no_namespaces)",
            "value": 691716.5985107422,
            "unit": "ns",
            "range": "± 21365.017537399966"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_interactions)",
            "value": 1082984.0983072917,
            "unit": "ns",
            "range": "± 27576.312956109265"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_no_interactions)",
            "value": 662820.2448918269,
            "unit": "ns",
            "range": "± 18013.126778678197"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_interactions)",
            "value": 4467349.371936275,
            "unit": "ns",
            "range": "± 181892.56175595536"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_no_interactions)",
            "value": 1387059.6368018617,
            "unit": "ns",
            "range": "± 53939.22168304957"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_no_namespaces)",
            "value": 1165424.4395380435,
            "unit": "ns",
            "range": "± 44710.96243363675"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_interactions)",
            "value": 3654079.296875,
            "unit": "ns",
            "range": "± 95401.66353105535"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_no_interactions)",
            "value": 1178882.1314102565,
            "unit": "ns",
            "range": "± 40905.16509794134"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: few_features)",
            "value": 4000.195339747838,
            "unit": "ns",
            "range": "± 113.59914377655457"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: few_features)",
            "value": 13831.023322211371,
            "unit": "ns",
            "range": "± 386.48016619480165"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: many_features)",
            "value": 134603.54517886514,
            "unit": "ns",
            "range": "± 2930.5872783520954"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: many_features)",
            "value": 23182.4228515625,
            "unit": "ns",
            "range": "± 599.5817568984999"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: quadratic)",
            "value": 5372277.08984375,
            "unit": "ns",
            "range": "± 187248.40759650298"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: simple)",
            "value": 262331.04553222656,
            "unit": "ns",
            "range": "± 11879.819766347617"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "rucker.mark@gmail.com",
            "name": "Mark Rucker",
            "username": "mrucker"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "e3685a0e59a892464fd46ff8670890c7f9a77a6d",
          "message": "fix: This patches a bug with flat_example collision cleanup (#4265)\n\n* Fixed a bug with flat example collision cleanup. Added two unit tests for the old bug.\r\n\r\n* Fixed a documentation typo.\r\n\r\n* Update vowpalwabbit/core/tests/flat_example_test.cc\r\n\r\nCo-authored-by: Jack Gerrits <jackgerrits@users.noreply.github.com>\r\n\r\n* Update vowpalwabbit/core/tests/flat_example_test.cc\r\n\r\nCo-authored-by: Jack Gerrits <jackgerrits@users.noreply.github.com>\r\n\r\n* fix test output\r\n\r\n* finish example\r\n\r\n* free the flat example\r\n\r\n* fix tests\r\n\r\n* Use make_args\r\n\r\n* test diff\r\n\r\n* update test\r\n\r\nCo-authored-by: Jack Gerrits <jackgerrits@users.noreply.github.com>\r\nCo-authored-by: Jack Gerrits <jackgerrits95@gmail.com>",
          "timestamp": "2022-11-07T14:18:24-05:00",
          "tree_id": "058ca0cc7cc3c460117c3290d26dbc100922a812",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/e3685a0e59a892464fd46ff8670890c7f9a77a6d"
        },
        "date": 1667849701392,
        "tool": "googlecpp",
        "benches": [
          {
            "name": "bench_text/120_string_fts",
            "value": 6895.2228210553285,
            "unit": "ns/iter",
            "extra": "iterations: 616755\ncpu: 6885.546448751937 ns\nthreads: 1"
          },
          {
            "name": "bench_text/120_num_fts",
            "value": 4961.130453525458,
            "unit": "ns/iter",
            "extra": "iterations: 849015\ncpu: 4958.394845791888 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/8_features",
            "value": 89.94145913010243,
            "unit": "ns/iter",
            "extra": "iterations: 48321284\ncpu: 89.93108295714991 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/1_feature",
            "value": 197.12172744674294,
            "unit": "ns/iter",
            "extra": "iterations: 20954510\ncpu: 197.09418163440714 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features",
            "value": 8514.857925928058,
            "unit": "ns/iter",
            "extra": "iterations: 494573\ncpu: 8514.180515313205 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features",
            "value": 15220.250932221974,
            "unit": "ns/iter",
            "extra": "iterations: 268981\ncpu: 15218.81136585855 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features_no_predict",
            "value": 4105.855973533022,
            "unit": "ns/iter",
            "extra": "iterations: 1032831\ncpu: 4105.555216681143 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features_no_predic",
            "value": 6091.306079982996,
            "unit": "ns/iter",
            "extra": "iterations: 704525\ncpu: 6090.881657854592 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/few_features",
            "value": 1588.0259588615866,
            "unit": "ns/iter",
            "extra": "iterations: 2681435\ncpu: 1587.9178126637423 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/many_features/min_time:15.000",
            "value": 84028.76750868425,
            "unit": "ns/iter",
            "extra": "iterations: 249876\ncpu: 84011.34082504922 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_no_namespaces/min_time:15.000",
            "value": 331669.60096366465,
            "unit": "ns/iter",
            "extra": "iterations: 65583\ncpu: 331637.7658844518 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 342041.4903097604,
            "unit": "ns/iter",
            "extra": "iterations: 61402\ncpu: 342015.83498908824 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_interactions/min_time:15.000",
            "value": 633031.4600392831,
            "unit": "ns/iter",
            "extra": "iterations: 33095\ncpu: 632978.4801329507 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_no_interactions/min_time:15.000",
            "value": 325125.6937676939,
            "unit": "ns/iter",
            "extra": "iterations: 64647\ncpu: 325098.9728835059 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_interactions/min_time:15.000",
            "value": 513594.8194474968,
            "unit": "ns/iter",
            "extra": "iterations: 40941\ncpu: 513556.3444957373 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_no_namespaces/min_time:15.000",
            "value": 786484.3373313319,
            "unit": "ns/iter",
            "extra": "iterations: 26680\ncpu: 786375.8058470772 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 978647.4567268455,
            "unit": "ns/iter",
            "extra": "iterations: 22046\ncpu: 978560.2331488711 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_interactions/min_time:15.000",
            "value": 3058354.1558823413,
            "unit": "ns/iter",
            "extra": "iterations: 6800\ncpu: 3058116.1911764727 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_no_interactions/min_time:15.000",
            "value": 785150.7973573222,
            "unit": "ns/iter",
            "extra": "iterations: 26791\ncpu: 785080.6016945982 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_interactions/min_time:15.000",
            "value": 2320828.7449856647,
            "unit": "ns/iter",
            "extra": "iterations: 9074\ncpu: 2320613.588274184 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi_predict/cb_las_small_300_onestep/min_time:15.000/real_time",
            "value": 51.756069197530906,
            "unit": "ms/iter",
            "extra": "iterations: 405\ncpu: 51.75215086419751 ms\nthreads: 1"
          },
          {
            "name": "benchmark_multi_predict/cb_las_small_300_onestep_max_threads/min_time:15.000/real_time",
            "value": 28.367023432732303,
            "unit": "ms/iter",
            "extra": "iterations: 721\ncpu: 2.6231434119278956 ms\nthreads: 1"
          },
          {
            "name": "benchmark_multi_predict/cb_las_small_300_plaincb/min_time:15.000/real_time",
            "value": 1.504105482800003,
            "unit": "ms/iter",
            "extra": "iterations: 10000\ncpu: 1.5039692799999955 ms\nthreads: 1"
          },
          {
            "name": "benchmark_multi_predict/cb_las_small_500_onestep/min_time:15.000/real_time",
            "value": 86.94527708713665,
            "unit": "ms/iter",
            "extra": "iterations: 241\ncpu: 86.93572821576755 ms\nthreads: 1"
          },
          {
            "name": "benchmark_multi_predict/cb_las_small_500_onestep_max_threads/min_time:15.000/real_time",
            "value": 47.391939794117796,
            "unit": "ms/iter",
            "extra": "iterations: 442\ncpu: 4.289816742081431 ms\nthreads: 1"
          },
          {
            "name": "benchmark_multi_predict/cb_las_small_500_plaincb/min_time:15.000/real_time",
            "value": 2.561109244181793,
            "unit": "ms/iter",
            "extra": "iterations: 8207\ncpu: 2.5608368100402106 ms\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/simple/min_time:15.000",
            "value": 76070.96556498297,
            "unit": "ns/iter",
            "extra": "iterations: 276637\ncpu: 76064.58716657569 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/quadratic/min_time:15.000",
            "value": 2795952.0665122033,
            "unit": "ns/iter",
            "extra": "iterations: 7337\ncpu: 2795710.344827583 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_string_fts",
            "value": 1149.7545514319393,
            "unit": "ns/iter",
            "extra": "iterations: 3648577\ncpu: 1149.6592781240363 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_string_fts",
            "value": 8332.701765856473,
            "unit": "ns/iter",
            "extra": "iterations: 509328\ncpu: 8331.918331605448 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_num_fts",
            "value": 865.5168067355871,
            "unit": "ns/iter",
            "extra": "iterations: 4875099\ncpu: 865.4506503355145 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_num_fts",
            "value": 5776.1683687884215,
            "unit": "ns/iter",
            "extra": "iterations: 725663\ncpu: 5775.68568329921 ns\nthreads: 1"
          },
          {
            "name": "benchmark_example_reuse",
            "value": 1023.4945839054974,
            "unit": "ns/iter",
            "extra": "iterations: 4094094\ncpu: 1023.4114556236277 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_char",
            "value": 55.17542509432118,
            "unit": "ns/iter",
            "extra": "iterations: 77110416\ncpu: 55.169987670667034 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_extent",
            "value": 80.92851170756494,
            "unit": "ns/iter",
            "extra": "iterations: 52315881\ncpu: 80.92048760490204 ns\nthreads: 1"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "rucker.mark@gmail.com",
            "name": "Mark Rucker",
            "username": "mrucker"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "e3685a0e59a892464fd46ff8670890c7f9a77a6d",
          "message": "fix: This patches a bug with flat_example collision cleanup (#4265)\n\n* Fixed a bug with flat example collision cleanup. Added two unit tests for the old bug.\r\n\r\n* Fixed a documentation typo.\r\n\r\n* Update vowpalwabbit/core/tests/flat_example_test.cc\r\n\r\nCo-authored-by: Jack Gerrits <jackgerrits@users.noreply.github.com>\r\n\r\n* Update vowpalwabbit/core/tests/flat_example_test.cc\r\n\r\nCo-authored-by: Jack Gerrits <jackgerrits@users.noreply.github.com>\r\n\r\n* fix test output\r\n\r\n* finish example\r\n\r\n* free the flat example\r\n\r\n* fix tests\r\n\r\n* Use make_args\r\n\r\n* test diff\r\n\r\n* update test\r\n\r\nCo-authored-by: Jack Gerrits <jackgerrits@users.noreply.github.com>\r\nCo-authored-by: Jack Gerrits <jackgerrits95@gmail.com>",
          "timestamp": "2022-11-07T14:18:24-05:00",
          "tree_id": "058ca0cc7cc3c460117c3290d26dbc100922a812",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/e3685a0e59a892464fd46ff8670890c7f9a77a6d"
        },
        "date": 1667851073178,
        "tool": "benchmarkdotnet",
        "benches": [
          {
            "name": "BenchmarkText.Benchmark(args: 120_num_features)",
            "value": 5824.916780911959,
            "unit": "ns",
            "range": "± 33.09959081527648"
          },
          {
            "name": "BenchmarkText.Benchmark(args: 120_string_fts)",
            "value": 9418.35686819894,
            "unit": "ns",
            "range": "± 38.46222768740183"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 1_feature)",
            "value": 648.6748082297189,
            "unit": "ns",
            "range": "± 4.371259545122142"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 8_features)",
            "value": 491.6290219624837,
            "unit": "ns",
            "range": "± 5.010149059818464"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_interactions)",
            "value": 889841.9596354166,
            "unit": "ns",
            "range": "± 1301.5882921833534"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_no_interactions)",
            "value": 549955.5078125,
            "unit": "ns",
            "range": "± 2258.9662674139286"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_no_namespaces)",
            "value": 555544.7721354166,
            "unit": "ns",
            "range": "± 2276.9658809324783"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_interactions)",
            "value": 906353.2486979166,
            "unit": "ns",
            "range": "± 2420.280251704903"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_no_interactions)",
            "value": 555496.19140625,
            "unit": "ns",
            "range": "± 973.4545180137894"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_interactions)",
            "value": 3562134.7395833335,
            "unit": "ns",
            "range": "± 12039.929293144414"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_no_interactions)",
            "value": 1098760.5859375,
            "unit": "ns",
            "range": "± 2794.9971261364312"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_no_namespaces)",
            "value": 922678.75,
            "unit": "ns",
            "range": "± 4008.001080054917"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_interactions)",
            "value": 2855655.7291666665,
            "unit": "ns",
            "range": "± 8985.208167728006"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_no_interactions)",
            "value": 950975.1171875,
            "unit": "ns",
            "range": "± 2549.5861773404204"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: few_features)",
            "value": 3198.016560872396,
            "unit": "ns",
            "range": "± 13.901353288974361"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: few_features)",
            "value": 11071.164143880209,
            "unit": "ns",
            "range": "± 25.845092900894276"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: many_features)",
            "value": 109226.94173177083,
            "unit": "ns",
            "range": "± 198.70110937097454"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: many_features)",
            "value": 18903.47399030413,
            "unit": "ns",
            "range": "± 50.20786203319398"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: quadratic)",
            "value": 4235195.104166667,
            "unit": "ns",
            "range": "± 10594.445795284288"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: simple)",
            "value": 203637.11111886162,
            "unit": "ns",
            "range": "± 1162.1871551529684"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "olgavrou@gmail.com",
            "name": "olgavrou",
            "username": "olgavrou"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "0406c0f9310dac89fce1b268123d6e91b8d71440",
          "message": "fix: explore_eval don't learn if logged action not in predicted actions (#4262)",
          "timestamp": "2022-11-08T10:47:21-05:00",
          "tree_id": "50e68b506d58e541f57047afe30a335116ef5fc9",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/0406c0f9310dac89fce1b268123d6e91b8d71440"
        },
        "date": 1667923382465,
        "tool": "googlecpp",
        "benches": [
          {
            "name": "bench_text/120_string_fts",
            "value": 6859.246889412877,
            "unit": "ns/iter",
            "extra": "iterations: 606879\ncpu: 6858.179307572021 ns\nthreads: 1"
          },
          {
            "name": "bench_text/120_num_fts",
            "value": 5099.522836039229,
            "unit": "ns/iter",
            "extra": "iterations: 811371\ncpu: 5098.696157491456 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/8_features",
            "value": 91.50292376736475,
            "unit": "ns/iter",
            "extra": "iterations: 49356184\ncpu: 91.48961759280253 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/1_feature",
            "value": 180.85046611630935,
            "unit": "ns/iter",
            "extra": "iterations: 22420477\ncpu: 180.82628215269466 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features",
            "value": 7894.693481554826,
            "unit": "ns/iter",
            "extra": "iterations: 547109\ncpu: 7893.11252419536 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features",
            "value": 13814.558032090756,
            "unit": "ns/iter",
            "extra": "iterations: 308002\ncpu: 13805.949961363882 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features_no_predict",
            "value": 3660.5250660030465,
            "unit": "ns/iter",
            "extra": "iterations: 1121160\ncpu: 3659.9943808198636 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features_no_predic",
            "value": 5407.565200410372,
            "unit": "ns/iter",
            "extra": "iterations: 768124\ncpu: 5406.810228556851 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/few_features",
            "value": 1360.482464639369,
            "unit": "ns/iter",
            "extra": "iterations: 3181999\ncpu: 1360.2994218414267 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/many_features/min_time:15.000",
            "value": 63844.12027446962,
            "unit": "ns/iter",
            "extra": "iterations: 324699\ncpu: 63834.88923587691 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_no_namespaces/min_time:15.000",
            "value": 280690.5571169016,
            "unit": "ns/iter",
            "extra": "iterations: 78707\ncpu: 280592.1061659064 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 319413.84153553785,
            "unit": "ns/iter",
            "extra": "iterations: 65775\ncpu: 319362.596731281 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_interactions/min_time:15.000",
            "value": 588180.0233226302,
            "unit": "ns/iter",
            "extra": "iterations: 35502\ncpu: 588024.9000056334 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_no_interactions/min_time:15.000",
            "value": 273750.60057744134,
            "unit": "ns/iter",
            "extra": "iterations: 75852\ncpu: 273705.6649791699 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_interactions/min_time:15.000",
            "value": 434086.4962543702,
            "unit": "ns/iter",
            "extra": "iterations: 48056\ncpu: 434016.0729149328 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_no_namespaces/min_time:15.000",
            "value": 675477.6652833199,
            "unit": "ns/iter",
            "extra": "iterations: 31325\ncpu: 675362.1069433353 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 843845.4708563968,
            "unit": "ns/iter",
            "extra": "iterations: 24568\ncpu: 843683.0918267677 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_interactions/min_time:15.000",
            "value": 2694927.405802565,
            "unit": "ns/iter",
            "extra": "iterations: 7962\ncpu: 2694197.488068325 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_no_interactions/min_time:15.000",
            "value": 676326.671571302,
            "unit": "ns/iter",
            "extra": "iterations: 31127\ncpu: 676209.8596074133 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_interactions/min_time:15.000",
            "value": 1998921.7750000018,
            "unit": "ns/iter",
            "extra": "iterations: 10440\ncpu: 1998364.6168582358 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi_predict/cb_las_small_300_onestep/min_time:15.000/real_time",
            "value": 44.4910033837719,
            "unit": "ms/iter",
            "extra": "iterations: 456\ncpu: 44.483434210526326 ms\nthreads: 1"
          },
          {
            "name": "benchmark_multi_predict/cb_las_small_300_onestep_max_threads/min_time:15.000/real_time",
            "value": 25.38691017303109,
            "unit": "ms/iter",
            "extra": "iterations: 838\ncpu: 2.1604644391408114 ms\nthreads: 1"
          },
          {
            "name": "benchmark_multi_predict/cb_las_small_300_plaincb/min_time:15.000/real_time",
            "value": 1.3179023820645754,
            "unit": "ms/iter",
            "extra": "iterations: 16013\ncpu: 1.3176932117654412 ms\nthreads: 1"
          },
          {
            "name": "benchmark_multi_predict/cb_las_small_500_onestep/min_time:15.000/real_time",
            "value": 80.85715444615377,
            "unit": "ms/iter",
            "extra": "iterations: 260\ncpu: 80.84437461538458 ms\nthreads: 1"
          },
          {
            "name": "benchmark_multi_predict/cb_las_small_500_onestep_max_threads/min_time:15.000/real_time",
            "value": 42.94962639578946,
            "unit": "ms/iter",
            "extra": "iterations: 475\ncpu: 3.7200082105262386 ms\nthreads: 1"
          },
          {
            "name": "benchmark_multi_predict/cb_las_small_500_plaincb/min_time:15.000/real_time",
            "value": 2.2883975603252007,
            "unit": "ms/iter",
            "extra": "iterations: 9225\ncpu: 2.288018905149053 ms\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/simple/min_time:15.000",
            "value": 55873.78808159301,
            "unit": "ns/iter",
            "extra": "iterations: 386847\ncpu: 55856.288661925784 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/quadratic/min_time:15.000",
            "value": 3104115.6210670373,
            "unit": "ns/iter",
            "extra": "iterations: 6579\ncpu: 3102846.922024622 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_string_fts",
            "value": 1047.2768071718115,
            "unit": "ns/iter",
            "extra": "iterations: 3940093\ncpu: 1047.1289890873063 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_string_fts",
            "value": 8602.171299819298,
            "unit": "ns/iter",
            "extra": "iterations: 487537\ncpu: 8600.886701932319 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_num_fts",
            "value": 819.9111843598711,
            "unit": "ns/iter",
            "extra": "iterations: 5263949\ncpu: 819.4408988385086 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_num_fts",
            "value": 6301.139352745809,
            "unit": "ns/iter",
            "extra": "iterations: 654117\ncpu: 6300.204397684182 ns\nthreads: 1"
          },
          {
            "name": "benchmark_example_reuse",
            "value": 883.8469412049826,
            "unit": "ns/iter",
            "extra": "iterations: 4561306\ncpu: 883.7164399845244 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_char",
            "value": 59.09813759967895,
            "unit": "ns/iter",
            "extra": "iterations: 72158439\ncpu: 59.089897995160904 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_extent",
            "value": 83.8388783912335,
            "unit": "ns/iter",
            "extra": "iterations: 50364157\ncpu: 83.82736754632877 ns\nthreads: 1"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "olgavrou@gmail.com",
            "name": "olgavrou",
            "username": "olgavrou"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "0406c0f9310dac89fce1b268123d6e91b8d71440",
          "message": "fix: explore_eval don't learn if logged action not in predicted actions (#4262)",
          "timestamp": "2022-11-08T10:47:21-05:00",
          "tree_id": "50e68b506d58e541f57047afe30a335116ef5fc9",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/0406c0f9310dac89fce1b268123d6e91b8d71440"
        },
        "date": 1667925213127,
        "tool": "benchmarkdotnet",
        "benches": [
          {
            "name": "BenchmarkText.Benchmark(args: 120_num_features)",
            "value": 7248.531913757324,
            "unit": "ns",
            "range": "± 188.3466784361335"
          },
          {
            "name": "BenchmarkText.Benchmark(args: 120_string_fts)",
            "value": 10655.110814021184,
            "unit": "ns",
            "range": "± 137.4237944981856"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 1_feature)",
            "value": 836.7834981282552,
            "unit": "ns",
            "range": "± 14.921830175662175"
          },
          {
            "name": "BenchmarkLearnSimple.Benchmark(args: 8_features)",
            "value": 683.8892936706543,
            "unit": "ns",
            "range": "± 9.347453598931901"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_interactions)",
            "value": 1247520.9391276042,
            "unit": "ns",
            "range": "± 29911.941634445713"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_diff_char_no_interactions)",
            "value": 762284.4876802885,
            "unit": "ns",
            "range": "± 12556.143584865926"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_no_namespaces)",
            "value": 749556.5494791666,
            "unit": "ns",
            "range": "± 13019.321600802232"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_interactions)",
            "value": 1177728.7020596592,
            "unit": "ns",
            "range": "± 28653.11353467836"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: cb_adf_same_char_no_interactions)",
            "value": 741835.7759915865,
            "unit": "ns",
            "range": "± 18708.155176193726"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_interactions)",
            "value": 4930271.40625,
            "unit": "ns",
            "range": "± 91928.40430378255"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_diff_char_no_interactions)",
            "value": 1518749.8121995192,
            "unit": "ns",
            "range": "± 40538.486575635994"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_no_namespaces)",
            "value": 1229535.224609375,
            "unit": "ns",
            "range": "± 27998.652788770192"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_interactions)",
            "value": 4281342.96875,
            "unit": "ns",
            "range": "± 134744.0429505872"
          },
          {
            "name": "BenchmarkMulti.Benchmark(args: ccb_adf_same_char_no_interactions)",
            "value": 1288673.4969429348,
            "unit": "ns",
            "range": "± 31548.84758668734"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: few_features)",
            "value": 4358.1057371916595,
            "unit": "ns",
            "range": "± 180.4520927570517"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: few_features)",
            "value": 15023.271814982096,
            "unit": "ns",
            "range": "± 379.05940736503095"
          },
          {
            "name": "BenchmarkCbAdfLearn.Benchmark(args: many_features)",
            "value": 154570.13790246213,
            "unit": "ns",
            "range": "± 4872.773490263056"
          },
          {
            "name": "BenchmarkCcbAdfLearn.Benchmark(args: many_features)",
            "value": 25719.305877685547,
            "unit": "ns",
            "range": "± 588.6808759101548"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: quadratic)",
            "value": 5630784.147135417,
            "unit": "ns",
            "range": "± 142819.85094119186"
          },
          {
            "name": "BenchmarkRCV1.Benchmark(args: simple)",
            "value": 319866.56319754466,
            "unit": "ns",
            "range": "± 11672.727796618528"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "olgavrou@gmail.com",
            "name": "olgavrou",
            "username": "olgavrou"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "16e9114f41343eed0a5f3f9881b171ce4ea6774a",
          "message": "fix: [LAS] full predictions regardless of learn/predict path (#4273)",
          "timestamp": "2022-11-08T13:46:00-05:00",
          "tree_id": "a9e20d311c625825aa7f51c08d1816c2c811bca2",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/16e9114f41343eed0a5f3f9881b171ce4ea6774a"
        },
        "date": 1667934043975,
        "tool": "googlecpp",
        "benches": [
          {
            "name": "bench_text/120_string_fts",
            "value": 5242.394135605011,
            "unit": "ns/iter",
            "extra": "iterations: 798923\ncpu: 5241.526404922627 ns\nthreads: 1"
          },
          {
            "name": "bench_text/120_num_fts",
            "value": 3957.584241334863,
            "unit": "ns/iter",
            "extra": "iterations: 1062590\ncpu: 3957.3980556941056 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/8_features",
            "value": 69.69209094194585,
            "unit": "ns/iter",
            "extra": "iterations: 60630899\ncpu: 69.69086339953498 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/1_feature",
            "value": 159.19806141092477,
            "unit": "ns/iter",
            "extra": "iterations: 26310269\ncpu: 159.194852777826 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features",
            "value": 7306.395902963316,
            "unit": "ns/iter",
            "extra": "iterations: 572072\ncpu: 7306.116013368944 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features",
            "value": 12678.255896844028,
            "unit": "ns/iter",
            "extra": "iterations: 330219\ncpu: 12677.719331716225 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features_no_predict",
            "value": 3582.4623443706223,
            "unit": "ns/iter",
            "extra": "iterations: 1198360\ncpu: 3582.366651089824 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features_no_predic",
            "value": 5326.505359312464,
            "unit": "ns/iter",
            "extra": "iterations: 823893\ncpu: 5326.375997854093 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/few_features",
            "value": 1240.7490680638218,
            "unit": "ns/iter",
            "extra": "iterations: 3403130\ncpu: 1240.7173983950063 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/many_features/min_time:15.000",
            "value": 52370.87339065346,
            "unit": "ns/iter",
            "extra": "iterations: 404046\ncpu: 52369.43664830241 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_no_namespaces/min_time:15.000",
            "value": 208303.26733532202,
            "unit": "ns/iter",
            "extra": "iterations: 102594\ncpu: 208296.02218453318 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 237685.68083238899,
            "unit": "ns/iter",
            "extra": "iterations: 90006\ncpu: 237678.2636713108 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_interactions/min_time:15.000",
            "value": 468196.8784220807,
            "unit": "ns/iter",
            "extra": "iterations: 45148\ncpu: 468180.53512890945 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_no_interactions/min_time:15.000",
            "value": 206655.61256575497,
            "unit": "ns/iter",
            "extra": "iterations: 101514\ncpu: 206648.9676300805 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_interactions/min_time:15.000",
            "value": 352412.26071297424,
            "unit": "ns/iter",
            "extra": "iterations: 59974\ncpu: 352399.82158935553 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_no_namespaces/min_time:15.000",
            "value": 585801.9579968589,
            "unit": "ns/iter",
            "extra": "iterations: 35664\ncpu: 585778.5217586361 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 723410.3452089695,
            "unit": "ns/iter",
            "extra": "iterations: 29023\ncpu: 723381.2252351582 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_interactions/min_time:15.000",
            "value": 2248179.9404412634,
            "unit": "ns/iter",
            "extra": "iterations: 9201\ncpu: 2248095.674383215 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_no_interactions/min_time:15.000",
            "value": 595556.0912149436,
            "unit": "ns/iter",
            "extra": "iterations: 34479\ncpu: 595530.3924127728 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_interactions/min_time:15.000",
            "value": 1771685.5209278835,
            "unit": "ns/iter",
            "extra": "iterations: 11898\ncpu: 1771612.1617078504 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi_predict/cb_las_small_300_onestep/min_time:15.000/real_time",
            "value": 46.46434129867266,
            "unit": "ms/iter",
            "extra": "iterations: 452\ncpu: 46.462920796460146 ms\nthreads: 1"
          },
          {
            "name": "benchmark_multi_predict/cb_las_small_300_onestep_max_threads/min_time:15.000/real_time",
            "value": 24.5413473908984,
            "unit": "ms/iter",
            "extra": "iterations: 857\ncpu: 1.8537323220536746 ms\nthreads: 1"
          },
          {
            "name": "benchmark_multi_predict/cb_las_small_300_plaincb/min_time:15.000/real_time",
            "value": 1.2026120234285047,
            "unit": "ms/iter",
            "extra": "iterations: 17372\ncpu: 1.2025791158185581 ms\nthreads: 1"
          },
          {
            "name": "benchmark_multi_predict/cb_las_small_500_onestep/min_time:15.000/real_time",
            "value": 77.5116927638378,
            "unit": "ms/iter",
            "extra": "iterations: 271\ncpu: 77.50939372693719 ms\nthreads: 1"
          },
          {
            "name": "benchmark_multi_predict/cb_las_small_500_onestep_max_threads/min_time:15.000/real_time",
            "value": 40.893079214843866,
            "unit": "ms/iter",
            "extra": "iterations: 512\ncpu: 3.0096675781249305 ms\nthreads: 1"
          },
          {
            "name": "benchmark_multi_predict/cb_las_small_500_plaincb/min_time:15.000/real_time",
            "value": 2.0554383041954503,
            "unit": "ms/iter",
            "extra": "iterations: 9987\ncpu: 2.0552584359667576 ms\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/simple/min_time:15.000",
            "value": 48265.602890082366,
            "unit": "ns/iter",
            "extra": "iterations: 432306\ncpu: 48263.97991237689 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/quadratic/min_time:15.000",
            "value": 2227994.4016560856,
            "unit": "ns/iter",
            "extra": "iterations: 9299\ncpu: 2227699.3547693305 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_string_fts",
            "value": 930.2505837556106,
            "unit": "ns/iter",
            "extra": "iterations: 4516873\ncpu: 930.21563811955 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_string_fts",
            "value": 7217.702822616136,
            "unit": "ns/iter",
            "extra": "iterations: 582403\ncpu: 7217.439127202298 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_num_fts",
            "value": 692.2545806894503,
            "unit": "ns/iter",
            "extra": "iterations: 6069392\ncpu: 692.2345599031931 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_num_fts",
            "value": 5071.671693209714,
            "unit": "ns/iter",
            "extra": "iterations: 829596\ncpu: 5071.541690172152 ns\nthreads: 1"
          },
          {
            "name": "benchmark_example_reuse",
            "value": 773.9353066462919,
            "unit": "ns/iter",
            "extra": "iterations: 5413941\ncpu: 773.9113521924288 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_char",
            "value": 40.28210835133132,
            "unit": "ns/iter",
            "extra": "iterations: 104148714\ncpu: 40.280983210219865 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_extent",
            "value": 55.73616083643171,
            "unit": "ns/iter",
            "extra": "iterations: 78799560\ncpu: 55.73482770715896 ns\nthreads: 1"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "jackgerrits@users.noreply.github.com",
            "name": "Jack Gerrits",
            "username": "jackgerrits"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "3e615eb4b3515198f90818fe3608cf3c5bf61a01",
          "message": "ci: use shared caches for vcpkg job (#4270)\n\n* ci: use shared caches for vcpkg job\r\n\r\n* Update vcpkg_build.yml\r\n\r\n* Try change to generator expression\r\n\r\n* Update vcpkg_build.yml\r\n\r\n* Update VWFlags.cmake",
          "timestamp": "2022-11-08T23:53:03-05:00",
          "tree_id": "c3d03859d523d0d158eaaf3b376b06b010540255",
          "url": "https://github.com/VowpalWabbit/vowpal_wabbit/commit/3e615eb4b3515198f90818fe3608cf3c5bf61a01"
        },
        "date": 1667970455505,
        "tool": "googlecpp",
        "benches": [
          {
            "name": "bench_text/120_string_fts",
            "value": 5262.543156733074,
            "unit": "ns/iter",
            "extra": "iterations: 791939\ncpu: 5260.605046600812 ns\nthreads: 1"
          },
          {
            "name": "bench_text/120_num_fts",
            "value": 3961.068441533388,
            "unit": "ns/iter",
            "extra": "iterations: 1062615\ncpu: 3960.93674566988 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/8_features",
            "value": 69.72101426674128,
            "unit": "ns/iter",
            "extra": "iterations: 62280027\ncpu: 69.71882173397259 ns\nthreads: 1"
          },
          {
            "name": "benchmark_learn_simple/1_feature",
            "value": 157.98015531080608,
            "unit": "ns/iter",
            "extra": "iterations: 25648827\ncpu: 157.97598073393382 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features",
            "value": 6961.220503869315,
            "unit": "ns/iter",
            "extra": "iterations: 593527\ncpu: 6960.6095426155825 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features",
            "value": 12388.915794822291,
            "unit": "ns/iter",
            "extra": "iterations: 336547\ncpu: 12388.395974410705 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/few_features_no_predict",
            "value": 3306.1730017208174,
            "unit": "ns/iter",
            "extra": "iterations: 1271444\ncpu: 3306.0479266094294 ns\nthreads: 1"
          },
          {
            "name": "benchmark_ccb_adf_learn/many_features_no_predic",
            "value": 5089.436963122214,
            "unit": "ns/iter",
            "extra": "iterations: 825374\ncpu: 5089.234698451853 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/few_features",
            "value": 1253.8804930305014,
            "unit": "ns/iter",
            "extra": "iterations: 3368473\ncpu: 1253.8375697237311 ns\nthreads: 1"
          },
          {
            "name": "benchmark_cb_adf_learn/many_features/min_time:15.000",
            "value": 52323.28657734234,
            "unit": "ns/iter",
            "extra": "iterations: 399742\ncpu: 52321.66372310141 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_no_namespaces/min_time:15.000",
            "value": 199593.30069621725,
            "unit": "ns/iter",
            "extra": "iterations: 106576\ncpu: 199586.11319621676 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 230283.34177928593,
            "unit": "ns/iter",
            "extra": "iterations: 90497\ncpu: 230273.15933124858 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_diff_char_interactions/min_time:15.000",
            "value": 471227.05814975937,
            "unit": "ns/iter",
            "extra": "iterations: 45486\ncpu: 471207.5187969923 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_no_interactions/min_time:15.000",
            "value": 199097.85749104983,
            "unit": "ns/iter",
            "extra": "iterations: 105032\ncpu: 199090.80756340924 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/cb_adf_same_char_interactions/min_time:15.000",
            "value": 342736.31234025845,
            "unit": "ns/iter",
            "extra": "iterations: 61036\ncpu: 342724.16442755074 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_no_namespaces/min_time:15.000",
            "value": 575523.3658603232,
            "unit": "ns/iter",
            "extra": "iterations: 36585\ncpu: 575495.7058903922 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_no_interactions/min_time:15.000",
            "value": 719127.8189266411,
            "unit": "ns/iter",
            "extra": "iterations: 29049\ncpu: 719092.4162621773 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_diff_char_interactions/min_time:15.000",
            "value": 2244625.696289178,
            "unit": "ns/iter",
            "extra": "iterations: 9351\ncpu: 2244513.5279649217 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_no_interactions/min_time:15.000",
            "value": 598940.8311673524,
            "unit": "ns/iter",
            "extra": "iterations: 35088\ncpu: 598912.3689010482 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi/ccb_adf_same_char_interactions/min_time:15.000",
            "value": 1795781.8014257487,
            "unit": "ns/iter",
            "extra": "iterations: 11643\ncpu: 1795697.3546336826 ns\nthreads: 1"
          },
          {
            "name": "benchmark_multi_predict/cb_las_small_300_onestep/min_time:15.000/real_time",
            "value": 46.54765603769412,
            "unit": "ms/iter",
            "extra": "iterations: 451\ncpu: 46.54573325942345 ms\nthreads: 1"
          },
          {
            "name": "benchmark_multi_predict/cb_las_small_300_onestep_max_threads/min_time:15.000/real_time",
            "value": 24.75313323244547,
            "unit": "ms/iter",
            "extra": "iterations: 826\ncpu: 1.9416865617433792 ms\nthreads: 1"
          },
          {
            "name": "benchmark_multi_predict/cb_las_small_300_plaincb/min_time:15.000/real_time",
            "value": 1.2159906193300747,
            "unit": "ms/iter",
            "extra": "iterations: 17196\ncpu: 1.2159368981158418 ms\nthreads: 1"
          },
          {
            "name": "benchmark_multi_predict/cb_las_small_500_onestep/min_time:15.000/real_time",
            "value": 77.80594898524022,
            "unit": "ms/iter",
            "extra": "iterations: 271\ncpu: 77.80261771217691 ms\nthreads: 1"
          },
          {
            "name": "benchmark_multi_predict/cb_las_small_500_onestep_max_threads/min_time:15.000/real_time",
            "value": 41.01540782031243,
            "unit": "ms/iter",
            "extra": "iterations: 512\ncpu: 3.1137574218749187 ms\nthreads: 1"
          },
          {
            "name": "benchmark_multi_predict/cb_las_small_500_plaincb/min_time:15.000/real_time",
            "value": 2.065290345954045,
            "unit": "ms/iter",
            "extra": "iterations: 10010\ncpu: 2.065173936063933 ms\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/simple/min_time:15.000",
            "value": 49389.94599732352,
            "unit": "ns/iter",
            "extra": "iterations: 422942\ncpu: 49388.315892013634 ns\nthreads: 1"
          },
          {
            "name": "benchmark_rcv1_dataset/quadratic/min_time:15.000",
            "value": 2373143.7868777313,
            "unit": "ns/iter",
            "extra": "iterations: 8718\ncpu: 2373058.6831842214 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_string_fts",
            "value": 928.9982517624807,
            "unit": "ns/iter",
            "extra": "iterations: 4540573\ncpu: 928.9702643256695 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_string_fts",
            "value": 7187.951818860155,
            "unit": "ns/iter",
            "extra": "iterations: 584212\ncpu: 7187.757012865201 ns\nthreads: 1"
          },
          {
            "name": "bench_cache_io_buf/120_num_fts",
            "value": 695.1992033905049,
            "unit": "ns/iter",
            "extra": "iterations: 6036835\ncpu: 695.1744581390786 ns\nthreads: 1"
          },
          {
            "name": "bench_text_io_buf/120_num_fts",
            "value": 5061.663181757974,
            "unit": "ns/iter",
            "extra": "iterations: 829290\ncpu: 5061.514428004656 ns\nthreads: 1"
          },
          {
            "name": "benchmark_example_reuse",
            "value": 771.2447004330063,
            "unit": "ns/iter",
            "extra": "iterations: 5437048\ncpu: 771.2236125191351 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_char",
            "value": 40.217980006028306,
            "unit": "ns/iter",
            "extra": "iterations: 104323747\ncpu: 40.21685494099518 ns\nthreads: 1"
          },
          {
            "name": "benchmark_sum_ft_squared_extent",
            "value": 53.28139056195141,
            "unit": "ns/iter",
            "extra": "iterations: 78785990\ncpu: 53.27958562175711 ns\nthreads: 1"
          }
        ]
      }
    ]
  }
}