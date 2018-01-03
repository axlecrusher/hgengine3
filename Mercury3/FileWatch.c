#include <Windows.h>
#include <stdint.h>
#include <stdio.h>
#include <wchar.h>
#include <stddef.h>

#include <LinkedList.h>
#include <str_utils.h>
#include <FileWatch.h>

static ListItem* listHead = NULL;

typedef struct FileInfo {
	char* path;
	uint64_t mod_time;
	fileChangedCallback changed;
	void* clbkData;
} FileInfo;

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

void WatchFileForChange(const char* path, fileChangedCallback clbk, void *clbkData) {
	uint64_t time = _GetFileTime(path);

	FileInfo* info = malloc(sizeof *info);
	info->path = str_copy(path);
	info->mod_time = time;
	info->changed = clbk;
	info->clbkData = clbkData;

	push(&listHead, info);
	fprintf(stderr, "Watching:%s\n", path);
}

void CheckFilesForChange() {
	ListItem* item = listHead;
	while (item != NULL) {
		FileInfo* info = item->data;
		uint64_t t = _GetFileTime(info->path);
		if (t != info->mod_time) {
			info->mod_time = t;
			info->changed(info->clbkData);
		}
		item = next_item(item);
	}
}