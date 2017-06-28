#pragma once

void WatchFileForChange(const char* path, void (*changed)(void *data), void *clbkData);
void CheckFilesForChange();