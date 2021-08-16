#pragma once

namespace snakeml
{

class ICastable
{
public:
	virtual ~ICastable() = default;

	template<class ConcreteType> ConcreteType* As();

protected:
	virtual const std::type_info& GetTypeId() = 0;
};

template<class ConcreteType>
inline ConcreteType* ICastable::As()
{
	if (typeid(ConcreteType) == GetTypeId())
	{
		return static_cast<ConcreteType*>(this);
	}
	return nullptr;
}

#define REGISTER_CASTABLE_TYPE(InterfaceType) \
	template<class ConcreteType> \
	class InterfaceType##CastableImpl : public InterfaceType \
	{ \
	public: \
		template<typename ...Params> \
		InterfaceType##CastableImpl(Params&&... params) : InterfaceType(std::forward<Params>(params)...) { } \
		virtual ~InterfaceType##CastableImpl() = default; \
	\
	protected: \
		const std::type_info& GetTypeId() override \
		{ \
			return typeid(ConcreteType); \
		} \
	};

}