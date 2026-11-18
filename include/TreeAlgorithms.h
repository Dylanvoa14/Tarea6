#pragma once
// ============================================================================
// Problema 3: DFS iterativo (solo std::stack)  -> camino raíz-hoja con suma K
// Problema 4: BFS iterativo (solo std::queue)  -> completitud, ancho máximo,
//                                                profundidad mínima
// Sin recursión. Las funciones BFS son plantillas: sirven para cualquier
// nodo con miembros `left` y `right` (TreeNode, BST::Node, RedBlackTree::Node).
// ============================================================================
#include <cstddef>
#include <optional>
#include <queue>
#include <stack>
#include <utility>
#include <vector>

#include "BinaryTree.h"

namespace algo {

// ----------------------------------------------------------------------------
// Problema 3: ¿Existe un camino raíz-hoja cuya suma sea K? Si existe, se
// devuelve (el primero en orden izquierda->derecha); si no, std::nullopt.
//
// Idea: DFS en pre-orden con una pila de "marcos" (nodo, profundidad, suma
// acumulada). Invariante: cuando se desapila un nodo de profundidad d, el
// vector `path` contiene en sus posiciones [0, d) exactamente a sus
// ancestros. Por eso basta con path.resize(d) y path.push_back(nodo) para
// tener el camino raíz -> nodo, sin guardar una copia del camino por nodo.
//
// Tiempo: O(n) (cada nodo se apila/desapila una vez; resize amortizado O(1)).
// Memoria: O(h) para el camino + O(h) marcos en la pila
//          (a lo más 1 hermano pendiente por nivel => <= h marcos + 1).
// Funciona con valores negativos (no se poda por suma parcial).
// ----------------------------------------------------------------------------
inline std::optional<std::vector<int>> pathWithSum(const TreeNode* root, long long K) {
    if (root == nullptr) return std::nullopt;

    struct Frame {
        const TreeNode* node;
        std::size_t depth;  // profundidad del nodo (raíz = 0)
        long long sum;      // suma desde la raíz hasta este nodo, inclusive
    };

    std::stack<Frame> st;
    std::vector<int> path;
    st.push({root, 0, root->val});

    while (!st.empty()) {
        Frame f = st.top();
        st.pop();

        path.resize(f.depth);         // descartamos la rama ya explorada
        path.push_back(f.node->val);  // path = raíz ... f.node

        const bool isLeaf = (f.node->left == nullptr && f.node->right == nullptr);
        if (isLeaf && f.sum == K) return path;

        // Derecha primero para que la izquierda se procese antes (orden natural).
        if (f.node->right) st.push({f.node->right, f.depth + 1, f.sum + f.node->right->val});
        if (f.node->left)  st.push({f.node->left,  f.depth + 1, f.sum + f.node->left->val});
    }
    return std::nullopt;
}

// ----------------------------------------------------------------------------
// Problema 4a: ¿Es el árbol completo?
// Un árbol es completo si todos los niveles están llenos excepto quizá el
// último, cuyos nodos están lo más a la izquierda posible.
// Equivalente (numeración de heap): en el recorrido por niveles incluyendo
// los hijos nulos, NO aparece ningún nodo después del primer nulo.
// Tiempo O(n), memoria O(ancho).
// ----------------------------------------------------------------------------
template <typename Node>
bool isComplete(const Node* root) {
    if (root == nullptr) return true;
    std::queue<const Node*> q;
    q.push(root);
    bool seenNull = false;
    while (!q.empty()) {
        const Node* n = q.front();
        q.pop();
        if (n == nullptr) {
            seenNull = true;
        } else {
            if (seenNull) return false;  // hay un "hueco" antes de este nodo
            q.push(n->left);             // se encolan también los nulos
            q.push(n->right);
        }
    }
    return true;
}

// ----------------------------------------------------------------------------
// Problema 4b: Ancho máximo = mayor cantidad de nodos en un mismo nivel.
// Devuelve {ancho, nivel donde ocurre (raíz = nivel 1)}.
// Tiempo O(n), memoria O(ancho).
// ----------------------------------------------------------------------------
struct WidthInfo {
    std::size_t width = 0;
    std::size_t level = 0;
};

template <typename Node>
WidthInfo maxWidth(const Node* root) {
    WidthInfo best;
    if (root == nullptr) return best;
    std::queue<const Node*> q;
    q.push(root);
    std::size_t level = 0;
    while (!q.empty()) {
        const std::size_t levelSize = q.size();  // la cola contiene un nivel completo
        ++level;
        if (levelSize > best.width) best = {levelSize, level};
        for (std::size_t i = 0; i < levelSize; ++i) {
            const Node* n = q.front();
            q.pop();
            if (n->left)  q.push(n->left);
            if (n->right) q.push(n->right);
        }
    }
    return best;
}

// Variante (definición alternativa, p.ej. LeetCode 662): ancho de un nivel =
// distancia entre el nodo más a la izquierda y el más a la derecha, contando
// las posiciones nulas intermedias. Se usan índices de heap (hijo izq = 2i,
// der = 2i+1) normalizados por nivel para evitar overflow.
template <typename Node>
unsigned long long maxPositionalWidth(const Node* root) {
    if (root == nullptr) return 0;
    std::queue<std::pair<const Node*, unsigned long long>> q;
    q.push({root, 0});
    unsigned long long best = 0;
    while (!q.empty()) {
        const std::size_t levelSize = q.size();
        const unsigned long long base = q.front().second;
        unsigned long long first = 0, last = 0;
        for (std::size_t i = 0; i < levelSize; ++i) {
            auto [n, idx] = q.front();
            q.pop();
            idx -= base;  // normalizamos: el primero del nivel es 0
            if (i == 0) first = idx;
            last = idx;
            if (n->left)  q.push({n->left, 2 * idx});
            if (n->right) q.push({n->right, 2 * idx + 1});
        }
        if (last - first + 1 > best) best = last - first + 1;
    }
    return best;
}

// ----------------------------------------------------------------------------
// Problema 4c: Profundidad mínima = nº de nodos en el camino más corto de la
// raíz a una HOJA. BFS visita los niveles en orden creciente, así que la
// primera hoja que aparece está a profundidad mínima: se retorna de inmediato
// (no se recorren los niveles más profundos).
// Tiempo O(n) en el peor caso; memoria O(ancho).
// ----------------------------------------------------------------------------
template <typename Node>
std::size_t minDepth(const Node* root) {
    if (root == nullptr) return 0;
    std::queue<std::pair<const Node*, std::size_t>> q;
    q.push({root, 1});
    while (!q.empty()) {
        auto [n, depth] = q.front();
        q.pop();
        if (n->left == nullptr && n->right == nullptr) return depth;  // primera hoja
        if (n->left)  q.push({n->left, depth + 1});
        if (n->right) q.push({n->right, depth + 1});
    }
    return 0;  // inalcanzable
}

}  // namespace algo
