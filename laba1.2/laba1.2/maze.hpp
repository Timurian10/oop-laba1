#pragma once
#include <algorithm>
#include <cstdlib>
#include <stdexcept>
#include <vector>

struct Position {
    int x = 0;
    int y = 0;
    bool operator==(const Position& o) const { 
        return x == o.x && y == o.y;
    }
    bool operator!=(const Position& o) const { 
        return !(*this == o);
    }
};

class MazeReader {
public:
    virtual int getWidth() const = 0;
    virtual int getHeight() const = 0;
    virtual std::vector<Position> openNeighbors(Position p) const = 0;
    virtual bool hasPassage(Position a, Position b) const = 0;
    virtual int passageCount() const = 0;

    int cellCount() const { 
        return getWidth() * getHeight();
    }

    bool inBounds(Position p) const { 
        return p.x >= 0 && p.x < getWidth() && p.y >= 0 && p.y < getHeight();
    }

    int indexOf(Position p) const {
        if (!inBounds(p)) 
            throw std::out_of_range("позиция вне лабиринта");
        return p.y * getWidth() + p.x;
    }

    Position positionOf(int index) const { 
        return Position(index % getWidth(), index / getWidth());
    }

    std::vector<Position> neighbors(Position p) const {
        indexOf(p);
        const int dx[4] = {0, 1, 0, -1};
        const int dy[4] = {-1, 0, 1, 0};
        std::vector<Position> result;
        for (int i = 0; i < 4; ++i) {
            Position q(p.x + dx[i], p.y + dy[i]);
            if (inBounds(q)) 
                result.push_back(q);
        }
        return result;
    }
};

class MazeEditor : public MazeReader {
public:
    virtual void openPassage(Position a, Position b) = 0;
};

class MazeGraph : public MazeEditor {
public:
    MazeGraph(int inWidth, int inHeight) {
        width = inWidth;
        height = inHeight;
        if (inWidth < 2 || inHeight < 2) 
            throw std::invalid_argument("размер не меньше 2x2");
        links.resize(inWidth * inHeight);
    }

    int getWidth() const override { 
        return width; 
    }
    int getHeight() const override { 
        return height; 
    }

    void openPassage(Position a, Position b) override {
        int ia = indexOf(a);
        int ib = indexOf(b);
        if (std::abs(a.x - b.x) + std::abs(a.y - b.y) != 1)
            throw std::invalid_argument("проход только между соседними клетками");
        if (hasPassage(a, b)) 
            return;
        links[ia].push_back(ib);
        links[ib].push_back(ia);
    }

    bool hasPassage(Position a, Position b) const override {
        const std::vector<int>& list = links[indexOf(a)];
        return std::find(list.begin(), list.end(), indexOf(b)) != list.end();
    }

    std::vector<Position> openNeighbors(Position p) const override {
        std::vector<Position> result;
        for (int i : links[indexOf(p)]) 
            result.push_back(positionOf(i));
        return result;
    }

    int passageCount() const override {
        int sum = 0;
        for (const auto& list : links) 
            sum += static_cast<int>(list.size());
        return sum / 2;
    }

private:
    int width;
    int height;
    std::vector<std::vector<int>> links;
};

template <class Cell>
class Maze {
public:
    Maze(int width, int height) : graph(width, height), cells(width * height) {}

    MazeGraph& getGraph() {
        return graph;
    }
    const MazeGraph& getGraph() const {
        return graph;
    }

    Cell& getCell(Position p) {
        return cells[graph.indexOf(p)];
    }
    const Cell& getCell(Position p) const {
        return cells[graph.indexOf(p)];
    }

private:
    MazeGraph graph;
    std::vector<Cell> cells;
};
