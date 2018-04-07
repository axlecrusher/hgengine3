#pragma once

#include <map>
#include <string>
#include <vector>

#include <ini.h>

namespace IniLoader {
	//struct {
	//	std::string name;
	//	std::string value;
	//} value_pairs;

	typedef std::map < std::string, std::string > value_pairs;

	typedef struct ini_section {
		std::string header;
		value_pairs pairs;
	} ini_section;

	typedef std::map < std::string, ini_section > ini_contents;

	const std::string& getValue(const ini_contents& c, const std::string& sec, const std::string& name);

	class Contents {
	public:
		inline const std::string& getValue(const std::string& sec, const std::string& name) {
			return IniLoader::getValue(m_ini, sec, name);
		}

	private:
		ini_contents m_ini;
		friend Contents parse(const std::string &filename);
	};

	Contents parse(const std::string &filename);
}

//class IniRegistrar {
//public:
//	IniRegistrar(std::string tag, std::string value);
//};
