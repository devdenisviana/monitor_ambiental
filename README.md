# 🌍 Monitor Ambiental

Monitor ambiental baseado em **Raspberry Pi Pico W** que mede luminosidade e controla uma matriz de LEDs RGB inteligente baseada nas condições de luz ambiente.

---

## 📋 Visão Geral do Projeto

Este projeto implementa um sistema de monitoramento ambiental que:

- ✅ **Mede luminosidade** em tempo real usando o sensor BH1750
- ✅ **Exibe dados** em um display OLED SSD1306 (128x64 pixels)
- ✅ **Controla matriz de LEDs** WS2812 (5x5 = 25 LEDs RGB) responsivamente
- ✅ **Oferece controle manual** via botões integrados (BitDogLab)
- ✅ **Implementa comunicação serial** para debug e monitoramento
- ✅ **Usa múltiplos protocolos de comunicação** (I2C, GPIO, PIO)

---

## 🔧 Plataforma Alvo

- **Microcontrolador**: Raspberry Pi Pico W (RP2040)
- **SDK**: Pico SDK v2.2.0
- **Linguagem**: C11
- **Compilador**: ARM GCC 14.2 Release 1
- **Build Tool**: CMake 3.13+ com Ninja

### Especificações do RP2040
- **CPU**: ARM Cortex-M0+ dual-core @ 125 MHz
- **RAM**: 264 KB SRAM
- **Flash**: 2 MB
- **GPIO**: 28 pinos (26 disponíveis para uso)
- **Periféricos**: I2C, SPI, UART, ADC, PIO, PWM

---

## 🔌 Mapeamento de Pinos

### Comunicação I2C

#### **I2C0** (Barrramento 0) - Sensor de Luminosidade BH1750
| Pino RP2040 | Sinal | Sensor BH1750 | Descrição |
|---|---|---|---|
| **GPIO 0 (GP0)** | SDA | PIN 4 (SDA) | Dados |
| **GPIO 1 (GP1)** | SCL | PIN 6 (SCL) | Clock |
| GND | GND | PIN 2 (GND) | Terra |
| 3V3 | VCC | PIN 1 (VCC) | Alimentação |

**Frequência**: 400 kHz (modo padrão I2C)
**Endereço I2C**: 0x23 (padrão) ou 0x5C (alternativo - se ADDR em HIGH)

#### **I2C1** (Barramento 1) - Display OLED SSD1306
| Pino RP2040 | Sinal | Display SSD1306 | Descrição |
|---|---|---|---|
| **GPIO 14 (GP14)** | SDA | PIN 4 (SDA) | Dados |
| **GPIO 15 (GP15)** | SCL | PIN 3 (SCL) | Clock |
| GND | GND | PIN 2 (GND) | Terra |
| 3V3 | VCC | PIN 1 (VCC) | Alimentação |

**Frequência**: 400 kHz
**Endereço I2C**: 0x3C
**Resolução**: 128x64 pixels
**Protocolo**: OLED monocromático (preto e branco)

### Outras Saídas

#### **Matriz de LEDs WS2812**
| Pino RP2040 | Sinal | WS2812 | Descrição |
|---|---|---|---|
| **GPIO 7 (GP7)** | DIN | PIN 1 (DIN) | Dados série (sinal PWM) |
| 5V | VCC | PIN 2 (VCC) | Alimentação (requer potência adequada) |
| GND | GND | PIN 3 (GND) | Terra |

**Quantidade**: 25 LEDs (matriz 5x5)
**Protocolo**: WS2812 (NeoPixel/Addressable RGB)
**Implementação**: PIO (Programmable I/O) - microcontrolador a 800 kHz

### Controle de Entrada

#### **Botões BitDogLab**
| Pino RP2040 | Botão | Descrição |
|---|---|---|
| **GPIO 5 (GP5)** | **Botão A** | **Desativa matriz de LEDs** |
| **GPIO 6 (GP6)** | **Botão B** | **Ativa matriz de LEDs** |

**Tipo**: Pull-up interno habilitado
**Acionamento**: Nível baixo (0 = pressionado)
**Debounce**: 300 ms por software

### Comunicação Serial

#### **UART0** (Serial via USB)
- **Habilitada**: Sim (stdio via USB e UART)
- **Baud Rate**: 115200 bps
- **Uso**: Debug, mensagens de status, leituras de sensores
- **Conectar**: Porta USB-C do Pico ou UART0 (GP0/GP1)

---

## 🖥️ Protocolo I2C

### Configuração
```c
// I2C0: BH1750
i2c_init(i2c0, 400 * 1000);      // 400 kHz
gpio_set_function(0, GPIO_FUNC_I2C);
gpio_set_function(1, GPIO_FUNC_I2C);

// I2C1: SSD1306
i2c_init(i2c1, 400 * 1000);      // 400 kHz
gpio_set_function(14, GPIO_FUNC_I2C);
gpio_set_function(15, GPIO_FUNC_I2C);
```

### Operações I2C Utilizadas

#### **Leitura Bloqueante** (BH1750)
```c
int result = i2c_read_blocking(i2c0, 0x23, data, 2, false);
// Lê 2 bytes do sensor em 0x23
```

#### **Escrita Bloqueante** (BH1750, SSD1306)
```c
int result = i2c_write_blocking(i2c0, 0x23, &cmd, 1, false);
// Envia 1 byte de comando ao sensor em 0x23
```

### Scanner I2C Integrado
Ao inicializar, o programa executa uma varredura de todos os endereços I2C (0x00-0x7F) em ambos os barramentos para detectar e listar dispositivos conectados.

---

## 📡 Sensores e Periféricos

### 1️⃣ Sensor de Luz - BH1750

**Fabricante**: Rohm Semiconductor  
**Tipo**: Sensor de luminosidade (luxímetro digital)  
**Protocolo**: I2C (barramento I2C0)  
**Endereço**: 0x23 (padrão) ou 0x5C (alternativo)

#### Especificações
| Parâmetro | Valor |
|---|---|
| Faixa de Medição | 1 - 65535 lux |
| Resolução Modo Alta | 0.5 a 1.0 lux |
| Resolução Modo Baixa | 4 lux |
| Tempo de Medição | 16 ms (modo baixo) a 120 ms (modo alto) |
| Alimentação | 2.4V - 3.6V (típico 3.3V) |
| Consumo | ~0.16 mA (operação contínua) |

#### Modos de Operação Implementados
```c
#define BH1750_POWER_DOWN           0x00  // Desliga (economia de energia)
#define BH1750_POWER_ON             0x01  // Liga
#define BH1750_RESET                0x07  // Reset

#define BH1750_CONTINUOUS_HIGH_RES_MODE  0x10  // 1 lx, 120ms (ATUAL)
#define BH1750_CONTINUOUS_HIGH_RES_MODE2 0x11  // 0.5 lx, 120ms
#define BH1750_CONTINUOUS_LOW_RES_MODE   0x13  // 4 lx, 16ms
```

**Modo Utilizado**: `CONTINUOUS_HIGH_RES_MODE` (0x10)
- Resolução: 1 lux
- Tempo de atualização: ~120 ms
- Recomendado para uso em tempo real

#### Fórmula de Conversão
```c
uint16_t raw_value = (data[0] << 8) | data[1];
float lux = raw_value / 1.2f;
```

#### Calibração por Luminosidade Ambiente
O programa agrupa a luminosidade em faixas de ambiente típicas:
```
Valor Lux | Ambiente | Descrição
0-50      | Escuro   | Noite, ambiente sem iluminação
50-200    | Pouco    | Ambiente interior com iluminação fraca
200-500   | Moderado | Ambiente interior bem iluminado
500+      | Claro    | Luz solar ou iluminação forte
```

### 2️⃣ Display OLED - SSD1306

**Fabricante**: Solomon Systech  
**Tipo**: Display OLED monocromático  
**Protocolo**: I2C (barramento I2C1)  
**Endereço**: 0x3C (padrão)

#### Especificações
| Parâmetro | Valor |
|---|---|
| Resolução | 128 x 64 pixels |
| Tamanho | 0.96 polegadas (diagonal) |
| Cores | Preto e Branco |
| Tipo de Tela | OLED (Auto-emissiva) |
| Alimentação | 3.3V - 5V |
| Consumo | ~10 mA (típico) |

#### Inicialização
```c
ssd1306_init(&display, i2c1, 0x3C);
```

#### Operações Implementadas
```c
ssd1306_clear(&display);                    // Limpa tela
ssd1306_show(&display);                     // Atualiza display
ssd1306_draw_pixel(&display, x, y, on);     // Desenha pixel
ssd1306_draw_char(&display, x, y, 'A');    // Desenha caractere (5x8)
ssd1306_draw_string(&display, x, y, "Txt"); // Desenha string
```

#### Buffer de Tela
- Tamanho: 128 x 64 bits = 1024 bytes
- Organizado em 8 páginas de 128 bytes
- Cada byte representa 8 pixels verticalmente

#### Fontes Disponíveis
- **Fonte 5x8**: 96 caracteres (ASCII 32-127)
- Inclui números, letras maiúsculas/minúsculas, símbolos

---

### 3️⃣ Matriz de LEDs WS2812 (NeoPixel)

**Tipo**: LED RGB Endereçável (Smart LED)  
**Quantidade**: 25 LEDs (matriz 5x5)  
**Protocolo**: WS2812 (1-wire, 800 kHz)  
**Pino**: GPIO 7

#### Especificações
| Parâmetro | Valor |
|---|---|
| Tensão Operacional | 5V (recomendado) |
| Corrente por LED | ~60 mA (white, máx) |
| Corrente Total | ~1.5 A (25 LEDs brancos no máximo) |
| Tempo de Bit | 1.25 µs |
| Frequência de Dados | 800 kHz |

#### Implementação com PIO (Programmable I/O)
```c
#define PIO_FREQ 800000  // 800 kHz para WS2812
uint offset = pio_add_program(pio0, &ws2812_program);
ws2812_program_init(pio0, sm, offset, gpio_pin, 800_000, false);
```

**Por que PIO?** O timing crítico do WS2812 exige controle de hardware preciso que a CPU não pode garantir com interrupções. O PIO executa o protocolo de forma determinística em hardware dedicado.

#### Cores e Intensidades
A implementação usa **cores brancas** (R=G=B) com intensidades variáveis:

```c
Intensidade | Nível | RGB Value | Descrição
OFF         | 0     | 0x000000  | Desligado
LOW         | 3     | 0x030303  | 1% (fraco - notável)
MEDIUM      | 8     | 0x080808  | 3% (médio)
HIGH        | 25    | 0x191919  | 10% (forte - bem visível)
```

#### Lógica de Intensidade por Luminosidade
```
Ambiente Lux | Estado LED | Objetivo
0-50         | HIGH (10%) | Auxiliar em ambiente escuro
50-200       | MEDIUM (3%)| Indicador em ambiente fraco
200-500      | LOW (1%)   | Indicador suave em ambiente claro
500+         | OFF        | Desligado em ambiente muito claro
```

**Racional**: Lógica inversa - quanto mais luz natural, menos LEDs precisam acender para poupar energia e evitar ofuscação.

---

## 🎮 Controle de Interface

### Botões BitDogLab

#### Botão A (GPIO 5)
- **Função**: Desativar matriz de LEDs
- **Estado**: LED OFF mesmo se houver detecção de luz
- **Debounce**: 300 ms

#### Botão B (GPIO 6)
- **Função**: Reativar matriz de LEDs
- **Estado**: LEDs respondem normalmente à luminosidade
- **Debounce**: 300 ms

#### Comportamento
```
Estado Inicial: LEDs ATIVADOS
├─ Pressionar Botão A → LEDs DESATIVADOS
│  └─ Exibe "Status: OFF" no display
└─ Pressionar Botão B → LEDs ATIVADOS
   └─ Exibe "Status: ON" no display
```

---

## 📊 Fluxo Principal de Execução

```
1. INICIALIZAÇÃO (3 segundos)
   ├─ Configurar GPIO (botões)
   ├─ Inicializar I2C0 (400 kHz)
   ├─ Inicializar I2C1 (400 kHz)
   ├─ Executar Scanner I2C (ambos barramentos)
   ├─ Inicializar Display SSD1306
   ├─ Inicializar Sensor BH1750
   ├─ Inicializar Matriz WS2812 (PIO)
   └─ Exibir "Monitor Ambiental" no display

2. LOOP PRINCIPAL (atualização a cada 200 ms)
   ├─ [A] Verificar Botão A → Desativa LEDs
   ├─ [B] Verificar Botão B → Ativa LEDs
   ├─ Ler sensor BH1750
   │  ├─ Obter valor em lux
   │  ├─ Converter para intensidade
   │  └─ Se LEDs ativados: aplicar intensidade
   ├─ Atualizar Display SSD1306
   │  ├─ "Luminosidade:"
   │  ├─ "Luz: XX.X lux"
   │  ├─ "LED: [Fraco/Médio/Forte/Desligado]"
   │  └─ "Status: [ON/OFF]"
   ├─ Enviar dados via UART (debug)
   └─ Aguardar 200 ms
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

### Teste 1: Verificar Inicialização
1. Compilar e fazer flash do firmware
2. **Conectar Monitor Serial** (115200 baud)
   - VS Code: Usar a extensão "Serial Port Monitor"
   - Ou: `putty.exe` com 115200 baud
3. **Observar**:
   - ✅ Scanner I2C encontra 0x23 (BH1750) em I2C0
   - ✅ Scanner I2C encontra 0x3C (SSD1306) em I2C1
   - ✅ Display mostra "Monitor Ambiental"
   - ✅ Mensagens "[OK]" para todos os componentes

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

### Teste 3: Controle de Botões
1. **Pressionar Botão A** (GPIO 5)
   - LEDs apagam
   - Display: "Status: OFF"
   - Serial: "[BTN A] Matriz de LEDs DESATIVADA"

2. **Pressionar Botão B** (GPIO 6)
   - LEDs retomam brilho conforme luz ambiente
   - Display: "Status: ON"
   - Serial: "[BTN B] Matriz de LEDs ATIVADA"

### Teste 4: Verificar I2C
Execute o scanner I2C manualmente:
```c
i2c_scan(i2c0, "I2C0 (GP0/GP1)");
i2c_scan(i2c1, "I2C1 (GP14/GP15)");
```

Esperado:
- I2C0: **Encontrado 1 dispositivo** (0x23)
- I2C1: **Encontrado 1 dispositivo** (0x3C)

### Teste 5: Performance e Responsividade
- **Latência de Atualização**: ~200 ms
- **Tempo de Resposta dos Botões**: ~300 ms (debounce)
- **Frequência de Leitura**: 5 Hz (200 ms)
- **Taxa de Atualização Display**: 5 Hz

---

## 📦 Estrutura de Arquivos

```
projeto/
├─ MonitorAmbiental.c          # Programa principal
├─ CMakeLists.txt              # Configuração CMake
├─ pico_sdk_import.cmake       # Import Pico SDK
├─ README.md                   # Este arquivo
│
├─ Sensor BH1750
│  ├─ bh1750.h                 # Header
│  └─ bh1750.c                 # Implementação
│   └─ Funções:
│       ├─ bh1750_init()       # Inicializar
│       ├─ bh1750_read_light() # Ler luminosidade
│       ├─ bh1750_power_on()   # Ligar
│       └─ bh1750_power_down() # Desligar
│
├─ Display SSD1306
│  ├─ ssd1306.h                # Header
│  └─ ssd1306.c                # Implementação
│   └─ Funções:
│       ├─ ssd1306_init()      # Inicializar
│       ├─ ssd1306_clear()     # Limpar
│       ├─ ssd1306_show()      # Atualizar
│       ├─ ssd1306_draw_pixel()# Pixel
│       ├─ ssd1306_draw_char() # Caractere
│       └─ ssd1306_draw_string()# String
│
├─ Matriz de LEDs WS2812
│  ├─ led_matrix.h             # Header
│  ├─ led_matrix.c             # Implementação
│  └─ ws2812.pio               # Programa PIO (protocolo WS2812)
│   └─ Funções:
│       ├─ led_matrix_init()
│       ├─ led_matrix_set_intensity()
│       ├─ led_matrix_clear()
│       └─ led_matrix_get_intensity_from_lux()
│
└─ build/                       # Diretório de build
   ├─ MonitorAmbiental.elf     # Executável
   ├─ MonitorAmbiental.uf2     # Firmware
   └─ ... (arquivos CMake)
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
- Aumentar debounce (300 ms)
- Testar com LED serial `printf()`

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

## 📚 Recursos e Documentação

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

## 📝 Versão

- **Versão do Projeto**: 0.1 (Protótipo)
- **SDK Pico**: 2.2.0
- **Data**: Dezembro 2025
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
- [x] Display SSD1306 exibe dados
- [x] Matriz WS2812 responde à luminosidade
- [x] Botões A e B controlam LEDs
- [x] Comunicação UART/USB em 115200 baud
- [x] Responsividade satisfatória (200 ms)
- [x] Consumo de energia otimizado

---

## 🔮 Melhorias Futuras

- [ ] Implementar FreeRTOS para multitarefa
- [ ] Adicionar leitura de temperatura/umidade (DHT22, BMP280)
- [ ] Sincronização com WiFi (Pico W)
- [ ] Log de dados em SD card
- [ ] Dashboard web para monitoramento remoto
- [ ] Modos de economia de energia (sleep mode)
- [ ] Padrões de animação na matriz de LEDs
- [ ] Calibração automática de sensores
- [ ] Interface gráfica avançada no display

---

**Desenvolvido com ❤️ para aprendizado em microcontroladores**
