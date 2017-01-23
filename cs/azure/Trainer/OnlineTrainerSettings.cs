// --------------------------------------------------------------------------------------------------------------------
// <copyright file="OnlineTrainerSettings.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

namespace VW.Azure.Trainer
{
    /// <summary>
    /// Settings for the online trainer.
    /// </summary>
    public class OnlineTrainerSettings
    {
        /// <summary>
        /// Azure storage container name containing the latest model.
        /// </summary>
        public const string ModelContainerName = "mwt-models";

        /// <summary>
        /// Azure storage blob name of the latest model.
        /// </summary>
        public const string LatestModelBlobName = "current";

        /// <summary>
        /// Azure storage container name containing the latest settings.
        /// </summary>
        public const string SettingsContainerName = "mwt-settings";

        /// <summary>
        /// Azure storage blob name of the latest settings.
        /// </summary>
        public const string LatestClientSettingsBlobName = "client";

        /// <summary>
        /// Application ID used by performance counter instance name.
        /// </summary>
        public string ApplicationID { get; set; }

        /// <summary>
        /// Training arguments to be used in training service.
        /// </summary>
        public string TrainArguments { get; set; }
    }
}
