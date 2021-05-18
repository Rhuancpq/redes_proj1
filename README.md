# Projeto de Redes

Integrantes:
|Nome| Matrícula|
|--|:-:|
| Igor Batista Paiva | 18/0018728 |
| Rhuan Carlos Pereira de Queiroz | 18/0054848 |

## Ex 1 - UDP Bidirecional

### Como executar

```
gcc main.c -o bin

./bin <ip servidor origem> <porta servidor origem> <ip servidor destino> <porta servidor destino>
```

## Ex 2 - TCP Bidirecional

### Como executar

Cliente:

```
gcc Cliente.c -o cliente

./cliente <ip servidor destino> <porta servidor destino>
```

Servidor:

```
gcc Servidor.c -o servidor

./servidor <ip origem> <porta em que vai escutar>
```

## Ex 3 - TCP Half-duplex

### Como executar

Cliente:

```
gcc Cliente.c -lpthread -o cliente

./cliente <ip servidor> <porta servidor>
```

Servidor:

```
gcc Servidor.c -lpthread -o servidor

./servidor <ip servidor> <porta servidor>
```

## Ex 3 - TCP Full-duplex

Para que fique melhor organizado a recepção de mensagens nos Clientes é preciso pressionar enter sempre que quiser verificar se há novas mensagens.

Essa decisão foi tomada pois vimos que utilizar o stdin e o stdout ao mesmo tempo atrapalhava a compreensão das mensagens. Utilizamos semáforos para contornar essa situação, mas nada que seja muito específico.

### Como executar

Cliente:

```
gcc Cliente.c -lpthread -o cliente

./cliente <ip servidor> <porta servidor>
```

Servidor:

```
gcc Servidor.c -lpthread -o servidor

./servidor <ip servidor> <porta servidor>
```
