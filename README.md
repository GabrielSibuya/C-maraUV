# MicroCâmara UV – Sistema de Monitoramento de Irradiação Ultravioleta

## 1. Sobre o projeto

Este repositório contém o código-fonte do sistema de monitoramento desenvolvido para uma **microcâmara de exposição à radiação ultravioleta (UV)**, destinada a estudos de envelhecimento e degradação fotoquímica de materiais, especialmente microplásticos.

O equipamento utiliza um microcontrolador Arduino Nano para realizar a aquisição de dados de temperatura e da resposta elétrica de um sensor de radiação UV. As informações são apresentadas em um display OLED e também enviadas pela comunicação serial, permitindo o acompanhamento das condições experimentais durante os ensaios de irradiação.

O projeto foi desenvolvido no contexto de atividades de pesquisa em degradação fotoquímica de materiais poliméricos.

## 2. Objetivos

- Monitorar a temperatura no interior da microcâmara;
- Monitorar a resposta elétrica do sensor de radiação UV;
- Registrar os valores mínimos e máximos das grandezas monitoradas;
- Calcular a integral temporal da tensão fornecida pelo sensor UV;
- Disponibilizar as informações em um display OLED e pela interface serial.

## 3. Componentes utilizados

| Componente | Função |
|---|---|
| Arduino Nano | Unidade de controle e aquisição de dados |
| Sensor UV GUVA-S12SD | Detecção da radiação ultravioleta |
| DS18B20 | Monitoramento da temperatura |
| Display OLED SSD1306 (128 × 64) | Exibição dos dados |
| Módulo relé | Controle do acionamento do sistema |
| Fonte de alimentação | Alimentação dos componentes |

## 4. Funcionamento do sistema

O sistema realiza a leitura periódica dos sensores e atualiza as informações disponibilizadas no display OLED.

A interface possui três páginas:

1. **Temperatura:** apresenta a temperatura atual, além dos valores mínimo e máximo registrados.
2. **Sensor UV:** apresenta a tensão média obtida a partir das leituras analógicas do sensor, juntamente com os valores mínimo e máximo.
3. **Integral temporal:** apresenta a integral acumulada da tensão do sensor UV em relação ao tempo.

A alternância entre as páginas ocorre automaticamente, sem interromper o processo de aquisição de dados.

## 5. Aquisição e processamento dos dados

### 5.1. Temperatura

A temperatura é obtida por meio do sensor DS18B20, utilizando a biblioteca `DallasTemperature`.

### 5.2. Resposta elétrica do sensor UV

A aquisição do sinal do sensor UV é realizada pela entrada analógica A0 do Arduino Nano.

Para cada ciclo de aquisição, são realizadas 100 leituras consecutivas, com intervalo de 2 ms entre elas. O valor médio é convertido em tensão utilizando uma referência de 5 V e a resolução de 10 bits do conversor analógico-digital:

\[
V_{\mathrm{UV}} = \frac{\overline{ADC}\cdot 5}{1023}
\]

### 5.3. Integral temporal da tensão UV

O sistema também calcula a integral da tensão fornecida pelo sensor ao longo do tempo:

\[
I_{\mathrm{UV}}(t)=\int_0^t V_{\mathrm{UV}}(t')\,dt'
\]

A implementação numérica utiliza o método de integração por intervalos temporais, considerando o valor mais recente da tensão:

\[
I_{\mathrm{UV}} \approx \sum_i V_i\Delta t_i
\]

A grandeza obtida possui unidade de **V·s (volt-segundo)**.

**Importante:** como o sensor ainda não possui calibração radiométrica, a integral calculada representa a acumulação temporal da tensão do sensor, não diretamente a energia ou a fluência de radiação incidente sobre a amostra.

## 6. Configurações principais

Os principais parâmetros de aquisição podem ser modificados diretamente no código-fonte:

| Parâmetro | Valor | Descrição |
|---|---:|---|
| `intervaloLeitura` | 15000 ms | Intervalo entre aquisições |
| `intervaloTela` | 4000 ms | Intervalo de alternância das páginas |
| `NleiturasUV` | 100 | Número de leituras para cálculo da tensão média |
| `Ndados` | 1 | Número de leituras de temperatura |
| `pinoUV` | A0 | Entrada analógica do sensor UV |
| `rele` | D12 | Pino de controle do relé |

## 7. Bibliotecas necessárias

Para compilar o código, é necessário instalar as seguintes bibliotecas na Arduino IDE:

- `Wire`
- `Adafruit GFX Library`
- `Adafruit SSD1306`
- `OneWire`
- `DallasTemperature`

## 8. Estrutura do repositório

```text
MicroCamaraUV/
│
├── MicroCamaraUV_v6.ino
└── README.md
```

## 9. Desenvolvimentos futuros

Entre as possibilidades de aprimoramento do sistema estão:

- Calibração radiométrica do sensor UV;
- Conversão da resposta elétrica em irradiância;
- Determinação da dose de radiação incidente sobre as amostras;
- Implementação de armazenamento automático dos dados;
- Desenvolvimento de uma interface para acompanhamento e análise dos experimentos.

## 10. Autoria

Projeto desenvolvido no âmbito de atividades de pesquisa em instrumentação experimental e degradação fotoquímica de materiais poliméricos.

**LABSOLAR – Laboratório de pesquisa**

---

*Este projeto possui finalidade científica e experimental. Os resultados obtidos dependem das características dos sensores utilizados, das condições de montagem e da calibração do sistema.*
