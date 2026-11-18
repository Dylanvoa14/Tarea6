#pragma once
// ============================================================================
// Problema 2: Red-Black Tree (inserción)
//   - Rotación izquierda / derecha
//   - Recoloración
//   - Corrección de violaciones (insertFixup, 3 casos + simétricos)
//   - Verificación formal de las 5 propiedades (+ orden BST y punteros padre)
//   - Altura
// Las hojas nulas (NIL) se representan con nullptr y se consideran NEGRAS.
// ============================================================================
#include <cstdint>
#include <queue>
#include <sstream>
#include <stack>
#include <string>
#include <vector>

class RedBlackTree {
public:
    enum class Color { RED, BLACK };

    struct Node {
        int key;
        Color color = Color::RED;  // todo nodo nuevo nace ROJO
        Node* left = nullptr;
        Node* right = nullptr;
        Node* parent = nullptr;
        explicit Node(int k) : key(k) {}
    };

    // Resultado de la verificación formal
    struct Report {
        bool ok = true;
        bool rootIsBlack = true;      // Propiedad 2
        bool noRedRed = true;         // Propiedad 3
        bool sameBlackHeight = true;  // Propiedad 4
        bool bstOrder = true;         // propiedad de BST
        bool parentLinks = true;      // consistencia de punteros parent
        int blackHeight = 0;          // nodos negros por camino raíz -> NIL (sin contar NIL)
        std::string error;
    };

    RedBlackTree() = default;
    RedBlackTree(const RedBlackTree&) = delete;
    RedBlackTree& operator=(const RedBlackTree&) = delete;
    ~RedBlackTree() { clear(); }

    // ------------------------------------------------------------------
    // Inserción: descenso de BST (O(log n)) + insertFixup (O(log n)).
    // ------------------------------------------------------------------
    bool insert(int key) {
        Node* y = nullptr;
        Node* x = root_;
        while (x != nullptr) {
            ++comparisons_;
            if (key == x->key) return false;  // sin duplicados
            y = x;
            x = (key < x->key) ? x->left : x->right;
        }
        Node* z = new Node(key);
        z->parent = y;
        if (y == nullptr)      root_ = z;
        else if (key < y->key) y->left = z;
        else                   y->right = z;
        ++size_;
        insertFixup(z);
        return true;
    }

    // ------------------------------------------------------------------
    // Verificación formal de propiedades (iterativa, O(n)):
    //   P1. Cada nodo es rojo o negro  -> garantizado por el tipo enum Color.
    //   P2. La raíz es negra.
    //   P3. Ningún nodo rojo tiene hijo rojo.
    //   P4. Todo camino de un nodo a sus hojas NIL tiene igual nº de negros.
    //       (Basta verificarlo desde la raíz: si todos los caminos raíz->NIL
    //        tienen igual nº de negros, también lo cumplen los subárboles,
    //        pues dos caminos desde x comparten el prefijo raíz->x.)
    //   P5. Las hojas NIL son negras -> nullptr se trata como negro.
    // ------------------------------------------------------------------
    Report verify() const {
        Report r;
        if (root_ == nullptr) return r;  // árbol vacío: válido
        if (root_->color != Color::BLACK) {
            r.rootIsBlack = false;
            fail(r, "P2: la raiz es roja");
        }
        if (root_->parent != nullptr) {
            r.parentLinks = false;
            fail(r, "la raiz tiene padre");
        }

        struct Frame { const Node* n; int blacks; long long lo, hi; };
        std::stack<Frame> st;
        st.push({root_, 0, (long long)INT32_MIN - 1, (long long)INT32_MAX + 1});
        int expected = -1;
        while (!st.empty()) {
            Frame f = st.top();
            st.pop();
            const Node* n = f.n;
            int blacks = f.blacks + (n->color == Color::BLACK ? 1 : 0);

            if (!(f.lo < n->key && n->key < f.hi)) {
                r.bstOrder = false;
                fail(r, "orden BST violado en " + std::to_string(n->key));
            }
            if (n->color == Color::RED &&
                (isRed(n->left) || isRed(n->right))) {
                r.noRedRed = false;
                fail(r, "P3: rojo con hijo rojo en " + std::to_string(n->key));
            }
            for (const Node* c : {n->left, n->right}) {
                if (c == nullptr) {  // llegamos a una hoja NIL (negra)
                    if (expected == -1) expected = blacks;
                    else if (blacks != expected) {
                        r.sameBlackHeight = false;
                        fail(r, "P4: altura negra distinta bajo " + std::to_string(n->key));
                    }
                } else if (c->parent != n) {
                    r.parentLinks = false;
                    fail(r, "puntero parent incorrecto en " + std::to_string(c->key));
                }
            }
            if (n->right) st.push({n->right, blacks, n->key, f.hi});
            if (n->left)  st.push({n->left, blacks, f.lo, n->key});
        }
        r.blackHeight = expected;
        return r;
    }

    // Altura = número de niveles (igual definición que en BST::height()).
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

    // Pre-orden iterativo con color: "clave(N)" negro, "clave(R)" rojo.
    std::string preorderString() const {
        std::ostringstream os;
        if (root_ == nullptr) return "";
        std::stack<Node*> st;
        st.push(root_);
        bool first = true;
        while (!st.empty()) {
            Node* n = st.top();
            st.pop();
            if (!first) os << ' ';
            first = false;
            os << n->key << (n->color == Color::BLACK ? "(N)" : "(R)");
            if (n->right) st.push(n->right);
            if (n->left)  st.push(n->left);
        }
        return os.str();
    }

    std::vector<int> inorder() const {
        std::vector<int> out;
        std::stack<Node*> st;
        Node* cur = root_;
        while (cur != nullptr || !st.empty()) {
            while (cur != nullptr) { st.push(cur); cur = cur->left; }
            cur = st.top(); st.pop();
            out.push_back(cur->key);
            cur = cur->right;
        }
        return out;
    }

    // Exporta a Graphviz DOT con los colores reales del árbol.
    std::string toDot(const std::string& name = "RBTree") const {
        std::ostringstream os;
        os << "digraph " << name << " {\n  graph [ordering=out];\n"
           << "  node [shape=circle, fixedsize=true, width=0.55, style=filled, fontcolor=white];\n";
        if (root_ != nullptr) {
            std::stack<Node*> st;
            st.push(root_);
            int nilId = 0;
            while (!st.empty()) {
                Node* n = st.top();
                st.pop();
                os << "  " << n->key << " [fillcolor="
                   << (n->color == Color::BLACK ? "black" : "red") << "];\n";
                for (Node* c : {n->left, n->right}) {
                    if (c) {
                        os << "  " << n->key << " -> " << c->key << ";\n";
                        st.push(c);
                    } else {
                        os << "  nil" << nilId << " [shape=box, width=0.2, height=0.15, "
                           << "label=\"\", fillcolor=black];\n  "
                           << n->key << " -> nil" << nilId << ";\n";
                        ++nilId;
                    }
                }
            }
        }
        os << "}\n";
        return os.str();
    }

    void clear() {
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
    unsigned long long comparisons() const { return comparisons_; }
    unsigned long long rotations() const { return rotations_; }
    unsigned long long recolorings() const { return recolorings_; }

private:
    Node* root_ = nullptr;
    std::size_t size_ = 0;
    unsigned long long comparisons_ = 0;
    unsigned long long rotations_ = 0;
    unsigned long long recolorings_ = 0;

    static bool isRed(const Node* n) { return n != nullptr && n->color == Color::RED; }

    static void fail(Report& r, const std::string& msg) {
        if (r.ok) r.error = msg;  // guardamos la primera violación encontrada
        r.ok = false;
    }

    /*        x                 y
             / \               / \
            a   y     ==>     x   c
               / \           / \
              b   c         a   b          (rotación izquierda en x) */
    void rotateLeft(Node* x) {
        Node* y = x->right;
        x->right = y->left;
        if (y->left != nullptr) y->left->parent = x;
        y->parent = x->parent;
        if (x->parent == nullptr)          root_ = y;
        else if (x == x->parent->left)     x->parent->left = y;
        else                               x->parent->right = y;
        y->left = x;
        x->parent = y;
        ++rotations_;
    }

    /*          y             x
               / \           / \
              x   c   ==>   a   y
             / \               / \
            a   b             b   c        (rotación derecha en y) */
    void rotateRight(Node* y) {
        Node* x = y->left;
        y->left = x->right;
        if (x->right != nullptr) x->right->parent = y;
        x->parent = y->parent;
        if (y->parent == nullptr)          root_ = x;
        else if (y == y->parent->right)    y->parent->right = x;
        else                               y->parent->left = x;
        x->right = y;
        y->parent = x;
        ++rotations_;
    }

    // ------------------------------------------------------------------
    // Corrección tras insertar z (rojo). La única propiedad que puede
    // romperse es P3 (z y su padre rojos) o P2 (z es la raíz).
    // Sea p = padre, g = abuelo, u = tío.
    //   Caso 1 (tío rojo):  recolorear p,u -> negro, g -> rojo; subir z = g.
    //   Caso 2 (tío negro, z forma "triángulo"): rotar en p -> se vuelve Caso 3.
    //   Caso 3 (tío negro, z forma "línea"): p -> negro, g -> rojo, rotar en g.
    // Los casos 2 y 3 terminan el ciclo => a lo más 2 rotaciones por inserción.
    // ------------------------------------------------------------------
    void insertFixup(Node* z) {
        while (isRed(z->parent)) {
            Node* p = z->parent;
            Node* g = p->parent;  // existe: p es rojo => p no es la raíz
            if (p == g->left) {
                Node* u = g->right;
                if (isRed(u)) {                       // Caso 1
                    p->color = Color::BLACK;
                    u->color = Color::BLACK;
                    g->color = Color::RED;
                    ++recolorings_;
                    z = g;
                } else {
                    if (z == p->right) {              // Caso 2 (triángulo)
                        z = p;
                        rotateLeft(z);
                        p = z->parent;
                    }
                    p->color = Color::BLACK;          // Caso 3 (línea)
                    g->color = Color::RED;
                    ++recolorings_;
                    rotateRight(g);
                }
            } else {                                  // simétrico (p es hijo derecho)
                Node* u = g->left;
                if (isRed(u)) {                       // Caso 1'
                    p->color = Color::BLACK;
                    u->color = Color::BLACK;
                    g->color = Color::RED;
                    ++recolorings_;
                    z = g;
                } else {
                    if (z == p->left) {               // Caso 2'
                        z = p;
                        rotateRight(z);
                        p = z->parent;
                    }
                    p->color = Color::BLACK;          // Caso 3'
                    g->color = Color::RED;
                    ++recolorings_;
                    rotateLeft(g);
                }
            }
        }
        root_->color = Color::BLACK;  // Propiedad 2
    }
};
