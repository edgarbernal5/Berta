/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_WINDOW_HEADER
#define BT_WINDOW_HEADER

#include <string>
#include <vector>
#include "Berta/Core/BasicTypes.h"
#include "Berta/GUI/Renderer.h"
#include "Berta/GUI/ControlWindow.h"
#include "Berta/API/WindowAPI.h"

namespace Berta
{
	class Graphics;
	struct ControlEvents;
	struct ControlAppearance;
	class DrawBatch;

	enum class WindowType
	{
		Form = 0,		//Native window.
		Control,
		Panel			//It has no renderer, empty Graphics. Serves as a logical container of Window.
	};

	enum class DrawWindowStatus
	{
		None,
		Updated
	};

	struct Window
	{
		Window() = default;
		Window(WindowType type) : Type(type) {}
		~Window();

		WindowType Type;
		API::NativeWindowHandle RootHandle{};

#if BT_DEBUG
		std::string Name;
#endif

		std::wstring Title;
		bool Visible{ false };
		
		Size ClientSize;
		Point Position;
		Size BorderSize;
		Size MinSize;
		Size MaxSize;
		uint32_t DPI{ 0 };
		float DPIScaleFactor{ 1.0f };

		Renderer Renderer;
		Graphics* RootGraphics{ nullptr };
		DrawBatch* Batcher{ nullptr };
		std::shared_ptr<ControlAppearance> Appearance{ nullptr };
		std::shared_ptr<ControlEvents> Events{ nullptr };
		std::unique_ptr<ControlWindowInterface> ControlWindowPtr{ nullptr }; //TODO: a lo mejor debemos usar un puntero a ControlBase y eliminamos esta interfaz

		Window* Parent{ nullptr };		//A parent window is directly above a child window in the window hierarchy.
		Window* Owner{ nullptr };		//An owner window is a window that is responsible for another window but not necessarily in a direct hierarchical manner.
		std::vector<Window*> Children;

		Window* RootWindow{ nullptr };

		struct Flags
		{
			bool IsEnabled : 1;
			bool IsDisposed : 1;
			bool MakeActive : 1;
			int IsDeferredCount{ 0 };
			bool isUpdating : 1;
			bool IgnoreMouseFocus : 1;
		}Flags;

		Window* MakeTargetWhenInactive{ nullptr };

		DrawWindowStatus DrawStatus{ DrawWindowStatus::None };

		void Init(ControlBase* control);

		//TODO: JustCtrl_AlignToDipsReturnPixels

		uint32_t ToScale(uint32_t units) const
		{
			return static_cast<uint32_t>(units * DPIScaleFactor);
		}

		int ToScale(int units) const
		{
			return static_cast<int>(units * DPIScaleFactor);
		}

		int ToDownwardScale(int units) const
		{
			return static_cast<int>(units / DPIScaleFactor);
		}

		uint32_t ToDownwardScale(uint32_t units) const
		{
			return static_cast<uint32_t>(units / DPIScaleFactor);
		}

		Berta::Size ToScale(Berta::Size units) const
		{
			return (units * DPIScaleFactor);
		}

		bool IsNested() const
		{
			return Type == WindowType::Form && !Owner && Parent;
		}

		bool IsBatchActive() const;

		Window* FindFirstNonPanelAncestor() const;
		Window* FindFirstPanelOrFormAncestor() const;
		bool AreParentsVisible() const;
		bool IsVisible() const;
		bool IsAncestorOf(Window* window) const;

		int GetHierarchyIndex() const;

	private:
		int GetHierarchyIndexInternal(Window* current, Window* target, bool& found) const;
	};
}

#endif