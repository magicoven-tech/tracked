# Walkthrough — Menu Universal e Sincronização de Alarmes

Evoluímos o sistema do **Trenzin** com navegação aprimorada no menu de configurações físicas (através dos Botões 1, 2, 3 e 4), isolamos as modificações em variáveis temporárias até o salvamento definitivo, sincronizamos todas as configurações adicionando novos alarmes pela placa, redesenhamos a visualização de alarmes no painel web e configuramos o hardware para rodar de forma 100% **Standalone**.

---

## O que foi alterado

### 1. Sistema Standalone (Independente do Web App)
Agora o Trenzin inicializa, expressa emoções, pisca os olhos e roda os timers de forma independente logo no boot, sem precisar que um cliente web se conecte:
- **Remoção de Bloqueios:** Removemos as checagens `connectedClients == 0` que impediam a renderização de `drawFace()` e `drawStatusLine()`.
- **Limpeza do Loop:** Removemos a tela de "Aguardando app.." que travava o LCD no loop principal enquanto nenhum cliente web estava conectado.

### 2. Mensagem Temporária de Conexão do PWA
Sempre que um usuário abrir a página do painel web (`http://trenzin.local`) e se conectar ao robô:
- O sistema intercepta o evento de conexão WebSocket (`WStype_CONNECTED`) e dispara uma mensagem temporária de **3 segundos** no display LCD: `" App Conectado! "`.
- Após os 3 segundos, a tela volta a renderizar a barra de status ou o Pomodoro ativo sem qualquer interrupção.

### 3. Instruções de Portal Captivo no LCD (WiFiManager)
Se o robô ligar e não encontrar nenhuma rede Wi-Fi configurada (ou a conexão falhar):
- Configuramos um callback (`configModeCallback`) no `WiFiManager`.
- O LCD exibirá instruções claras instruindo o usuário a se conectar no ponto de acesso local gerado pelo ESP32 para configurar as credenciais:
  - Linha 1: `Conecte no WiFi:`
  - Linha 2: `Trenzin-Setup   `

### 4. Layout de Grade Responsiva para Alarmes no PWA
Redesenhamos a interface de exibição de alarmes no painel web (`web/style.css`) para se adaptar de forma responsiva (grid de duas colunas em desktops, empilhado verticalmente em telas menores).

### 5. Seleção Inteligente de Alarmes Existentes + Próximo Disponível
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

1. **Inicialização Autônoma:** Faça o upload do firmware. Assim que o Wi-Fi conectar, os olhinhos devem aparecer piscando de forma autônoma no LCD, sem abrir o navegador.
2. **Teste do Portal de Configuração:**
   - Descomente temporariamente `wifiManager.resetSettings();` no código ou configure uma rede inválida para forçar o portal.
   - Verifique se o LCD muda para `Conecte no WiFi: / Trenzin-Setup` de forma informativa.
3. **Mensagem de Conexão:** 
   - Com o robô ligado e piscando, abra a página `http://trenzin.local`.
   - Assim que o site conectar, confirme que a mensagem `" App Conectado! "` aparece no LCD por 3 segundos antes de retornar à tela normal.