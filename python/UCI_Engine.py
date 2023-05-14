import chess
import chess.engine
import serial
import time

#Configuramos tooo lo relacionado con el puerto serial
ser = serial.Serial()
ser.port = '/dev/ttyACM0'
ser.baudrate = 115200
ser.open()
ser.timeout =5

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

#Llama y configura al Engine
engine = chess.engine.SimpleEngine.popen_uci('/usr/games/stockfish')
engine.configure({'UCI_LimitStrength': 900})
limit = chess.engine.Limit(time=0.1)

#Ingresa parametros
input('Presione "ENTER" para iniciar')
side = input('Seleccione el color con el que quiere jugar (b/n): ')

while (side != 'n') and (side != 'b'):
	side = input('Ingrese un valor valido (b/n): ')

#configura el tablero 
board = chess.Board()

#Le informamos al stm que el juego va a iniciar
ser.write(b' ')

#Muestra el tablero inicial
print(board) 
print(board.fen())
#Configuramos el primer movimiento segun el color que escogio el usuario
if side == 'n':

	#Ingresa el movimiento del engine
	result = engine.play(board, limit)
	jugadaEngine = result.move.uci() #Convertimos la jugada a string

	#ingresa el movimiento del engine a tablero de la libreria
	board.push(result.move)

	#Imprime los resultados
	print('\nLas casillas que movió el engine fueron: ',result.move)
	print(board)  
	print(board.fen())

	EnviarDatos(jugadaEngine)

#Inicia el juego
while not board.is_game_over():
	entrada = input('\nIngrese movimiento: ')

	if entrada == 'quit':      #metodo para salirse del juego
			break 
	elif entrada == 'undo':    #metodo para deshacer un movimiento del usuario
		print("La jugada anterior fue: " ,board.peek())
		board.pop()
		'''
		#Ingresa el movimiento anteriro al engine
		result = engine.play(board, limit)

		#ingresa el movimiento del engine a tablero de la libreria
		board.push(result.move)'''

	else:

		try:

			#Ingresa el movimiento del usuario al engine
			movi = board.push_san(entrada) 
			result = engine.play(board, limit)
		except:

			print('Movimiento invalido. Por favor ingrese un movimiento valido')
			continue

	#Imprime los resultados del mov del usuario
	#print('\nSu jugada fue: ',movi)
	jugadaUsuario = movi.uci()
	print('Es jaque?: ',board.is_check())
	print(board)  
	print(board.fen())

	#Enviamos la jugada del usuario al stm
	EnviarDatos(jugadaUsuario)

	#ingresa el movimiento del engine a tablero de la libreria
	time.sleep(0.5)
	board.push(result.move)

	#Imprime los resultados del movimiento del engine
	jugadaEngine = result.move.uci()
	print('\nLas casillas que movió el engine fueron: ',result.move)
	print('Es jaque?: ',board.is_check())
	print(board)  
	print(board.fen())

	#Enviamos la jugada del engine al stm
	EnviarDatos(jugadaEngine)

engine.quit() 
print('Juego Finalizado')
ser.close()
