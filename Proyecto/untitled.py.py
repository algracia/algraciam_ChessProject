import chess
import chess.engine

#Llama y configura al Engine
engine = chess.engine.SimpleEngine.popen_uci('/usr/games/stockfish')

#Ingresa parametros
input('Presione "ENTER" para iniciar')
side = input('Seleccione el color con el que quiere jugar (B/N): ')

#Configura los parametros
engine.configure({'UCI_LimitStrength': 900})
limit = chess.engine.Limit(time=0.1)

#configura el tablero 
board = chess.Board()

#Muestra el tablero inicial
print(board) 
print(board.fen())

#Inicia el juego
while not board.is_game_over():
	entrada = input('Ingrese movimiento: ')

	if entrada == 'quit':      #metodo para salirse del juego
			break 
	elif entrada == 'undo':    #metodo para deshacer un movimiento del usuario
			board.pop()
	else:

		try:

			#Se revisa el lado que juega
			if side == 'N':
				entrada = chess.Move.null()
				
			#Ingresa el movimiento del usuario al engine
			movi = board.push_san(entrada) 
			result = engine.play(board, limit)

			#ingresa el movimiento del engine a tablero de la libreria
			board.push(result.move)
		except:

			print('Movimiento invalido. Por favor ingrese un movimiento valido')
			continue

	#Imprime los resultados
	print(result)
	print(result.move)
	print(board)  
	print(board.fen())

engine.quit() 
print('Juego Finalizado')

