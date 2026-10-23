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
		// base colors
		"White"				"242 242 242 255"
		"OffWhite"			"214 214 214 255"
		"DullWhite"			"140 140 140 255"
		"Orange"			"224 224 224 255"
		"TransparentBlack"	"10 10 10 248"
		"Black"				"0 0 0 255"

		"Blank"				"0 0 0 0"

		"SteamLightGreen"	"245 245 245 255"
		"AchievementsLightGrey"		"79 79 79 255"
		"AchievementsDarkGrey"		"55 55 55 255"
		"AchievementsInactiveFG"	"130 130 130 255"
		"SCP.Console.Edge" "57 57 57 255"
		"SCP.Console.EdgeSoft" "34 34 34 255"
		"SCP.Console.EdgeActive" "112 112 112 255"
		"BaseText" "214 214 214 255"
		"BrightBaseText" "242 242 242 255"
		"SelectedText" "255 255 255 255"
		"DimBaseText" "140 140 140 255"
		"LabelDimText" "140 140 140 255"
		"ControlText" "214 214 214 255"
		"BrightControlText" "242 242 242 255"
		"ControlBG" "12 12 12 255"
		"ControlDarkBG" "17 17 17 255"
		"WindowBG" "8 8 8 255"
		"SelectionBG" "50 50 50 255"
		"SelectionBG2" "35 35 35 255"
		"ListBG" "8 8 8 255"
		"TitleText" "235 235 235 255"
		"TitleDimText" "110 110 110 255"
		"TitleBG" "16 16 16 255"
		"TitleDimBG" "12 12 12 255"
		"BorderBright" "57 57 57 255"
		"BorderDark" "34 34 34 255"
		"BorderSelection" "112 112 112 255"
	}

	///////////////////// BASE SETTINGS ////////////////////////
	//
	// default settings for all panels
	// controls use these to determine their settings
	BaseSettings
	{
		// vgui_controls color specifications
		Border.Bright					"57 57 57 255"	// the lit side of a control
		Border.Dark						"34 34 34 255"		// the dark/unlit side of a control
		Border.Selection				"112 112 112 255"			// the additional border color for displaying the default/selected button

		Button.TextColor				"204 204 204 255"
		Button.BgColor					"24 24 24 255"
		Button.ArmedTextColor			"255 255 255 255"
		Button.ArmedBgColor				"43 43 43 255"				[$WIN32]
		Button.ArmedBgColor				"43 43 43 255"		[$X360]
		Button.DepressedTextColor		"255 255 255 255"
		Button.DepressedBgColor			"17 17 17 255"
		Button.FocusBorderColor			"112 112 112 255"

		CheckButton.TextColor			"White"
		CheckButton.SelectedTextColor		"White"
		CheckButton.BgColor			"TransparentBlack"
		CheckButton.Border1  			"Border.Dark" 		// the left checkbutton border
		CheckButton.Border2  			"Border.Bright"		// the right checkbutton border
		CheckButton.Check			"White"	// color of the check itself
		CheckButton.HighlightFgColor		"OffWhite"
		CheckButton.ArmedBgColor		"Blank"
		CheckButton.DepressedBgColor		"Blank"
		CheckButton.DisabledBgColor	   	"TransparentBlack"

		ComboBoxButton.ArrowColor		"DullWhite"
		ComboBoxButton.ArmedArrowColor	"White"
		ComboBoxButton.BgColor			"Blank"
		ComboBoxButton.DisabledBgColor	"Blank"

		Frame.TitleTextInsetX			"14"
		Frame.ClientInsetX				"10"
		Frame.ClientInsetY				"8"
		Frame.BgColor					"12 12 12 250"	[$WIN32]
		Frame.BgColor					"12 12 12 250"		[$X360]
		Frame.OutOfFocusBgColor			"10 10 10 245"	[$WIN32]
		Frame.OutOfFocusBgColor			"10 10 10 245"		[$X360]
		Frame.FocusTransitionEffectTime	"0.12"							// time it takes for a window to fade in/out on focus/out of focus
		Frame.TransitionEffectTime		"0.18"				[$WIN32]	// time it takes for a window to fade in/out on open/close
		Frame.TransitionEffectTime		"0.18"				[$X360]		// time it takes for a window to fade in/out on open/close
		Frame.AutoSnapRange				"0"
		FrameGrip.Color1				"90 90 90 150"
		FrameGrip.Color2				"14 14 14 255"
		FrameTitleButton.FgColor		"200 200 200 255"
		FrameTitleButton.BgColor		"0 0 0 0"
		FrameTitleButton.DisabledFgColor	"90 90 90 255"
		FrameTitleButton.DisabledBgColor	"0 0 0 0"
		FrameSystemButton.FgColor		"Blank"
		FrameSystemButton.BgColor		"Blank"
		FrameSystemButton.Icon			""
		FrameSystemButton.DisabledIcon	""
		FrameTitleBar.Font				"SCPConsoleHeading"		[$WIN32]
		FrameTitleBar.Font				"SCPConsoleHeading"	[$WIN32]
		FrameTitleBar.TextColor			"222 222 222 255"
		FrameTitleBar.BgColor			"16 16 16 255"
		FrameTitleBar.DisabledTextColor	"110 110 110 255"
		FrameTitleBar.DisabledBgColor	"12 12 12 255"

		GraphPanel.FgColor				"White"
		GraphPanel.BgColor				"TransparentBlack"

		Label.TextDullColor				"DullWhite"
		Label.TextColor					"OffWhite"
		Label.TextBrightColor			"White"
		Label.SelectedTextColor			"White"
		Label.BgColor					"Blank"
		Label.DisabledFgColor1			"117 117 117 255"
		Label.DisabledFgColor2			"30 30 30 255"

		ListPanel.TextColor					"OffWhite"
		ListPanel.TextBgColor				"Blank"
		ListPanel.BgColor					"TransparentBlack"
		ListPanel.SelectedTextColor			"Black"
		ListPanel.SelectedBgColor			"Orange"
		ListPanel.SelectedOutOfFocusBgColor	"28 28 28 128"
		ListPanel.EmptyListInfoTextColor	"OffWhite"

		Menu.TextColor					"202 202 202 255"
		Menu.BgColor					"18 18 18 255"
		Menu.ArmedTextColor				"255 255 255 255"
		Menu.ArmedBgColor				"43 43 43 255"
		Menu.TextInset					"8"

		Panel.FgColor					"DullWhite"
		Panel.BgColor					"Blank"

		ProgressBar.FgColor				"White"
		ProgressBar.BgColor				"TransparentBlack"

		PropertySheet.TextColor			"OffWhite"
		PropertySheet.SelectedTextColor	"White"
		PropertySheet.TransitionEffectTime	"0.25"	// time to change from one tab to another

		RadioButton.TextColor			"OffWhite"
		RadioButton.SelectedTextColor	"White"

		RichText.TextColor				"214 214 214 255"
		RichText.BgColor				"8 8 8 255"
		RichText.SelectedTextColor		"255 255 255 255"
		RichText.SelectedBgColor		"65 65 65 255"

		ScrollBar.Wide					"10"

		ScrollBarButton.FgColor				"135 135 135 255"
		ScrollBarButton.BgColor				"13 13 13 255"
		ScrollBarButton.ArmedFgColor		"238 238 238 255"
		ScrollBarButton.ArmedBgColor		"30 30 30 255"
		ScrollBarButton.DepressedFgColor	"255 255 255 255"
		ScrollBarButton.DepressedBgColor	"20 20 20 255"

		ScrollBarSlider.FgColor				"67 67 67 255"			// nob color
		ScrollBarSlider.BgColor				"13 13 13 255"	// slider background color

		SectionedListPanel.HeaderTextColor	"White"
		SectionedListPanel.HeaderBgColor	"Blank"
		SectionedListPanel.DividerColor		"Black"
		SectionedListPanel.TextColor		"DullWhite"
		SectionedListPanel.BrightTextColor	"White"
		SectionedListPanel.BgColor			"TransparentBlack"
		SectionedListPanel.SelectedTextColor			"Black"
		SectionedListPanel.SelectedBgColor				"Orange"
		SectionedListPanel.OutOfFocusSelectedTextColor	"Black"
		SectionedListPanel.OutOfFocusSelectedBgColor	"28 28 28 128"

		Slider.NobColor				"108 108 108 255"
		Slider.TextColor			"180 180 180 255"
		Slider.TrackColor			"31 31 31 255"
		Slider.DisabledTextColor1	"117 117 117 255"
		Slider.DisabledTextColor2	"30 30 30 255"

		TextEntry.TextColor			"238 238 238 255"
		TextEntry.BgColor			"19 19 19 255"
		TextEntry.CursorColor		"245 245 245 255"
		TextEntry.DisabledTextColor	"110 110 110 255"
		TextEntry.DisabledBgColor	"14 14 14 255"
		TextEntry.SelectedTextColor	"255 255 255 255"
		TextEntry.SelectedBgColor	"65 65 65 255"
		TextEntry.OutOfFocusSelectedBgColor	"38 38 38 255"
		TextEntry.FocusEdgeColor	"112 112 112 255"

		ToggleButton.SelectedTextColor	"White"

		Tooltip.TextColor			"0 0 0 196"
		Tooltip.BgColor				"Orange"

		TreeView.BgColor			"TransparentBlack"

		WizardSubPanel.BgColor		"Blank"

		// scheme-specific colors
		MainMenu.TextColor			"White"				[$WIN32]
		MainMenu.TextColor			"200 200 200 255"	[$X360]
		MainMenu.ArmedTextColor		"200 200 200 255"	[$WIN32]
		MainMenu.ArmedTextColor		"White"				[$X360]
		MainMenu.DepressedTextColor	"245 245 245 255"
		MainMenu.MenuItemHeight		"30"				[$WIN32]
		MainMenu.MenuItemHeight			"22"				[$X360]
		MainMenu.MenuItemHeight_hidef	"32"				[$X360]
		MainMenu.Inset				"32"
		MainMenu.Backdrop			"0 0 0 156"

		Console.TextColor			"214 214 214 255"
		Console.DevTextColor		"156 156 156 255"

		NewGame.TextColor			"White"
		NewGame.FillColor			"0 0 0 255"
		NewGame.SelectionColor		"Orange"			[$WIN32]
		NewGame.SelectionColor		"0 0 0 255"			[$X360]
		NewGame.DisabledColor		"128 128 128 196"

		MessageDialog.MatchmakingBG			"28 28 28 255"	[$X360]
		MessageDialog.MatchmakingBGBlack			"22 22 22 255"	[$X360]

		MatchmakingMenuItemTitleColor			"245 245 245 255"	[$X360]
		MatchmakingMenuItemDescriptionColor		"245 245 245 255"	[$X360]

		"QuickListBGDeselected"		"AchievementsDarkGrey"
	 	"QuickListBGSelected"           "AchievementsLightGrey"
		"FrameTitleBar.SmallFont" "SCPConsoleHeadingSmall"
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
		// fonts are used in order that they are listed
		// fonts listed later in the order will only be used if they fulfill a range not already filled
		// if a font fails to load then the subsequent fonts will replace
		// fonts are used in order that they are listed
		"DebugFixed"
		{
			"1"
			{
				"name"		"Trebuchet MS"
				"tall"		"10"
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
				"tall"		"7"
				"weight"	"700"
				"antialias" "1"
			"range" "0x0020 0x04FF" }
		}
		"DefaultFixedOutline"
		{
			"1"
			{
				"name"		 "Trebuchet MS" [!$OSX]
				"name"		 "Trebuchet MS" [$X360]
				"name"		 "Trebuchet MS" [$OSX]
				"tall"		"14" [$LINUX]
				"tall"		"11" [$OSX]
				"tall"		 "10"
				"tall_lodef" "15"
				"tall_hidef" "20"
				"weight"	 "700"
				"outline"	 "1"
			"range" "0x0020 0x04FF" }
		}
		"Default"
		{
			"1"
			{
				"name" "Trebuchet MS"
				"tall" "14"
				"weight" "700"
				"antialias" "1"
			"range" "0x0020 0x04FF" }
		}
		"DefaultBold"
		{
			"1"
			{
				"name" "Trebuchet MS"
				"tall" "14"
				"weight" "700"
				"antialias" "1"
			"range" "0x0020 0x04FF" }
		}
		"DefaultUnderline"
		{
			"1"
			{
				"name"		"Trebuchet MS" [!$OSX]
				"name"		"Trebuchet MS" [$OSX]
				"tall"		"16" [!$LINUX]
				"tall"		"18" [$LINUX]
				"weight"	"700"
				"underline" "1"
			"range" "0x0020 0x04FF" }
		}
		"DefaultSmall"
		{
			"1"
			{
				"name"		"Trebuchet MS" [!$OSX]
				"name"		"Trebuchet MS" [$OSX]
				"tall"		"12" [!$POSIX]
				"tall"		"13" [$POSIX]
				"weight"	"700"
			"range" "0x0020 0x04FF" }
		}
		"DefaultSmallDropShadow"
		{
			"1"
			{
				"name"		"Trebuchet MS" [!$OSX]
				"name"		"Trebuchet MS" [$OSX]
				"tall"		"13"
				"weight"	"700"
				"dropshadow" "1"
			"range" "0x0020 0x04FF" }
		}
		"DefaultVerySmall"
		{
			"1"
			{
				"name"		"Trebuchet MS" [!$OSX]
				"name"		"Trebuchet MS" [$OSX]
				"tall"		"12"
				"weight"	"700"
			"range" "0x0020 0x04FF" }
		}

		"DefaultLarge"
		{
			"1"
			{
				"name"		"Trebuchet MS" [!$OSX]
				"name"		"Trebuchet MS" [$OSX]
				"tall"		"18"
				"weight"	"700"
			"range" "0x0020 0x04FF" }
		}
		"UiBold"
		{
			"1"
			{
				"name" "Trebuchet MS"
				"tall" "16"
				"weight" "700"
				"antialias" "1"
			"range" "0x0020 0x04FF" }
		}
		"ChapterTitle"	[$X360]
		{
			"1"
			{
				"name"			"Trebuchet MS"
				"tall"			"20"
				"tall_hidef"	"28"
				"weight"		"700"
				"outline"		"1"
			"range" "0x0020 0x04FF" }
		}
		"ChapterTitleBlur"	[$X360]
		{
			"1"
			{
				"name"			"Trebuchet MS"
				"tall"			"20"
				"tall_hidef"	"28"
				"weight"		"700"
				"blur"			"3"
				"blur_hidef"	"5"
			"range" "0x0020 0x04FF" }
		}
		"MenuLarge"
		{
			"1"	[$OSX]
			{
				"name"		"Trebuchet MS"
				"tall"		"20"
				"weight" "700"
				"antialias" "1"
			"range" "0x0020 0x04FF" }
			"1"	[$LINUX]
			{
				"name"		"Trebuchet MS"
				"tall"		"24"
				"weight"	"700"
				"antialias" "1"
			"range" "0x0020 0x04FF" }
			"1"	[$WINDOWS]
			{
				"name"		"Trebuchet MS"
				"tall"		"16"
				"weight"	"700"
				"antialias" "1"
			"range" "0x0020 0x04FF" }
			"1"	[$X360]
			{
				"name"		"Trebuchet MS"
				"tall"			"14"
				"tall_hidef"	"20"
				"weight"	"700"
				"antialias" "1"
				"outline" "1"
			"range" "0x0020 0x04FF" }
		}
		"AchievementTitleFont"
		{
			"1"
			{
				"name"		"Trebuchet MS"
				"tall"		"20"
				"weight"	"700"
				"antialias" "1"
				"outline" "1"
			"range" "0x0020 0x04FF" }
		}

		"AchievementTitleFontSmaller"
		{
			"1"
			{
				"name"		"Trebuchet MS"
				"tall"		"18"
				"weight"	"700"
				"antialias" "1"
				//"outline" "1"
			"range" "0x0020 0x04FF" }
		}


		"AchievementDescriptionFont"
		{
			"1"
			{
				"name"		"Trebuchet MS"
				"tall"		"15"
				"weight"	"700"
				"antialias" "1"
				"outline" "1"
				"yres"		"0 480"
			"range" "0x0020 0x04FF" }
			"2"
			{
				"name"		"Trebuchet MS"
				"tall"		"20"
				"weight"	"700"
				"antialias" "1"
				"outline" "1"
				"yres"	 "481 10000"
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
				"scalex_lodef"	"0.75"
				"scaley_lodef"	"0.75"
			}
		}
		"ConsoleText"
		{
			"1"
			{
				"name"		 "Trebuchet MS" [!$OSX]
				"name"		 "Trebuchet MS" [$X360]
				"name"		 "Trebuchet MS" [$OSX]
				"tall"		"11" [$OSX]
				"tall"		"14" [$LINUX]
				"tall"		"10"
				"weight"	"700"
			"range" "0x0020 0x04FF" }
		}

		// this is the symbol font
		"Marlett"
		{
			"1"
			{
				"name" "SCP Console Symbols"
				"tall" "16"
				"weight" "400"
				"antialias" "1"
			}
		}

		"Trebuchet24"
		{
			"1"
			{
				"name"		"Trebuchet MS"
				"tall"		"24"
				"weight"	"700"
			"range" "0x0020 0x04FF" }
		}

		"Trebuchet20"
		{
			"1"
			{
				"name"		"Trebuchet MS"
				"tall"		"20"
				"weight"	"700"
			"range" "0x0020 0x04FF" }
		}

		"Trebuchet18"
		{
			"1"
			{
				"name"		"Trebuchet MS"
				"tall"		"18"
				"weight"	"700"
			"range" "0x0020 0x04FF" }
		}

		// HUD numbers
		// We use multiple fonts to 'pulse' them in the HUD, hence the need for many of near size
		"HUDNumber"
		{
			"1"
			{
				"name"		"Trebuchet MS"
				"tall"		"40"
				"weight"	"700"
			"range" "0x0020 0x04FF" }
		}
		"HUDNumber1"
		{
			"1"
			{
				"name"		"Trebuchet MS"
				"tall"		"41"
				"weight"	"700"
			"range" "0x0020 0x04FF" }
		}
		"HUDNumber2"
		{
			"1"
			{
				"name"		"Trebuchet MS"
				"tall"		"42"
				"weight"	"700"
			"range" "0x0020 0x04FF" }
		}
		"HUDNumber3"
		{
			"1"
			{
				"name"		"Trebuchet MS"
				"tall"		"43"
				"weight"	"700"
			"range" "0x0020 0x04FF" }
		}
		"HUDNumber4"
		{
			"1"
			{
				"name"		"Trebuchet MS"
				"tall"		"44"
				"weight"	"700"
			"range" "0x0020 0x04FF" }
		}
		"HUDNumber5"
		{
			"1"
			{
				"name"		"Trebuchet MS"
				"tall"		"45"
				"weight"	"700"
			"range" "0x0020 0x04FF" }
		}
		"DefaultFixed"
		{
			"1"
			{
				"name"		 "Trebuchet MS" [$WINDOWS]
				"name"		 "Trebuchet MS" [$X360]
				"name"		 "Trebuchet MS" [$POSIX]
				"tall"		"11" [$POSIX]
				"tall"		"10"
				"weight"	"700"
			"range" "0x0020 0x04FF" }
//			"1"
//			{
//				"name"		"Trebuchet MS"
//				"tall"		"20"
//				"weight"	"700"
//			"range" "0x0020 0x04FF" }
		}

		"DefaultFixedDropShadow"
		{
			"1"
			{
				"name"		 "Trebuchet MS" [$WINDOWS]
				"name"		 "Trebuchet MS" [$X360]
				"name"		 "Trebuchet MS" [$OSX]
				"name"		 "Trebuchet MS" [$LINUX]
				"tall"		"14" [$LINUX]
				"tall"		"11" [$POSIX]
				"tall"		"10"
				"weight"	"700"
				"dropshadow" "1"
			"range" "0x0020 0x04FF" }
//			"1"
//			{
//				"name"		"Trebuchet MS"
//				"tall"		"20"
//				"weight"	"700"
//			"range" "0x0020 0x04FF" }
		}

		"CloseCaption_Normal"
		{
			"1"
			{
				"name"		"Trebuchet MS" [!$OSX]
				"name"		"Trebuchet MS" [$OSX]
				"tall"		"12"
				"weight"	"700"
				"antialias"	"1"
			"range" "0x0020 0x04FF" }
		}
		"CloseCaption_Italic"
		{
			"1"
			{
				"name"		"Trebuchet MS" [!$OSX]
				"name"		"Trebuchet MS" [$OSX]
				"tall"		"12"
				"weight"	"700"
				"italic"	"1"
				"antialias"	"1"
			"range" "0x0020 0x04FF" }
		}
		"CloseCaption_Bold"
		{
			"1"
			{
				"name"		"Trebuchet MS" [!$OSX]
				"name"		"Trebuchet MS" [$OSX]
				"tall"		"12"
				"weight"	"700"
				"antialias"	"1"
			"range" "0x0020 0x04FF" }
		}
		"CloseCaption_BoldItalic"
		{
			"1"
			{
				"name"		"Trebuchet MS" [!$OSX]
				"name"		"Trebuchet MS" [$OSX]
				"tall"		"12"
				"weight"	"700"
				"italic"	"1"
				"antialias"	"1"
			"range" "0x0020 0x04FF" }
		}
		"CloseCaption_Small"
		{
			"1"
			{
				"name"		"Trebuchet MS" [!$OSX]
				"name"		"Trebuchet MS" [$OSX]
				"tall"		"12"
				"weight"	"700"
				"antialias"	"1"
			"range" "0x0020 0x04FF" }
		}

		TitleFont
		{
			"1"
			{
				"name"		"HalfLife2"
				"tall"		"72"
				"weight"	"400"
				"antialias"	"1"
				"custom"	"1"
			}
		}

		TitleFont2
		{
			"1"
			{
				"name"		"HalfLife2"
				"tall"		"120"
				"weight"	"400"
				"antialias"	"1"
				"custom"	"1"
			}
		}

		AppchooserGameTitleFont	[$X360]
		{
			"1"
			{
				"name"			"Trebuchet MS"
				"tall"			"16"
				"tall_hidef"	"24"
				"weight"		"700"
				"antialias"		"1"
			"range" "0x0020 0x04FF" }
		}

		AppchooserGameTitleFontBlur	[$X360]
		{
			"1"
			{
				"name"			"Trebuchet MS"
				"tall"			"16"
				"tall_hidef"	"24"
				"weight"		"700"
				"blur"			"3"
				"blur_hidef"	"5"
				"antialias"		"1"
			"range" "0x0020 0x04FF" }
		}

		StatsTitle	[$WIN32]
		{
			"1"
			{
				"name"		"Trebuchet MS" [!$POSIX]
				"name"		"Trebuchet MS" [$POSIX]
				"weight"		"700"
				"tall"			"20"
				"antialias"		"1"
			"range" "0x0020 0x04FF" }
		}

		StatsText	[$WIN32]
		{
			"1"
			{
				"name"		"Trebuchet MS" [!$POSIX]
				"name"		"Trebuchet MS" [$POSIX]
				"weight"		"700"
				"tall"			"18"
				"antialias"		"1"
			"range" "0x0020 0x04FF" }
		}

		AchievementItemTitle	[$WIN32]
		{
			"1"
			{
				"name"		"Trebuchet MS" [!$POSIX]
				"name"		"Trebuchet MS" [$POSIX]
				"weight"		"700"
				"tall"			"16" [!$POSIX]
				"tall"			"18" [$POSIX]
				"antialias"		"1"
			"range" "0x0020 0x04FF" }
		}

		AchievementItemDate	[$WIN32]
		{
			"1"
			{
				"name"		"Trebuchet MS" [!$POSIX]
				"name"		"Trebuchet MS" [$POSIX]
				"weight"		"700"
				"tall"			"16"
				"antialias"		"1"
			"range" "0x0020 0x04FF" }
		}


		StatsPageText
		{
			"1"
			{
				"name"		"Trebuchet MS" [!$POSIX]
				"name"		"Trebuchet MS" [$POSIX]
				"weight"		"700"
				"tall"			"14" [!$POSIX]
				"tall"			"16" [$POSIX]
				"antialias"		"1"
			"range" "0x0020 0x04FF" }
		}

		AchievementItemTitleLarge	[$WIN32]
		{
			"1"
			{
				"name"		"Trebuchet MS" [!$POSIX]
				"name"		"Trebuchet MS" [$POSIX]
				"weight"		"700"
				"tall"			"18" [!$POSIX]
				"tall"			"19" [$POSIX]
				"antialias"		"1"
			"range" "0x0020 0x04FF" }
		}

		AchievementItemDescription	[$WIN32]
		{
			"1"
			{
				"name"		"Trebuchet MS" [!$POSIX]
				"name"		"Trebuchet MS" [$OSX]
				"weight"		"700"
				"tall"			"14" [!$POSIX]
				"tall"			"15" [$POSIX]
				"antialias"		"1"
			"range" "0x0020 0x04FF" }
		}


		"ServerBrowserTitle"
		{
			"1"
			{
				"name"		"Trebuchet MS" [!$OSX]
				"name"		"Trebuchet MS" [$OSX]
				"tall"		"35"
				"tall_lodef"	"40"
				"weight"	"700"
				"additive"	"0"
				"antialias" "1"
			"range" "0x0020 0x04FF" }
		}

		"ServerBrowserSmall"
		{
			"1"
			{
				"name"		"Trebuchet MS"
				"tall"		"16"
				"weight"	"700"
				"yres"	"480 599"
			"range" "0x0020 0x04FF" }
			"2"
			{
				"name"		"Trebuchet MS"
				"tall"		"16"
				"weight"	"700"
				"yres"	"600 767"
			"range" "0x0020 0x04FF" }
			"3"
			{
				"name"		"Trebuchet MS"
				"tall"		"16"
				"weight"	"700"
				"yres"	"768 1023"
				"antialias"	"1"
			"range" "0x0020 0x04FF" }
			"4"
			{
				"name"		"Trebuchet MS"
				"tall"		"19"
				"weight"	"700"
				"yres"	"1024 1199"
				"antialias"	"1"
			"range" "0x0020 0x04FF" }
			"5"
			{
				"name"		"Trebuchet MS"
				"tall"		"19"
				"weight"	"700"
				"yres"	"1200 6000"
				"antialias"	"1"
			"range" "0x0020 0x04FF" }
		}
		"SCPConsoleHeading"
		{
			"1"
			{
				"name" "Trebuchet MS"
				"tall" "16"
				"weight" "700"
				"antialias" "1"
			"range" "0x0020 0x04FF" }
		}
		"SCPConsoleHeadingSmall"
		{
			"1"
			{
				"name" "Trebuchet MS"
				"tall" "13"
				"weight" "700"
				"antialias" "1"
			"range" "0x0020 0x04FF" }
		}
		"MarlettSmall"
		{
			"1"
			{
				"name" "SCP Console Symbols"
				"tall" "12"
				"weight" "400"
				"antialias" "1"
			}
		}
	}

	//
	//////////////////// BORDERS //////////////////////////////
	//
	// describes all the border types
	Borders
	{
		BaseBorder		DepressedBorder
		ButtonBorder	RaisedBorder
		ComboBoxBorder	DepressedBorder
		MenuBorder		RaisedBorder
		BrowserBorder	DepressedBorder
		PropertySheetBorder	RaisedBorder

		FrameBorder
		{
			// rounded corners for frames
			"backgroundtype" "2"
		}

		DepressedBorder
		{
			"inset" "0 0 1 1"
			Left
			{
				"1"
				{
					"color" "Border.Dark"
					"offset" "0 1"
				}
			}

			Right
			{
				"1"
				{
					"color" "Border.Bright"
					"offset" "1 0"
				}
			}

			Top
			{
				"1"
				{
					"color" "Border.Dark"
					"offset" "0 0"
				}
			}

			Bottom
			{
				"1"
				{
					"color" "Border.Bright"
					"offset" "0 0"
				}
			}
		}
		RaisedBorder
		{
			"inset" "0 0 1 1"
			Left
			{
				"1"
				{
					"color" "Border.Bright"
					"offset" "0 1"
				}
			}

			Right
			{
				"1"
				{
					"color" "Border.Dark"
					"offset" "0 0"
				}
			}

			Top
			{
				"1"
				{
					"color" "Border.Bright"
					"offset" "0 1"
				}
			}

			Bottom
			{
				"1"
				{
					"color" "Border.Dark"
					"offset" "0 0"
				}
			}
		}

		TitleButtonBorder
		{
			"backgroundtype" "0"
			"inset" "0 0 0 0"
		}

		TitleButtonDisabledBorder
		{
			"backgroundtype" "0"
			"inset" "0 0 0 0"
		}

		TitleButtonDepressedBorder
		{
			"backgroundtype" "0"
			"inset" "0 0 0 0"
		}

		ScrollBarButtonBorder
		{
			"inset" "2 2 0 0" [!$OSX]
			"inset" "-3 2 0 0" [$OSX]
			Left
			{
				"1"
				{
					"color" "Border.Bright"
					"offset" "0 1"
				}
			}

			Right
			{
				"1"
				{
					"color" "Border.Dark"
					"offset" "1 0"
				}
			}

			Top
			{
				"1"
				{
					"color" "Border.Bright"
					"offset" "0 0"
				}
			}

			Bottom
			{
				"1"
				{
					"color" "Border.Dark"
					"offset" "0 0"
				}
			}
		}

		ScrollBarButtonDepressedBorder
		{
			"inset" "2 2 0 0" [!$OSX]
			"inset" "-3 2 0 0" [$OSX]
			Left
			{
				"1"
				{
					"color" "Border.Dark"
					"offset" "0 1"
				}
			}

			Right
			{
				"1"
				{
					"color" "Border.Bright"
					"offset" "1 0"
				}
			}

			Top
			{
				"1"
				{
					"color" "Border.Dark"
					"offset" "0 0"
				}
			}

			Bottom
			{
				"1"
				{
					"color" "Border.Bright"
					"offset" "0 0"
				}
			}
		}

		TabBorder
		{
			"inset" "0 0 1 1"
			Left
			{
				"1"
				{
					"color" "Border.Bright"
					"offset" "0 1"
				}
			}

			Right
			{
				"1"
				{
					"color" "Border.Dark"
					"offset" "1 0"
				}
			}

			Top
			{
				"1"
				{
					"color" "Border.Bright"
					"offset" "0 0"
				}
			}

		}

		TabActiveBorder
		{
			"inset" "0 0 1 0"
			Left
			{
				"1"
				{
					"color" "Border.Bright"
					"offset" "0 0"
				}
			}

			Right
			{
				"1"
				{
					"color" "Border.Dark"
					"offset" "1 0"
				}
			}

			Top
			{
				"1"
				{
					"color" "Border.Bright"
					"offset" "0 0"
				}
			}

		}


		ToolTipBorder
		{
			"inset" "0 0 1 0"
			Left
			{
				"1"
				{
					"color" "Border.Dark"
					"offset" "0 0"
				}
			}

			Right
			{
				"1"
				{
					"color" "Border.Dark"
					"offset" "1 0"
				}
			}

			Top
			{
				"1"
				{
					"color" "Border.Dark"
					"offset" "0 0"
				}
			}

			Bottom
			{
				"1"
				{
					"color" "Border.Dark"
					"offset" "0 0"
				}
			}
		}

		// this is the border used for default buttons (the button that gets pressed when you hit enter)
		ButtonKeyFocusBorder
		{
			"inset" "0 0 1 1"
			Left
			{
				"1"
				{
					"color" "Border.Selection"
					"offset" "0 0"
				}
				"2"
				{
					"color" "Border.Bright"
					"offset" "0 1"
				}
			}
			Top
			{
				"1"
				{
					"color" "Border.Selection"
					"offset" "0 0"
				}
				"2"
				{
					"color" "Border.Bright"
					"offset" "1 0"
				}
			}
			Right
			{
				"1"
				{
					"color" "Border.Selection"
					"offset" "0 0"
				}
				"2"
				{
					"color" "Border.Dark"
					"offset" "1 0"
				}
			}
			Bottom
			{
				"1"
				{
					"color" "Border.Selection"
					"offset" "0 0"
				}
				"2"
				{
					"color" "Border.Dark"
					"offset" "0 0"
				}
			}
		}

		ButtonDepressedBorder
		{
			"inset" "2 1 1 1"
			Left
			{
				"1"
				{
					"color" "Border.Dark"
					"offset" "0 1"
				}
			}

			Right
			{
				"1"
				{
					"color" "Border.Bright"
					"offset" "1 0"
				}
			}

			Top
			{
				"1"
				{
					"color" "Border.Dark"
					"offset" "0 0"
				}
			}

			Bottom
			{
				"1"
				{
					"color" "Border.Bright"
					"offset" "0 0"
				}
			}
		}
	}

	//////////////////////// CUSTOM FONT FILES /////////////////////////////
	//
	// specifies all the custom (non-system) font files that need to be loaded to service the above described fonts
	CustomFontFiles
	{
		"1"		"resource/HALFLIFE2.ttf"
		"2"		"resource/HL2EP2.ttf"
		"3"		"resource/marlett.ttf"

		"4"		"resource/linux_fonts/DejaVuSans.ttf"
		"5"		"resource/linux_fonts/DejaVuSans-Bold.ttf"
		"6"		"resource/linux_fonts/DejaVuSans-BoldOblique.ttf"
		"7"		"resource/linux_fonts/DejaVuSans-Oblique.ttf"
		"8"		"resource/linux_fonts/LiberationSans-Regular.ttf"
		"9"		"resource/linux_fonts/LiberationSans-Bold.ttf"
		"10"		"resource/linux_fonts/LiberationMono-Regular.ttf"
		"SCPConsoleText" "resource/fonts/scp_console_text.ttf"
		"SCPConsoleHeading" "resource/fonts/scp_console_heading.ttf"
		"SCPConsoleSymbols" "resource/fonts/scp_console_symbols.ttf"
	}
}
