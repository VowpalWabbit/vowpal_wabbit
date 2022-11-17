import vowpalwabbit


def test_cats():
    min_value = 10
    max_value = 20

    vw = vowpalwabbit.Workspace(
        "--cats 4 --min_value "
        + str(min_value)
        + " --max_value "
        + str(max_value)
        + " --bandwidth 1"
    )
    vw_example = vw.parse(
        "ca 15:0.657567:6.20426e-05 | f1 f2 f3 f4", vowpalwabbit.LabelType.CONTINUOUS
    )
    vw.learn(vw_example)
    vw.finish_example(vw_example)

    assert (
        vw.get_prediction_type() == vowpalwabbit.PredictionType.ACTION_PDF_VALUE
    ), "prediction_type should be action_pdf_value"

    action, pdf_value = vw.predict("| f1 f2 f3 f4")
    assert action >= 10
    assert action <= 20
    vw.finish()


def test_cats_pdf():
    min_value = 10
    max_value = 20

    vw = vowpalwabbit.Workspace(
        "--cats_pdf 4 --min_value "
        + str(min_value)
        + " --max_value "
        + str(max_value)
        + " --bandwidth 1"
    )
    vw_example = vw.parse(
        "ca 15:0.657567:6.20426e-05 | f1 f2 f3 f4", vowpalwabbit.LabelType.CONTINUOUS
    )
    vw.learn(vw_example)
    vw.finish_example(vw_example)

    assert (
        vw.get_prediction_type() == vowpalwabbit.PredictionType.PDF
    ), "prediction_type should be pdf"

    pdf_segments = vw.predict("| f1 f2 f3 f4")
    mass = 0
    for segment in pdf_segments:
        assert len(segment) == 3

        # returned action range should lie within supplied limits
        assert segment[0] >= min_value
        assert segment[0] <= max_value
        assert segment[1] >= min_value
        assert segment[1] <= max_value

        # pdf value must be non-negative
        assert segment[2] >= 0

        mass += (segment[1] - segment[0]) * segment[2]

    assert mass >= 0.9999 and mass <= 1.0001

    vw.finish()
