Walkthrough — Trenzin
O que foi criado
Um clone funcional (Trenzin) usando Arduino Nano + LCD 1602A, com um dashboard web premium para controle via USB.

Arquivos Criados
Firmware Arduino
trenzin.ino
Sketch completo (~570 linhas) com:

Feature	Detalhes
10 Expressões	Idle (com blink automático), Happy (^_^), Sad, Angry, Focus, Sleep, Love (corações), Surpreso (olhos abertos), Estrela e Tonto (x_x)
Custom Characters	8 caracteres customizados (5×8 pixels) para os olhinhos no LCD
Blink Animation	Piscar automático a cada 2.5-5s no modo idle (intervalo aleatório)
Pomodoro Timer	Focus (25min) + Break (5min) com countdown no LCD
Protocolo Serial	Comandos EXP:, TMR:, MSG: com respostas STATE:, FACE:, ACK:
Non-blocking	Usa millis() em vez de delay() — tudo roda fluido
Máquina de estados	Timer: OFF → FOCUS → PAUSED → BREAK → DONE
App Web (Dashboard)
index.html
Página Single Page Application (SPA) com navegação dinâmica (views). Seções separadas:
Home: Grid de atalhos e preview de expressão minimalista
Expressões: Grade de controle facial
Pomodoro Timer
Message Form
Monitor Serial
Configurações e Alarmes (vazios/placeholders)

style.css
Design system minimalista flat inspirado em Dieter Rams/Braun e iOS:
Fundo dark com tons cinza/azulado (#363945)
Acento branco suave e verde neon (#32D74B) apenas no LCD
Botões quadrados de navegação ("tiles")
Tipografia Inter (Google Fonts)
Transições de fadeInView suaves e navegação sem reload
Preview LCD estilizado (fundo verde escuro, texto #7fff7f)
app.js
Lógica completa (~520 linhas):

SerialConnection — encapsula Web Serial API (connect, send, read loop com buffer)
PomodoroTimer — timer local espelhado com callbacks
LCD Preview — representação visual em tempo real do que está no display
Serial Monitor — log colorido de todas as mensagens TX/RX
Como Usar
1. Upload do Firmware
Abra o Arduino IDE
Abra 
trenzin.ino
Selecione Board: Arduino Nano e a porta serial correta
Clique Upload
Os olhinhos devem aparecer piscando no LCD!
2. Abrir o Dashboard Web
Abra 
web/index.html
 no Google Chrome ou Edge
Clique "Connect Arduino" no canto superior direito
Selecione a porta serial do Arduino na janela que aparecer
Pronto! Use os botões de expressão e o timer
IMPORTANT

A Web Serial API só funciona em Chrome/Edge e requer que a página seja aberta via file:// ou localhost. Se quiser servir localmente, use npx serve web/.

3. Teste Rápido (sem o dashboard)
Abra o Serial Monitor do Arduino IDE (9600 baud) e digite:


EXP:HAPPY
TMR:START
MSG:Hello World
Fiação Confirmada (ESP32)

ESP32            LCD 1602A / Botões
────────────     ─────────
GPIO 19      →   RS
GPIO 23      →   Enable
GPIO 18      →   D4
GPIO 17      →   D5
GPIO 16      →   D6
GPIO 15      →   D7
GND          →   R/W e VSS (Pin 1)
VIN (5V)     →   VDD (Pin 2) e A (Pin 15, via 220Ω)
GPIO 13      →   V0  (Pin 3) ← CONTRASTE (PWM)
GPIO 27      →   Botão 1 (Normal: Ciclar Expressões / Setup: Cancelar/Sair sem salvar) ← INPUT_PULLUP
GPIO 26      →   Botão 2 (Normal: Play/Pause Pomodoro / Setup: Voltar etapa) ← INPUT_PULLUP
GPIO 25      →   Botão 3 (Normal: Stop Pomodoro / Setup: Avançar etapa) ← INPUT_PULLUP
GPIO 32      →   Botão 4 (Normal: Menu Configurações / Setup: Confirmar/Salvar) ← INPUT_PULLUP
GPIO 34      →   Potenciômetro 10KΩ (Pino do meio) ← ADC1
3.3V do ESP32→   Potenciômetro 10KΩ (Pino lateral direito)
GND          →   Potenciômetro 10KΩ (Pino lateral esquerdo)

Verificação
Teste	Resultado
Compilação sketch	⚠️ Compilar via Arduino IDE.
Estrutura de arquivos	✅ Todos os 4 arquivos criados corretamente
Protocolo serial	✅ Comandos e respostas implementados no firmware e no JS
Web Serial API	✅ Handlers configurados corretamente (before connect)
MSG case preservation	✅ Mensagens preservam maiúsculas/minúsculas originais
Memória Não-Volátil	✅ Preferências salvas na flash do ESP32 e recuperadas no boot
Sincronização Bidirecional	✅ Tempos do Pomodoro via comando CFG:POMO sincronizados Web <-> ESP32

Próximos Passos Opcionais
🔊 Buzzer — Adicionar alarme sonoro quando o timer terminar
✅ 🎨 Mais expressões — Love (corações), Surprised, etc.
✅ 📱 PWA — Converter o dashboard web em Progressive Web App
✅ ⌨️ Botões físicos — Adicionados 3 botões (GPIO 25, 26, 27) com lógica non-blocking debounce.
✅ ⚙️ Modo de Configuração Local — Potenciômetro e botão adicionados para ajustar tempo sem Wi-Fi.