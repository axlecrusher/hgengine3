#include <map>
#include <string>
#include <IniLoader.h>
#include <algorithm>
#include <string> 

static std::string emptyString;

//static std::map<std::string, std::string> handlers;
namespace IniLoader {
	static std::string toLower(const char* str) {
		std::string tmp(str);
		std::transform(tmp.begin(), tmp.end(), tmp.begin(), ::tolower);
		return tmp; //RVO
//		return std::move(tmp);
	}

	int sectionHandler(void* user, const char* _section, const char* _name, const char* value) {
		ini_contents* contents = (ini_contents*)user;
		std::string sec_name = toLower(_section);
		std::string name = toLower(_name);

		auto tmp = contents->find(sec_name);
		if (tmp != contents->end()) {
			ini_section& s = tmp->second;
			s.pairs.erase(name);
			s.pairs.insert(std::make_pair(std::move(name), std::string(value)));
		}
		else {
			ini_section s;
			s.header = sec_name;
			s.pairs.insert(std::make_pair(std::move(name), std::string(value)));
			contents->insert(std::make_pair(std::move(sec_name), std::move(s)));
		}
		return 0;
	}

	Contents parse(const std::string &filename) {
		Contents contents;
		ini_parse(filename.c_str(), &sectionHandler, &contents.m_ini);
		return contents; //std::move is not a good idea here, prevents RVO
	}

	const std::string& getValue(const ini_contents& c, const std::string& sec, const std::string& name) {
		auto t1 = c.find(sec);
		if (t1 == c.end()) return emptyString;

		const ini_section& section = t1->second;
		auto t2 = section.pairs.find(name);
		if (t2 == section.pairs.end()) return emptyString;

		return t2->second;
	}
}
//
//IniRegistrar::IniRegistrar(std::string tag, std::string value) {
//	auto t = std::make_pair(tag, value);
//	handlers.insert(t);
//}