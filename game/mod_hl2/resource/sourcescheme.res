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
		// base colors
		"White"				"242 242 242 255"
		"OffWhite"			"214 214 214 255"
		"DullWhite"			"140 140 140 255"

		"TransparentBlack"	"10 10 10 248"
		"Black"				"0 0 0 255"

		"Blank"				"0 0 0 0"

		// base colors
		"BaseText"			"214 214 214 255"	// used in text windows, lists
		"BrightBaseText"	"242 242 242 255"	// brightest text
		"SelectedText"		"255 255 255 255"	// selected text
		"DimBaseText"		"140 140 140 255"	// dim base text
		"LabelDimText"		"140 140 140 255"	// used for info text
		"ControlText"		"214 214 214 255"	// used in all text controls
		"BrightControlText"	"242 242 242 255"	// use for selected controls
		"DisabledText1"		"117 128 111 255"	// disabled text
		"DisabledText2"		"40 46 34 255"		// overlay color for disabled text (to give that inset look)
		"DimListText"		"180 180 180 255"	// offline friends, unsubscribed games, etc.

		// background colors
		"ControlBG"			"12 12 12 255"		// background color of controls
		"ControlDarkBG"		"17 17 17 255"		// darker background color; used for background of scrollbars
		"WindowBG"			"8 8 8 255"		// background color of text edit panes (chat, text entries, etc.)
		"SelectionBG"		"50 50 50 255"	// background color of any selected text or menu item
		"SelectionBG2"		"35 35 35 255"		// selection background in window w/o focus
		"ListBG"			"8 8 8 255"		// background of server browser, buddy list, etc.

		// titlebar colors
		"TitleText"			"235 235 235 255"
		"TitleDimText"		"110 110 110 255"
		"TitleBG"			"16 16 16 255"
		"TitleDimBG"		"12 12 12 255"

		// slider tick colors
		"SliderTickColor"	"180 180 180 255"
		"SliderTrackColor"	"31 31 31 255"

		// border colors
		"BorderBright"		"57 57 57 255"	// the lit side of a control
		"BorderDark"		"34 34 34 255"		// the dark/unlit side of a control
		"BorderSelection"	"112 112 112 255"			// the additional border color for displaying the default/selected button

		"TestColor"			"245 245 245 255"
		"Orange" "224 224 224 255"
		"SCP.Console.Edge" "57 57 57 255"
		"SCP.Console.EdgeSoft" "34 34 34 255"
		"SCP.Console.EdgeActive" "112 112 112 255"
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
//		Button.ArmedTextColor			"BrightBaseText"
//		Button.ArmedBgColor				"SelectionBG"
//		Button.DepressedTextColor		"DimBaseText"
//		Button.DepressedBgColor			"ControlDarkBG"
		Button.FocusBorderColor			"112 112 112 255"

		CheckButton.TextColor			"BaseText"
		CheckButton.SelectedTextColor	"BrightControlText"
		CheckButton.BgColor				"ListBG"
		CheckButton.Border1  			"Border.Dark" 		// the left checkbutton border
		CheckButton.Border2  			"Border.Bright"		// the right checkbutton border
		CheckButton.Check				"BrightControlText"	// color of the check itself

		ComboBoxButton.ArrowColor		"DimBaseText"
		ComboBoxButton.ArmedArrowColor	"BrightBaseText"
		ComboBoxButton.BgColor			"ListBG"
		ComboBoxButton.DisabledBgColor	"ControlBG"

		Frame.TitleTextInsetX			"14"
		Frame.ClientInsetX				"10"
		Frame.ClientInsetY				"8"
		Frame.BgColor					"12 12 12 250"
		Frame.OutOfFocusBgColor			"10 10 10 245"
		Frame.FocusTransitionEffectTime	"0.12"	// time it takes for a window to fade in/out on focus/out of focus
		Frame.TransitionEffectTime		"0.18"	// time it takes for a window to fade in/out on open/close
		Frame.AutoSnapRange				"0"
		FrameGrip.Color1				"90 90 90 150"
		FrameGrip.Color2				"14 14 14 255"
		FrameTitleButton.FgColor		"200 200 200 255"
		FrameTitleButton.BgColor		"0 0 0 0"
		FrameTitleButton.DisabledFgColor	"90 90 90 255"
		FrameTitleButton.DisabledBgColor	"0 0 0 0"
		FrameSystemButton.FgColor		"TitleBG"
		FrameSystemButton.BgColor		"TitleBG"
		FrameSystemButton.Icon			"resource/icon_steam"
		FrameSystemButton.DisabledIcon	"resource/icon_steam_disabled"
		FrameTitleBar.TextColor			"222 222 222 255"
		FrameTitleBar.BgColor			"16 16 16 255"
		FrameTitleBar.DisabledTextColor	"110 110 110 255"
		FrameTitleBar.DisabledBgColor	"12 12 12 255"

		GraphPanel.FgColor				"White"
		GraphPanel.BgColor				"TransparentBlack"

		Label.TextDullColor				"DimBaseText"
		Label.TextColor					"ControlText"
		Label.TextBrightColor			"BrightBaseText"
		Label.SelectedTextColor			"BrightControlText"
		Label.BgColor					"ControlBG"
		Label.DisabledFgColor1			"DisabledText1"
		Label.DisabledFgColor2			"DisabledText2"

		ListPanel.TextColor					"BaseText"
		ListPanel.BgColor					"ListBG"
		ListPanel.SelectedTextColor			"BrightBaseText"
		ListPanel.SelectedBgColor			"SelectionBG"
		ListPanel.SelectedOutOfFocusBgColor	"SelectionBG2"
		ListPanel.EmptyListInfoTextColor	"DimBaseText"

		Menu.TextInset					"8"
		Menu.FgColor			"DimBaseText"
		Menu.BgColor			"18 18 18 255"
		Menu.ArmedFgColor		"BrightBaseText"
		Menu.ArmedBgColor		"43 43 43 255"
		Menu.DividerColor		"BorderDark"

		Panel.FgColor					"BorderSelection"
		Panel.BgColor					"Blank"

		ProgressBar.FgColor				"Label.FgColor"
		ProgressBar.BgColor				"Label.BgColor"

		PropertySheet.TextColor			"ControlText"
		PropertySheet.SelectedTextColor		"BrightControlText"
		PropertySheet.TransitionEffectTime	"0"	// time to change from one tab to another

		RadioButton.TextColor			"ToggleButton.TextColor"
		RadioButton.SelectedTextColor		"ToggleButton.SelectedTextColor"

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

		ScrollBarSlider.BgColor				"13 13 13 255"		// this isn't really used
		ScrollBarSlider.FgColor				"67 67 67 255"		// handle with which the slider is grabbed

//		SectionedListPanel.HeaderTextColor	"White"
//		SectionedListPanel.HeaderBgColor	"Blank"
//		SectionedListPanel.DividerColor		"Black"
//		SectionedListPanel.TextColor		"DullWhite"
//		SectionedListPanel.BrightTextColor	"White"
//		SectionedListPanel.BgColor			"TransparentBlack"
//		SectionedListPanel.SelectedTextColor			"Black"
//		SectionedListPanel.SelectedBgColor				"255 155 0 255"
//		SectionedListPanel.OutOfFocusSelectedTextColor	"Black"
//		SectionedListPanel.OutOfFocusSelectedBgColor	"255 155 0 128"

		Slider.NobColor				"ControlDarkBG"
		Slider.TextColor			"ControlBG"
		Slider.TrackColor			"ControlDarkBG"
//		Slider.DisabledTextColor1	"117 117 117 255"
//		Slider.DisabledTextColor2	"30 30 30 255"

		TextEntry.TextColor			"238 238 238 255"
		TextEntry.BgColor			"19 19 19 255"
		TextEntry.CursorColor		"245 245 245 255"
		TextEntry.DisabledTextColor	"110 110 110 255"
		TextEntry.DisabledBgColor	"14 14 14 255"
		TextEntry.SelectedTextColor	"255 255 255 255"
		TextEntry.SelectedBgColor	"65 65 65 255"
		TextEntry.OutOfFocusSelectedBgColor	"38 38 38 255"
		TextEntry.FocusEdgeColor	"112 112 112 255"

		ToggleButton.SelectedTextColor	"Label.SelectedTextColor"

//		Tooltip.TextColor			"0 0 0 196"
//		Tooltip.BgColor				"255 155 0 255"

		TreeView.BgColor			"WindowBG"

		WizardSubPanel.BgColor		"ControlBG"
		"FrameTitleBar.Font" "SCPConsoleHeading"
		"Button.ArmedTextColor" "255 255 255 255"
		"Button.ArmedBgColor" "43 43 43 255"
		"Button.DepressedTextColor" "255 255 255 255"
		"Button.DepressedBgColor" "17 17 17 255"
		"Menu.TextColor" "202 202 202 255"
		"Menu.ArmedTextColor" "255 255 255 255"
		"Console.TextColor" "214 214 214 255"
		"Console.DevTextColor" "156 156 156 255"
		"FrameTitleBar.SmallFont" "SCPConsoleHeadingSmall"
	}

	//
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
				"name"		"Trebuchet MS"
				"tall"		"10"
				"weight"	"700"
				"outline"	"1"
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
				"name"		"Trebuchet MS"
				"tall"		"16"
				"weight"	"700"
				"underline" "1"
			"range" "0x0020 0x04FF" }
		}
		"DefaultSmall"
		{
			"1"
			{
				"name"		"Trebuchet MS"
				"tall"		"13"
				"weight"	"700"
			"range" "0x0020 0x04FF" }
		}
		"DefaultSmallDropShadow"
		{
			"1"
			{
				"name"		"Trebuchet MS"
				"tall"		"13"
				"weight"	"700"
				"dropshadow" "1"
			"range" "0x0020 0x04FF" }
		}
		"DefaultVerySmall"
		{
			"1"
			{
				"name"		"Trebuchet MS"
				"tall"		"12"
				"weight"	"700"
			"range" "0x0020 0x04FF" }
		}

		"DefaultLarge"
		{
			"1"
			{
				"name"		"Trebuchet MS"
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
		"MenuLarge"
		{
			"1"
			{
				"name"		"Trebuchet MS"
				"tall"		"16"
				"weight"	"700"
				"antialias" "1"
			"range" "0x0020 0x04FF" }
		}

		"ConsoleText"
		{
			"1"
			{
				"name"		"Trebuchet MS"
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
				"name"		"Trebuchet MS"
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
				"name"		"Trebuchet MS"
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
		BaseBorder		InsetBorder
		ButtonBorder	RaisedBorder
		ComboBoxBorder	InsetBorder
		MenuBorder		RaisedBorder
		BrowserBorder	InsetBorder
		PropertySheetBorder	RaisedBorder
		TabBorder		RaisedBorder
		CheckBorder InsetBorder
		FrameBorder	RaisedBorder
		TitleButtonBorder
		{
			"backgroundtype" "0"
			"inset" "0 0 0 0"
		}
		TitleButtonDepressedBorder
		{
			"backgroundtype" "0"
			"inset" "0 0 0 0"
		}
		TitleButtonDisabledBorder
		{
			"backgroundtype" "0"
			"inset" "0 0 0 0"
		}


		ScrollBarButtonBorder
		{
			"inset" "1 0 0 0"
			Left
			{
				"1"
				{
					"color" "BorderBright"
					"offset" "0 1"
				}
			}

			Right
			{
				"1"
				{
					"color" "BorderDark"
					"offset" "1 0"
				}
			}

			Top
			{
				"1"
				{
					"color" "BorderBright"
					"offset" "0 0"
				}
			}

			Bottom
			{
				"1"
				{
					"color" "BorderDark"
					"offset" "0 0"
				}
			}
		}

		InsetBorder
		{
			"inset" "0 0 1 1"
			Right
			{
				"1"
				{
					"color" "Border.Bright"
					"offset" "0 1"
				}
			}

			Left
			{
				"1"
				{
					"color" "Border.Dark"
					"offset" "1 0"
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

			Top
			{
				"1"
				{
					"color" "Border.Dark"
					"offset" "0 0"
				}
			}
		}

		DepressedBorder
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

			Bottom
			{
				"1"
				{
					"color" "Border.Dark"
					"offset" "0 0"
				}
			}
		}
		RaisedBorder
		{
			"inset" "0 0 1 1"
			Right
			{
				"1"
				{
					"color" "Border.Dark"
					"offset" "0 1"
				}
			}

			Left
			{
				"1"
				{
					"color" "Border.Bright"
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

		ScrollBarButtonBorder
		{
			"inset" "2 2 0 0"
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

		ScrollBarButtonDepressedBorder
		{
			"inset" "2 2 0 0"
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
					"color" "Border.Bright"
					"offset" "0 0"
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
					"color" "Border.Bright"
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

		// this is the border used for default buttons (the button that gets pressed when you hit enter)
		ButtonKeyFocusBorder
		{
			"inset" "0 0 1 1"
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
					"offset" "1 1"
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
					"offset" "1 1"
				}
			}
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
					"offset" "1 0"
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
		}

		ButtonDepressedBorder
		{
			"inset" "2 1 1 1"
			Right
			{
				"1"
				{
					"color" "Border.Bright"
					"offset" "1 1"
				}
			}

			Left
			{
				"1"
				{
					"color" "Border.Dark"
					"offset" "1 0"
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

			Top
			{
				"1"
				{
					"color" "Border.Dark"
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
