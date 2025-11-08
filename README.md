# Projeto

Criar um sintetizador simples e portátil.

### Metas

- [x] Gerar sons na escala maior
- [x] Adicionar modificadores de nota
- [x] Enviar sons por amplificador e line-out
- [x] Exibir no display as notas tocadas

## Circuito

![circuit.png](./assets/circuit.png)

### Bibliotecas

- [Maximilian](https://github.com/pschatzmann/Maximilian)
- [arduino-audio-tools](https://github.com/pschatzmann/arduino-audio-tools)
- [Adafruit_SSD1306](https://github.com/adafruit/Adafruit_SSD1306)
- [Adafruit-GFX-Library](https://github.com/adafruit/Adafruit-GFX-Library)

## Funcionalidades

- Ajuste de escala maior
- Ajuste de pitch
- Ajuste de ADSR
- Ajuste de filtro (lowpass, hipass)

## Modificador

Dependendo da direção do analógico o tom do acorde pode mudar:

| Direção        | Modificação   | Exemplo |
| :------------- | :------------ | :-----: |
| Base           | Base          | `Cmaj`  |
| Cima           | Maior/menor   | `Cmin`  |
| Cima Direita   | Sétima        |  `C7`   |
| Direita        | Maior7/menor7 | `Cmaj7` |
| Baixo Direita  | Maior9/menor9 | `Cmaj9` |
| Baixo          | Suspenso4     | `Csus4` |
| Baixo Esquerda | Suspenso2     | `Csus2` |
| Esquerda       | Diminuto      | `Cdim`  |
| Cima Esquerda  | Aumentado     | `Caug`  |

## Componentes

- ESP32-WROOM-32D
- Display OLED 128x64 I2C
- DAC externo (PCM5102)
- Amplificador (MAX98357A)
- Alto falante
- Analógico
- Switches/keycaps ou botões
- Botão switch

### Referências/Inspirações

- [HiChord](https://hichord.shop/)
- [Making A Synth With Python — Oscillators](https://python.plainenglish.io/making-a-synth-with-python-oscillators-2cb8e68e9c3b)
