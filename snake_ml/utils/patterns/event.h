// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#pragma once
#include <functional>
#include <vector>

namespace snakeml
{

/*
* TODO:
*  - trigger events in parallel
*  - thread safety
*  - auto unsubscribe when the lifetime is over
*/

template<class Dispatcher_T, class EventData>
class Event
{
	friend Dispatcher_T;

public:
	Event() = default;
	~Event() = default;

	void Subscribe(void* subscriber, const std::function<void(EventData)>& callable)
	{
		auto foundSubscriber = std::find_if(m_subscriptions.begin(), m_subscriptions.end(),
			[subscriber](const Subscription& a)
			{
				return a.subscriber == subscriber;
			});

		ASSERT(foundSubscriber == m_subscriptions.end(), "Trying to subscribe to the same event more than once");
		if (foundSubscriber == m_subscriptions.end())
		{
			Subscription subcription(subscriber, callable);
			m_subscriptions.push_back(std::move(subcription));
		}
	}

	void Unsubscribe(void* subscriber)
	{
		auto foundSubscriber = std::find_if(m_subscriptions.begin(), m_subscriptions.end(),
			[subscriber](const Subscription& a)
			{
				return a.subscriber == subscriber;
			});

		ASSERT(foundSubscriber != m_subscriptions.end(), "Trying to unsubscribe someone who didn't subscribe");
		if (foundSubscriber != m_subscriptions.end())
		{
			m_subscriptions.erase(foundSubscriber);
		}
	}

private:
	struct Subscription
	{
		Subscription(void* _subscriber, const std::function<void(EventData)>& _callable)
			: subscriber(_subscriber)
			, callable(_callable)
		{}
		void*							subscriber;
		std::function<void(EventData)>	callable;
	};

	void Dispatch(const EventData& data) const
	{
		for (const Subscription& subscription : m_subscriptions)
		{
			subscription.callable(data);
		}
	}

	std::vector<Subscription> m_subscriptions;
};

template<class Dispatcher_T>
class Event<Dispatcher_T, void>
{
	friend Dispatcher_T;

public:
	Event() = default;
	~Event() = default;

	void Subscribe(void* subscriber, const std::function<void()> & callable)
	{
		auto foundSubscriber = std::find_if(m_subscriptions.begin(), m_subscriptions.end(),
			[subscriber](const Subscription& a)
			{
				return a.subscriber == subscriber;
			});

		ASSERT(foundSubscriber == m_subscriptions.end(), "Trying to subscribe to the same event more than once");
		if (foundSubscriber == m_subscriptions.end())
		{
			Subscription subcription(subscriber, callable);
			m_subscriptions.push_back(std::move(subcription));
		}
	}

	void Unsubscribe(void* subscriber)
	{
		auto foundSubscriber = std::find_if(m_subscriptions.begin(), m_subscriptions.end(),
			[subscriber](const Subscription& a)
			{
				return a.subscriber == subscriber;
			});

		ASSERT(foundSubscriber != m_subscriptions.end(), "Trying to unsubscribe someone who didn't subscribe");
		if (foundSubscriber != m_subscriptions.end())
		{
			m_subscriptions.erase(foundSubscriber);
		}
	}

private:
	struct Subscription
	{
		Subscription(void* _subscriber, const std::function<void()>& _callable)
			: subscriber(_subscriber)
			, callable(_callable)
		{}
		void*					subscriber;
		std::function<void()>	callable;
	};

	void Dispatch() const
	{
		for (const Subscription& subscription : m_subscriptions)
		{
			subscription.callable();
		}
	}

	std::vector<Subscription> m_subscriptions;
};

}
