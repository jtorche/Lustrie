#pragma once

#include "API.h"
#include <memory>
#include <geometry\Mesh.h>

class MeshBuffers
{
	friend class Graphics;

public:
	enum Topology
	{
		Triangles, Points
	};

	MeshBuffers() = default;
	MeshBuffers(const MeshBuffers&) = default;
	MeshBuffers& operator=(const MeshBuffers&) = default;
	MeshBuffers(const std::shared_ptr<dx12::GpuBuffer>& vb, const std::shared_ptr<dx12::GpuBuffer>& ib, uint32_t offset=0, int64_t numIndices=0);

	const std::shared_ptr<dx12::GpuBuffer>& vb() const { return _vb; }
	const std::shared_ptr<dx12::GpuBuffer>& ib() const { return _ib; }

	static MeshBuffers createFromMesh(const tim::BaseMesh&, uint64_t* fence = nullptr, uint32_t nbPointInFace = 3, bool useNormal = true, bool useUV = true);
	static std::shared_ptr<dx12::GpuBuffer> createVertexBufferFromMesh(const tim::BaseMesh&, uint64_t* fence = nullptr);

	void setOffset(uint32_t);
	void setNumIndices(int64_t);
	void setTopology(Topology);

	uint32_t offset() const;
	int64_t numIndices() const;
	Topology topology() const;

private:
	std::shared_ptr<dx12::GpuBuffer> _vb, _ib;
	uint32_t _offset = 0;
	int64_t _numIndexes = -1;
	Topology _topology = Triangles;
};

inline void MeshBuffers::setOffset(uint32_t o) { _offset = o; }
inline void MeshBuffers::setNumIndices(int64_t n) { _numIndexes = n; }
inline void MeshBuffers::setTopology(Topology topo) { _topology = topo; }

inline uint32_t MeshBuffers::offset() const { return _offset; }
inline int64_t MeshBuffers::numIndices() const { return _numIndexes; }
inline MeshBuffers::Topology MeshBuffers::topology() const { return _topology; }