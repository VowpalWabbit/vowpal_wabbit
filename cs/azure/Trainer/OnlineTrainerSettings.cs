// --------------------------------------------------------------------------------------------------------------------
// <copyright file="OnlineTrainerSettings.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

namespace VW.Azure.Trainer
{
    public class OnlineTrainerSettings
    {
        // Model blobs
        public const string ModelContainerName = "mwt-models";
        public const string LatestModelBlobName = "current";

        // Settings blobs
        public const string SettingsContainerName = "mwt-settings";
        public const string LatestClientSettingsBlobName = "client";

        public string ApplicationID { get; set; }

        /// <summary>
        /// Training arguments to be used in training service.
        /// </summary>
        public string TrainArguments { get; set; }
    }
}
