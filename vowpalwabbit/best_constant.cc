#include "best_constant.h"

bool  is_more_than_two_labels_observed = false;
float first_observed_label = FLT_MAX;
float second_observed_label = FLT_MAX;

bool get_best_constant(vw& all, float& best_constant, float& best_constant_loss)
{
    if (    first_observed_label == FLT_MAX || // no non-test labels observed or function was never called
            (all.loss == NULL) || (all.sd == NULL)) return false;

    float label1 = first_observed_label; // observed labels might be inside [sd->Min_label, sd->Max_label], so can't use Min/Max
    float label2 = (second_observed_label == FLT_MAX)?0:second_observed_label; // if only one label observed, second might be 0
    if (label1 > label2) {float tmp = label1; label1 = label2; label2 = tmp;} // as don't use min/max - make sure label1 < label2

    float label1_cnt;
    float label2_cnt;

    if (label1 != label2)
    {
        float weighted_labeled_examples = (float)(all.sd->weighted_examples - all.sd->weighted_unlabeled_examples + all.initial_t);
        label1_cnt = (float) (all.sd->weighted_labels - label2*weighted_labeled_examples)/(label1 - label2);
        label2_cnt = weighted_labeled_examples - label1_cnt;
    } else
        return false;

    if ( (label1_cnt + label2_cnt) <= 0.) return false;

    po::parsed_options pos = po::command_line_parser(all.args).
            style(po::command_line_style::default_style ^ po::command_line_style::allow_guessing).
            options(all.opts).allow_unregistered().run();

    po::variables_map vm = po::variables_map();

    po::store(pos, vm);
    po::notify(vm);

    string funcName;
    if(vm.count("loss_function"))
        funcName = vm["loss_function"].as<string>();
    else
        funcName = "squared";

    if(funcName.compare("squared") == 0 || funcName.compare("Huber") == 0 || funcName.compare("classic") == 0)
    {
        best_constant = (float) all.sd->weighted_labels / (float) (all.sd->weighted_examples - all.sd->weighted_unlabeled_examples + all.initial_t); //GENERIC. WAS: (label1*label1_cnt + label2*label2_cnt) / (label1_cnt + label2_cnt);

    } else if (is_more_than_two_labels_observed) {
        //loss functions below don't have generic formuas for constant yet.
        return false;

    } else if(funcName.compare("hinge") == 0) {

        best_constant = label2_cnt <= label1_cnt ? -1.f: 1.f;

    } else if(funcName.compare("logistic") == 0) {

        label1 = -1.; //override {-50, 50} to get proper loss
        label2 =  1.;

        if (label1_cnt <= 0) best_constant = 1.;
        else
            if (label2_cnt <= 0) best_constant = -1.;
            else
                best_constant = log(label2_cnt/label1_cnt);

    } else if(funcName.compare("quantile") == 0 || funcName.compare("pinball") == 0 || funcName.compare("absolute") == 0) {

        float tau = 0.5;
        if(vm.count("quantile_tau"))
            tau = vm["quantile_tau"].as<float>();

        float q = tau*(label1_cnt + label2_cnt);
        if (q < label2_cnt) best_constant = label2;
        else best_constant = label1;
    } else
        return false;


    if (!is_more_than_two_labels_observed)
    best_constant_loss = ( all.loss->getLoss(all.sd, best_constant, label1) * label1_cnt +
                           all.loss->getLoss(all.sd, best_constant, label2) * label2_cnt )
            / (label1_cnt + label2_cnt);
    else best_constant_loss = FLT_MIN;


    return true;
}
