#include "graphics.h"
#include <vector>
#include <string>
#include <map>

using std::map;
using std::vector;

// the 3ds has different screen width, but the same screen height.
const int TOP_SCREEN_WIDTH = 400;
const int BOTTOM_SCREEN_WIDTH = 320;
const int SCREEN_HEIGHT = 240;

C3D_RenderTarget *topScreen = nullptr;
C3D_RenderTarget *bottomScreen = nullptr;

C2D_TextBuf textDynamicBuffer;

C2D_TextBuf textStaticBuffer;
C2D_Text staticTexts[1];

float textSize = 1.0f;

const int TOTAL_ROWS = 12;
const int TOTAL_COLUMNS = 10;
const int CELL_SIZE = 20;

int grid[TOTAL_ROWS][TOTAL_COLUMNS];

const int POSITION_OFFSET = 2;
const int CELL_OFFSET = 2;

bool isGamePaused;
bool isGameOver;

int score;

typedef struct Vector2
{
	float x;
	float y;
} Vector2;

typedef struct
{
	int id;
	map<int, vector<Vector2>> cells;
	int rotationState;
	int columnOffset;
	int rowOffset;
} Block;

Block lBlock;
Block jBlock;
Block iBlock;
Block oBlock;
Block sBlock;
Block tBlock;
Block zBlock;
Block currentBlock;
Block nextBlock;

vector<Block> blocks;

vector<Vector2> getCellPositions(Block &block)
{
	// getting the reference of the vector instead of copying to create a new one.
	vector<Vector2> &blockTiles = block.cells[block.rotationState];

	vector<Vector2> movedTiles;
	movedTiles.reserve(blockTiles.size());

	for (Vector2 blockTile : blockTiles)
	{
		Vector2 newPosition = {blockTile.x + block.rowOffset, blockTile.y + block.columnOffset};
		movedTiles.push_back(newPosition);
	}

	return movedTiles;
}

bool isCellOutside(int cellRow, int cellColumn)
{
	if (cellRow >= 0 && cellRow < TOTAL_ROWS && cellColumn >= 0 && cellColumn < TOTAL_COLUMNS)
	{
		return false;
	}

	return true;
}

bool isBlockOutside(Block &block)
{
	vector<Vector2> blockTiles = getCellPositions(block);

	for (Vector2 blockTile : blockTiles)
	{
		if (isCellOutside(blockTile.x, blockTile.y))
		{
			return true;
		}
	}

	return false;
}

void undoRotation(Block &block)
{
	block.rotationState--;

	if (block.rotationState == -1)
	{
		block.rotationState = block.cells.size() - 1;
	}
}

bool isCellEmpty(int rowToCheck, int columnToCheck)
{
	if (grid[rowToCheck][columnToCheck] == 0)
	{
		return true;
	}

	return false;
}

bool blockFits(Block &block)
{
	auto blockCells = getCellPositions(block);

	// I need to write in the grid the id of the block that I'm going to lock
	for (Vector2 blockCell : blockCells)
	{
		if (!isCellEmpty(blockCell.x, blockCell.y))
		{
			return false;
		}
	}

	return true;
}

void rotateBlock(Block &block)
{
	block.rotationState++;

	if (block.rotationState == (int)block.cells.size())
	{
		block.rotationState = 0;
	}

	if (isBlockOutside(block) || !blockFits(currentBlock))
	{
		undoRotation(block);
	}
}

void moveBlock(Block &block, int rowsToMove, int columnsToMove)
{
	block.rowOffset += rowsToMove;
	block.columnOffset += columnsToMove;
}

int rand_range(int min, int max)
{
	return min + rand() / (RAND_MAX / (max - min + 1) + 1);
}

Block getRandomBlock()
{
	if (blocks.empty())
	{
		blocks = {lBlock, jBlock, iBlock, oBlock, sBlock, tBlock, zBlock};
	}

	int randomIndex = rand_range(0, blocks.size() - 1);

	Block actualBlock = blocks[randomIndex];
	blocks.erase(blocks.begin() + randomIndex);

	return actualBlock;
}

bool isRowFull(int rowToCheck)
{
	for (int column = 0; column < TOTAL_COLUMNS; column++)
	{
		if (grid[rowToCheck][column] == 0)
		{
			return false;
		}
	}

	return true;
}

void clearRow(int rowToClear)
{
	for (int column = 0; column < TOTAL_COLUMNS; column++)
	{
		grid[rowToClear][column] = 0;
	}
}

void moveRowDown(int row, int totalRows)
{
	for (int column = 0; column < TOTAL_COLUMNS; column++)
	{
		grid[row + totalRows][column] = grid[row][column];
		grid[row][column] = 0;
	}
}

int clearFullRow()
{
	int completedRow = 0;
	for (int row = TOTAL_ROWS - 1; row >= 0; row--)
	{
		if (isRowFull(row))
		{
			clearRow(row);
			completedRow++;
		}
		else if (completedRow > 0)
		{
			moveRowDown(row, completedRow);
		}
	}

	return completedRow;
}

void lockBlock(Block &block)
{
	auto blockCells = getCellPositions(block);

	// I need to write in the grid the id of the block that I'm going to lock
	for (Vector2 blockCell : blockCells)
	{
		grid[(int)blockCell.x][(int)blockCell.y] = block.id;
	}

	// and then update the current and next blocks.
	block = nextBlock;

	if (!blockFits(block))
	{
		isGameOver = true;
	}

	nextBlock = getRandomBlock();

	int totalClearRows = clearFullRow();

	if (totalClearRows == 1)
	{
		score += 100;
	}

	else if (totalClearRows == 2)
	{
		score += 300;
	}

	else if (totalClearRows > 2)
	{
		score += 500;
	}
}

void initializeGrid()
{
	for (int row = 0; row < TOTAL_ROWS; row++)
	{
		for (int column = 0; column < TOTAL_COLUMNS; column++)
		{
			grid[row][column] = 0;
		}
	}
}

double lastUpdateTime = 0;

bool eventTriggered(float deltaTime, float intervalUpdate)
{
	lastUpdateTime += deltaTime;

	if (lastUpdateTime >= intervalUpdate)
	{
		lastUpdateTime = 0;

		return true;
	}

	return false;
}

void update(int keyDown)
{
	if (keyDown & KEY_UP)
	{
		rotateBlock(currentBlock);
	}

	if (keyDown & KEY_RIGHT)
	{
		moveBlock(currentBlock, 0, 1);

		if (isBlockOutside(currentBlock) || !blockFits(currentBlock))
		{
			moveBlock(currentBlock, 0, -1);
		}
	}

	else if (keyDown & KEY_LEFT)
	{
		moveBlock(currentBlock, 0, -1);

		if (isBlockOutside(currentBlock) || !blockFits(currentBlock))
		{
			moveBlock(currentBlock, 0, 1);
		}
	}

	if (!isGameOver && keyDown & KEY_DOWN)
	{
		score++;
		moveBlock(currentBlock, 1, 0);

		if (isBlockOutside(currentBlock) || !blockFits(currentBlock))
		{
			moveBlock(currentBlock, -1, 0);
			lockBlock(currentBlock);
		}
	}

	// if (!isGameOver && eventTriggered(deltaTime, 0.5))
	// {
	//     moveBlock(currentBlock, 1, 0);

	//     if (isBlockOutside(currentBlock) || !blockFits(currentBlock))
	//     {
	//         moveBlock(currentBlock, -1, 0);
	//         lockBlock(currentBlock);
	//     }
	// }
}

void initializeBlocks()
{
	// defining Blocks 4 rotations with a map id and vector2 2
	lBlock.id = 1;
	lBlock.cells[0] = {{0, 2}, {1, 0}, {1, 1}, {1, 2}};
	lBlock.cells[1] = {{0, 1}, {1, 1}, {2, 1}, {2, 2}};
	lBlock.cells[2] = {{1, 0}, {1, 1}, {1, 2}, {2, 0}};
	lBlock.cells[3] = {{0, 0}, {0, 1}, {1, 1}, {2, 1}};
	// for all the block to start in the midle of the grid, I need to move to the (0, 3)
	moveBlock(lBlock, 0, 3);

	jBlock.id = 2;
	jBlock.cells[0] = {{0, 0}, {1, 0}, {1, 1}, {1, 2}};
	jBlock.cells[1] = {{0, 1}, {0, 2}, {1, 1}, {2, 1}};
	jBlock.cells[2] = {{1, 0}, {1, 1}, {1, 2}, {2, 2}};
	jBlock.cells[3] = {{0, 1}, {1, 1}, {2, 0}, {2, 1}};

	moveBlock(jBlock, 0, 3);

	iBlock.id = 3;
	iBlock.cells[0] = {{1, 0}, {1, 1}, {1, 2}, {1, 3}};
	iBlock.cells[1] = {{0, 2}, {1, 2}, {2, 2}, {3, 2}};
	iBlock.cells[2] = {{2, 0}, {2, 1}, {2, 2}, {2, 3}};
	iBlock.cells[3] = {{0, 1}, {1, 1}, {2, 1}, {3, 1}};

	moveBlock(iBlock, -1, 3);

	// I don't need rotaion with this block
	oBlock.id = 4;
	oBlock.cells[0] = {{0, 0}, {0, 1}, {1, 0}, {1, 1}};

	moveBlock(oBlock, 0, 4);

	sBlock.id = 5;
	sBlock.cells[0] = {{0, 1}, {0, 2}, {1, 0}, {1, 1}};
	sBlock.cells[1] = {{0, 1}, {1, 1}, {1, 2}, {2, 2}};
	sBlock.cells[2] = {{1, 1}, {1, 2}, {2, 0}, {2, 1}};
	sBlock.cells[3] = {{0, 0}, {1, 0}, {1, 1}, {2, 1}};

	moveBlock(sBlock, 0, 3);

	tBlock.id = 6;
	tBlock.cells[0] = {{0, 1}, {1, 0}, {1, 1}, {1, 2}};
	tBlock.cells[1] = {{0, 1}, {1, 1}, {1, 2}, {2, 1}};
	tBlock.cells[2] = {{1, 0}, {1, 1}, {1, 2}, {2, 1}};
	tBlock.cells[3] = {{0, 1}, {1, 0}, {1, 1}, {2, 1}};

	moveBlock(tBlock, 0, 3);

	zBlock.id = 7;
	zBlock.cells[0] = {{0, 0}, {0, 1}, {1, 1}, {1, 2}};
	zBlock.cells[1] = {{0, 2}, {1, 1}, {1, 2}, {2, 1}};
	zBlock.cells[2] = {{1, 0}, {1, 1}, {2, 1}, {2, 2}};
	zBlock.cells[3] = {{0, 1}, {1, 0}, {1, 1}, {2, 0}};

	moveBlock(zBlock, 0, 3);

	blocks.reserve(7);
	blocks = {lBlock, jBlock, iBlock, oBlock, sBlock, tBlock, zBlock};

	currentBlock = getRandomBlock();
	nextBlock = getRandomBlock();
}

u32 getColorByIndex(int index)
{
	const u32 lightGrey = C2D_Color32(80, 80, 80, 255);
	const u32 green = C2D_Color32(47, 230, 23, 255);
	const u32 red = C2D_Color32(232, 18, 18, 255);
	const u32 orange = C2D_Color32(226, 116, 17, 255);
	const u32 yellow = C2D_Color32(237, 234, 4, 255);
	const u32 purple = C2D_Color32(166, 0, 247, 255);
	const u32 cyan = C2D_Color32(21, 204, 209, 255);
	const u32 blue = C2D_Color32(13, 64, 216, 255);

	u32 colors[] = {lightGrey, green, red, orange, yellow, purple, cyan, blue};

	return colors[index];
}

void drawGrid()
{
	for (int row = 0; row < TOTAL_ROWS; row++)
	{
		for (int column = 0; column < TOTAL_COLUMNS; column++)
		{
			int cellValue = grid[row][column];

			u32 cellColor = getColorByIndex(cellValue);

			Rectangle rect = {(float)column * CELL_SIZE + POSITION_OFFSET, (float)row * CELL_SIZE + POSITION_OFFSET, 0, CELL_SIZE - CELL_OFFSET, CELL_SIZE - CELL_OFFSET, cellColor};
			drawRectangle(rect);
		}
	}
}

void drawBlock(Block &block, int offsetX, int offsetY)
{
	vector<Vector2> blockTiles = getCellPositions(block);

	for (Vector2 blockTile : blockTiles)
	{
		u32 cellColor = getColorByIndex(block.id);

		Rectangle rect = {blockTile.y * CELL_SIZE + offsetX, blockTile.x * CELL_SIZE + offsetY, 0, CELL_SIZE - CELL_OFFSET, CELL_SIZE - CELL_OFFSET, cellColor};
		drawRectangle(rect);
	}
}

void drawBlock(Block &block)
{
	vector<Vector2> blockTiles = getCellPositions(block);

	for (Vector2 blockTile : blockTiles)
	{
		u32 cellColor = getColorByIndex(block.id);

		Rectangle rect = {(float)blockTile.y * CELL_SIZE + POSITION_OFFSET, blockTile.x * CELL_SIZE + POSITION_OFFSET, 0, CELL_SIZE - CELL_OFFSET, CELL_SIZE - CELL_OFFSET, cellColor};
		drawRectangle(rect);
	}
}

void renderTopScreen()
{
	C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
	C2D_TargetClear(topScreen, BLACK);
	C2D_SceneBegin(topScreen);

	drawGrid();

	drawBlock(currentBlock);

	if (isGamePaused)
	{
		C2D_DrawText(&staticTexts[0], C2D_AtBaseline | C2D_WithColor, 110, 60, 0, textSize, textSize, WHITE);
	}

	C3D_FrameEnd(0);
}

void renderBottomScreen()
{
	C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
	C2D_TargetClear(bottomScreen, BLACK);
	C2D_SceneBegin(bottomScreen);

	// drawRectangle(bottomBounds);

	// drawDynamicText("Total collisions: %d", 0, textDynamicBuffer, 150, 175, textSize);

	C3D_FrameEnd(0);
}

int main(int argc, char *argv[])
{
	romfsInit();
	gfxInitDefault();
	C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
	C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
	C2D_Prepare();

	topScreen = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
	bottomScreen = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);

	textStaticBuffer = C2D_TextBufNew(1024);
	textDynamicBuffer = C2D_TextBufNew(4096);

	C2D_TextParse(&staticTexts[0], textStaticBuffer, "Game Paused");

	C2D_TextOptimize(&staticTexts[0]);

	initializeGrid();
	initializeBlocks();

	touchPosition touch;

	// Main loop
	while (aptMainLoop())
	{
		hidScanInput();

		hidTouchRead(&touch);

		int keyDown = hidKeysDown();

		if (keyDown & KEY_START)
		{
			isGamePaused = !isGamePaused;
		}

		if (!isGamePaused)
		{
			update(keyDown);
		}

		renderTopScreen();

		renderBottomScreen();
	}

	// Delete the text buffers
	C2D_TextBufDelete(textDynamicBuffer);
	C2D_TextBufDelete(textStaticBuffer);

	// Deinit libs
	C2D_Fini();
	C3D_Fini();
	gfxExit();
	romfsExit();
}
