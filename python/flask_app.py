from flask import Flask, render_template

app = Flask(__name__)

import chess
import chess.engine
import serial
import time
import re


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

def IngresoJugadas(jugadaIngresada):

	entrada = jugadaIngresada

	try:
		#Ingresa el movimiento del usuario al engine
		movimientoUsr = board.push_san(entrada) 
		jugadaUsuario = movimientoUsr.uci()
		result = engine.play(board, limit)

	except:
		print('Movimiento invalido. Por favor ingrese un movimiento valido')
		#continue

	#Revisamos si la jugada es de captura o enroque
	revision = funcionesEspeciales(entrada,jugadaUsuario)

	#Imprime los resultados del mov del usuario
	print('Su jugada en notacion UCI fue: ',jugadaUsuario)
	if board.is_check():
		print('JAQUE')
	print(board) 
	print(revision) 

	#Enviamos los datos al stm
	#EnviarDatos(revision)

	#Le damos cierto tiempo para que el tablero se mueva
	#DelayEjecucion(revision)


	#Obtenemos la jugada del engine en string
	try:
		jugadaEngine = result.move.uci()
		
	except:
		print("")

	#Revisamos si la jugada es de captura o enroque
	revision = funcionesEspeciales(result.move,jugadaEngine)

	#ingresa el movimiento del engine a tablero de la libreria
	board.push(result.move)

	#Imprime los resultados del movimiento del engine
	print('La jugada del engine en UCI fue: ',jugadaEngine)
	if board.is_check():
		print('JAQUE')
	print(board) 
	print(revision)

	#Enviamos los datos al stm
	#EnviarDatos(revision)

	#Le damos cierto tiempo para que el tablero se mueva
	#DelayEjecucion(revision)

	return jugadaEngine

#Seleccionamos la dificultad del engine
elo = input('Ingrese la dificultad en puntos Elo (500 - 1500): ')

while True:
	try:
		if (int(elo) > 1500) or (int(elo) < 500):
			elo = input('Ingrese un valor valido (500 - 1500): ')
		else:
			break
	except:
		elo = input('Ingrese un valor valido (500 - 1500): ')	


#Llama y configura al Engine
engine = chess.engine.SimpleEngine.popen_uci('/usr/games/stockfish')
engine.configure({'UCI_LimitStrength': int(elo)})
limit = chess.engine.Limit(time=0.1)	

#Seleccionamos el lado con el que queremos jugar
side = input('Seleccione el color con el que quiere jugar (b/n): ')

while (side != 'n') and (side != 'b'):
	side = input('Ingrese un valor valido (b/n): ')

#Le informamos al stm que el juego va a iniciar
#ser.write(b' ')

#configura el tablero 
board = chess.Board()

#Muestra el tablero inicial
print(board) 

#Configuramos el primer movimiento segun el color que escogio el usuario
if side == 'n':

	time.sleep(2)

	#Ingresa el movimiento del engine
	result = engine.play(board, limit)
	jugadaEngine = result.move.uci() #Convertimos la jugada a string


	#Revisamos si hubo captura o enroque
	#revision = funcionesEspeciales(result.move,jugadaEngine)

	#ingresa el movimiento del engine a tablero de la libreria
	board.push(result.move)

	#Imprime los resultados
	print('La jugada del engine en UCI fue: ',result.move)
	print(board)  

	#Enviamos la jugada al stm
	#EnviarDatos(jugadaEngine)

	#Le damos cierto tiempo para que el tablero se mueva
	#DelayEjecucion(revision)


@app.route('/')
def index():
    return render_template("index.html")


@app.route('/move/<int:depth>/<san>')
def get_move(depth, san):
    jugadaFinal = IngresoJugadas(san)
    print("La jugada del engine fue: ", jugadaFinal)
    return jugadaFinal


if __name__ == '__main__':
    app.run(debug=True, host = "0.0.0.0")
