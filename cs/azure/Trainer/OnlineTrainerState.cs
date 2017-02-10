// --------------------------------------------------------------------------------------------------------------------
// <copyright file="OnlineTrainerState.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Linq;

namespace VW.Azure.Trainer
{
    /// <summary>
    /// Online trainer state structure used to serialize state to state.json.
    /// </summary>
    public class OnlineTrainerState
    {
        /// <summary>
        /// Initializes a new <see cref="OnlineTrainerState"/> instance.
        /// </summary>
        public OnlineTrainerState()
        {
            this.Partitions = new Dictionary<string, string>();
            this.PartitionsDateTime = new Dictionary<string, DateTime>();
        }

        /// <summary>
        /// Current EventHub state.
        /// </summary>
        public Dictionary<string, string> Partitions { get; private set; }

        /// <summary>
        /// Current EventHub state using data time.
        /// </summary>
        public Dictionary<string, DateTime> PartitionsDateTime { get; private set; }

        /// <summary>
        /// Union of <see cref="Partitions"/> and <see cref="PartitionsDateTime"/>. 
        /// </summary>
        [JsonIgnore]
        public Dictionary<string, string> PartitionsDetailed
        {
            get
            {
                // PartitionsDetailed
                return this.Partitions.Union(this.PartitionsDateTime.Select(kv => new KeyValuePair<string, string>(kv.Key, kv.Value.ToString("u"))))
                        .GroupBy(kv => kv.Key)
                        .ToDictionary(kv => kv.Key, group => string.Join(";", group.Select(kv => kv.Value)));
            }
        }

        /// <summary>
        /// The models name (timestamp + name).
        /// </summary>
        public string ModelName { get; set; }
    }
}
