#include "HgFreetype.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
#include <HgTexture.h>
#include <HgVbo.h>

namespace HgText
{
	uint16_t Text::m_canvasWidth = 1920;
	uint16_t Text::m_canvasHeight = 1080;

	std::unordered_map<std::string, FontGlyphMap> Text::m_glyphMaps;


	static vbo_layout_vnu textQuad[] = {
	{{-1.0f, -1.0f, 0.0f}, {0.0, 0.0, 0.0}, {0, 0}}, //bottom left
	{{1.0f, -1.0f, 0.0f}, {0.0, 0.0, 0.0}, {0, 0}}, //bottom right
	{{1.0f, 1.0f, 0.0f}, {0.0, 0.0, 0.0}, {0, 0}},  //top right
	{{-1.0f, 1.0f, 0.0f}, {0.0, 0.0, 0.0}, {0, 0}}  //top left
	};


	uint16_t Text::getCanvasHeight()
	{
		return m_canvasHeight;
	}

	uint16_t Text::getCanvasWidth()
	{
		return m_canvasWidth;

	}

	void Text::setCanvasHeight(uint16_t pixels)
	{
		m_canvasHeight = pixels;
	}

	void Text::setCanvasWidth(uint16_t pixels)
	{
		m_canvasWidth = pixels;
	}

	Text::Text()
	{
		FreetypeTextureLoader loader("assets/fonts/RobotoMono-Regular.ttf", 64);
		auto texture = HgTexture::acquire(loader, HgTexture::TextureType::DIFFUSE);

		if (texture)
		{
			m_fontImage = texture;
		}

setFontIdentifier(loader.uniqueIdentifier());
	}

	void Text::setText(std::string str)
	{
		m_text = std::move(str);
		BuildVbo();
	}

	void Text::setFontIdentifier(std::string fontId)
	{
		m_fontId = std::move(fontId);
	}

	void Text::BuildVbo()
	{
		const auto canvasHeight = Text::getCanvasHeight();
		const auto canvasWidth = Text::getCanvasWidth();

		const auto glyphMap = getGlyphMap(m_fontId);
		uint32_t xPos = 0;
		uint32_t yPos = 0;

		std::vector< vbo_layout_vnu > vertexData;
		std::vector< uint32_t > indices;

		//create a quad for each character with proper spacing from one character to the next

		for (uint32_t i = 0; i < m_text.size(); ++i)
		{
			const char c = m_text[i];

			auto itr = glyphMap.find(c);
			if (itr != glyphMap.end())
			{
				const auto& glyphData = itr->second;
				auto quad = textQuad;

				double xmin = xPos + glyphData.leftOffset;
				double xmax = xmin + glyphData.width;

				double ymax = yPos + glyphData.topOffset;
				double ymin = ymax - glyphData.height;

				//convert pixel values to canvas relative
				xmin /= canvasWidth;
				xmax /= canvasWidth;

				ymax /= canvasHeight;
				ymin /= canvasHeight;

				//bottom left
				quad[0].v.object.x((float)xmin);
				quad[0].v.object.y((float)ymin);
				quad[0].uv.x = glyphData.topLeft.x;
				quad[0].uv.y = glyphData.bottomRight.y;
				//quad[0].uv.x = 0 / 512.0;
				//quad[0].uv.y = 43 / 512.0;

				//bottom right
				quad[1].v.object.x((float)xmax);
				quad[1].v.object.y((float)ymin);
				quad[1].uv = glyphData.bottomRight;
				//quad[1].uv.x = 14/512.0;
				//quad[1].uv.y = 43/512.0;

				//top right
				quad[2].v.object.x((float)xmax);
				quad[2].v.object.y((float)ymax);
				quad[2].uv.x = glyphData.bottomRight.x;
				quad[2].uv.y = glyphData.topLeft.y;
				//quad[2].uv.x = 14/512.0;
				//quad[2].uv.y = 0/512.0;

				//top left
				quad[3].v.object.x((float)xmin);
				quad[3].v.object.y((float)ymax);
				quad[3].uv = glyphData.topLeft;
				//quad[3].uv.x = 0.0;
				//quad[3].uv.y = 0.0;

				uint32_t idx = vertexData.size();

				//ignore whitespace characters
				if (glyphData.width > 0)
				{
					vertexData.push_back(quad[0]);
					vertexData.push_back(quad[1]);
					vertexData.push_back(quad[2]);
					vertexData.push_back(quad[3]);

					indices.push_back(idx + 0);
					indices.push_back(idx + 1);
					indices.push_back(idx + 2);

					indices.push_back(idx + 2);
					indices.push_back(idx + 3);
					indices.push_back(idx + 0);
				}

				xPos += glyphData.advanceX; //advance to next character origin
			}

		}

		if (vertexData.size() > 0 && indices.size() > 0)
		{
			RenderDataPtr rd = getEntity().getRenderDataPtr();
			if (rd == nullptr)
			{
				rd = RenderData::Create();
				rd->getMaterial().setShader(HgShader::acquire("assets/shaders/FontShader.vert", "assets/shaders/FontShader.frag"));
				rd->getMaterial().setTransparent(true);
				rd->getMaterial().setBlendMode(BlendMode::BLEND_ALPHA);
				getEntity().setRenderData(rd);
				getEntity().setDrawOrder(127);
			}

			auto rec = HgVbo::GenerateFrom(vertexData.data(), vertexData.size());
			rd->VertexVboRecord(rec);

			auto iRec = HgVbo::GenerateFrom(indices.data(), indices.size());
			rd->indexVboRecord(iRec);

			rd->getMaterial().addTexture(m_fontImage);
		}
	}

	void Text::getInstanceData(gpuStruct* instanceData)
	{
		const auto mat = getEntity().computeWorldSpaceMatrix();
		mat.store(instanceData->matrix);
	}

	void Text::init()
	{
		IUpdatableInstance<gpuStruct>::init();

		//RenderDataPtr rd = std::make_shared<RenderData>(*crd);

		//HgEntity* e = &getEntity();
		//e->setRenderData(rd);
	}

	std::string FreetypeTextureLoader::uniqueIdentifier() const
	{
		constexpr size_t bufSize = 1024;
		char tmp[bufSize];
		tmp[0] = 0;

		int r = snprintf(tmp, bufSize, "%s::%d", m_fontPath.c_str(), m_pt);
		if (r > 0 && r < bufSize)
		{
			return std::string(tmp, r);
		}

		return std::string();
	}

	bool FreetypeTextureLoader::load(AssetPtr& asset) const
	{
		HgFreetypeLoader fontLoader;
		bool success = fontLoader.LoadFont(m_fontPath, m_pt);

		fontLoader.BuildDigest();

		auto size = fontLoader.getFontImage().getSize();
		auto data = std::unique_ptr<unsigned char, HgTexture::free_deleter>((unsigned char*)malloc(size));
		memcpy(data.get(), fontLoader.getFontImage().getPixels(), size);

		auto ltd = std::make_unique<HgTexture::LoadedTextureData>();
		ltd->m_data = std::move(data);
		ltd->properties.format = HgTexture::format::GRAY;
		ltd->properties.mipMapCount = 0;
		ltd->properties.height = fontLoader.getFontImage().getHeight();
		ltd->properties.width = fontLoader.getFontImage().getWidth();

		asset->setLoadedTextureData(ltd);

		const std::string uid = uniqueIdentifier();
		Text::setGlyphMap(uid, fontLoader.getGlyphMap());

		return success;
	}

	bool FreetypeTextureLoader::sourceChanged() const
	{
		return false;
	}


	void Text::setGlyphMap(const std::string& str, const FontGlyphMap& map)
	{
		m_glyphMaps[str] = map;
	}

	const FontGlyphMap& Text::getGlyphMap(const std::string& str)
	{
		static FontGlyphMap emptyMap;

		auto itr = m_glyphMaps.find(str);
		if (itr == m_glyphMaps.end()) return emptyMap;

		return itr->second;
	}

	HgFreetypeLoader::HgFreetypeLoader()
	{
		memset(&m_freeType, 0, sizeof(FT_Library));
		memset(&m_face, 0, sizeof(FT_Face));

		auto error = FT_Init_FreeType(&m_freeType);
		if (error)
		{
			fprintf(stderr, "Error init freetype library %d\n", error);
		}
	}

	HgFreetypeLoader::~HgFreetypeLoader()
	{
		FT_Done_Face(m_face);
		FT_Done_FreeType(m_freeType);
	}

	bool HgFreetypeLoader::LoadFont(const std::string& path, uint16_t pt)
	{
		auto error = FT_New_Face(m_freeType, path.c_str(), 0, &m_face);

		if (error != 0)
		{
			fprintf(stderr, "Error loading font \"%s\", %d\n", path.c_str(), error);
			return false;
		}

		m_pt = pt;

		error = FT_Set_Char_Size(m_face, 0,
			pt * 64,   // char_height in 1/64th of points
			72,     // horizontal device resolution
			72);   // vertical device resolution

		return true;
	}

	void HgFreetypeLoader::BuildDigest()
	{
		m_fontImage = RawImage<Grayscale>(512, 512);

		for (char c = ' '; c <= '~'; c++)
		{
			auto glyphIxd = FT_Get_Char_Index(m_face, c);
			auto error = FT_Load_Glyph(m_face, glyphIxd, FT_LOAD_DEFAULT);
			error = FT_Render_Glyph(m_face->glyph, FT_RENDER_MODE_NORMAL);

			//this can change the cursor position. do it before reading position
			m_fontImage.Concatenate((Grayscale*)m_face->glyph->bitmap.buffer, m_face->glyph->bitmap.width, m_face->glyph->bitmap.rows);

			UVCoordinates topRight;

			//Concatenate add 2 extra pixels between images, don't forget to subtract 2
			topRight.x = m_fontImage.getCursorHorizontalPosition() - 2;
			topRight.y = m_fontImage.getCursorVerticalPosition();

			HgText::GlyphData gd;

			gd.topLeft = topRight;
			gd.topLeft.x -= m_face->glyph->bitmap.width;

			gd.bottomRight = topRight;
			gd.bottomRight.y += m_face->glyph->bitmap.rows;

			gd.topLeft.x /= m_fontImage.getWidth();
			gd.topLeft.y /= m_fontImage.getHeight();

			gd.bottomRight.x /= m_fontImage.getWidth();
			gd.bottomRight.y /= m_fontImage.getHeight();

			gd.width = m_face->glyph->bitmap.width;
			gd.height = m_face->glyph->bitmap.rows;

			gd.leftOffset = m_face->glyph->bitmap_left;
			gd.topOffset = m_face->glyph->bitmap_top;
			gd.advanceX = m_face->glyph->advance.x / 64.0; // advance.x is 1/64th of a pixel
			//gd.advanceY = m_face->glyph->advance.y;

			m_glyphMap[c] = gd;
		}

		//int r = stbi_write_bmp("font.bmp", m_fontImage.getWidth(), m_fontImage.getHeight(), 1, m_fontImage.getPixels());
	}

}//namespace Text

REGISTER_LINKTIME2(Text, HgText::Text);