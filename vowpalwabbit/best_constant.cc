#include "best_constant.h"

bool get_best_constant(vw& all, float& best_constant, float& best_constant_loss)
{
    if ((all.loss == NULL) || (all.sd == NULL)) return false;

    float label1 = all.sd->min_label;
    float label2 = all.sd->max_label;

    float label1_cnt;
    float label2_cnt;

    if (label1 != label2)
    {
        float weighted_labeled_examples = (float)(all.sd->weighted_examples - all.sd->weighted_unlabeled_examples + all.initial_t);
        label1_cnt = (all.sd->weighted_labels - label2*weighted_labeled_examples)/(label1 - label2);
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
        funcName = "squaredloss";

    if(funcName.compare("squared") == 0 || funcName.compare("Huber") == 0 || funcName.compare("classic") == 0)
    {
        best_constant = (label1*label1_cnt + label2*label2_cnt) / (label1_cnt + label2_cnt);

    } else if(funcName.compare("hinge") == 0) {

        best_constant = label2_cnt <= label1_cnt ? -1.: 1.;

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


    best_constant_loss = ( all.loss->getLoss(all.sd, best_constant, label1) * label1_cnt +
                           all.loss->getLoss(all.sd, best_constant, label2) * label2_cnt )
            / (label1_cnt + label2_cnt);


    return true;
}
