// ============================================================================
// Programación III (CS2013) - Tarea #6: Árboles y Grafos
// Función principal: comprueba las operaciones de los 4 problemas.
// ============================================================================
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <random>
#include <string>
#include <vector>

#include "BST.h"
#include "BinaryTree.h"
#include "RedBlackTree.h"
#include "TreeAlgorithms.h"

namespace {

void title(const std::string& t) {
    std::cout << "\n" << std::string(78, '=') << "\n " << t << "\n"
              << std::string(78, '=') << "\n";
}

std::string join(const std::vector<int>& v) {
    std::string s;
    for (std::size_t i = 0; i < v.size(); ++i) s += (i ? " " : "") + std::to_string(v[i]);
    return s;
}

void saveDot(const std::string& file, const std::string& dot) {
    std::filesystem::create_directories("graficos");
    std::ofstream("graficos/" + file) << dot;
}

// ---------------------------------------------------------------------------
void problema1() {
    title("PROBLEMA 1: BST iterativo y analisis de degradacion");

    const std::vector<int> datos = {50, 30, 70, 20, 40, 60, 80, 10, 25, 35,
                                    45, 55, 65, 75, 85, 5, 15, 90, 95, 100};
    BST bst;
    for (int x : datos) bst.insert(x);

    std::cout << "Insertados (" << bst.size() << "): " << join(datos) << "\n";
    std::cout << "In-order : " << join(bst.inorder()) << "\n";
    std::cout << "Altura   : " << bst.height() << "\n";
    saveDot("p1_bst_inicial.dot", bst.toDot("BST_inicial"));

    // Eliminamos 2 elementos: la raíz (caso 2 hijos) y 90 (caso 1 hijo).
    std::cout << "\nEliminar 50 (raiz, 2 hijos -> se reemplaza por su sucesor 55): "
              << (bst.remove(50) ? "OK" : "no existe") << "\n";
    std::cout << "Eliminar 90 (1 hijo -> se puentea con 95): "
              << (bst.remove(90) ? "OK" : "no existe") << "\n";

    const auto in = bst.inorder();
    std::cout << "\nIn-order (std::stack) : " << join(in) << "\n";
    std::cout << "Tamano                : " << bst.size() << "\n";
    std::cout << "Ordenado?             : " << (std::is_sorted(in.begin(), in.end()) ? "si" : "NO") << "\n";
    std::cout << "Altura (niveles)      : " << bst.height() << "\n";
    std::cout << "Pre-orden             : " << join(bst.preorder()) << "\n";
    std::cout << "Es AVL?               : " << (bst.isAVL() ? "si" : "no") << "\n";
    saveDot("p1_bst_final.dot", bst.toDot("BST_final"));

    // Degradación: mismos tamaños pero insertando en orden.
    BST cadena;
    for (int x = 1; x <= 20; ++x) cadena.insert(x);
    std::cout << "\nDegradacion: insertar 1..20 en orden ascendente\n";
    std::cout << "  Altura = " << cadena.height() << " (= n, el arbol es una lista enlazada)\n";
    std::cout << "  Comparaciones totales = " << cadena.comparisons()
              << " (= n(n-1)/2 = " << 20 * 19 / 2 << ")\n";
    std::cout << "  Es AVL? " << (cadena.isAVL() ? "si" : "no") << "\n";
}

// ---------------------------------------------------------------------------
void problema2() {
    title("PROBLEMA 2: Red-Black Tree - insercion de 1..20 (orden ascendente)");

    BST bst;
    RedBlackTree rb;
    bool siempreValido = true;
    for (int x = 1; x <= 20; ++x) {
        bst.insert(x);
        rb.insert(x);
        auto rep = rb.verify();  // verificamos las propiedades tras CADA inserción
        if (!rep.ok) {
            siempreValido = false;
            std::cout << "  Violacion tras insertar " << x << ": " << rep.error << "\n";
        }
    }

    auto rep = rb.verify();
    std::cout << "Propiedades verificadas tras cada insercion: "
              << (siempreValido ? "TODAS OK" : "FALLO") << "\n\n";
    std::cout << "Verificacion formal final:\n";
    std::cout << "  P1 cada nodo rojo o negro      : OK (tipo enum Color)\n";
    std::cout << "  P2 raiz negra                  : " << (rep.rootIsBlack ? "OK" : "FALLA") << "\n";
    std::cout << "  P3 sin rojo-rojo               : " << (rep.noRedRed ? "OK" : "FALLA") << "\n";
    std::cout << "  P4 misma altura negra          : " << (rep.sameBlackHeight ? "OK" : "FALLA")
              << " (bh = " << rep.blackHeight << ")\n";
    std::cout << "  P5 hojas NIL negras            : OK (nullptr se trata como negro)\n";
    std::cout << "  Orden BST / punteros parent    : " << (rep.bstOrder ? "OK" : "FALLA") << " / "
              << (rep.parentLinks ? "OK" : "FALLA") << "\n";
    std::cout << "  Rotaciones = " << rb.rotations() << ", recoloraciones = " << rb.recolorings() << "\n";

    const int n = 20;
    std::cout << "\nComparacion de alturas (mismos datos 1..20):\n";
    std::cout << "  BST (Problema 1) : " << bst.height() << "\n";
    std::cout << "  Red-Black Tree   : " << rb.height() << "\n";
    std::cout << "  Cota 2*log2(n+1) : " << std::fixed << std::setprecision(2)
              << 2 * std::log2(n + 1) << "\n";
    std::cout << "  Minimo posible ceil(log2(n+1)) : " << (int)std::ceil(std::log2(n + 1)) << "\n";

    std::cout << "\nPre-orden BST : " << join(bst.preorder()) << "\n";
    std::cout << "Pre-orden RBT : " << rb.preorderString() << "\n";
    std::cout << "               (N = negro, R = rojo)\n";
    std::cout << "In-order  RBT : " << join(rb.inorder()) << "\n";

    saveDot("p2_bst_ordenado.dot", bst.toDot("BST_ordenado"));
    saveDot("p2_rbt_ordenado.dot", rb.toDot("RBT_ordenado"));
}

// ---------------------------------------------------------------------------
void experimento() {
    title("COMPARACION EXPERIMENTAL: BST vs Red-Black Tree");
    std::cout << std::left << std::setw(7) << "n" << std::setw(12) << "|BST ord"
              << std::setw(10) << "|RBT ord" << std::setw(12) << "|BST aleat" << std::setw(12)
              << "|RBT aleat" << std::setw(11) << "|2lg(n+1)" << std::setw(15) << "|comp BST ord"
              << "|comp RBT ord\n";
    std::cout << std::string(92, '-') << "\n";

    std::mt19937 rng(42);
    for (int n : {20, 100, 1000, 10000, 20000}) {
        std::vector<int> ord(n);
        std::iota(ord.begin(), ord.end(), 1);
        std::vector<int> ale = ord;
        std::shuffle(ale.begin(), ale.end(), rng);

        BST b1, b2;
        RedBlackTree r1, r2;
        for (int x : ord) { b1.insert(x); r1.insert(x); }
        for (int x : ale) { b2.insert(x); r2.insert(x); }
        if (!r1.verify().ok || !r2.verify().ok) std::cout << "  !! RBT invalido\n";

        std::cout << std::left << std::setw(7) << n << "|" << std::setw(11) << b1.height() << "|"
                  << std::setw(9) << r1.height() << "|" << std::setw(11) << b2.height() << "|"
                  << std::setw(11) << r2.height() << "|" << std::setw(10) << std::fixed
                  << std::setprecision(2) << 2 * std::log2(n + 1.0) << "|" << std::setw(14)
                  << b1.comparisons() << "|" << r1.comparisons() << "\n";
    }
    std::cout << "(alturas en niveles; 'comp' = comparaciones totales para insertar los n datos)\n";
}

// ---------------------------------------------------------------------------
void problema3() {
    title("PROBLEMA 3: DFS iterativo (std::stack) - camino raiz-hoja con suma K");

    using std::nullopt;
    /*            5
                /   \
               4     8
              /     / \
             11    13  4
            /  \      / \
           7    2    5   1        */
    BinaryTree t({5, 4, 8, 11, nullopt, 13, 4, 7, 2, nullopt, nullopt, 5, 1});
    std::cout << "Arbol (pre-orden): " << t.preorderString() << "\n";

    for (long long K : {22LL, 26LL, 18LL, 27LL, 5LL, 100LL}) {
        auto p = algo::pathWithSum(t.root(), K);
        std::cout << "  K = " << std::setw(3) << K << " -> ";
        if (p) std::cout << "existe: [" << join(*p) << "]\n";
        else   std::cout << "no existe\n";
    }

    // Con valores negativos (no se puede podar por suma parcial).
    BinaryTree neg({1, -2, -3, 1, 3, -2, nullopt, -1});
    std::cout << "\nArbol con negativos (pre-orden): " << neg.preorderString() << "\n";
    for (long long K : {-1LL, 2LL, -4LL, 0LL}) {
        auto p = algo::pathWithSum(neg.root(), K);
        std::cout << "  K = " << std::setw(3) << K << " -> ";
        if (p) std::cout << "existe: [" << join(*p) << "]\n";
        else   std::cout << "no existe\n";
    }

    BinaryTree vacio;
    std::cout << "\nArbol vacio, K = 0 -> "
              << (algo::pathWithSum(vacio.root(), 0) ? "existe" : "no existe") << "\n";
}

// ---------------------------------------------------------------------------
template <typename Node>
void imprimirMetricas(const std::string& nombre, const Node* root) {
    auto w = algo::maxWidth(root);
    std::cout << std::left << std::setw(37) << nombre << "| completo: " << std::setw(3)
              << (algo::isComplete(root) ? "si" : "no") << "| ancho max: " << std::setw(3)
              << w.width << "(nivel " << std::setw(2) << w.level << ") | ancho posicional: "
              << std::setw(4) << algo::maxPositionalWidth(root) << "| prof. min: "
              << algo::minDepth(root) << "\n";
}

void problema4() {
    title("PROBLEMA 4: BFS iterativo (std::queue) - completitud, ancho, prof. minima");
    using std::nullopt;

    BinaryTree completo({1, 2, 3, 4, 5, 6});                 // completo
    BinaryTree hueco({1, 2, 3, 4, nullopt, 6});              // hueco en el ultimo nivel
    BinaryTree lleno({1, 2, 3, 4, 5, 6, 7});                 // perfecto
    BinaryTree incompleto({1, 2, 3, nullopt, nullopt, 6, 7}); // ultimo nivel a la derecha
    BinaryTree desigual({1, 2, 3, 4, nullopt, nullopt, nullopt, 8});  // rama larga
    BinaryTree ancho({1, 3, 2, 5, nullopt, nullopt, 9, 6, nullopt, nullopt, 7});
    BinaryTree unNodo({42});
    BinaryTree vacio;

    imprimirMetricas("[1,2,3,4,5,6]", completo.root());
    imprimirMetricas("[1,2,3,4,null,6]", hueco.root());
    imprimirMetricas("[1,2,3,4,5,6,7] (perfecto)", lleno.root());
    imprimirMetricas("[1,2,3,null,null,6,7]", incompleto.root());
    imprimirMetricas("[1,2,3,4,null,null,null,8]", desigual.root());
    imprimirMetricas("[1,3,2,5,null,null,9,6,null,null,7]", ancho.root());
    imprimirMetricas("[42] (un nodo)", unNodo.root());
    imprimirMetricas("[] (vacio)", vacio.root());

    // Las mismas funciones (plantillas) aplicadas a los árboles del Problema 2.
    BST bst;
    RedBlackTree rb;
    for (int x = 1; x <= 20; ++x) { bst.insert(x); rb.insert(x); }
    std::cout << "\nAplicado a los arboles del Problema 2 (1..20 en orden):\n";
    imprimirMetricas("BST (degenerado)", bst.root());
    imprimirMetricas("Red-Black Tree", rb.root());
}

}  // namespace

int main() {
    problema1();
    problema2();
    experimento();
    problema3();
    problema4();
    std::cout << "\nArchivos .dot generados en ./graficos (usar: dot -Tpng archivo.dot -o archivo.png)\n";
    return 0;
}
