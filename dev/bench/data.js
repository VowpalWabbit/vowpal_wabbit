window.BENCHMARK_DATA = {
  "lastUpdate": 1666050232649,
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
      }
    ]
  }
}