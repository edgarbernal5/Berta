/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_COMBO_BOX_HEADER
#define BT_COMBO_BOX_HEADER

#include "Berta/GUI/Window.h"
#include "Berta/GUI/Control.h"
#include "Berta/Controls/Floating/InteractionData.h"
#include "Berta/Paint/Image.h"

#include <optional>
#include <string>

namespace Berta
{
	class TextEditor;
	class FloatBox;
	class ComboBox;
	
	namespace Internal::ComboBox
	{
		struct Appearance : public ControlAppearance
		{
			uint32_t ButtonSize = 18;
			uint32_t ComboBoxItemHeight = 20;
		};
		
		class ComboBoxItem;

		class Reactor : public ControlReactor
		{
		public:
			~Reactor() override;

			void Update(Graphics& graphics) override;

			void MouseEnter(Graphics& graphics, const ArgMouse& args) override;
			void MouseLeave(Graphics& graphics, const ArgMouse& args) override;
			void MouseDown(Graphics& graphics, const ArgMouse& args) override;
			void KeyPressed(Graphics& graphics, const ArgKeyboard& args) override;

			enum class State : uint8_t
			{
				Normal,
				Pressed,
				Hovered
			};

			struct Module
			{
				Float::InteractionData::ItemType& At(size_t index);
				void Clear();
				size_t Count() const;
				void Erase(size_t index);
				void PushBack(const std::wstring& text);
				void PushBack(const std::wstring& text, const Image& icon);
				std::optional<size_t> GetSelectedIndex() const;
				void SetSelectedIndex(std::optional<size_t> index);
				
				std::wstring GetText(size_t index) const;
				std::wstring GetText() const;
				void SetText(const std::wstring& text);
				
				void EmitSelectionEvent(std::optional<size_t> index) const;
				void UpdateItem(size_t index);
				
				bool IsEditable() const;
				void SetEditable(bool editable);
				
				Float::InteractionData Data;

				Window* m_owner{ nullptr };
				TextEditor* m_textEditor{ nullptr };
				std::wstring m_text;

				State m_status{ State::Normal };

				FloatBox* m_floatBox{ nullptr };
				Berta::ComboBox* m_comboBox{ nullptr };
				bool m_isEditable{ false };
			};

			const Module& GetModule() const { return m_module; }
			Module& GetModule() { return m_module; }
			
		protected:
			void DoOnInit() override;
			
		private:
			Module m_module;
		};
		
		class ComboBoxItem
		{
		public:
			ComboBoxItem() = default;
			ComboBoxItem(size_t index, Reactor::Module* module) : m_index(index), m_module(module)
			{
			}
			
			void SetText(const std::wstring& text);
			void SetImage(const Image& icon);
			
			operator bool() const
			{
				return m_module != nullptr;
			}
		private:
			size_t m_index{ 0 };
			Reactor::Module* m_module{ nullptr };
		};
	}

	struct ArgComboBox
	{
		std::optional<size_t> SelectedIndex;
	};

	namespace Internal::ComboBox
	{
		struct Events : public ControlEvents
		{
			Event<ArgComboBox> Selected;
		};
	}
	
	class ComboBox : public Control<Category::ControlTag, Internal::ComboBox::Reactor, Internal::ComboBox::Events, Internal::ComboBox::Appearance>
	{
	public:
		using ComboBoxItem = Internal::ComboBox::ComboBoxItem;
		
	public:
		ComboBox() = default;
		ComboBox(Window* parent, const Rectangle& rectangle = {});

		ComboBoxItem At(size_t index);
		void Clear();
		size_t Count() const;
		void Erase(size_t index);

		void PushBack(const std::wstring& text);
		void PushBack(const std::string& text);
		void PushBack(const std::wstring& text, const Image& icon);
		void PushBack(const std::string& text, const Image& icon);

		std::optional<size_t> GetSelectedIndex() { return GetReactor().GetModule().GetSelectedIndex(); }
		// Allows user to deselect by passing std::nullopt
		void SetSelectedIndex(std::optional<size_t> index);

		std::wstring GetText() const;

		bool isEditable() const { return GetReactor().GetModule().IsEditable(); }
		void SetEditable(bool editable);
		
	protected:
		void DoOnCaption(const std::wstring& caption) override;
		std::wstring DoOnCaption() const override;
	};
}

#endif