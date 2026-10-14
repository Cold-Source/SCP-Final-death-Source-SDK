#pragma once
#include <vgui_controls/EditablePanel.h>
#include <vector>
#include <string>

struct InventoryItem {
    int id;
    int width;
    int height;
    std::string name;
    int x, y;
    InventoryItem(int i, int w, int h, const std::string& n)
        : id(i), width(w), height(h), name(n), x(-1), y(-1) {
    }
};

class InventoryGrid {
public:
    InventoryGrid(int w, int h) : m_Width(w), m_Height(h) {
        m_Grid.resize(w * h, false);
    }

    ~InventoryGrid() = default;

    int GetWidth() const { return m_Width; }
    int GetHeight() const { return m_Height; }

    bool AddItemAuto(InventoryItem* item) {
        for (int y = 0; y <= m_Height - item->height; ++y) {
            for (int x = 0; x <= m_Width - item->width; ++x) {
                if (CanPlaceItem(item, x, y)) {
                    PlaceItem(item, x, y);
                    return true;
                }
            }
        }
        return false;
    }

    bool AddItem(InventoryItem* item, int x, int y) {
        if (x < 0 || y < 0 || x + item->width > m_Width || y + item->height > m_Height)
            return false;
        if (CanPlaceItem(item, x, y)) {
            PlaceItem(item, x, y);
            return true;
        }
        return false;
    }

    void RemoveItem(InventoryItem* item) {
        if (item->x < 0 || item->y < 0) return;
        for (int y = item->y; y < item->y + item->height; ++y) {
            for (int x = item->x; x < item->x + item->width; ++x) {
                m_Grid[y * m_Width + x] = false;
            }
        }
        item->x = -1;
        item->y = -1;
    }

private:
    bool CanPlaceItem(InventoryItem* item, int startX, int startY) {
        for (int y = startY; y < startY + item->height; ++y) {
            for (int x = startX; x < startX + item->width; ++x) {
                if (m_Grid[y * m_Width + x]) return false;
            }
        }
        return true;
    }

    void PlaceItem(InventoryItem* item, int startX, int startY) {
        item->x = startX;
        item->y = startY;
        for (int y = startY; y < startY + item->height; ++y) {
            for (int x = startX; x < startX + item->width; ++x) {
                m_Grid[y * m_Width + x] = true;
            }
        }
    }

    int m_Width, m_Height;
    std::vector<bool> m_Grid;
};

class CHudInventory : public vgui::EditablePanel
{
    DECLARE_CLASS_SIMPLE(CHudInventory, vgui::EditablePanel);
public:
    CHudInventory(vgui::Panel* parent);
    virtual ~CHudInventory();

    void ToggleVisible();

protected:
    void Paint() override;
    void OnMousePressed(vgui::MouseCode code) override;
    void OnMouseReleased(vgui::MouseCode code) override;

private:
    int m_CellSize;
    int m_GridStartX;
    int m_GridStartY;
    InventoryGrid* m_pInventoryGrid;
    std::vector<InventoryItem*> m_Items;
    InventoryItem* m_pDraggedItem;
    int m_DragOffsetX;
    int m_DragOffsetY;
};

CHudInventory* GetInventoryPanel();