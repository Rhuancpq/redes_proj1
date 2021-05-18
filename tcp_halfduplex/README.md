# Ex 3 - TCP Half-duplex

## Como executar

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

## Observações

Algumas observações sobre os diálogos entre:

- Pela ordem em que o servidor recebeu a mensagem
- O diálogo com o próximo cliente só é redefinido quando a mensagem para o atual for enviada ou o comando `PROX` for digitado
- O diálogo começa sempre a partir do cliente, sendo assim o cliente quem envia a primeira mensagem
- O diálogo só pode ser encerrado pelo cliente utilizando o comando `FIM`, entretanto o servidor permanece ativo
- Caso os clientes não enviem nenhuma mensagem o servidor estará a espera das mensagens
- Sempre que uma nova mensagem for recebida o servidor entrará em modo de resposta para aquele cliente, contanto que o servidor não esteja em modo de resposta para um determinado cliente.
- Sempre que uma mensagem for enviada por parte do servidor ou o comando `PROX` for utilizado, o servidor estará passível a informar sobre novas conexões e mensagens recebidas enquanto estava digitando.
- O ciclo de mensagens do servidor é: receber uma mensagem do cliente, enviar mensagem para cliente. Entre um ciclo e outro novos ciclos podem ser iniciados pela ordem de recebimento de mensagens ou novas conexões.
