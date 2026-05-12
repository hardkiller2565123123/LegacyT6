#include "STDInc.h"

const char* InGameConsole::DebugLog = "";

static const char* ConsoleTitle = "T6M>";
static const char* ConsoleText = "press ` to toggle overlay";

static std::vector<const char*> DebugLogList;
static int VectorCountDebugLogList = 0;

static Font_s* ConsoleFont = NULL;
static Font_s* NormalFont = NULL;
static Material* Material_White = NULL;

static bool ConsoleOpen = true;
static bool ConsoleReady = false;
static bool HooksInstalled = false;

static SHORT LastBacktickState = 0;

bool InGameConsole::IsOpen()
{
	return ConsoleOpen;
}

void InGameConsole::Toggle()
{
	ConsoleOpen = !ConsoleOpen;

	printf(
		"[InGameConsole] overlay %s\n",
		ConsoleOpen ? "opened" : "closed");
}

static void PollConsoleToggleKey()
{
	SHORT state = GetAsyncKeyState(VK_OEM_3);

	if ((state & 0x8000) && !(LastBacktickState & 0x8000))
	{
		InGameConsole::Toggle();
	}

	LastBacktickState = state;
}

static bool SafeLoadConsoleAssets()
{
	Font_s* loadedConsoleFont = NULL;
	Font_s* loadedNormalFont = NULL;
	Material* loadedWhite = NULL;

	__try
	{
		loadedConsoleFont =
			Addresses::DB_FindXAssetHeader(
				XAssetType::ASSET_TYPE_FONT,
				"consoleFont",
				true,
				-1).font;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		printf("[InGameConsole] consoleFont crashed\n");
	}

	__try
	{
		loadedNormalFont =
			Addresses::DB_FindXAssetHeader(
				XAssetType::ASSET_TYPE_FONT,
				"normalFont",
				true,
				-1).font;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		printf("[InGameConsole] normalFont crashed\n");
	}

	__try
	{
		loadedWhite =
			Addresses::DB_FindXAssetHeader(
				XAssetType::ASSET_TYPE_MATERIAL,
				"white",
				true,
				-1).material;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		printf("[InGameConsole] white material crashed\n");
	}

	if (!loadedConsoleFont)
	{
		__try
		{
			loadedConsoleFont =
				Addresses::DB_FindXAssetHeader(
					XAssetType::ASSET_TYPE_FONT,
					"fonts/720/consolefont",
					true,
					-1).font;
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			printf("[InGameConsole] fonts/720/consolefont crashed\n");
		}
	}

	if (!loadedNormalFont)
	{
		__try
		{
			loadedNormalFont =
				Addresses::DB_FindXAssetHeader(
					XAssetType::ASSET_TYPE_FONT,
					"normalfont",
					true,
					-1).font;
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			printf("[InGameConsole] normalfont crashed\n");
		}
	}

	if (!loadedConsoleFont || !loadedNormalFont || !loadedWhite)
	{
		printf(
			"[InGameConsole] assets missing console=%p normal=%p white=%p\n",
			loadedConsoleFont,
			loadedNormalFont,
			loadedWhite);

		return false;
	}

	ConsoleFont = loadedConsoleFont;
	NormalFont = loadedNormalFont;
	Material_White = loadedWhite;

	printf("[InGameConsole] assets loaded\n");
	return true;
}

static void InitGameConsoleSafe()
{
	ConsoleReady = SafeLoadConsoleAssets();
}

static void DrawTextSafe(
	const char* text,
	Font_s* font,
	float x,
	float y,
	float sx,
	float sy,
	tColor color)
{
	if (!text || !font || !Addresses::R_AddCmdDrawText)
		return;

	__try
	{
		Addresses::R_AddCmdDrawText(
			text,
			0x7FFFFFFF,
			font,
			x,
			y,
			sx,
			sy,
			0,
			color,
			0);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
	}
}

static void DrawPicSafe(
	float x,
	float y,
	float w,
	float h,
	tColor color)
{
	if (!Material_White || !Addresses::DrawRotatedPic)
		return;

	__try
	{
		tColor localColor =
		{
			color[0],
			color[1],
			color[2],
			color[3]
		};

		Addresses::DrawRotatedPic(
			x,
			y,
			1.0f,
			w,
			h,
			0,
			0,
			0,
			0,
			&localColor,
			Material_White);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
	}
}
static void DrawConsoleSafe()
{
	__try
	{
		PollConsoleToggleKey();



		if (!Addresses::RendererWidth || !Addresses::RendererHeight)
			return;

		if (!ConsoleReady)
		{
			static int retryCounter = 0;

			if (++retryCounter > 120)
			{
				retryCounter = 0;
				InitGameConsoleSafe();
			}

			return;
		}

		float width = (float)*(DWORD*)Addresses::RendererWidth;

		tColor watermark = { 1.0f, 1.0f, 1.0f, 0.15f };
		DrawTextSafe("T6M", NormalFont, 20.0f, 40.0f, 1.0f, 1.0f, watermark);

		if (!ConsoleOpen)
			return;

		tColor bg = { 0.02f, 0.02f, 0.02f, 0.88f };
		tColor top = { 0.20f, 0.95f, 0.20f, 0.95f };
		tColor border = { 1.0f, 1.0f, 0.0f, 0.55f };
		tColor text = { 1.0f, 1.0f, 1.0f, 1.0f };

		DrawPicSafe(10.0f, 10.0f, width - 60.0f, 34.0f, bg);
		DrawPicSafe(10.0f, 10.0f, width - 60.0f, 2.0f, top);
		DrawPicSafe(10.0f, 42.0f, width - 60.0f, 1.0f, border);
		DrawPicSafe(10.0f, 10.0f, 1.0f, 34.0f, border);
		DrawPicSafe(width - 50.0f, 10.0f, 1.0f, 34.0f, border);

		float titleLen = 45.0f;

		__try
		{
			if (Addresses::GetTextLengthSize && ConsoleFont)
			{
				titleLen =
					Addresses::GetTextLengthSize(
						0,
						ConsoleTitle,
						10,
						ConsoleFont);
			}
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			titleLen = 45.0f;
		}

		DrawTextSafe(ConsoleTitle, ConsoleFont, 16.0f, 33.0f, 1.0f, 1.0f, text);
		DrawTextSafe(ConsoleText, ConsoleFont, titleLen + 26.0f, 33.0f, 1.0f, 1.0f, text);

		for (size_t i = 0; i < DebugLogList.size(); i++)
		{
			DrawTextSafe(
				DebugLogList[i],
				ConsoleFont,
				20.0f,
				62.0f + ((float)i * 15.0f),
				1.0f,
				1.0f,
				text);
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		ConsoleReady = false;
		printf("[InGameConsole] draw crashed, disabling overlay\n");
	}
}

void __declspec(naked) RendererFrame_Stub()
{
	DrawConsoleSafe();

	__asm jmp Addresses::RendererFrameO
}

void __declspec(naked) RendererStart_Stub()
{
	InitGameConsoleSafe();

	__asm jmp Addresses::RendererStartO
}

void InGameConsole::Log(const char* text)
{
	if (!text)
		return;

	if (DebugLogList.size() + 1 >= 25)
		DebugLogList.erase(DebugLogList.begin());

	VectorCountDebugLogList++;

	DebugLogList.push_back(
		hString::va(
			"%i : %s",
			VectorCountDebugLogList,
			text));

	DebugLog = text;
}

void InGameConsole::Initialize()
{
	if (HooksInstalled)
	{
		printf("[InGameConsole] already initialized\n");
		return;
	}

	printf("[InGameConsole] initialize safe backtick overlay\n");

	if (!Addresses::RendererFrame)
	{
		printf("[InGameConsole] missing RendererFrame address\n");
		return;
	}

	__try
	{
		QCALL(Addresses::RendererFrame, RendererFrame_Stub, QPATCH_CALL);
		printf("[InGameConsole] RendererFrame hook installed\n");
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		printf("[InGameConsole] RendererFrame hook crashed\n");
		return;
	}
	/*
	__try
	{
		QCALL(Addresses::RendererStart, RendererStart_Stub, QPATCH_JUMP);
		printf("[InGameConsole] RendererStart hook installed\n");
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		printf("[InGameConsole] RendererStart hook crashed\n");
		return;
	}
	*/

	HooksInstalled = true;

	// Load now too, in case RendererStart already ran.
	InitGameConsoleSafe();

	InGameConsole::Log("In-game overlay initialized");
	InGameConsole::Log("Press ` to show/hide");
}