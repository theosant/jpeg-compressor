Trabalho desenvolvido para a disciplina SCC0261 - Multimídia 

Ministrada pelo professor Rudinei Goularte

# Objetivo
O Objetivo do projeto é desenvolver um compressor e descompressor de imagens do tipo jpeg do zero utilizando a linguagem C.

Para compilar 
make release

para executar 
./bin/compressor -lossy entrada.bmp saida.bin
ou
./bin/compressor -lossless entrada.bmp saida.bin

para executar o descompressor
./bin/descompressor -lossy saida.bin imagem_rec.bmp
ou
./bin/descompressor -lossless saida.bin imagem_rec.bmp
