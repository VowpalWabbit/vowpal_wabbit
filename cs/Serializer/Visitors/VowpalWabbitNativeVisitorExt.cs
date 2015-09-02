// --------------------------------------------------------------------------------------------------------------------
// <copyright file="VowpalWabbitInterfaceVisitor.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using System;
using System.Collections;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using VW.Serializer.Interfaces;

namespace VW.Serializer.Visitors
{
    public sealed partial class VowpalWabbitInterfaceVisitor : IVowpalWabbitVisitor<VowpalWabbitExample>
    {
			    /// <summary>
        /// Transfers feature data to native space.
        /// </summary>
        /// <param name="feature">The feature.</param>
        public void Visit(IFeature<System.Byte> feature)
        {
            this.namespaceBuilder.AddFeature(this.vw.HashFeature(feature.Name, this.namespaceHash), feature.Value);
        }

		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
        /// <param name="feature">The feature.</param>
        public void Visit(IFeature<System.Byte?> feature)
        {
            this.namespaceBuilder.AddFeature(this.vw.HashFeature(feature.Name, this.namespaceHash), (float)feature.Value);
        }
		/// <summary>
        /// Transfers namespace data to native space.
        /// </summary>
        /// <param name="namespaceDense">The dense namespace.</param>
        public void Visit(INamespaceDense<System.Byte> namespaceDense)
        {
            this.featureGroup = namespaceDense.FeatureGroup ?? '\0';

            this.namespaceHash = namespaceDense.Name == null ? 
                this.vw.HashSpace(this.featureGroup.ToString()) :
                this.vw.HashSpace(this.featureGroup + namespaceDense.Name);

            this.namespaceBuilder = this.builder.AddNamespace(this.featureGroup);
            this.namespaceBuilder.PreAllocate(namespaceDense.DenseFeature.Value.Count);

            var i = 0;
            foreach (var v in namespaceDense.DenseFeature.Value)
            {
                this.namespaceBuilder.AddFeature((uint)(this.namespaceHash + i), v);
                i++;
            }
        }
			    /// <summary>
        /// Transfers feature data to native space.
        /// </summary>
        /// <param name="feature">The feature.</param>
        public void Visit(IFeature<System.SByte> feature)
        {
            this.namespaceBuilder.AddFeature(this.vw.HashFeature(feature.Name, this.namespaceHash), feature.Value);
        }

		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
        /// <param name="feature">The feature.</param>
        public void Visit(IFeature<System.SByte?> feature)
        {
            this.namespaceBuilder.AddFeature(this.vw.HashFeature(feature.Name, this.namespaceHash), (float)feature.Value);
        }
		/// <summary>
        /// Transfers namespace data to native space.
        /// </summary>
        /// <param name="namespaceDense">The dense namespace.</param>
        public void Visit(INamespaceDense<System.SByte> namespaceDense)
        {
            this.featureGroup = namespaceDense.FeatureGroup ?? '\0';

            this.namespaceHash = namespaceDense.Name == null ? 
                this.vw.HashSpace(this.featureGroup.ToString()) :
                this.vw.HashSpace(this.featureGroup + namespaceDense.Name);

            this.namespaceBuilder = this.builder.AddNamespace(this.featureGroup);
            this.namespaceBuilder.PreAllocate(namespaceDense.DenseFeature.Value.Count);

            var i = 0;
            foreach (var v in namespaceDense.DenseFeature.Value)
            {
                this.namespaceBuilder.AddFeature((uint)(this.namespaceHash + i), v);
                i++;
            }
        }
			    /// <summary>
        /// Transfers feature data to native space.
        /// </summary>
        /// <param name="feature">The feature.</param>
        public void Visit(IFeature<System.Int16> feature)
        {
            this.namespaceBuilder.AddFeature(this.vw.HashFeature(feature.Name, this.namespaceHash), feature.Value);
        }

		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
        /// <param name="feature">The feature.</param>
        public void Visit(IFeature<System.Int16?> feature)
        {
            this.namespaceBuilder.AddFeature(this.vw.HashFeature(feature.Name, this.namespaceHash), (float)feature.Value);
        }
		/// <summary>
        /// Transfers namespace data to native space.
        /// </summary>
        /// <param name="namespaceDense">The dense namespace.</param>
        public void Visit(INamespaceDense<System.Int16> namespaceDense)
        {
            this.featureGroup = namespaceDense.FeatureGroup ?? '\0';

            this.namespaceHash = namespaceDense.Name == null ? 
                this.vw.HashSpace(this.featureGroup.ToString()) :
                this.vw.HashSpace(this.featureGroup + namespaceDense.Name);

            this.namespaceBuilder = this.builder.AddNamespace(this.featureGroup);
            this.namespaceBuilder.PreAllocate(namespaceDense.DenseFeature.Value.Count);

            var i = 0;
            foreach (var v in namespaceDense.DenseFeature.Value)
            {
                this.namespaceBuilder.AddFeature((uint)(this.namespaceHash + i), v);
                i++;
            }
        }
			    /// <summary>
        /// Transfers feature data to native space.
        /// </summary>
        /// <param name="feature">The feature.</param>
        public void Visit(IFeature<System.Int32> feature)
        {
            this.namespaceBuilder.AddFeature(this.vw.HashFeature(feature.Name, this.namespaceHash), feature.Value);
        }

		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
        /// <param name="feature">The feature.</param>
        public void Visit(IFeature<System.Int32?> feature)
        {
            this.namespaceBuilder.AddFeature(this.vw.HashFeature(feature.Name, this.namespaceHash), (float)feature.Value);
        }
		/// <summary>
        /// Transfers namespace data to native space.
        /// </summary>
        /// <param name="namespaceDense">The dense namespace.</param>
        public void Visit(INamespaceDense<System.Int32> namespaceDense)
        {
            this.featureGroup = namespaceDense.FeatureGroup ?? '\0';

            this.namespaceHash = namespaceDense.Name == null ? 
                this.vw.HashSpace(this.featureGroup.ToString()) :
                this.vw.HashSpace(this.featureGroup + namespaceDense.Name);

            this.namespaceBuilder = this.builder.AddNamespace(this.featureGroup);
            this.namespaceBuilder.PreAllocate(namespaceDense.DenseFeature.Value.Count);

            var i = 0;
            foreach (var v in namespaceDense.DenseFeature.Value)
            {
                this.namespaceBuilder.AddFeature((uint)(this.namespaceHash + i), v);
                i++;
            }
        }
			    /// <summary>
        /// Transfers feature data to native space.
        /// </summary>
        /// <param name="feature">The feature.</param>
        public void Visit(IFeature<System.UInt16> feature)
        {
            this.namespaceBuilder.AddFeature(this.vw.HashFeature(feature.Name, this.namespaceHash), feature.Value);
        }

		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
        /// <param name="feature">The feature.</param>
        public void Visit(IFeature<System.UInt16?> feature)
        {
            this.namespaceBuilder.AddFeature(this.vw.HashFeature(feature.Name, this.namespaceHash), (float)feature.Value);
        }
		/// <summary>
        /// Transfers namespace data to native space.
        /// </summary>
        /// <param name="namespaceDense">The dense namespace.</param>
        public void Visit(INamespaceDense<System.UInt16> namespaceDense)
        {
            this.featureGroup = namespaceDense.FeatureGroup ?? '\0';

            this.namespaceHash = namespaceDense.Name == null ? 
                this.vw.HashSpace(this.featureGroup.ToString()) :
                this.vw.HashSpace(this.featureGroup + namespaceDense.Name);

            this.namespaceBuilder = this.builder.AddNamespace(this.featureGroup);
            this.namespaceBuilder.PreAllocate(namespaceDense.DenseFeature.Value.Count);

            var i = 0;
            foreach (var v in namespaceDense.DenseFeature.Value)
            {
                this.namespaceBuilder.AddFeature((uint)(this.namespaceHash + i), v);
                i++;
            }
        }
			    /// <summary>
        /// Transfers feature data to native space.
        /// </summary>
        /// <param name="feature">The feature.</param>
        public void Visit(IFeature<System.UInt32> feature)
        {
            this.namespaceBuilder.AddFeature(this.vw.HashFeature(feature.Name, this.namespaceHash), feature.Value);
        }

		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
        /// <param name="feature">The feature.</param>
        public void Visit(IFeature<System.UInt32?> feature)
        {
            this.namespaceBuilder.AddFeature(this.vw.HashFeature(feature.Name, this.namespaceHash), (float)feature.Value);
        }
		/// <summary>
        /// Transfers namespace data to native space.
        /// </summary>
        /// <param name="namespaceDense">The dense namespace.</param>
        public void Visit(INamespaceDense<System.UInt32> namespaceDense)
        {
            this.featureGroup = namespaceDense.FeatureGroup ?? '\0';

            this.namespaceHash = namespaceDense.Name == null ? 
                this.vw.HashSpace(this.featureGroup.ToString()) :
                this.vw.HashSpace(this.featureGroup + namespaceDense.Name);

            this.namespaceBuilder = this.builder.AddNamespace(this.featureGroup);
            this.namespaceBuilder.PreAllocate(namespaceDense.DenseFeature.Value.Count);

            var i = 0;
            foreach (var v in namespaceDense.DenseFeature.Value)
            {
                this.namespaceBuilder.AddFeature((uint)(this.namespaceHash + i), v);
                i++;
            }
        }
			    /// <summary>
        /// Transfers feature data to native space.
        /// </summary>
        /// <param name="feature">The feature.</param>
        public void Visit(IFeature<System.Single> feature)
        {
            this.namespaceBuilder.AddFeature(this.vw.HashFeature(feature.Name, this.namespaceHash), feature.Value);
        }

		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
        /// <param name="feature">The feature.</param>
        public void Visit(IFeature<System.Single?> feature)
        {
            this.namespaceBuilder.AddFeature(this.vw.HashFeature(feature.Name, this.namespaceHash), (float)feature.Value);
        }
		/// <summary>
        /// Transfers namespace data to native space.
        /// </summary>
        /// <param name="namespaceDense">The dense namespace.</param>
        public void Visit(INamespaceDense<System.Single> namespaceDense)
        {
            this.featureGroup = namespaceDense.FeatureGroup ?? '\0';

            this.namespaceHash = namespaceDense.Name == null ? 
                this.vw.HashSpace(this.featureGroup.ToString()) :
                this.vw.HashSpace(this.featureGroup + namespaceDense.Name);

            this.namespaceBuilder = this.builder.AddNamespace(this.featureGroup);
            this.namespaceBuilder.PreAllocate(namespaceDense.DenseFeature.Value.Count);

            var i = 0;
            foreach (var v in namespaceDense.DenseFeature.Value)
            {
                this.namespaceBuilder.AddFeature((uint)(this.namespaceHash + i), v);
                i++;
            }
        }
		
			    /// <summary>
        /// Transfers feature data to native space.
        /// </summary>
        /// <param name="feature">The feature.</param>
        public void Visit(IFeature<System.Int64> feature)
        {
#if DEBUG
            if (feature.Value > float.MaxValue || feature.Value < float.MinValue)
            {
                Trace.TraceWarning("Precision lost for feature value: " + feature.Value);
            }
#endif
            this.namespaceBuilder.AddFeature(this.vw.HashFeature(feature.Name, this.namespaceHash), (float)feature.Value);
        }

		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
        /// <param name="feature">The feature.</param>
        public void Visit(IFeature<System.Int64?> feature)
        {
#if DEBUG
            if (feature.Value > float.MaxValue || feature.Value < float.MinValue)
            {
                Trace.TraceWarning("Precision lost for feature value: " + feature.Value);
            }
#endif
            this.namespaceBuilder.AddFeature(this.vw.HashFeature(feature.Name, this.namespaceHash), (float)feature.Value);
        }
		
		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
        /// <param name="feature">The feature.</param>
        public void VisitEnumerize(IFeature<System.Int64> feature)
        {
            var strValue = Convert.ToString(feature.Value);

            this.namespaceBuilder.AddFeature(this.vw.HashFeature(feature.Name + strValue, this.namespaceHash), 1f);
        }

		/// <summary>
        /// Transfers namespace data to native space.
        /// </summary>
        /// <param name="namespaceDense">The dense namespace.</param>
        public void Visit(INamespaceDense<System.Int64> namespaceDense)
        {
            this.featureGroup = namespaceDense.FeatureGroup ?? '\0';

            this.namespaceHash = namespaceDense.Name == null ? 
                this.vw.HashSpace(this.featureGroup.ToString()) :
                this.vw.HashSpace(this.featureGroup + namespaceDense.Name);

            this.namespaceBuilder = this.builder.AddNamespace(this.featureGroup);
            this.namespaceBuilder.PreAllocate(namespaceDense.DenseFeature.Value.Count);

            var i = 0;
            foreach (var v in namespaceDense.DenseFeature.Value)
            {
				#if DEBUG
				if (v > float.MaxValue || v < float.MinValue)
				{
					Trace.TraceWarning("Precision lost for feature value: " + v);
				}
				#endif

                this.namespaceBuilder.AddFeature((uint)(this.namespaceHash + i), (float)v);
                i++;
            }
        }
			    /// <summary>
        /// Transfers feature data to native space.
        /// </summary>
        /// <param name="feature">The feature.</param>
        public void Visit(IFeature<System.UInt64> feature)
        {
#if DEBUG
            if (feature.Value > float.MaxValue || feature.Value < float.MinValue)
            {
                Trace.TraceWarning("Precision lost for feature value: " + feature.Value);
            }
#endif
            this.namespaceBuilder.AddFeature(this.vw.HashFeature(feature.Name, this.namespaceHash), (float)feature.Value);
        }

		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
        /// <param name="feature">The feature.</param>
        public void Visit(IFeature<System.UInt64?> feature)
        {
#if DEBUG
            if (feature.Value > float.MaxValue || feature.Value < float.MinValue)
            {
                Trace.TraceWarning("Precision lost for feature value: " + feature.Value);
            }
#endif
            this.namespaceBuilder.AddFeature(this.vw.HashFeature(feature.Name, this.namespaceHash), (float)feature.Value);
        }
		
		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
        /// <param name="feature">The feature.</param>
        public void VisitEnumerize(IFeature<System.UInt64> feature)
        {
            var strValue = Convert.ToString(feature.Value);

            this.namespaceBuilder.AddFeature(this.vw.HashFeature(feature.Name + strValue, this.namespaceHash), 1f);
        }

		/// <summary>
        /// Transfers namespace data to native space.
        /// </summary>
        /// <param name="namespaceDense">The dense namespace.</param>
        public void Visit(INamespaceDense<System.UInt64> namespaceDense)
        {
            this.featureGroup = namespaceDense.FeatureGroup ?? '\0';

            this.namespaceHash = namespaceDense.Name == null ? 
                this.vw.HashSpace(this.featureGroup.ToString()) :
                this.vw.HashSpace(this.featureGroup + namespaceDense.Name);

            this.namespaceBuilder = this.builder.AddNamespace(this.featureGroup);
            this.namespaceBuilder.PreAllocate(namespaceDense.DenseFeature.Value.Count);

            var i = 0;
            foreach (var v in namespaceDense.DenseFeature.Value)
            {
				#if DEBUG
				if (v > float.MaxValue || v < float.MinValue)
				{
					Trace.TraceWarning("Precision lost for feature value: " + v);
				}
				#endif

                this.namespaceBuilder.AddFeature((uint)(this.namespaceHash + i), (float)v);
                i++;
            }
        }
			    /// <summary>
        /// Transfers feature data to native space.
        /// </summary>
        /// <param name="feature">The feature.</param>
        public void Visit(IFeature<System.Double> feature)
        {
#if DEBUG
            if (feature.Value > float.MaxValue || feature.Value < float.MinValue)
            {
                Trace.TraceWarning("Precision lost for feature value: " + feature.Value);
            }
#endif
            this.namespaceBuilder.AddFeature(this.vw.HashFeature(feature.Name, this.namespaceHash), (float)feature.Value);
        }

		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
        /// <param name="feature">The feature.</param>
        public void Visit(IFeature<System.Double?> feature)
        {
#if DEBUG
            if (feature.Value > float.MaxValue || feature.Value < float.MinValue)
            {
                Trace.TraceWarning("Precision lost for feature value: " + feature.Value);
            }
#endif
            this.namespaceBuilder.AddFeature(this.vw.HashFeature(feature.Name, this.namespaceHash), (float)feature.Value);
        }
		
		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
        /// <param name="feature">The feature.</param>
        public void VisitEnumerize(IFeature<System.Double> feature)
        {
            var strValue = Convert.ToString(feature.Value);

            this.namespaceBuilder.AddFeature(this.vw.HashFeature(feature.Name + strValue, this.namespaceHash), 1f);
        }

		/// <summary>
        /// Transfers namespace data to native space.
        /// </summary>
        /// <param name="namespaceDense">The dense namespace.</param>
        public void Visit(INamespaceDense<System.Double> namespaceDense)
        {
            this.featureGroup = namespaceDense.FeatureGroup ?? '\0';

            this.namespaceHash = namespaceDense.Name == null ? 
                this.vw.HashSpace(this.featureGroup.ToString()) :
                this.vw.HashSpace(this.featureGroup + namespaceDense.Name);

            this.namespaceBuilder = this.builder.AddNamespace(this.featureGroup);
            this.namespaceBuilder.PreAllocate(namespaceDense.DenseFeature.Value.Count);

            var i = 0;
            foreach (var v in namespaceDense.DenseFeature.Value)
            {
				#if DEBUG
				if (v > float.MaxValue || v < float.MinValue)
				{
					Trace.TraceWarning("Precision lost for feature value: " + v);
				}
				#endif

                this.namespaceBuilder.AddFeature((uint)(this.namespaceHash + i), (float)v);
                i++;
            }
        }
		
				 
		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
        /// <param name="feature">The feature.</param>
        public void Visit<TValue>(IFeature<IDictionary<System.Char, TValue>> feature)
        {
            foreach (var kvp in feature.Value)
            {
                this.namespaceBuilder.AddFeature((uint)(this.namespaceHash + kvp.Key), (float)Convert.ToDouble(kvp.Value));
            }
        }

				
		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
        /// <param name="feature">The feature.</param>
        public void Visit(IFeature<IDictionary<System.Char, System.Byte>> feature)
        {
            foreach (var kvp in feature.Value)
            {
                this.namespaceBuilder.AddFeature((uint)(this.namespaceHash + kvp.Key), kvp.Value);
            }
        }
				
		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
        /// <param name="feature">The feature.</param>
        public void Visit(IFeature<IDictionary<System.Char, System.Int32>> feature)
        {
            foreach (var kvp in feature.Value)
            {
                this.namespaceBuilder.AddFeature((uint)(this.namespaceHash + kvp.Key), kvp.Value);
            }
        }
				
		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
        /// <param name="feature">The feature.</param>
        public void Visit(IFeature<IDictionary<System.Char, System.Int16>> feature)
        {
            foreach (var kvp in feature.Value)
            {
                this.namespaceBuilder.AddFeature((uint)(this.namespaceHash + kvp.Key), kvp.Value);
            }
        }
				
		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
        /// <param name="feature">The feature.</param>
        public void Visit(IFeature<IDictionary<System.Char, System.Single>> feature)
        {
            foreach (var kvp in feature.Value)
            {
                this.namespaceBuilder.AddFeature((uint)(this.namespaceHash + kvp.Key), kvp.Value);
            }
        }
		
						
		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
        /// <param name="feature">The feature.</param>
        public void Visit(IFeature<IDictionary<System.Char, System.Int64>> feature)
        {
            foreach (var kvp in feature.Value)
            {
				#if DEBUG
				if (kvp.Value > float.MaxValue || kvp.Value < float.MinValue)
				{
					Trace.TraceWarning("Precision lost for feature value: " + kvp.Value);
				}
				#endif

                this.namespaceBuilder.AddFeature((uint)(this.namespaceHash + kvp.Key), (float)kvp.Value);
            }
        }
						
		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
        /// <param name="feature">The feature.</param>
        public void Visit(IFeature<IDictionary<System.Char, System.Double>> feature)
        {
            foreach (var kvp in feature.Value)
            {
				#if DEBUG
				if (kvp.Value > float.MaxValue || kvp.Value < float.MinValue)
				{
					Trace.TraceWarning("Precision lost for feature value: " + kvp.Value);
				}
				#endif

                this.namespaceBuilder.AddFeature((uint)(this.namespaceHash + kvp.Key), (float)kvp.Value);
            }
        }
		
				 
		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
        /// <param name="feature">The feature.</param>
        public void Visit<TValue>(IFeature<IDictionary<System.Byte, TValue>> feature)
        {
            foreach (var kvp in feature.Value)
            {
                this.namespaceBuilder.AddFeature((uint)(this.namespaceHash + kvp.Key), (float)Convert.ToDouble(kvp.Value));
            }
        }

				
		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
        /// <param name="feature">The feature.</param>
        public void Visit(IFeature<IDictionary<System.Byte, System.Byte>> feature)
        {
            foreach (var kvp in feature.Value)
            {
                this.namespaceBuilder.AddFeature((uint)(this.namespaceHash + kvp.Key), kvp.Value);
            }
        }
				
		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
        /// <param name="feature">The feature.</param>
        public void Visit(IFeature<IDictionary<System.Byte, System.Int32>> feature)
        {
            foreach (var kvp in feature.Value)
            {
                this.namespaceBuilder.AddFeature((uint)(this.namespaceHash + kvp.Key), kvp.Value);
            }
        }
				
		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
        /// <param name="feature">The feature.</param>
        public void Visit(IFeature<IDictionary<System.Byte, System.Int16>> feature)
        {
            foreach (var kvp in feature.Value)
            {
                this.namespaceBuilder.AddFeature((uint)(this.namespaceHash + kvp.Key), kvp.Value);
            }
        }
				
		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
        /// <param name="feature">The feature.</param>
        public void Visit(IFeature<IDictionary<System.Byte, System.Single>> feature)
        {
            foreach (var kvp in feature.Value)
            {
                this.namespaceBuilder.AddFeature((uint)(this.namespaceHash + kvp.Key), kvp.Value);
            }
        }
		
						
		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
        /// <param name="feature">The feature.</param>
        public void Visit(IFeature<IDictionary<System.Byte, System.Int64>> feature)
        {
            foreach (var kvp in feature.Value)
            {
				#if DEBUG
				if (kvp.Value > float.MaxValue || kvp.Value < float.MinValue)
				{
					Trace.TraceWarning("Precision lost for feature value: " + kvp.Value);
				}
				#endif

                this.namespaceBuilder.AddFeature((uint)(this.namespaceHash + kvp.Key), (float)kvp.Value);
            }
        }
						
		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
        /// <param name="feature">The feature.</param>
        public void Visit(IFeature<IDictionary<System.Byte, System.Double>> feature)
        {
            foreach (var kvp in feature.Value)
            {
				#if DEBUG
				if (kvp.Value > float.MaxValue || kvp.Value < float.MinValue)
				{
					Trace.TraceWarning("Precision lost for feature value: " + kvp.Value);
				}
				#endif

                this.namespaceBuilder.AddFeature((uint)(this.namespaceHash + kvp.Key), (float)kvp.Value);
            }
        }
		
				 
		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
        /// <param name="feature">The feature.</param>
        public void Visit<TValue>(IFeature<IDictionary<System.Int32, TValue>> feature)
        {
            foreach (var kvp in feature.Value)
            {
                this.namespaceBuilder.AddFeature((uint)(this.namespaceHash + kvp.Key), (float)Convert.ToDouble(kvp.Value));
            }
        }

				
		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
        /// <param name="feature">The feature.</param>
        public void Visit(IFeature<IDictionary<System.Int32, System.Byte>> feature)
        {
            foreach (var kvp in feature.Value)
            {
                this.namespaceBuilder.AddFeature((uint)(this.namespaceHash + kvp.Key), kvp.Value);
            }
        }
				
		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
        /// <param name="feature">The feature.</param>
        public void Visit(IFeature<IDictionary<System.Int32, System.Int32>> feature)
        {
            foreach (var kvp in feature.Value)
            {
                this.namespaceBuilder.AddFeature((uint)(this.namespaceHash + kvp.Key), kvp.Value);
            }
        }
				
		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
        /// <param name="feature">The feature.</param>
        public void Visit(IFeature<IDictionary<System.Int32, System.Int16>> feature)
        {
            foreach (var kvp in feature.Value)
            {
                this.namespaceBuilder.AddFeature((uint)(this.namespaceHash + kvp.Key), kvp.Value);
            }
        }
				
		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
        /// <param name="feature">The feature.</param>
        public void Visit(IFeature<IDictionary<System.Int32, System.Single>> feature)
        {
            foreach (var kvp in feature.Value)
            {
                this.namespaceBuilder.AddFeature((uint)(this.namespaceHash + kvp.Key), kvp.Value);
            }
        }
		
						
		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
        /// <param name="feature">The feature.</param>
        public void Visit(IFeature<IDictionary<System.Int32, System.Int64>> feature)
        {
            foreach (var kvp in feature.Value)
            {
				#if DEBUG
				if (kvp.Value > float.MaxValue || kvp.Value < float.MinValue)
				{
					Trace.TraceWarning("Precision lost for feature value: " + kvp.Value);
				}
				#endif

                this.namespaceBuilder.AddFeature((uint)(this.namespaceHash + kvp.Key), (float)kvp.Value);
            }
        }
						
		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
        /// <param name="feature">The feature.</param>
        public void Visit(IFeature<IDictionary<System.Int32, System.Double>> feature)
        {
            foreach (var kvp in feature.Value)
            {
				#if DEBUG
				if (kvp.Value > float.MaxValue || kvp.Value < float.MinValue)
				{
					Trace.TraceWarning("Precision lost for feature value: " + kvp.Value);
				}
				#endif

                this.namespaceBuilder.AddFeature((uint)(this.namespaceHash + kvp.Key), (float)kvp.Value);
            }
        }
		
				 
		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
        /// <param name="feature">The feature.</param>
        public void Visit<TValue>(IFeature<IDictionary<System.Int16, TValue>> feature)
        {
            foreach (var kvp in feature.Value)
            {
                this.namespaceBuilder.AddFeature((uint)(this.namespaceHash + kvp.Key), (float)Convert.ToDouble(kvp.Value));
            }
        }

				
		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
        /// <param name="feature">The feature.</param>
        public void Visit(IFeature<IDictionary<System.Int16, System.Byte>> feature)
        {
            foreach (var kvp in feature.Value)
            {
                this.namespaceBuilder.AddFeature((uint)(this.namespaceHash + kvp.Key), kvp.Value);
            }
        }
				
		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
        /// <param name="feature">The feature.</param>
        public void Visit(IFeature<IDictionary<System.Int16, System.Int32>> feature)
        {
            foreach (var kvp in feature.Value)
            {
                this.namespaceBuilder.AddFeature((uint)(this.namespaceHash + kvp.Key), kvp.Value);
            }
        }
				
		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
        /// <param name="feature">The feature.</param>
        public void Visit(IFeature<IDictionary<System.Int16, System.Int16>> feature)
        {
            foreach (var kvp in feature.Value)
            {
                this.namespaceBuilder.AddFeature((uint)(this.namespaceHash + kvp.Key), kvp.Value);
            }
        }
				
		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
        /// <param name="feature">The feature.</param>
        public void Visit(IFeature<IDictionary<System.Int16, System.Single>> feature)
        {
            foreach (var kvp in feature.Value)
            {
                this.namespaceBuilder.AddFeature((uint)(this.namespaceHash + kvp.Key), kvp.Value);
            }
        }
		
						
		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
        /// <param name="feature">The feature.</param>
        public void Visit(IFeature<IDictionary<System.Int16, System.Int64>> feature)
        {
            foreach (var kvp in feature.Value)
            {
				#if DEBUG
				if (kvp.Value > float.MaxValue || kvp.Value < float.MinValue)
				{
					Trace.TraceWarning("Precision lost for feature value: " + kvp.Value);
				}
				#endif

                this.namespaceBuilder.AddFeature((uint)(this.namespaceHash + kvp.Key), (float)kvp.Value);
            }
        }
						
		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
        /// <param name="feature">The feature.</param>
        public void Visit(IFeature<IDictionary<System.Int16, System.Double>> feature)
        {
            foreach (var kvp in feature.Value)
            {
				#if DEBUG
				if (kvp.Value > float.MaxValue || kvp.Value < float.MinValue)
				{
					Trace.TraceWarning("Precision lost for feature value: " + kvp.Value);
				}
				#endif

                this.namespaceBuilder.AddFeature((uint)(this.namespaceHash + kvp.Key), (float)kvp.Value);
            }
        }
		
		    }
}
