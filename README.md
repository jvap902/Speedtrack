# Speedtrack

## Contribuições de cada membro
**João Vitor Angelo Pereira**
  - Cálculos de colisão
  - Malhas poligonais complexas
  - Animações baseadas em tempo

**Lucas Pereira Vargas**
  - Iluminação
  - Curvas de Bézier
  - Refatoração do código (necessário em determinado ponto)
  
**Ambos**
  - Mapeamento de texturas
  - Instâncias de objetos
  - Câmeras livre e look-at

## Uso de IA para desenvolvimento do trabalho
  Ambos membros da dupla utilizaram IAs, principalmente ChatGPT e Gemini, para o desenvolvimento do trabalho, de diversas maneiras e em múltiplos locais no código, como:
   - Debug: útil quando se trabalhou com cálculos de colisão, atualização do CMake ou quando se identificava um erro mas não se sabia a exata razão.
   - Geração de código: utilizado quando se tinha uma ideia teórica de como fazer algo, mas não se tinha certeza sobre a maneira de implementar.

  **Quanto a utilidade:** As LLMs utilizadas foram muito úteis sem dúvida, contudo às vezes elas mais atrapalhavam do que ajudavam, houveram situações em que debugar o código gerado acabou sendo mais trabalhoso do que fazer ele do zero. O momento em que as IAs menos conseguiam ajudar era quando se tinha alguma anomalia visual, por mais que pudessem apontar inconsistências no código, não eram capazes de ver o que estava acontecendo e ajudar a consertar o código de forma efetiva.

## O processo de desenvolvimento
  O processo de desenvolvimento começou de forma muito lenta, por não termos muitas funções iniciais prontas, passávamos muito tempo construíndo coisas do zero para se ter um resultado pequeno; conforme o desenvolvimento foi avançando, foi possível reaproveitar funções em diversas partes do código, permitindo que resultados fossem visualizados mais rapidamente, além disso, a familiaridade que se adquiriu ao longo do processo foi responsável por torná-lo mais simples de se compreender e construir.

## Imagens:
<img width="1916" height="1077" alt="image" src="https://github.com/user-attachments/assets/d8854d88-f74d-4a2b-9828-fa1b6d752e0b" />
<img width="1916" height="1074" alt="image" src="https://github.com/user-attachments/assets/77f03ca2-aa83-4a38-8c21-b7d924e47503" />

## Manual:
  Para controle do carro: W (anda para frente), A (vira para a esquerda), S (anda para trás), D (vira para a direita).
  V alterna modo de câmera (look-at ou câmera livre).
  Movimento do mouse controla a câmera livre.
  R reseta o timer e reposiciona o carro para a posição inicial.
  Espaço congela o jogo.
  
## Compilação e execução:
