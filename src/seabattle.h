#pragma once

#include <array>
#include <cassert>
#include <iostream>
#include <iterator>
#include <optional>
#include <random>
#include <set>
#include <vector>

class SeabattleField {
public:
    enum class State { UNKNOWN, EMPTY, KILLED, SHIP };
    enum class ShotResult { MISS = 0, HIT = 1, KILL = 2 };

    static constexpr size_t field_size = 8;
    static constexpr int initial_weight = 1 * 4 + 2 * 3 + 3 * 2 + 4 * 1;  // 20

    explicit SeabattleField(State default_elem = State::UNKNOWN);

    template <class T>
    static SeabattleField GetRandomField(T&& random_engine);

    State operator()(size_t x, size_t y) const;
    bool IsKilled(size_t x, size_t y) const;
    static void PrintDigitLine(std::ostream& out);
    void PrintLine(std::ostream& out, size_t y) const;
    bool IsLoser() const;

    ShotResult Shoot(size_t x, size_t y);
    void MarkMiss(size_t x, size_t y);
    void MarkHit(size_t x, size_t y);
    void MarkKill(size_t x, size_t y);

private:
    template <class T>
    static std::optional<SeabattleField> TryGetRandomField(T&& random_engine);

    bool IsKilledInDirection(size_t x, size_t y, int dx, int dy) const;
    void MarkKillInDirection(size_t x, size_t y, int dx, int dy);

    State& Get(size_t x, size_t y);
    State Get(size_t x, size_t y) const;

    static char Repr(State state);

    std::array<State, field_size * field_size> field_;
    int weight_ = initial_weight;
};

template <class T>
SeabattleField SeabattleField::GetRandomField(T&& random_engine) {
    std::optional<SeabattleField> res;
    do {
        res = TryGetRandomField(random_engine);
    } while (!res);

    return *res;
}

template <class T>
std::optional<SeabattleField> SeabattleField::TryGetRandomField(T&& random_engine) {
    SeabattleField result{State::EMPTY};

    std::set<std::pair<size_t, size_t>> availableElements;
    std::vector ship_sizes = {4, 3, 3, 2, 2, 2, 1, 1, 1, 1};

    const auto mark_cell_as_used = [&](size_t x, size_t y) {
        for (int dy = -1; dy <= 1; ++dy) {
            for (int dx = -1; dx <= 1; ++dx) {
                availableElements.erase({x + dx, y + dy});
            }
        }
    };

    const auto dir_to_dxdy = [](size_t dir) -> std::pair<int, int> {
        int dx = dir == 1 ? 1 : dir == 3 ? -1 : 0;
        int dy = dir == 0 ? 1 : dir == 2 ? -1 : 0;

        return {dx, dy};
    };

    const auto check_ship_available = [&](size_t x, size_t y, size_t dir, int ship_length) {
        auto [dx, dy] = dir_to_dxdy(dir);

        for (int i = 0; i < ship_length; ++i) {
            size_t cx = x + dx * i;
            size_t cy = y + dy * i;

            if (cx >= field_size || cy >= field_size) {
                return false;
            }
            if (availableElements.count({cx, cy}) == 0) {
                return false;
            }
        }

        return true;
    };

    const auto mark_ship = [&](size_t x, size_t y, size_t dir, int ship_length) {
        auto [dx, dy] = dir_to_dxdy(dir);

        for (int i = 0; i < ship_length; ++i) {
            size_t cx = x + dx * i;
            size_t cy = y + dy * i;

            result.Get(cx, cy) = State::SHIP;
            mark_cell_as_used(cx, cy);
        }
    };

    for (size_t y = 0; y < field_size; ++y) {
        for (size_t x = 0; x < field_size; ++x) {
            availableElements.insert({x, y});
        }
    }

    using Distr = std::uniform_int_distribution<size_t>;
    using Param = Distr::param_type;

    static const int max_attempts = 100;

    for (int length : ship_sizes) {
        std::pair<size_t, size_t> pos;
        size_t direction;
        int attempt = 0;

        Distr d;
        do {
            if (attempt++ >= max_attempts || availableElements.empty()) {
                return std::nullopt;
            }

            size_t pos_index = d(random_engine, Param(0, availableElements.size() - 1));
            direction = d(random_engine, Param(0, 3));
            pos = *std::next(availableElements.begin(), pos_index);
        } while (!check_ship_available(pos.first, pos.second, direction, length));
        mark_ship(pos.first, pos.second, direction, length);
    }

    return result;
}
