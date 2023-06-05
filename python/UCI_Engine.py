import chess
import chess.engine
import chess.svg
import pygame as pg
import time
import re

tiempoExtra = 0;

def funcionesEspeciales (move,jugada):
	#a esta funcion le ingresamos la jugada tipo 'Move'
	#del engine o la jugada que le ingresa el usuario 
	#de tipo str y ademas la jugada en UCI de ambos
	try:
		jugadaSan = board.san(move)

	except:
		jugadaSan = move

	print('\nLa jugada en notacion alegraica fue: ',jugadaSan)

	#Revisamos si hubo enroque y en caso tal, 
	#se le añade una '& o @' para luego ser procesada
	#en el STM
	if jugadaSan == 'O-O':
		print('enroque corto')
		jugadaUCI = jugada + '&'
		tiempoExtra = 1

	elif jugadaSan == 'O-O-O':
		print('enroque largo')
		jugadaUCI = jugada + '@'
		tiempoExtra = 1 

	else:

		#Revisamos si hubo captura y en caso tal, 
		#se le añade una 'x' para luego ser procesada
		#en el STM
		if re.search('x',jugadaSan):
			print('captura')
			jugadaUCI = jugada + 'x'
			tiempoExtra = 1
	
		else:
			jugadaUCI = jugada

	return jugadaUCI


#hacemos que el programa se ejecute siempre
while True:

	print("INFORMACION IMPORTANTE \n/////////////////////////////////////////")

	print('''\nPara jugar con este dispositivo es indispensable que ingrese sus jugada en la notacion algebraica de ajedrez (SAN)\npor lo cual, en esta introduccion, se le va a dar un breve resumen de esta notacion: ''')

	print('''\n* Todas las piezas son representadas por una letra en ¡mayuscula! proveniente de su nombre en ingles -> Peon = P,\n\tTorre = R, Caballo = N, Alfil = B, Reina = Q y Rey = K ''')

	print('''\n* Todas la jugadas inician con la letra que representa la pieza que se a mover, seguido de la casilla hacia donde esta se va a mover\n\ta modo de ejemplo, suponga que le interesa mover el caballo a la casilla f3, entonces escribiria: 'Nf3\'''')

	print('''\n* Esto ultimo no aplica para los peones, donde solo basta con colocar la casilla hacia donde se mueve\n\tpor ejemplo, un peon que se mueve hacia la casilla e4, bastara solo con escribir 'e4\'''')

	print('''\n* Si existe ambigüedad en una jugada, es decir, si dos piezas iguales pueden llegar a la misma casilla,entonces, se coloca la letra\n\tde la columna donde la pieza se ubica originalmente en medio de la letra que representa la pieza y la casilla hacia donde esta\n\tse mueve, por ejemplo, suponga que en la fila 1 solo estan las dos torres blancas en sus posiciones normales y se quiere que\n\tla torre en la columna 'a' llegue a la casilla d1; como ambas torres pueden acceder a esa casilla, para eliminar la ambigüedad,\n\tla jugada se escribiria: 'Rad1\'''')
	
	print('''\n* Si se va a capturar una pieza, se coloca una x entre la letra de la pieza y la casilla hacia donde se mueve, por ejemplo,\n\tun alfil que captura en b5: sBxb5. Para el caso de los peones, siempre se debe indicar la columna en la cual estan originalmente,\n\tpor ejemplo, un peon en e4 que captura en d5: exd5. Esta logica aplica tambien para jugada ambigüas, por ejemplo,\n\tel caso anterior de las torres pero con captura en d1: Raxd1.\n\n\tA este item hay que prestarle especial atencion ya que si no se indica esa 'x' no se ejecutará correctamente la funcion de captura en el tablero.''')
	
	print('''\n* El enroque corto (del lado el rey) se escribe O-O (las 'o' en mayuscula), mientras que el enroque largo (del lado de la reina) se escribe O-O-O.\n\n\tEsta notacion tambien es crucial escribirla bien para que se ejecute correctamente la funcion de enroque en el tablero''')

	print('''\n* Si se da una coronacion de peon, se debe indicar la casilla en la que corona, seguida de un signo '=' y la pieza por la cual se va a cambiar,\n\tpor ejemplo, un peon que se corona en e8 y se cambia por la reina seria: e8=Q. Cabe mencionar, que es deber del usuario reemplazar\n\tmanualmente el peon por la pieza seleccionada en la coronación.''')

	print('''\n* Los 'Jaque' no son necesario informarlos ya que el programa lo hace por si mismo''')

	print('''\n\nEso es todo lo que hay que tener presente respecto a la notacion algebraica, ¡Disfrute del juego!''')

	#Ingresa parametros
	input('\n\nPresione "ENTER" para iniciar')
	elo = input('Ingrese la dificultad en puntos Elo (500 - 1500): ')

	#Llama y configura al Engine
	engine = chess.engine.SimpleEngine.popen_uci('/usr/games/stockfish')
	engine.configure({'UCI_LimitStrength': elo})
	limit = chess.engine.Limit(time=0.1)

	#Seleccionamos el lado con el que queremos jugar
	side = input('Seleccione el color con el que quiere jugar (b/n): ')

	while (side != 'n') and (side != 'b'):
		side = input('Ingrese un valor valido (b/n): ')

	#configura el tablero 
	board = chess.Board()

	#Muestra el tablero inicial
	print(board) 

	chess.svg.board(board,size = 350)
	
	#Configuramos el primer movimiento segun el color que escogio el usuario
	if side == 'n':

		#Ingresa el movimiento del engine
		result = engine.play(board, limit)
		jugadaEngine = result.move.uci() #Convertimos la jugada a string

		#Revisamos si hubo captura o enroque
		revision = funcionesEspeciales(result.move,jugadaEngine)

		print(revision)

		#ingresa el movimiento del engine a tablero de la libreria
		board.push(result.move)

		#Imprime los resultados
		print('\nLas casillas que movió el engine fueron: ',result.move)
		print(board)  

		if tiempoExtra == 1:
			print(tiempoExtra)
			time.sleep(1)

		tiempoExtra =0

	#Inicia el juego
	while not board.is_game_over():
		entrada = input('\nIngrese movimiento: ')

		if entrada == 'quit':      #metodo para salirse del juego
				break 

		elif entrada == 'undo':    #metodo para deshacer un movimiento del usuario
			try:
				undoUCI = board.peek().uci()
			except:
				print('No hay jugadas previas')
				continue

	
			print("La jugada anterior en UCI fue: " ,undoUCI)

			#Regresamos el tablero a un estado anterior
			board.pop()
			if board.is_check():
				print('JAQUE')

			print(board)

			#Convertimos la jugada hecha en notacion san
			undoMove = chess.Move.from_uci(undoUCI)

			jugadaAnterior = undoUCI[2:4] + undoUCI[:2] 


			#Revisamos si la jugada es de captura o enroque y configuramos
			revision = funcionesEspeciales(undoMove,jugadaAnterior)

			print(revision + '-')


			if tiempoExtra == 1:
				print(tiempoExtra)
				time.sleep(1)

			tiempoExtra =0
			
			
		elif entrada == 'redo':	#metodo para rehacer un movimiento del usuario

			try:
				redoUCI = undoUCI

				#Convertimos la jugada hecha en notacion san
				redoMove = board.san(chess.Move.from_uci(redoUCI))

				#Ingresamos la jugada al tablero
				board.push(chess.Move.from_uci(redoUCI))

				print("La jugada que se va a rehacer es: ",redoUCI)
				if board.is_check(): 
					print('JAQUE')

				print(board)

				#Revisamos si la jugada es de captura o enroque
				revision = funcionesEspeciales(redoMove,redoUCI)

				print(revision)


				if tiempoExtra == 1:
					print(tiempoExtra)
					time.sleep(1)

				tiempoExtra =0

			except:
				print('\nNo es posible rehacer el movimiento')
				continue

		else:

			try:

				#Ingresa el movimiento del usuario al engine
				movimientoUsr = board.push_san(entrada) 
				jugadaUsuario = movimientoUsr.uci()
				result = engine.play(board, limit)
			except:

				print('Movimiento invalido. Por favor ingrese un movimiento valido')
				continue

			#Revisamos si la jugada es de captura o enroque
			revision = funcionesEspeciales(entrada,jugadaUsuario)

			print(revision)


			if tiempoExtra == 1:
				print(tiempoExtra)
				time.sleep(1)

			tiempoExtra =0

			#Imprime los resultados del mov del usuario
			print('Su jugada en notacion UCI fue: ',jugadaUsuario)
			if board.is_check():
				print('JAQUE')
			print(board)


			#convertimos la jugada del engine a UCI
			jugadaEngine = result.move.uci()

			#Revisamos si la jugada es de captura o enroque
			revision = funcionesEspeciales(result.move,jugadaEngine)
			print(revision)

				
			if tiempoExtra == 1:
				print(tiempoExtra)
				time.sleep(1)

			tiempoExtra =0

			#ingresa el movimiento del engine a tablero de la libreria
			board.push(result.move)

			#Imprime los resultados del movimiento del engine
			print('La jugada del engine en notacion UCI fue: ',jugadaEngine)
			if board.is_check():
				print('JAQUE')
			print(board)  


	engine.quit() 

	if board.is_checkmate():
		print('Juego Finalizado por JAQUE MATE')
	else:
		print('Juego Finalizado')

	juego = input('¿Quiere volver a jugar?(y/n): ')

	while (juego != 'y') and (juego != 'n'):
		juego = input('Ingrese un valor valido (y/n): ')

	if juego == 'y':
		print('Se va a iniciar un nuevo juego')

		time.sleep(0.5)
		continue

	else:
		print('Se cierra el programa')
		break