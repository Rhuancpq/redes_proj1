# Ex 2 - TCP Bidirecional

## Como executar

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

## Limitações

As mensagens são trocadas simultaneamente, pode aparecer de forma desorganizada na tela.