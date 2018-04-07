#pragma once

#include <HgTypes.h>
#include <quaternion.h>
#include <string>

namespace StringConverters {
	template<typename T>
	bool convertValue(const std::string& str_val, T& value) { }

	template<>
	inline bool convertValue(const std::string& str_val, float& value) {
		if (str_val.empty()) return false;
		float x = (float)::atof(str_val.c_str());
		value = x;
		return true;
	}

	inline bool convertValue(const std::string& str_val, vector3& value) {
		float x, y, z;
		int r = sscanf(str_val.c_str(), "%f,%f,%f", &x, &y, &z);
		if (r == 3) {
			value = { x,y,z };
			return true;
		}
		//warn
		return false;
	}

	inline bool convertValue(const std::string& str_val, quaternion& value) {
		float x, y, z;
		int r = sscanf(str_val.c_str(), "%f,%f,%f", &x, &y, &z);
		if (r == 3) {
			value = toQuaternion2(y, x, z); //y,x,z
			return true;
		}
		//warn
		return false;
	}
}