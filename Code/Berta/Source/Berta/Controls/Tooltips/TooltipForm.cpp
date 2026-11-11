#include "btpch.h"
#include "TooltipForm.h"

namespace Berta
{
    namespace Internal::TooltipForm
    {
        void Reactor::Update(Graphics& graphics)
        {
            auto rect = m_owner->ClientSize.ToRectangle();
            
            graphics.FillRectangle(rect, m_owner->Appearance->TooltipBackground);
            
            int paddingX = m_owner->ToScale(4);
            int paddingY = m_owner->ToScale(2);
            
            graphics.DrawString({paddingX, paddingY},m_owner->Title, m_owner->Appearance->Foreground);
            graphics.DrawRectangle(rect, m_owner->Appearance->TooltipBorder);
            
        }

        void Reactor::DoOnInit()
        {
            m_owner = m_control->Handle();
            GUI::SetIgnoreMouseFocus(m_owner, true);
        }
    }
    
    TooltipForm::TooltipForm(Window* owner, const Size& size, const FormStyle& windowStyle)
    {
        Create(owner, false, GUI::GetCenteredOnScreen(size), windowStyle, false, false);
        GUI::MakeWindowActive(m_handle, false, this->GetParent());

#if BT_DEBUG
        m_handle->Name = "TooltipForm";
#endif
    }

    TooltipForm::TooltipForm(Window* owner, const Rectangle& rectangle, const FormStyle& windowStyle)
    {
        Create(owner, false, rectangle, windowStyle, false, false);
        GUI::MakeWindowActive(m_handle, false, this->GetParent());

#if BT_DEBUG
        m_handle->Name = "TooltipForm";
#endif
    }
}
