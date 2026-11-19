#pragma once
#include <algorithm>
#include <optional>
#include <queue>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

#include "maze.hpp"

class MoveRule {
public:
    virtual bool allows(Position from, Position to) const = 0;
};

class AllowAllMoves : public MoveRule {
public:
    bool allows(Position, Position) const override { return true; }
};

class MazeGenerator {
public:
    virtual void generate(MazeEditor& maze) = 0;
};

class BacktrackingGenerator : public MazeGenerator {
public:
    explicit BacktrackingGenerator(unsigned seed) : rng(seed) {}

    void generate(MazeEditor& maze) override {
        if (maze.passageCount() != 0) 
            throw std::logic_error("лабиринт уже содержит проходы");
        std::vector<bool> visited(maze.cellCount(), false);
        carve(maze, Position{0, 0}, visited);
    }

private:
    void carve(MazeEditor& maze, Position current, std::vector<bool>& visited) {
        visited[maze.indexOf(current)] = true;
        std::vector<Position> next = maze.neighbors(current);
        std::shuffle(next.begin(), next.end(), rng);
        for (Position n : next) {
            if (!visited[maze.indexOf(n)]) {
                maze.openPassage(current, n);
                carve(maze, n, visited);
            }
        }
    }

    std::mt19937 rng;
};

class PathFinder {
public:
    virtual std::optional<std::vector<Position>> find(const MazeReader& maze, Position start,
                                                      Position goal, const MoveRule& rule) const = 0;
};

class BfsPathFinder : public PathFinder {
public:
    std::optional<std::vector<Position>> find(const MazeReader& maze, Position start, Position goal,
                                              const MoveRule& rule) const override {
        int s = maze.indexOf(start);
        int g = maze.indexOf(goal);

        std::vector<bool> visited(maze.cellCount(), false);
        std::vector<int> parent(maze.cellCount(), -1);
        std::queue<Position> toVisit;
        visited[s] = true;
        toVisit.push(start);

        while (!toVisit.empty()) {
            Position current = toVisit.front();
            toVisit.pop();
            for (Position n : maze.openNeighbors(current)) {
                int i = maze.indexOf(n);
                if (visited[i] || !rule.allows(current, n)) 
                    continue;
                visited[i] = true;
                parent[i] = maze.indexOf(current);
                toVisit.push(n);
            }
        }
        if (!visited[g]) 
            return std::nullopt;

        std::vector<Position> path;
        for (int i = g; i != -1; i = parent[i]) 
            path.push_back(maze.positionOf(i));
        std::reverse(path.begin(), path.end());
        return path;
    }
};

bool isConnected(const MazeReader& maze) {
    std::vector<bool> seen(maze.cellCount(), false);
    std::vector<Position> stack{Position{0, 0}};
    seen[0] = true;
    int count = 1;
    while (!stack.empty()) {
        Position current = stack.back();
        stack.pop_back();
        for (Position n : maze.openNeighbors(current)) {
            int i = maze.indexOf(n);
            if (!seen[i]) {
                seen[i] = true;
                ++count;
                stack.push_back(n);
            }
        }
    }
    return count == maze.cellCount();
}

bool isPerfect(const MazeReader& maze) {
    return isConnected(maze) && maze.passageCount() == maze.cellCount() - 1;
}

std::string renderMaze(const MazeReader& maze, const std::vector<Position>& path = {}) {
    std::vector<std::string> rows(2 * maze.getHeight() + 1, std::string(2 * maze.getWidth() + 1, '#'));

    for (int i = 0; i < maze.cellCount(); ++i) {
        Position p = maze.positionOf(i);
        rows[2 * p.y + 1][2 * p.x + 1] = ' ';
        for (Position n : maze.openNeighbors(p))
            rows[p.y + n.y + 1][p.x + n.x + 1] = ' ';
    }

    for (size_t i = 0; i < path.size(); ++i) {
        rows[2 * path[i].y + 1][2 * path[i].x + 1] = '.';
        if (i + 1 < path.size())
            rows[path[i].y + path[i + 1].y + 1][path[i].x + path[i + 1].x + 1] = '.';
    }
    if (!path.empty()) {
        rows[2 * path.front().y + 1][2 * path.front().x + 1] = 'S';
        rows[2 * path.back().y + 1][2 * path.back().x + 1] = 'E';
    }

    std::string result;
    for (const std::string& row : rows) 
        result += row + "\n";
    return result;
}
