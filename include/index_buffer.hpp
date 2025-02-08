#pragma once

#include "buffer.hpp"

namespace ignis {

class Device;

class VertexBuffer : public Buffer {
public:
	VertexBuffer(Device&,
				 uint32_t verticesCount,
				 VkDeviceSize stride,
				 const void* vertices = {});

	~VertexBuffer() override;

public:
	VertexBuffer(const VertexBuffer&) = delete;
	VertexBuffer(VertexBuffer&&) = delete;
	VertexBuffer& operator=(const VertexBuffer&) = delete;
	VertexBuffer& operator=(VertexBuffer&&) = delete;
};

}  // namespace ignis
