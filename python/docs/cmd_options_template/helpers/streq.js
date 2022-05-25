var streq = function () {};

streq.register = function (Handlebars) {
    Handlebars.registerHelper('streq', function(arg1, arg2) {
        return arg1 === arg2;
    });
};

module.exports = streq;