#include "polypch.h"
#include "LayerStack.h"

namespace Poly {

	LayerStack::~LayerStack()
	{
		for (auto layer : m_layers)
		{
			layer->onDetach();
			delete layer;
		}
	}

	void LayerStack::pushLayer(Layer* layer)
	{
		m_layers.emplace(m_layers.begin() + m_layerInsertIndex++, layer);
	}

	void LayerStack::pushOverlay(Layer* layer)
	{
		m_layers.emplace_back(layer);
	}

	void LayerStack::popLayer(Layer* layer)
	{
		// Find the layer, erase it from the list if it is not the end (as that would be the end or an overlay)
		auto it = std::find(m_layers.begin(), m_layers.begin() + m_layerInsertIndex, layer);
		if (it != m_layers.begin() + m_layerInsertIndex)
		{
			layer->onDetach();
			m_layers.erase(it);
			--m_layerInsertIndex;
		}
	}

	void LayerStack::popOverlay(Layer* layer)
	{
		auto it = std::find(m_layers.begin() + m_layerInsertIndex, m_layers.end(), layer);
		if (it != m_layers.end())
		{
			layer->onDetach();
			m_layers.erase(it);
		}
	}

}