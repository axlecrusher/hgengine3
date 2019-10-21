#pragma once

#include <string>
#include <vector>
#include <functional>

class FileWatcher
{
public:
	typedef std::function<void()> CallbackFunction;
	void watchFile(std::string path, CallbackFunction callback);
	void checkForChange();
private:
	typedef struct FileInfo {
		std::string path;
		uint64_t mod_time;
		CallbackFunction callback;
	} FileInfo;

	std::vector< FileInfo > m_fileCallbacks;
};

void WatchFileForChange(const char* path, FileWatcher::CallbackFunction clbk);
void CheckFilesForChange();
