/*----------------------------------------------------------------------------
	Copyright 2020-2021 NXP
	SPDX-License-Identifier: BSD-3-Clause
----------------------------------------------------------------------------*/

#include <AFEConfigState.h>
#include <algorithm>
#include <fstream>
#include <iostream>

namespace AFEConfig
{
	AFEConfigState::AFEConfigState()
	{
		const string configIni = "/unit_tests/nxp-afe/Config.ini";
		ifstream configuration(configIni.c_str());
		if (configuration.good())
		{
			std::string line;
			while (getline(configuration, line))
			{
				line.erase(std::remove(line.begin(), line.end(), ' '),line.end());
				if (line[0] == '#' || line.empty()) continue;

				auto delimiterPos = line.find("=");
				auto key = line.substr(0, delimiterPos);
				auto value = line.substr(delimiterPos + 1);
				//mic xyz
				if ((delimiterPos = value.find(',')) != string::npos)
				{
					mic_xyz mic;
					mic.x = atof(value.substr(0, delimiterPos).c_str());
					auto delimiterPos1 = value.find(',', delimiterPos + 1);
					mic.y = atof(value.substr(delimiterPos + 1, delimiterPos1).c_str());
					mic.z = atof(value.substr(delimiterPos1 + 1).c_str());
					ConfigXYZ.insert(std::pair<string,mic_xyz>(key, mic));
					continue;
				}

				ConfigMap.insert(std::pair<string,int>(key, atoi(value.c_str())));
			}
		}
		else
		{
			std::cout << "No config file provide, using default settings" << std::endl;
		}
	}

	AFEConfigState::~AFEConfigState()
	{
	}

	int AFEConfigState::isConfigurationEnable(const string config, int defaultState) const
	{
		int value;
		try{
			value = ConfigMap.at(config);
		} catch (const std::out_of_range&)
		{
			value = defaultState;
		}
		return value;
	}

	mic_xyz AFEConfigState::isConfigurationEnable(const string config, mic_xyz defaultState) const
	{
		mic_xyz value;
		try{
			value = ConfigXYZ.at(config);
		} catch (const std::out_of_range&)
		{
			value = defaultState;
		}
		return value;
	}
}
