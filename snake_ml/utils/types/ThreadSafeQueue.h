#pragma once
#pragma region (copyright)
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
  *  @file ThreadSafeQueue.h
  *  @date January 7, 2016
  *  @author Jeremiah
  *
  *  @brief Thread safe queue.
  */
#pragma endregion

#include <queue>
#include <mutex>

namespace snakeml
{
namespace types
{

template<typename T>
class ThreadSafeQueue
{
public:
    ThreadSafeQueue();
    ThreadSafeQueue(const ThreadSafeQueue& copy);
    ThreadSafeQueue& operator=(const ThreadSafeQueue& copy);

    /**
     * Push a value into the back of the queue.
     */
    void Push(T value);

    /**
     * Try to pop a value from the front of the queue.
     * @returns false if the queue is empty.
     */
    bool TryPop(T& value);

    /**
     * Check to see if there are any items in the queue.
     */
    bool Empty() const;

    /**
     * Retrieve the number of items in the queue.
     */
    size_t Size() const;

private:
    std::queue<T> m_queue;
    mutable std::mutex m_mutex;
};

template<typename T>
ThreadSafeQueue<T>::ThreadSafeQueue()
{}

template<typename T>
ThreadSafeQueue<T>::ThreadSafeQueue(const ThreadSafeQueue<T>& copy)
{
    std::lock_guard<std::mutex> lock(copy.m_mutex);
    m_queue = copy.m_queue;
}

template<typename T>
inline ThreadSafeQueue<T>& ThreadSafeQueue<T>::operator=(const ThreadSafeQueue<T>& copy)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    {
        std::queue<T> emptyQueue;
        std::swap(emptyQueue, m_queue);
    }

    std::lock_guard<std::mutex> lock(copy.m_mutex);
    m_queue = copy.m_queue;

    return *this;
}

template<typename T>
void ThreadSafeQueue<T>::Push(T value)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_queue.push(std::move(value));
}

template<typename T>
bool ThreadSafeQueue<T>::TryPop(T& value)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_queue.empty())
        return false;

    value = m_queue.front();
    m_queue.pop();

    return true;
}

template<typename T>
bool ThreadSafeQueue<T>::Empty() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_queue.empty();
}

template<typename T>
size_t ThreadSafeQueue<T>::Size() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_queue.size();
}

}
}