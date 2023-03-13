import vowpalwabbit


# Named specifically as the delimiter used is specific for the number of actions
# used in this test case.
def count_weights_from_readable_model_file_for_equiv_test(file_name):
    with open(file_name) as file:
        model_file_contents = file.read()
        DELIM = "action_sum 5\n:0\n"
        weights_idx = model_file_contents.find(DELIM) + len(DELIM)
        return model_file_contents[weights_idx:].count("\n")


def test_ccb_single_slot_and_cb_equivalence_no_slot_features():
    # --- CCB
    ccb_model_file_name = "model_file_ccb_equiv.txt"
    ccb_workspace = vowpalwabbit.Workspace(
        quiet=True,
        predict_only_model=True,
        ccb_explore_adf=True,
        readable_model=ccb_model_file_name,
    )

    ccb_ex = """
    ccb shared |User b
    ccb action |Action d
    ccb action |Action e
    ccb action |Action f
    ccb action |Action ff
    ccb action |Action fff
    ccb slot 4:1:0.2 |
    """
    ccb_workspace.learn(ccb_ex)
    ccb_workspace.finish()

    ccb_num_weights = count_weights_from_readable_model_file_for_equiv_test(
        ccb_model_file_name
    )

    # --- CB
    cb_model_file_name = "model_file_cb_equiv.txt"
    cb_workspace = vowpalwabbit.Workspace(
        quiet=True,
        predict_only_model=True,
        cb_explore_adf=True,
        readable_model=cb_model_file_name,
    )

    cb_ex = """
    shared |User b
    |Action d
    |Action e
    |Action f
    |Action ff
    4:1:0.2 |Action fff
    """

    cb_workspace.learn(cb_ex)
    cb_workspace.finish()
    cb_num_weights = count_weights_from_readable_model_file_for_equiv_test(
        cb_model_file_name
    )

    assert ccb_num_weights == cb_num_weights


def test_ccb_single_slot_and_cb_non_equivalence_with_slot_features():
    # --- CCB
    ccb_model_file_name = "model_file_ccb_no_equiv.txt"
    ccb_workspace = vowpalwabbit.Workspace(
        quiet=True, ccb_explore_adf=True, readable_model=ccb_model_file_name
    )

    ccb_ex = """
    ccb shared |User b
    ccb action |Action d
    ccb action |Action e
    ccb action |Action f
    ccb action |Action ff
    ccb action |Action fff
    ccb slot 4:1:0.2 | slot_feature_1
    """
    ccb_workspace.learn(ccb_ex)
    ccb_workspace.finish()

    ccb_num_weights = count_weights_from_readable_model_file_for_equiv_test(
        ccb_model_file_name
    )

    # --- CB
    cb_model_file_name = "model_file_cb_no_equiv.txt"
    cb_workspace = vowpalwabbit.Workspace(
        quiet=True, cb_explore_adf=True, readable_model=cb_model_file_name
    )

    cb_ex = """
    shared |User b
    |Action d
    |Action e
    |Action f
    |Action ff
    4:1:0.2 |Action fff
    """

    cb_workspace.learn(cb_ex)
    cb_workspace.finish()
    cb_num_weights = count_weights_from_readable_model_file_for_equiv_test(
        cb_model_file_name
    )

    # Since there was at least one slot feature supplied, the equivalent mode
    # does not apply and so we expect there to be more weights in the CCB model.
    assert ccb_num_weights > cb_num_weights


def test_ccb_non_slot_none_outcome():
    model = vowpalwabbit.Workspace(quiet=True, ccb_explore_adf=True)
    example = vowpalwabbit.Example(
        vw=model, labelType=vowpalwabbit.LabelType.CONDITIONAL_CONTEXTUAL_BANDIT
    )
    label = example.get_label(vowpalwabbit.CCBLabel)
    # CCB label is set to UNSET by default.
    assert label.type == vowpalwabbit.CCBLabelType.UNSET
    assert label.outcome is None


def test_ccb_and_automl():
    import random, json, os, shutil
    import numpy as np
    from vw_executor.vw import Vw

    people_ccb = ["Tom", "Anna"]
    topics_ccb = ["sports", "politics", "music"]

    def my_ccb_simulation(n=10000, swap_after=5000, variance=0, bad_features=0, seed=0):
        random.seed(seed)
        np.random.seed(seed)

        envs = [[[0.8, 0.4], [0.2, 0.4]]]
        offset = 0
        for i in range(1, n):
            person = random.randint(0, 1)
            chosen = [int(i) for i in np.random.permutation(2)]
            rewards = [envs[offset][person][chosen[0]], envs[offset][person][chosen[1]]]

            for i in range(len(rewards)):
                rewards[i] += np.random.normal(0.5, variance)

            yield {
                "c": {
                    "shared": {"name": people_ccb[person]},
                    "_multi": [{"a": {"topic": topics_ccb[i]}} for i in range(2)],
                    "_slots": [{"_id": i} for i in range(2)],
                },
                "_outcomes": [
                    {
                        "_label_cost": -min(rewards[i], 1),
                        "_a": chosen[i:],
                        "_p": [1.0 / (2 - i)] * (2 - i),
                    }
                    for i in range(2)
                ],
            }

    def save_examples(examples, path):
        with open(path, "w") as f:
            for ex in examples:
                f.write(f'{json.dumps(ex, separators=(",", ":"))}\n')

    input_file = "ccb.json"
    cache_dir = ".cache"
    save_examples(
        my_ccb_simulation(n=1000, variance=0.1, bad_features=1, seed=0), input_file
    )

    assert os.path.exists(input_file)

    vw = Vw(cache_dir, "/root/vowpal_wabbit/build/vowpalwabbit/cli/vw", handler=None)
    q = vw.train(
        input_file, "-b 18 -q :: --ccb_explore_adf --dsjson", ["--invert_hash"]
    )
    fts_names_q = set([n for n in q[0].model9("--invert_hash").weights.index])

    assert len(fts_names_q) == 39

    os.remove(input_file)
    shutil.rmtree(cache_dir)
    assert not os.path.exists(input_file)
    assert not os.path.exists(cache_dir)
