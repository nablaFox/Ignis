#pragma once

#include "buffer.hpp"

namespace ignis {

class Device;

class IndexBuffer : public Buffer {
public:
	IndexBuffer(Device&,
				uint32_t verticesCount,
				VkDeviceSize stride,
				VkIndexType indexType,
				const void* indices = {});

	~IndexBuffer() override;

	VkIndexType getIndexType() const { return m_indexType; }

private:
	VkIndexType m_indexType;

public:
	IndexBuffer(const IndexBuffer&) = delete;
	IndexBuffer(IndexBuffer&&) = delete;
	IndexBuffer& operator=(const IndexBuffer&) = delete;
	IndexBuffer& operator=(IndexBuffer&&) = delete;
};

}  // namespace ignis

