#include <algorithm>
#include <iostream>
#include <string>
#include <utility>
#include <vector>
#ifdef _WIN32
#include <windows.h>
#endif

#include "algorithms.hpp"
#include "maze.hpp"
#include "test.hpp"

class ForbidEntering : public MoveRule {
public:
    explicit ForbidEntering(std::vector<Position> cells) : cells_(std::move(cells)) {}
    bool allows(Position, Position to) const override {
        return std::find(cells_.begin(), cells_.end(), to) == cells_.end();
    }

private:
    std::vector<Position> cells_;
};

MazeGraph makeSquare() {
    MazeGraph m(2, 2);
    m.openPassage({0, 0}, {1, 0});
    m.openPassage({1, 0}, {1, 1});
    m.openPassage({1, 1}, {0, 1});
    m.openPassage({0, 1}, {0, 0});
    return m;
}

void test_size() {
    MazeGraph m(2, 3);
    check(m.getWidth() == 2 && m.getHeight() == 3 && m.cellCount() == 6);
    check(m.passageCount() == 0);
    checkThrows([] { MazeGraph bad(1, 5); });
    checkThrows([] { MazeGraph bad(5, 1); });
    checkThrows([] { MazeGraph bad(0, 0); });
    checkThrows([] { MazeGraph bad(-3, 4); });
    checkThrows([] { Maze<int> bad(1, 5); });
}

void test_neighbors() {
    MazeGraph m(4, 3);
    check(m.neighbors({0, 0}).size() == 2);
    check(m.neighbors({1, 0}).size() == 3);
    check(m.neighbors({1, 1}).size() == 4);
    check((m.neighbors({1, 1}) == std::vector<Position>{{1, 0}, {2, 1}, {1, 2}, {0, 1}}));
    checkThrows([&] { m.neighbors({4, 0}); });
    checkThrows([&] { m.neighbors({0, -1}); });
}

void test_passages() {
    MazeGraph m(3, 3);
    check(!m.hasPassage({0, 0}, {1, 0}));
    m.openPassage({0, 0}, {1, 0});
    check(m.hasPassage({0, 0}, {1, 0}));
    check(m.hasPassage({1, 0}, {0, 0}));
    check(m.passageCount() == 1);
    m.openPassage({1, 0}, {0, 0});
    check(m.passageCount() == 1);
    check(m.openNeighbors({0, 0}) == std::vector<Position>{Position{1, 0}});
    check(m.openNeighbors({2, 2}).empty());
}

void test_passages_invalid() {
    MazeGraph m(3, 3);
    checkThrows([&] { m.openPassage({0, 0}, {1, 1}); });
    checkThrows([&] { m.openPassage({0, 0}, {2, 0}); });
    checkThrows([&] { m.openPassage({1, 1}, {1, 1}); });
    checkThrows([&] { m.openPassage({2, 0}, {3, 0}); });
    checkThrows([&] { m.hasPassage({0, 0}, {9, 9}); });
    check(m.passageCount() == 0);
}

void test_interfaces() {
    MazeGraph graph(3, 3);
    MazeEditor& editor = graph;
    editor.openPassage({0, 0}, {1, 0});
    const MazeReader& reader = graph;
    check(reader.hasPassage({0, 0}, {1, 0}));
    check(reader.passageCount() == 1);
}

void test_cell_data() {
    struct Room {
        int id = 0;
        std::string name = "empty";
    };
    Maze<Room> m(3, 2);
    m.getCell({1, 1}).name = "treasure";
    BacktrackingGenerator generator(7);
    generator.generate(m.getGraph());
    check(isPerfect(m.getGraph()));
    check(m.getCell({1, 1}).name == "treasure");
    check(m.getCell({0, 0}).name == "empty");
}

void test_generation() {
    const int sizes[][2] = {{2, 2}, {2, 6}, {7, 3}, {10, 10}, {25, 17}};
    for (const auto& size : sizes) {
        for (unsigned seed = 1; seed <= 5; ++seed) {
            MazeGraph m(size[0], size[1]);
            BacktrackingGenerator backtracking(seed);
            MazeGenerator& generator = backtracking;
            generator.generate(m);
            check(isConnected(m));
            check(m.passageCount() == m.cellCount() - 1);
            check(isPerfect(m));
        }
    }
}

void test_generation_twice() {
    MazeGraph m(4, 4);
    BacktrackingGenerator generator(1);
    generator.generate(m);
    checkThrows([&] { generator.generate(m); });
}

void test_generation_is_reproducible() {
    MazeGraph a(8, 6), b(8, 6), c(8, 6);
    BacktrackingGenerator g1(5), g2(5), g3(6);
    g1.generate(a);
    g2.generate(b);
    g3.generate(c);
    check(renderMaze(a) == renderMaze(b));
    check(renderMaze(a) != renderMaze(c));
}

void test_properties() {
    MazeGraph empty(2, 2);
    check(!isConnected(empty));
    check(!isPerfect(empty));

    MazeGraph square = makeSquare();
    check(isConnected(square));
    check(!isPerfect(square));
}

void test_path_basic() {
    MazeGraph m(4, 4);
    BacktrackingGenerator generator(9);
    generator.generate(m);
    BfsPathFinder finder;
    AllowAllMoves any;

    auto same = finder.find(m, {2, 2}, {2, 2}, any);
    check(same.has_value() && same->size() == 1);

    checkThrows([&] { finder.find(m, {-1, 0}, {1, 1}, any); });
    checkThrows([&] { finder.find(m, {0, 0}, {4, 0}, any); });
}

void test_path_correct() {
    MazeGraph m(6, 5);
    BacktrackingGenerator generator(123);
    generator.generate(m);
    BfsPathFinder finder;
    AllowAllMoves any;
    for (int i = 0; i < m.cellCount(); ++i) {
        Position goal = m.positionOf(i);
        auto path = finder.find(m, {0, 0}, goal, any);
        check(path.has_value());
        if (!path) 
            continue;
        check(path->front() == Position{0, 0} && path->back() == goal);
        for (size_t k = 0; k + 1 < path->size(); ++k)
            check(m.hasPassage((*path)[k], (*path)[k + 1]));
    }
}

void test_path_unreachable() {
    BfsPathFinder finder;
    AllowAllMoves any;

    MazeGraph m(3, 3);
    check(!finder.find(m, {0, 0}, {1, 0}, any).has_value());

    MazeGraph two(3, 2);
    two.openPassage({0, 0}, {1, 0});
    two.openPassage({0, 1}, {1, 1});
    check(!finder.find(two, {0, 0}, {1, 1}, any).has_value());
    check(finder.find(two, {0, 0}, {1, 0}, any).has_value());
}

void test_path_shortest_and_rules() {
    MazeGraph m = makeSquare();
    BfsPathFinder finder;
    AllowAllMoves any;
    auto path = finder.find(m, {0, 0}, {1, 1}, any);
    check(path.has_value() && path->size() == 3);

    ForbidEntering noTop({{1, 0}});
    auto around = finder.find(m, {0, 0}, {1, 1}, noTop);
    check(around.has_value() && (*around)[1] == Position{0, 1});

    ForbidEntering noMiddle({{1, 0}, {0, 1}});
    check(!finder.find(m, {0, 0}, {1, 1}, noMiddle).has_value());
}

void test_render() {
    MazeGraph m(2, 2);
    m.openPassage({0, 0}, {1, 0});
    m.openPassage({1, 0}, {1, 1});
    check(renderMaze(m) == "#####\n#   #\n### #\n# # #\n#####\n");
    BfsPathFinder finder;
    AllowAllMoves any;
    auto path = finder.find(m, {0, 0}, {1, 1}, any);
    check(renderMaze(m, *path) == "#####\n#S..#\n###.#\n# #E#\n#####\n");
}

void demo() {
    std::cout << "\n=== Демонстрация ===\n";
    BacktrackingGenerator generator(2024);
    BfsPathFinder finder;
    AllowAllMoves any;

    MazeGraph tiny(5, 3);
    generator.generate(tiny);
    std::cout << "Лабиринт 5x3, идеальный: " << (isPerfect(tiny) ? "да" : "нет") << "\n"
              << renderMaze(tiny) << "\n";

    MazeGraph big(15, 7);
    generator.generate(big);
    auto path = finder.find(big, {0, 0}, {14, 6}, any);
    std::cout << "Лабиринт 15x7, идеальный: " << (isPerfect(big) ? "да" : "нет")
              << ", путь из (0,0) в (14,6): " << path->size() - 1 << " шагов\n"
              << renderMaze(big, *path);
}

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif
    std::cout << "=== Автотесты ===\n";
    runTest("test_size", test_size);
    runTest("test_neighbors", test_neighbors);
    runTest("test_passages", test_passages);
    runTest("test_passages_invalid", test_passages_invalid);
    runTest("test_interfaces", test_interfaces);
    runTest("test_cell_data", test_cell_data);
    runTest("test_generation", test_generation);
    runTest("test_generation_twice", test_generation_twice);
    runTest("test_generation_is_reproducible", test_generation_is_reproducible);
    runTest("test_properties", test_properties);
    runTest("test_path_basic", test_path_basic);
    runTest("test_path_correct", test_path_correct);
    runTest("test_path_unreachable", test_path_unreachable);
    runTest("test_path_shortest_and_rules", test_path_shortest_and_rules);
    runTest("test_render", test_render);
    std::cout << "Проверок: " << g_checks << ", провалено: " << g_failed << "\n";

    if (g_failed == 0) 
        demo();
    return g_failed == 0 ? 0 : 1;
}
