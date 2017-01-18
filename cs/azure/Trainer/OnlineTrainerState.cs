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
    public class OnlineTrainerState
    {
        public OnlineTrainerState()
        {
            this.Partitions = new Dictionary<string, string>();
            this.PartitionsDateTime = new Dictionary<string, DateTime>();
        }

        public Dictionary<string, string> Partitions { get; private set; }

        public Dictionary<string, DateTime> PartitionsDateTime { get; private set; }

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

        public string ModelName { get; set; }
    }
}
