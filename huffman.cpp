#include <algorithm>
#include <bitset>
#include <fstream>
#include <memory>
#include <print>
#include <unordered_map>
#include <utility>
#include <vector>

class huffman_code_map {
    struct tree_node;
    using freq_node_pair_type = std::pair<uint32_t, std::unique_ptr<tree_node>>;

    static constexpr int MAX_BITS = 1 << 5;

    static constexpr auto MIN_HEAP_CMP = [](
	const freq_node_pair_type& first, const freq_node_pair_type& second
    ) -> bool {
	return first.first > second.first;
    };

public:
    // Creates a huffman code map from a serialized encoding.
    static huffman_code_map FromSerialized(const std::string& serialized_code_map) {
	
    }

    // Creates a huffman code map from raw file contents.
    static huffman_code_map Create(const std::string& plain_file_contents) {
	std::unordered_map<char, uint32_t> char_counts;
	for (const char c : plain_file_contents) {
	    ++char_counts[c];
	}
	std::vector<freq_node_pair_type> min_freq_heap = get_min_freq_heap(char_counts);
	std::unique_ptr<tree_node> tree_root = get_huffman_tree_root(min_freq_heap);
	return create_code_map_with_tree(min_freq_heap, tree_root.get(), char_counts.size());
    }

private:
    struct tree_node {
	std::unique_ptr<tree_node> left;
	std::unique_ptr<tree_node> right;
	char c = '\0';
    };

    explicit huffman_code_map(size_t n) : code_map_(n) {}

    std::unordered_map<char, std::bitset<MAX_BITS>> code_map_;

    static std::vector<freq_node_pair_type>
    get_min_freq_heap(const std::unordered_map<char, uint32_t>& char_counts) {
	std::vector<freq_node_pair_type> min_freq_heap;
	min_freq_heap.reserve(char_counts.size());
	// Return true when the element should bubble down.
	for (auto& [c, freq] : char_counts) {
	    min_freq_heap.push_back(
		std::make_pair(freq, std::make_unique<tree_node>( tree_node{.c = c} ))
	    );
	}
	std::make_heap(min_freq_heap.begin(), min_freq_heap.end(), MIN_HEAP_CMP);
	return min_freq_heap;
    }

    static std::unique_ptr<tree_node> get_huffman_tree_root(std::vector<freq_node_pair_type>& min_freq_heap) {
	while (min_freq_heap.size() > 1) {
	    std::pop_heap(min_freq_heap.begin(), min_freq_heap.end(), MIN_HEAP_CMP);
	    auto left_pair = std::move(min_freq_heap.back());
	    min_freq_heap.pop_back();

	    std::pop_heap(min_freq_heap.begin(), min_freq_heap.end(), MIN_HEAP_CMP);
	    auto right_pair = std::move(min_freq_heap.back());
	    min_freq_heap.pop_back();

	    min_freq_heap.push_back(std::make_pair(
		/*freq=*/left_pair.first  + right_pair.first,
		/*freq_node_pair_type=*/std::make_unique<tree_node>(tree_node{
		    .left = std::move(left_pair.second),
		    .right = std::move(right_pair.second),
		    .c = '\0'
		})
	    ));
	    std::push_heap(min_freq_heap.begin(), min_freq_heap.end(), MIN_HEAP_CMP);
	}
	return std::move(min_freq_heap.front().second);
    }
	
    static huffman_code_map create_code_map_with_tree(
	std::vector<freq_node_pair_type>& min_freq_heap, tree_node* tree_root, size_t initial_buckets = 16) {
	huffman_code_map tree(initial_buckets);
	std::bitset<MAX_BITS> path_state;
	auto dfs = [&](
		this const auto self, tree_node* const root, size_t bit_idx, huffman_code_map& hf) -> void {
	    if (root->c != '\0') {
		hf.code_map_[root->c] = path_state;
		return;
	    }

	    self(root->left.get(), bit_idx + 1, hf);
	    path_state.set(bit_idx);
	    self(root->right.get(), bit_idx + 1, hf);
	    path_state.reset(bit_idx);
	};

	dfs(min_freq_heap.front().second.get(), 0uz, tree);
	return tree;
    }
};

int main() {
    std::ifstream input_file("text.txt", std::ios::binary);
    if (!input_file.is_open()) {
	std::print("Error opening file\n");
	return 1;
    }

    return 0;
}
