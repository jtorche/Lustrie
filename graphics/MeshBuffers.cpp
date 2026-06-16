#include "MeshBuffers.h"
#include <memory>
#include <algorithm>

MeshBuffers::MeshBuffers(const std::shared_ptr<dx12::GpuBuffer>& vb, const std::shared_ptr<dx12::GpuBuffer>& ib , uint32_t offset, int64_t numIndices)
	: _vb(vb), _ib(ib), _offset(offset), _numIndexes(numIndices)
{

}

MeshBuffers MeshBuffers::createFromMesh(const tim::BaseMesh& mesh, uint64_t* fence, uint32_t nbPointInFace, bool useNormal, bool useUV)
{
	if (mesh.nbVertices() <= 0)
		return MeshBuffers();

	uint32_t bufferSize = mesh.requestBufferSize(useNormal, useUV);
	auto indexBuffer = mesh.indexData(nbPointInFace);

	if (indexBuffer.empty() || bufferSize == 0)
		return MeshBuffers();

	dx12::GpuBuffer* vb = new dx12::GpuBuffer(mesh.nbVertices(), bufferSize / mesh.nbVertices());
	dx12::GpuBuffer* ib = new dx12::GpuBuffer((uint32_t)indexBuffer.size(), sizeof(uint32_t));
	
	byte* buffer_data = new byte[bufferSize];
	mesh.fillBuffer(buffer_data, true, true);

	auto& commandContext = dx12::CommandContext::AllocContext(dx12::CommandQueue::COPY);

	for (uint32_t i = 0; i < bufferSize; i += (1 << 20))
	{
		uint32_t numBytes = std::min(uint32_t(1 << 20), bufferSize - i);
		commandContext.initBuffer(*vb, buffer_data + i, numBytes, i);

		if (i > 0 && i % (20 << 20) == 0 && fence == nullptr)
			commandContext.flush(true);
	}

	delete buffer_data;
	bufferSize = (uint32_t)indexBuffer.size() * sizeof(uint32_t);
	buffer_data = (byte*)indexBuffer.data();

	for (uint32_t i = 0; i < bufferSize; i += (1 << 20))
	{
		uint32_t numBytes = std::min(uint32_t(1 << 20), bufferSize - i);
		commandContext.initBuffer(*ib, buffer_data + i, numBytes, i);

		if (i > 0 && i % (20 << 20) == 0 && fence == nullptr)
			commandContext.flush(true);
	}

	if (fence != nullptr)
		*fence = commandContext.finish(false);
	else
		commandContext.finish(true);
		
	MeshBuffers res;
	res._vb = std::shared_ptr<dx12::GpuBuffer>(vb);
	res._ib = std::shared_ptr<dx12::GpuBuffer>(ib);
	return res;
}

std::shared_ptr<dx12::GpuBuffer> MeshBuffers::createVertexBufferFromMesh(const tim::BaseMesh& mesh, uint64_t* fence)
{
	if (mesh.nbVertices() <= 0)
		return std::shared_ptr<dx12::GpuBuffer>();

	uint32_t bufferSize = mesh.requestBufferSize(true, true);

	if (bufferSize == 0)
		return std::shared_ptr<dx12::GpuBuffer>();

	dx12::GpuBuffer* vb = new dx12::GpuBuffer(mesh.nbVertices(), bufferSize / mesh.nbVertices());

	byte* buffer_data = new byte[bufferSize];
	mesh.fillBuffer(buffer_data, true, true);

	auto& commandContext = dx12::CommandContext::AllocContext(dx12::CommandQueue::COPY);

	commandContext.initBuffer(*vb, buffer_data, bufferSize);

	if (fence != nullptr)
		*fence = commandContext.finish(false);
	else
		commandContext.finish(true);

	return std::shared_ptr<dx12::GpuBuffer>(vb);
}