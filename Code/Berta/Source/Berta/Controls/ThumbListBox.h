/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_THUMB_LIST_BOX_HEADER
#define BT_THUMB_LIST_BOX_HEADER

#include "Berta/GUI/Window.h"
#include "Berta/GUI/Control.h"
#include "Berta/Controls/ScrollBar.h"
#include "Berta/Paint/Image.h"
#include "Berta/GUI/ScrollableView.h"
#include "Berta/GUI/SelectionController.h"

#include <string>
#include <vector>

#include "Berta/GUI/LassoSelection.h"


namespace Berta
{
	struct ThumbListBoxItem;

	struct ThumbListBoxAppearance : public ControlAppearance
	{
		uint32_t ThumbnailCardHeight = 45;
	};

	struct ArgThumbListBoxItem
	{
		size_t Index{ 0 };
	};

	struct ArgThumbListBox
	{
		std::vector<ThumbListBoxItem> SelectedItems;
	};

	struct ArgThumbListBoxItemVisibility
	{
		size_t Index{ 0 };
		bool Visible{ false };
	};

	struct ThumbListBoxEvents : public ControlEvents
	{
		Event<ArgThumbListBoxItem>	ItemDblClick;
		Event<ArgThumbListBoxItemVisibility>	ItemVisibility;
		Event<ArgThumbListBox>	SelectionChanged;
	};
	
	// LRU = Least Recently Used
	class ThumbnailCacheLRU
	{
	public:
		ThumbnailCacheLRU(size_t capacity = 100) : 
			m_capacity(capacity)
		{
		}

		bool TryGet(uint64_t id, Image& outImage)
		{
			auto it = m_cacheMap.find(id);
			if (it == m_cacheMap.end())
			{
				return false;
			}
			
			// Hit in cache: Move to front (O(1))
			m_lruList.splice(m_lruList.begin(), m_lruList, it->second.listIterator);
			outImage = it->second.image;
			return true;
		}

		void Put(uint64_t id, const Image& image)
		{
			auto it = m_cacheMap.find(id);
			if (it != m_cacheMap.end())
			{
				it->second.image = image;
				m_lruList.splice(m_lruList.begin(), m_lruList, it->second.listIterator);
				return;
			}

			// If we exceed the maximum capacity, we eject the least used one.
			if (m_cacheMap.size() >= m_capacity)
			{
				size_t last = m_lruList.back();
				m_cacheMap.erase(last);
				m_lruList.pop_back();
			}

			m_lruList.push_front(id);
			m_cacheMap[id] = { image, m_lruList.begin() };
		}

		void Erase(size_t id)
		{
			auto it = m_cacheMap.find(id);
			if (it != m_cacheMap.end())
			{
				m_lruList.erase(it->second.listIterator);
				m_cacheMap.erase(it);
			}
		}

		void Clear()
		{
			m_cacheMap.clear();
			m_lruList.clear();
		}
		
		void SetCapacity(size_t newCapacity) 
		{
			m_capacity = newCapacity;
			
			while (m_cacheMap.size() > m_capacity) 
			{
				size_t last = m_lruList.back();
				m_cacheMap.erase(last);
				m_lruList.pop_back();
			}
		}
		
	private:
		size_t m_capacity;
		std::list<uint64_t> m_lruList; 
		
		struct CacheItem
		{
			Image image;
			std::list<uint64_t>::iterator listIterator;
		};
		std::unordered_map<uint64_t, CacheItem> m_cacheMap;
	};

	class ThumbListBoxReactor : public ControlReactor
	{
	public:
		void Init(ControlBase& control, Graphics* graphics) override;
		void Update(Graphics& graphics) override;
		void Resize(Graphics& graphics, const ArgResize& args) override;
		void DblClick(Graphics& graphics, const ArgMouse& args) override;
		void MouseDown(Graphics& graphics, const ArgMouse& args) override;
		void MouseMove(Graphics& graphics, const ArgMouse& args) override;
		void MouseUp(Graphics& graphics, const ArgMouse& args) override;
		void MouseLeave(Graphics& graphics, const ArgMouse& args) override;
		void MouseWheel(Graphics& graphics, const ArgWheel& args) override;
		void KeyPressed(Graphics& graphics, const ArgKeyboard& args) override;
		void KeyReleased(Graphics& graphics, const ArgKeyboard& args) override;

		struct Module
		{
			struct ItemType
			{
				ItemType() = default;

				uint64_t m_id { 0 };
				std::wstring m_text;
				bool m_hasThumbnail { false };
			};

			bool AddItem(const std::wstring& text);
			bool AddItem(const std::wstring& text, const Image& thumbnail);
			ThumbListBoxItem At(size_t index);
			
			bool Clear();
			void Erase(size_t index);
			void SetThumbnailSize(uint32_t size);
			void UpdateScrollMetrics();
			
			bool IsEnabledMultiselection() const;
			bool EnableMultiselection(bool enabled);
			
			std::optional<size_t> GetItemIndexAtMousePosition(const Point& position);
			
			int GetLayoutWidth() const;
			Rectangle GetItemBounds(size_t index, int overrideWidth = -1) const;
			
			void TriggerVisibilityEvent();
			void TriggerSelectionChanged();
			
			void Draw() const;
			void DrawItem(Graphics& graphics, ItemType& item, Point& offset);
			void DrawItemText(Graphics& graphics, ItemType& item, const Rectangle& cardRect);

			std::vector<ThumbListBoxItem> GetSelectedItems();
			
			void InitScrollableView();
			void EnsureVisibility(size_t index);
			
			uint64_t m_idCounter{ 1 };
			std::unique_ptr<ScrollableView> m_scrollableView;
			ThumbnailCacheLRU m_imageCache;
			SelectionController<size_t> m_selectionController;
			LassoSelection m_lassoSelection;
			
			std::vector<ItemType> m_items;
			
			Window* m_window{ nullptr };
			ControlBase* m_control{ nullptr };
			
			uint32_t m_thumbnailSize{ 96u };
			
			size_t m_lastVisibleStart{ 0 };
			size_t m_lastVisibleEnd{ 0 };
			
			bool m_shiftPressed{ false };
			bool m_ctrlPressed{ false };
			std::optional<size_t> m_focusedIndex;
			std::optional<size_t> m_hoveredIndex;

			ThumbListBoxEvents* m_events{ nullptr };
		};

		Module& GetModule() { return m_module; }
		const Module& GetModule() const { return m_module; }

	private:
		Module m_module;
	};

	struct ThumbListBoxItem
	{
		ThumbListBoxItem(size_t logicalIndex, ThumbListBoxReactor::Module* module) :
			m_logicalIndex(logicalIndex), m_module(module)
		{
		}

		void SetText(const std::wstring& text);
		void SetIcon(const Image& image);
		
		explicit operator bool() const
		{
			return m_module;
		}
	private:
		size_t m_logicalIndex { 0 };
		ThumbListBoxReactor::Module* m_module;
	};

	class ThumbListBox : public Control<ThumbListBoxReactor, ThumbListBoxEvents, ThumbListBoxAppearance>
	{
	public:
		ThumbListBox() = default;
		ThumbListBox(Window* parent, const Rectangle& rectangle = {});

		void AddItem(const std::wstring& text);
		void AddItem(const std::string& text);
		void AddItem(const std::wstring& text, const Image& thumbnail);
		void AddItem(const std::string& text, const Image& thumbnail);
		ThumbListBoxItem At(size_t index);
		void Clear();
		void Erase(size_t index);
		void SetThumbnailSize(uint32_t size);

		bool IsEnabledMultiselection() const;
		void EnableMultiselection(bool enabled);

		std::vector<ThumbListBoxItem> GetSelected();
		
		void SetCacheCapacity(size_t maxImages);
		
		void ScrollTo(size_t index);
	};
}

#endif
