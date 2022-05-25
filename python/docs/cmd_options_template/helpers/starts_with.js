var starts_with = function () {};

starts_with.register = function (Handlebars) {
    Handlebars.registerHelper('startswith', function(text, value) {
        return text.startsWith(value)
    });
};

module.exports = starts_with;