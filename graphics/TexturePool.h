#pragma once

#include "API.h"
#include <queue>
#include <memory>

class ProxyTexture
{
	friend class TexturePool;
	friend class dx12::Renderer;

public:
	ProxyTexture(dx12::Texture* tex) : _texture(tex) {}
	ProxyTexture() = default;
	ProxyTexture(const ProxyTexture&) = default;
	ProxyTexture& operator=(const ProxyTexture&) = default;

	tim::uivec2 resolution() const { return _texture ? _texture->size().to<2>() : 0; }

	bool isEmpty() const { return !(bool)_texture; }

private:
	std::shared_ptr<dx12::Texture> _texture;
};

// manage a pool of textures with the associated GPU-visible descriptors
class TexturePool : NonCopyable
{
	friend class Graphics;
	friend class dx12::Renderer;

public:
	TexturePool(uint32_t size);
	~TexturePool() = default;

	void setTexture(uint32_t, const ProxyTexture&);
	uint32_t size() const;

private:
	dx12::DescriptorHeap& getHeap();
	void lock() { _mutex.lock(); }
	void unlock() { _mutex.unlock(); }

private:
	uint32_t _size;

	std::vector<dx12::Descriptor> _srvDescr;

	struct HeapSet
	{
		std::vector<ProxyTexture> textures;
		std::unique_ptr<dx12::DescriptorHeap> heap;
	};
	HeapSet _curHeap;
	
	std::queue<HeapSet> _freeHeaps;
	bool _used = false;
	std::mutex _mutex;

	void setupFreshHeap();
};

inline uint32_t TexturePool::size() const { return _size; }