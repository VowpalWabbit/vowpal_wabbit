// --------------------------------------------------------------------------------------------------------------------
// <copyright file="ObjectFactory.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using System;

namespace VW
{
    /// <summary>
    /// Helper to conveniently create <see cref="ObjectFactory{TSource,TObject}"/>.
    /// </summary>
    public static class ObjectFactory
    {
        /// <summary>
        /// Disposable object factory.
        /// </summary>
        /// <typeparam name="TSource">The disposable context needed to create objects of <typeparamref name="TObject"/>.</typeparam>
        /// <typeparam name="TObject">The type of the objects to be created.</typeparam>
        public static ObjectFactory<TSource, TObject> Create<TSource, TObject>(TSource context, Func<TSource, TObject> creator)
            where TSource : class, IDisposable
        {
            return new ObjectFactory<TSource,TObject>(context, creator);
        }
    }

    /// <summary>
    /// Disposable object factory.
    /// </summary>
    /// <typeparam name="TSource">The disposable context needed to create objects of <typeparamref name="TObject"/>.</typeparam>
    /// <typeparam name="TObject">The type of the objects to be created.</typeparam>
    public class ObjectFactory<TSource, TObject> : IDisposable
        where TSource : class, IDisposable
    {
        /// <summary>
        /// Factory function to create new instances.
        /// </summary>
        private readonly Func<TSource, TObject> creator;

        /// <summary>
        /// The source object passed to <see cref="creator"/>.
        /// </summary>
        private TSource source;

        /// <summary>
        /// True if this instance is already disposed.
        /// </summary>
        private bool disposed;

        internal ObjectFactory(TSource source, Func<TSource, TObject> creator)
        {
            this.source = source;
            this.creator = creator;
            this.disposed = false;
        }

        /// <summary>
        /// Creates a new object of type T.
        /// </summary>
        public TObject Create()
        {
            return this.creator(source);
        }

        /// <summary>
        /// Cleanup.
        /// </summary>
        public void Dispose()
        {
            this.Dispose(true);
            GC.SuppressFinalize(this);
        }

        private void Dispose(bool disposing)
        {
            if (disposing)
            {
                if (!disposed)
                {
                    if (this.source != null)
                    {
                        this.source.Dispose();
                        this.source = null;
                    }
                    this.disposed = true;
                }
            }
        }
    }
}
