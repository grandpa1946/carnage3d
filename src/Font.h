#pragma once

#include "GuiContext.h"

// Drawable font instance
class Font final: public cxx::noncopyable
{
public:
    ~Font();

    // Load font characters and palette from input stream
    bool LoadFromStream(std::istream& inStream);
    void Clear();

    // Set base character code
    void SetFontBaseCharCode(int charCode);

    // Simple draw text characters on screen
    // @param guiContext: Context
    // @param text: Source string
    // @param position: Screen position in pixels
    void DrawString(GuiContext& guiContext, const std::string& text, const Point& position, int paletteIndex);

    // Get font line height in pixels
    int GetLineHeight() const;

    // Calculcate string dimensions, will process newlines and tabulations
    // @param text: Source string
    // @param outputSize: Text dimensions in pixels
    void MeasureString(const std::string& text, Point& outputSize) const;

    // Dump font characters to specified folder, for debug purposes only
    void DumpCharacters(const std::string& outputPath);

private:
    bool CreateFontAtlas();

private:

    struct RawCharacter
    {
        unsigned char mCharWidth = 0;
        unsigned char* mCharData = nullptr;
    };

    struct RawFontData
    {
        Palette256 mPalette;
        std::vector<RawCharacter> mRawCharacters;
    };

    std::vector<TextureRegion> mCharacters;
    RawFontData mFontData;
    GpuTexture2D* mFontTexture = nullptr;

    int mBaseCharCode = 0;
    int mLineHeight = 0;
};