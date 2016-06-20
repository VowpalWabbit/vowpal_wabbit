// --------------------------------------------------------------------------------------------------------------------
// <copyright file="ConfigInspector.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Linq;
using System.Linq.Expressions;
using System.Reflection;
using System.Text.RegularExpressions;

namespace VW.Serializer
{
    /// <summary>
    /// Constructs a marshalling schema based on string based configuration using 
    /// this format: Property.Property.Property(Enumerize = true, Namespace = , ...)
    /// </summary>
    public static class ConfigInspector
    {
        /// <summary>
        /// Format: Property.Property.Property(Enumerize = true, Namespace = , ...)
        /// </summary>
        private static readonly Regex LineRegex = new Regex(@"
                ^\s*
                (?<prop>[^\s.\(]+)       # first property
                (?:\.
                    (?<prop>[^\s.\(]+)
                )*   # other properties
                (\( # first attribute
                    (?<attrname>\w+) \s* = \s* (?<attrvalue>[^,)\s]+)
                    (?:\s*,\s*   # other attributes
                        (?<attrname>\w+) \s* = \s* (?<attrvalue>[^,)\s]+) \s*
                    )*
                \))?
                \s*$
                ",
            RegexOptions.IgnorePatternWhitespace | RegexOptions.Compiled);

        /// <summary>
        /// Creates a <see cref="Schema"/> for the given <paramref name="type"/>.
        /// </summary>
        /// <param name="type">The type to be inspected.</param>
        /// <param name="config">An optional config restricting the properties to be used as features.</param>
        /// <param name="warnings">An otpional callback to signal warnings during inspection.</param>
        /// <returns>Returns a schema describing the feature marshalling.</returns>
        public static Schema CreateSchema(Type type, string config, Action<string> warnings = null)
        {
            if (warnings == null)
                warnings = msg => { };

            var featureExpressionCtor = typeof(FeatureExpression).GetConstructors()[0];

            var featureExpressions = new List<FeatureExpression>();
            var lines = config.Split(new[] { "\r\n", "\n" }, StringSplitOptions.RemoveEmptyEntries)
                .Select(l => l.Trim())
                .Where(l => l.Length > 0);

            foreach (var line in lines)
	        {
                var match = LineRegex.Match(line);
                if (!match.Success)
                {
                    warnings(string.Format("Unable to parse: '{0}'", line));
                    continue;
                }

                var propPath = GetGroupMatch(match.Groups["prop"])
                    .ToList();

                var names = GetGroupMatch(match.Groups["attrname"])
                    .Select(n => n.ToLowerInvariant())
                    .ToList();

                var values = GetGroupMatch(match.Groups["attrvalue"])
                    .ToList();

                if (names.Count != values.Count)
                {
                    warnings(string.Format("Number of attribute names ({0}) and values ({1}) need match. '{2}'",
                        names.Count, values.Count, line));
                    continue;
                }

                var currentType = type;
                bool valid = true;
                var properties = new List<PropertyInfo>();

                foreach (var prop in propPath)
	            {
                    var property = currentType.GetProperty(prop);
                    // ignore, since we can't find
                    if (property == null)
                    {
                        warnings(string.Format("Property '{0}' not found in type: '{1}'. Specification: {2}",
                            prop, currentType, line));

                        valid = false;
                        break;
                    }

                    properties.Add(property);
                
                    currentType = property.PropertyType;
                }

                if (!valid || properties.Count == 0)
                {
                    warnings(string.Format("No properties found on '{0}' specified by '{1}'", type, line));
                    continue;
                }

                Func<Expression, Expression> valueExpression = e => WalkPropertyPath(e, properties);
                var valueValidExpression = new List<Func<Expression, Expression>>();

                for (int i = 1; i < properties.Count - 1; i++)
			    {
			        valueValidExpression.Add(e => 
                        Expression.NotEqual(
                            WalkPropertyPath(e, properties.Take(i)), 
                            Expression.Constant(null)));
			    }

                var arguments = featureExpressionCtor.GetParameters()
                    .Select(p =>
                    {
                        var index = names.IndexOf(p.Name.ToLowerInvariant());
                        if (index == -1)
                            return p.DefaultValue;

                        var value = values[index];
                        if (p.ParameterType == typeof(string))
                            return ParseString(value);

                        if (p.ParameterType == typeof(char?))
                            return ParseChar(value);

                        if (p.ParameterType == typeof(bool))
                            return Convert.ToBoolean(value);

                        if (p.ParameterType == typeof(int?))
                            return Convert.ToInt32(value);

                        warnings(string.Format("Unsupported type '{0}' for parameter '{1}'", p.ParameterType, p.Name));

                        return p.DefaultValue;
                    })
                    .Skip(4)
                    .ToList();

                // name, valueExpressionFactory, valueValidExpressionFactories
                arguments.InsertRange(0, 
                    new object[] 
                    { 
                        properties.Last().PropertyType, 
                        properties.Last().Name, 
                        valueExpression, 
                        valueValidExpression 
                    });

                featureExpressions.Add((FeatureExpression)featureExpressionCtor.Invoke(arguments.ToArray()));
            }

            // TODO: adf?
            // p1.p2(Multi)
            // have a different return type... 
            // run twice. separate schemas for shared and ADF
            //
            // Options from enum to factories
            // new VowpalWabbit<AccessViolationException, BadImageFormatException>

            return new Schema
            {
                Features = featureExpressions
            };
        }

        private static string ParseString(string value)
        {
            // supports "Markus", Markus, "Markus, Markus"
            var match = Regex.Match(value.Trim(), "^\"?([^\"]+)\"?$");
            if (!match.Success)
                throw new ArgumentException("Unable to parse: '" + value + "'");

            return match.Groups[1].Value;
        }

        private static char ParseChar(string value)
        {
            var match = Regex.Match(value.Trim(), "^\'?([^'])'?$");
            if (!match.Success)
                throw new ArgumentException("Unable to parse: '" + value + "'");

            return match.Groups[1].Value[0];
        }

        private static Expression WalkPropertyPath(Expression e, IEnumerable<PropertyInfo> properties)
        {
            var propAccess = Expression.Property(e, properties.First());
            var leftProperties = properties.Skip(1);

            if (leftProperties.Count() == 0)
                return propAccess;

            return WalkPropertyPath(propAccess, leftProperties);
        }

        private static IEnumerable<string> GetGroupMatch(Group group)
        {
            if (group.Success)
            {
                for (int i = 0; i < group.Captures.Count; i++)
                    yield return group.Captures[i].Value;
            }
        }
    }
}
