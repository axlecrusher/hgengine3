#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

typedef void(*fileChangedCallback)(void *data);

void WatchFileForChange(const char* path, fileChangedCallback clbk, void *clbkData);
void CheckFilesForChange();

#ifdef __cplusplus
}
#endif