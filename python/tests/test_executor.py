from vw_executor.vw import Vw
from vw_executor.vw_opts import Grid

from numpy.testing import assert_allclose


def core_test(files, grid, outputs, job_assert):
    vw = Vw('.vw_cache', reset=True, handlers=[])
    result = vw.train(files, grid, outputs)

    for job in result:
        job_assert(job)


def assert_valid_pmf(pmf, err_msg):
    for p in pmf:
        assert p >= 0, err_msg
        assert p <= 1, err_msg
    assert_allclose([sum(pmf)], [1], atol=1e-3, err_msg=err_msg)


def test_cb_predicitons_are_valid_pmf():
    options = Grid({
        '#base': ['--cb_explore_adf --dsjson'],
        '#explorations': [
            '--epsilon 0.1',
            '--squarecb',
            '--squarecb --gamma_scale 100',
            '--cover 5',
            '--synthcover',
            '--softmax',
        ]
    })

    def are_predictions_pmf(job):
        predictions = job[0].predictions('-p').cb
        for _, row in predictions.iterrows():
            pmf = list(row)
            assert_valid_pmf(pmf, f'Predictions for "{job.name}" is not valid PMF')

    core_test('test/train-sets/cb_simulated.json', options, ['-p'], are_predictions_pmf)