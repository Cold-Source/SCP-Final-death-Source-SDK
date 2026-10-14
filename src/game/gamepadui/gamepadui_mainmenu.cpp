#include "gamepadui_interface.h"
#include "gamepadui_basepanel.h"
#include "gamepadui_mainmenu.h"
#include "gamepadui_language.h"

#include "vgui/ISurface.h"
#include "vgui/ILocalize.h"
#include "vgui/IVGui.h"

#include "KeyValues.h"
#include "filesystem.h"

#include "tier0/memdbgon.h"

#define GAMEPADUI_MAINMENU_SCHEME GAMEPADUI_RESOURCE_FOLDER "schememainmenu.res"
#define GAMEPADUI_MAINMENU_FILE GAMEPADUI_RESOURCE_FOLDER "mainmenu.res"

namespace
{
// Keep this menu addition entirely in C++; no new resource or project files are needed.
const char* const RETURN_MENU_COMMAND = "return_mainmenu";
const char* const RETURN_MENU_BUTTON = "ReturnToMainMenu";
const char* const RETURN_MENU_SCHEME = GAMEPADUI_RESOURCE_FOLDER "schemelanguage.res";

bool IsReturnMenuRussian()
{
    return !V_stricmp( GamepadUIGetLanguageCode(), "RU" );
}

const wchar_t* GetReturnMenuText()
{
    // Unicode escapes also work when the legacy compiler uses a non-UTF-8 code page.
    return IsReturnMenuRussian()
        ? L"\u0412\u0435\u0440\u043D\u0443\u0442\u044C\u0441\u044F \u0432 \u0433\u043B\u0430\u0432\u043D\u043E\u0435 \u043C\u0435\u043D\u044E"
        : L"Return to main menu";
}

class GamepadUIReturnMenuChoice : public GamepadUIButton
{
    DECLARE_CLASS_SIMPLE( GamepadUIReturnMenuChoice, GamepadUIButton );
public:
    GamepadUIReturnMenuChoice( vgui::Panel* parent, const char* command, const wchar_t* text )
        : BaseClass( parent, parent, RETURN_MENU_SCHEME, command, text, L"" ) {}

    void Paint() OVERRIDE
    {
        const bool selected = GetCurrentButtonState() != ButtonStates::Out;
        const Color background = selected ? Color( 255, 255, 255, 255 ) : Color( 28, 28, 28, 255 );
        const Color foreground = selected ? Color( 0, 0, 0, 255 ) : Color( 255, 255, 255, 255 );
        vgui::surface()->DrawSetTexture( -1 );
        vgui::surface()->DrawSetColor( background );
        vgui::surface()->DrawFilledRect( 0, 0, GetWide(), GetTall() );
        vgui::surface()->DrawSetColor( Color( 255, 255, 255, selected ? 255 : 100 ) );
        vgui::surface()->DrawOutlinedRect( 0, 0, GetWide(), GetTall() );
        const vgui::HFont font = selected ? m_hTextFontOver : m_hTextFont;
        int width = 0, height = 0;
        vgui::surface()->GetTextSize( font, GetButtonText().String(), width, height );
        vgui::surface()->DrawSetTextFont( font );
        vgui::surface()->DrawSetTextColor( foreground );
        vgui::surface()->DrawSetTextPos( ( GetWide() - width ) / 2, ( GetTall() - height ) / 2 );
        vgui::surface()->DrawPrintText( GetButtonText().String(), GetButtonText().Length() );
    }

    void OnKeyCodePressed( vgui::KeyCode code ) OVERRIDE
    {
        const ButtonCode_t button = GetBaseButtonCode( code );
        if ( button == KEY_ESCAPE || button == KEY_XBUTTON_B
#ifdef HL2_RETAIL
             || button == STEAMCONTROLLER_B
#endif
           )
            GetParent()->OnCommand( "return_no" );
        else
            BaseClass::OnKeyCodePressed( code );
    }
};

class GamepadUIReturnMenuPanel;
vgui::DHANDLE<GamepadUIReturnMenuPanel> s_ReturnMenuPanel;

class GamepadUIReturnMenuPanel : public GamepadUIFrame
{
    DECLARE_CLASS_SIMPLE( GamepadUIReturnMenuPanel, GamepadUIFrame );
public:
    explicit GamepadUIReturnMenuPanel( vgui::Panel* parent )
        : BaseClass( parent, "ReturnToMainMenuConfirmation" )
    {
        SetScheme( vgui::scheme()->LoadSchemeFromFile( RETURN_MENU_SCHEME, "SchemeReturnMenu" ) );
        m_Title.SetText( IsReturnMenuRussian()
            ? L"\u0412\u044B \u0443\u0432\u0435\u0440\u0435\u043D\u044B?" : L"Are you sure?" );
        m_pYes = new GamepadUIReturnMenuChoice( this, "return_yes",
            IsReturnMenuRussian() ? L"\u0414\u0430" : L"Yes" );
        m_pNo = new GamepadUIReturnMenuChoice( this, "return_no",
            IsReturnMenuRussian() ? L"\u041D\u0435\u0442" : L"No" );
        m_pYes->SetNavRight( m_pNo );
        m_pYes->SetNavDown( m_pNo );
        m_pNo->SetNavLeft( m_pYes );
        m_pNo->SetNavUp( m_pYes );
        DoModal();
        LayoutCard();
        m_pNo->NavigateTo();
    }

    void OnThink() OVERRIDE
    {
        BaseClass::OnThink();
        if ( !GamepadUI::GetInstance().IsInLevel() )
        {
            OnCommand( "return_no" );
            return;
        }
        LayoutCard();
    }

    void Paint() OVERRIDE
    {
        vgui::surface()->DrawSetTexture( -1 );
        vgui::surface()->DrawSetColor( Color( 0, 0, 0, 185 ) );
        vgui::surface()->DrawFilledRect( 0, 0, GetWide(), GetTall() );
        vgui::surface()->DrawSetColor( Color( 18, 18, 18, 255 ) );
        vgui::surface()->DrawFilledRect( m_nX, m_nY, m_nX + m_nWidth, m_nY + m_nHeight );
        vgui::surface()->DrawSetColor( Color( 255, 255, 255, 160 ) );
        vgui::surface()->DrawOutlinedRect( m_nX, m_nY, m_nX + m_nWidth, m_nY + m_nHeight );
        int width = 0, height = 0;
        vgui::surface()->GetTextSize( m_hGenericFont, m_Title.String(), width, height );
        vgui::surface()->DrawSetTextFont( m_hGenericFont );
        vgui::surface()->DrawSetTextColor( Color( 255, 255, 255, 255 ) );
        vgui::surface()->DrawSetTextPos( m_nX + ( m_nWidth - width ) / 2,
            m_nY + ( m_nHeight / 2 - height ) / 2 );
        vgui::surface()->DrawPrintText( m_Title.String(), m_Title.Length() );
    }

    void OnCommand( const char* command ) OVERRIDE
    {
        if ( !command || m_bClosing )
            return;
        const bool confirm = !V_strcmp( command, "return_yes" );
        if ( confirm || !V_strcmp( command, "return_no" ) )
        {
            m_bClosing = true;
            s_ReturnMenuPanel = nullptr;
            // Release the modal input capture before the level and menu change.
            CloseModal();
            if ( confirm && GamepadUI::GetInstance().IsInLevel() )
                GamepadUI::GetInstance().GetEngineClient()->ClientCmd_Unrestricted( "disconnect" );
        }
        else
            BaseClass::OnCommand( command );
    }

    void OnKeyCodePressed( vgui::KeyCode code ) OVERRIDE
    {
        const ButtonCode_t button = GetBaseButtonCode( code );
        if ( button == KEY_ESCAPE || button == KEY_XBUTTON_B
#ifdef HL2_RETAIL
             || button == STEAMCONTROLLER_B
#endif
           )
            OnCommand( "return_no" );
        else
            BaseClass::OnKeyCodePressed( code );
    }

private:
    void LayoutCard()
    {
        int parentWidth = 0, parentHeight = 0;
        GetParent()->GetSize( parentWidth, parentHeight );
        SetBounds( 0, 0, parentWidth, parentHeight );
        const int padding = Max( 8, vgui::scheme()->GetProportionalScaledValueEx( GetScheme(), 18 ) );
        m_nWidth = Max( 1, Min( parentWidth - 2 * padding,
            vgui::scheme()->GetProportionalScaledValueEx( GetScheme(), 360 ) ) );
        m_nHeight = Max( 1, Min( parentHeight - 2 * padding,
            vgui::scheme()->GetProportionalScaledValueEx( GetScheme(), 150 ) ) );
        m_nX = ( parentWidth - m_nWidth ) / 2;
        m_nY = ( parentHeight - m_nHeight ) / 2;
        const int gap = padding;
        const int width = Max( 1, ( m_nWidth - 2 * padding - gap ) / 2 );
        const int height = Max( 1, Min( m_nHeight / 3,
            vgui::scheme()->GetProportionalScaledValueEx( GetScheme(), 40 ) ) );
        GamepadUIButton* buttons[] = { m_pYes, m_pNo };
        for ( int i = 0; i < ARRAYSIZE( buttons ); ++i )
        {
            GamepadUIButton* button = buttons[i];
            button->m_flWidth = float( width );
            button->m_flHeight = float( height );
            for ( int state = 0; state < ButtonStates::Count; ++state )
            {
                button->m_flWidthAnimationValue[state] = float( width );
                button->m_flHeightAnimationValue[state] = float( height );
            }
            button->SetBounds( m_nX + padding + i * ( width + gap ),
                m_nY + m_nHeight - padding - height, width, height );
        }
    }

    GamepadUIButton* m_pYes = nullptr;
    GamepadUIButton* m_pNo = nullptr;
    GamepadUIString m_Title;
    bool m_bClosing = false;
    int m_nX = 0, m_nY = 0, m_nWidth = 0, m_nHeight = 0;
};

void OpenReturnMenuDialog( vgui::Panel* parent )
{
    if ( !GamepadUI::GetInstance().IsInLevel() )
        return;
    if ( s_ReturnMenuPanel.Get() )
        s_ReturnMenuPanel->Activate();
    else
        s_ReturnMenuPanel = new GamepadUIReturnMenuPanel( parent );
}
}

// Utility buttons retain their dimensions when focused. The language tile is square
// even when VGUI applies its widescreen correction to button widths.
class GamepadUIMenuUtilityButton : public GamepadUIButton
{
public:
    GamepadUIMenuUtilityButton( vgui::Panel* pParent, const char* pScheme,
                               const char* pCommand, const char* pText, bool bSquare )
        : GamepadUIButton( pParent, pParent, pScheme, pCommand, pText, "" ), m_bSquare( bSquare ) {}

    void ApplySchemeSettings( vgui::IScheme* pScheme ) OVERRIDE
    {
        GamepadUIButton::ApplySchemeSettings( pScheme );
        if ( m_bSquare )
        {
            m_flWidth = m_flHeight;
            for ( int i = 0; i < ButtonStates::Count; ++i )
                m_flWidthAnimationValue[i] = m_flHeightAnimationValue[i];
            SetSize( int( m_flWidth ), int( m_flHeight ) );
        }
    }
private:
    bool m_bSquare;
};

GamepadUIMainMenu::GamepadUIMainMenu( vgui::Panel* pParent )
    : BaseClass( pParent, "MainMenu" )
{
    vgui::HScheme hScheme = vgui::scheme()->LoadSchemeFromFile( GAMEPADUI_MAINMENU_SCHEME, "SchemeMainMenu" );
    SetScheme( hScheme );

    KeyValues* pModData = new KeyValues( "ModData" );
    if ( pModData )
    {
        if ( pModData->LoadFromFile( g_pFullFileSystem, "gameinfo.txt" ) )
        {
            m_LogoText[ 0 ] = pModData->GetString( "gamepadui_title", pModData->GetString( "title" ) );
            m_LogoText[ 1 ] = pModData->GetString( "gamepadui_title2", pModData->GetString( "title2" ) );
        }
        pModData->deleteThis();
    }

    LoadMenuButtons();

    RefreshMenuLanguage();

    SetFooterButtons( FooterButtons::Select, FooterButtons::Select );
}

void GamepadUIMainMenu::UpdateGradients()
{
    const float flTime = GamepadUI::GetInstance().GetTime();
    GamepadUI::GetInstance().GetGradientHelper()->ResetTargets( flTime );
    GamepadUI::GetInstance().GetGradientHelper()->SetTargetGradient( GradientSide::Left, { 1.0f, 0.666f }, flTime );
}

void GamepadUIMainMenu::LoadMenuButtons()
{
    KeyValues* pDataFile = new KeyValues( "MainMenuScript" );
    if ( pDataFile )
    {
        if ( pDataFile->LoadFromFile( g_pFullFileSystem, GAMEPADUI_MAINMENU_FILE ) )
        {
            for ( KeyValues* pData = pDataFile->GetFirstSubKey(); pData != NULL; pData = pData->GetNextKey() )
            {
                const bool bQuit = !V_stricmp( pData->GetName(), "Quit" );
                const bool bMode = !V_stricmp( pData->GetName(), "Mode" );
                const bool bLanguage = !V_stricmp( pData->GetName(), "Language" );
                GamepadUIButton* pButton = ( bQuit || bMode || bLanguage )
                    ? new GamepadUIMenuUtilityButton( this,
                        pData->GetString( "scheme", GAMEPADUI_MAINMENU_SCHEME ),
                        pData->GetString( "command", "" ), pData->GetString( "text", "" ), bLanguage )
                    : new GamepadUIButton( this, this,
                    pData->GetString( "scheme", GAMEPADUI_MAINMENU_SCHEME ),
                    pData->GetString( "command" ),
                    pData->GetString( "text", "Sample Text" ),
                    pData->GetString( "description", "" ) );
                pButton->SetPriority( V_atoi( pData->GetString( "priority", "0" ) ) );
                pButton->SetName( pData->GetName() );
                pButton->m_bMainMenuAlignRight = pData->GetBool( "align_right", false );
                if ( bQuit ) m_pQuitButton = pButton;
                if ( bMode ) m_pModeButton = pButton;
                if ( bLanguage ) m_pLanguageButton = pButton;
                pButton->SetVisible( true );

                const char* pFamily = pData->GetString( "family", "all" );
                if ( !V_strcmp( pFamily, "ingame" ) || !V_strcmp( pFamily, "all" ) )
                    m_Buttons[ GamepadUIMenuStates::InGame ].AddToTail( pButton );
                if ( !V_strcmp( pFamily, "mainmenu" ) || !V_strcmp( pFamily, "all" ) )
                    m_Buttons[ GamepadUIMenuStates::MainMenu ].AddToTail( pButton );
            }
        }

        pDataFile->deleteThis();
    }

    GamepadUIButton* returnButton = new GamepadUIButton( this, this,
        GAMEPADUI_MAINMENU_SCHEME, RETURN_MENU_COMMAND, GetReturnMenuText(), L"" );
    returnButton->SetName( RETURN_MENU_BUTTON );
    returnButton->SetPriority( 0 );
    m_Buttons[GamepadUIMenuStates::InGame].AddToTail( returnButton );

    UpdateButtonVisibility();
}

void GamepadUIMainMenu::RefreshMenuLanguage()
{
    KeyValues* menu = new KeyValues( "MainMenu" );
    if ( menu->LoadFromFile( g_pFullFileSystem, GAMEPADUI_MAINMENU_FILE ) )
    {
        for ( GamepadUIButton* button : m_Buttons[GamepadUIMenuStates::InGame] )
        {
            if ( !V_strcmp( button->GetName(), RETURN_MENU_BUTTON ) )
            {
                button->GetButtonText().SetText( GetReturnMenuText() );
                continue;
            }
            KeyValues* data = menu->FindKey( button->GetName() );
            if ( data )
            {
                button->GetButtonText().SetText( data->GetString( "text", "" ) );
                button->GetButtonDescription().SetText( data->GetString( "description", "" ) );
            }
        }
        for ( GamepadUIButton* button : m_Buttons[GamepadUIMenuStates::MainMenu] )
        {
            KeyValues* data = menu->FindKey( button->GetName() );
            if ( data ) button->GetButtonText().SetText( data->GetString( "text", "" ) );
        }
    }
    menu->deleteThis();
    if ( m_pLanguageButton )
        m_pLanguageButton->GetButtonText().SetText( GamepadUIGetLanguageCode() );
    SetFooterButtons( FooterButtons::Select, FooterButtons::Select );
}

void GamepadUIMainMenu::ApplySchemeSettings( vgui::IScheme* pScheme )
{
    BaseClass::ApplySchemeSettings( pScheme );

    int nParentW, nParentH;
	GetParent()->GetSize( nParentW, nParentH );
    SetBounds( 0, 0, nParentW, nParentH );

    const char *pImage = pScheme->GetResourceString( "Logo.Image" );
    m_LogoImage.Cleanup();
    if ( pImage && *pImage )
        m_LogoImage.SetImage( pImage );
    m_hLogoFont = pScheme->GetFont( "Logo.Font", true );
}

void GamepadUIMainMenu::LayoutMainMenu()
{
    // Keep the original menu and logo position after detaching Quit from the stack.
    int nY = GetCurrentButtonOffset() + ( m_pQuitButton ? m_flQuitStackReserve : 0 );
    int nTopLeftY = -1;
    CUtlVector<GamepadUIButton*>& currentButtons = GetCurrentButtons();
    for ( GamepadUIButton *pButton : currentButtons )
    {
        if ( pButton->m_bMainMenuAlignRight || pButton == m_pQuitButton ||
             pButton == m_pModeButton || pButton == m_pLanguageButton )
            continue;

        nY += pButton->GetTall();
        nTopLeftY = GetTall() - nY;
        pButton->SetPos( int( m_flButtonsOffsetX ), nTopLeftY );
        nY += m_flButtonSpacing;
    }

    // Right-hand buttons start on the same row as the top left-hand button.
    int nRightY = nTopLeftY;
    for ( int i = currentButtons.Count() - 1; i >= 0; --i )
    {
        GamepadUIButton *pButton = currentButtons[i];
        if ( !pButton->m_bMainMenuAlignRight || pButton == m_pQuitButton ||
             pButton == m_pModeButton || pButton == m_pLanguageButton )
            continue;

        if ( nRightY == -1 )
            nRightY = GetTall() - int( GetCurrentButtonOffset() ) - pButton->GetTall();
        const int nX = Max( 0, GetWide() - pButton->GetWide() - int( m_flButtonsOffsetX ) );
        pButton->SetPos( nX, nRightY );
        nRightY += pButton->GetTall() + int( m_flButtonSpacing );
    }

    if ( m_pQuitButton )
        m_pQuitButton->SetPos( Max( 18, int( m_flButtonsOffsetX - m_flQuitShiftLeft ) ),
                              Max( 0, GetTall() - m_pQuitButton->GetTall() - int( m_flQuitBottomMargin ) ) );

    const bool bMainMenu = GetCurrentMenuState() == GamepadUIMenuStates::MainMenu;
    const int nNewsLeft = bMainMenu ? GetNewsLeft() : GetWide();
    if ( m_pModeButton )
        m_pModeButton->SetPos( Max( 0, nNewsLeft - m_pModeButton->GetWide() ),
                              Max( 0, GetTall() - m_pModeButton->GetTall() ) );
    if ( m_pLanguageButton )
        m_pLanguageButton->SetPos( Max( 0, nNewsLeft - ( bMainMenu && m_pModeButton ? m_pModeButton->GetWide() : 0 ) - m_pLanguageButton->GetWide() ),
                                  Max( 0, GetTall() - m_pLanguageButton->GetTall() ) );
}

void GamepadUIMainMenu::PaintLogo()
{
    if ( m_LogoImage.IsValid() )
    {
        int nTextureW = 0, nTextureH = 0;
        vgui::surface()->DrawGetTextureSize( m_LogoImage, nTextureW, nTextureH );
        if ( nTextureW <= 0 || nTextureH <= 0 )
            return;

        const float flU = Clamp( m_flLogoImageUVWidth, 0.001f, 1.0f );
        const float flV = Clamp( m_flLogoImageUVHeight, 0.001f, 1.0f );
        const float flAspect = ( nTextureW * flU ) / ( nTextureH * flV );
        const int nMargin = vgui::scheme()->GetProportionalScaledValueEx( GetScheme(), 16 );
        const int nX = Max( nMargin, int( m_flLogoOffsetX ) );
        int nButtonsTop = GetTall() - int( GetCurrentButtonOffset() );
        for ( GamepadUIButton *pButton : GetCurrentButtons() )
        {
            if ( pButton->m_bMainMenuAlignRight || pButton == m_pQuitButton ||
                 pButton == m_pModeButton || pButton == m_pLanguageButton )
                continue;
            int nButtonX, nButtonY;
            pButton->GetPos( nButtonX, nButtonY );
            nButtonsTop = Min( nButtonsTop, nButtonY );
        }

        const int nBottom = nButtonsTop - Max( 0, int( m_flLogoImageGap ) );
        const int nRight = GetCurrentMenuState() == GamepadUIMenuStates::MainMenu ? GetNewsLeft() : GetWide();
        const int nAvailableW = Max( 0, nRight - nX - nMargin );
        const int nAvailableH = Max( 0, nBottom - nMargin );
        const int nWidth = int( Min( Max( 0.0f, m_flLogoImageWidth ),
                                    Min( float( nAvailableW ), nAvailableH * flAspect ) ) );
        if ( nWidth <= 0 )
            return;
        const int nHeight = Max( 1, int( nWidth / flAspect + 0.5f ) );

        vgui::surface()->DrawSetColor( m_colLogoColor );
        vgui::surface()->DrawSetTexture( m_LogoImage );
        vgui::surface()->DrawTexturedSubRect( nX, nBottom - nHeight, nX + nWidth, nBottom,
                                             0.0f, 0.0f, flU, flV );
        vgui::surface()->DrawSetTexture( -1 );
        return;
    }

    vgui::surface()->DrawSetTextColor( m_colLogoColor );
    vgui::surface()->DrawSetTextFont( m_hLogoFont );

    int nMaxLogosW = 0, nTotalLogosH = 0;
    int nLogoW[ 2 ], nLogoH[ 2 ];
    for ( int i = 0; i < 2; i++ )
    {
        nLogoW[ i ] = 0;
        nLogoH[ i ] = 0;
        if ( !m_LogoText[ i ].IsEmpty() )
            vgui::surface()->GetTextSize( m_hLogoFont, m_LogoText[ i ].String(), nLogoW[ i ], nLogoH[ i ] );
        nMaxLogosW = Max( nLogoW[ i ], nMaxLogosW );
        nTotalLogosH += nLogoH[ i ];
    }

    int nLogoY = GetTall() - ( GetCurrentLogoOffset() + nTotalLogosH );

    for ( int i = 1; i >= 0; i-- )
    {
        vgui::surface()->DrawSetTextPos( m_flLogoOffsetX, nLogoY );
        vgui::surface()->DrawPrintText( m_LogoText[ i ].String(), m_LogoText[ i ].Length() );

        nLogoY -= nLogoH[ i ];
    }
}

void GamepadUIMainMenu::OnThink()
{
    BaseClass::OnThink();

    LayoutMainMenu();
}

void GamepadUIMainMenu::Paint()
{
    BaseClass::Paint();

    PaintLogo();

    if ( GetCurrentMenuState() == GamepadUIMenuStates::MainMenu )
    {
        const int nNewsLeft = GetNewsLeft();
        vgui::surface()->DrawSetTexture( -1 );
        vgui::surface()->DrawSetColor( m_colNewsBackground );
        vgui::surface()->DrawFilledRect( nNewsLeft, 0, GetWide(), GetTall() );
        vgui::surface()->DrawSetColor( m_colNewsBorder );
        vgui::surface()->DrawFilledRect( nNewsLeft, 0, nNewsLeft + 1, GetTall() );
    }
}

void GamepadUIMainMenu::OnCommand( char const* pCommand )
{
    if ( !pCommand || !*pCommand )
        return;
    if ( !V_strcmp( pCommand, RETURN_MENU_COMMAND ) )
    {
        OpenReturnMenuDialog( this );
        return;
    }
    if ( StringHasPrefixCaseSensitive( pCommand, "cmd " ) )
    {
        const char* pszClientCmd = &pCommand[ 4 ];
        if ( *pszClientCmd )
            GamepadUI::GetInstance().GetEngineClient()->ClientCmd_Unrestricted( pszClientCmd );
    }
    else
    {
        BaseClass::OnCommand( pCommand );
    }
}

void GamepadUIMainMenu::OnSetFocus()
{
    BaseClass::OnSetFocus();
    OnMenuStateChanged();
}

void GamepadUIMainMenu::OnMenuStateChanged()
{
    UpdateGradients();
    UpdateButtonVisibility();
}

void GamepadUIMainMenu::UpdateButtonVisibility()
{
    for ( CUtlVector<GamepadUIButton*>& buttons : m_Buttons )
    {
        for ( GamepadUIButton* pButton : buttons )
        {
            pButton->NavigateFrom();
            pButton->SetVisible( false );
        }
    }

    CUtlVector<GamepadUIButton*>& currentButtons = GetCurrentButtons();
    currentButtons.Sort( []( GamepadUIButton* const* a, GamepadUIButton* const* b ) -> int
    {
        const int nA = ( *a )->GetPriority();
        const int nB = ( *b )->GetPriority();
        return nA < nB ? -1 : ( nA > nB ? 1 : 0 );
    });

    // Reset all directions when moving between the main menu and the pause menu.
    for ( GamepadUIButton* pButton : currentButtons )
    {
        pButton->SetNavUp( static_cast<vgui::Panel*>( nullptr ) );
        pButton->SetNavDown( static_cast<vgui::Panel*>( nullptr ) );
        pButton->SetNavLeft( static_cast<vgui::Panel*>( nullptr ) );
        pButton->SetNavRight( static_cast<vgui::Panel*>( nullptr ) );
        pButton->SetVisible( true );
    }
    CUtlVector<GamepadUIButton*> leftColumn, rightColumn;
    for ( int i = currentButtons.Count() - 1; i >= 0; --i )
    {
        GamepadUIButton* button = currentButtons[i];
        if ( button == m_pQuitButton || button == m_pModeButton || button == m_pLanguageButton )
            continue;
        if ( button->m_bMainMenuAlignRight )
            rightColumn.AddToTail( button );
        else
            leftColumn.AddToTail( button );
    }
    GamepadUIButton* lastMain = leftColumn.IsEmpty() ? nullptr : leftColumn.Tail();
    if ( m_pQuitButton && m_pQuitButton->IsVisible() )
        leftColumn.AddToTail( m_pQuitButton );
    CUtlVector<GamepadUIButton*>* columns[] = { &leftColumn, &rightColumn };
    for ( CUtlVector<GamepadUIButton*>* column : columns )
    {
        for ( int i = 1; i < column->Count(); ++i )
        {
            (*column)[i - 1]->SetNavDown( (*column)[i] );
            (*column)[i]->SetNavUp( (*column)[i - 1] );
        }
    }
    const bool languageVisible = m_pLanguageButton && m_pLanguageButton->IsVisible();
    const bool modeVisible = m_pModeButton && m_pModeButton->IsVisible();
    for ( int i = 0; i < leftColumn.Count(); ++i )
    {
        if ( !rightColumn.IsEmpty() )
            leftColumn[i]->SetNavRight( rightColumn[Min( i, rightColumn.Count() - 1 )] );
        else if ( languageVisible )
            leftColumn[i]->SetNavRight( m_pLanguageButton );
    }
    for ( int i = 0; i < rightColumn.Count() && !leftColumn.IsEmpty(); ++i )
        rightColumn[i]->SetNavLeft( leftColumn[Min( i, leftColumn.Count() - 1 )] );
    if ( languageVisible )
    {
        m_pLanguageButton->SetNavUp( lastMain );
        m_pLanguageButton->SetNavLeft( leftColumn.IsEmpty() ? nullptr : leftColumn.Tail() );
        if ( modeVisible )
            m_pLanguageButton->SetNavRight( m_pModeButton );
    }
    if ( modeVisible )
    {
        m_pModeButton->SetNavLeft( languageVisible ? m_pLanguageButton : nullptr );
        m_pModeButton->SetNavUp( rightColumn.IsEmpty() ? lastMain : rightColumn.Tail() );
    }
    if ( !leftColumn.IsEmpty() )
        leftColumn[0]->NavigateTo();
    else if ( !currentButtons.IsEmpty() )
        currentButtons.Tail()->NavigateTo();
}

void GamepadUIMainMenu::OnKeyCodeReleased( vgui::KeyCode code )
{
    ButtonCode_t buttonCode = GetBaseButtonCode( code );
    switch (buttonCode)
    {
#ifdef HL2_RETAIL // Steam input and Steam Controller are not supported in SDK2013 (Madi)
    case STEAMCONTROLLER_B:
#endif

    case KEY_XBUTTON_B:
        if ( GamepadUI::GetInstance().IsInLevel() )
        {
            GamepadUI::GetInstance().GetEngineClient()->ClientCmd_Unrestricted( "gamemenucommand resumegame" );
            // I tried it and didn't like it.
            // Oh well.
            //vgui::surface()->PlaySound( "UI/buttonclickrelease.wav" );
        }
        break;
    default:
        BaseClass::OnKeyCodeReleased( code );
        break;
    }
}

GamepadUIMenuState GamepadUIMainMenu::GetCurrentMenuState() const
{
    if ( GamepadUI::GetInstance().IsInLevel() )
        return GamepadUIMenuStates::InGame;
    return GamepadUIMenuStates::MainMenu;
}

CUtlVector<GamepadUIButton*>& GamepadUIMainMenu::GetCurrentButtons()
{
    return m_Buttons[ GetCurrentMenuState() ];
}

float GamepadUIMainMenu::GetCurrentButtonOffset()
{
    return GetCurrentMenuState() == GamepadUIMenuStates::InGame ? m_flButtonsOffsetYInGame : m_flButtonsOffsetYMenu;
}

float GamepadUIMainMenu::GetCurrentLogoOffset()
{
    return GetCurrentMenuState() == GamepadUIMenuStates::InGame ? m_flLogoOffsetYInGame : m_flLogoOffsetYMenu;
}

int GamepadUIMainMenu::GetNewsLeft()
{
    // The supplied design uses a 240 px sidebar at 720p; scale it with the viewport.
    const int width = Clamp( int( m_flNewsWidth * GetTall() / 720.0f ), 0, GetWide() / 3 );
    return GetWide() - width;
}
