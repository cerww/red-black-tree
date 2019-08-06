#pragma once
#include <utility>
#include <memory>
#include "iterator_facade.h"
#include <algorithm>

enum struct rb_node_type:bool{
	red,
	black
};

template<typename T>
struct rb_node {
	T me;
	rb_node* parent = nullptr;
	std::unique_ptr<rb_node> left = nullptr;
	std::unique_ptr<rb_node> right = nullptr;
	rb_node_type color = rb_node_type::red;

	void fix_parents() {
		if (left && left->parent != this) {
			left->parent = this;
			left->fix_parents();
		}
		if(right && right->parent != this) {
			right->parent = this;
			right->fix_parents();
		}
	}

	bool is_leaf()const noexcept {
		return !left && !right;
	}
};

template<typename T>
rb_node<T>* left_most_node(rb_node<T>* node) {
	while(node->left) {
		node = node->left.get();
	}
	return node;
}

template<typename T>
rb_node<T>* right_most_node(rb_node<T>* node) {
	while (node->right) {
		node = node->right.get();
	}
	return node;
}

template<typename T>
rb_node<T>* inorder_successor(rb_node<T>* node) {
	//assert(node->right);
	return left_most_node(node->right.get());
}

template<typename T>
rb_node<T>* inorder_predecesor(rb_node<T>* node) {
	//assert(node->left);
	return right_most_node(node->left.get());
}

template<typename T,typename C = std::less<>>
struct rb_tree{
	using node = rb_node<T>;

	template<bool is_const = false>
	struct cursor {
		using value_type = std::conditional_t<is_const, const T, T>;

		cursor() = default;
		explicit cursor(node* current) :
			m_current_node(current)
		{}

		void next() {
			if (!m_current_node->right) {
				node* parent = m_current_node->parent;
				while (parent && parent->right.get() == m_current_node) {
					m_current_node = std::exchange(parent, parent->parent);
				}
				m_current_node = parent;
			}
			else {
				m_current_node = left_most_node(m_current_node->right.get());
			}
		}

		void prev() {
			if (!m_current_node->left) {
				node* parent = m_current_node->parent;
				while (parent && parent->left.get() == m_current_node) {
					m_current_node = std::exchange(parent, parent->parent);
				}
				m_current_node = parent;
			}
			else {
				m_current_node = right_most_node(m_current_node->left.get());
			}
		}

		value_type read() const {
			return m_current_node->me;
		}

		bool operator==(cursor<true> other) const noexcept{
			return other.m_current_node == m_current_node;
		}

		operator cursor<true>() const noexcept {
			return cursor<true>(m_current_node);
		}
	private:
		node* m_current_node = nullptr;
		friend struct cursor<true>;
		friend struct rb_tree<T, C>;
	};

	using iterator = iterator_facade<cursor<false>>;
	using const_iterator = iterator_facade<cursor<true>>;

	void insert(T a) {
		if (!m_root) {
			m_root = std::make_unique<node>(node{ std::move(a),nullptr,nullptr,nullptr,rb_node_type::black });
			++m_size;
		} else {
			std::unique_ptr<node>* parent_parent = nullptr;
			std::unique_ptr<node>* parent = nullptr;
			std::unique_ptr<node>* current = &m_root;
			while (*current) {
				parent_parent = std::exchange(parent, current);
				if (m_comp(a, (**current).me)) {
					current = &(**current).left;
				} else {
					current = &(**current).right;
				}
			}
			assert(parent != nullptr);
			*current = std::make_unique<node>(node{ std::move(a),parent->get() });
			if (parent_parent) {
				auto parent_parent_parent = (*parent_parent)->parent;
				if (rebalance(*parent_parent)) {
					if(parent_parent_parent) {
						parent_parent_parent->fix_parents();
					}else {
						parent_parent->get()->fix_parents();
					}
				}
			} 
			m_root->color = rb_node_type::black;
			m_root->parent = nullptr;			
		}
	}

	int height() {
		return height_impl(m_root.get());
	}

	int height_impl(node* n) {
		if(!n) {
			return 0;
		}
		return 1 + std::max(height_impl(n->left.get()), height_impl(n->right.get()));
	}

	void rebalance_everything() {
		std::vector<std::unique_ptr<node>*> queue(1,&m_root);

		while(!queue.empty()) {
			std::vector<std::unique_ptr<node>*> next_level;
			for(const auto& node:queue) {
				rebalance(*node);
				if((*node)->left) {
					next_level.push_back(&(*node)->left);
				}
				if ((*node)->right) {
					next_level.push_back(&(*node)->right);
				}
			}
			std::swap(queue, next_level);
		}
		m_root->color = rb_node_type::black;
	}

	const_iterator erase(const_iterator it) {
		auto t = it;
		++t;
		erase(it.base().m_current_node);
		return t;
	}

	iterator begin() {
		if (m_root) {
			return iterator(left_most_node(m_root.get()));
		}else {
			return iterator(nullptr);
		}
	}

	iterator end() {
		return iterator(nullptr);
	}

	const_iterator begin()const {
		if (m_root) {
			return const_iterator(left_most_node(m_root.get()));
		}
		else {
			return const_iterator(nullptr);
		}
	}

	const_iterator end()const {
		return const_iterator(nullptr);
	}

private:
	bool rebalance(std::unique_ptr<node>& center) {
		if(center->color!=rb_node_type::black) {
			return false;
		}
		auto& left = center->left;
		auto& right = center->right;
		if(center->left && center->left->color == rb_node_type::red) {
			auto& left_left = left->left;
			auto& left_right = left->right;
			if(center->left->left && center->left->left->color == rb_node_type::red) {								
				//left => left_right
				//center => left
				//left_right => n
				//left_left->color = rb_node_type::black;
				center->color = rb_node_type::black;
				left->color = rb_node_type::red;
				left_left->color = rb_node_type::black;

				std::swap(left, left_right);
				std::swap(left_right, center);
				//std::cout << "ll" << std::endl;
				return true;
			}else if(left_right && left_right->color == rb_node_type::red) {
				auto& left_right_left = left_right->left;
				auto& left_right_right = left_right->right;
				//center=>left_right
				//left_right=>left_right_left
				//left=>left_right_right
				//left_right_left=>left
				//left_right_right=>n
				left_right->color = rb_node_type::red;
				center->color = rb_node_type::black;
				left->color = rb_node_type::black;

				std::tie(center, left, left_right, left_right_left, left_right_right) = std::make_tuple(
					std::move(left_right), std::move(left_right_right), std::move(left_right_left), std::move(left), std::move(center));
				//std::cout << "lr" << std::endl;
				return true;
			}
		}else if(right && right->color == rb_node_type::red) {
			auto& right_left = right->left;
			auto& right_right = right->right;
			if (center->right->left && center->right->left->color == rb_node_type::red) {
				auto& right_left_left = right_left->left;
				auto& right_left_right = right_left->right;

				right_left->color = rb_node_type::red;
				center->color = rb_node_type::black;
				right->color = rb_node_type::black;

				std::tie(center, right, right_left,  right_left_right, right_left_left) = 
					std::make_tuple(std::move(right_left), std::move(right_left_left), std::move(right_left_right), std::move(right), std::move(center));
				
				//std::cout << "rr" << std::endl;
				return true;
			} else if (right_right && right_right->color == rb_node_type::red) {
				center->color = rb_node_type::black;
				right->color = rb_node_type::red;
				right_right->color = rb_node_type::black;
				std::swap(right, right_left);
				std::swap(right_left, center);
				//std::cout << "rl" << std::endl;
				return true;
			}
		}
		return false;
	}

	void erase(node* n) {
		if (n->is_leaf()) {
			if (n == m_root.get()) {
				m_root = nullptr;
			}else {
				auto parent = n->parent;
				parent->color = rb_node_type::red;
				if (parent->left.get() == n) {
					parent->left = nullptr;
				}
				else {
					parent->right = nullptr;
				}
			}
		}
		else if (n->left && n->right) {
			auto in_order_successor = inorder_successor(n);
			std::swap(in_order_successor->me, n->me);
			erase(in_order_successor);
		}
		else if (n->left) {
			//n->right == nullptr
			std::swap(n->me, n->left->me);
			erase(n->left.get());
		}
		else {//n->right
			std::swap(n->me, n->right->me);
			erase(n->right.get());
		}

	}

	std::unique_ptr<node> m_root;
	int m_size = 0;
	C m_comp;
};
