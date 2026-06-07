# Walkthrough — Menu Universal e Sincronização de Alarmes

Evoluímos o sistema do **Trenzin** com navegação aprimorada no menu de configurações físicas (através dos Botões 1, 2, 3 e 4), isolamos as modificações em variáveis temporárias até o salvamento definitivo, sincronizamos todas as configurações adicionando novos alarmes pela placa e redesenhamos a visualização de alarmes no painel web.

---

## O que foi alterado

### 1. Novo Layout de Grade Responsiva para Alarmes no PWA
Redesenhamos a interface de exibição de alarmes no painel web (`web/style.css`):
- **Antes (Scroll Horizontal):** Os cards de alarme ficavam alinhados lado a lado, exigindo rolagem horizontal em telas menores (o que tornava a visualização incompleta e poluída).
- **Depois (Grade Responsiva / Stacked):** Convertemos o layout de `.alarms-list` para usar **CSS Grid**. 
  - Em telas de computadores/tablets, os alarmes aparecem organizados em uma grade de duas colunas (`repeat(2, 1fr)`).
  - Em telas menores de celulares (abaixo de 480px de largura), a grade passa a ter apenas uma coluna (`1fr`), fazendo com que os alarmes empilhem verticalmente de forma elegante e limpa, idêntico aos botões de expressões.
- Compilamos as atualizações usando o `build_web.js` para atualizar o firmware do ESP32 (`trenzin/web_assets.h`).

### 2. Seleção Inteligente de Alarmes Existentes + Próximo Disponível
Resolvemos um bug crítico na seleção física de alarmes e implementamos a navegação dinâmica:
- **Navegação Dinâmica (Existentes + Próximo Disponível):** O potenciômetro agora mapeia a rotação para todos os alarmes que já possuem configuração e disponibiliza o **próximo slot disponível** (primeiro livre) para criação física.
- **Ajuste de Formatação (Sem Cortes):** Alteramos a exibição na tela de seleção para `"Sel. Alarme: %02d "`. Esta string possui exatamente 16 caracteres, garantindo que o número completo do alarme (`01` a `10`) apareça corretamente sem qualquer corte de caracteres na tela LCD de 16 colunas.

### 3. Navegação Física Completa no Menu de Configurações
- **Botão 1 (Cancelar) [GPIO 27]**: Cancela e sai das configurações sem salvar.
- **Botão 2 (Voltar) [GPIO 26]**: Volta para a etapa/tela anterior do menu.
- **Botão 3 (Avançar) [GPIO 25]**: Avança para a próxima etapa (se houver).
- **Botão 4 (Confirmar) [GPIO 32]**: Confirma e grava na última tela de cada menu.

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

1. **Compilação Web:** Rode `node build_web.js` para garantir que as alterações no CSS estão minificadas e salvas dentro do sketch do ESP32.
2. **Exibição Responsiva:** 
   - Abra a interface em `http://trenzin.local/` no computador e diminua o tamanho do navegador para testar a responsividade.
   - Os cartões de alarmes devem se autoajustar e empilhar para baixo em telas estreitas, sem causar rolagem horizontal na página.