#include "graphics.h"
#include <string>

void drawRectangle(Rectangle &rectangle)
{
	C2D_DrawRectSolid(rectangle.x, rectangle.y, rectangle.z, rectangle.w, rectangle.h, rectangle.color);
}

bool hasCollision(Rectangle &rectangle, Rectangle &rectangle2)
{
	return rectangle.x < rectangle2.x + rectangle2.w && rectangle.x + rectangle.w > rectangle2.x &&
		   rectangle.y < rectangle2.y + rectangle2.h && rectangle.y + rectangle.h > rectangle2.y;
}

Sprite loadSprite(const char *filePath, float positionX, float positionY, float width, float height)
{
	std::string basePath = "romfs:/gfx/";
	std::string fullPath = basePath + filePath;

	C2D_SpriteSheet sheet = C2D_SpriteSheetLoad(fullPath.c_str());
	C2D_Image image = C2D_SpriteSheetGetImage(sheet, 0);

	Rectangle bounds = {positionX, positionY, 0, width, height, WHITE};

	Sprite sprite = {image, bounds, sheet};

	return sprite;
}

// ( 	C2D_Image img, float x, float y, float depth, const C2D_ImageTint *tint C2D_OPTIONALnullptr, float scaleX C2D_OPTIONAL1.0f, float scaleY C2D_OPTIONAL1.0f)
void drawSprite(Sprite &sprite)
{
	C2D_DrawImageAt(sprite.texture, sprite.bounds.x, sprite.bounds.y, sprite.bounds.z, NULL, 1, 1);
}

void drawDynamicText(const char *textFormat, int value, C2D_TextBuf &dynamicBuffer, int positionX, int positionY, float textSize)
{
	char buf[160];
	C2D_Text dynamicText;
	snprintf(buf, sizeof(buf), textFormat, value);

	C2D_TextParse(&dynamicText, dynamicBuffer, buf);
	C2D_TextOptimize(&dynamicText);
	C2D_DrawText(&dynamicText, C2D_AlignCenter | C2D_WithColor, positionX, positionY, 0, textSize, textSize, WHITE);

	C2D_TextBufClear(dynamicBuffer);
}

void drawAndRotateImage(Sprite &sprite, float rotation)
{
	// need to make this adjustment, because the x and y coordinates place the image in the center, instead of the top-left corner.
	float positionX = sprite.bounds.x + sprite.bounds.w / 2;
	float positionY = sprite.bounds.y + sprite.bounds.h / 2;

	C2D_DrawImageAtRotated(sprite.texture, positionX, positionY, sprite.bounds.z, rotation, NULL, 1, 1);
}