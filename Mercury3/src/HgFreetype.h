#pragma once

#include <string>
#include <ft2build.h>
#include FT_FREETYPE_H

#include <HgTexture.h>
#include <algorithm>
#include <unordered_map>

#include <HgTypes.h>
#include <InstancedCollection.h>

namespace HgText
{

	struct GlyphData
	{
		GlyphData()
			:width(0), height(0), leftOffset(0), topOffset(0), advanceX(0)
			//, advanceY(0)
		{}

		UVCoordinates topLeft;
		UVCoordinates bottomRight;

		uint32_t width, height;
		uint16_t leftOffset; //offset from character origin
		uint16_t topOffset; //offset above character baseline
		double advanceX; //distance to the next character in a word
		//uint16_t advanceY; //vertical advance
	};

	using FontGlyphMap = std::unordered_map<char, HgText::GlyphData>;

	/* The text system is imagined to be on a canvas. The default size is 1920x1080.
	When font quads are rendered, the vertex positions are computed as if they will be
	rendered on an 1920x1080 surface, with coordinates ranging from 0.0 to 1.0.
	*/

	struct gpuStruct {
		float matrix[16];
	};

	class Text// : public IUpdatableInstance<gpuStruct>
	{
	public:
		using InstanceCollection = InstancedCollection<Text, gpuStruct, 1>;
		using InstanceCollectionPtr = std::shared_ptr<InstanceCollection>;

		//width of text system canvas
		static uint16_t getCanvasHeight();

		//height of text system canvas
		static uint16_t getCanvasWidth();

		static void setCanvasHeight(uint16_t pixels);
		static void setCanvasWidth(uint16_t pixels);

		static void setGlyphMap(const std::string& str, const FontGlyphMap& map);
		static const FontGlyphMap& getGlyphMap(const std::string& str);

		Text();

		void setText(std::string str);
		RenderDataPtr CreateRenderData();

		void setFontIdentifier(std::string);

	private:
		static uint16_t m_canvasWidth, m_canvasHeight;
		static std::unordered_map<std::string, FontGlyphMap> m_glyphMaps;

		std::string m_text;
		std::string m_fontId;

		std::shared_ptr<HgTexture> m_fontImage;
	};

	class FreetypeTextureLoader : public HgTexture::AssetManagerType::IAssetLoader
	{
	public:
		FreetypeTextureLoader(std::string fontPath, uint16_t pt)
			:m_fontPath(fontPath), m_pt(pt)
		{}

		virtual std::string uniqueIdentifier() const;
		virtual bool load(AssetPtr& asset) const;

		//return true if the source data changed and should be reloaded
		virtual bool sourceChanged() const;
	private:
		std::string m_fontPath;
		uint16_t m_pt;
	};

}

struct Grayscale
{
	uint8_t color;
};

template<typename ColorType>
class RawImage
{
public:
	RawImage(uint32_t width=0, uint32_t height=0)
	{
		m_pixels = new ColorType[width * height];
		memset(m_pixels, 0, sizeof(ColorType) * width * height);
		m_width = width;
		m_height = height;
		//m_currentWidth = m_currentHeight = 0;

		m_cursorTopRow = 0;
		m_cursorHeight = 0;
		m_cursorHorizontalPos = 0;

	}

	bool Concatenate(ColorType* pixels, uint32_t width, uint32_t height)
	{
		if (!CheckHorizontalSpace(width))
		{
			nextRow();
		}

		if (!ExpandRowHeight(height))
		{
			return false;
		}

		//copy pixels
		ColorType* dest = m_pixels + (m_width * m_cursorTopRow) + m_cursorHorizontalPos;
		ColorType* src = pixels;
		for (uint32_t i = 0; i < height; i++)
		{
			memcpy(dest, src, sizeof(ColorType) * width);

			//advance by the width of the images
			dest = dest + m_width;
			src = src + width;
		}

		m_cursorHorizontalPos += width + 2;

		return true;
	}


	ColorType* getPixels() const { return m_pixels; }
	uint32_t getHeight() const { return m_height; }
	uint32_t getWidth() const { return m_width; }

	uint32_t getCursorVerticalPosition() const { return m_cursorTopRow; }
	uint32_t getCursorHorizontalPosition() const { return m_cursorHorizontalPos; }

	//return the size of the image data in bytes
	size_t getSize() const { return getHeight() * getWidth() * sizeof(ColorType); }


private:
	ColorType* m_pixels;

	bool CheckHorizontalSpace(uint32_t width)
	{
		return (m_cursorHorizontalPos + width) <= m_width;
	}

	bool ExpandRowHeight(uint32_t height)
	{
		auto cursorHeight = (std::max(height, m_cursorHeight));
		if ((cursorHeight + m_cursorTopRow) > m_height)
		{
			return false;
		}

		m_cursorHeight = cursorHeight;
		return true;
	}

	//advance cursor to next available row of pixels
	void nextRow()
	{
		m_cursorTopRow += m_cursorHeight + 1 + 2;
		m_cursorHeight = 0;
		m_cursorHorizontalPos = 0;
	}

	uint32_t m_width, m_height;

	uint32_t m_cursorTopRow; //pixel row the cursor is at
	uint32_t m_cursorHeight; //height of the cursor. pixels high.
	uint32_t m_cursorHorizontalPos;

//	uint32_t m_currentWidth, m_currentHeight;
//	uint32_t m_rowHeight;

};

namespace HgText
{

	class HgFreetypeLoader
	{
	public:
		HgFreetypeLoader();
		~HgFreetypeLoader();

		bool LoadFont(const std::string& path, uint16_t pt = 64);
		void BuildDigest();

		const RawImage<Grayscale>& getFontImage() const { return m_fontImage; }
		const std::unordered_map<char, HgText::GlyphData>& getGlyphMap() const { return m_glyphMap; }
	private:
		FT_Library m_freeType;
		FT_Face m_face;

		uint16_t m_pt;

		RawImage<Grayscale> m_fontImage;
		std::unordered_map<char, HgText::GlyphData> m_glyphMap;
	};

}