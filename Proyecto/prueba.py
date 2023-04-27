import chess
import chess.engine
engine = chess.engine.SimpleEngine.popen_uci('/usr/games/stockfish')
limit = chess.engine.Limit(time=5.0)

board = chess.Board()
print(chess.SQUARES)
print(chess.SQUARE_NAMES)
