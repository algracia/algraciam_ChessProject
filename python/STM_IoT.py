from flask import Flask, render_template

app = Flask(__name__)

import chess
import chess.engine
import serial
import time
import re

#Configuramos tooo lo relacionado con el puerto serial
ser = serial.Serial()
ser.port = '/dev/ttyACM0'
ser.baudrate = 115200
ser.open()
ser.timeout =3

#definimos funcion para mandar datos al STM
def EnviarDatos(string):

	if not ser.is_open:
		ser.open()

	dato = string + '$'
	ser.write(dato.encode('utf8','ignore'))

	i=0
	while i<= 4:
		msg = ser.readline().decode('utf8','ignore')
		print(msg)
		i +=1
	return

'''
Esta funcion nos permite añadir un simbolo especial
al string de la jugada en caso tal de que haya enroque 
o captura
'''
def funcionesEspeciales (move,jugada):
	#a esta funcion le ingresamos la jugada tipo 'Move'
	#del engine o la jugada que le ingresa el usuario 
	#de tipo str y ademas la jugada en UCI de ambos
	try:
		jugadaSan = board.san(move)

	except:
		jugadaSan = move

	print('\nLa jugada en notacion algebraica fue: ',jugadaSan)

	#Revisamos si hubo enroque y en caso tal, 
	#se le añade una '& o @' para luego ser procesada
	#en el STM
	if jugadaSan == 'O-O':
		print('enroque corto')
		jugadaUCI = jugada + '&'

	elif jugadaSan == 'O-O-O':
		print('enroque largo')
		jugadaUCI = jugada + '@'

	else:

		#Revisamos si hubo captura y en caso tal, 
		#se le añade una 'x' para luego ser procesada
		#en el STM
		if re.search('x',jugadaSan):
			print('captura')
			jugadaUCI = jugada + 'x'
	
		else:
			jugadaUCI = jugada


	return jugadaUCI


def DelayEjecucion (revision):

	orden = ser.read_until('*',2).decode('utf8','ignore')

	j=0
	while (orden != '*'):

		try:
			captura = revision[4]
		except:
			captura = '\n'

		if captura == 'x':
			if(j >= 6):
				break
		else:
			if(j >= 2):
				break

		orden = ser.read_until('*',2).decode('utf8','ignore')
		time.sleep(1)
		j+=1
		continue

	ser.close()	

def CalculoEngine(tablero,limiteEngine):

	#Ingresa el movimiento del engine
	result = engine.play(tablero, limiteEngine)
	jugadaSanEngine = board.san(result.move) #Convertimos la jugada a string

	#Esta funcion nos devuleve la jugada del engine ya en notacion algebraica
	return jugadaSanEngine



def IngresoJugadas(jugadaUsr):

	try:
		#Ingresa el movimiento del primer usuario al tablero
		movimientoUsr = board.push_san(jugadaUsr) 
		jugadaUsuario = movimientoUsr.uci()

	except:
		print('Movimiento invalido. Por favor ingrese un movimiento valido')
		return ""

	#Revisamos si la jugada es de captura o enroque
	revision = funcionesEspeciales(jugadaUsr,jugadaUsuario)

	#Imprime los resultados del mov del usuario
	print('Su jugada en notacion UCI fue: ',jugadaUsuario)
	if board.is_check():
		print('JAQUE')
	print(board) 
	print(revision)

	#Enviamos los datos al stm
	EnviarDatos(revision)

	#Le damos cierto tiempo para que el tablero se mueva
	DelayEjecucion(revision)

@app.route('/')
def index():
    return render_template("index.html")

#Ingresa parametros
@app.route('/config/<typeGameParam>/<colorParam>/<int:eloParam>')
def set_config(typeGameParam, colorParam, eloParam):
	global typeGame, color, engine, limit,board
	typeGame = typeGameParam 
	color = colorParam
	elo = eloParam

	#Llama y configura al Engine
	engine = chess.engine.SimpleEngine.popen_uci('/usr/games/stockfish')
	engine.configure({'UCI_LimitStrength': int(elo)})
	limit = chess.engine.Limit(time=0.1)

	#Le informamos al stm que el juego va a iniciar
	ser.write(b' ')

	#configura el tablero 
	board = chess.Board()

	#Muestra el tablero inicial
	print(board) 

	#Aplicamos un pequeño delay en el codigo
	time.sleep(2)

	if (typeGame == "Eng"):
		if (color == "N"):
			#Posterior a eso, recibimos la jugada el engine
			jugadaEng = CalculoEngine(board, limit)

			#Ahora, ingresamos la jugada del engine
			IngresoJugadas(jugadaEng)
			return jugadaEng
		
		else:
			return ""
	else:
		return ""


@app.route('/move/<san>')
def get_move(san):

	print("Si entra a la funcion")

	if(san == "UNDO"):

		global undoUCI

		try:
			undoUCI = board.peek().uci()
		except:
			print('No hay jugadas previas')
			return ""


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


		#Enviamos los datos al stm
		EnviarDatos(revision + '-')

		#Le damos cierto tiempo para que el tablero se mueva
		DelayEjecucion(revision)

		return ""


	elif (san == "REDO"):
		print(undoUCI)
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

			EnviarDatos(revision)

			#Le damos cierto tiempo para que el tablero se mueva
			DelayEjecucion(revision)

			return redoMove

		except:
			print('\nNo es posible rehacer el movimiento')
			return ""


	else:

		if (typeGame == "Eng"):
			#Primero enviamos la jugada del usuario al STM
			IngresoJugadas(san)

			#Posterior a eso, recivimos la jugada el engine
			jugadaEng = CalculoEngine(board,limit)

			#Ahora, ingresamos la jugada del engine
			IngresoJugadas(jugadaEng)

			if board.is_game_over():
				engine.quit() 
				ser.close()
				print("\nFin del juego")

			print(jugadaEng)	
			return jugadaEng
		
		else:
			#enviamos la jugada de cada usuario al STM
			IngresoJugadas(san)

			if board.is_game_over():
				engine.quit() 
				ser.close()
				print("\nFin del juego")
				
			return ""


if __name__ == '__main__':
    app.run(debug=True, host = "0.0.0.0")
