#pragma once
// ============================================================================
// Árbol binario genérico (no necesariamente BST) para los Problemas 3 y 4.
// Se construye a partir de su representación por niveles (level-order),
// donde std::nullopt indica un hijo vacío. Ej: {1, 2, 3, nullopt, 5}
/*
              1
             / \
            2   3
             \
              5
*/
// ============================================================================
#include <optional>
#include <queue>
#include <sstream>
#include <stack>
#include <string>
#include <vector>

struct TreeNode {
    int val;
    TreeNode* left = nullptr;
    TreeNode* right = nullptr;
    explicit TreeNode(int v) : val(v) {}
};

class BinaryTree {
public:
    using LevelOrder = std::vector<std::optional<int>>;

    BinaryTree() = default;
    explicit BinaryTree(const LevelOrder& values) { build(values); }
    BinaryTree(const BinaryTree&) = delete;
    BinaryTree& operator=(const BinaryTree&) = delete;
    ~BinaryTree() { clear(); }

    // Construcción iterativa con std::queue: O(n).
    void build(const LevelOrder& values) {
        clear();
        if (values.empty() || !values[0].has_value()) return;
        root_ = new TreeNode(*values[0]);
        std::queue<TreeNode*> q;
        q.push(root_);
        std::size_t i = 1;
        while (!q.empty() && i < values.size()) {
            TreeNode* n = q.front();
            q.pop();
            if (i < values.size() && values[i].has_value()) {
                n->left = new TreeNode(*values[i]);
                q.push(n->left);
            }
            ++i;
            if (i < values.size() && values[i].has_value()) {
                n->right = new TreeNode(*values[i]);
                q.push(n->right);
            }
            ++i;
        }
    }

    std::string preorderString() const {
        std::ostringstream os;
        if (!root_) return "";
        std::stack<TreeNode*> st;
        st.push(root_);
        bool first = true;
        while (!st.empty()) {
            TreeNode* n = st.top();
            st.pop();
            os << (first ? "" : " ") << n->val;
            first = false;
            if (n->right) st.push(n->right);
            if (n->left)  st.push(n->left);
        }
        return os.str();
    }

    void clear() {
        if (!root_) return;
        std::stack<TreeNode*> st;
        st.push(root_);
        while (!st.empty()) {
            TreeNode* n = st.top();
            st.pop();
            if (n->left)  st.push(n->left);
            if (n->right) st.push(n->right);
            delete n;
        }
        root_ = nullptr;
    }

    const TreeNode* root() const { return root_; }

private:
    TreeNode* root_ = nullptr;
};
