# Ex 1 - UDP Bidirecional

## Como executar

```
gcc main.c -o bin

./bin <ip servidor origem> <porta servidor origem> <ip servidor destino> <porta servidor destino>
```

## Observações

Por utilizar o protocolo UDP, qualquer programa serve como cliente e servidor ao mesmo tempo.

## Limitações

As mensagens são trocadas simultaneamente, pode aparecer de forma desorganizada na tela.
