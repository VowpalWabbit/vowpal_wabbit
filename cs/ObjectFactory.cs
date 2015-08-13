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
    public static class ObjectFactory
    {    /// <summary>
        /// Disposable object factory.
        /// </summary>
        /// <typeparam name="TSource">The disposable context needed to create objects of <typeparamref name="TObject"/>.</typeparam>
        /// <typeparam name="TObject">The type of the objects to be created.</typeparam>
        public static ObjectFactory<TSource, TObject> Create<TSource, TObject>(TSource context, Func<TSource, TObject> creator)
            where TSource : IDisposable
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
        where TSource : IDisposable
    {
        private readonly Func<TSource, TObject> creator;
        
        private TSource source;

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
                    this.source.Dispose();
                    this.source = default(TSource);
                    this.disposed = true;
                }
            }
        }
    }
}
