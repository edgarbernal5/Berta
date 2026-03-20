/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_OBJECT_POOL_HEADER
#define BT_OBJECT_POOL_HEADER

#include <vector>
#include <memory>
#include <cstddef>

namespace Berta
{
    template <typename T, size_t ChunkSize = 1024>
    class ObjectPool
    {
    public:
        ObjectPool() = default;
        ~ObjectPool()
        {
            ClearAll();
        }
        
        template <typename... Args>
        T* Allocate(Args&&... args)
        {
            if (m_freeList.empty())
            {
                AllocateChunk();
            }

            T* ptr = m_freeList.back();
            m_freeList.pop_back();

            return new (ptr) T(std::forward<Args>(args)...);
        }

        void Deallocate(T* ptr)
        {
            if (ptr)
            {
                ptr->~T();
                m_freeList.push_back(ptr);
            }
        }

        void ClearAll()
        {
            // Nota: ClearAll asume que ya llamaste a Deallocate para los objetos vivos.
            m_freeList.clear();
            m_chunks.clear();
        }

    private:
        struct Chunk 
        {
            // Usamos std::byte crudo para NO llamar al constructor de T al reservar memoria
            std::unique_ptr<std::byte[]> data;
            Chunk() : data(std::make_unique<std::byte[]>(ChunkSize * sizeof(T)))
            {
            }
        };

        void AllocateChunk()
        {
            m_chunks.emplace_back();
            std::byte* blockStart = m_chunks.back().data.get();
            
            // Llenamos la lista de libres con las direcciones de este nuevo bloque
            for (size_t i = 0; i < ChunkSize; ++i)
            {
                m_freeList.push_back(reinterpret_cast<T*>(blockStart + (i * sizeof(T))));
            }
        }
        
        std::vector<Chunk> m_chunks;
        std::vector<T*> m_freeList;
    };
}
#endif