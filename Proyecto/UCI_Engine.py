import chess
import chess.engine

#Llama y configura al Engine
engine = chess.engine.SimpleEngine.popen_uci('/usr/games/stockfish')

#Ingresa parametros
input('Presione "ENTER" para iniciar')
side = input('Seleccione el color con el que quiere jugar (b/n): ')
while (side != 'n') and (side != 'b'):
	side = input('Ingrese un valor valido (b/n): ')


#Configura los parametros
engine.configure({'UCI_LimitStrength': 900})
limit = chess.engine.Limit(time=0.1)

#configura el tablero 
board = chess.Board()

#Muestra el tablero inicial
print(board) 
print(board.fen())

#Configuramos el primer movimiento segun el color que escogio el usuario
if side == 'n':

	#Ingresa el movimiento del engine
	result = engine.play(board, limit)

	#ingresa el movimiento del engine a tablero de la libreria
	board.push(result.move)

	#Imprime los resultados
	print(result)
	print(result.move)
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
	print(board.is_capture(result.move))

engine.quit() 
print('Juego Finalizado')
