# Tracked OS

O **Tracked** é uma plataforma de **Interação Humano-Máquina e Intervenção Artística** equipada com um display LCD, controle de visuais 3D por gestos (MediaPipe + WebGL/Three.js), temporizador Pomodoro e controle de parâmetros em tempo real. Desenvolvido para rodar em um **ESP32** integrado a uma aplicação Desktop e interface Web responsiva via WebSockets/MQTT.

---

## 🛠 Funcionalidades do sistema

- **Rostos e expressões:** O Tracked possui 10 expressões dinâmicas com "olhos" animados no LCD (usando caracteres customizados de 5x8). Ele pisca de forma aleatória quando ocioso (Idle) e conta com expressões de Feliz, Triste, Zangado, Focado (Focus), Dormindo (Sleep), Love (coração), Surpreso, Estrela e Tonto (X_X).
- **Temporizador Pomodoro:**
  - Foco (25 min padrão), pausa curta (5 min) e pausa longa (15 min).
  - **Sincronização Bidirecional em Tempo Real:** O estado ativo do temporizador (ativo, pausado, tempo restante) é sincronizado em tempo real entre o hardware (LCD) e múltiplos clientes Web via WebSockets. Se um temporizador for iniciado standalone direto no ESP32, o Web App se autoajusta e sincroniza o tempo restante ao se conectar.
- **Sistema de múltiplos alarmes:**
  - Suporta até 10 alarmes diferentes.
  - Permite configurar hora, minuto, título/nome personalizado e ligar/desligar individualmente pela interface web.
- **Botões Físicos (Hardware):**
  - Integração de 4 push buttons (GPIO 25, 26, 27 e 32) com *debounce non-blocking* para controle direto.
  - Permite iniciar/pausar e parar o Pomodoro, além de ciclar manualmente pelas expressões, tudo sem depender da interface Web.
- **Configuração Local e Memória Não-Volátil (NVRAM):**
  - Integração com Potenciômetro Linear 10KΩ (GPIO 34) e menu interativo no LCD para configurar os tempos do Pomodoro e os Alarmes fisicamente.
  - **Duração do Pomodoro:** Ajuste fino de Foco (1-60 min), Pausa Curta (1-30 min) e Pausa Longa (1-45 min) usando variáveis temporárias (as alterações só se tornam ativas ao salvar).
  - **Configuração de Alarmes:** Navegação inteligente via potenciômetro apenas pelos alarmes que existem/estão cadastrados no sistema (evitando poluição visual), permitindo editar a hora, minuto e status (LIGADO/DESLIGADO) de forma persistente.
  - **Salvamento Automático:** Todas as alterações feitas fisicamente são gravadas na memória Flash do chip através da biblioteca `<Preferences.h>`.
  - **Sincronização Bidirecional em Tempo Real:** Alterações feitas na interface Web atualizam o hardware instantaneamente (e vice-versa), mantendo os alarmes e os tempos do Pomodoro sincronizados em todos os dispositivos em tempo real.
- **Conectividade smart:**
  - **WiFiManager (Standalone & AP Mode):** Configuração de rede Wi-Fi através de portal captivo (`Tracked-Setup`). Se nenhuma rede estiver cadastrada, o LCD exibe instruções amigáveis indicando para qual rede local se conectar. O sistema roda de forma 100% autônoma logo após a conexão Wi-Fi (sem travar à espera do app).
  - **NTP time sync:** Atualização de horário automático baseado na rede (fuso horário UTC-3).
  - **mDNS:** Permite acessar a interface do controlador digitando `http://tracked.local` no navegador.
- **Web app integrado (SPA & PWA):**
  - Hospedado no próprio chip ESP32 (servidor Web embutido).
  - Interface desenvolvida em Vanilla JS, HTML e CSS (sem dependências pesadas de frameworks) com tema escuro elegante.
  - **Progressive Web App (PWA):** Instale o painel como um aplicativo nativo no celular ou desktop para ter ícone na tela inicial e acesso em tela cheia (standalone) como um app de verdade.

---

## 🔌 Hardware e diagrama de ligações

**Componentes:**
- 1x Placa de desenvolvimento ESP32 (NodeMCU-32S, ESP32-WROOM, etc.)
- 1x Display LCD 1602A (16x2 com backlight)
- 1x Potenciômetro Linear de 10KΩ (para ajustar os tempos no modo Configuração)
- *Opcional:* Potenciômetro extra para ajuste de contraste manual (estamos usando PWM via código no GPIO 13 no lugar do potenciômetro para ajuste via software).

**⚠️ IMPORTANTE (nível lógico):** 
O ESP32 trabalha com 3.3V, mas a tela LCD opera em 5V. Como *não* estamos usando um conversor de nível lógico, é **CRÍTICO** aterrar o pino R/W da tela para garantir que o LCD nunca envie 5V de volta para os pinos do ESP32 (funcionamento write-only).

**Esquema de ligação (modo paralelo de 4-bits):**

| Pino LCD 1602A | Nome LCD | Ligação no ESP32 | Observação |
|---|---|---|---|
| 1 | VSS | GND | Terra |
| 2 | VDD | 5V / VIN | Alimentação da tela |
| 3 | V0 / VO | GPIO 13 | Contraste gerado via PWM (`analogWrite`) |
| 4 | RS | GPIO 19 | Register Select |
| 5 | R/W | GND | **CRÍTICO:** Aterrar para proteger o ESP32 |
| 6 | E / EN | GPIO 23 | Enable |
| 11 | D4 | GPIO 18 | Data pin |
| 12 | D5 | GPIO 17 | Data pin |
| 13 | D6 | GPIO 16 | Data pin |
| 14 | D7 | GPIO 15 | Data pin |
| 15 | A | 5V / VIN | Anodo do backlight (pode usar um resistor de 220Ω) |
| 16 | K | GND | Cátodo do backlight |

**Ligação dos Botões Físicos (`INPUT_PULLUP`):**
A configuração dos botões não exige resistores externos. Basta conectar um terminal do botão no **GND** e o outro terminal no pino indicado:

| Componente | Ligação no ESP32 | Ação em Modo Normal | Ação em Modo Configuração (Setup) |
|---|---|---|---|
| Botão 1 | GPIO 27 | Ciclar entre todas as 10 expressões | Cancelar / Sair das configurações sem salvar |
| Botão 2 | GPIO 26 | Iniciar / Pausar Pomodoro | Voltar para a configuração/etapa anterior |
| Botão 3 | GPIO 25 | Parar Pomodoro / Limpar Mensagem ou Expressão | Avançar para a próxima configuração/etapa |
| Botão 4 | GPIO 32 | Menu de Configurações Universal (Pomodoro & Alarmes) | Confirmar seleção / Avançar (Salvar na última etapa) |

**Ligação do Potenciômetro (Ajuste de Tempo):**
| Componente (Pernas) | Ligação no ESP32 | Função |
|---|---|---|
| Perna 1 (Esquerda) | GND | Terra |
| Perna 2 (Meio) | GPIO 34 (ADC1) | Leitura Analógica |
| Perna 3 (Direita) | 3.3V | Alimentação (⚠️ *NUNCA* use 5V no ADC do ESP32) |

---

## 💻 Tecnologias utilizadas

- **C++ (Arduino core para ESP32):** Lógica do hardware e controle do display.
- **WebSocketsServer:** Para comunicação bidirecional em tempo real entre a interface web e o hardware.
- **Node.js (build_web.js):** Um script customizado criado para ler os arquivos web (HTML/CSS/JS), minificá-los, zipar e transformá-los num arquivo C++ (`web_assets.h`) usando matrizes `PROGMEM`.
- **Vanilla JS, HTML5, CSS3:** Stack frontend.
- **Lucide Icons:** Conjunto de ícones leves renderizados via SVG na interface gráfica.

---

## 🧗 Desafios e soluções no desenvolvimento

Para chegar na versão estável e fluida do Tracked OS 2.0, passamos por uma série de pedreiras técnicas notáveis:

1. **Gestão de espaço no ESP32 (frontend embutido):**
   * *Desafio:* Hospedar arquivos `.html`, `.css` e `.js` pesados na flash do ESP32 era inviável com a biblioteca `WebServer` padrão por causa da RAM, além de ser terrível desenvolver strings HTML dentro do código C++.
   * *Solução:* Separamos o ambiente de desenvolvimento web em uma pasta à parte e escrevemos o `build_web.js`. Sempre que o layout web muda, o script Node lê a pasta `/web`, remove espaços inúteis e injeta tudo em um arquivo `web_assets.h`.

2. **O bug do Pomodoro automático (race condition via WebSocket):**
   * *Desafio:* Sempre que o celular se conectava na rede ou atualizava a página, a sessão do Pomodoro iniciava sozinha, sem o usuário apertar "Iniciar".
   * *Solução:* Identificamos que a função JavaScript `onConnect` disparava para o servidor o comando `TMR:FOCUS`, achando que era apenas um "sync", mas o Arduino interpretava como "dar play". Removemos a linha ofensiva e implementamos um `sendTimerState()` no C++ para ser a "fonte da verdade".

3. **O inferno do cache mobile e a saga do PWA:**
   * *Desafio:* iPhones e navegadores Safari/Chrome continuavam exibindo páginas antigas devido ao cache persistente. Além disso, a Apple tem regras extremamente restritas para ícones PWA, ignorando SVGs e exigindo a entrega de arquivos PNG físicos.
   * *Solução:* Para o cache, adicionamos headers `Cache-Control: no-cache`. Para o PWA, criamos um Service Worker (`sw.js`) e um Manifesto. Além disso, atualizamos o `build_web.js` para ler um PNG convertido fisicamente, convertê-lo em um array de bytes hexadecimal (`PROGMEM`) e enviar o binário via C++ puro para fazer o `apple-touch-icon` brilhar na tela inicial dos usuários!

4. **100vh vs iOS viewport (o footer escondido):**
   * *Desafio:* A interface web no mobile ficava quebrada com o *footer* fixado embaixo da barra de endereços do iPhone (sendo obrigado a rolar a tela).
   * *Solução:* Substituímos o ultrapassado `100vh` pelas unidades dinâmicas modernas do CSS: `100dvh` (dynamic viewport height).

5. **Interface sem fio do 1602A (limitação de custom chars):**
   * *Desafio:* O chip padrão HD44780 do LCD só permite 8 caracteres customizados carregados na RAM ao mesmo tempo. 
   * *Solução:* Fizemos um sistema robusto de estado em `loadExpressionChars()` que substitui e redesenha os arrays na memória do LCD dinamicamente na transição entre expressões.

---

## 📦 Instalação do Aplicativo Desktop (.dmg no macOS - Sem Terminal)

Qualquer pessoa pode instalar e usar o **MagicTracked Desktop** no Mac sem precisar abrir o terminal ou instalar o Node.js:

1. Acesse a aba **Releases** do projeto no GitHub e baixe o arquivo `MagicTracked-1.0.0-arm64.dmg` (para Macs M1/M2/M3/M4) ou `MagicTracked-1.0.0-x64.dmg` (para Macs Intel).
2. Dê um duplo clique no arquivo `.dmg` baixado e **arraste o ícone do MagicTracked para a pasta Aplicações (Applications)**.
3. **Primeira Execução (Aviso de Desenvolvedor Não Verificado do macOS):**
   - Na primeira vez em que abrir o aplicativo, o macOS poderá exibir uma mensagem informando que o desenvolvedor não foi verificado.
   - Para abrir: **Clique com o botão direito (ou Control + Clique)** no ícone do MagicTracked na pasta Aplicações e selecione **Abrir** (ou vá em *Ajustes do Sistema > Privacidade e Segurança* e clique em *Abrir mesmo assim*).
4. **Pronto!** 
   - O aplicativo iniciará automaticamente o **Broker MQTT (porta 1883)**, o anúncio na rede (`magictracked.local`) e o **Servidor Web (porta 5173)**.
   - O controle **ESP32** se conectará sozinho ao app via Wi-Fi assim que for ligado.
   - Dispositivos na mesma rede (celulares/tablets) podem acessar a interface pelo navegador no endereço `http://<IP-DO-MAC>:5173`.

### 💡 Conectando ou Reconfigurando o Controle ESP32
- Na barra de título do aplicativo Desktop é exibido o **IP local do seu Mac** (ex: `IP do Mac: 192.168.X.X`).
- Caso o IP do Mac mude na sua rede Wi-Fi e o ESP32 não conecte automaticamente, ligue o ESP32 **mantendo o Botão MENU (GPIO 27) pressionado** para abrir o portal captivo `MagicTracked-Remote`.
- Conecte o celular na rede do ESP32 e atualize o campo **"IP do Mac (Broker MQTT)"** com o IP mostrado no aplicativo.

---

## 🚀 Como operar e modificar (Para Desenvolvedores)

1. Altere o código HTML/CSS/JS na pasta `web/`.
2. Rode o construtor usando o Node: `node build_web.js`.
3. Verifique se o `tracked/web_assets.h` foi gerado/atualizado.
4. Faça upload pelo IDE do Arduino (ou `arduino-cli`) para o ESP32 (`tracked.ino`).
5. Quando o controlador ligar, se ele nunca se conectou no seu Wi-Fi, um ponto de acesso **"Tracked-Setup"** aparecerá. Entre nele com seu celular para cadastrar sua rede.
6. Digite `http://tracked.local` no seu navegador!

<div align="center">
  <i>Criado com muito café pela MagicOven. ☕</i>
</div>
