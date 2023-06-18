from flask import Flask, render_template

app = Flask(__name__)

import chess
import chess.engine
import serial
import time
import re

# #Configuramos tooo lo relacionado con el puerto serial
# ser = serial.Serial()
# ser.port = '/dev/ttyACM0'
# ser.baudrate = 115200
# ser.open()
# ser.timeout =3

# #definimos funcion para mandar datos al STM
# def EnviarDatos(string):

# 	if not ser.is_open:
# 		ser.open()

# 	dato = string + '$'
# 	ser.write(dato.encode('utf8','ignore'))

# 	i=0
# 	while i<= 4:
# 		msg = ser.readline().decode('utf8','ignore')
# 		print(msg)
# 		i +=1
# 	return

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


# def DelayEjecucion (revision):

# 	orden = ser.read_until('*',2).decode('utf8','ignore')

# 	j=0
# 	while (orden != '*'):

# 		try:
# 			captura = revision[4]
# 		except:
# 			captura = '\n'

# 		if captura == 'x':
# 			if(j >= 6):
# 				break
# 		else:
# 			if(j >= 2):
# 				break

# 		orden = ser.read_until('*',2).decode('utf8','ignore')
# 		time.sleep(1)
# 		j+=1
# 		continue

# 	ser.close()	

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
		return

	#Revisamos si la jugada es de captura o enroque
	revision = funcionesEspeciales(jugadaUsr,jugadaUsuario)

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



#Ingresa parametros
input('\nPresione "ENTER" para iniciar')
	
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

# #Le informamos al stm que el juego va a iniciar
# ser.write(b' ')

#configura el tablero 
board = chess.Board()

#Muestra el tablero inicial
print(board) 

#Configuramos el primer movimiento segun el color que escogio el usuario
if side == 'n':

	#En este caso, si el usuario 1 (el del servidor) seleccionó negras, entonces,
	#la primera jugada sera por parte del segundo usuario.

	#De momento, el segundo usuario será el engine

	time.sleep(2)

	#Le pedimos al engine que nos devuelva la jugada
	jugadaEng = CalculoEngine(board,limit)

	#Enviamos la juagada al STM
	IngresoJugadas(jugadaEng)

@app.route('/')
def index():
    return render_template("index.html")


@app.route('/move/<int:depth>/<san>')
def get_move(depth,san):
    
	#Primero enviamos la jugada del usuario al STM
	#IngresoJugadas(san)

	#Posterior a eso, recivimos la jugada el engine
	jugadaEng = CalculoEngine(board,limit)

	#Ahora, ingresamos la jugada del engine
	#IngresoJugadas(jugadaEng)

	if board.is_game_over():
		engine.quit() 
		#ser.close()
    
	else:
		return jugadaEng


if __name__ == '__main__':
    app.run(debug=True, host = "0.0.0.0")
