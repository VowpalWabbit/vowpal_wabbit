window.BENCHMARK_DATA = {
  "lastUpdate": 1665498683392,
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
      }
    ]
  }
}