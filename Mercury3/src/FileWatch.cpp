#include <Windows.h>
#include <stdio.h>
#include <wchar.h>

#include <FileWatch.h>
#include <mutex>

static FileWatcher watcher;
static std::mutex watcherMutex;

static uint64_t _GetFileTime(const char* path) {
	FILETIME filetime;
	wchar_t wstr[1000];
	size_t cSize;

	mbstowcs_s(&cSize, wstr, 999, path, strlen(path));

	HANDLE file_handle = CreateFile(wstr, GENERIC_READ,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (file_handle != INVALID_HANDLE_VALUE) {
		GetFileTime(file_handle, NULL, NULL, &filetime);
		CloseHandle(file_handle);
		uint64_t t = (((uint64_t)filetime.dwHighDateTime) << 32) + filetime.dwLowDateTime;
		return t;
	}
	else {
		fprintf(stderr, "Unable to open file: %s\n", path);
	}
	return 0;
}

void FileWatcher::watchFile(std::string path, CallbackFunction callback)
{
	FileInfo info;
	info.mod_time = _GetFileTime(path.c_str());
	info.path = std::move(path);
	info.callback = callback;

	m_fileCallbacks.push_back(info);

	fprintf(stdout, "Watching:%s\n", info.path.c_str());
}

void FileWatcher::checkForChange()
{
	for (auto& info : m_fileCallbacks)
	{
		uint64_t t = _GetFileTime(info.path.c_str());
		if (t != info.mod_time) {
			fprintf(stdout, "File changed:%s\n", info.path.c_str());
			info.mod_time = t;
			info.callback();
		}
	}
}

//void WatchFileForChange(const char* path, FileWatcher::CallbackFunction clbk)
//{
//	std::lock_guard<std::mutex> lock(watcherMutex);
//	watcher.watchFile(path, clbk);
//}

void WatchFileForChange(std::string path, FileWatcher::CallbackFunction clbk)
{
	std::lock_guard<std::mutex> lock(watcherMutex);
	watcher.watchFile(std::move(path), clbk);
}

void CheckFilesForChange() {
	std::lock_guard<std::mutex> lock(watcherMutex);
	watcher.checkForChange();
}