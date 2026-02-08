# 🌍 Monitor Ambiental

Monitor ambiental baseado em **Raspberry Pi Pico W** que mede luminosidade, temperatura e umidade, controla uma matriz de LEDs WS2812 e disponibiliza um painel web com autenticação.

---

## 📋 Visao Geral do Projeto

Este projeto implementa um sistema de monitoramento ambiental que:

- ✅ **Mede luminosidade** em tempo real usando o sensor BH1750
- ✅ **Mede temperatura e umidade** com o sensor AHT10
- ✅ **Exibe dados** em um display OLED SSD1306 (128x64)
- ✅ **Controla matriz de LEDs** WS2812 (5x5 = 25 LEDs RGB)
- ✅ **Oferece controle manual** via botoes integrados (BitDogLab)
- ✅ **Disponibiliza painel web** com login, dashboard e JSON
- ✅ **Implementa comandos via UART** para diagnostico e controle
- ✅ **Usa FreeRTOS** com tarefas dedicadas e sincronizacao de dados

---

## Titulo

Monitor Ambiental (Raspberry Pi Pico W + FreeRTOS + WiFi + Sensores)

## Introducao

Este projeto apresenta um sistema de monitoramento ambiental embarcado em Raspberry Pi Pico W, com leitura de sensores, exibicao local em display grafico e acesso remoto via rede WiFi. A implementacao prioriza organizacao em tarefas com FreeRTOS e interfaces padronizadas (I2C, UART e HTTP/TCP).

## Objetivos

- Medir luminosidade, temperatura e umidade com dois sensores I2C.
- Exibir informacoes localmente em display OLED e em painel web.
- Utilizar FreeRTOS para separar tarefas de sensores, display, UART e rede.
- Disponibilizar comunicacao via WiFi com protocolo HTTP/TCP.
- Comprovar uso de interrupcao para botoes e comunicacao serial UART.

## Justificativa

Monitoramento ambiental com interface local e remota permite tomada de decisao rapida e aprendizado de integracao entre sensores, RTOS e conectividade IoT. O projeto tambem consolida o uso do Pico W e do stack lwIP em cenarios reais.

## Problematica

Sistemas embarcados de baixo custo normalmente nao possuem monitoramento remoto e organizacao concorrente de tarefas. O desafio e integrar sensores, display e rede WiFi mantendo responsividade, estabilidade e baixo consumo.

## Requisitos obrigatorios (verificacao)

- Linguagem C: sim (C11 no projeto e arquivos .c/.h).
- Raspberry Pi Pico W: sim (CYW43/WiFi em uso).
- FreeRTOS: sim (tarefas dedicadas).
- Dois sensores: sim (BH1750 e AHT10).
- WiFi: sim (conexao com CYW43).
- Protocolo: sim (HTTP/TCP via lwIP).
- Display grafico: sim (SSD1306 OLED).
- Interface UART: sim (comandos UART).
- Interrupcao: sim (IRQ para botoes).

## Arquitetura do sistema (Hardware + Firmware + IoT) - Diagrama de blocos

```
        +-------------------+
        |     Ambiente      |
        +---------+---------+
             |
       +---------------+----------------+
       |                                |
   +--------v-------+                +-------v--------+
   |  BH1750 (Luz)  |                | AHT10 (Temp/U) |
   +--------+-------+                +-------+--------+
       |                                |
       +---------------+----------------+
             |
          +-------v--------+
          | Pico W (RP2040)|
          +---+---+---+----+
         |   |   |
      +-----------------+   |   +-----------------+
      |                     |                     |
 +----v-----+         +-----v-----+         +----v-----+
 | OLED     |         | WS2812    |         | UART/USB |
 | SSD1306  |         | 5x5 LEDs  |         +----------+
 +----------+         +-----------+
         |
       +------v-------+
       | FreeRTOS     |
       | (Tasks)      |
       +------+-------+
         |
      +-------v--------+
      | WiFi CYW43     |
      | + lwIP         |
      +-------+--------+
         |
      +-------v--------+
      | HTTP/TCP       |
      | Painel Web     |
      +----------------+
```

**Hardware**: Pico W + BH1750 + AHT10 + OLED SSD1306 + WS2812 + botoes.

**Firmware**: Drivers I2C/PIO, tarefas FreeRTOS, controle de LEDs, UART e servidor HTTP.

**IoT**: WiFi (CYW43) + lwIP com servidor HTTP/TCP e painel web autenticado.

## Desenvolvimento do sistema

- **Modulos principais**: `drivers` (sensores, display, LEDs), `src` (aplicacao), `web` (servidor HTTP e paginas).
- **Tarefas FreeRTOS**: `task_sensors`, `task_display`, `task_uart`, `task_web`.
- **Sensores**: BH1750 (luminosidade) e AHT10 (temperatura/umidade).
- **Protocolos**: I2C para sensores/display; HTTP/TCP via lwIP; UART para comandos.

---

## 🔧 Plataforma Alvo

- **Microcontrolador**: Raspberry Pi Pico W (RP2040)
- **SDK**: Pico SDK v2.2.0
- **RTOS**: FreeRTOS
- **Rede**: CYW43 + lwIP (HTTP/TCP)
- **Linguagem**: C11
- **Compilador**: ARM GCC 14.2 Release 1
- **Build Tool**: CMake 3.13+ com Ninja

### Especificacoes do RP2040
- **CPU**: ARM Cortex-M0+ dual-core @ 125 MHz
- **RAM**: 264 KB SRAM
- **Flash**: 2 MB
- **GPIO**: 28 pinos (26 disponiveis para uso)
- **Perifericos**: I2C, SPI, UART, ADC, PIO, PWM

---

## 🔌 Mapeamento de Pinos

### Comunicacao I2C

#### **I2C0** (Barramento 0) - BH1750 + AHT10
| Pino RP2040 | Sinal | Sensor | Descricao |
|---|---|---|---|
| **GPIO 0 (GP0)** | SDA | BH1750/AHT10 | Dados |
| **GPIO 1 (GP1)** | SCL | BH1750/AHT10 | Clock |
| GND | GND | Ambos | Terra |
| 3V3 | VCC | Ambos | Alimentacao |

**Frequencia**: 100 kHz (compatibilidade com AHT10)
**Enderecos I2C**:
- BH1750: 0x23 (padrao) ou 0x5C (alternativo)
- AHT10: 0x38

#### **I2C1** (Barramento 1) - Display OLED SSD1306
| Pino RP2040 | Sinal | Display SSD1306 | Descricao |
|---|---|---|---|
| **GPIO 14 (GP14)** | SDA | PIN 4 (SDA) | Dados |
| **GPIO 15 (GP15)** | SCL | PIN 3 (SCL) | Clock |
| GND | GND | PIN 2 (GND) | Terra |
| 3V3 | VCC | PIN 1 (VCC) | Alimentacao |

**Frequencia**: 400 kHz
**Endereco I2C**: 0x3C
**Resolucao**: 128x64 pixels

### Outras Saidas

#### **Matriz de LEDs WS2812**
| Pino RP2040 | Sinal | WS2812 | Descricao |
|---|---|---|---|
| **GPIO 7 (GP7)** | DIN | PIN 1 (DIN) | Dados serie (PIO) |
| 5V | VCC | PIN 2 (VCC) | Alimentacao (requer potencia adequada) |
| GND | GND | PIN 3 (GND) | Terra |

**Quantidade**: 25 LEDs (matriz 5x5)
**Protocolo**: WS2812 (1-wire, 800 kHz)
**Implementacao**: PIO

### Controle de Entrada

#### **Botoes BitDogLab**
| Pino RP2040 | Botao | Descricao |
|---|---|---|
| **GPIO 5 (GP5)** | **Botao A** | **Desativa matriz de LEDs** |
| **GPIO 6 (GP6)** | **Botao B** | **Ativa matriz de LEDs** |

**Tipo**: Pull-up interno habilitado
**Acionamento**: Nivel baixo (0 = pressionado)
**Debounce**: 200 ms (ISR + tarefa)

### Comunicacao Serial

#### **UART0** (Serial via USB)
- **Habilitada**: Sim (stdio via USB e UART)
- **Baud Rate**: 115200 bps
- **Uso**: Debug, comandos e diagnostico

---

## 🖥️ Protocolo I2C

### Configuracao
```c
// I2C0: BH1750 + AHT10
i2c_init(i2c0, 100 * 1000);      // 100 kHz
gpio_set_function(0, GPIO_FUNC_I2C);
gpio_set_function(1, GPIO_FUNC_I2C);

// I2C1: SSD1306
i2c_init(i2c1, 400 * 1000);      // 400 kHz
gpio_set_function(14, GPIO_FUNC_I2C);
gpio_set_function(15, GPIO_FUNC_I2C);
```

### Scanner I2C Integrado
Ao inicializar, o programa executa uma varredura de enderecos (0x00-0x7F) em ambos os barramentos para detectar e listar dispositivos conectados.

---

## 📡 Sensores e Perifericos

### 1️⃣ Sensor de Luz - BH1750

**Tipo**: Sensor de luminosidade (luximetro digital)
**Protocolo**: I2C (barramento I2C0)
**Endereco**: 0x23 (padrao) ou 0x5C (alternativo)

#### Conversao
```c
uint16_t raw_value = (data[0] << 8) | data[1];
float lux = raw_value / 1.2f;
```

#### Logica de Intensidade por Luminosidade
```
Ambiente Lux | Estado LED | Objetivo
0-50         | HIGH (10%) | Auxiliar em ambiente escuro
50-200       | MEDIUM (3%)| Indicador em ambiente fraco
200-500      | LOW (1%)   | Indicador suave em ambiente claro
500+         | OFF        | Desligado em ambiente muito claro
```

### 2️⃣ Sensor de Temperatura/Umidade - AHT10

**Tipo**: Sensor digital de temperatura e umidade
**Protocolo**: I2C (barramento I2C0)
**Endereco**: 0x38
**Tempo de medicao**: ~80 ms

#### Formulas
```
Temperatura (C) = (raw * 200 / 2^20) - 50
Umidade (%) = raw * 100 / 2^20
```

### 3️⃣ Display OLED - SSD1306

**Tipo**: Display OLED monocromatico
**Protocolo**: I2C (barramento I2C1)
**Endereco**: 0x3C
**Telas**:
- Tela 1: luminosidade + intensidade de LED + status
- Tela 2: temperatura + umidade

### 4️⃣ Matriz de LEDs WS2812 (NeoPixel)

**Tipo**: LED RGB enderecavel (Smart LED)
**Quantidade**: 25 LEDs (matriz 5x5)
**Protocolo**: WS2812 (1-wire, 800 kHz)
**Pino**: GPIO 7

---

## 🌐 WiFi e Servidor Web

### Conexao WiFi
- Chip CYW43 inicializa no boot
- Conexao com SSID e senha em [include/wifi_config.h](include/wifi_config.h)
- Timeout padrao: 30 s

### Servidor HTTP (porta 80)
Rotas implementadas:
- `/` ou `/index.html`: dashboard (autenticado)
- `/login`: formulario de login
- `/settings`: altera usuario e senha (autenticado)
- `/logout`: encerra sessao
- `/data`: JSON com leituras (autenticado)

### Credenciais
- Usuario/senha padrao: `root / root`
- Pode ser alterado via pagina de configuracao ou por comandos UART
- Sessao baseada em cookie `session`

---

## 🧵 FreeRTOS e Tarefas

Tarefas criadas em [src/rtos/rtos_app.c](src/rtos/rtos_app.c):

- **task_sensors**: leitura BH1750/AHT10, botoes (IRQ), atualiza LED (200 ms)
- **task_display**: alterna telas OLED (a cada 3 s), atualiza a cada 200 ms
- **task_uart**: comandos e diagnostico (poll a cada 20 ms)
- **task_web**: poll de rede e servidor (100 ms)

Dados compartilhados via `sensor_data` com mutex quando FreeRTOS esta ativo.

---

## 🔌 Comandos UART

Disponiveis via serial (115200 bps):

```
HELP
STATUS
WIFI?
LED ON
LED OFF
LOGIN RESET
LOGIN SET <usuario> <senha>
```

---

## 📊 Fluxo Principal de Execucao

```
1. Inicializa display e CYW43
2. Inicializa botoes, I2C0 (100 kHz) e I2C1 (400 kHz)
3. Scanner I2C em ambos os barramentos
4. Inicializa BH1750, AHT10 e matriz WS2812
5. Conecta ao WiFi e inicia servidor web (se possivel)
6. Cria tarefas FreeRTOS e inicia o scheduler
```

### Exemplo de Saída Serial
```
========================================
    MONITOR AMBIENTAL - INICIANDO
========================================

[INFO] Inicializando botoes...
[OK] Botoes A e B inicializados

[INFO] Inicializando I2C0 (GP0/GP1)...
[OK] I2C0 inicializado

[INFO] Inicializando I2C1 (GP14/GP15)...
[OK] I2C1 inicializado

=== Scanner I2C - I2C0 (GP0/GP1) ===
Escaneando enderecos 0x00 a 0x7F...
Dispositivo encontrado no endereco 0x23
Total: 1 dispositivo(s) encontrado(s)
========================

=== Scanner I2C - I2C1 (GP14/GP15) ===
Escaneando enderecos 0x00 a 0x7F...
Dispositivo encontrado no endereco 0x3C
Total: 1 dispositivo(s) encontrado(s)
========================

[INFO] Inicializando display OLED SSD1306...
[OK] Display OLED inicializado

[INFO] Inicializando sensor BH1750 no endereco 0x23...
[OK] BH1750 inicializado no endereco 0x23

[INFO] Inicializando matriz de LEDs...
[OK] Matriz de LEDs WS2812 inicializada no GPIO 7

Monitor Ambiental iniciado!

Luz: 245.8 lux
[LED] Intensidade FRACA
Luz: 248.3 lux
[LED] Intensidade FRACA
[BTN A] Matriz de LEDs DESATIVADA
[BTN B] Matriz de LEDs ATIVADA
```

---

## 🛠️ Como Compilar

### Pré-requisitos
- Windows PowerShell ou CMD
- CMake 3.13+
- GCC ARM Toolchain 14.2
- Pico SDK 2.2.0 instalado
- Ninja build tool

### Passos

1. **Abrir Terminal PowerShell** no diretório do projeto
   ```powershell
   # Navegue até o diretório do projeto
   cd "k:\ResidenciaDPM\Material Moodle\projetos-ideias\Projeto-monitor\codigo\monitor-ambiental"
   ```

2. **Criar Diretório de Build** (se não existir)
   ```powershell
   mkdir build -Force
   cd build
   ```

3. **Executar CMake**
   ```powershell
   cmake -G Ninja ..
   ```

4. **Compilar com Ninja**
   ```powershell
   ninja
   ```

   Ou usar VS Code:
   - Pressione **Ctrl+Shift+B** para compilar
   - Task: **Compile Project**

### Saída de Build Bem-sucedida
```
[1/10] Building C object CMakeFiles/MonitorAmbiental.dir/MonitorAmbiental.c.obj
[2/10] Building C object CMakeFiles/MonitorAmbiental.dir/ssd1306.c.obj
[3/10] Building C object CMakeFiles/MonitorAmbiental.dir/bh1750.c.obj
[4/10] Building C object CMakeFiles/MonitorAmbiental.dir/led_matrix.c.obj
[5/10] Generating ws2812.pio.h
[6/10] Linking C executable MonitorAmbiental.elf
[7/10] Generating UF2 image
[8/10] Compiling command database
[9/10] Built target MonitorAmbiental
```

### Arquivos Gerados
```
build/
├─ MonitorAmbiental.elf      # Executável ELF
├─ MonitorAmbiental.uf2      # Firmware para Raspberry Pi Pico
├─ MonitorAmbiental.dis      # Disassembly (debug)
└─ compile_commands.json     # Comandos de compilação (IntelliSense)
```

---

## 📲 Como Fazer o Flash

### Método 1: Modo Bootloader (Recomendado)

1. **Conectar Pico ao PC**
   - Pressione e mantenha o botão **BOOTSEL** do Pico
   - Enquanto mantém pressionado, conecte via USB-C
   - Solte o botão - o Pico aparecerá como unidade de armazenamento

2. **Copiar Arquivo UF2**
   ```powershell
   Copy-Item "build\MonitorAmbiental.uf2" "X:\"  # X: é a letra da unidade Pico
   ```
   Ou usar drag-and-drop no Windows Explorer

3. **Aguardar Reboot**
   - O Pico rebootará automaticamente
   - Desconecte e reconecte para uso normal

### Método 2: Usando Picotool (via VS Code)
```powershell
& "${env:USERPROFILE}\.pico-sdk\picotool\2.2.0-a4\picotool\picotool.exe" load "build\MonitorAmbiental.uf2" -fx
```

Ou pressionar **Ctrl+F5** no VS Code para executar a task "Run Project".

### Método 3: Usando OpenOCD (Debug via SWD)
```powershell
& "${env:USERPROFILE}\.pico-sdk\openocd\0.12.0+dev\openocd.exe" `
  -s "${env:USERPROFILE}\.pico-sdk\openocd\0.12.0+dev\scripts" `
  -f "interface/cmsis-dap.cfg" `
  -f "target/rp2040.cfg" `
  -c "adapter speed 5000; program 'build\MonitorAmbiental.elf' verify reset exit"
```

---

## 📝 Como Testar

### Teste 1: Verificar Inicializacao
1. Compilar e fazer flash do firmware
2. **Conectar Monitor Serial** (115200 baud)
   - VS Code: Usar a extensão "Serial Port Monitor"
   - Ou: `putty.exe` com 115200 baud
3. **Observar**:
   - ✅ Scanner I2C encontra 0x23/0x5C (BH1750) e 0x38 (AHT10) em I2C0
   - ✅ Scanner I2C encontra 0x3C (SSD1306) em I2C1
   - ✅ Display mostra mensagens de boot e depois o titulo
   - ✅ Mensagens "[OK]" para sensores, LEDs e WiFi (se conectado)

### Teste 2: Leitura de Luminosidade
1. **Ambiente bem iluminado** (>500 lux)
   - LEDs devem **apagar** automaticamente
   - Display: "LED: Desligado"

2. **Ambiente moderado** (200-500 lux)
   - LEDs com **brilho fraco** (1%)
   - Display: "LED: Fraco"

3. **Ambiente pouco iluminado** (50-200 lux)
   - LEDs com **brilho médio** (3%)
   - Display: "LED: Médio"

4. **Ambiente muito escuro** (<50 lux)
   - LEDs em **máximo brilho** (10%)
   - Display: "LED: Forte"

**Dica**: Use luz de celular para aumentar/diminuir a luminosidade sobre o sensor

### Teste 3: Controle de Botoes
1. **Pressionar Botão A** (GPIO 5)
   - LEDs apagam
   - Display: "Status: OFF"
   - Serial: "[BTN A] Matriz de LEDs DESATIVADA"

2. **Pressionar Botão B** (GPIO 6)
   - LEDs retomam brilho conforme luz ambiente
   - Display: "Status: ON"
   - Serial: "[BTN B] Matriz de LEDs ATIVADA"

### Teste 4: Temperatura e Umidade (AHT10)
1. Aguarde a alternancia de telas no display
2. Verifique se aparecem valores de temperatura e umidade
3. Se der erro, confira o endereco 0x38 e cabos do AHT10

### Teste 5: Verificar I2C
Execute o scanner I2C manualmente:
```c
i2c_scan(i2c0, "I2C0 (GP0/GP1)");
i2c_scan(i2c1, "I2C1 (GP14/GP15)");
```

Esperado:
- I2C0: **Encontrado 2 dispositivos** (0x23/0x5C e 0x38)
- I2C1: **Encontrado 1 dispositivo** (0x3C)

### Teste 6: UART (Comandos)
1. Abra o monitor serial (115200)
2. Digite `HELP` para listar comandos
3. Teste:
   - `STATUS` (leituras atuais)
   - `WIFI?` (estado e IP)
   - `LED ON` / `LED OFF`
   - `LOGIN SET usuario senha`
   - `LOGIN RESET`

### Teste 7: WiFi e Servidor Web
1. Configure SSID e senha em [include/wifi_config.h](include/wifi_config.h)
2. Observe o IP no serial e no display
3. Abra o navegador em `http://<IP>`
4. Login com `root / root`
5. Verifique o dashboard e o refresh automatico
6. Acesse `http://<IP>/data` e confirme JSON valido

### Teste 8: Performance e Responsividade
- **Latência de Atualização**: ~200 ms
- **Tempo de Resposta dos Botoes**: ~200 ms (debounce)
- **Frequência de Leitura**: 5 Hz (200 ms)
- **Taxa de Atualização Display**: 5 Hz

---

## 📦 Estrutura de Arquivos

```
projeto/
├─ src/
│  ├─ MonitorAmbiental.c       # Programa principal
│  ├─ sensor_data.c            # Estado compartilhado de sensores
│  ├─ wifi_manager.c           # Conexao WiFi (CYW43)
│  └─ rtos/
│     ├─ rtos_app.c            # Cria tarefas FreeRTOS
│     ├─ task_sensors.c        # Leitura de sensores e botoes
│     ├─ task_display.c        # Telas OLED
│     ├─ task_uart.c           # Comandos UART
│     └─ task_web.c            # Servidor HTTP
│
├─ drivers/
│  ├─ bh1750.c/.h              # Sensor de luminosidade
│  ├─ aht10.c/.h               # Temperatura e umidade
│  ├─ ssd1306.c/.h             # Display OLED
│  └─ led_matrix.c/.h          # WS2812
│
├─ web/
│  ├─ web_server.c/.h          # Servidor HTTP (lwIP)
│  ├─ web_pages.c/.h           # Paginas HTML/JSON
│  └─ auth.c/.h                # Login/sessao
│
├─ include/
│  ├─ wifi_config.h            # SSID, senha e porta
│  ├─ sensor_data.h            # API do estado compartilhado
│  └─ rtos_tasks.h             # Declaracoes de tarefas
│
├─ CMakeLists.txt              # Configuracao CMake
├─ pico_sdk_import.cmake       # Import Pico SDK
└─ build/                       # Diretorio de build
```

---

## 🔌 Diagrama de Conexão Simplificado

```
┌────────────────────────────────────────────────────────────────┐
│                    Raspberry Pi Pico W                         │
│                                                                │
│  I2C0:            I2C1:              GPIO:                     │
│  ├─ GP0 (SDA) ──── SDA ───────────┐  ├─ GP5 (BTN A)           │
│  ├─ GP1 (SCL) ──── SCL ────────┐  │  ├─ GP6 (BTN B)           │
│  │                              │  │  └─ GP7 (WS2812)          │
│  │                              │  │                           │
└──┼──────────────────────────────┼──┼────────────────────────────┘
   │                              │  │
   │                              │  │
   ▼                              ▼  ▼
┌──────────────────┐   ┌──────────────────┐   ┌──────────────────┐
│   BH1750 (LUX)   │   │ SSD1306 (OLED)   │   │ WS2812 (5x5 LED) │
│                  │   │ 128x64 Display   │   │ Matriz RGB       │
│ 0x23 on I2C0     │   │ 0x3C on I2C1     │   │                  │
│                  │   │                  │   │ (25 endereçáveis)│
│ SDA, SCL, VCC+GND│   │ SDA, SCL, VCC+GND│   │ DIN + VCC + GND  │
└──────────────────┘   └──────────────────┘   └──────────────────┘

Alimentação: 3.3V (Pico) → SSD1306, BH1750
             5V (Externo) → WS2812 (recomendado)
```

---

## ⚡ Considerações de Energia

### Consumo Esperado por Componente
| Componente | Estado | Consumo |
|---|---|---|
| RP2040 | Operação normal | ~30 mA |
| BH1750 | Lendo continuamente | ~0.16 mA |
| SSD1306 | Display 50% ligado | ~5 mA |
| WS2812 (25 LEDs) | OFF | ~0 mA |
| WS2812 (25 LEDs) | LOW (1%) | ~15 mA |
| WS2812 (25 LEDs) | MEDIUM (3%) | ~45 mA |
| WS2812 (25 LEDs) | HIGH (10%) | ~150 mA |
| **Total Mínimo** | - | **~35 mA** |
| **Total Máximo** | - | **~215 mA** |

### Fontes de Alimentação Recomendadas
- **USB 5V**: Adequado para testes (fornece até 500 mA)
- **Power Bank 5V/2A**: Operação prolongada
- **Fonte 5V/1A**: Uso contínuo em máximo brilho
- **Bateria 4xAA (6V)**: Portabilidade (com regulador para 5V)

### Otimizações de Energia Disponíveis
```c
// Desligar sensor quando não usado
bh1750_power_down(&light_sensor);

// Desligar display quando não necessário
// (não implementado atualmente)

// Usar LED_INTENSITY_OFF quando não precisar
led_matrix_clear(&led_matrix);
```

---

## 🐛 Troubleshooting

### Problema: BH1750 não encontrado no I2C
**Solução**:
- Verificar conexões SDA/SCL
- Confirmar pull-up habilitado
- Tentar endereço 0x5C (ADDR em HIGH)
- Verificar tensão 3.3V do sensor

### Problema: Display SSD1306 não exibe nada
**Solução**:
- Verificar endereço I2C (deve ser 0x3C)
- Confirmar barramento I2C1 (GP14/GP15)
- Tentar aumentar contraste (comando 0x81)
- Verificar alimentação 3.3V

### Problema: LEDs WS2812 não acendem
**Solução**:
- Verificar pino GP7 conectado ao DIN
- Confirmar alimentação 5V separada
- Verificar GND compartilhado
- Tentar resetar com `led_matrix_clear()`
- Verificar se PIO0 está disponível

### Problema: Botões não respondem
**Solução**:
- Verificar pinos GP5 (BTN A) e GP6 (BTN B)
- Confirmar pull-up habilitado
- Aumentar debounce (200 ms)
- Testar com log via serial

### Problema: WiFi nao conecta
**Solucao**:
- Verificar SSID e senha em [include/wifi_config.h](include/wifi_config.h)
- Confirmar sinal WiFi e 2.4 GHz
- Verificar mensagens no serial

### Problema: Compilação falha
**Solução**:
```powershell
# Limpar cache CMake
Remove-Item build -Recurse -Force
mkdir build
cd build
cmake -G Ninja ..
ninja
```

### Problema: Flash não funciona
**Solução**:
1. Modo Bootloader: Pressionar BOOTSEL + reconectar USB
2. Usar UF2 em vez de ELF
3. Verificar letra da unidade do Pico

---

## Evidencias de funcionamento

> Reserve esta area e insira suas evidencias reais.

- Prints do painel web (login, dashboard, JSON).
- Fotos do hardware montado (Pico W + sensores + display + LEDs).
- Logs de comunicacao (serial UART e requisicoes HTTP).

Exemplo de log (ja presente acima): secao "Exemplo de Saida Serial".

## Video obrigatorio (5 a 7 minutos)

- Link do video: [INSERIR LINK AQUI]
- Roteiro sugerido: demonstrar inicializacao, sensores, display, UART, WiFi e painel web.

## Conclusao

O projeto cumpre os requisitos do sistema embarcado com Pico W, integrando dois sensores via I2C, interface grafica, controle por UART e conectividade WiFi com servidor HTTP. A divisao em tarefas FreeRTOS garante responsividade e facilita a manutencao do firmware.

## Referencias

### Datasheets
- [BH1750 Ambient Light Sensor](https://www.mouser.com/datasheet/2/348/bh1750fvi-e-485057.pdf)
- [SSD1306 OLED Driver](https://cdn-shop.adafruit.com/datasheets/SSD1306.pdf)
- [WS2812 RGB LED](https://datasheets.maximintegrated.com/en/ds/WS2812.pdf)
- [RP2040 Microcontroller](https://datasheets.raspberrypi.org/rp2040/rp2040-datasheet.pdf)

### Documentação Oficial
- [Pico SDK Documentation](https://datasheets.raspberrypi.org/pico/raspberry-pi-pico-c-sdk.pdf)
- [Pico Hardware Design](https://datasheets.raspberrypi.org/pico/pico-design-guide.pdf)

### Bibliotecas Utilizadas
- `pico/stdlib.h` - Biblioteca padrão Pico
- `hardware/i2c.h` - Controlador I2C
- `hardware/pio.h` - Interface PIO
- `hardware/clocks.h` - Configuração de clock

---

## 📝 Versao

- **Versao do Projeto**: 0.2
- **SDK Pico**: 2.2.0
- **Data**: Fevereiro 2026
- **Status**: Funcional - Em desenvolvimento

---

## 👥 Contribuidores

Este projeto foi desenvolvido como parte da residência DPM (Desenvolvimento de Projetos em Microcontroladores).

---

## 📄 Licença

Código livre para uso educacional e comercial com atribuição.

---

## ✅ Checklist de Funcionamento

- [x] Compilação bem-sucedida
- [x] Flash via UF2/Picotool
- [x] Scanner I2C detecta ambos sensores
- [x] Leitura BH1750 em tempo real
- [x] Leitura AHT10 (temperatura/umidade)
- [x] Display SSD1306 exibe dados
- [x] Matriz WS2812 responde à luminosidade
- [x] Botões A e B controlam LEDs
- [x] Comunicação UART/USB em 115200 baud
- [x] Comandos UART operacionais
- [x] WiFi conectado (Pico W)
- [x] Servidor web com login e dashboard
- [x] Responsividade satisfatória (200 ms)
- [x] Consumo de energia otimizado

---

## 🔮 Melhorias Futuras

- [ ] Log de dados em SD card
- [ ] Modos de economia de energia (sleep mode)
- [ ] Padrões de animação na matriz de LEDs
- [ ] Calibração automática de sensores
- [ ] Interface gráfica avançada no display

---

## Apendices

- Codigo fonte (GitHub): https://github.com/devdenisviana/monitor_ambiental
- Esquematicos: [INSERIR LINK AQUI]
- Diagramas adicionais: [INSERIR LINK AQUI]

**Desenvolvido para aprendizado em microcontroladores**
