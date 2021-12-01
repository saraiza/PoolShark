#pragma once
#ifndef concurrent_queue_h__
#define concurrent_queue_h__

#include <QMutex>
#include <QQueue>
#include "WinEvent.h"

template <typename T>
class ConcurrentQueue
{
public:
	ConcurrentQueue() = default;
    virtual ~ConcurrentQueue() = default;

    int Count() const { return m_queue.count(); }

	/**@brief pop the top item in the queue, returns false if there isn't one */
    bool Pop(T& item)
    {
		QMutexLocker lock(&m_mutex);
		if (m_queue.isEmpty())
			return false;

        item = std::move(m_queue.front());
		m_queue.pop_front();

		if (m_queue.isEmpty())
			m_evtHasData.ResetEvent();

        return true;
    }

	/** @brief Emplaces (enqueues) an item onto the back (end) of the queue. */
	template<class... Args>
	void Emplace(Args&&... args)
	{
		QMutexLocker lock(&m_mutex);
		m_queue.emplace_back(std::forward<Args>(args)...);
		m_evtHasData.SetEvent();
	}

    /**@Brief Gets a list of all the data in the queue and clears it */
	QVector<T> GetData()
	{
		QMutexLocker lock(&m_mutex);
		QVector<T> vectData = m_queue.toVector();
		m_queue.clear();
		m_evtHasData.ResetEvent();
		return vectData;
	}

	/**@brief Pushes object to the back of the queue */
    template<typename U>
    void Push(U&& item)
    {
		QMutexLocker lock(&m_mutex);
		m_queue.push_back(std::forward<U>(item));
		m_evtHasData.SetEvent();
    }

	/**@brief Clears the queue */
    void Clear()
    {
		QMutexLocker lock(&m_mutex);
		m_queue.clear();
		m_evtHasData.ResetEvent();
	}

	int Count()
	{
		QMutexLocker lock(&m_mutex);
		return m_queue.count();
	}

   	// Thread Sync
	const IWinEvent& GetHasData() const { return m_evtHasData.GetEventIs(); }
	const IWinEvent& GetIsEmpty() const { return m_evtHasData.GetEventIsNot(); }

private:
	QMutex m_mutex;
	QQueue<T> m_queue;
	WinEventManualResetPair m_evtHasData;
};

#endif // concurrent_queue_h__
