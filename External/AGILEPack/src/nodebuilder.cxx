#include "AGILEPack/nodebuilder.h"
#include "AGILEPack/mark.h"
#include "AGILEPack/node.h"
#include "AGILEPack/impl.h"
#include <cassert>

namespace YAML
{
	NodeBuilder::NodeBuilder(): m_pMemory(new detail::memory_holder), m_pRoot(0), m_mapDepth(0)
	{
		m_anchors.push_back(0); // since the anchors start at 1
	}
	
	NodeBuilder::~NodeBuilder()
	{
	}
	
	Node NodeBuilder::Root()
	{
		if(!m_pRoot)
			return Node();
		
		return Node(*m_pRoot, m_pMemory);
	}

	void NodeBuilder::OnDocumentStart(const Mark&)
	{
	}
	
	void NodeBuilder::OnDocumentEnd()
	{
	}
	
	void NodeBuilder::OnNull(const Mark& /* mark */, anchor_t anchor)
	{
		detail::node& node = Push(anchor);
		node.set_null();
		Pop();
	}
	
	void NodeBuilder::OnAlias(const Mark& /* mark */, anchor_t anchor)
	{
		detail::node& node = *m_anchors[anchor];
		Push(node);
		Pop();
	}
	
	void NodeBuilder::OnScalar(const Mark& /* mark */, const std::string& tag, anchor_t anchor, const std::string& value)
	{
		detail::node& node = Push(anchor);
		node.set_scalar(value);
		node.set_tag(tag);
		Pop();
	}
	
	void NodeBuilder::OnSequenceStart(const Mark& /* mark */, const std::string& tag, anchor_t anchor)
	{
		detail::node& node = Push(anchor);
		node.set_tag(tag);
		node.set_type(NodeType::Sequence);
	}
	
	void NodeBuilder::OnSequenceEnd()
	{
		Pop();
	}
	
	void NodeBuilder::OnMapStart(const Mark& /* mark */, const std::string& tag, anchor_t anchor)
	{
		detail::node& node = Push(anchor);
		node.set_type(NodeType::Map);
		node.set_tag(tag);
		m_mapDepth++;
	}
	
	void NodeBuilder::OnMapEnd()
	{
		assert(m_mapDepth > 0);
		m_mapDepth--;
		Pop();
	}

	detail::node& NodeBuilder::Push(anchor_t anchor)
	{
		detail::node& node = m_pMemory->create_node();
		RegisterAnchor(anchor, node);
		Push(node);
		return node;
	}
	
	void NodeBuilder::Push(detail::node& node)
	{
		const bool needsKey = (!m_stack.empty() && m_stack.back()->type() == NodeType::Map && m_keys.size() < m_mapDepth);
		
		m_stack.push_back(&node);
		if(needsKey)
			m_keys.push_back(PushedKey(&node, false));
	}
	
	void NodeBuilder::Pop()
	{
		assert(!m_stack.empty());
		if(m_stack.size() == 1) {
			m_pRoot = m_stack[0];
			m_stack.pop_back();
			return;
		}
		
		detail::node& node = *m_stack.back();
		m_stack.pop_back();

		detail::node& collection = *m_stack.back();
		
		if(collection.type() == NodeType::Sequence) {
			collection.push_back(node, m_pMemory);
		} else if(collection.type() == NodeType::Map) {
			assert(!m_keys.empty());
			PushedKey& key = m_keys.back();
			if(key.second) {
				collection.insert(*key.first, node, m_pMemory);
				m_keys.pop_back();
			} else {
				key.second = true;
			}
		} else {
			assert(false);
			m_stack.clear();
		}
	}

	void NodeBuilder::RegisterAnchor(anchor_t anchor, detail::node& node)
	{
		if(anchor) {
			assert(anchor == m_anchors.size());
			m_anchors.push_back(&node);
		}
	}
}
