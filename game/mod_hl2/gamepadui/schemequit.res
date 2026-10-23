"SchemeMainmenu"
{
	"BaseSettings"
	{
        "Logo.Image"          "gamepadui/scp_final_death"
        "Logo.Image.Width"    "300"
        "Logo.Image.Gap"      "24"
        "Logo.Image.UVWidth"  "0.52099609375"
        "Logo.Image.UVHeight" "0.935546875"
		"Logo.OffsetX"							"85"
		"Logo.OffsetY.MainMenu"					"290"
		"Logo.OffsetY.InGame"					"340"
	
		"Buttons.OffsetX"						"85"
		"Buttons.OffsetY.MainMenu"				"165"
		"Buttons.OffsetY.InGame"				"140"
		"Button.Text.CenterX" "1"
        "Buttons.Space"							"6"
	
		"Button.Width.Out"						"70"
		"Button.Width.Over"					"70"
		"Button.Width.Pressed"					"70"

		"Button.Height.Out"					"20"
		"Button.Height.Over"				"20"
		"Button.Height.Pressed"				"20"

		"FooterButtons.OffsetX"					"64"
		"FooterButtons.OffsetY"					"48"
		"FooterButtons.Spacing"					"8"

		"Button.Text.OffsetX.Out"				"0"
		"Button.Text.OffsetX.Over"				"0"
		"Button.Text.OffsetX.Pressed"			"0"
		"Button.Text.OffsetY.Out"				"0"
		"Button.Text.OffsetY.Over"				"0"
		"Button.Text.OffsetY.Pressed"			"0"

		"Button.Text.LeftBorder.Out"			"0"
		"Button.Text.LeftBorder.Over"			"0"
		"Button.Text.LeftBorder.Pressed"		"0"

		"Button.Description.OffsetX"			"1"
		"Button.Description.OffsetY"			"-3"

		"Button.Description.Hide.Out"			"1"
		"Button.Description.Hide.Over"			"1"
		"Button.Description.Hide.Pressed"		"1"

		"Button.Text.OffsetX.Animation.Duration"		"0.15"
		"Button.Width.Animation.Duration"				"0.15"
		"Button.Height.Animation.Duration"				"0.25"
		"Button.Background.Animation.Duration"			"0.2"
		"Button.Text.Animation.Duration"				"0.2"
		"Button.Description.Animation.Duration"			"0.5"
	}

	"Colors"
	{
		"Button.Text.Out"                    "220 220 220 255"
        "Button.Text.Over"                   "255 255 255 255"
        "Button.Text.Pressed"                "0 0 0 255"

        "Button.Background.Out"              "0 0 0 0"
        "Button.Background.Over"             "30 30 30 210"
        "Button.Background.Pressed"          "255 255 255 255"

        "Button.Background.LeftBorder.Out"   "0 0 0 0"
        "Button.Background.LeftBorder.Over"  "255 255 255 255"
        "Button.Background.LeftBorder.Pressed" "255 255 255 255"

         "Logo"                              "255 255 255 255"
	}
	
	"Fonts"
	{
		"Logo.Font"
		{
			"settings"
			{
				"name"			"HalfLife2"
				"tall"			"34"
				"weight"		"400"
				"antialias"		"1"
                "range" "0x0000 0xFFFF"
			}
		}
		
		"Button.Text.Font"
        {
            "1"
            {
                "name" "Trebuchet MS"
                "tall" "16"
                "weight" "700"
                "antialias" "1"
                "range" "0x0020 0x04FF"
            }
        }

		"Button.Text.Font.Over"
        {
            "1"
            {
                "name" "Trebuchet MS"
                "tall" "16"
                "weight" "700"
                "antialias" "1"
                "range" "0x0020 0x04FF"
            }
        }
	}

	"CustomFontFiles"
	{
        "file" "gamepadui/fonts/NotoSans-Regular.ttf"
		"file"		"resource/HALFLIFE2.ttf"
		"file"		"gamepadui/fonts/din1451alt.ttf"
	}
}
