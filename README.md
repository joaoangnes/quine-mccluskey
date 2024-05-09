# README

## Método de Quine McCluskey

O projeto implementa o método Quine McCluskey para minimizar funções booleanas. Este método é usado para simplificar expressões booleanas encontrando o conjunto mínimo de implícitos primos que cobrem todos os mintermos da função.

O método de Quine McCluskey funciona comparando pares de mintermos e encontrando aqueles que diferem apenas por um bit. Estes pares são combinados para formar novos termos, e o processo é repetido até que não seja possível efetuar mais combinações. Os termos resultantes são então utilizados para criar uma expressão booleana mínima.

Os passos envolvidos no método de Quine McCluskey são os seguintes:

1. Converter os valores de entrada em representação binária.
2. Agrupar os valores com base no número de 1s na sua representação binária.
3. Efetuar a diferenciação entre pares de valores para encontrar aqueles que diferem apenas por um bit.
4. Marcar os valores que passam nos filtros de diferenciação como cobertos.
5. Remova quaisquer valores duplicados da lista.
6. Criar uma tabela final para verificar se existem entradas não cobertas e gerar o circuito mínimo.

No final do processo, o projeto devolverá o circuito mínimo ao utilizador.

## Check-List

- [x] - Receber o array de unsigned char e percorrer os valores;
- [x] - Criar uma função para converter um valor unsigned char e retornar seu binário;
- [x] - Criar uma função para contar numero de bits 1 de um valor unsigned char;
- [x] - Verificar qual estrutura de dados utilizar para armazenar esses agrupamentos (Lista Encadeada);
- [x] - Criar uma função que recebe um array de valores e agrupa eles pela sua quantidade de bits 1;
- [x] - Criar uma função para realizar o diferimento entre dois valores unsigned char e verificar se eles diferem de apenas 1 bit, lembrando que tem que ser o mesmo. Deve ser considerado também sua mascara que irá retornar qual é o bit insignificante para o diferimento;
- [x] - Quando realizar o diferimento, terá que retornar também uma máscara de bit insignificante;
- [x] - Caso ele realmente passe pelos filtros, marcar os valores dos dois valores como cobertos na lista;
- [x] - Criar uma função para verificar os valores repetidos dentro de sua lista, caso obtenha retire um deles e continue o processo;
- [x] - Criar função para realizar a verificar na tabela final onde irá realizar a verificação das entradas que não foram cobertas ainda, para retonar seu circuito minimo;
- [x] - Varrer a lista final encontrada e criar uma matriz com os valores de combinações encontrados;
- [x] - Retornar no final o seu circuito minimo para o usuário;
