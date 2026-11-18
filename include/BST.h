#pragma once
// ============================================================================
// Problema 1: Árbol Binario de Búsqueda (BST) 100% iterativo
//   - Inserción iterativa
//   - Eliminación iterativa (3 casos: hoja, 1 hijo, 2 hijos)
//   - Recorrido in-order con std::stack
//   - Altura (iterativa, por niveles con std::queue)
//   - Extra: pre-orden con std::stack, verificación AVL, exportación a DOT
// ============================================================================
#include <cstdlib>
#include <queue>
#include <sstream>
#include <stack>
#include <string>
#include <unordered_map>
#include <vector>

class BST {
public:
    struct Node {
        int key;
        Node* left = nullptr;
        Node* right = nullptr;
        explicit Node(int k) : key(k) {}
    };

    BST() = default;
    BST(const BST&) = delete;
    BST& operator=(const BST&) = delete;
    ~BST() { clear(); }

    // ------------------------------------------------------------------
    // Inserción iterativa: O(h). Devuelve false si la clave ya existe.
    // ------------------------------------------------------------------
    bool insert(int key) {
        Node* parent = nullptr;
        Node* cur = root_;
        while (cur != nullptr) {
            ++comparisons_;
            if (key == cur->key) return false;  // no se permiten duplicados
            parent = cur;
            cur = (key < cur->key) ? cur->left : cur->right;
        }
        Node* n = new Node(key);
        if (parent == nullptr)      root_ = n;
        else if (key < parent->key) parent->left = n;
        else                        parent->right = n;
        ++size_;
        return true;
    }

    // ------------------------------------------------------------------
    // Eliminación iterativa: O(h).
    //   Caso A: el nodo tiene 0 o 1 hijo  -> se "puentea" con su hijo.
    //   Caso B: el nodo tiene 2 hijos     -> se copia la clave del sucesor
    //           in-order (mínimo del subárbol derecho) y se elimina el
    //           sucesor, que tiene a lo más un hijo (derecho) -> Caso A.
    // ------------------------------------------------------------------
    bool remove(int key) {
        Node* parent = nullptr;
        Node* cur = root_;
        while (cur != nullptr && cur->key != key) {
            parent = cur;
            cur = (key < cur->key) ? cur->left : cur->right;
        }
        if (cur == nullptr) return false;  // no existe

        if (cur->left != nullptr && cur->right != nullptr) {  // Caso B
            Node* succParent = cur;
            Node* succ = cur->right;
            while (succ->left != nullptr) {
                succParent = succ;
                succ = succ->left;
            }
            cur->key = succ->key;  // copiamos la clave del sucesor
            parent = succParent;   // ahora eliminamos el sucesor
            cur = succ;
        }

        // Caso A: cur tiene a lo más un hijo
        Node* child = (cur->left != nullptr) ? cur->left : cur->right;
        if (parent == nullptr)         root_ = child;
        else if (parent->left == cur)  parent->left = child;
        else                           parent->right = child;
        delete cur;
        --size_;
        return true;
    }

    bool contains(int key) const {
        const Node* cur = root_;
        while (cur != nullptr) {
            if (key == cur->key) return true;
            cur = (key < cur->key) ? cur->left : cur->right;
        }
        return false;
    }

    // ------------------------------------------------------------------
    // In-order iterativo con std::stack: O(n) tiempo, O(h) memoria.
    // Se baja siempre a la izquierda apilando; al no poder bajar más se
    // desapila, se visita y se pasa al subárbol derecho.
    // ------------------------------------------------------------------
    std::vector<int> inorder() const {
        std::vector<int> out;
        std::stack<Node*> st;
        Node* cur = root_;
        while (cur != nullptr || !st.empty()) {
            while (cur != nullptr) {
                st.push(cur);
                cur = cur->left;
            }
            cur = st.top();
            st.pop();
            out.push_back(cur->key);
            cur = cur->right;
        }
        return out;
    }

    // Pre-orden iterativo con std::stack (raíz, izquierda, derecha).
    std::vector<int> preorder() const {
        std::vector<int> out;
        if (root_ == nullptr) return out;
        std::stack<Node*> st;
        st.push(root_);
        while (!st.empty()) {
            Node* n = st.top();
            st.pop();
            out.push_back(n->key);
            if (n->right) st.push(n->right);  // derecha primero para que
            if (n->left)  st.push(n->left);   // la izquierda salga antes
        }
        return out;
    }

    // ------------------------------------------------------------------
    // Altura = número de niveles (nodos en el camino raíz-hoja más largo).
    // Árbol vacío -> 0; un solo nodo -> 1. (Altura en aristas = altura - 1.)
    // Se cuenta por niveles con BFS: O(n).
    // ------------------------------------------------------------------
    int height() const {
        if (root_ == nullptr) return 0;
        std::queue<Node*> q;
        q.push(root_);
        int levels = 0;
        while (!q.empty()) {
            std::size_t levelSize = q.size();
            ++levels;
            for (std::size_t i = 0; i < levelSize; ++i) {
                Node* n = q.front();
                q.pop();
                if (n->left)  q.push(n->left);
                if (n->right) q.push(n->right);
            }
        }
        return levels;
    }

    // ------------------------------------------------------------------
    // Verifica la definición AVL del enunciado: |H(left) - H(right)| <= 1
    // en TODOS los nodos. Post-orden iterativo con dos pilas: O(n).
    // ------------------------------------------------------------------
    bool isAVL() const {
        if (root_ == nullptr) return true;
        std::stack<Node*> s1, s2;
        s1.push(root_);
        while (!s1.empty()) {  // s2 queda con el orden post-orden invertido
            Node* n = s1.top();
            s1.pop();
            s2.push(n);
            if (n->left)  s1.push(n->left);
            if (n->right) s1.push(n->right);
        }
        std::unordered_map<const Node*, int> h;
        auto H = [&h](const Node* n) { return n ? h[n] : 0; };
        while (!s2.empty()) {
            Node* n = s2.top();
            s2.pop();
            int hl = H(n->left), hr = H(n->right);
            if (std::abs(hl - hr) > 1) return false;
            h[n] = 1 + (hl > hr ? hl : hr);
        }
        return true;
    }

    // Exporta el árbol en formato Graphviz DOT (para graficarlo).
    std::string toDot(const std::string& name = "BST") const {
        std::ostringstream os;
        os << "digraph " << name << " {\n  graph [ordering=out];\n  node [shape=circle, fixedsize=true, width=0.55, style=filled, fillcolor=\"#dbe9f6\"];\n";
        if (root_ != nullptr) {
            std::stack<Node*> st;
            st.push(root_);
            int nullId = 0;
            while (!st.empty()) {
                Node* n = st.top();
                st.pop();
                os << "  " << n->key << ";\n";
                for (Node* c : {n->left, n->right}) {
                    if (c) {
                        os << "  " << n->key << " -> " << c->key << ";\n";
                        st.push(c);
                    } else {  // nodo invisible para que izquierda/derecha se vean bien
                        os << "  nil" << nullId << " [shape=point, style=invis];\n  "
                           << n->key << " -> nil" << nullId << " [style=invis];\n";
                        ++nullId;
                    }
                }
            }
        }
        os << "}\n";
        return os.str();
    }

    void clear() {  // liberación iterativa de memoria
        if (root_ == nullptr) return;
        std::stack<Node*> st;
        st.push(root_);
        while (!st.empty()) {
            Node* n = st.top();
            st.pop();
            if (n->left)  st.push(n->left);
            if (n->right) st.push(n->right);
            delete n;
        }
        root_ = nullptr;
        size_ = 0;
    }

    const Node* root() const { return root_; }
    std::size_t size() const { return size_; }
    // Número total de comparaciones de claves hechas por insert() (para el experimento).
    unsigned long long comparisons() const { return comparisons_; }

private:
    Node* root_ = nullptr;
    std::size_t size_ = 0;
    unsigned long long comparisons_ = 0;
};
