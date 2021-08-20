#pragma once

namespace snakeml
{

#define declare_type2(type) \
struct type##2 \
{ \
	type x, y; \
} 

#define declare_type3(type) \
struct type##3 \
{ \
	type x, y, z; \
}

#define declare_type4(type) \
struct type##3 \
{ \
	type x, y, z, w; \
}

declare_type2(uint32_t);
declare_type2(float);

declare_type3(float);

}
