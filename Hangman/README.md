# Jogo da Forca - Servidor Cliente

Implementação de um jogo de forca com a comunicação de cliente servidor utilizando protocolo TCP.

## Primeiros Passos

Estas instruções deixarão o projeto pronto para uso em seu ambiente.

### Pré-Requisitos

```
Um sistema Linux.
gcc versão 8.2.1
libncurses5-dev
libncursesw5-dev
```

### Instalação

Para compilar todos programas utilize o Makefile pela linha:

```
make
```

Utilize o “make cliente” ou “make servidor” para compilar cada programa em específico.

Caso deseje limpar a área de trabalho utilize o comando:

```
make clean
```

## Execução

Para executar a aplicação é necessário a utilização da aplicação é necessário duas ou mais janelas de terminais podendo estar em máquinas diferentes. Uma janela precisará executar o programa do servidor e as demais janelas utilizaram o programa do cliente

Para a execução do servidor utilize o comando:
```
./servidor <PORTA>
```

Para a execução do cliente utilize o comando:

```
./cliente <IP DO SERVIDOR> <PORTA DO SERVIDOR>
```

## Versionamento
Nós usamos git para controle de versão


## Autores
* **Daniel Helú**
* **Gustavo Fernandez**
