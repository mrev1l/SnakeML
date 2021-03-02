#pragma once
#pragma region(copyright)
/*
 *  Copyright(c) 2018 Jeremiah van Oosten
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files(the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions :
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 *  IN THE SOFTWARE.
 */

 /**
  *  @file UploadBuffer.h
  *  @date October 24, 2018
  *  @author Jeremiah van Oosten
  *
  *  @brief An UploadBuffer provides a convenient method to upload resources to the GPU.
  */
#pragma endregion

#include "lib_includes/directX_includes.h"
#include <deque>
#include <wrl.h>

namespace snakeml
{
namespace system
{
namespace win
{

#define _KB(x) ((x) * 1024)
#define _MB(x) ((x) * 1024 * 1024)

#define _64KB _KB(64)
#define _1MB _MB(1)
#define _2MB _MB(2)
#define _4MB _MB(4)
#define _8MB _MB(8)
#define _16MB _MB(16)
#define _32MB _MB(32)
#define _64MB _MB(64)
#define _128MB _MB(128)
#define _256MB _MB(256)

class DX12UploadBuffer
{
public:
    // Use to upload data to the GPU
    struct Allocation
    {
        void* CPU;
        D3D12_GPU_VIRTUAL_ADDRESS GPU;
    };

    /**
     * @param pageSize The size to use to allocate new pages in GPU memory.
     */
    explicit DX12UploadBuffer(size_t pageSize = _2MB);
    virtual ~DX12UploadBuffer() = default;

    /**
     * The maximum size of an allocation is the size of a single page.
     */
    size_t GetPageSize() const { return m_pageSize; }

    /**
     * Allocate memory in an Upload heap.
     * An allocation must not exceed the size of a page.
     * Use a memcpy or similar method to copy the
     * buffer data to CPU pointer in the Allocation structure returned from
     * this function.
     */
    Allocation Allocate(size_t sizeInBytes, size_t alignment);

    /**
     * Release all allocated pages. This should only be done when the command list
     * is finished executing on the CommandQueue.
     */
    void Reset();

private:
    // A single page for the allocator.
    class Page
    {
    public:

        Page(size_t sizeInBytes);
        ~Page();

        // Check to see if the page has room to satisfy the requested allocation.
        bool HasSpace(size_t sizeInBytes, size_t alignment) const;

        // Allocate memory from the page.
        // Throws std::bad_alloc if the the allocation size is larger
        // that the page size or the size of the allocation exceeds the 
        // remaining space in the page.
        Allocation Allocate(size_t sizeInBytes, size_t alignment);

        // Reset the page for reuse.
        void Reset();

    private:
        Microsoft::WRL::ComPtr<ID3D12Resource> m_d3d12Resource;

        // Base pointer.
        void* m_CPUPtr;
        D3D12_GPU_VIRTUAL_ADDRESS m_GPUPtr;

        // Allocated page size.
        size_t m_pageSize;
        // Current allocation offset in bytes.
        size_t m_offset;
    };

    // A pool of memory pages.
    using PagePool = std::deque<std::shared_ptr<Page>>;

    // Request a page from the pool of available pages
    // or create a new page if there are no available pages.
    std::shared_ptr<Page> RequestPage();

    PagePool m_pagePool;
    PagePool m_availablePages;
    std::shared_ptr<Page> m_currentPage;
    size_t m_pageSize;
};

}
}
}