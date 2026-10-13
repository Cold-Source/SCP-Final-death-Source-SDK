#include "cbase.h"
#include "hud_inventory.h"
#include "clientmode.h"
#include <vgui/ISurface.h>
#include <vgui/IInput.h>
#include <vgui/ILocalize.h>
#include <vgui_controls/Controls.h>

using namespace vgui;

int screenW, screenH;
static CHudInventory* g_pHudInventory = nullptr;

CHudInventory* GetInventoryPanel()
{
    if (!g_pHudInventory)
    {
        vgui::Panel* parent = g_pClientMode ? g_pClientMode->GetViewport() : nullptr;
        if (parent)
        {
            g_pHudInventory = new CHudInventory(parent);
            g_pHudInventory->SetVisible(false);
        }
    }
    return g_pHudInventory;
}

CON_COMMAND(toggle_inventory, "Toggle custom inventory UI")
{
    CHudInventory* pInv = GetInventoryPanel();
    if (pInv)
    {
        pInv->ToggleVisible();
    }
}

CHudInventory::CHudInventory(vgui::Panel* parent) : BaseClass(parent, "HudInventory")
{
    SetPaintBackgroundEnabled(true);
    surface()->GetScreenSize(screenW, screenH);

    int panelW = 450;
    int panelH = 350;

    int x = (screenW - panelW) / 2;
    int y = (screenH - panelH) / 2;

    SetBounds(x, y, panelW, panelH);

    m_pInventoryGrid = new InventoryGrid(10, 7);

    InventoryItem* pArmor = new InventoryItem(1, 3, 5, "Heavy Armor (3x5)");
    m_Items.push_back(pArmor);
    m_pInventoryGrid->AddItemAuto(pArmor);

    m_pDraggedItem = nullptr;
    SetMouseInputEnabled(true);
    SetKeyBoardInputEnabled(true);
}

CHudInventory::~CHudInventory()
{
    delete m_pInventoryGrid;
    for (auto item : m_Items) {
        delete item;
    }
    m_Items.clear();
}

void CHudInventory::ToggleVisible()
{
    bool bVisible = !IsVisible();
    SetVisible(bVisible);

    // Управляем видимостью и доступностью курсора мыши
    surface()->SetCursorAlwaysVisible(bVisible);

    if (bVisible)
    {
        SetMouseInputEnabled(true);
        SetKeyBoardInputEnabled(true);
        RequestFocus();
    }
    else
    {
        SetMouseInputEnabled(false);
        SetKeyBoardInputEnabled(false);
    }
}

void CHudInventory::Paint()
{
    BaseClass::Paint();

    int gridW = m_pInventoryGrid->GetWidth();
    int gridH = m_pInventoryGrid->GetHeight();

    for (int y = 0; y < gridH; ++y) {
        for (int x = 0; x < gridW; ++x) {
            int drawX = m_GridStartX + x * m_CellSize;
            int drawY = m_GridStartY + y * m_CellSize;

            surface()->DrawSetColor(45, 45, 45, 220);
            surface()->DrawFilledRect(drawX, drawY, drawX + m_CellSize - 2, drawY + m_CellSize - 2);
        }
    }

    for (auto item : m_Items) {
        if (item == m_pDraggedItem) continue;

        if (item->x >= 0 && item->y >= 0) {
            int drawX = m_GridStartX + item->x * m_CellSize;
            int drawY = m_GridStartY + item->y * m_CellSize;
            int w = item->width * m_CellSize - 2;
            int h = item->height * m_CellSize - 2;

            surface()->DrawSetColor(50, 90, 150, 230);
            surface()->DrawFilledRect(drawX, drawY, drawX + w, drawY + h);

            surface()->DrawSetTextColor(255, 255, 255, 255);
            HFont hFont = scheme()->GetIScheme(scheme()->GetScheme("ClientScheme"))->GetFont("Default", false);
            surface()->DrawSetTextFont(hFont);
            surface()->DrawSetTextPos(drawX + 6, drawY + 6);

            wchar_t wszName[64];
            g_pVGuiLocalize->ConvertANSIToUnicode(item->name.c_str(), wszName, sizeof(wszName));
            surface()->DrawPrintText(wszName, wcslen(wszName));
        }
    }

    if (m_pDraggedItem) {
        int cursorX, cursorY;
        input()->GetCursorPos(cursorX, cursorY);
        ScreenToLocal(cursorX, cursorY);

        int drawX = cursorX - m_DragOffsetX;
        int drawY = cursorY - m_DragOffsetY;
        int w = m_pDraggedItem->width * m_CellSize - 2;
        int h = m_pDraggedItem->height * m_CellSize - 2;

        surface()->DrawSetColor(70, 130, 200, 250);
        surface()->DrawFilledRect(drawX, drawY, drawX + w, drawY + h);
    }
}

void CHudInventory::OnMousePressed(MouseCode code)
{
    if (code == MOUSE_LEFT) {
        int cursorX, cursorY;
        input()->GetCursorPos(cursorX, cursorY);
        ScreenToLocal(cursorX, cursorY);

        for (auto item : m_Items) {
            if (item->x >= 0 && item->y >= 0) {
                int itemX = m_GridStartX + item->x * m_CellSize;
                itemX = m_GridStartX + item->x * m_CellSize; // дублирование не нужно, но оставим чисто
                int itemY = m_GridStartY + item->y * m_CellSize;
                int itemW = item->width * m_CellSize;
                int itemH = item->height * m_CellSize;

                if (cursorX >= itemX && cursorX <= itemX + itemW &&
                    cursorY >= itemY && cursorY <= itemY + itemH)
                {
                    m_pDraggedItem = item;
                    m_DragOffsetX = cursorX - itemX;
                    m_DragOffsetY = cursorY - itemY;
                    m_pInventoryGrid->RemoveItem(item);
                    break;
                }
            }
        }
    }
}

void CHudInventory::OnMouseReleased(MouseCode code)
{
    if (code == MOUSE_LEFT && m_pDraggedItem) {
        int cursorX, cursorY;
        input()->GetCursorPos(cursorX, cursorY);
        ScreenToLocal(cursorX, cursorY);

        int targetX = (cursorX - m_DragOffsetX - m_GridStartX + m_CellSize / 2) / m_CellSize;
        int targetY = (cursorY - m_DragOffsetY - m_GridStartY + m_CellSize / 2) / m_CellSize;

        if (!m_pInventoryGrid->AddItem(m_pDraggedItem, targetX, targetY)) {
            m_pInventoryGrid->AddItemAuto(m_pDraggedItem);
        }

        m_pDraggedItem = nullptr;
    }
}