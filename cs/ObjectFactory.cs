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
    {
        public static ObjectFactory<TContext, TObject> Create<TContext, TObject>(TContext context, Func<TContext, TObject> creator)
            where TContext : IDisposable
        {
            return new ObjectFactory<TContext,TObject>(context, creator);
        }
    }

    /// <summary>
    /// Disposable object factory.
    /// </summary>
    /// <typeparam name="TContext">The disposable context needed to create objects of <typeparamref name="TObject"/>.</typeparam>
    /// <typeparam name="TObject">The type of the objects to be created.</typeparam>
    public class ObjectFactory<TContext, TObject> : IDisposable
        where TContext : IDisposable
    {
        private readonly Func<TContext, TObject> creator;
        
        private TContext context;

        private bool disposed;

        internal ObjectFactory(TContext context, Func<TContext, TObject> creator)
        {
            this.context = context;
            this.creator = creator;
            this.disposed = false;
        }

        /// <summary>
        /// Creates a new object of type T.
        /// </summary>
        public TObject Create()
        {
            return this.creator(context);
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
                    this.context.Dispose();
                    this.context = default(TContext);
                    this.disposed = true;
                }
            }
        }
    }
}
