#include <HgVbo.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#include <string.h>
#include <RenderBackend.h>

HgVboMemory<vbo_layout_vc> staticVbo;
HgVboMemory<vbo_layout_vnu> staticVboVNU;
HgVboMemory<vbo_layout_vnut> staticVboVNUT;

//static void* _currentVbo;




template<typename T>
void HgVboMemory<T>::use() {
	if ((_currentVbo == this) && (needsUpdate == false)) return;
	_currentVbo = this;
	use_common();
}

//8 bit index
template<>
void HgVboMemory<uint8_t>::use() {
	use_common();
}

//16 bit index
template<>
void HgVboMemory<uint16_t>::use() {
	use_common();
}

template<>
void HgVboMemory<color>::use() {
	use_common();
}

