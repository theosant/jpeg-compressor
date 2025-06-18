# Compressor de Imagens em C (Padrão JPEG)

**Trabalho desenvolvido para a disciplina SCC0261 - Multimídia**  
**Instituto de Ciências Matemáticas e de Computação (ICMC) - USP São Carlos**  
**Professor:** Dr. Rudinei Goularte

---

**Autor:**  
**NUSP:** 
**Autor:**  Théo da Mota dos Santos
**NUSP:**  10691331

---

## 🎯 Objetivo

O objetivo deste projeto é o desenvolvimento, utilizando a linguagem C, de um compressor e descompressor de imagens que implementa as principais etapas do padrão JPEG. O sistema suporta tanto um modo de compressão com perdas (`lossy`), que segue o pipeline JPEG clássico, quanto um modo sem perdas (`lossless`), para fins comparativos.

## ✨ Funcionalidades

- Compressão de imagens `.bmp` (24 bits) para um formato binário customizado.
- **Modo com perdas (`-lossy`)**: Implementa DCT, quantização e codificação de entropia.
- **Modo sem perdas (`-lossless`)**: Implementa codificação preditiva (DPCM).
- Descompressão dos formatos customizados para imagens `.bmp` visualizáveis.

## 🛠️ Tecnologias Utilizadas

- **Linguagem:** C (padrão C99)  
- **Compilador:** GCC  
- **Automação de Build:** Make

## 🚀 Como Compilar e Executar

### Pré-requisitos

Certifique-se de ter `gcc` e `make` instalados em seu sistema.

### Compilação

Na pasta raiz do projeto, execute:

```bash
make release
```

Os executáveis serão gerados na pasta `./bin/`.

### Execução

#### Compressor

**Modo com perdas (`-lossy`)**  
```bash
./bin/compressor -lossy <caminho_imagem_entrada.bmp> <caminho_arquivo_saida.jpc>
```
**Exemplo:**  
```bash
./bin/compressor -lossy test_images/paisagem_32x32.bmp saida_lossy.jpc
```

**Modo sem perdas (`-lossless`)**  
```bash
./bin/compressor -lossless <caminho_imagem_entrada.bmp> <caminho_arquivo_saida.jpc>
```
**Exemplo:**  
```bash
./bin/compressor -lossless test_images/paisagem_32x32.bmp saida_lossless.jpc
```

#### Descompressor

**Modo com perdas (`-lossy`)**  
```bash
./bin/descompressor -lossy <caminho_arquivo_comprimido.jpc> <caminho_imagem_saida.bmp>
```
**Exemplo:**  
```bash
./bin/descompressor -lossy saida_lossy.jpc imagem_reconstruida.bmp
```

**Modo sem perdas (`-lossless`)**  
```bash
./bin/descompressor -lossless <caminho_arquivo_comprimido.jpc> <caminho_imagem_saida.bmp>
```
**Exemplo:**  
```bash
./bin/descompressor -lossless saida_lossless.jpc imagem_reconstruida.bmp
```

## ⚙️ Pipeline de Compressão

O processo de compressão é dividido em dois pipelines principais, conforme o modo escolhido.

### Pipeline com Perdas (`-lossy`)

Este pipeline segue as etapas fundamentais do padrão JPEG:

1. **Leitura do arquivo BMP**: A imagem de entrada (24 bits) é carregada.
2. **Conversão de espaço de cor**: De RGB para YCbCr (luminância e crominância).
3. **Subamostragem de croma**: Aplicação do esquema 4:2:0 nos canais Cb e Cr.
4. **Transformada Discreta de Cosseno (DCT)**: Blocos 8×8 são transformados para o domínio da frequência.
5. **Quantização**: Coeficientes da DCT são divididos por uma matriz de quantização e arredondados (perda de dados).
6. **Codificação de entropia**:
   - **DC**: Codificação DPCM.
   - **AC**: Codificação em zigue-zague + RLE (Run-Length Encoding).
7. **Escrita do arquivo**: Um arquivo binário customizado é gerado.

### Pipeline Sem Perdas (`-lossless`)

Neste modo, há reconstrução perfeita da imagem original:

1. **Leitura e conversão para YCbCr**.
2. **Codificação preditiva (DPCM)**: Armazena diferenças entre pixels sucessivos.
3. **Escrita do arquivo**: As diferenças são armazenadas no arquivo binário.

## 📂 Estrutura do Projeto

```
/
├── include/         # Arquivos de cabeçalho (.h)
├── src/             # Código-fonte (.c)
├── test_images/     # Imagens de teste (formato BMP)
├── bin/             # Executáveis e arquivos gerados
├── makefile         # Script de automação da compilação
└── README.md        # Este arquivo de documentação
```

