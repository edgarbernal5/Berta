/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_SINGLE_LINE_EDITOR_HEADER
#define BT_SINGLE_LINE_EDITOR_HEADER

#include "Berta/Controls/TextEditors/TextInputCore.h"
#include "Berta/GUI/Caret.h"
#include "Berta/Core/Timer.h"

namespace Berta
{
    class SingleLineEditor : public TextInputCore
    {
    public:
        SingleLineEditor(Window* owner);
        ~SingleLineEditor() override = default;

        // --- Interfaz Implementada de TextInputCore ---
        void Cut() override;
        void Copy() const override;
        void Paste() override;
        bool SelectAll() override;

        std::wstring GetContent() const override;
        void SetContent(const std::wstring& newContent) override;
        std::wstring GetSelectedText() const override;

        void Render(Graphics& graphics) override;
        bool OnFocus(const ArgFocus& args) override;

        // --- Eventos Específicos ---
        void OnMouseDown(const ArgMouse& args);
        void OnMouseMove(const ArgMouse& args);
        void OnMouseUp(const ArgMouse& args);
        bool OnKeyPressed(const ArgKeyboard& args);
        bool OnDblClick(const ArgMouse& args);
        void OnResize(const ArgResize& args);
		
        void SetEditorArea(const Rectangle& area);

    protected:
        void InsertChar(wchar_t wChr) override; // Llamado por la base en OnKeyChar

    private:
        void MoveCaretHorizontal(int direction, bool wordJump, bool select);
        void HandleDelete();
        void HandleBackspace();
        void DeleteRange(TextPosition start, TextPosition end);

        int GetCaretX() const;
        void UpdateCaretPosition();
        void AdjustView();
        TextPosition GetPositionNextWord(TextPosition currentPos, int direction) const;
        TextPosition GetPositionUnderMouse(const Point& mousePosition) const;
        std::pair<TextPosition, TextPosition> GetWordBounds(TextPosition pos) const;

        void EnsureLayout() const;
        void InvalidateLayout();
        
        // Modelo de datos ultraligero y continuo
        std::wstring m_content; 
        Rectangle m_editorArea;
        mutable TextPaintNativeHandle m_textHandle;
        
        // Renderizado lineal sin dependencias de VisualLine ni ScrollableView
        int m_scrollX{ 0 }; 

        std::unique_ptr<Caret> m_caret;
        Point m_lastMousePosition;
        Timer m_selectionTimer;
        bool m_wasDblClick{ false };
    };
}

#endif