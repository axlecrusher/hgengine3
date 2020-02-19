#pragma once

#include <string>
#include <ft2build.h>
#include FT_FREETYPE_H

class HgFreetype
{
public:
	HgFreetype();

	bool LoadFont(const std::string& path);
	void BuildDigest();

private:
	FT_Library m_freeType;
	FT_Face m_face;

};