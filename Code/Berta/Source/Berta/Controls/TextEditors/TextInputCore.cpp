#include "btpch.h"
#include "TextInputCore.h"

#include <cwctype>

#include "Berta/GUI/ControlAppearance.h"
#include "Berta/GUI/Interface.h"

namespace Berta
{
    bool TextInputCore::Deselect()
    {
        if (m_selection.IsEmpty())
        {
            return false;
        }
        m_selection.Reset(m_selection.m_endPosition);
		
        return true;
    }
    
    bool TextInputCore::OnKeyChar(const ArgKeyboard& args)
    {
        if (!m_isEditable)
        {
            return false;
        }

        if (!m_predicate || m_predicate(args.Key))
        {
            if (std::iswprint(args.Key))
            {
                InsertChar(args.Key); 
                return true;
            }
        }
        return false;
    }

    void TextInputCore::EmitSelectionChanged(TextPosition oldStart, TextPosition oldEnd) const
    {
        if (oldStart != m_selection.m_startPosition || oldEnd != m_selection.m_endPosition)
        {
        }
    }

    void TextInputCore::EmitValueChanged() const
    {
        if (m_valueChangedCallback)
        {
            m_valueChangedCallback();
        }
    }

    Color TextInputCore::GetBackgroundColor() const
    {
        return GUI::IsWindowEnabled(m_owner) ? m_owner->Appearance->BoxBackground : m_owner->Appearance->BoxPressedBackground;
    }

    TextInputCore::CharClass TextInputCore::GetCharClass(wchar_t c)
    {
        if (std::iswalnum(c) || c == L'_') 
            return CharClass::Alphanumeric;
        if (std::iswspace(c)) 
            return CharClass::Whitespace;
    
        return CharClass::Punctuation;
    }
}
