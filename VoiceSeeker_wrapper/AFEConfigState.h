/*----------------------------------------------------------------------------
	Copyright 2020-2021 NXP
	SPDX-License-Identifier: BSD-3-Clause
----------------------------------------------------------------------------*/
#pragma once

#include <map>
#include <string>

using namespace std;
namespace AFEConfig
{
	typedef struct mic_geometry {
		float x;
		float y;
		float z;
	} mic_xyz;

	class AFEConfigState
	{
		public:
			AFEConfigState();
			~AFEConfigState();

			int isConfigurationEnable(const string config, int defaultState) const;
			mic_xyz isConfigurationEnable(const string config, mic_xyz defaultState) const;
			string isConfigurationEnable(const string config, string defaultState) const;
		private:
			string ConfigIni;
			map<string, int> ConfigMap;
			map<string, mic_xyz> ConfigXYZ;
			map<string, string> ConfigStr;
	};
}
