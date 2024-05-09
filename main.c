#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <limits.h>

int numBytes;
char alphabet[26];
char invertAlphabet[26];

// Estrutura para representar um valor
typedef struct Value {
    unsigned char value; // O próprio valor
    unsigned char mask;  // Máscara associada ao valor
    bool covered;        // Flag indicando se o valor já foi coberto
    struct Value *next;  // Ponteiro para o próximo valor na lista
    unsigned char *combination;  // Combinação de valores comparados
    size_t combinationSize; // Tamanho da combinação
} Value;

// Estrutura para representar um grupo de valores
typedef struct ValueGroup {
    unsigned char numBits; // Quantidade de bits 1
    Value* values;         // Lista de valores do grupo
    size_t numValues;      // Número de valores no grupo
    struct ValueGroup *next; // Ponteiro para o próximo grupo
} ValueGroup;

// Estrutura para representar os valores dos primos implicantes essencais
typedef struct PrimeInfo {
  int* values; // Valores dos primos implicantes essenciais
  int* masks;  // Máscaras dos primos implicantes essenciais
  int size;    // Tamanho dos primos implicantes essenciais
} PrimeInfo;

// Initilize the alphabet
void initializeAlphabet() {
  for (int i = 0; i < 26; i++) {
    alphabet[i] = 'A' + i;
    invertAlphabet[i] = alphabet[i];
  }
}

// Função que conta a quantidade de bytes necesitados para representar um valor em binário
int numBytesRequired(unsigned char num) {
  if (num == 0) {
    return 1;
  } else {
    return (int)ceil(log2((double)num + 1));
  }
}

// Função que converte um valor unsigned char para binário
char *charToBinary(unsigned char valor) {
  int numBits = numBytes;
  char *binario = malloc((numBits + 1) * sizeof(char)); // Aloca espaço para a string binária (numBits bits + terminador '\0')
  if (binario == NULL) {
      printf("Erro ao alocar memória.\n");
      exit(EXIT_FAILURE);
  }
  for (int i = numBits - 1; i >= 0; i--) {
      unsigned int bit = (valor >> i) & 1;
      binario[numBits - 1 - i] = bit + '0'; // Converte o bit em char '0' ou '1' e atribui à string binária
  }
  binario[numBits] = '\0'; // Adiciona o terminador de string
  return binario;
}

// Função para contar e mostrar os bits diferentes entre dois valores, considerando uma máscara de bits insignificantes
// Retorno:
// - int -> Retorna a quantidade de bits diferentes entre os valores
// - unsigned char *diferenca -> Retorna a mascara resultante para desconsiderar bits insignificantes
int verificarBitsDiferentes(unsigned char valor1, unsigned char valor2, unsigned char mascara1, unsigned char mascara2, unsigned char *mascara_output) {
  unsigned char diferenca = valor1 ^ valor2; // Calcula a diferença utilizando o operador XOR (^)  
  int bitsDiferentes = 0; // Inicializa o contador de bits diferentes

  // Caso tenha sido informado as duas máscaras, realiza a operação de OR entre as duas máscaras com a diferenca e retorna a mascara resultante
  if (mascara1 != 0xFF && mascara2 != 0xFF) {
    // Realiza um OR entre as máscaras para obter a máscara de saída
    // E depois realiza mais um OR, com o XOR dos dois valores, assim eu consigo retornar a mascara resultante dos dois valores e suas mascaras
    *mascara_output = (mascara1 | mascara2) | diferenca; 

    // Caso a mascara seja difernete, já aborta a verifação, pois não tem como valor ter diferença de apenas 1 bit
    if (mascara1 != mascara2){
      // printf("\nMascaras com diferença maior que 1 bit");
      return INT_MAX; // Retorna um valor que não é 1, para não ser considerado como verdadeiro
    }
      
    // Loop para percorrer os bits da diferença e contar os bits diferentes
    for (int i = 0; i < 8; i++) {
      if ( ((diferenca >> i) & 1 ) && (!((mascara1 >> i) & 1) || !((mascara2 >> i) & 1)) ){
        bitsDiferentes++; // Incrementa o contador se o bit for diferente
        // printf("Bit %d é diferente\n", i);
      }
    }
  } else { // Caso contrario apenas retorna a sua diferenca
    *mascara_output = diferenca; 

    // Loop para percorrer os bits da diferença e contar os bits diferentes
    for (int i = 0; i < 8; i++) {
      unsigned char bit = (diferenca >> i) & 1; // Obtém o bit na posição i da diferença
      if (bit == 1) {
          bitsDiferentes++; // Incrementa o contador se o bit for diferente
          // printf("Bit %d é diferente\n", i);
      }
    }
  }

  // printf("\nBits diferentes entre %hhu e %hhu (mascara output: %s):\n", valor1, valor2, charToBinary(*mascara_output));
  // printf("\nValor 1: %hhu (%s)  Mascara 1: %hhu (%s)", valor1, charToBinary(valor1), mascara1, charToBinary(mascara1));
  // printf("\nValor 2: %hhu (%s)  Mascara 2: %hhu (%s)", valor2, charToBinary(valor2), mascara2, charToBinary(mascara2));
  // printf("\nTotal de bits diferentes: %d\n", bitsDiferentes);

  // Verifica se a quantidade de bits diferentes é igual a 1
  // return bitsDiferentes == 1 ? true : false;
  return bitsDiferentes;
}

// Função que conta a quantidade de bits 1 em um valor unsigned char
int countBits(unsigned char value, unsigned char mask) {
  unsigned int count = 0;
  unsigned char maskedValue = value;

  if (mask != 0xFF) {
    maskedValue &= ~mask; // Aplica a mascara para ignorar os bits insignificantes
  }

  // Loop para contar os bits 1
  while (maskedValue) {
    // Utiliza a operação de AND para verificar se o bit é 1
    // E decrementa o valor para verificar o próximo bit
    // E incrementa o contador
    maskedValue &= maskedValue - 1;
    count++;
  }
  return count;
  // return __builtin_popcount(value);
}

// Função para criar um novo valor com máscara, cobertura e combinação dinâmica
Value *createValue(unsigned char value, unsigned char mask, unsigned char *combination, size_t size) {
    Value *newValue = malloc(sizeof(Value));
    if (newValue == NULL) {
        fprintf(stderr, "Erro ao alocar memória para novo valor.\n");
        exit(EXIT_FAILURE);
    }
    newValue->value = value;
    newValue->mask = mask;
    newValue->covered = false;
    newValue->next = NULL;
    newValue->combinationSize = size;
    newValue->combination = malloc(size * sizeof(unsigned char));
    if (newValue->combination == NULL) {
        fprintf(stderr, "Erro ao alocar memória para combinação de valores.\n");
        exit(EXIT_FAILURE);
    }
    memcpy(newValue->combination, combination, size * sizeof(unsigned char)); // Copia a combinação para o novo valor
    return newValue;
}

// Função para criar um novo grupo com um valor inicial
ValueGroup *createValueGroup(unsigned char value, unsigned char mask, unsigned char *combination, size_t size) {
  ValueGroup *newGroup = malloc(sizeof(ValueGroup));
  if (newGroup == NULL) {
      printf("Erro ao alocar memória.\n");
      exit(EXIT_FAILURE);
  }
  newGroup->numBits = countBits(value, mask);
  newGroup->numValues = 1;
  newGroup->values = malloc(sizeof(Value));
  if (newGroup->values == NULL) {
      printf("Erro ao alocar memória.\n");
      exit(EXIT_FAILURE);
  }
  newGroup->values = createValue(value, mask, combination, size); // Inicializa o valor com máscara padrão e não coberto
  newGroup->next = NULL;
  return newGroup;
}

// Função para adicionar um valor a um grupo existente
void addToValueGroup(ValueGroup *group, unsigned char value, unsigned char mask, unsigned char *combination, size_t size) {
  Value *newValue = createValue(value, mask, combination, size); // Cria um novo valor 
  Value *currentValue = group->values; // Aponta para o primeiro valor da lista de valores

  // Percorre até o último valor da lista
  while (currentValue->next != NULL) { 
    currentValue = currentValue->next;
  }

  currentValue->next = newValue; // Adiciona o valor informado no final da lista 
  group->numValues++; // Atualiza o número de valores no grupo
}

// Função para liberar a memória alocada para os valores de um grupo
void freeValues(Value *values) {
    while (values != NULL) {
        Value *temp = values;
        values = values->next;
        free(temp);
    }
}

// Função para liberar a memória alocada para os grupos
void freeValueGroups(ValueGroup *head) {
  ValueGroup *current = head;
  while (current != NULL) {
      ValueGroup *temp = current;
      current = current->next;
      freeValues(temp->values); // Libera os valores do grupo
      free(temp);
  }
}

// Função responsável pelo primeiro agrupamento dos valores em grupos de acordo com a quantidade de bits 1
ValueGroup *initializeListValue(unsigned char v[], int n) {
  // Cria uma Lista Encadeada de Grupos de Valores nova
  ValueGroup *head = NULL;

  // Percorre os valores e agrupa de acordo com a quantidade de bits 1
  for (int i = 0; i < n; i++) {
    int bits = countBits(v[i], 0XFF); // Busca a quantidade de bits 1 do valor corrente
    ValueGroup *current = head; // Aponta para o início da lista
    ValueGroup *prev = NULL; // Aponta para o grupo anterior

    // Percorre a lista de grupos para encontrar um grupo com a mesma quantidade de bits 1
    while (current != NULL && current->numBits != bits) {  
      prev = current; // Coloca o ponteiro prev no grupo corrente verificado
      // Avança para o próximo grupo, caso seja null, não há grupo com a mesma quantidade de bits 1
      // Caso tenha encontrado um grupo com a mesma quantidade de bits 1, o loop é interrompido
      current = current->next; 
    }

    // Se não encontrou um grupo com a mesma quantidade de bits 1, cria um novo grupo
    if (current == NULL) {
        ValueGroup *newGroup = createValueGroup(v[i], 0xFF, &v[i], 1); // Chama função para criar um novo grupo de quantidade de bits 1
        if (prev == NULL) { // Caso tenha sido o primeiro grupo, aponta o head para o novo grupo
          head = newGroup;
        } else { // Caso não tenha sido o primeiro grupo, aponta o ponteiro next do grupo anterior para o novo grupo
          prev->next = newGroup;
        }
    } else { // Se encontrou um grupo com a mesma quantidade de bits 1 adiciona o valor ao grupo
        addToValueGroup(current, v[i], 0xFF, &v[i], 1);
    }
  }
  return head;
}

// Função responsável por abstrair a verificação se o valor vai ter que criar um novo grupo na lista encadeada ou ser adicionado em um já criado anteriormente
void findOrCreateGroup(ValueGroup** listResult, int bits, Value* currentValue, int mask_output, unsigned char* combination, size_t combinationSize) {
  ValueGroup *current = *listResult;
  ValueGroup *prev = NULL;

  // Percorre a lista de grupos para encontrar um grupo com a mesma quantidade de bits 1
  while (current != NULL && current->numBits != bits) {  
    prev = current;
    current = current->next; 
  }

  // Se não encontrou um grupo com a mesma quantidade de bits 1, cria um novo grupo
  if (current == NULL) {
    ValueGroup *newGroup = createValueGroup(currentValue->value, mask_output, combination, combinationSize);
    // Adiciona o novo grupo na lista de resultados
    if (prev == NULL) {
        *listResult = newGroup;
    } else {
        prev->next = newGroup;
    }
  } else { // Se encontrou um grupo com a mesma quantidade de bits 1 adiciona o valor ao grupo
    addToValueGroup(current, currentValue->value, mask_output, combination, combinationSize);
  }
}

// Função para imprimir os grupos
void printValueGroups(ValueGroup *head) {
  ValueGroup *current = head;
  while (current != NULL) {
      printf("Grupo de %hhu bit(s):\n", current->numBits);
      Value *currentValue = current->values;
      while (currentValue != NULL) {
        printf("  Valor: %hhu Mask: %hhu", currentValue->value,currentValue->mask); 
        printf(" Combinação: ");
        for (int i = 0; i < currentValue->combinationSize; i++) {
          if (currentValue->combinationSize == 1 || i == currentValue->combinationSize - 1) {
            printf("%hhu", currentValue->combination[i]);
            continue;
          }
          printf("%hhu, ", currentValue->combination[i]);
        }
        printf("\n");
        currentValue = currentValue->next;
      }
      current = current->next;
  }
}

// Função para verificar se o valor já foi coberto e preencher ele no grupo de valores
void processValue(Value* currentValue, ValueGroup** listResult) {
  if (!currentValue->covered) {
    // printf("\nValor %hhu não coberto\n", currentValue->value);
    int bits = countBits(currentValue->value, currentValue->mask);
    findOrCreateGroup(listResult, bits, currentValue, currentValue->mask, currentValue->combination, currentValue->combinationSize);
    currentValue->covered = true;
  }
}

// Função para percorrer a lista de grupos e realizar as comparações
ValueGroup *compareGroups(ValueGroup *head) {
  // printf("===== INICIO DA COMPARAÇÃO =====\n");
  // Cria uma Lista Encadeada de Grupos de Valores nova
  ValueGroup *listResult = NULL; 
  ValueGroup *current = head;

  // Percorre a lista de grupos
  while ((current != NULL && current->next != NULL)) {
    // printf("\nComparação do Grupo de %hhu bits contra %hhu bits\n", current->numBits, current->numBits+1);
    ValueGroup *nextGroup = current->next; //  Seta o próximo grupo para comparação
    Value *currentValue = current->values;

    // Verifica todos valores do grupo atual 
    while (currentValue != NULL) { 
      unsigned char mask_output = 0xFF;
      Value *nextValue = nextGroup->values;
      // Verifica o valor do grupo atual com todos os valores do próximo grupo
      while (nextValue != NULL) { 
        // printf("\nVerificação: %hhu e %hhu\n", currentValue->value, nextValue->value);
        // Chama a verificação de bits diferentes e verifica se o retorno dele é valido ou não
        // Caso seja valido, adiciona os valores na lista nova de resultados e atualiza os valores da lista original 
        // para cobertos 
        int bitsDiferentes = verificarBitsDiferentes(currentValue->value, nextValue->value, currentValue->mask, nextValue->mask, &mask_output);
        bool isValid = (bitsDiferentes == 1) ? true : false;

        // Verifica se a quantidade de bits diferentes é igual a 1
        if (isValid){
          int bits = countBits(currentValue->value, mask_output);
          // Adiciona as combinacoes dos valores comparados
          size_t combinationSize = currentValue->combinationSize + nextValue->combinationSize;
          unsigned char combination[combinationSize];
          for (int i = 0; i < currentValue->combinationSize; i++) {
              combination[i] = currentValue->combination[i];
          }
          for (int i = 0; i < nextValue->combinationSize; i++) {
              combination[i + currentValue->combinationSize] = nextValue->combination[i];
          }

          // Adiciona os valores na lista de resultados
          findOrCreateGroup(&listResult, bits, currentValue, mask_output, combination, combinationSize);

          currentValue->covered = true;
          nextValue->covered = true;
        }
        nextValue = nextValue->next;
      }
      // Caso o valor corrente não tenha sido coberto ao final de sua comparação com o outro grupo, adicione ele na lista e marque como coberto
      processValue(currentValue, &listResult);
      currentValue = currentValue->next;
    }
    current = current->next;
  }

  // Laço para percorrer o ultimo grupo caso ele obtenha algum valor que não tenha sido coberto para ser inserido na lista, pois se não ele desconsidera o ultimo grupo
  while(current != NULL) {
    Value *currentValue = current->values;
    while (currentValue != NULL) {
      // Caso o valor corrente não tenha sido coberto ao final de sua comparação com o outro grupo, adicione ele na lista e marque como coberto
      processValue(currentValue, &listResult);
      currentValue = currentValue->next;
    }
    current = current->next;
  }
  // printf("\n===== FIM DA COMPARAÇÃO =====\n");
  return listResult; // Retorna a lista de resultados
}

// Função para percorrer a lista e remover valores com a mesma máscara
void removeDuplicates(ValueGroup *head) {
  ValueGroup *current = head;

  // Percorre a lista inteira
  while (current != NULL) {
    Value *currentValue = current->values;

    // Verifica o valor atual com todos do seu grupo de bits até o ultimo
    while (currentValue != NULL && currentValue->next != NULL) {
      Value *nextValue = currentValue->next;
      unsigned char mask_output = 0xFF;

      // Verifica a quantidade de bits diferentes que os valores possuem
      // Caso a não obtenha diferença, quer dizer que os numeros são iguais e a mascara também
      // Então remove o proximo valor da lista
      int bitsDiferentes = verificarBitsDiferentes(currentValue->value, nextValue->value, currentValue->mask, nextValue->mask, &mask_output);
      if (bitsDiferentes == 0) {  
        currentValue->next = nextValue->next;
        free(nextValue);
      } else { // Se não, continua a verificação
        currentValue = currentValue->next;
      }
    }

    // Pula para o proximo grupo de bits
    current = current->next;
  }
}

// Função que irá verificar se os valores dentro dos grupos são os mesmos
bool isGroupValuesIdentical(Value* groupValue1, Value* groupValue2) {
  // Compare os grupos em ambas as listas
  while (groupValue1 != NULL && groupValue2 != NULL) {
    // Compare todos os campos relevantes nos dois grupos
    if (groupValue1->value != groupValue2->value || 
        groupValue1->mask != groupValue2->mask || 
        groupValue1->combinationSize != groupValue2->combinationSize) {
      return false;
    }

    // Verifica todos os valores contidos nas combinações
    for (int i = 0; i < groupValue1->combinationSize; i++) {
      if (groupValue1->combination[i] != groupValue2->combination[i]) {
        return false;
      }
    }

    // Avance para o próximo grupo em ambas as listas
    groupValue1 = groupValue1->next;
    groupValue2 = groupValue2->next;
  }

  // Se chegamos ao final de um grupo, mas não de outro, os grupos não são idênticos
  if (groupValue1 != NULL || groupValue2 != NULL) {
    return false;
  }

  // Se todos os campos são idênticos, os grupos são idênticos
  return true;
}

/// Função comparadora de duas lista encadeadas, caso ela seja identica retorna True, se não False
bool isIdentical(ValueGroup* list1, ValueGroup* list2) {
  // Compare os grupos de bits em ambas as listas
  while (list1 != NULL && list2 != NULL) {
    // Se os grupos atuais não forem idênticos, as listas não são idênticas
    if (!isGroupValuesIdentical(list1->values, list2->values)) {
      return false;
    }

    // Avance para o próximo grupo em ambas as listas
    list1 = list1->next;
    list2 = list2->next;
  }

  // Se chegamos ao final de uma lista, mas não da outra, as listas não são idênticas
  if (list1 != NULL || list2 != NULL) {
    return false;
  }

  // Se chegamos ao final de ambas as listas, elas são idênticas
  return true;
}

// Função que irá realizar a comparação dos valores da lista e gerar uma nova
// Após essa geração, irá realizar a remoção de duplicadas
// Caso a lista anterior a comparação de grupos, quer dizer que chegou no circuito minimo
// então retorna para a função principal e encerra a recursão.
ValueGroup* compareAndRemoveDuplicates(ValueGroup* currentList, ValueGroup* previousList) {
  ValueGroup* newList = compareGroups(currentList);
  // printf("\nGrupos resultantes:\n");
  // printValueGroups(newList);

  removeDuplicates(newList);
  // printf("\nGrupos resultantes - Resumido [1]:\n");
  // printValueGroups(newList);

  if (isIdentical(newList, previousList)) {
    return newList;
  } else {
    return compareAndRemoveDuplicates(newList, currentList);
  }
}

// Função que irá encontrar a posição de um valor em um array
int findPosition(unsigned char* array, int size, int value) {
  for (int i = 0; i < size; i++) {
    if (array[i] == value) {
      return i;
    }
  }
  return -1; // Retorna -1 se o valor não for encontrado
}

// Função para imprimir a matriz
void printMatrix(int** matrix, int numRows, int numCols) {
  for (int i = 0; i < numRows; i++) {
    for (int j = 0; j < numCols; j++) {
      printf("%d ", matrix[i][j]);
    }
    printf("\n");
  }
}

// Alterar o tipo de retorno para int**
int** generateCoverageMatrix(ValueGroup* finalList, unsigned char inputArray[], int numRecords, int* numRows, int* numCols)  {
  *numCols = numRecords;  // Obtenha o número de colunas da matriz

  // Criar a matriz com a quantidade de colunas igual ao número de registros
  int** matrix = (int**)malloc(numRecords * sizeof(int*));
  if (matrix == NULL) {
    fprintf(stderr, "Erro ao alocar memória para a matriz.\n");
    exit(EXIT_FAILURE);
  }

  for (int i = 0; i < numRecords; i++) {
    matrix[i] = (int*)malloc(numRecords * sizeof(int));
     if (matrix[i] == NULL) {
      fprintf(stderr, "Erro ao alocar memória para a linha %d da matriz.\n", i);
      exit(EXIT_FAILURE);
    }
    // Inicializar a linha com zeros
    memset(matrix[i], 0, numRecords * sizeof(int));
  }

  // Preencher a matriz com o de-para das posições e valores da lista final
  ValueGroup* current = finalList;
  int row = 0;
  while (current != NULL) {
      Value* tempValues = current->values; // Use uma variável temporária para iterar sobre os valores
      while(tempValues != NULL) {
        for (int i = 0; i < tempValues->combinationSize; i++) {
          // Encontrar a posição do valor no array de entrada
          int pos = findPosition(inputArray, numRecords, tempValues->combination[i]);
          if (pos != -1) {
            // Se o valor estiver presente, preencha a posição correspondente na matriz com 1
            matrix[row][pos] = 1;
          }
        }
        row++;
        tempValues = tempValues->next; // Atualize a variável temporária, não a lista original
      }
      current = current->next;
  }
  *numRows = row; // Definir o número de linhas da matriz
  return matrix; // Retornar a matriz
}

// Função para liberar a memória alocada para a matriz
void freeMatrixTable(int** head, int numRows) {
    for (int i = 0; i < numRows; i++) {
      free(head[i]);
    }
    free(head);
}

// Função para extrair as informações dos primos implicantes essenciais e restantes
PrimeInfo* extractPrimeInfo(ValueGroup* finalList, int* essentialPrimes, int numRows) {
  PrimeInfo* primeInfo = malloc(sizeof(PrimeInfo));
  if (primeInfo == NULL) {
    fprintf(stderr, "Erro ao alocar memória para primeInfo.\n");
    exit(EXIT_FAILURE);
  }
  primeInfo->values = malloc(numRows * sizeof(int));
  primeInfo->masks = malloc(numRows * sizeof(int));
  primeInfo->size = 0;

  ValueGroup* current = finalList;
  int row = 0;
  while (current != NULL) {
    Value* tempValues = current->values;
    while(tempValues != NULL) {
      if (essentialPrimes[row] == 1) {
        primeInfo->values[primeInfo->size] = tempValues->value;
        primeInfo->masks[primeInfo->size] = tempValues->mask;
        primeInfo->size++;
      }
      row++;
      tempValues = tempValues->next;
    }
    current = current->next;
  }

  return primeInfo;
}

// Função para encontrar e imprimir os implicantes primos essenciais
PrimeInfo* findEssentialPrimeImplicants(ValueGroup* finalList, int** matrix, int numRows, int numCols) {
  // Aloca memória para os vetores que armazenarão os implicantes primos essenciais, suas colunas correspondentes e os índices cobertos
  int* essentialPrimes = calloc(numRows, sizeof(int));
  int* essentialPrimeCols = calloc(numCols, sizeof(int));
  int* coveredIndices = calloc(numCols, sizeof(int));

  // Verifica se a alocação de memória foi bem sucedida
  if (!essentialPrimes || !essentialPrimeCols || !coveredIndices) {
    fprintf(stderr, "Erro ao alocar memória.\n");
    exit(EXIT_FAILURE);
  }

  int essentialPrimeFound = 0;
  // Loop para encontrar implicantes primos essenciais
  for (int j = 0; j < numCols; j++) {
    int count = 0;
    int index;
    // Conta quantos implicantes primos cobrem um minitermo
    for (int i = 0; i < numRows; i++) {
      if (matrix[i][j] == 1) {
        count++;
        index = i;
      }
    }
    // Se apenas um implicante primo cobre o minitermo, ele é considerado essencial
    if (count == 1) {
      essentialPrimes[index] = 1;
      essentialPrimeCols[index] = j;
      coveredIndices[j] = 1; // Marca o valor como coberto
      essentialPrimeFound = 1;
    }
  }
  
  // Imprime os implicantes primos essenciais
  printf("\nImplicantes primos essenciais: ");
  for (int i = 0; i < numRows; i++) {
    if (essentialPrimes[i] == 1) {
      printf("\nLinha %d, Coluna %d ", i, essentialPrimeCols[i]);
    }
  }

  // Marca os minitermos que são cobertos pelos implicantes primos essenciais
  for (int i = 0; i < numRows; i++) {
    if (essentialPrimes[i] == 1) {
      for (int j = 0; j < numCols; j++) {
        if (matrix[i][j] == 1 && essentialPrimeCols[i] != j) {
          coveredIndices[j] = 1;
        }
      }
    }
  }

  // Verifica se todos os minitermos são cobertos
  for (int j = 0; j < numCols; j++) {
    if (coveredIndices[j] == 0) {
      printf("\nMinitermo na coluna %d não está coberto.\n", j);
      // Se um minitermo não é coberto, encontra o implicante primo que cobre a maioria dos minitermos não cobertos
      int maxCover = 0;
      int maxCoverIndex = -1;

      // Percorre as linhas da matriz
      for (int i = 0; i < numRows; i++) {
        // Verifica se o implicante primo não é essencial
        if (essentialPrimes[i] == 0) {
          int cover = 0;
          // Conta quantos minitermos não cobertos são cobertos pelo implicante primo
          for (int k = 0; k < numCols; k++) {
            if (matrix[i][k] == 1 && coveredIndices[k] == 0) {
              cover++;
            }
          }
          printf("Implicante primo na linha %d cobre %d minitermos não cobertos.\n", i, cover);
          // Seleciona o implicante primo que cobre a maior quantidade de minitermos não cobertos
          if (cover > maxCover) {
            maxCover = cover;
            maxCoverIndex = i;
          }
        }
      }
      // Marca o implicante primo encontrado como essencial e atualiza os índices cobertos
      if (maxCoverIndex != -1) {
        printf("Implicante primo na linha %d foi selecionado como essencial.\n", maxCoverIndex);
        essentialPrimes[maxCoverIndex] = 1;
        for (int k = 0; k < numCols; k++) {
          if (matrix[maxCoverIndex][k] == 1) {
            coveredIndices[k] = 1;
          }
        }
      }
    }
  }

  // Extrai as informações dos implicantes primos para retornar
  PrimeInfo* primeInfo = extractPrimeInfo(finalList, essentialPrimes, numRows);

  // Libera a memória alocada
  free(essentialPrimes);
  free(essentialPrimeCols);
  free(coveredIndices);

  return primeInfo;
}

// Função que irá criar a tabela final do circuito
int** createFinalTableCircuit(PrimeInfo* primeInfo) {
  // Verifica se primeInfo, primeInfo->masks, primeInfo->values são NULL ou se numBytes é um valor inválido
  if (primeInfo == NULL || primeInfo->masks == NULL || primeInfo->values == NULL || numBytes <= 0) {
    printf("Error: Invalid input\n");
    exit(EXIT_FAILURE);
  }

  // Aloca memória para a tabela
  int** table = malloc(primeInfo->size * sizeof(int*));
  if (table == NULL) {
    printf("Erro: Não foi possível alocar memória para a tabela\n");
    exit(EXIT_FAILURE);
  }

  // Aloca memória para cada linha da tabela
  for (int i = 0; i < primeInfo->size; i++) {
    table[i] = malloc(numBytes * sizeof(int));
    if (table[i] == NULL) {
      printf("Erro: Não foi possível atribuir memória para a linha da tabela\n");
      exit(EXIT_FAILURE);
    }
  }

  // Preenche a tabela com os valores de primeInfo
  for (int i = 0; i < primeInfo->size; i++) {
    for (int j = numBytes - 1; j >= 0; j--) {
      // Desconsidera os bits da máscara
      if ((primeInfo->masks[i] >> j) & 1) {
        table[i][numBytes - 1 - j] = 'X'; // Valor desconsiderado
      } else {
        table[i][numBytes - 1 - j] = (primeInfo->values[i] >> j) & 1; // Valor do bit
      }
    }
  }

  printf("\n");
  int index = 0;
  // Imprime o cabeçalho da tabela
  // e armazena o valor do alfabeto invertido
  for (int i = numBytes - 1; i >= 0; i--) {
    printf("%c ", alphabet[i]);
    invertAlphabet[index] = alphabet[i];
    index++;
  }
  printf("\n");

  // Imprime os dados da tabela
  for (int i = 0; i < primeInfo->size; i++) {
    for (int j = 0; j < numBytes; j++) {
      printf("%d ", table[i][j]);
    }
    printf("\n");
  }

  return table;
}

// Função que retorna o circuito final a partir da tabela de circuito final
void printTheFinalCircuit(int** finalCircuitTable, int numPrimes) {
  printf("\nCircuito final: \n");

  // Percorre a tabela 
  for (int i = 0; i < numPrimes; i++) {
    for (int j = 0; j < numBytes; j++) {
      if (finalCircuitTable[i][j] == 1) {
        printf("%c", invertAlphabet[j]);
      } else if (finalCircuitTable[i][j] == 0) {
        printf("~%c", invertAlphabet[j]);
      } 
    }
    if (i != numPrimes - 1) {
      printf(" + ");
    }
  }
}

// Função para gerar o circuito mínimo
void generate_min_circuit(unsigned char v[], int n) {
  unsigned char lastValue = v[n-1];
  numBytes = numBytesRequired(lastValue);

  printf("Generating minimal circuit for %d elements\n", n);
  // Cria a lista de grupos de valores a partir do vetor de valores informado
  ValueGroup *listaInicial = initializeListValue(v, n); // Lista Inicial 
  
  printf("\nGrupos Lista Inicial:\n");
  printValueGroups(listaInicial);
  printf("\n");
  
  // Irá realizar todos os agrupamentos e comparações dos valores da lista inicial
  // retornado assim a lista de valores de agrupamentos finais
  ValueGroup *listaFinal = compareAndRemoveDuplicates(listaInicial, NULL); // Lista Final
  
  printf("\nGrupos Lista Final:\n");
  printValueGroups(listaFinal);
  printf("\n");
  
  // Chama a função para gerar a matriz de cobertura
  int numRows, numCols;
  int** coverageMatrix = generateCoverageMatrix(listaFinal, v, n, &numRows, &numCols);

  // Verifica se a matriz foi criada com sucesso
  if (coverageMatrix == NULL) {
    fprintf(stderr, "Erro ao criar a matriz de cobertura.\n");
    exit(EXIT_FAILURE);
  }

  // Imprime a matriz
  printf("Matriz de Cobertura (%d linhas x %d colunas):\n", numRows, numCols);
  printMatrix(coverageMatrix, numRows, numCols);

  // Encontre os primos implicantes essenciais 
  PrimeInfo* primeInfo = findEssentialPrimeImplicants(listaFinal, coverageMatrix, numRows, numCols);
  printf("\n\nInformações dos primos implicantes:\n");
  for (int i = 0; i < primeInfo->size; i++) {
    printf("Valor: %d (%s), Máscara: %d (%s)\n", primeInfo->values[i] ,charToBinary(primeInfo->values[i]), primeInfo->masks[i], charToBinary(primeInfo->masks[i]));
  }

  // Cria a tabela dos primos implicantes
  int** finalCircuitTable = createFinalTableCircuit(primeInfo);

  // Printa o circuito final a partir da tabela de circuito final
  printTheFinalCircuit(finalCircuitTable, primeInfo->size);

  // Libera a memoria
  freeValueGroups(listaInicial);
  freeValueGroups(listaFinal);
  freeMatrixTable(coverageMatrix, numRows);
  freeMatrixTable(finalCircuitTable, numRows);
};


// Função principal
int main() {
  initializeAlphabet();
  unsigned char v[] = {4, 8, 9, 10, 11, 12, 14, 15};
  generate_min_circuit(v, 8);
  // unsigned char v[] = {0, 1, 2, 5, 6, 7};
  // generate_min_circuit(v, 6); // https://atozmath.com/KMap.aspx?q=quine&q1=0%2c1%2c2%2c5%2c6%2c7%60%60a%2cb%2cc%601&do=1#PrevPart
  // unsigned char v[] = {3, 5, 7, 9 ,11};
  // generate_min_circuit(v, 5);
  return 0;
}
