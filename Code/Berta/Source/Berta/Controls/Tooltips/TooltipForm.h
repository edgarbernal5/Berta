#pragma once
#include "Berta/GUI/Control.h"

namespace Berta
{
    namespace Internal::TooltipForm
    {
        class Reactor : public ControlReactor
        {
        public:
            void Update(Graphics& graphics) override;
			
        protected:
            void DoOnInit() override;
            
        private:
            Window* m_owner { nullptr };
        };
    }
    
    class TooltipForm : public Control<Category::RootTag, Internal::TooltipForm::Reactor, FormEvents>
    {
    public:
        explicit TooltipForm(Window* owner, const Size& size, const FormStyle& windowStyle = { false, false, false, false, true, false });
        TooltipForm(Window* owner, const Rectangle& rectangle, const FormStyle& windowStyle = { false, false, false, false, true, false });

    };
}
