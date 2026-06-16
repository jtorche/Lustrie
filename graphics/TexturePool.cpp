#include "TexturePool.h"
#include <memory>
#include "graphics/Graphics.h"

TexturePool::TexturePool(uint32_t size) 
	: _size(size), _srvDescr(size), _curHeap{ std::vector<ProxyTexture>(size), std::make_unique<dx12::DescriptorHeap>(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, size, true) }
{
	for (uint32_t i = 0; i < size; ++i)
	{
		_srvDescr[i] = _curHeap.heap->alloc(1);
		setTexture(i, Graphics::g_dummyTexture);
	}
}

void TexturePool::setTexture(uint32_t index, const ProxyTexture& proxy)
{
	_ASSERT(index < _size);

	std::lock_guard<std::mutex> _(_mutex);

	if (_used) // need to preserve the old heap and get a new one
	{
		setupFreshHeap();
		_used = false;
	}

	_curHeap.textures[index] = proxy;

	dx12::g_device->CopyDescriptorsSimple(1, _srvDescr[index].cpuHandle(), proxy._texture->SRV().cpuHandle(),
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

dx12::DescriptorHeap& TexturePool::getHeap()
{
	_used = true;
	return *(_curHeap.heap);
}

void TexturePool::setupFreshHeap()
{
	auto tmpCpy = _curHeap.textures;
	_freeHeaps.push({ std::move(_curHeap) });
	if (_freeHeaps.size() > _size)
	{
		std::swap(_curHeap.heap, _freeHeaps.front().heap);
		_curHeap.textures = std::move(tmpCpy);
		_freeHeaps.pop();
	}
	else
	{
		_curHeap.heap = std::make_unique<dx12::DescriptorHeap>(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, _size, true);
		_curHeap.textures = std::move(tmpCpy);

		for (uint32_t i = 0; i < _size; ++i)
			_curHeap.heap->alloc(1);
	}

	for (uint32_t i = 0; i < _size; ++i)
	{
		_srvDescr[i] = _curHeap.heap->get(i);

		if (!_curHeap.textures[i].isEmpty())
		{
			dx12::g_device->CopyDescriptorsSimple(1, _srvDescr[i].cpuHandle(),
				_curHeap.textures[i]._texture->SRV().cpuHandle(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		}
	}

	/*UINT sizeRange[1] = { _size };
	D3D12_CPU_DESCRIPTOR_HANDLE destRange[1] = { _srvDescr[0].cpuHandle() };
	D3D12_CPU_DESCRIPTOR_HANDLE srcRange[1] = { _freeHeaps.back().heap->get(0).cpuHandle() };
	dx12::g_device->CopyDescriptors(1, destRange, sizeRange, 1, srcRange, sizeRange, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	*/
}