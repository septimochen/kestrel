#include "kestrel/board.hpp"

#include <charconv>
#include <cstdlib>
#include <limits>
#include <sstream>

namespace kestrel {

namespace {

Piece pieceFromFen(char c) {
    Color color = c >= 'A' && c <= 'Z' ? Color::White : Color::Black;

    switch (static_cast<char>(c | 32)) {
    case 'p':
        return {PieceType::Pawn, color};
    case 'n':
        return {PieceType::Knight, color};
    case 'b':
        return {PieceType::Bishop, color};
    case 'r':
        return {PieceType::Rook, color};
    case 'q':
        return {PieceType::Queen, color};
    case 'k':
        return {PieceType::King, color};
    default:
        return {};
    }
}

} // namespace

Board::Board() { setStartingPosition(); }

void Board::clear() {
    squares_.fill({});
    sideToMove_ = Color::White;
    enPassantSquare_ = -1;
    castlingRights_ = 0;
    halfmoveClock_ = 0;
    fullmoveNumber_ = 1;
}

void Board::setStartingPosition() {
    const auto result =
        loadFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    (void)result;
}

bool Board::setFromFen(const std::string &fen) {
    return loadFen(fen).has_value();
}

std::expected<void, FenError> Board::loadFen(const std::string &fen) {
    // Parse into local values so every failure preserves the current position.
    std::array<Piece, 64> squares{};
    std::istringstream input(fen);
    std::string placement, side, castling, enPassant, halfmove, fullmove, extra;
    if (!(input >> placement >> side >> castling >> enPassant >> halfmove >>
          fullmove) ||
        (input >> extra)) {
        return std::unexpected(FenError::Fields);
    }
    int rank = 7;
    int file = 0;
    bool previousDigit = false;
    for (char c : placement) {
        if (c == '/') {
            if (file != 8 || rank == 0)
                return std::unexpected(FenError::Placement);
            --rank;
            file = 0;
            previousDigit = false;
        } else if (c >= '1' && c <= '8') {
            if (previousDigit)
                return std::unexpected(FenError::Placement);
            file += c - '0';
            previousDigit = true;
        } else {
            const Piece piece = pieceFromFen(c);
            if (piece.empty() || file >= 8)
                return std::unexpected(FenError::Placement);
            squares[makeSquare(file++, rank)] = piece;
            previousDigit = false;
        }
        if (file > 8)
            return std::unexpected(FenError::Placement);
    }
    if (rank != 0 || file != 8)
        return std::unexpected(FenError::Placement);
    if (side != "w" && side != "b")
        return std::unexpected(FenError::Side);
    const Color color = side == "w" ? Color::White : Color::Black;
    uint8_t rights = 0;
    if (castling != "-") {
        for (char c : castling) {
            uint8_t right = 0;
            switch (c) {
            case 'K':
                right = WhiteKingSide;
                break;
            case 'Q':
                right = WhiteQueenSide;
                break;
            case 'k':
                right = BlackKingSide;
                break;
            case 'q':
                right = BlackQueenSide;
                break;
            default:
                return std::unexpected(FenError::Castling);
            }
            if (rights & right)
                return std::unexpected(FenError::Castling);
            rights |= right;
        }
    }
    int target = -1;
    if (enPassant != "-") {
        if (enPassant.size() != 2 || enPassant[0] < 'a' || enPassant[0] > 'h' ||
            enPassant[1] != (color == Color::White ? '6' : '3')) {
            return std::unexpected(FenError::EnPassant);
        }
        target = makeSquare(enPassant[0] - 'a', enPassant[1] - '1');
    }
    auto parseCounter = [](const std::string &text,
                           int minimum) -> std::expected<int, FenError> {
        if (text.empty() || text.front() < '0' || text.front() > '9')
            return std::unexpected(FenError::Counters);
        int value = 0;
        const auto [end, error] =
            std::from_chars(text.data(), text.data() + text.size(), value);
        if (error != std::errc{} || end != text.data() + text.size() ||
            value < minimum)
            return std::unexpected(FenError::Counters);
        return value;
    };
    const auto half = parseCounter(halfmove, 0);
    const auto full = parseCounter(fullmove, 1);
    if (!half || !full)
        return std::unexpected(FenError::Counters);
    squares_ = squares;
    sideToMove_ = color;
    castlingRights_ = rights;
    enPassantSquare_ = target;
    halfmoveClock_ = *half;
    fullmoveNumber_ = *full;
    return {};
}

const Piece &Board::pieceAt(Square square) const { return squares_[square]; }

Piece &Board::pieceAt(Square square) { return squares_[square]; }

BoardState Board::makeMove(const Move &move) {
    const Piece moving = squares_[move.from];
    const Square capturedSquare =
        move.isEnPassant
            ? static_cast<Square>(static_cast<int>(move.to) +
                                  (moving.color == Color::White ? -8 : 8))
            : move.to;
    BoardState state{moving,           squares_[capturedSquare], capturedSquare,
                     halfmoveClock_,   fullmoveNumber_,          sideToMove_,
                     enPassantSquare_, castlingRights_};

    squares_[move.to] = moving;
    squares_[move.from] = {};

    if (move.promotion != PieceType::None) {
        squares_[move.to].type = move.promotion;
    }

    if (move.isEnPassant) {
        int captureOffset = moving.color == Color::White ? -8 : 8;
        squares_[move.to + captureOffset] = {};
    }

    if (move.isCastling) {
        if (move.to > move.from) {
            Square rookFrom = move.from + 3;
            Square rookTo = move.from + 1;
            squares_[rookTo] = squares_[rookFrom];
            squares_[rookFrom] = {};
        } else {
            Square rookFrom = move.from - 4;
            Square rookTo = move.from - 1;
            squares_[rookTo] = squares_[rookFrom];
            squares_[rookFrom] = {};
        }
    }

    auto removeRookRight = [this](Square square, Color color) {
        if (color == Color::White && square == A1)
            castlingRights_ &= ~WhiteQueenSide;
        if (color == Color::White && square == H1)
            castlingRights_ &= ~WhiteKingSide;
        if (color == Color::Black && square == A8)
            castlingRights_ &= ~BlackQueenSide;
        if (color == Color::Black && square == H8)
            castlingRights_ &= ~BlackKingSide;
    };
    if (moving.type == PieceType::King) {
        castlingRights_ &= moving.color == Color::White
                               ? ~(WhiteKingSide | WhiteQueenSide)
                               : ~(BlackKingSide | BlackQueenSide);
    }
    if (moving.type == PieceType::Rook)
        removeRookRight(move.from, moving.color);
    if (state.capturedPiece.type == PieceType::Rook)
        removeRookRight(capturedSquare, state.capturedPiece.color);
    // Saturate counters rather than overflowing on structurally valid extreme
    // FENs.
    if (moving.type == PieceType::Pawn || !state.capturedPiece.empty())
        halfmoveClock_ = 0;
    else if (halfmoveClock_ < std::numeric_limits<int>::max())
        ++halfmoveClock_;
    if (sideToMove_ == Color::Black &&
        fullmoveNumber_ < std::numeric_limits<int>::max())
        ++fullmoveNumber_;

    enPassantSquare_ = -1;
    if (moving.type == PieceType::Pawn &&
        std::abs(static_cast<int>(move.to) - static_cast<int>(move.from)) ==
            16) {
        enPassantSquare_ =
            (static_cast<int>(move.from) + static_cast<int>(move.to)) / 2;
    }

    sideToMove_ = opposite(sideToMove_);
    return state;
}

void Board::undoMove(const Move &move, const BoardState &state) {
    if (move.isCastling) {
        if (move.to > move.from) {
            Square rookFrom = move.from + 3;
            Square rookTo = move.from + 1;
            squares_[rookFrom] = squares_[rookTo];
            squares_[rookTo] = {};
        } else {
            Square rookFrom = move.from - 4;
            Square rookTo = move.from - 1;
            squares_[rookFrom] = squares_[rookTo];
            squares_[rookTo] = {};
        }
    }

    squares_[move.from] = state.movingPiece;
    squares_[move.to] = {};
    squares_[state.capturedSquare] = state.capturedPiece;
    halfmoveClock_ = state.halfmoveClock;
    fullmoveNumber_ = state.fullmoveNumber;

    sideToMove_ = state.sideToMove;
    enPassantSquare_ = state.enPassantSquare;
    castlingRights_ = state.castlingRights;
}

} // namespace kestrel
