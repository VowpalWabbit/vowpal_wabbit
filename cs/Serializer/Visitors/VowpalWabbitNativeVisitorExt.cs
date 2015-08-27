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
using VW.Serializer.Interfaces;

namespace VW.Serializer.Visitors
{
    public partial class VowpalWabbitInterfaceVisitor
    {
			    /// <summary>
        /// Transfers feature data to native space.
        /// </summary>
        /// <param name="feature">The feature.</param>
        public void Visit(IFeature<System.Byte> feature)
        {
			Contract.Requires(feature != null);
            this.namespaceBuilder.AddFeature(this.vw.HashFeature(feature.Name, this.namespaceHash), feature.Value);
        }

		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
        /// <param name="feature">The feature.</param>
        public void Visit(IFeature<System.Byte?> feature)
        {
			Contract.Requires(feature != null);
            this.namespaceBuilder.AddFeature(this.vw.HashFeature(feature.Name, this.namespaceHash), (float)feature.Value);
        }
		/// <summary>
        /// Transfers namespace data to native space.
        /// </summary>
        /// <param name="namespaceDense">The dense namespace.</param>
        public void Visit(INamespaceDense<System.Byte> namespaceDense)
        {
			Contract.Requires(namespaceDense != null);
			Contract.Requires(namespaceDense.DenseFeature != null);

            this.featureGroup = namespaceDense.FeatureGroup ?? '\0';

            this.namespaceHash = namespaceDense.Name == null ? 
                this.vw.HashSpace(this.featureGroup.ToString()) :
                this.vw.HashSpace(this.featureGroup + namespaceDense.Name);

            this.namespaceBuilder = this.builder.AddNamespace(this.featureGroup);
            this.namespaceBuilder.PreAllocate(namespaceDense.DenseFeature.Value.Count);

            var i = 0;

            // support anchor feature
            if(namespaceDense.DenseFeature.AddAnchor)
            {
                this.namespaceBuilder.AddFeature(this.namespaceHash, 1);
                i++;
            }

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
			Contract.Requires(feature != null);
            this.namespaceBuilder.AddFeature(this.vw.HashFeature(feature.Name, this.namespaceHash), feature.Value);
        }

		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
        /// <param name="feature">The feature.</param>
        public void Visit(IFeature<System.SByte?> feature)
        {
			Contract.Requires(feature != null);
            this.namespaceBuilder.AddFeature(this.vw.HashFeature(feature.Name, this.namespaceHash), (float)feature.Value);
        }
		/// <summary>
        /// Transfers namespace data to native space.
        /// </summary>
        /// <param name="namespaceDense">The dense namespace.</param>
        public void Visit(INamespaceDense<System.SByte> namespaceDense)
        {
			Contract.Requires(namespaceDense != null);
			Contract.Requires(namespaceDense.DenseFeature != null);

            this.featureGroup = namespaceDense.FeatureGroup ?? '\0';

            this.namespaceHash = namespaceDense.Name == null ? 
                this.vw.HashSpace(this.featureGroup.ToString()) :
                this.vw.HashSpace(this.featureGroup + namespaceDense.Name);

            this.namespaceBuilder = this.builder.AddNamespace(this.featureGroup);
            this.namespaceBuilder.PreAllocate(namespaceDense.DenseFeature.Value.Count);

            var i = 0;

            // support anchor feature
            if(namespaceDense.DenseFeature.AddAnchor)
            {
                this.namespaceBuilder.AddFeature(this.namespaceHash, 1);
                i++;
            }

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
			Contract.Requires(feature != null);
            this.namespaceBuilder.AddFeature(this.vw.HashFeature(feature.Name, this.namespaceHash), feature.Value);
        }

		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
        /// <param name="feature">The feature.</param>
        public void Visit(IFeature<System.Int16?> feature)
        {
			Contract.Requires(feature != null);
            this.namespaceBuilder.AddFeature(this.vw.HashFeature(feature.Name, this.namespaceHash), (float)feature.Value);
        }
		/// <summary>
        /// Transfers namespace data to native space.
        /// </summary>
        /// <param name="namespaceDense">The dense namespace.</param>
        public void Visit(INamespaceDense<System.Int16> namespaceDense)
        {
			Contract.Requires(namespaceDense != null);
			Contract.Requires(namespaceDense.DenseFeature != null);

            this.featureGroup = namespaceDense.FeatureGroup ?? '\0';

            this.namespaceHash = namespaceDense.Name == null ? 
                this.vw.HashSpace(this.featureGroup.ToString()) :
                this.vw.HashSpace(this.featureGroup + namespaceDense.Name);

            this.namespaceBuilder = this.builder.AddNamespace(this.featureGroup);
            this.namespaceBuilder.PreAllocate(namespaceDense.DenseFeature.Value.Count);

            var i = 0;

            // support anchor feature
            if(namespaceDense.DenseFeature.AddAnchor)
            {
                this.namespaceBuilder.AddFeature(this.namespaceHash, 1);
                i++;
            }

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
			Contract.Requires(feature != null);
            this.namespaceBuilder.AddFeature(this.vw.HashFeature(feature.Name, this.namespaceHash), feature.Value);
        }

		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
        /// <param name="feature">The feature.</param>
        public void Visit(IFeature<System.Int32?> feature)
        {
			Contract.Requires(feature != null);
            this.namespaceBuilder.AddFeature(this.vw.HashFeature(feature.Name, this.namespaceHash), (float)feature.Value);
        }
		/// <summary>
        /// Transfers namespace data to native space.
        /// </summary>
        /// <param name="namespaceDense">The dense namespace.</param>
        public void Visit(INamespaceDense<System.Int32> namespaceDense)
        {
			Contract.Requires(namespaceDense != null);
			Contract.Requires(namespaceDense.DenseFeature != null);

            this.featureGroup = namespaceDense.FeatureGroup ?? '\0';

            this.namespaceHash = namespaceDense.Name == null ? 
                this.vw.HashSpace(this.featureGroup.ToString()) :
                this.vw.HashSpace(this.featureGroup + namespaceDense.Name);

            this.namespaceBuilder = this.builder.AddNamespace(this.featureGroup);
            this.namespaceBuilder.PreAllocate(namespaceDense.DenseFeature.Value.Count);

            var i = 0;

            // support anchor feature
            if(namespaceDense.DenseFeature.AddAnchor)
            {
                this.namespaceBuilder.AddFeature(this.namespaceHash, 1);
                i++;
            }

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
			Contract.Requires(feature != null);
            this.namespaceBuilder.AddFeature(this.vw.HashFeature(feature.Name, this.namespaceHash), feature.Value);
        }

		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
        /// <param name="feature">The feature.</param>
        public void Visit(IFeature<System.UInt16?> feature)
        {
			Contract.Requires(feature != null);
            this.namespaceBuilder.AddFeature(this.vw.HashFeature(feature.Name, this.namespaceHash), (float)feature.Value);
        }
		/// <summary>
        /// Transfers namespace data to native space.
        /// </summary>
        /// <param name="namespaceDense">The dense namespace.</param>
        public void Visit(INamespaceDense<System.UInt16> namespaceDense)
        {
			Contract.Requires(namespaceDense != null);
			Contract.Requires(namespaceDense.DenseFeature != null);

            this.featureGroup = namespaceDense.FeatureGroup ?? '\0';

            this.namespaceHash = namespaceDense.Name == null ? 
                this.vw.HashSpace(this.featureGroup.ToString()) :
                this.vw.HashSpace(this.featureGroup + namespaceDense.Name);

            this.namespaceBuilder = this.builder.AddNamespace(this.featureGroup);
            this.namespaceBuilder.PreAllocate(namespaceDense.DenseFeature.Value.Count);

            var i = 0;

            // support anchor feature
            if(namespaceDense.DenseFeature.AddAnchor)
            {
                this.namespaceBuilder.AddFeature(this.namespaceHash, 1);
                i++;
            }

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
			Contract.Requires(feature != null);
            this.namespaceBuilder.AddFeature(this.vw.HashFeature(feature.Name, this.namespaceHash), feature.Value);
        }

		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
        /// <param name="feature">The feature.</param>
        public void Visit(IFeature<System.UInt32?> feature)
        {
			Contract.Requires(feature != null);
            this.namespaceBuilder.AddFeature(this.vw.HashFeature(feature.Name, this.namespaceHash), (float)feature.Value);
        }
		/// <summary>
        /// Transfers namespace data to native space.
        /// </summary>
        /// <param name="namespaceDense">The dense namespace.</param>
        public void Visit(INamespaceDense<System.UInt32> namespaceDense)
        {
			Contract.Requires(namespaceDense != null);
			Contract.Requires(namespaceDense.DenseFeature != null);

            this.featureGroup = namespaceDense.FeatureGroup ?? '\0';

            this.namespaceHash = namespaceDense.Name == null ? 
                this.vw.HashSpace(this.featureGroup.ToString()) :
                this.vw.HashSpace(this.featureGroup + namespaceDense.Name);

            this.namespaceBuilder = this.builder.AddNamespace(this.featureGroup);
            this.namespaceBuilder.PreAllocate(namespaceDense.DenseFeature.Value.Count);

            var i = 0;

            // support anchor feature
            if(namespaceDense.DenseFeature.AddAnchor)
            {
                this.namespaceBuilder.AddFeature(this.namespaceHash, 1);
                i++;
            }

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
			Contract.Requires(feature != null);
            this.namespaceBuilder.AddFeature(this.vw.HashFeature(feature.Name, this.namespaceHash), feature.Value);
        }

		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
        /// <param name="feature">The feature.</param>
        public void Visit(IFeature<System.Single?> feature)
        {
			Contract.Requires(feature != null);
            this.namespaceBuilder.AddFeature(this.vw.HashFeature(feature.Name, this.namespaceHash), (float)feature.Value);
        }
		/// <summary>
        /// Transfers namespace data to native space.
        /// </summary>
        /// <param name="namespaceDense">The dense namespace.</param>
        public void Visit(INamespaceDense<System.Single> namespaceDense)
        {
			Contract.Requires(namespaceDense != null);
			Contract.Requires(namespaceDense.DenseFeature != null);

            this.featureGroup = namespaceDense.FeatureGroup ?? '\0';

            this.namespaceHash = namespaceDense.Name == null ? 
                this.vw.HashSpace(this.featureGroup.ToString()) :
                this.vw.HashSpace(this.featureGroup + namespaceDense.Name);

            this.namespaceBuilder = this.builder.AddNamespace(this.featureGroup);
            this.namespaceBuilder.PreAllocate(namespaceDense.DenseFeature.Value.Count);

            var i = 0;

            // support anchor feature
            if(namespaceDense.DenseFeature.AddAnchor)
            {
                this.namespaceBuilder.AddFeature(this.namespaceHash, 1);
                i++;
            }

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
			Contract.Requires(feature != null);

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
			Contract.Requires(feature != null);

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
			Contract.Requires(feature != null);

            var strValue = Convert.ToString(feature.Value);

            this.namespaceBuilder.AddFeature(this.vw.HashFeature(feature.Name + strValue, this.namespaceHash), 1f);
        }

		/// <summary>
        /// Transfers namespace data to native space.
        /// </summary>
        /// <param name="namespaceDense">The dense namespace.</param>
        public void Visit(INamespaceDense<System.Int64> namespaceDense)
        {
			Contract.Requires(namespaceDense != null);
			Contract.Requires(namespaceDense.DenseFeature != null);

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
			Contract.Requires(feature != null);

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
			Contract.Requires(feature != null);

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
			Contract.Requires(feature != null);

            var strValue = Convert.ToString(feature.Value);

            this.namespaceBuilder.AddFeature(this.vw.HashFeature(feature.Name + strValue, this.namespaceHash), 1f);
        }

		/// <summary>
        /// Transfers namespace data to native space.
        /// </summary>
        /// <param name="namespaceDense">The dense namespace.</param>
        public void Visit(INamespaceDense<System.UInt64> namespaceDense)
        {
			Contract.Requires(namespaceDense != null);
			Contract.Requires(namespaceDense.DenseFeature != null);

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
			Contract.Requires(feature != null);

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
			Contract.Requires(feature != null);

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
			Contract.Requires(feature != null);

            var strValue = Convert.ToString(feature.Value);

            this.namespaceBuilder.AddFeature(this.vw.HashFeature(feature.Name + strValue, this.namespaceHash), 1f);
        }

		/// <summary>
        /// Transfers namespace data to native space.
        /// </summary>
        /// <param name="namespaceDense">The dense namespace.</param>
        public void Visit(INamespaceDense<System.Double> namespaceDense)
        {
			Contract.Requires(namespaceDense != null);
			Contract.Requires(namespaceDense.DenseFeature != null);

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
		[ContractVerification(false)]
        public void Visit<TValue>(IFeature<IDictionary<System.Char, TValue>> feature)
        {
			Contract.Requires(feature != null);

            foreach (var kvp in feature.Value)
            {
                this.namespaceBuilder.AddFeature((uint)(this.namespaceHash + kvp.Key), (float)Convert.ToDouble(kvp.Value));
            }
        }

				
		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
        /// <param name="feature">The feature.</param>
		[ContractVerification(false)]
        public void Visit(IFeature<IDictionary<System.Char, System.Byte>> feature)
        {
			Contract.Requires(feature != null);

            foreach (var kvp in feature.Value)
            {
                this.namespaceBuilder.AddFeature((uint)(this.namespaceHash + kvp.Key), kvp.Value);
            }
        }
				
		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
        /// <param name="feature">The feature.</param>
		[ContractVerification(false)]
        public void Visit(IFeature<IDictionary<System.Char, System.Int32>> feature)
        {
			Contract.Requires(feature != null);

            foreach (var kvp in feature.Value)
            {
                this.namespaceBuilder.AddFeature((uint)(this.namespaceHash + kvp.Key), kvp.Value);
            }
        }
				
		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
        /// <param name="feature">The feature.</param>
		[ContractVerification(false)]
        public void Visit(IFeature<IDictionary<System.Char, System.Int16>> feature)
        {
			Contract.Requires(feature != null);

            foreach (var kvp in feature.Value)
            {
                this.namespaceBuilder.AddFeature((uint)(this.namespaceHash + kvp.Key), kvp.Value);
            }
        }
				
		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
        /// <param name="feature">The feature.</param>
		[ContractVerification(false)]
        public void Visit(IFeature<IDictionary<System.Char, System.Single>> feature)
        {
			Contract.Requires(feature != null);

            foreach (var kvp in feature.Value)
            {
                this.namespaceBuilder.AddFeature((uint)(this.namespaceHash + kvp.Key), kvp.Value);
            }
        }
		
						
		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
        /// <param name="feature">The feature.</param>
		[ContractVerification(false)]
        public void Visit(IFeature<IDictionary<System.Char, System.Int64>> feature)
        {
			Contract.Requires(feature != null);

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
		[ContractVerification(false)]
        public void Visit(IFeature<IDictionary<System.Char, System.Double>> feature)
        {
			Contract.Requires(feature != null);

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
		[ContractVerification(false)]
        public void Visit<TValue>(IFeature<IDictionary<System.Byte, TValue>> feature)
        {
			Contract.Requires(feature != null);

            foreach (var kvp in feature.Value)
            {
                this.namespaceBuilder.AddFeature((uint)(this.namespaceHash + kvp.Key), (float)Convert.ToDouble(kvp.Value));
            }
        }

				
		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
        /// <param name="feature">The feature.</param>
		[ContractVerification(false)]
        public void Visit(IFeature<IDictionary<System.Byte, System.Byte>> feature)
        {
			Contract.Requires(feature != null);

            foreach (var kvp in feature.Value)
            {
                this.namespaceBuilder.AddFeature((uint)(this.namespaceHash + kvp.Key), kvp.Value);
            }
        }
				
		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
        /// <param name="feature">The feature.</param>
		[ContractVerification(false)]
        public void Visit(IFeature<IDictionary<System.Byte, System.Int32>> feature)
        {
			Contract.Requires(feature != null);

            foreach (var kvp in feature.Value)
            {
                this.namespaceBuilder.AddFeature((uint)(this.namespaceHash + kvp.Key), kvp.Value);
            }
        }
				
		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
        /// <param name="feature">The feature.</param>
		[ContractVerification(false)]
        public void Visit(IFeature<IDictionary<System.Byte, System.Int16>> feature)
        {
			Contract.Requires(feature != null);

            foreach (var kvp in feature.Value)
            {
                this.namespaceBuilder.AddFeature((uint)(this.namespaceHash + kvp.Key), kvp.Value);
            }
        }
				
		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
        /// <param name="feature">The feature.</param>
		[ContractVerification(false)]
        public void Visit(IFeature<IDictionary<System.Byte, System.Single>> feature)
        {
			Contract.Requires(feature != null);

            foreach (var kvp in feature.Value)
            {
                this.namespaceBuilder.AddFeature((uint)(this.namespaceHash + kvp.Key), kvp.Value);
            }
        }
		
						
		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
        /// <param name="feature">The feature.</param>
		[ContractVerification(false)]
        public void Visit(IFeature<IDictionary<System.Byte, System.Int64>> feature)
        {
			Contract.Requires(feature != null);

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
		[ContractVerification(false)]
        public void Visit(IFeature<IDictionary<System.Byte, System.Double>> feature)
        {
			Contract.Requires(feature != null);

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
		[ContractVerification(false)]
        public void Visit<TValue>(IFeature<IDictionary<System.Int32, TValue>> feature)
        {
			Contract.Requires(feature != null);

            foreach (var kvp in feature.Value)
            {
                this.namespaceBuilder.AddFeature((uint)(this.namespaceHash + kvp.Key), (float)Convert.ToDouble(kvp.Value));
            }
        }

				
		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
        /// <param name="feature">The feature.</param>
		[ContractVerification(false)]
        public void Visit(IFeature<IDictionary<System.Int32, System.Byte>> feature)
        {
			Contract.Requires(feature != null);

            foreach (var kvp in feature.Value)
            {
                this.namespaceBuilder.AddFeature((uint)(this.namespaceHash + kvp.Key), kvp.Value);
            }
        }
				
		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
        /// <param name="feature">The feature.</param>
		[ContractVerification(false)]
        public void Visit(IFeature<IDictionary<System.Int32, System.Int32>> feature)
        {
			Contract.Requires(feature != null);

            foreach (var kvp in feature.Value)
            {
                this.namespaceBuilder.AddFeature((uint)(this.namespaceHash + kvp.Key), kvp.Value);
            }
        }
				
		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
        /// <param name="feature">The feature.</param>
		[ContractVerification(false)]
        public void Visit(IFeature<IDictionary<System.Int32, System.Int16>> feature)
        {
			Contract.Requires(feature != null);

            foreach (var kvp in feature.Value)
            {
                this.namespaceBuilder.AddFeature((uint)(this.namespaceHash + kvp.Key), kvp.Value);
            }
        }
				
		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
        /// <param name="feature">The feature.</param>
		[ContractVerification(false)]
        public void Visit(IFeature<IDictionary<System.Int32, System.Single>> feature)
        {
			Contract.Requires(feature != null);

            foreach (var kvp in feature.Value)
            {
                this.namespaceBuilder.AddFeature((uint)(this.namespaceHash + kvp.Key), kvp.Value);
            }
        }
		
						
		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
        /// <param name="feature">The feature.</param>
		[ContractVerification(false)]
        public void Visit(IFeature<IDictionary<System.Int32, System.Int64>> feature)
        {
			Contract.Requires(feature != null);

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
		[ContractVerification(false)]
        public void Visit(IFeature<IDictionary<System.Int32, System.Double>> feature)
        {
			Contract.Requires(feature != null);

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
		[ContractVerification(false)]
        public void Visit<TValue>(IFeature<IDictionary<System.Int16, TValue>> feature)
        {
			Contract.Requires(feature != null);

            foreach (var kvp in feature.Value)
            {
                this.namespaceBuilder.AddFeature((uint)(this.namespaceHash + kvp.Key), (float)Convert.ToDouble(kvp.Value));
            }
        }

				
		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
        /// <param name="feature">The feature.</param>
		[ContractVerification(false)]
        public void Visit(IFeature<IDictionary<System.Int16, System.Byte>> feature)
        {
			Contract.Requires(feature != null);

            foreach (var kvp in feature.Value)
            {
                this.namespaceBuilder.AddFeature((uint)(this.namespaceHash + kvp.Key), kvp.Value);
            }
        }
				
		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
        /// <param name="feature">The feature.</param>
		[ContractVerification(false)]
        public void Visit(IFeature<IDictionary<System.Int16, System.Int32>> feature)
        {
			Contract.Requires(feature != null);

            foreach (var kvp in feature.Value)
            {
                this.namespaceBuilder.AddFeature((uint)(this.namespaceHash + kvp.Key), kvp.Value);
            }
        }
				
		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
        /// <param name="feature">The feature.</param>
		[ContractVerification(false)]
        public void Visit(IFeature<IDictionary<System.Int16, System.Int16>> feature)
        {
			Contract.Requires(feature != null);

            foreach (var kvp in feature.Value)
            {
                this.namespaceBuilder.AddFeature((uint)(this.namespaceHash + kvp.Key), kvp.Value);
            }
        }
				
		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
        /// <param name="feature">The feature.</param>
		[ContractVerification(false)]
        public void Visit(IFeature<IDictionary<System.Int16, System.Single>> feature)
        {
			Contract.Requires(feature != null);

            foreach (var kvp in feature.Value)
            {
                this.namespaceBuilder.AddFeature((uint)(this.namespaceHash + kvp.Key), kvp.Value);
            }
        }
		
						
		/// <summary>
        /// Transfers feature data to native space.
        /// </summary>
        /// <param name="feature">The feature.</param>
		[ContractVerification(false)]
        public void Visit(IFeature<IDictionary<System.Int16, System.Int64>> feature)
        {
			Contract.Requires(feature != null);

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
		[ContractVerification(false)]
        public void Visit(IFeature<IDictionary<System.Int16, System.Double>> feature)
        {
			Contract.Requires(feature != null);

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
