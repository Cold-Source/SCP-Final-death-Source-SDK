///////////////////////////////////////////////////////////
// Tracker scheme resource file
//
// sections:
//		Colors			- all the colors used by the scheme
//		BaseSettings	- contains settings for app to use to draw controls
//		Fonts			- list of all the fonts used by app
//		Borders			- description of all the borders
//
///////////////////////////////////////////////////////////
Scheme
{
	//////////////////////// COLORS ///////////////////////////
	// color details
	// this is a list of all the colors used by the scheme
	Colors
	{
		"SteamDeckLoadingBar" "0 0 0 255"
		"SteamDeckSpinner" "0 0 0 255"
	}

	///////////////////// BASE SETTINGS ////////////////////////
	//
	// default settings for all panels
	// controls use these to determine their settings
	BaseSettings
	{
		"FgColor"			"245 245 245 100"
		"BgColor"			"0 0 0 76"

		"Panel.FgColor"			"245 245 245 100"
		"Panel.BgColor"			"0 0 0 76"

		//Highlighted, fade from green into blue
		"BrightFg"		"245 245 245 255"

		"DamagedBg"			"28 28 28 200"
		"DamagedFg"			"245 245 245 230"
		"BrightDamagedFg"		"245 245 245 255"

		// weapon selection colors
		"SelectionNumberFg"		"245 245 245 255"
		"SelectionTextFg"		"245 245 245 255"
		"SelectionEmptyBoxBg" 	"0 0 0 80"
		"SelectionBoxBg" 		"0 0 0 80"
		"SelectionSelectedBoxBg" "0 0 0 80"

		"ZoomReticleColor"	"245 245 245 255"

		// HL1-style HUD colors
		"Yellowish"			"245 245 245 255"
		"Normal"			"245 245 245 255"
		"Caution"			"245 245 245 255"

		// Top-left corner of the "Half-Life 2" on the main screen
		"Main.Title1.X"				"50" // 76
		"Main.Title1.Y"				"145"
		"Main.Title1.Y_hidef"		"130"
		"Main.Title1.Color"	"255 255 255 0"

		// Top-left corner of secondary title e.g. "DEMO" on the main screen
		"Main.Title2.X"				"76"
		"Main.Title2.Y"				"190"
		"Main.Title2.Y_hidef"		"174"
		"Main.Title2.Color"	"255 255 255 0"

		// Top-left corner of the menu on the main screen
		"Main.Menu.X"			"53"
		"Main.Menu.X_hidef"		"76"
		"Main.Menu.Y"			"240"

		// Blank space to leave beneath the menu on the main screen
		"Main.BottomBorder"	"32"
	}

	//////////////////////// BITMAP FONT FILES /////////////////////////////
	//
	// Bitmap Fonts are ****VERY*** expensive static memory resources so they are purposely sparse
	BitmapFontFiles
	{
		// UI buttons, custom font, (256x64)
		"Buttons"		"materials/vgui/fonts/buttons_32.vbf"
	}


	//////////////////////// FONTS /////////////////////////////
	//
	// describes all the fonts
	Fonts
	{
		"SCPHUDLabel"
        {
            "1"
            {
                "name" "Trebuchet MS"
                "tall" "8"
                "weight" "700"
                "dropshadow" "1"
                "antialias" "1"
                "range" "0x0020 0x04FF"
            }
        }

		SCPHUDValue
		{
			"1"
			{
				"name" "Trebuchet MS"
				"tall" "16"
				"weight" "700"
				"dropshadow" "1"
				"antialias" "1"
			"range" "0x0020 0x04FF" }
		}

		// fonts are used in order that they are listed
		// fonts are used in order that they are listed
		"DebugFixed"
		{
			"1"
			{
				"name"		"Trebuchet MS"
				"tall"		"14"
				"weight"	"700"
				"antialias" "1"
			"range" "0x0020 0x04FF" }
		}
		// fonts are used in order that they are listed
		"DebugFixedSmall"
		{
			"1"
			{
				"name"		"Trebuchet MS"
				"tall"		"14"
				"weight"	"700"
				"antialias" "1"
			"range" "0x0020 0x04FF" }
		}
		// fonts listed later in the order will only be used if they fulfill a range not already filled
		// if a font fails to load then the subsequent fonts will replace
		Default
		{
			"1"	[$X360]
			{
				"name"		"Trebuchet MS"
				"tall"		"12"
				"weight"	"700"
				"antialias" "1"
			"range" "0x0020 0x04FF" }
			"1"	[$WIN32]
			{
				"name"		"Trebuchet MS"
				"tall"		"9"
				"weight"	"700"
				"yres"	"1 599"
				"antialias" "1"
			"range" "0x0020 0x04FF" }
			"2"
			{
				"name"		"Trebuchet MS"
				"tall"		"12"
				"weight"	"700"
				"yres"	"600 767"
				"antialias" "1"
			"range" "0x0020 0x04FF" }
			"3"
			{
				"name"		"Trebuchet MS"
				"tall"		"14"
				"weight"	"700"
				"yres"	"768 1023"
				"antialias" "1"
			"range" "0x0020 0x04FF" }
			"4"
			{
				"name"		"Trebuchet MS"
				"tall"		"20"
				"weight"	"700"
				"yres"	"1024 1199"
				"antialias" "1"
			"range" "0x0020 0x04FF" }
			"5"
			{
				"name"		"Trebuchet MS"
				"tall"		"24"
				"weight"	"700"
				"yres"	"1200 10000"
				"additive"	"1"
				"antialias" "1"
			"range" "0x0020 0x04FF" }
		}
		"DefaultSmall"
		{
			"1"
			{
				"name"		"Trebuchet MS"
				"tall"		"12"
				"weight"	"700"
				"yres"	"480 599"
				"antialias" "1"
			"range" "0x0020 0x04FF" }
			"2"
			{
				"name"		"Trebuchet MS"
				"tall"		"13"
				"weight"	"700"
				"yres"	"600 767"
				"antialias" "1"
			"range" "0x0020 0x04FF" }
			"3"
			{
				"name"		"Trebuchet MS"
				"tall"		"14"
				"weight"	"700"
				"yres"	"768 1023"
				"antialias" "1"
			"range" "0x0020 0x04FF" }
			"4"
			{
				"name"		"Trebuchet MS"
				"tall"		"20"
				"weight"	"700"
				"yres"	"1024 1199"
				"antialias" "1"
			"range" "0x0020 0x04FF" }
			"5"
			{
				"name"		"Trebuchet MS"
				"tall"		"24"
				"weight"	"700"
				"yres"	"1200 6000"
				"antialias" "1"
			"range" "0x0020 0x04FF" }
			"6"
			{
				"name"		"Trebuchet MS"
				"tall"		"12"
				"weight"		"700"
				"antialias" "1"
			"range" "0x0020 0x04FF" }
		}
		"DefaultVerySmall"
		{
			"1"
			{
				"name"		"Trebuchet MS"
				"tall"		"12"
				"weight"	"700"
				"yres"	"480 599"
				"antialias" "1"
			"range" "0x0020 0x04FF" }
			"2"
			{
				"name"		"Trebuchet MS"
				"tall"		"13"
				"weight"	"700"
				"yres"	"600 767"
				"antialias" "1"
			"range" "0x0020 0x04FF" }
			"3"
			{
				"name"		"Trebuchet MS"
				"tall"		"14"
				"weight"	"700"
				"yres"	"768 1023"
				"antialias" "1"
			"range" "0x0020 0x04FF" }
			"4"
			{
				"name"		"Trebuchet MS"
				"tall"		"20"
				"weight"	"700"
				"yres"	"1024 1199"
				"antialias" "1"
			"range" "0x0020 0x04FF" }
			"5"
			{
				"name"		"Trebuchet MS"
				"tall"		"24"
				"weight"	"700"
				"yres"	"1200 6000"
				"antialias" "1"
			"range" "0x0020 0x04FF" }
			"6"
			{
				"name"		"Trebuchet MS"
				"tall"		"12"
				"weight"		"700"
				"antialias" "1"
			"range" "0x0020 0x04FF" }
			"7"
			{
				"name"		"Trebuchet MS"
				"tall"		"11"
				"weight"		"700"
				"antialias" "1"
			"range" "0x0020 0x04FF" }
		}
		WeaponIcons
		{
			"1"
			{
				"name"		"HalfLife2"
				"tall"		"64"
				"tall_hidef"	"58"
				"weight"	"0"
				"antialias" "1"
				"additive"	"1"
				"custom"	"1"
			}
		}
		WeaponIconsSelected
		{
			"1"
			{
				"name"		"HalfLife2"
				"tall"		"64"
				"tall_hidef"	"58"
				"weight"	"0"
				"antialias" "1"
				"blur"		"5"
				"scanlines"	"2"
				"additive"	"1"
				"custom"	"1"
				"scanlines" "2"
			}
		}
		WeaponIconsSmall
		{
			"1"
			{
				"name"		"HalfLife2"
				"tall"		"32"
				"weight"	"0"
				"antialias" "1"
				"additive"	"1"
				"custom"	"1"
				"scanlines" "2"
			}
		}
		Crosshairs
		{
			"1"
			{
				"name"		"HalfLife2"
				"tall"		"40" [!$OSX]
				"tall"		"41" [$OSX]
				"weight"	"0"
				"antialias" "0"
				"additive"	"1"
				"custom"	"1"
				"yres"		"1 10000"
			}
		}
		QuickInfo
		{
			"1"	[$X360]
			{
				"name"		"HL2cross"
				"tall"		"57"
				"weight"	"0"
				"antialias" 	"1"
				"additive"	"1"
				"custom"	"1"
				"scanlines" "2"
			}
			"1"	[$WIN32]
			{
				"name"		"HL2cross"
				"tall"		"28" [!$OSX]
				"tall"		"50" [$OSX]
				"weight"	"0"
				"antialias" "1"
				"additive"	"1"
				"custom"	"1" [!$OSX]
				"scanlines" "2"
			}
		}
		HudNumbers
		{
			"1"
			{
				"name"		"Trebuchet MS"
				"tall"		"32"	[$WIN32]
				"tall"		"38"	[$X360]
				"weight"	"700"
				"additive"	"1"
				"antialias" "1"
			"range" "0x0020 0x04FF" }
		}
		SquadIcon	[$X360]
		{
			"1"
			{
				"name"		"HalfLife2"
				"tall"		"50"
				"weight"	"0"
				"antialias" "1"
				"additive"	"1"
				"custom"	"1"
			}
		}
		HudNumbersGlow
		{
			"1"
			{
				"name"		"Trebuchet MS"
				"tall"		"32"	[$WIN32]
				"tall"		"38"	[$X360]
				"weight"	"700"
				"blur"		"4"
				"additive"	"1"
				"antialias" "1"
			"range" "0x0020 0x04FF" }
		}
		HudNumbersSmall
		{
			"1"
			{
				"name"		"Trebuchet MS" [!$OSX]
				"name"		"Trebuchet MS" [$OSX]
				"tall"		"16"	[$WIN32]
				"tall"		"22"	[$X360]
				"weight"	"700"
				"additive"	"1"
				"antialias" "1"
			"range" "0x0020 0x04FF" }
		}
		HudSelectionNumbers
		{
			"1"
			{
				"name"		"Trebuchet MS"
				"tall"		"11"
				"weight"	"700"
				"additive"	"1"
				"antialias" "1"
			"range" "0x0020 0x04FF" }
		}
		HudHintTextLarge
		{
			"1"	[$X360]
			{
				"bitmap"	"1"
				"name"		"Buttons"
				"scalex"	"1.0"
				"scaley"	"1.0"
			}
			"1"	[$WIN32]
			{
				"name"		"Trebuchet MS" [!$OSX]
				"name"		"Trebuchet MS" [$OSX]
				"tall"		"14"
				"weight"	"700"
				"additive"	"1"
				"antialias" "1"
			"range" "0x0020 0x04FF" }
		}
		HudHintTextSmall
		{
			"1"	[$WIN32]
			{
				"name"		"Trebuchet MS" [!$OSX]
				"name"		"Trebuchet MS" [$OSX]
				"tall"		"11"
				"weight"	"700"
				"additive"	"1"
				"antialias" "1"
			"range" "0x0020 0x04FF" }
			"1"	[$X360]
			{
				"name"		"Trebuchet MS"
				"tall"		"12"
				"weight"	"700"
				"additive"	"1"
				"antialias" "1"
			"range" "0x0020 0x04FF" }
		}
		HudSelectionText
		{
			"1"
			{
				"name"		"Trebuchet MS"
				"tall"		"8"
				"weight"	"700"
				"yres"	"1 599"
				"antialias" "1"
			"range" "0x0020 0x04FF" }
			"2"
			{
				"name"		"Trebuchet MS"
				"tall"		"10"
				"weight"	"700"
				"yres"	"600 767"
				"antialias" "1"
			"range" "0x0020 0x04FF" }
			"3"
			{
				"name"		"Trebuchet MS"
				"tall"		"12"
				"weight"	"700"
				"yres"	"768 1023"
				"antialias" "1"
			"range" "0x0020 0x04FF" }
			"4"
			{
				"name"		"Trebuchet MS"
				"tall"		"16"
				"weight"	"700"
				"yres"	"1024 1199"
				"antialias" "1"
			"range" "0x0020 0x04FF" }
			"5"
			{
				"name"		"Trebuchet MS"
				"tall"		"17"
				"weight"	"700"
				"yres"	"1200 10000"
				"antialias" "1"
			"range" "0x0020 0x04FF" }
		}
		GameUIButtons
		{
			"1"	[$X360]
			{
				"bitmap"	"1"
				"name"		"Buttons"
				"scalex"	"0.63"
				"scaley"	"0.63"
				"scalex_hidef"	"1.0"
				"scaley_hidef"	"1.0"
			}
		}
		BudgetLabel
		{
			"1"
			{
				"name"		"Trebuchet MS"
				"tall"		"14"
				"weight"	"700"
				"outline"	"1"
				"antialias" "1"
			"range" "0x0020 0x04FF" }
		}
		DebugOverlay
		{
			"1"	[$WIN32]
			{
				"name"		"Trebuchet MS"
				"tall"		"14"
				"weight"	"700"
				"outline"	"1"
				"antialias" "1"
			"range" "0x0020 0x04FF" }
			"1"	[$X360]
			{
				"name"		"Trebuchet MS"
				"tall"		"18"
				"weight"	"700"
				"outline"	"1"
				"antialias" "1"
			"range" "0x0020 0x04FF" }
		}
		"CloseCaption_Normal"
		{
			"1"
			{
				"name"		"Trebuchet MS" [!$OSX]
				"name"		"Trebuchet MS" [$OSX]
				"tall"		"26" [!$OSX]
				"tall"		"24" [$OSX]
				"weight"	"700"
				"antialias" "1"
			"range" "0x0020 0x04FF" }
		}
		"CloseCaption_Italic"
		{
			"1"
			{
				"name"		"Trebuchet MS" [!$OSX]
				"name"		"Trebuchet MS" [$OSX]
				"tall"		"26" [!$OSX]
				"tall"		"24" [$OSX]
				"weight"	"700"
				"italic"	"1"
				"antialias" "1"
			"range" "0x0020 0x04FF" }
		}
		"CloseCaption_Bold"
		{
			"1"
			{
				"name"		"Trebuchet MS" [!$OSX]
				"name"		"Trebuchet MS" [$OSX]
				"tall"		"26" [!$OSX]
				"tall"		"24" [$OSX]
				"weight"	"700"
				"antialias" "1"
			"range" "0x0020 0x04FF" }
		}
		"CloseCaption_BoldItalic"
		{
			"1"
			{
				"name"		"Trebuchet MS" [!$OSX]
				"name"		"Trebuchet MS" [$OSX]
				"tall"		"26" [!$OSX]
				"tall"		"24" [$OSX]
				"weight"	"700"
				"italic"	"1"
				"antialias" "1"
			"range" "0x0020 0x04FF" }
		}
		"CloseCaption_Small"
		{
			"1"
			{
				"name"		"Trebuchet MS" [!$OSX]
				"name"		"Trebuchet MS" [$OSX]
				"tall"		"16" [!$OSX]
				"tall"		"14" [$OSX]
				"tall_hidef"	"24"
				"weight"	"700"
				"antialias" "1"
			"range" "0x0020 0x04FF" }
		}
		// this is the symbol font
		"Marlett"
		{
			"1"
			{
				"name"		"Marlett"
				"tall"		"14"
				"weight"	"0"
				"symbol"	"1"
			}
		}
		"Trebuchet24"
		{
			"1"
			{
				"name"		"Trebuchet MS"
				"tall"		"24"
				"weight"	"700"
				"additive"	"1"
				"antialias" "1"
			"range" "0x0020 0x04FF" }
		}
		"Trebuchet18"
		{
			"1"
			{
				"name"		"Trebuchet MS"
				"tall"		"18"
				"weight"	"700"
				"antialias" "1"
			"range" "0x0020 0x04FF" }
		}
		ClientTitleFont
		{
			"1"
			{
				"name"  "Trebuchet MS"
				"tall"			"34"
				"tall_hidef"	"48"
				"weight" "700"
				"additive" "0"
				"antialias" "1"
			"range" "0x0020 0x04FF" }
		}
		CreditsLogo
		{
			"1"
			{
				"name"		"HalfLife2"
				"tall"		"128"
				"weight"	"0"
				"antialias" "1"
				"additive"	"1"
				"custom"	"1"
			}
		}
		CreditsText
		{
			"1"
			{
				"name"		"Trebuchet MS"
				"tall"		"20"
				"weight"	"700"
				"additive"	"1"
				"antialias" "1"
			"range" "0x0020 0x04FF" }
		}
		CreditsOutroLogos
		{
			"1"
			{
				"name"		"HalfLife2"
				"tall"		"48"
				"weight"	"0"
				"antialias" "1"
				"additive"	"1"
				"custom"	"1"
			}
		}
		CreditsOutroText
		{
			"1"
			{
				"name"		"Trebuchet MS" [!$OSX]
				"name"		"Trebuchet MS" [$OSX]
				"tall"		"9"
				"weight"	"700"
				"antialias" "1"
			"range" "0x0020 0x04FF" }
		}
		CenterPrintText
		{
			// note that this scales with the screen resolution
			"1"
			{
				"name"		"Trebuchet MS" [!$OSX]
				"name"		"Trebuchet MS" [$OSX]
				"tall"		"18"
				"weight"	"700"
				"additive"	"1"
				"antialias" "1"
			"range" "0x0020 0x04FF" }
		}
		HDRDemoText
		{
			// note that this scales with the screen resolution
			"1"
			{
				"name"		"Trebuchet MS"
				"tall"		"24"
				"weight"	"700"
				"additive"	"1"
				"antialias" "1"
			"range" "0x0020 0x04FF" }
		}
		"AchievementNotification"
		{
			"1"
			{
				"name"		"Trebuchet MS"
				"tall"		"14"
				"weight"	"700"
				"antialias" "1"
			"range" "0x0020 0x04FF" }
		}
		"CommentaryDefault"
		{
			"1"
			{
				"name"		"Trebuchet MS"
				"tall"		"12"
				"weight"	"700"
				"yres"	"480 599"
				"antialias" "1"
			"range" "0x0020 0x04FF" }
			"2"
			{
				"name"		"Trebuchet MS"
				"tall"		"13"	[$WIN32]
				"tall"		"20"	[$X360]
				"weight"	"700"
				"yres"	"600 767"
				"antialias" "1"
			"range" "0x0020 0x04FF" }
			"3"
			{
				"name"		"Trebuchet MS"
				"tall"		"14"
				"weight"	"700"
				"yres"	"768 1023"
				"antialias" "1"
			"range" "0x0020 0x04FF" }
			"4"
			{
				"name"		"Trebuchet MS"
				"tall"		"20"
				"weight"	"700"
				"yres"	"1024 1199"
				"antialias" "1"
			"range" "0x0020 0x04FF" }
			"5"
			{
				"name"		"Trebuchet MS"
				"tall"		"24"
				"weight"	"700"
				"yres"	"1200 6000"
				"antialias" "1"
			"range" "0x0020 0x04FF" }
			"6"
			{
				"name"		"Trebuchet MS"
				"tall"		"12"
				"weight"		"700"
				"antialias" "1"
			"range" "0x0020 0x04FF" }
			"7"
			{
				"name"		"Trebuchet MS"
				"tall"		"12"
				"weight"		"700"
				"antialias" "1"
			"range" "0x0020 0x04FF" }

		}
	}


	//////////////////////// CUSTOM FONT FILES /////////////////////////////
	//
	// specifies all the custom (non-system) font files that need to be loaded to service the above described fonts
	CustomFontFiles
	{
		"1"		"resource/HALFLIFE2.ttf"
		"2"		"resource/HL2crosshairs.ttf"
		"3"		"resource/HL2EP2.ttf"
		"4" 	"resource/Capture_it.ttf"
		"5" 	"resource/Capture_it_2.ttf"
	}

}
