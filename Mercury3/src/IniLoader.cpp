#include <map>
#include <string>
#include <IniLoader.h>
#include <algorithm>
#include <string> 

static std::string emptyString;

namespace IniLoader {
	static std::string toLower(const char* str) {
		std::string tmp(str);
		std::transform(tmp.begin(), tmp.end(), tmp.begin(), ::tolower);
		return tmp; //RVO
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
		return 1; //non-zero on success
	}

	void handleError(int r, const std::string &filename) {
		if (r == -1) {
			fprintf(stderr, "Error opening file \"%s\"\n", filename.c_str());
		}
		else if (r == -2) {
			fprintf(stderr, "Memory alignment error opening file \"%s\"\n", filename.c_str());
		}
		else if (r > 0) {
			fprintf(stderr, "Error on line %d in file \"%s\"\n", r, filename.c_str());
		}
		else if (r != 0) {
			fprintf(stderr, "Unknown error opening file \"%s\"\n", filename.c_str());
		}
	}

	Contents parse(const std::string &filename) {
		Contents contents;
		int r = ini_parse(filename.c_str(), &sectionHandler, &contents.m_ini);
		if (r != 0) {
			handleError(r, filename);
		}
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