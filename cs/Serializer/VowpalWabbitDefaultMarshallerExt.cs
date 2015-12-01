// --------------------------------------------------------------------------------------------------------------------
// <copyright file="VowpalWabbitInterfaceVisitorExt.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using System;
using System.Collections;
using System.Collections.Generic;
using System.Diagnostics;
using System.Diagnostics.Contracts;
using System.Linq;
using System.Globalization;
using System.Text;
using VW.Serializer.Intermediate;


namespace VW.Serializer
{
    public partial class VowpalWabbitDefaultMarshaller
    {
				/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
		/// <param name="context">The marshalling context.</param>
		/// <param name="ns">The namespace description.</param>
		/// <param name="feature">The feature description.</param>
		/// <param name="value">The feature value.</param>
        public void MarshalFeature(VowpalWabbitMarshalContext context, Namespace ns, PreHashedFeature feature, System.Byte value)
        {
            Contract.Requires(context != null);
            Contract.Requires(ns != null);
            Contract.Requires(feature != null);

			
			context.NamespaceBuilder.AddFeature(feature.FeatureHash, (float)value);

            context.AppendStringExample(
				feature.Dictify,
                " {0}:" + (context.VW.Settings.EnableStringFloatCompact ? "{1}" : "{1:E20}"),
                feature.Name,
                value);
        }

		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
		/// <param name="context">The marshalling context.</param>
		/// <param name="ns">The namespace description.</param>
		/// <param name="feature">The feature description.</param>
		/// <param name="value">The feature value.</param>
        public unsafe void MarshalFeature(VowpalWabbitMarshalContext context, Namespace ns, PreHashedFeature feature, System.Byte[] value)
        {
            if (value == null)
            {
                return;
            }

            var i = 0;

            // support anchor feature
            if (feature.AddAnchor)
            {
				context.NamespaceBuilder.PreAllocate(value.Length + 1);

                context.NamespaceBuilder.AddFeature(ns.NamespaceHash, 1);
                i++;
            }
			else
			{
				context.NamespaceBuilder.PreAllocate(value.Length);
			}

			
            foreach (var v in value)
            {
				
                context.NamespaceBuilder.AddFeature((uint)(ns.NamespaceHash + i), (float)v);
                i++;
            }
			
            if (context.StringExample == null)
            {
                return;
            }

			string featureString;
			if (feature.Dictify && context.FastDictionary != null)
			{
				if (context.FastDictionary.TryGetValue(value, out featureString))
				{
					context.AppendStringExample(feature.Dictify, featureString);
					return;
				}
			}

			var featureBuilder = new StringBuilder();

            // support anchor feature
            i = 0;
            if (feature.AddAnchor)
            {
                featureBuilder.Append(" 0:1");
                i++;
            }

			var format = " {0}:" + (context.VW.Settings.EnableStringFloatCompact ? "{1}" : "{1:E20}");
            foreach (var v in value)
            {
                featureBuilder.AppendFormat(
					CultureInfo.InvariantCulture, 
					format, 
					i, 
					v);
                i++;
            }

			featureString = featureBuilder.ToString();

			if (feature.Dictify && context.FastDictionary != null)
			{
				context.FastDictionary.Add(value, featureString);
			}
			
			context.AppendStringExample(feature.Dictify, featureString);
        }

				/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
		/// <param name="context">The marshalling context.</param>
		/// <param name="ns">The namespace description.</param>
		/// <param name="feature">The feature description.</param>
		/// <param name="value">The feature value.</param>
        public void MarshalFeature(VowpalWabbitMarshalContext context, Namespace ns, PreHashedFeature feature, System.SByte value)
        {
            Contract.Requires(context != null);
            Contract.Requires(ns != null);
            Contract.Requires(feature != null);

			
			context.NamespaceBuilder.AddFeature(feature.FeatureHash, (float)value);

            context.AppendStringExample(
				feature.Dictify,
                " {0}:" + (context.VW.Settings.EnableStringFloatCompact ? "{1}" : "{1:E20}"),
                feature.Name,
                value);
        }

		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
		/// <param name="context">The marshalling context.</param>
		/// <param name="ns">The namespace description.</param>
		/// <param name="feature">The feature description.</param>
		/// <param name="value">The feature value.</param>
        public unsafe void MarshalFeature(VowpalWabbitMarshalContext context, Namespace ns, PreHashedFeature feature, System.SByte[] value)
        {
            if (value == null)
            {
                return;
            }

            var i = 0;

            // support anchor feature
            if (feature.AddAnchor)
            {
				context.NamespaceBuilder.PreAllocate(value.Length + 1);

                context.NamespaceBuilder.AddFeature(ns.NamespaceHash, 1);
                i++;
            }
			else
			{
				context.NamespaceBuilder.PreAllocate(value.Length);
			}

			
            foreach (var v in value)
            {
				
                context.NamespaceBuilder.AddFeature((uint)(ns.NamespaceHash + i), (float)v);
                i++;
            }
			
            if (context.StringExample == null)
            {
                return;
            }

			string featureString;
			if (feature.Dictify && context.FastDictionary != null)
			{
				if (context.FastDictionary.TryGetValue(value, out featureString))
				{
					context.AppendStringExample(feature.Dictify, featureString);
					return;
				}
			}

			var featureBuilder = new StringBuilder();

            // support anchor feature
            i = 0;
            if (feature.AddAnchor)
            {
                featureBuilder.Append(" 0:1");
                i++;
            }

			var format = " {0}:" + (context.VW.Settings.EnableStringFloatCompact ? "{1}" : "{1:E20}");
            foreach (var v in value)
            {
                featureBuilder.AppendFormat(
					CultureInfo.InvariantCulture, 
					format, 
					i, 
					v);
                i++;
            }

			featureString = featureBuilder.ToString();

			if (feature.Dictify && context.FastDictionary != null)
			{
				context.FastDictionary.Add(value, featureString);
			}
			
			context.AppendStringExample(feature.Dictify, featureString);
        }

				/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
		/// <param name="context">The marshalling context.</param>
		/// <param name="ns">The namespace description.</param>
		/// <param name="feature">The feature description.</param>
		/// <param name="value">The feature value.</param>
        public void MarshalFeature(VowpalWabbitMarshalContext context, Namespace ns, PreHashedFeature feature, System.Int16 value)
        {
            Contract.Requires(context != null);
            Contract.Requires(ns != null);
            Contract.Requires(feature != null);

			
			context.NamespaceBuilder.AddFeature(feature.FeatureHash, (float)value);

            context.AppendStringExample(
				feature.Dictify,
                " {0}:" + (context.VW.Settings.EnableStringFloatCompact ? "{1}" : "{1:E20}"),
                feature.Name,
                value);
        }

		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
		/// <param name="context">The marshalling context.</param>
		/// <param name="ns">The namespace description.</param>
		/// <param name="feature">The feature description.</param>
		/// <param name="value">The feature value.</param>
        public unsafe void MarshalFeature(VowpalWabbitMarshalContext context, Namespace ns, PreHashedFeature feature, System.Int16[] value)
        {
            if (value == null)
            {
                return;
            }

            var i = 0;

            // support anchor feature
            if (feature.AddAnchor)
            {
				context.NamespaceBuilder.PreAllocate(value.Length + 1);

                context.NamespaceBuilder.AddFeature(ns.NamespaceHash, 1);
                i++;
            }
			else
			{
				context.NamespaceBuilder.PreAllocate(value.Length);
			}

			
            foreach (var v in value)
            {
				
                context.NamespaceBuilder.AddFeature((uint)(ns.NamespaceHash + i), (float)v);
                i++;
            }
			
            if (context.StringExample == null)
            {
                return;
            }

			string featureString;
			if (feature.Dictify && context.FastDictionary != null)
			{
				if (context.FastDictionary.TryGetValue(value, out featureString))
				{
					context.AppendStringExample(feature.Dictify, featureString);
					return;
				}
			}

			var featureBuilder = new StringBuilder();

            // support anchor feature
            i = 0;
            if (feature.AddAnchor)
            {
                featureBuilder.Append(" 0:1");
                i++;
            }

			var format = " {0}:" + (context.VW.Settings.EnableStringFloatCompact ? "{1}" : "{1:E20}");
            foreach (var v in value)
            {
                featureBuilder.AppendFormat(
					CultureInfo.InvariantCulture, 
					format, 
					i, 
					v);
                i++;
            }

			featureString = featureBuilder.ToString();

			if (feature.Dictify && context.FastDictionary != null)
			{
				context.FastDictionary.Add(value, featureString);
			}
			
			context.AppendStringExample(feature.Dictify, featureString);
        }

				/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
		/// <param name="context">The marshalling context.</param>
		/// <param name="ns">The namespace description.</param>
		/// <param name="feature">The feature description.</param>
		/// <param name="value">The feature value.</param>
        public void MarshalFeature(VowpalWabbitMarshalContext context, Namespace ns, PreHashedFeature feature, System.Int32 value)
        {
            Contract.Requires(context != null);
            Contract.Requires(ns != null);
            Contract.Requires(feature != null);

			
			context.NamespaceBuilder.AddFeature(feature.FeatureHash, (float)value);

            context.AppendStringExample(
				feature.Dictify,
                " {0}:" + (context.VW.Settings.EnableStringFloatCompact ? "{1}" : "{1:E20}"),
                feature.Name,
                value);
        }

		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
		/// <param name="context">The marshalling context.</param>
		/// <param name="ns">The namespace description.</param>
		/// <param name="feature">The feature description.</param>
		/// <param name="value">The feature value.</param>
        public unsafe void MarshalFeature(VowpalWabbitMarshalContext context, Namespace ns, PreHashedFeature feature, System.Int32[] value)
        {
            if (value == null)
            {
                return;
            }

            var i = 0;

            // support anchor feature
            if (feature.AddAnchor)
            {
				context.NamespaceBuilder.PreAllocate(value.Length + 1);

                context.NamespaceBuilder.AddFeature(ns.NamespaceHash, 1);
                i++;
            }
			else
			{
				context.NamespaceBuilder.PreAllocate(value.Length);
			}

			
            foreach (var v in value)
            {
				
                context.NamespaceBuilder.AddFeature((uint)(ns.NamespaceHash + i), (float)v);
                i++;
            }
			
            if (context.StringExample == null)
            {
                return;
            }

			string featureString;
			if (feature.Dictify && context.FastDictionary != null)
			{
				if (context.FastDictionary.TryGetValue(value, out featureString))
				{
					context.AppendStringExample(feature.Dictify, featureString);
					return;
				}
			}

			var featureBuilder = new StringBuilder();

            // support anchor feature
            i = 0;
            if (feature.AddAnchor)
            {
                featureBuilder.Append(" 0:1");
                i++;
            }

			var format = " {0}:" + (context.VW.Settings.EnableStringFloatCompact ? "{1}" : "{1:E20}");
            foreach (var v in value)
            {
                featureBuilder.AppendFormat(
					CultureInfo.InvariantCulture, 
					format, 
					i, 
					v);
                i++;
            }

			featureString = featureBuilder.ToString();

			if (feature.Dictify && context.FastDictionary != null)
			{
				context.FastDictionary.Add(value, featureString);
			}
			
			context.AppendStringExample(feature.Dictify, featureString);
        }

				/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
		/// <param name="context">The marshalling context.</param>
		/// <param name="ns">The namespace description.</param>
		/// <param name="feature">The feature description.</param>
		/// <param name="value">The feature value.</param>
        public void MarshalFeature(VowpalWabbitMarshalContext context, Namespace ns, PreHashedFeature feature, System.UInt16 value)
        {
            Contract.Requires(context != null);
            Contract.Requires(ns != null);
            Contract.Requires(feature != null);

			
			context.NamespaceBuilder.AddFeature(feature.FeatureHash, (float)value);

            context.AppendStringExample(
				feature.Dictify,
                " {0}:" + (context.VW.Settings.EnableStringFloatCompact ? "{1}" : "{1:E20}"),
                feature.Name,
                value);
        }

		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
		/// <param name="context">The marshalling context.</param>
		/// <param name="ns">The namespace description.</param>
		/// <param name="feature">The feature description.</param>
		/// <param name="value">The feature value.</param>
        public unsafe void MarshalFeature(VowpalWabbitMarshalContext context, Namespace ns, PreHashedFeature feature, System.UInt16[] value)
        {
            if (value == null)
            {
                return;
            }

            var i = 0;

            // support anchor feature
            if (feature.AddAnchor)
            {
				context.NamespaceBuilder.PreAllocate(value.Length + 1);

                context.NamespaceBuilder.AddFeature(ns.NamespaceHash, 1);
                i++;
            }
			else
			{
				context.NamespaceBuilder.PreAllocate(value.Length);
			}

			
            foreach (var v in value)
            {
				
                context.NamespaceBuilder.AddFeature((uint)(ns.NamespaceHash + i), (float)v);
                i++;
            }
			
            if (context.StringExample == null)
            {
                return;
            }

			string featureString;
			if (feature.Dictify && context.FastDictionary != null)
			{
				if (context.FastDictionary.TryGetValue(value, out featureString))
				{
					context.AppendStringExample(feature.Dictify, featureString);
					return;
				}
			}

			var featureBuilder = new StringBuilder();

            // support anchor feature
            i = 0;
            if (feature.AddAnchor)
            {
                featureBuilder.Append(" 0:1");
                i++;
            }

			var format = " {0}:" + (context.VW.Settings.EnableStringFloatCompact ? "{1}" : "{1:E20}");
            foreach (var v in value)
            {
                featureBuilder.AppendFormat(
					CultureInfo.InvariantCulture, 
					format, 
					i, 
					v);
                i++;
            }

			featureString = featureBuilder.ToString();

			if (feature.Dictify && context.FastDictionary != null)
			{
				context.FastDictionary.Add(value, featureString);
			}
			
			context.AppendStringExample(feature.Dictify, featureString);
        }

				/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
		/// <param name="context">The marshalling context.</param>
		/// <param name="ns">The namespace description.</param>
		/// <param name="feature">The feature description.</param>
		/// <param name="value">The feature value.</param>
        public void MarshalFeature(VowpalWabbitMarshalContext context, Namespace ns, PreHashedFeature feature, System.UInt32 value)
        {
            Contract.Requires(context != null);
            Contract.Requires(ns != null);
            Contract.Requires(feature != null);

			
			context.NamespaceBuilder.AddFeature(feature.FeatureHash, (float)value);

            context.AppendStringExample(
				feature.Dictify,
                " {0}:" + (context.VW.Settings.EnableStringFloatCompact ? "{1}" : "{1:E20}"),
                feature.Name,
                value);
        }

		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
		/// <param name="context">The marshalling context.</param>
		/// <param name="ns">The namespace description.</param>
		/// <param name="feature">The feature description.</param>
		/// <param name="value">The feature value.</param>
        public unsafe void MarshalFeature(VowpalWabbitMarshalContext context, Namespace ns, PreHashedFeature feature, System.UInt32[] value)
        {
            if (value == null)
            {
                return;
            }

            var i = 0;

            // support anchor feature
            if (feature.AddAnchor)
            {
				context.NamespaceBuilder.PreAllocate(value.Length + 1);

                context.NamespaceBuilder.AddFeature(ns.NamespaceHash, 1);
                i++;
            }
			else
			{
				context.NamespaceBuilder.PreAllocate(value.Length);
			}

			
            foreach (var v in value)
            {
				
                context.NamespaceBuilder.AddFeature((uint)(ns.NamespaceHash + i), (float)v);
                i++;
            }
			
            if (context.StringExample == null)
            {
                return;
            }

			string featureString;
			if (feature.Dictify && context.FastDictionary != null)
			{
				if (context.FastDictionary.TryGetValue(value, out featureString))
				{
					context.AppendStringExample(feature.Dictify, featureString);
					return;
				}
			}

			var featureBuilder = new StringBuilder();

            // support anchor feature
            i = 0;
            if (feature.AddAnchor)
            {
                featureBuilder.Append(" 0:1");
                i++;
            }

			var format = " {0}:" + (context.VW.Settings.EnableStringFloatCompact ? "{1}" : "{1:E20}");
            foreach (var v in value)
            {
                featureBuilder.AppendFormat(
					CultureInfo.InvariantCulture, 
					format, 
					i, 
					v);
                i++;
            }

			featureString = featureBuilder.ToString();

			if (feature.Dictify && context.FastDictionary != null)
			{
				context.FastDictionary.Add(value, featureString);
			}
			
			context.AppendStringExample(feature.Dictify, featureString);
        }

				/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
		/// <param name="context">The marshalling context.</param>
		/// <param name="ns">The namespace description.</param>
		/// <param name="feature">The feature description.</param>
		/// <param name="value">The feature value.</param>
        public void MarshalFeature(VowpalWabbitMarshalContext context, Namespace ns, PreHashedFeature feature, System.Single value)
        {
            Contract.Requires(context != null);
            Contract.Requires(ns != null);
            Contract.Requires(feature != null);

			
			context.NamespaceBuilder.AddFeature(feature.FeatureHash, (float)value);

            context.AppendStringExample(
				feature.Dictify,
                " {0}:" + (context.VW.Settings.EnableStringFloatCompact ? "{1}" : "{1:E20}"),
                feature.Name,
                value);
        }

		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
		/// <param name="context">The marshalling context.</param>
		/// <param name="ns">The namespace description.</param>
		/// <param name="feature">The feature description.</param>
		/// <param name="value">The feature value.</param>
        public unsafe void MarshalFeature(VowpalWabbitMarshalContext context, Namespace ns, PreHashedFeature feature, System.Single[] value)
        {
            if (value == null)
            {
                return;
            }

            var i = 0;

            // support anchor feature
            if (feature.AddAnchor)
            {
				context.NamespaceBuilder.PreAllocate(value.Length + 1);

                context.NamespaceBuilder.AddFeature(ns.NamespaceHash, 1);
                i++;
            }
			else
			{
				context.NamespaceBuilder.PreAllocate(value.Length);
			}

			
			fixed (float* begin = value)
			{
				context.NamespaceBuilder.AddFeaturesUnchecked((uint)(ns.NamespaceHash + i), begin, begin + value.Length);
			}

			
            if (context.StringExample == null)
            {
                return;
            }

			string featureString;
			if (feature.Dictify && context.FastDictionary != null)
			{
				if (context.FastDictionary.TryGetValue(value, out featureString))
				{
					context.AppendStringExample(feature.Dictify, featureString);
					return;
				}
			}

			var featureBuilder = new StringBuilder();

            // support anchor feature
            i = 0;
            if (feature.AddAnchor)
            {
                featureBuilder.Append(" 0:1");
                i++;
            }

			var format = " {0}:" + (context.VW.Settings.EnableStringFloatCompact ? "{1}" : "{1:E20}");
            foreach (var v in value)
            {
                featureBuilder.AppendFormat(
					CultureInfo.InvariantCulture, 
					format, 
					i, 
					v);
                i++;
            }

			featureString = featureBuilder.ToString();

			if (feature.Dictify && context.FastDictionary != null)
			{
				context.FastDictionary.Add(value, featureString);
			}
			
			context.AppendStringExample(feature.Dictify, featureString);
        }

				/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
		/// <param name="context">The marshalling context.</param>
		/// <param name="ns">The namespace description.</param>
		/// <param name="feature">The feature description.</param>
		/// <param name="value">The feature value.</param>
        public void MarshalFeature(VowpalWabbitMarshalContext context, Namespace ns, PreHashedFeature feature, System.Int64 value)
        {
            Contract.Requires(context != null);
            Contract.Requires(ns != null);
            Contract.Requires(feature != null);

						#if DEBUG
            if (value > float.MaxValue || value < float.MinValue)
            {
                Trace.TraceWarning("Precision lost for feature value: " + value);
            }
			#endif
			
			context.NamespaceBuilder.AddFeature(feature.FeatureHash, (float)value);

            context.AppendStringExample(
				feature.Dictify,
                " {0}:" + (context.VW.Settings.EnableStringFloatCompact ? "{1}" : "{1:E20}"),
                feature.Name,
                value);
        }

		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
		/// <param name="context">The marshalling context.</param>
		/// <param name="ns">The namespace description.</param>
		/// <param name="feature">The feature description.</param>
		/// <param name="value">The feature value.</param>
        public unsafe void MarshalFeature(VowpalWabbitMarshalContext context, Namespace ns, PreHashedFeature feature, System.Int64[] value)
        {
            if (value == null)
            {
                return;
            }

            var i = 0;

            // support anchor feature
            if (feature.AddAnchor)
            {
				context.NamespaceBuilder.PreAllocate(value.Length + 1);

                context.NamespaceBuilder.AddFeature(ns.NamespaceHash, 1);
                i++;
            }
			else
			{
				context.NamespaceBuilder.PreAllocate(value.Length);
			}

			
            foreach (var v in value)
            {
								#if DEBUG
				if (v > float.MaxValue || v < float.MinValue)
				{
					Trace.TraceWarning("Precision lost for feature value: " + v);
				}
				#endif
				
                context.NamespaceBuilder.AddFeature((uint)(ns.NamespaceHash + i), (float)v);
                i++;
            }
			
            if (context.StringExample == null)
            {
                return;
            }

			string featureString;
			if (feature.Dictify && context.FastDictionary != null)
			{
				if (context.FastDictionary.TryGetValue(value, out featureString))
				{
					context.AppendStringExample(feature.Dictify, featureString);
					return;
				}
			}

			var featureBuilder = new StringBuilder();

            // support anchor feature
            i = 0;
            if (feature.AddAnchor)
            {
                featureBuilder.Append(" 0:1");
                i++;
            }

			var format = " {0}:" + (context.VW.Settings.EnableStringFloatCompact ? "{1}" : "{1:E20}");
            foreach (var v in value)
            {
                featureBuilder.AppendFormat(
					CultureInfo.InvariantCulture, 
					format, 
					i, 
					v);
                i++;
            }

			featureString = featureBuilder.ToString();

			if (feature.Dictify && context.FastDictionary != null)
			{
				context.FastDictionary.Add(value, featureString);
			}
			
			context.AppendStringExample(feature.Dictify, featureString);
        }

				/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
		/// <param name="context">The marshalling context.</param>
		/// <param name="ns">The namespace description.</param>
		/// <param name="feature">The feature description.</param>
		/// <param name="value">The feature value.</param>
        public void MarshalFeature(VowpalWabbitMarshalContext context, Namespace ns, PreHashedFeature feature, System.UInt64 value)
        {
            Contract.Requires(context != null);
            Contract.Requires(ns != null);
            Contract.Requires(feature != null);

						#if DEBUG
            if (value > float.MaxValue || value < float.MinValue)
            {
                Trace.TraceWarning("Precision lost for feature value: " + value);
            }
			#endif
			
			context.NamespaceBuilder.AddFeature(feature.FeatureHash, (float)value);

            context.AppendStringExample(
				feature.Dictify,
                " {0}:" + (context.VW.Settings.EnableStringFloatCompact ? "{1}" : "{1:E20}"),
                feature.Name,
                value);
        }

		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
		/// <param name="context">The marshalling context.</param>
		/// <param name="ns">The namespace description.</param>
		/// <param name="feature">The feature description.</param>
		/// <param name="value">The feature value.</param>
        public unsafe void MarshalFeature(VowpalWabbitMarshalContext context, Namespace ns, PreHashedFeature feature, System.UInt64[] value)
        {
            if (value == null)
            {
                return;
            }

            var i = 0;

            // support anchor feature
            if (feature.AddAnchor)
            {
				context.NamespaceBuilder.PreAllocate(value.Length + 1);

                context.NamespaceBuilder.AddFeature(ns.NamespaceHash, 1);
                i++;
            }
			else
			{
				context.NamespaceBuilder.PreAllocate(value.Length);
			}

			
            foreach (var v in value)
            {
								#if DEBUG
				if (v > float.MaxValue || v < float.MinValue)
				{
					Trace.TraceWarning("Precision lost for feature value: " + v);
				}
				#endif
				
                context.NamespaceBuilder.AddFeature((uint)(ns.NamespaceHash + i), (float)v);
                i++;
            }
			
            if (context.StringExample == null)
            {
                return;
            }

			string featureString;
			if (feature.Dictify && context.FastDictionary != null)
			{
				if (context.FastDictionary.TryGetValue(value, out featureString))
				{
					context.AppendStringExample(feature.Dictify, featureString);
					return;
				}
			}

			var featureBuilder = new StringBuilder();

            // support anchor feature
            i = 0;
            if (feature.AddAnchor)
            {
                featureBuilder.Append(" 0:1");
                i++;
            }

			var format = " {0}:" + (context.VW.Settings.EnableStringFloatCompact ? "{1}" : "{1:E20}");
            foreach (var v in value)
            {
                featureBuilder.AppendFormat(
					CultureInfo.InvariantCulture, 
					format, 
					i, 
					v);
                i++;
            }

			featureString = featureBuilder.ToString();

			if (feature.Dictify && context.FastDictionary != null)
			{
				context.FastDictionary.Add(value, featureString);
			}
			
			context.AppendStringExample(feature.Dictify, featureString);
        }

				/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
		/// <param name="context">The marshalling context.</param>
		/// <param name="ns">The namespace description.</param>
		/// <param name="feature">The feature description.</param>
		/// <param name="value">The feature value.</param>
        public void MarshalFeature(VowpalWabbitMarshalContext context, Namespace ns, PreHashedFeature feature, System.Double value)
        {
            Contract.Requires(context != null);
            Contract.Requires(ns != null);
            Contract.Requires(feature != null);

						#if DEBUG
            if (value > float.MaxValue || value < float.MinValue)
            {
                Trace.TraceWarning("Precision lost for feature value: " + value);
            }
			#endif
			
			context.NamespaceBuilder.AddFeature(feature.FeatureHash, (float)value);

            context.AppendStringExample(
				feature.Dictify,
                " {0}:" + (context.VW.Settings.EnableStringFloatCompact ? "{1}" : "{1:E20}"),
                feature.Name,
                value);
        }

		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
		/// <param name="context">The marshalling context.</param>
		/// <param name="ns">The namespace description.</param>
		/// <param name="feature">The feature description.</param>
		/// <param name="value">The feature value.</param>
        public unsafe void MarshalFeature(VowpalWabbitMarshalContext context, Namespace ns, PreHashedFeature feature, System.Double[] value)
        {
            if (value == null)
            {
                return;
            }

            var i = 0;

            // support anchor feature
            if (feature.AddAnchor)
            {
				context.NamespaceBuilder.PreAllocate(value.Length + 1);

                context.NamespaceBuilder.AddFeature(ns.NamespaceHash, 1);
                i++;
            }
			else
			{
				context.NamespaceBuilder.PreAllocate(value.Length);
			}

			
            foreach (var v in value)
            {
								#if DEBUG
				if (v > float.MaxValue || v < float.MinValue)
				{
					Trace.TraceWarning("Precision lost for feature value: " + v);
				}
				#endif
				
                context.NamespaceBuilder.AddFeature((uint)(ns.NamespaceHash + i), (float)v);
                i++;
            }
			
            if (context.StringExample == null)
            {
                return;
            }

			string featureString;
			if (feature.Dictify && context.FastDictionary != null)
			{
				if (context.FastDictionary.TryGetValue(value, out featureString))
				{
					context.AppendStringExample(feature.Dictify, featureString);
					return;
				}
			}

			var featureBuilder = new StringBuilder();

            // support anchor feature
            i = 0;
            if (feature.AddAnchor)
            {
                featureBuilder.Append(" 0:1");
                i++;
            }

			var format = " {0}:" + (context.VW.Settings.EnableStringFloatCompact ? "{1}" : "{1:E20}");
            foreach (var v in value)
            {
                featureBuilder.AppendFormat(
					CultureInfo.InvariantCulture, 
					format, 
					i, 
					v);
                i++;
            }

			featureString = featureBuilder.ToString();

			if (feature.Dictify && context.FastDictionary != null)
			{
				context.FastDictionary.Add(value, featureString);
			}
			
			context.AppendStringExample(feature.Dictify, featureString);
        }

		
		
		
		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
		/// <param name="context">The marshalling context.</param>
		/// <param name="ns">The namespace description.</param>
		/// <param name="feature">The feature description.</param>
		/// <param name="value">The feature value.</param>
		[ContractVerification(false)]
        public void MarshalFeature(VowpalWabbitMarshalContext context, Namespace ns, Feature feature, IDictionary<System.Byte, System.Byte> value)
        {
            Contract.Requires(context != null);
            Contract.Requires(ns != null);
            Contract.Requires(feature != null);

			if (value == null)
			{
				return;
			}

            foreach (var kvp in value)
            {
				
				context.NamespaceBuilder.AddFeature((uint)(ns.NamespaceHash + kvp.Key), (float)kvp.Value);
            }

			if (context.StringExample == null)
            {
                return;
            }

			var format = " {0}:" + (context.VW.Settings.EnableStringFloatCompact ? "{1}" : "{1:E20}");
			foreach (var kvp in value)
            {
				// TODO: not sure if negative numbers will work
                context.AppendStringExample(
					feature.Dictify,
                    format,
                    kvp.Key,
                    kvp.Value);
			}
        }
		
		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
		/// <param name="context">The marshalling context.</param>
		/// <param name="ns">The namespace description.</param>
		/// <param name="feature">The feature description.</param>
		/// <param name="value">The feature value.</param>
		[ContractVerification(false)]
        public void MarshalFeature(VowpalWabbitMarshalContext context, Namespace ns, Feature feature, IDictionary<System.Byte, System.SByte> value)
        {
            Contract.Requires(context != null);
            Contract.Requires(ns != null);
            Contract.Requires(feature != null);

			if (value == null)
			{
				return;
			}

            foreach (var kvp in value)
            {
				
				context.NamespaceBuilder.AddFeature((uint)(ns.NamespaceHash + kvp.Key), (float)kvp.Value);
            }

			if (context.StringExample == null)
            {
                return;
            }

			var format = " {0}:" + (context.VW.Settings.EnableStringFloatCompact ? "{1}" : "{1:E20}");
			foreach (var kvp in value)
            {
				// TODO: not sure if negative numbers will work
                context.AppendStringExample(
					feature.Dictify,
                    format,
                    kvp.Key,
                    kvp.Value);
			}
        }
		
		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
		/// <param name="context">The marshalling context.</param>
		/// <param name="ns">The namespace description.</param>
		/// <param name="feature">The feature description.</param>
		/// <param name="value">The feature value.</param>
		[ContractVerification(false)]
        public void MarshalFeature(VowpalWabbitMarshalContext context, Namespace ns, Feature feature, IDictionary<System.Byte, System.Int16> value)
        {
            Contract.Requires(context != null);
            Contract.Requires(ns != null);
            Contract.Requires(feature != null);

			if (value == null)
			{
				return;
			}

            foreach (var kvp in value)
            {
				
				context.NamespaceBuilder.AddFeature((uint)(ns.NamespaceHash + kvp.Key), (float)kvp.Value);
            }

			if (context.StringExample == null)
            {
                return;
            }

			var format = " {0}:" + (context.VW.Settings.EnableStringFloatCompact ? "{1}" : "{1:E20}");
			foreach (var kvp in value)
            {
				// TODO: not sure if negative numbers will work
                context.AppendStringExample(
					feature.Dictify,
                    format,
                    kvp.Key,
                    kvp.Value);
			}
        }
		
		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
		/// <param name="context">The marshalling context.</param>
		/// <param name="ns">The namespace description.</param>
		/// <param name="feature">The feature description.</param>
		/// <param name="value">The feature value.</param>
		[ContractVerification(false)]
        public void MarshalFeature(VowpalWabbitMarshalContext context, Namespace ns, Feature feature, IDictionary<System.Byte, System.Int32> value)
        {
            Contract.Requires(context != null);
            Contract.Requires(ns != null);
            Contract.Requires(feature != null);

			if (value == null)
			{
				return;
			}

            foreach (var kvp in value)
            {
				
				context.NamespaceBuilder.AddFeature((uint)(ns.NamespaceHash + kvp.Key), (float)kvp.Value);
            }

			if (context.StringExample == null)
            {
                return;
            }

			var format = " {0}:" + (context.VW.Settings.EnableStringFloatCompact ? "{1}" : "{1:E20}");
			foreach (var kvp in value)
            {
				// TODO: not sure if negative numbers will work
                context.AppendStringExample(
					feature.Dictify,
                    format,
                    kvp.Key,
                    kvp.Value);
			}
        }
		
		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
		/// <param name="context">The marshalling context.</param>
		/// <param name="ns">The namespace description.</param>
		/// <param name="feature">The feature description.</param>
		/// <param name="value">The feature value.</param>
		[ContractVerification(false)]
        public void MarshalFeature(VowpalWabbitMarshalContext context, Namespace ns, Feature feature, IDictionary<System.Byte, System.UInt16> value)
        {
            Contract.Requires(context != null);
            Contract.Requires(ns != null);
            Contract.Requires(feature != null);

			if (value == null)
			{
				return;
			}

            foreach (var kvp in value)
            {
				
				context.NamespaceBuilder.AddFeature((uint)(ns.NamespaceHash + kvp.Key), (float)kvp.Value);
            }

			if (context.StringExample == null)
            {
                return;
            }

			var format = " {0}:" + (context.VW.Settings.EnableStringFloatCompact ? "{1}" : "{1:E20}");
			foreach (var kvp in value)
            {
				// TODO: not sure if negative numbers will work
                context.AppendStringExample(
					feature.Dictify,
                    format,
                    kvp.Key,
                    kvp.Value);
			}
        }
		
		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
		/// <param name="context">The marshalling context.</param>
		/// <param name="ns">The namespace description.</param>
		/// <param name="feature">The feature description.</param>
		/// <param name="value">The feature value.</param>
		[ContractVerification(false)]
        public void MarshalFeature(VowpalWabbitMarshalContext context, Namespace ns, Feature feature, IDictionary<System.Byte, System.UInt32> value)
        {
            Contract.Requires(context != null);
            Contract.Requires(ns != null);
            Contract.Requires(feature != null);

			if (value == null)
			{
				return;
			}

            foreach (var kvp in value)
            {
				
				context.NamespaceBuilder.AddFeature((uint)(ns.NamespaceHash + kvp.Key), (float)kvp.Value);
            }

			if (context.StringExample == null)
            {
                return;
            }

			var format = " {0}:" + (context.VW.Settings.EnableStringFloatCompact ? "{1}" : "{1:E20}");
			foreach (var kvp in value)
            {
				// TODO: not sure if negative numbers will work
                context.AppendStringExample(
					feature.Dictify,
                    format,
                    kvp.Key,
                    kvp.Value);
			}
        }
		
		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
		/// <param name="context">The marshalling context.</param>
		/// <param name="ns">The namespace description.</param>
		/// <param name="feature">The feature description.</param>
		/// <param name="value">The feature value.</param>
		[ContractVerification(false)]
        public void MarshalFeature(VowpalWabbitMarshalContext context, Namespace ns, Feature feature, IDictionary<System.Byte, System.Single> value)
        {
            Contract.Requires(context != null);
            Contract.Requires(ns != null);
            Contract.Requires(feature != null);

			if (value == null)
			{
				return;
			}

            foreach (var kvp in value)
            {
				
				context.NamespaceBuilder.AddFeature((uint)(ns.NamespaceHash + kvp.Key), (float)kvp.Value);
            }

			if (context.StringExample == null)
            {
                return;
            }

			var format = " {0}:" + (context.VW.Settings.EnableStringFloatCompact ? "{1}" : "{1:E20}");
			foreach (var kvp in value)
            {
				// TODO: not sure if negative numbers will work
                context.AppendStringExample(
					feature.Dictify,
                    format,
                    kvp.Key,
                    kvp.Value);
			}
        }
		
		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
		/// <param name="context">The marshalling context.</param>
		/// <param name="ns">The namespace description.</param>
		/// <param name="feature">The feature description.</param>
		/// <param name="value">The feature value.</param>
		[ContractVerification(false)]
        public void MarshalFeature(VowpalWabbitMarshalContext context, Namespace ns, Feature feature, IDictionary<System.Byte, System.Int64> value)
        {
            Contract.Requires(context != null);
            Contract.Requires(ns != null);
            Contract.Requires(feature != null);

			if (value == null)
			{
				return;
			}

            foreach (var kvp in value)
            {
				
				context.NamespaceBuilder.AddFeature((uint)(ns.NamespaceHash + kvp.Key), (float)kvp.Value);
            }

			if (context.StringExample == null)
            {
                return;
            }

			var format = " {0}:" + (context.VW.Settings.EnableStringFloatCompact ? "{1}" : "{1:E20}");
			foreach (var kvp in value)
            {
				// TODO: not sure if negative numbers will work
                context.AppendStringExample(
					feature.Dictify,
                    format,
                    kvp.Key,
                    kvp.Value);
			}
        }
		
		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
		/// <param name="context">The marshalling context.</param>
		/// <param name="ns">The namespace description.</param>
		/// <param name="feature">The feature description.</param>
		/// <param name="value">The feature value.</param>
		[ContractVerification(false)]
        public void MarshalFeature(VowpalWabbitMarshalContext context, Namespace ns, Feature feature, IDictionary<System.Byte, System.UInt64> value)
        {
            Contract.Requires(context != null);
            Contract.Requires(ns != null);
            Contract.Requires(feature != null);

			if (value == null)
			{
				return;
			}

            foreach (var kvp in value)
            {
				
				context.NamespaceBuilder.AddFeature((uint)(ns.NamespaceHash + kvp.Key), (float)kvp.Value);
            }

			if (context.StringExample == null)
            {
                return;
            }

			var format = " {0}:" + (context.VW.Settings.EnableStringFloatCompact ? "{1}" : "{1:E20}");
			foreach (var kvp in value)
            {
				// TODO: not sure if negative numbers will work
                context.AppendStringExample(
					feature.Dictify,
                    format,
                    kvp.Key,
                    kvp.Value);
			}
        }
		
		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
		/// <param name="context">The marshalling context.</param>
		/// <param name="ns">The namespace description.</param>
		/// <param name="feature">The feature description.</param>
		/// <param name="value">The feature value.</param>
		[ContractVerification(false)]
        public void MarshalFeature(VowpalWabbitMarshalContext context, Namespace ns, Feature feature, IDictionary<System.Byte, System.Double> value)
        {
            Contract.Requires(context != null);
            Contract.Requires(ns != null);
            Contract.Requires(feature != null);

			if (value == null)
			{
				return;
			}

            foreach (var kvp in value)
            {
				
				context.NamespaceBuilder.AddFeature((uint)(ns.NamespaceHash + kvp.Key), (float)kvp.Value);
            }

			if (context.StringExample == null)
            {
                return;
            }

			var format = " {0}:" + (context.VW.Settings.EnableStringFloatCompact ? "{1}" : "{1:E20}");
			foreach (var kvp in value)
            {
				// TODO: not sure if negative numbers will work
                context.AppendStringExample(
					feature.Dictify,
                    format,
                    kvp.Key,
                    kvp.Value);
			}
        }
		
		
		
		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
		/// <param name="context">The marshalling context.</param>
		/// <param name="ns">The namespace description.</param>
		/// <param name="feature">The feature description.</param>
		/// <param name="value">The feature value.</param>
		[ContractVerification(false)]
        public void MarshalFeature(VowpalWabbitMarshalContext context, Namespace ns, Feature feature, IDictionary<System.SByte, System.Byte> value)
        {
            Contract.Requires(context != null);
            Contract.Requires(ns != null);
            Contract.Requires(feature != null);

			if (value == null)
			{
				return;
			}

            foreach (var kvp in value)
            {
				
				context.NamespaceBuilder.AddFeature((uint)(ns.NamespaceHash + kvp.Key), (float)kvp.Value);
            }

			if (context.StringExample == null)
            {
                return;
            }

			var format = " {0}:" + (context.VW.Settings.EnableStringFloatCompact ? "{1}" : "{1:E20}");
			foreach (var kvp in value)
            {
				// TODO: not sure if negative numbers will work
                context.AppendStringExample(
					feature.Dictify,
                    format,
                    kvp.Key,
                    kvp.Value);
			}
        }
		
		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
		/// <param name="context">The marshalling context.</param>
		/// <param name="ns">The namespace description.</param>
		/// <param name="feature">The feature description.</param>
		/// <param name="value">The feature value.</param>
		[ContractVerification(false)]
        public void MarshalFeature(VowpalWabbitMarshalContext context, Namespace ns, Feature feature, IDictionary<System.SByte, System.SByte> value)
        {
            Contract.Requires(context != null);
            Contract.Requires(ns != null);
            Contract.Requires(feature != null);

			if (value == null)
			{
				return;
			}

            foreach (var kvp in value)
            {
				
				context.NamespaceBuilder.AddFeature((uint)(ns.NamespaceHash + kvp.Key), (float)kvp.Value);
            }

			if (context.StringExample == null)
            {
                return;
            }

			var format = " {0}:" + (context.VW.Settings.EnableStringFloatCompact ? "{1}" : "{1:E20}");
			foreach (var kvp in value)
            {
				// TODO: not sure if negative numbers will work
                context.AppendStringExample(
					feature.Dictify,
                    format,
                    kvp.Key,
                    kvp.Value);
			}
        }
		
		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
		/// <param name="context">The marshalling context.</param>
		/// <param name="ns">The namespace description.</param>
		/// <param name="feature">The feature description.</param>
		/// <param name="value">The feature value.</param>
		[ContractVerification(false)]
        public void MarshalFeature(VowpalWabbitMarshalContext context, Namespace ns, Feature feature, IDictionary<System.SByte, System.Int16> value)
        {
            Contract.Requires(context != null);
            Contract.Requires(ns != null);
            Contract.Requires(feature != null);

			if (value == null)
			{
				return;
			}

            foreach (var kvp in value)
            {
				
				context.NamespaceBuilder.AddFeature((uint)(ns.NamespaceHash + kvp.Key), (float)kvp.Value);
            }

			if (context.StringExample == null)
            {
                return;
            }

			var format = " {0}:" + (context.VW.Settings.EnableStringFloatCompact ? "{1}" : "{1:E20}");
			foreach (var kvp in value)
            {
				// TODO: not sure if negative numbers will work
                context.AppendStringExample(
					feature.Dictify,
                    format,
                    kvp.Key,
                    kvp.Value);
			}
        }
		
		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
		/// <param name="context">The marshalling context.</param>
		/// <param name="ns">The namespace description.</param>
		/// <param name="feature">The feature description.</param>
		/// <param name="value">The feature value.</param>
		[ContractVerification(false)]
        public void MarshalFeature(VowpalWabbitMarshalContext context, Namespace ns, Feature feature, IDictionary<System.SByte, System.Int32> value)
        {
            Contract.Requires(context != null);
            Contract.Requires(ns != null);
            Contract.Requires(feature != null);

			if (value == null)
			{
				return;
			}

            foreach (var kvp in value)
            {
				
				context.NamespaceBuilder.AddFeature((uint)(ns.NamespaceHash + kvp.Key), (float)kvp.Value);
            }

			if (context.StringExample == null)
            {
                return;
            }

			var format = " {0}:" + (context.VW.Settings.EnableStringFloatCompact ? "{1}" : "{1:E20}");
			foreach (var kvp in value)
            {
				// TODO: not sure if negative numbers will work
                context.AppendStringExample(
					feature.Dictify,
                    format,
                    kvp.Key,
                    kvp.Value);
			}
        }
		
		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
		/// <param name="context">The marshalling context.</param>
		/// <param name="ns">The namespace description.</param>
		/// <param name="feature">The feature description.</param>
		/// <param name="value">The feature value.</param>
		[ContractVerification(false)]
        public void MarshalFeature(VowpalWabbitMarshalContext context, Namespace ns, Feature feature, IDictionary<System.SByte, System.UInt16> value)
        {
            Contract.Requires(context != null);
            Contract.Requires(ns != null);
            Contract.Requires(feature != null);

			if (value == null)
			{
				return;
			}

            foreach (var kvp in value)
            {
				
				context.NamespaceBuilder.AddFeature((uint)(ns.NamespaceHash + kvp.Key), (float)kvp.Value);
            }

			if (context.StringExample == null)
            {
                return;
            }

			var format = " {0}:" + (context.VW.Settings.EnableStringFloatCompact ? "{1}" : "{1:E20}");
			foreach (var kvp in value)
            {
				// TODO: not sure if negative numbers will work
                context.AppendStringExample(
					feature.Dictify,
                    format,
                    kvp.Key,
                    kvp.Value);
			}
        }
		
		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
		/// <param name="context">The marshalling context.</param>
		/// <param name="ns">The namespace description.</param>
		/// <param name="feature">The feature description.</param>
		/// <param name="value">The feature value.</param>
		[ContractVerification(false)]
        public void MarshalFeature(VowpalWabbitMarshalContext context, Namespace ns, Feature feature, IDictionary<System.SByte, System.UInt32> value)
        {
            Contract.Requires(context != null);
            Contract.Requires(ns != null);
            Contract.Requires(feature != null);

			if (value == null)
			{
				return;
			}

            foreach (var kvp in value)
            {
				
				context.NamespaceBuilder.AddFeature((uint)(ns.NamespaceHash + kvp.Key), (float)kvp.Value);
            }

			if (context.StringExample == null)
            {
                return;
            }

			var format = " {0}:" + (context.VW.Settings.EnableStringFloatCompact ? "{1}" : "{1:E20}");
			foreach (var kvp in value)
            {
				// TODO: not sure if negative numbers will work
                context.AppendStringExample(
					feature.Dictify,
                    format,
                    kvp.Key,
                    kvp.Value);
			}
        }
		
		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
		/// <param name="context">The marshalling context.</param>
		/// <param name="ns">The namespace description.</param>
		/// <param name="feature">The feature description.</param>
		/// <param name="value">The feature value.</param>
		[ContractVerification(false)]
        public void MarshalFeature(VowpalWabbitMarshalContext context, Namespace ns, Feature feature, IDictionary<System.SByte, System.Single> value)
        {
            Contract.Requires(context != null);
            Contract.Requires(ns != null);
            Contract.Requires(feature != null);

			if (value == null)
			{
				return;
			}

            foreach (var kvp in value)
            {
				
				context.NamespaceBuilder.AddFeature((uint)(ns.NamespaceHash + kvp.Key), (float)kvp.Value);
            }

			if (context.StringExample == null)
            {
                return;
            }

			var format = " {0}:" + (context.VW.Settings.EnableStringFloatCompact ? "{1}" : "{1:E20}");
			foreach (var kvp in value)
            {
				// TODO: not sure if negative numbers will work
                context.AppendStringExample(
					feature.Dictify,
                    format,
                    kvp.Key,
                    kvp.Value);
			}
        }
		
		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
		/// <param name="context">The marshalling context.</param>
		/// <param name="ns">The namespace description.</param>
		/// <param name="feature">The feature description.</param>
		/// <param name="value">The feature value.</param>
		[ContractVerification(false)]
        public void MarshalFeature(VowpalWabbitMarshalContext context, Namespace ns, Feature feature, IDictionary<System.SByte, System.Int64> value)
        {
            Contract.Requires(context != null);
            Contract.Requires(ns != null);
            Contract.Requires(feature != null);

			if (value == null)
			{
				return;
			}

            foreach (var kvp in value)
            {
				
				context.NamespaceBuilder.AddFeature((uint)(ns.NamespaceHash + kvp.Key), (float)kvp.Value);
            }

			if (context.StringExample == null)
            {
                return;
            }

			var format = " {0}:" + (context.VW.Settings.EnableStringFloatCompact ? "{1}" : "{1:E20}");
			foreach (var kvp in value)
            {
				// TODO: not sure if negative numbers will work
                context.AppendStringExample(
					feature.Dictify,
                    format,
                    kvp.Key,
                    kvp.Value);
			}
        }
		
		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
		/// <param name="context">The marshalling context.</param>
		/// <param name="ns">The namespace description.</param>
		/// <param name="feature">The feature description.</param>
		/// <param name="value">The feature value.</param>
		[ContractVerification(false)]
        public void MarshalFeature(VowpalWabbitMarshalContext context, Namespace ns, Feature feature, IDictionary<System.SByte, System.UInt64> value)
        {
            Contract.Requires(context != null);
            Contract.Requires(ns != null);
            Contract.Requires(feature != null);

			if (value == null)
			{
				return;
			}

            foreach (var kvp in value)
            {
				
				context.NamespaceBuilder.AddFeature((uint)(ns.NamespaceHash + kvp.Key), (float)kvp.Value);
            }

			if (context.StringExample == null)
            {
                return;
            }

			var format = " {0}:" + (context.VW.Settings.EnableStringFloatCompact ? "{1}" : "{1:E20}");
			foreach (var kvp in value)
            {
				// TODO: not sure if negative numbers will work
                context.AppendStringExample(
					feature.Dictify,
                    format,
                    kvp.Key,
                    kvp.Value);
			}
        }
		
		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
		/// <param name="context">The marshalling context.</param>
		/// <param name="ns">The namespace description.</param>
		/// <param name="feature">The feature description.</param>
		/// <param name="value">The feature value.</param>
		[ContractVerification(false)]
        public void MarshalFeature(VowpalWabbitMarshalContext context, Namespace ns, Feature feature, IDictionary<System.SByte, System.Double> value)
        {
            Contract.Requires(context != null);
            Contract.Requires(ns != null);
            Contract.Requires(feature != null);

			if (value == null)
			{
				return;
			}

            foreach (var kvp in value)
            {
				
				context.NamespaceBuilder.AddFeature((uint)(ns.NamespaceHash + kvp.Key), (float)kvp.Value);
            }

			if (context.StringExample == null)
            {
                return;
            }

			var format = " {0}:" + (context.VW.Settings.EnableStringFloatCompact ? "{1}" : "{1:E20}");
			foreach (var kvp in value)
            {
				// TODO: not sure if negative numbers will work
                context.AppendStringExample(
					feature.Dictify,
                    format,
                    kvp.Key,
                    kvp.Value);
			}
        }
		
		
		
		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
		/// <param name="context">The marshalling context.</param>
		/// <param name="ns">The namespace description.</param>
		/// <param name="feature">The feature description.</param>
		/// <param name="value">The feature value.</param>
		[ContractVerification(false)]
        public void MarshalFeature(VowpalWabbitMarshalContext context, Namespace ns, Feature feature, IDictionary<System.Int16, System.Byte> value)
        {
            Contract.Requires(context != null);
            Contract.Requires(ns != null);
            Contract.Requires(feature != null);

			if (value == null)
			{
				return;
			}

            foreach (var kvp in value)
            {
				
				context.NamespaceBuilder.AddFeature((uint)(ns.NamespaceHash + kvp.Key), (float)kvp.Value);
            }

			if (context.StringExample == null)
            {
                return;
            }

			var format = " {0}:" + (context.VW.Settings.EnableStringFloatCompact ? "{1}" : "{1:E20}");
			foreach (var kvp in value)
            {
				// TODO: not sure if negative numbers will work
                context.AppendStringExample(
					feature.Dictify,
                    format,
                    kvp.Key,
                    kvp.Value);
			}
        }
		
		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
		/// <param name="context">The marshalling context.</param>
		/// <param name="ns">The namespace description.</param>
		/// <param name="feature">The feature description.</param>
		/// <param name="value">The feature value.</param>
		[ContractVerification(false)]
        public void MarshalFeature(VowpalWabbitMarshalContext context, Namespace ns, Feature feature, IDictionary<System.Int16, System.SByte> value)
        {
            Contract.Requires(context != null);
            Contract.Requires(ns != null);
            Contract.Requires(feature != null);

			if (value == null)
			{
				return;
			}

            foreach (var kvp in value)
            {
				
				context.NamespaceBuilder.AddFeature((uint)(ns.NamespaceHash + kvp.Key), (float)kvp.Value);
            }

			if (context.StringExample == null)
            {
                return;
            }

			var format = " {0}:" + (context.VW.Settings.EnableStringFloatCompact ? "{1}" : "{1:E20}");
			foreach (var kvp in value)
            {
				// TODO: not sure if negative numbers will work
                context.AppendStringExample(
					feature.Dictify,
                    format,
                    kvp.Key,
                    kvp.Value);
			}
        }
		
		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
		/// <param name="context">The marshalling context.</param>
		/// <param name="ns">The namespace description.</param>
		/// <param name="feature">The feature description.</param>
		/// <param name="value">The feature value.</param>
		[ContractVerification(false)]
        public void MarshalFeature(VowpalWabbitMarshalContext context, Namespace ns, Feature feature, IDictionary<System.Int16, System.Int16> value)
        {
            Contract.Requires(context != null);
            Contract.Requires(ns != null);
            Contract.Requires(feature != null);

			if (value == null)
			{
				return;
			}

            foreach (var kvp in value)
            {
				
				context.NamespaceBuilder.AddFeature((uint)(ns.NamespaceHash + kvp.Key), (float)kvp.Value);
            }

			if (context.StringExample == null)
            {
                return;
            }

			var format = " {0}:" + (context.VW.Settings.EnableStringFloatCompact ? "{1}" : "{1:E20}");
			foreach (var kvp in value)
            {
				// TODO: not sure if negative numbers will work
                context.AppendStringExample(
					feature.Dictify,
                    format,
                    kvp.Key,
                    kvp.Value);
			}
        }
		
		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
		/// <param name="context">The marshalling context.</param>
		/// <param name="ns">The namespace description.</param>
		/// <param name="feature">The feature description.</param>
		/// <param name="value">The feature value.</param>
		[ContractVerification(false)]
        public void MarshalFeature(VowpalWabbitMarshalContext context, Namespace ns, Feature feature, IDictionary<System.Int16, System.Int32> value)
        {
            Contract.Requires(context != null);
            Contract.Requires(ns != null);
            Contract.Requires(feature != null);

			if (value == null)
			{
				return;
			}

            foreach (var kvp in value)
            {
				
				context.NamespaceBuilder.AddFeature((uint)(ns.NamespaceHash + kvp.Key), (float)kvp.Value);
            }

			if (context.StringExample == null)
            {
                return;
            }

			var format = " {0}:" + (context.VW.Settings.EnableStringFloatCompact ? "{1}" : "{1:E20}");
			foreach (var kvp in value)
            {
				// TODO: not sure if negative numbers will work
                context.AppendStringExample(
					feature.Dictify,
                    format,
                    kvp.Key,
                    kvp.Value);
			}
        }
		
		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
		/// <param name="context">The marshalling context.</param>
		/// <param name="ns">The namespace description.</param>
		/// <param name="feature">The feature description.</param>
		/// <param name="value">The feature value.</param>
		[ContractVerification(false)]
        public void MarshalFeature(VowpalWabbitMarshalContext context, Namespace ns, Feature feature, IDictionary<System.Int16, System.UInt16> value)
        {
            Contract.Requires(context != null);
            Contract.Requires(ns != null);
            Contract.Requires(feature != null);

			if (value == null)
			{
				return;
			}

            foreach (var kvp in value)
            {
				
				context.NamespaceBuilder.AddFeature((uint)(ns.NamespaceHash + kvp.Key), (float)kvp.Value);
            }

			if (context.StringExample == null)
            {
                return;
            }

			var format = " {0}:" + (context.VW.Settings.EnableStringFloatCompact ? "{1}" : "{1:E20}");
			foreach (var kvp in value)
            {
				// TODO: not sure if negative numbers will work
                context.AppendStringExample(
					feature.Dictify,
                    format,
                    kvp.Key,
                    kvp.Value);
			}
        }
		
		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
		/// <param name="context">The marshalling context.</param>
		/// <param name="ns">The namespace description.</param>
		/// <param name="feature">The feature description.</param>
		/// <param name="value">The feature value.</param>
		[ContractVerification(false)]
        public void MarshalFeature(VowpalWabbitMarshalContext context, Namespace ns, Feature feature, IDictionary<System.Int16, System.UInt32> value)
        {
            Contract.Requires(context != null);
            Contract.Requires(ns != null);
            Contract.Requires(feature != null);

			if (value == null)
			{
				return;
			}

            foreach (var kvp in value)
            {
				
				context.NamespaceBuilder.AddFeature((uint)(ns.NamespaceHash + kvp.Key), (float)kvp.Value);
            }

			if (context.StringExample == null)
            {
                return;
            }

			var format = " {0}:" + (context.VW.Settings.EnableStringFloatCompact ? "{1}" : "{1:E20}");
			foreach (var kvp in value)
            {
				// TODO: not sure if negative numbers will work
                context.AppendStringExample(
					feature.Dictify,
                    format,
                    kvp.Key,
                    kvp.Value);
			}
        }
		
		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
		/// <param name="context">The marshalling context.</param>
		/// <param name="ns">The namespace description.</param>
		/// <param name="feature">The feature description.</param>
		/// <param name="value">The feature value.</param>
		[ContractVerification(false)]
        public void MarshalFeature(VowpalWabbitMarshalContext context, Namespace ns, Feature feature, IDictionary<System.Int16, System.Single> value)
        {
            Contract.Requires(context != null);
            Contract.Requires(ns != null);
            Contract.Requires(feature != null);

			if (value == null)
			{
				return;
			}

            foreach (var kvp in value)
            {
				
				context.NamespaceBuilder.AddFeature((uint)(ns.NamespaceHash + kvp.Key), (float)kvp.Value);
            }

			if (context.StringExample == null)
            {
                return;
            }

			var format = " {0}:" + (context.VW.Settings.EnableStringFloatCompact ? "{1}" : "{1:E20}");
			foreach (var kvp in value)
            {
				// TODO: not sure if negative numbers will work
                context.AppendStringExample(
					feature.Dictify,
                    format,
                    kvp.Key,
                    kvp.Value);
			}
        }
		
		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
		/// <param name="context">The marshalling context.</param>
		/// <param name="ns">The namespace description.</param>
		/// <param name="feature">The feature description.</param>
		/// <param name="value">The feature value.</param>
		[ContractVerification(false)]
        public void MarshalFeature(VowpalWabbitMarshalContext context, Namespace ns, Feature feature, IDictionary<System.Int16, System.Int64> value)
        {
            Contract.Requires(context != null);
            Contract.Requires(ns != null);
            Contract.Requires(feature != null);

			if (value == null)
			{
				return;
			}

            foreach (var kvp in value)
            {
				
				context.NamespaceBuilder.AddFeature((uint)(ns.NamespaceHash + kvp.Key), (float)kvp.Value);
            }

			if (context.StringExample == null)
            {
                return;
            }

			var format = " {0}:" + (context.VW.Settings.EnableStringFloatCompact ? "{1}" : "{1:E20}");
			foreach (var kvp in value)
            {
				// TODO: not sure if negative numbers will work
                context.AppendStringExample(
					feature.Dictify,
                    format,
                    kvp.Key,
                    kvp.Value);
			}
        }
		
		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
		/// <param name="context">The marshalling context.</param>
		/// <param name="ns">The namespace description.</param>
		/// <param name="feature">The feature description.</param>
		/// <param name="value">The feature value.</param>
		[ContractVerification(false)]
        public void MarshalFeature(VowpalWabbitMarshalContext context, Namespace ns, Feature feature, IDictionary<System.Int16, System.UInt64> value)
        {
            Contract.Requires(context != null);
            Contract.Requires(ns != null);
            Contract.Requires(feature != null);

			if (value == null)
			{
				return;
			}

            foreach (var kvp in value)
            {
				
				context.NamespaceBuilder.AddFeature((uint)(ns.NamespaceHash + kvp.Key), (float)kvp.Value);
            }

			if (context.StringExample == null)
            {
                return;
            }

			var format = " {0}:" + (context.VW.Settings.EnableStringFloatCompact ? "{1}" : "{1:E20}");
			foreach (var kvp in value)
            {
				// TODO: not sure if negative numbers will work
                context.AppendStringExample(
					feature.Dictify,
                    format,
                    kvp.Key,
                    kvp.Value);
			}
        }
		
		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
		/// <param name="context">The marshalling context.</param>
		/// <param name="ns">The namespace description.</param>
		/// <param name="feature">The feature description.</param>
		/// <param name="value">The feature value.</param>
		[ContractVerification(false)]
        public void MarshalFeature(VowpalWabbitMarshalContext context, Namespace ns, Feature feature, IDictionary<System.Int16, System.Double> value)
        {
            Contract.Requires(context != null);
            Contract.Requires(ns != null);
            Contract.Requires(feature != null);

			if (value == null)
			{
				return;
			}

            foreach (var kvp in value)
            {
				
				context.NamespaceBuilder.AddFeature((uint)(ns.NamespaceHash + kvp.Key), (float)kvp.Value);
            }

			if (context.StringExample == null)
            {
                return;
            }

			var format = " {0}:" + (context.VW.Settings.EnableStringFloatCompact ? "{1}" : "{1:E20}");
			foreach (var kvp in value)
            {
				// TODO: not sure if negative numbers will work
                context.AppendStringExample(
					feature.Dictify,
                    format,
                    kvp.Key,
                    kvp.Value);
			}
        }
		
		
		
		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
		/// <param name="context">The marshalling context.</param>
		/// <param name="ns">The namespace description.</param>
		/// <param name="feature">The feature description.</param>
		/// <param name="value">The feature value.</param>
		[ContractVerification(false)]
        public void MarshalFeature(VowpalWabbitMarshalContext context, Namespace ns, Feature feature, IDictionary<System.Int32, System.Byte> value)
        {
            Contract.Requires(context != null);
            Contract.Requires(ns != null);
            Contract.Requires(feature != null);

			if (value == null)
			{
				return;
			}

            foreach (var kvp in value)
            {
				
				context.NamespaceBuilder.AddFeature((uint)(ns.NamespaceHash + kvp.Key), (float)kvp.Value);
            }

			if (context.StringExample == null)
            {
                return;
            }

			var format = " {0}:" + (context.VW.Settings.EnableStringFloatCompact ? "{1}" : "{1:E20}");
			foreach (var kvp in value)
            {
				// TODO: not sure if negative numbers will work
                context.AppendStringExample(
					feature.Dictify,
                    format,
                    kvp.Key,
                    kvp.Value);
			}
        }
		
		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
		/// <param name="context">The marshalling context.</param>
		/// <param name="ns">The namespace description.</param>
		/// <param name="feature">The feature description.</param>
		/// <param name="value">The feature value.</param>
		[ContractVerification(false)]
        public void MarshalFeature(VowpalWabbitMarshalContext context, Namespace ns, Feature feature, IDictionary<System.Int32, System.SByte> value)
        {
            Contract.Requires(context != null);
            Contract.Requires(ns != null);
            Contract.Requires(feature != null);

			if (value == null)
			{
				return;
			}

            foreach (var kvp in value)
            {
				
				context.NamespaceBuilder.AddFeature((uint)(ns.NamespaceHash + kvp.Key), (float)kvp.Value);
            }

			if (context.StringExample == null)
            {
                return;
            }

			var format = " {0}:" + (context.VW.Settings.EnableStringFloatCompact ? "{1}" : "{1:E20}");
			foreach (var kvp in value)
            {
				// TODO: not sure if negative numbers will work
                context.AppendStringExample(
					feature.Dictify,
                    format,
                    kvp.Key,
                    kvp.Value);
			}
        }
		
		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
		/// <param name="context">The marshalling context.</param>
		/// <param name="ns">The namespace description.</param>
		/// <param name="feature">The feature description.</param>
		/// <param name="value">The feature value.</param>
		[ContractVerification(false)]
        public void MarshalFeature(VowpalWabbitMarshalContext context, Namespace ns, Feature feature, IDictionary<System.Int32, System.Int16> value)
        {
            Contract.Requires(context != null);
            Contract.Requires(ns != null);
            Contract.Requires(feature != null);

			if (value == null)
			{
				return;
			}

            foreach (var kvp in value)
            {
				
				context.NamespaceBuilder.AddFeature((uint)(ns.NamespaceHash + kvp.Key), (float)kvp.Value);
            }

			if (context.StringExample == null)
            {
                return;
            }

			var format = " {0}:" + (context.VW.Settings.EnableStringFloatCompact ? "{1}" : "{1:E20}");
			foreach (var kvp in value)
            {
				// TODO: not sure if negative numbers will work
                context.AppendStringExample(
					feature.Dictify,
                    format,
                    kvp.Key,
                    kvp.Value);
			}
        }
		
		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
		/// <param name="context">The marshalling context.</param>
		/// <param name="ns">The namespace description.</param>
		/// <param name="feature">The feature description.</param>
		/// <param name="value">The feature value.</param>
		[ContractVerification(false)]
        public void MarshalFeature(VowpalWabbitMarshalContext context, Namespace ns, Feature feature, IDictionary<System.Int32, System.Int32> value)
        {
            Contract.Requires(context != null);
            Contract.Requires(ns != null);
            Contract.Requires(feature != null);

			if (value == null)
			{
				return;
			}

            foreach (var kvp in value)
            {
				
				context.NamespaceBuilder.AddFeature((uint)(ns.NamespaceHash + kvp.Key), (float)kvp.Value);
            }

			if (context.StringExample == null)
            {
                return;
            }

			var format = " {0}:" + (context.VW.Settings.EnableStringFloatCompact ? "{1}" : "{1:E20}");
			foreach (var kvp in value)
            {
				// TODO: not sure if negative numbers will work
                context.AppendStringExample(
					feature.Dictify,
                    format,
                    kvp.Key,
                    kvp.Value);
			}
        }
		
		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
		/// <param name="context">The marshalling context.</param>
		/// <param name="ns">The namespace description.</param>
		/// <param name="feature">The feature description.</param>
		/// <param name="value">The feature value.</param>
		[ContractVerification(false)]
        public void MarshalFeature(VowpalWabbitMarshalContext context, Namespace ns, Feature feature, IDictionary<System.Int32, System.UInt16> value)
        {
            Contract.Requires(context != null);
            Contract.Requires(ns != null);
            Contract.Requires(feature != null);

			if (value == null)
			{
				return;
			}

            foreach (var kvp in value)
            {
				
				context.NamespaceBuilder.AddFeature((uint)(ns.NamespaceHash + kvp.Key), (float)kvp.Value);
            }

			if (context.StringExample == null)
            {
                return;
            }

			var format = " {0}:" + (context.VW.Settings.EnableStringFloatCompact ? "{1}" : "{1:E20}");
			foreach (var kvp in value)
            {
				// TODO: not sure if negative numbers will work
                context.AppendStringExample(
					feature.Dictify,
                    format,
                    kvp.Key,
                    kvp.Value);
			}
        }
		
		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
		/// <param name="context">The marshalling context.</param>
		/// <param name="ns">The namespace description.</param>
		/// <param name="feature">The feature description.</param>
		/// <param name="value">The feature value.</param>
		[ContractVerification(false)]
        public void MarshalFeature(VowpalWabbitMarshalContext context, Namespace ns, Feature feature, IDictionary<System.Int32, System.UInt32> value)
        {
            Contract.Requires(context != null);
            Contract.Requires(ns != null);
            Contract.Requires(feature != null);

			if (value == null)
			{
				return;
			}

            foreach (var kvp in value)
            {
				
				context.NamespaceBuilder.AddFeature((uint)(ns.NamespaceHash + kvp.Key), (float)kvp.Value);
            }

			if (context.StringExample == null)
            {
                return;
            }

			var format = " {0}:" + (context.VW.Settings.EnableStringFloatCompact ? "{1}" : "{1:E20}");
			foreach (var kvp in value)
            {
				// TODO: not sure if negative numbers will work
                context.AppendStringExample(
					feature.Dictify,
                    format,
                    kvp.Key,
                    kvp.Value);
			}
        }
		
		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
		/// <param name="context">The marshalling context.</param>
		/// <param name="ns">The namespace description.</param>
		/// <param name="feature">The feature description.</param>
		/// <param name="value">The feature value.</param>
		[ContractVerification(false)]
        public void MarshalFeature(VowpalWabbitMarshalContext context, Namespace ns, Feature feature, IDictionary<System.Int32, System.Single> value)
        {
            Contract.Requires(context != null);
            Contract.Requires(ns != null);
            Contract.Requires(feature != null);

			if (value == null)
			{
				return;
			}

            foreach (var kvp in value)
            {
				
				context.NamespaceBuilder.AddFeature((uint)(ns.NamespaceHash + kvp.Key), (float)kvp.Value);
            }

			if (context.StringExample == null)
            {
                return;
            }

			var format = " {0}:" + (context.VW.Settings.EnableStringFloatCompact ? "{1}" : "{1:E20}");
			foreach (var kvp in value)
            {
				// TODO: not sure if negative numbers will work
                context.AppendStringExample(
					feature.Dictify,
                    format,
                    kvp.Key,
                    kvp.Value);
			}
        }
		
		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
		/// <param name="context">The marshalling context.</param>
		/// <param name="ns">The namespace description.</param>
		/// <param name="feature">The feature description.</param>
		/// <param name="value">The feature value.</param>
		[ContractVerification(false)]
        public void MarshalFeature(VowpalWabbitMarshalContext context, Namespace ns, Feature feature, IDictionary<System.Int32, System.Int64> value)
        {
            Contract.Requires(context != null);
            Contract.Requires(ns != null);
            Contract.Requires(feature != null);

			if (value == null)
			{
				return;
			}

            foreach (var kvp in value)
            {
				
				context.NamespaceBuilder.AddFeature((uint)(ns.NamespaceHash + kvp.Key), (float)kvp.Value);
            }

			if (context.StringExample == null)
            {
                return;
            }

			var format = " {0}:" + (context.VW.Settings.EnableStringFloatCompact ? "{1}" : "{1:E20}");
			foreach (var kvp in value)
            {
				// TODO: not sure if negative numbers will work
                context.AppendStringExample(
					feature.Dictify,
                    format,
                    kvp.Key,
                    kvp.Value);
			}
        }
		
		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
		/// <param name="context">The marshalling context.</param>
		/// <param name="ns">The namespace description.</param>
		/// <param name="feature">The feature description.</param>
		/// <param name="value">The feature value.</param>
		[ContractVerification(false)]
        public void MarshalFeature(VowpalWabbitMarshalContext context, Namespace ns, Feature feature, IDictionary<System.Int32, System.UInt64> value)
        {
            Contract.Requires(context != null);
            Contract.Requires(ns != null);
            Contract.Requires(feature != null);

			if (value == null)
			{
				return;
			}

            foreach (var kvp in value)
            {
				
				context.NamespaceBuilder.AddFeature((uint)(ns.NamespaceHash + kvp.Key), (float)kvp.Value);
            }

			if (context.StringExample == null)
            {
                return;
            }

			var format = " {0}:" + (context.VW.Settings.EnableStringFloatCompact ? "{1}" : "{1:E20}");
			foreach (var kvp in value)
            {
				// TODO: not sure if negative numbers will work
                context.AppendStringExample(
					feature.Dictify,
                    format,
                    kvp.Key,
                    kvp.Value);
			}
        }
		
		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
		/// <param name="context">The marshalling context.</param>
		/// <param name="ns">The namespace description.</param>
		/// <param name="feature">The feature description.</param>
		/// <param name="value">The feature value.</param>
		[ContractVerification(false)]
        public void MarshalFeature(VowpalWabbitMarshalContext context, Namespace ns, Feature feature, IDictionary<System.Int32, System.Double> value)
        {
            Contract.Requires(context != null);
            Contract.Requires(ns != null);
            Contract.Requires(feature != null);

			if (value == null)
			{
				return;
			}

            foreach (var kvp in value)
            {
				
				context.NamespaceBuilder.AddFeature((uint)(ns.NamespaceHash + kvp.Key), (float)kvp.Value);
            }

			if (context.StringExample == null)
            {
                return;
            }

			var format = " {0}:" + (context.VW.Settings.EnableStringFloatCompact ? "{1}" : "{1:E20}");
			foreach (var kvp in value)
            {
				// TODO: not sure if negative numbers will work
                context.AppendStringExample(
					feature.Dictify,
                    format,
                    kvp.Key,
                    kvp.Value);
			}
        }
		
		
		
		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
		/// <param name="context">The marshalling context.</param>
		/// <param name="ns">The namespace description.</param>
		/// <param name="feature">The feature description.</param>
		/// <param name="value">The feature value.</param>
		[ContractVerification(false)]
        public void MarshalFeature(VowpalWabbitMarshalContext context, Namespace ns, Feature feature, IDictionary<System.UInt16, System.Byte> value)
        {
            Contract.Requires(context != null);
            Contract.Requires(ns != null);
            Contract.Requires(feature != null);

			if (value == null)
			{
				return;
			}

            foreach (var kvp in value)
            {
				
				context.NamespaceBuilder.AddFeature((uint)(ns.NamespaceHash + kvp.Key), (float)kvp.Value);
            }

			if (context.StringExample == null)
            {
                return;
            }

			var format = " {0}:" + (context.VW.Settings.EnableStringFloatCompact ? "{1}" : "{1:E20}");
			foreach (var kvp in value)
            {
				// TODO: not sure if negative numbers will work
                context.AppendStringExample(
					feature.Dictify,
                    format,
                    kvp.Key,
                    kvp.Value);
			}
        }
		
		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
		/// <param name="context">The marshalling context.</param>
		/// <param name="ns">The namespace description.</param>
		/// <param name="feature">The feature description.</param>
		/// <param name="value">The feature value.</param>
		[ContractVerification(false)]
        public void MarshalFeature(VowpalWabbitMarshalContext context, Namespace ns, Feature feature, IDictionary<System.UInt16, System.SByte> value)
        {
            Contract.Requires(context != null);
            Contract.Requires(ns != null);
            Contract.Requires(feature != null);

			if (value == null)
			{
				return;
			}

            foreach (var kvp in value)
            {
				
				context.NamespaceBuilder.AddFeature((uint)(ns.NamespaceHash + kvp.Key), (float)kvp.Value);
            }

			if (context.StringExample == null)
            {
                return;
            }

			var format = " {0}:" + (context.VW.Settings.EnableStringFloatCompact ? "{1}" : "{1:E20}");
			foreach (var kvp in value)
            {
				// TODO: not sure if negative numbers will work
                context.AppendStringExample(
					feature.Dictify,
                    format,
                    kvp.Key,
                    kvp.Value);
			}
        }
		
		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
		/// <param name="context">The marshalling context.</param>
		/// <param name="ns">The namespace description.</param>
		/// <param name="feature">The feature description.</param>
		/// <param name="value">The feature value.</param>
		[ContractVerification(false)]
        public void MarshalFeature(VowpalWabbitMarshalContext context, Namespace ns, Feature feature, IDictionary<System.UInt16, System.Int16> value)
        {
            Contract.Requires(context != null);
            Contract.Requires(ns != null);
            Contract.Requires(feature != null);

			if (value == null)
			{
				return;
			}

            foreach (var kvp in value)
            {
				
				context.NamespaceBuilder.AddFeature((uint)(ns.NamespaceHash + kvp.Key), (float)kvp.Value);
            }

			if (context.StringExample == null)
            {
                return;
            }

			var format = " {0}:" + (context.VW.Settings.EnableStringFloatCompact ? "{1}" : "{1:E20}");
			foreach (var kvp in value)
            {
				// TODO: not sure if negative numbers will work
                context.AppendStringExample(
					feature.Dictify,
                    format,
                    kvp.Key,
                    kvp.Value);
			}
        }
		
		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
		/// <param name="context">The marshalling context.</param>
		/// <param name="ns">The namespace description.</param>
		/// <param name="feature">The feature description.</param>
		/// <param name="value">The feature value.</param>
		[ContractVerification(false)]
        public void MarshalFeature(VowpalWabbitMarshalContext context, Namespace ns, Feature feature, IDictionary<System.UInt16, System.Int32> value)
        {
            Contract.Requires(context != null);
            Contract.Requires(ns != null);
            Contract.Requires(feature != null);

			if (value == null)
			{
				return;
			}

            foreach (var kvp in value)
            {
				
				context.NamespaceBuilder.AddFeature((uint)(ns.NamespaceHash + kvp.Key), (float)kvp.Value);
            }

			if (context.StringExample == null)
            {
                return;
            }

			var format = " {0}:" + (context.VW.Settings.EnableStringFloatCompact ? "{1}" : "{1:E20}");
			foreach (var kvp in value)
            {
				// TODO: not sure if negative numbers will work
                context.AppendStringExample(
					feature.Dictify,
                    format,
                    kvp.Key,
                    kvp.Value);
			}
        }
		
		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
		/// <param name="context">The marshalling context.</param>
		/// <param name="ns">The namespace description.</param>
		/// <param name="feature">The feature description.</param>
		/// <param name="value">The feature value.</param>
		[ContractVerification(false)]
        public void MarshalFeature(VowpalWabbitMarshalContext context, Namespace ns, Feature feature, IDictionary<System.UInt16, System.UInt16> value)
        {
            Contract.Requires(context != null);
            Contract.Requires(ns != null);
            Contract.Requires(feature != null);

			if (value == null)
			{
				return;
			}

            foreach (var kvp in value)
            {
				
				context.NamespaceBuilder.AddFeature((uint)(ns.NamespaceHash + kvp.Key), (float)kvp.Value);
            }

			if (context.StringExample == null)
            {
                return;
            }

			var format = " {0}:" + (context.VW.Settings.EnableStringFloatCompact ? "{1}" : "{1:E20}");
			foreach (var kvp in value)
            {
				// TODO: not sure if negative numbers will work
                context.AppendStringExample(
					feature.Dictify,
                    format,
                    kvp.Key,
                    kvp.Value);
			}
        }
		
		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
		/// <param name="context">The marshalling context.</param>
		/// <param name="ns">The namespace description.</param>
		/// <param name="feature">The feature description.</param>
		/// <param name="value">The feature value.</param>
		[ContractVerification(false)]
        public void MarshalFeature(VowpalWabbitMarshalContext context, Namespace ns, Feature feature, IDictionary<System.UInt16, System.UInt32> value)
        {
            Contract.Requires(context != null);
            Contract.Requires(ns != null);
            Contract.Requires(feature != null);

			if (value == null)
			{
				return;
			}

            foreach (var kvp in value)
            {
				
				context.NamespaceBuilder.AddFeature((uint)(ns.NamespaceHash + kvp.Key), (float)kvp.Value);
            }

			if (context.StringExample == null)
            {
                return;
            }

			var format = " {0}:" + (context.VW.Settings.EnableStringFloatCompact ? "{1}" : "{1:E20}");
			foreach (var kvp in value)
            {
				// TODO: not sure if negative numbers will work
                context.AppendStringExample(
					feature.Dictify,
                    format,
                    kvp.Key,
                    kvp.Value);
			}
        }
		
		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
		/// <param name="context">The marshalling context.</param>
		/// <param name="ns">The namespace description.</param>
		/// <param name="feature">The feature description.</param>
		/// <param name="value">The feature value.</param>
		[ContractVerification(false)]
        public void MarshalFeature(VowpalWabbitMarshalContext context, Namespace ns, Feature feature, IDictionary<System.UInt16, System.Single> value)
        {
            Contract.Requires(context != null);
            Contract.Requires(ns != null);
            Contract.Requires(feature != null);

			if (value == null)
			{
				return;
			}

            foreach (var kvp in value)
            {
				
				context.NamespaceBuilder.AddFeature((uint)(ns.NamespaceHash + kvp.Key), (float)kvp.Value);
            }

			if (context.StringExample == null)
            {
                return;
            }

			var format = " {0}:" + (context.VW.Settings.EnableStringFloatCompact ? "{1}" : "{1:E20}");
			foreach (var kvp in value)
            {
				// TODO: not sure if negative numbers will work
                context.AppendStringExample(
					feature.Dictify,
                    format,
                    kvp.Key,
                    kvp.Value);
			}
        }
		
		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
		/// <param name="context">The marshalling context.</param>
		/// <param name="ns">The namespace description.</param>
		/// <param name="feature">The feature description.</param>
		/// <param name="value">The feature value.</param>
		[ContractVerification(false)]
        public void MarshalFeature(VowpalWabbitMarshalContext context, Namespace ns, Feature feature, IDictionary<System.UInt16, System.Int64> value)
        {
            Contract.Requires(context != null);
            Contract.Requires(ns != null);
            Contract.Requires(feature != null);

			if (value == null)
			{
				return;
			}

            foreach (var kvp in value)
            {
				
				context.NamespaceBuilder.AddFeature((uint)(ns.NamespaceHash + kvp.Key), (float)kvp.Value);
            }

			if (context.StringExample == null)
            {
                return;
            }

			var format = " {0}:" + (context.VW.Settings.EnableStringFloatCompact ? "{1}" : "{1:E20}");
			foreach (var kvp in value)
            {
				// TODO: not sure if negative numbers will work
                context.AppendStringExample(
					feature.Dictify,
                    format,
                    kvp.Key,
                    kvp.Value);
			}
        }
		
		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
		/// <param name="context">The marshalling context.</param>
		/// <param name="ns">The namespace description.</param>
		/// <param name="feature">The feature description.</param>
		/// <param name="value">The feature value.</param>
		[ContractVerification(false)]
        public void MarshalFeature(VowpalWabbitMarshalContext context, Namespace ns, Feature feature, IDictionary<System.UInt16, System.UInt64> value)
        {
            Contract.Requires(context != null);
            Contract.Requires(ns != null);
            Contract.Requires(feature != null);

			if (value == null)
			{
				return;
			}

            foreach (var kvp in value)
            {
				
				context.NamespaceBuilder.AddFeature((uint)(ns.NamespaceHash + kvp.Key), (float)kvp.Value);
            }

			if (context.StringExample == null)
            {
                return;
            }

			var format = " {0}:" + (context.VW.Settings.EnableStringFloatCompact ? "{1}" : "{1:E20}");
			foreach (var kvp in value)
            {
				// TODO: not sure if negative numbers will work
                context.AppendStringExample(
					feature.Dictify,
                    format,
                    kvp.Key,
                    kvp.Value);
			}
        }
		
		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
		/// <param name="context">The marshalling context.</param>
		/// <param name="ns">The namespace description.</param>
		/// <param name="feature">The feature description.</param>
		/// <param name="value">The feature value.</param>
		[ContractVerification(false)]
        public void MarshalFeature(VowpalWabbitMarshalContext context, Namespace ns, Feature feature, IDictionary<System.UInt16, System.Double> value)
        {
            Contract.Requires(context != null);
            Contract.Requires(ns != null);
            Contract.Requires(feature != null);

			if (value == null)
			{
				return;
			}

            foreach (var kvp in value)
            {
				
				context.NamespaceBuilder.AddFeature((uint)(ns.NamespaceHash + kvp.Key), (float)kvp.Value);
            }

			if (context.StringExample == null)
            {
                return;
            }

			var format = " {0}:" + (context.VW.Settings.EnableStringFloatCompact ? "{1}" : "{1:E20}");
			foreach (var kvp in value)
            {
				// TODO: not sure if negative numbers will work
                context.AppendStringExample(
					feature.Dictify,
                    format,
                    kvp.Key,
                    kvp.Value);
			}
        }
		
		    }
}
