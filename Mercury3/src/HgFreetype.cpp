#include "HgFreetype.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

HgFreetype::HgFreetype()
{
	memset(&m_freeType, 0, sizeof(FT_Library));
	memset(&m_face, 0, sizeof(FT_Face));

	auto error = FT_Init_FreeType(&m_freeType);
	if (error)
	{
		fprintf(stderr, "Error init freetype library %d\n", error);
	}
}

bool HgFreetype::LoadFont(const std::string& path)
{
	auto error = FT_New_Face(m_freeType, path.c_str(), 0, &m_face);

	if (error != 0)
	{
		fprintf(stderr, "Error loading font \"%s\", %d\n", path.c_str(), error);
		return false;
	}

	uint8_t pt = 64;

	error = FT_Set_Char_Size(m_face, 0,
		pt * 64,   // char_height in 1/64th of points
		72,     // horizontal device resolution
		72);   // vertical device resolution

	return true;
}

void HgFreetype::BuildDigest()
{
	for (char c = 'A'; c <= 'Z'; c++)
	{
		auto glyphIxd = FT_Get_Char_Index(m_face, c);
		auto error = FT_Load_Glyph(m_face, glyphIxd, FT_LOAD_DEFAULT);
		error = FT_Render_Glyph(m_face->glyph, FT_RENDER_MODE_NORMAL);

		char img[] = "font_C.bmp";
		img[5] = c;

		int r = stbi_write_bmp(img, m_face->glyph->bitmap.width, m_face->glyph->bitmap.rows, 1, m_face->glyph->bitmap.buffer);
	}

	for (char c = 'a'; c <= 'z'; c++)
	{
		auto glyphIxd = FT_Get_Char_Index(m_face, c);
		auto error = FT_Load_Glyph(m_face, glyphIxd, FT_LOAD_DEFAULT);
		error = FT_Render_Glyph(m_face->glyph, FT_RENDER_MODE_NORMAL);

		char img[] = "font_lC.bmp";
		img[6] = c;

		int r = stbi_write_bmp(img, m_face->glyph->bitmap.width, m_face->glyph->bitmap.rows, 1, m_face->glyph->bitmap.buffer);
	}

	for (char c = '0'; c <= '9'; c++)
	{
		auto glyphIxd = FT_Get_Char_Index(m_face, c);
		auto error = FT_Load_Glyph(m_face, glyphIxd, FT_LOAD_DEFAULT);
		error = FT_Render_Glyph(m_face->glyph, FT_RENDER_MODE_NORMAL);

		char img[] = "font_nC.bmp";
		img[6] = c;

		int r = stbi_write_bmp(img, m_face->glyph->bitmap.width, m_face->glyph->bitmap.rows, 1, m_face->glyph->bitmap.buffer);
	}
}