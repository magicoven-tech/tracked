# Walkthrough — Menu Universal e Sincronização de Alarmes

Evoluímos o sistema do **Tracked** com navegação aprimorada no menu de configurações físicas (através dos Botões 1, 2, 3 e 4), isolamos as modificações em variáveis temporárias até o salvamento definitivo, sincronizamos todas as configurações adicionando novos alarmes pela placa, redesenhamos a visualização de alarmes no painel web, configuramos o hardware para rodar de forma 100% **Standalone** e adicionamos sincronização bidirecional do temporizador Pomodoro.

---

## O que foi alterado

### 1. Sincronização Bidirecional do Pomodoro (PWA <-> ESP32)
Implementamos uma comunicação robusta e espelhada para o temporizador:
- **PWA -> ESP32:** Ao iniciar, pausar, retomar ou parar o temporizador no site, comandos WebSocket correspondentes (`TMR:FOCUS`, `TMR:PAUSE`, `TMR:RESUME`, `TMR:STOP`) são transmitidos imediatamente para o chip, controlando o LCD.
- **ESP32 -> PWA:** Ao operar o temporizador usando os botões físicos (Botão 2 para Play/Pause, Botão 3 para Stop), o ESP32 transmite periodicamente seu estado (`STATE:FOCUS:MM:SS`, `STATE:DONE`, `STATE:IDLE`, etc.).
- **Sincronização no Boot/Reconexão:** Atualizamos o parser de mensagens do PWA em `web/app.js`. Se o usuário abrir a página do painel com um temporizador rodando de forma standalone no ESP32, o PWA assume o estado de foco/pausa correspondente, sincroniza os segundos restantes com precisão e inicia seu contador local na tela de forma contínua, mantendo o gráfico de progresso e o LCD em perfeito alinhamento.

### 2. Sistema Standalone (Independente do Web App)
O Tracked inicializa de forma autônoma:
- **Remoção de Bloqueios:** O display exibe a face piscando e a barra de status de forma autônoma logo após a inicialização, sem travar na tela de "Aguardando app..".

### 3. Mensagens Temporárias de Status do PWA (Conectado / Desconectado)
- **Conexão Estabelecida:** Exibe `" App Conectado! "` no evento `WStype_CONNECTED`.
- **Conexão Perdida:** Exibe `"App Desconectado"` no evento `WStype_DISCONNECTED`.
Os avisos permanecem por **3 segundos** e somem sozinhos.

### 4. Instruções de Portal Captivo no LCD (WiFiManager)
Se o robô ligar e não encontrar nenhuma rede Wi-Fi configurada (ou a conexão falhar):
- O LCD exibirá instruções claras instruindo o usuário a se conectar no ponto de acesso local gerado pelo ESP32 para configurar as credenciais:
  - Linha 1: `Conecte no WiFi:`
  - Linha 2: `Tracked-Setup   `

### 5. Layout de Grade Responsiva para Alarmes no PWA
Redesenhamos a interface de exibição de alarmes no painel web (`web/style.css`) para se adaptar de forma responsiva (grid de duas colunas em desktops, empilhado verticalmente em telas menores).

### 6. Seleção Inteligente de Alarmes Existentes + Próximo Disponível
- O potenciômetro mapeia a rotação para todos os alarmes que já possuem configuração e disponibiliza o **próximo slot disponível** (primeiro livre) para criação física.
- A exibição foi otimizada para `"Sel. Alarme: %02d "`, eliminando o truncamento de texto na tela de 16 colunas.

---

## Máquina de Estados de Navegação do Menu (Fluxo de Botões)

| Estado Atual (`currentSetupState`) | Ação Botão 1 (Cancelar) | Ação Botão 2 (Voltar) | Ação Botão 3 (Avançar) | Ação Botão 4 (Confirmar) |
| --- | --- | --- | --- | --- |
| `SETUP_SELECT_MENU` | Retorna à face (`SETUP_OFF`) | - | - | Entra no menu selecionado |
| `SETUP_POMO_FOCUS` | Retorna à face (`SETUP_OFF`) | `SETUP_SELECT_MENU` | `SETUP_POMO_SHORT_BREAK` | `SETUP_POMO_SHORT_BREAK` |
| `SETUP_POMO_SHORT_BREAK` | Retorna à face (`SETUP_OFF`) | `SETUP_POMO_FOCUS` | `SETUP_POMO_LONG_BREAK` | `SETUP_POMO_LONG_BREAK` |
| `SETUP_POMO_LONG_BREAK` | Retorna à face (`SETUP_OFF`) | `SETUP_POMO_SHORT_BREAK` | - (Não avança) | **Grava no Flash/Sync & Exita** |
| `SETUP_ALARM_SELECT` | Retorna à face (`SETUP_OFF`) | `SETUP_SELECT_MENU` | `SETUP_ALARM_HOUR` | `SETUP_ALARM_HOUR` |
| `SETUP_ALARM_HOUR` | Retorna à face (`SETUP_OFF`) | `SETUP_ALARM_SELECT` | `SETUP_ALARM_MINUTE` | `SETUP_ALARM_MINUTE` |
| `SETUP_ALARM_MINUTE` | Retorna à face (`SETUP_OFF`) | `SETUP_ALARM_HOUR` | `SETUP_ALARM_STATE` | `SETUP_ALARM_STATE` |
| `SETUP_ALARM_STATE` | Retorna à face (`SETUP_OFF`) | `SETUP_ALARM_MINUTE` | - (Não avança) | **Grava no Flash/Sync & Exita** |

---

## Validação e Testes Recomendados

1. **Sincronização PWA -> ESP32:** Abra o painel web, clique em "Iniciar Foco". Verifique se o LCD do ESP32 entra na tela de contagem do Pomodoro no mesmo instante.
2. **Sincronização ESP32 -> PWA:** Pressione o Botão 2 (Play/Pause) no ESP32 físico. Verifique se o PWA inicia o temporizador na tela e aciona a contagem regressiva automaticamente. Pressione o Botão 3 (Stop) no ESP32 e confirme que o timer no site também é parado.
3. **Sincronização no Re-boot / Conexão Tardia:** 
   - Feche a página do PWA no navegador.
   - Pressione o Botão 2 no ESP32 para iniciar a contagem (deixe rodar standalone).
   - Abra a página do painel web novamente e certifique-se de que o timer conecta, reconhece a contagem active no ESP32 e sincroniza o tempo restante com precisão de segundos na tela.