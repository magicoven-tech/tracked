# 🤖 Trenzin — Seu companheiro de mesa inteligente

> Criado com muito amor e café pela [MagicOven](https://magicoven.tech). ☕

O **Trenzin** é um assistente de mesa com uma interface de expressões fofa, integrado a ferramentas de produtividade. Ele é equipado com um display LCD 1602A, operado por um microcontrolador da família ESP (ESP32/ESP8266) com conectividade Wi-Fi e um Dashboard Web direto no seu navegador.

---

## ✨ Features

- **Expressões interativas:** O rosto do Trenzin muda para refletir estados como feliz, triste, foco, dormindo ou neutro (com direito a piscadas automáticas!).
- **Temporizador oomodoro (focus timer):** Chega de se distrair. Inicie o cronômetro pelo celular e deixe o Trenzin guiar suas sessões de foco (25 min) e pausas (5 min) direto no visor LCD.
- **Mensagens customizadas:** Mande um texto rápido para o visor do Trenzin através do painel de controle.
- **Dashboard Web:** Uma interface linda, responsiva que se comporta como um aplicativo nativo no seu celular.
- **Setup automático de Wi-Fi:** Não precisa mexer no código para trocar a senha do Wi-Fi! O Trenzin cria um portal inteligente para você escolher sua rede.

---

## 🛠️ Hardware necessário

- 1x ESP32 (ou compatível com conectividade Wi-Fi)
- 1x Display LCD 1602A (com interface paralela padrão)
- 1x Potenciômetro de 10kΩ (para ajustar o contraste do LCD)
- Fios e jumpers

**Pinagem padrão (LiquidCrystal):**
| LCD Pin | Nome | Conexão ESP32 |
| :---: | :---: | :---: |
| 3 | V0 | Pino 13 (PWM para Contraste) |
| 4 | RS | Pino 19 |
| 6 | E | Pino 23 |
| 11 | D4 | Pino 18 |
| 12 | D5 | Pino 17 |
| 13 | D6 | Pino 16 |
| 14 | D7 | Pino 15 |

*(Se o seu microcontrolador for diferente, ajuste os pinos no topo do arquivo `trenzin.ino`)*

---

## 🚀 Primeiro acesso & configuração

O Trenzin foi feito para ser amigável. Para usar na casa de um amigo ou em um escritório novo, você não precisa reinstalar nenhum código. Siga estes passos:

1. **Ligue o Trenzin na energia.** (Pode ser no USB do computador ou em um carregador de celular).
2. Como ele ainda não conhece o Wi-Fi do local, ele mesmo criará uma rede Wi-Fi própria (Access Point) chamada:  
   👉 **`trenzin-by-magicoven`**
3. Pegue seu celular ou computador e **conecte-se a essa rede Wi-Fi**.
4. Uma página de configuração abrirá automaticamente (Portal Cativo). Caso não abra, acesse `http://192.168.4.1` no seu navegador.
5. Na lista que aparecer, selecione a rede Wi-Fi da sua casa, coloque a senha e salve!
6. O Trenzin vai reiniciar sozinho e conectar na sua rede local. A partir desse momento, a tela dele exibirá a mensagem de que ele está aguardando o App.

---

## 📱 Usando o dashboard (App)

Uma vez conectado no seu Wi-Fi, controlar o Trenzin é a coisa mais fácil do mundo:

1. Abra o navegador do seu celular ou computador (que deve estar conectado **no mesmo Wi-Fi**).
2. Acesse:  
   👉 **[http://trenzin.local](http://trenzin.local)**
3. A página de controle carregará instantaneamente. Lá você pode:
   - Trocar a carinha do Trenzin no painel **Expressions**.
   - Digitar mensagens de até 16 caracteres e enviar para a tela em **Send message**.
   - Controlar as sessões de trabalho através do **Pomodoro timer**.

> **Dica de ouro:** Adicione a página `trenzin.local` à **Tela de início** do seu celular (iOS/Android). O design foi pensado para remover a barra de endereços do navegador e funcionar exatamente como um aplicativo nativo!

---

## 💻 Guia para desenvolvedores

Se você for modificar o código-fonte, saiba como o projeto está estruturado:

- **`/trenzin/trenzin.ino`**: O código principal em C++ responsável pelo controle do display LCD, WebSockets, portal Wi-Fi e Servidor Web HTTP.
- **`/web`**: Os arquivos do frontend da aplicação web (`index.html`, `style.css`, `app.js`).
- **`/build_web.js`**: O empacotador. Todo arquivo que está na pasta web precisa ser minificado e convertido para uma variável C++ para que o ESP32 possa servir a página na rede.

### Como modificar a página web:
Sempre que fizer qualquer alteração nos arquivos HTML, CSS ou JS da pasta `web/`, abra o terminal na raiz do projeto e rode:

```bash
node build_web.js
```

Isso vai ler todos os arquivos da web, empacotá-los e regerar automaticamente o arquivo `/trenzin/web_assets.h`. Feito isso, basta abrir a Arduino IDE e fazer o *upload* do `trenzin.ino` novamente.

---

<p align="center">
  Desenvolvido com café por <b>MagicOven</b>
</p>
