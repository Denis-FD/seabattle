#include "seabattle.h"

SeabattleField::SeabattleField(State default_elem) {
    field_.fill(default_elem);
}

SeabattleField::State SeabattleField::operator()(size_t x, size_t y) const {
    return Get(x, y);
}

bool SeabattleField::IsKilled(size_t x, size_t y) const {
    return IsKilledInDirection(x, y, 1, 0) && IsKilledInDirection(x, y, -1, 0) && IsKilledInDirection(x, y, 0, 1) &&
           IsKilledInDirection(x, y, 0, -1);
}

void SeabattleField::PrintDigitLine(std::ostream& out) {
    out << "  1 2 3 4 5 6 7 8  ";
}

void SeabattleField::PrintLine(std::ostream& out, size_t y) const {
    std::array<char, field_size * 2 - 1> line;
    for (size_t x = 0; x < field_size; ++x) {
        line[x * 2] = Repr((*this)(x, y));
        if (x + 1 < field_size) {
            line[x * 2 + 1] = ' ';
        }
    }

    char line_char = static_cast<char>('A' + y);

    out.put(line_char);
    out.put(' ');
    out.write(line.data(), line.size());
    out.put(' ');
    out.put(line_char);
}

bool SeabattleField::IsLoser() const {
    return weight_ == 0;
}

SeabattleField::ShotResult SeabattleField::Shoot(size_t x, size_t y) {
    if (Get(x, y) != State::SHIP)
        return ShotResult::MISS;

    Get(x, y) = State::KILLED;
    --weight_;

    return IsKilled(x, y) ? ShotResult::KILL : ShotResult::HIT;
}

void SeabattleField::MarkMiss(size_t x, size_t y) {
    if (Get(x, y) != State::UNKNOWN) {
        return;
    }
    Get(x, y) = State::EMPTY;
}

void SeabattleField::MarkHit(size_t x, size_t y) {
    if (Get(x, y) != State::UNKNOWN) {
        return;
    }
    --weight_;
    Get(x, y) = State::KILLED;
}

void SeabattleField::MarkKill(size_t x, size_t y) {
    if (Get(x, y) != State::UNKNOWN) {
        return;
    }
    MarkHit(x, y);
    MarkKillInDirection(x, y, 1, 0);
    MarkKillInDirection(x, y, -1, 0);
    MarkKillInDirection(x, y, 0, 1);
    MarkKillInDirection(x, y, 0, -1);
}

bool SeabattleField::IsKilledInDirection(size_t x, size_t y, int dx, int dy) const {
    for (; x < field_size && y < field_size; x += dx, y += dy) {
        if (Get(x, y) == State::EMPTY) {
            return true;
        }
        if (Get(x, y) != State::KILLED) {
            return false;
        }
    }
    return true;
}

void SeabattleField::MarkKillInDirection(size_t x, size_t y, int dx, int dy) {
    auto mark_cell_empty = [this](size_t x, size_t y) {
        if (x >= field_size || y >= field_size) {
            return;
        }
        if (Get(x, y) != State::UNKNOWN) {
            return;
        }
        Get(x, y) = State::EMPTY;
    };
    for (; x < field_size && y < field_size; x += dx, y += dy) {
        mark_cell_empty(x + dy, y + dx);
        mark_cell_empty(x - dy, y - dx);
        mark_cell_empty(x, y);
        if (Get(x, y) != State::KILLED) {
            return;
        }
    }
}

SeabattleField::State& SeabattleField::Get(size_t x, size_t y) {
    return field_[x + y * field_size];
}

SeabattleField::State SeabattleField::Get(size_t x, size_t y) const {
    return field_[x + y * field_size];
}

char SeabattleField::Repr(State state) {
    switch (state) {
        case State::UNKNOWN:
            return '?';
        case State::EMPTY:
            return '.';
        case State::SHIP:
            return 'o';
        case State::KILLED:
            return 'x';
    }

    return '\0';
}
