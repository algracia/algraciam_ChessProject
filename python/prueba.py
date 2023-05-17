import re

notacion = input('Ingrese la notacion fen: ')

def Captura(fen):
	info= re.findall("[^ ]+",fen)
	filas = info[0].split('/')
	print(filas)

Captura(notacion)