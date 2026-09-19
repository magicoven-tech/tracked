import { app, BrowserWindow, ipcMain } from 'electron';
import path from 'path';
import { fileURLToPath } from 'url';
import createAedes from 'aedes';
import net from 'net';
import http from 'http';
import fs from 'fs';
import os from 'os';
import { Bonjour } from 'bonjour-service';

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

let mainWindow = null;
let aedesInstance = null;
let mqttServer = null;
let bonjour = null;

// Função para descobrir o IP local IPv4 do Mac
function getMacLocalIp() {
  const interfaces = os.networkInterfaces();
  for (const name of Object.keys(interfaces)) {
    for (const iface of interfaces[name]) {
      if (iface.family === 'IPv4' && !iface.internal) {
        return iface.address;
      }
    }
  }
  return '127.0.0.1';
}

const macIp = getMacLocalIp();

// Initialize Embedded MQTT Broker (Aedes) on port 1883 + mDNS Bonjour Service
async function startMqttBroker() {
  try {
    aedesInstance = createAedes();
    mqttServer = net.createServer(aedesInstance.handle);

    const PORT = 1883;
    mqttServer.listen(PORT, '0.0.0.0', () => {
      console.log(`=======================================================`);
      console.log(`[MQTT Broker] Aedes rodando na porta ${PORT} (0.0.0.0)`);
      console.log(`[Mac IP Local] 👉 ${macIp}`);
      console.log(`=======================================================`);

      try {
        bonjour = new Bonjour();
        bonjour.publish({
          name: 'magictracked',
          type: 'mqtt',
          port: PORT,
          txt: { service: 'MagicTracked MQTT Broker' }
        });
        console.log('[mDNS] Serviço Bonjour "magictracked.local" (MQTT) anunciado na rede local!');
      } catch (e) {
        console.warn('[mDNS] Aviso ao registrar serviço Bonjour:', e);
      }
    });

    aedesInstance.on('client', (client) => {
      console.log(`[MQTT Broker] ESP32 Conectado! Client ID: ${client ? client.id : 'desconhecido'}`);
    });

    aedesInstance.on('clientDisconnect', (client) => {
      console.log(`[MQTT Broker] ESP32 Desconectado: ${client ? client.id : 'desconhecido'}`);
    });

    mqttServer.on('error', (err) => {
      console.warn('[MQTT Broker] Aviso no servidor MQTT (net):', err.message);
    });

    // Setup WebSocket server on port 1884 for Web browser clients (e.g. localhost:5173)
    let wss = null;
    try {
      const { WebSocketServer } = await import('ws');
      wss = new WebSocketServer({ port: 1884 });
      wss.on('error', (err) => {
        console.warn('[WebSocket Server] Aviso no servidor WebSocket:', err.message);
      });
      console.log('[WebSocket Server] Broadcast em ws://0.0.0.0:1884 ativo!');
    } catch (e) {
      console.warn('[WebSocket Server] Não foi possível iniciar WebSocket server:', e.message);
    }

    // Listen to incoming published messages
    aedesInstance.on('publish', (packet, client) => {
      if (!packet || !packet.topic) return;
      
      const topic = packet.topic.toString().trim();
      if (topic.startsWith('$SYS')) return;

      const payloadStr = packet.payload ? packet.payload.toString('utf8').trim() : '';

      console.log(`[MQTT RX] Tópico: ${topic} | Payload: ${payloadStr}`);

      // 1. Forward remote control commands to Electron renderer window
      if (mainWindow && !mainWindow.isDestroyed() && topic.startsWith('magictracked/cmd/')) {
        mainWindow.webContents.send('mqtt-command', {
          topic,
          payload: payloadStr
        });
      }

      // 2. Broadcast to Web Browser clients via WebSocket
      if (wss && topic.startsWith('magictracked/cmd/')) {
        const msgJson = JSON.stringify({ topic, payload: payloadStr });
        wss.clients.forEach((wsClient) => {
          if (wsClient.readyState === 1) { // OPEN
            wsClient.send(msgJson);
          }
        });
      }
    });

  } catch (err) {
    console.error('[MQTT Broker] Erro ao iniciar servidor MQTT:', err);
  }
}

function createWindow() {
  mainWindow = new BrowserWindow({
    width: 1280,
    height: 800,
    minWidth: 900,
    minHeight: 600,
    title: `MagicTracked - Remotely Controlled Visual Studio (IP do Mac: ${macIp})`,
    webPreferences: {
      preload: path.join(__dirname, 'preload.js'),
      contextIsolation: true,
      nodeIntegration: false,
      sandbox: false
    }
  });

  // Enable Camera access permissions automatically in Electron
  mainWindow.webContents.session.setPermissionCheckHandler((webContents, permission) => {
    if (permission === 'media') return true;
    return true;
  });

  mainWindow.webContents.session.setDevicePermissionHandler((details) => {
    if (details.deviceType === 'media') return true;
    return true;
  });

  const isDev = process.env.NODE_ENV !== 'production' && !app.isPackaged;
  if (isDev) {
    mainWindow.loadURL('http://localhost:5173').catch(() => {
      console.log('[Electron] Port 5173 não respondeu, tentando http://localhost:5174...');
      mainWindow.loadURL('http://localhost:5174');
    });
  } else {
    mainWindow.loadFile(path.join(__dirname, '../dist/index.html'));
  }

  mainWindow.on('closed', () => {
    mainWindow = null;
  });
}

// Servidor Web HTTP estático embutido para produção (permite clientes web externos na porta 5173)
function startStaticWebServer() {
  const isDev = process.env.NODE_ENV !== 'production' && !app.isPackaged;
  if (isDev) return;

  const WEB_PORT = 5173;
  const distPath = path.join(__dirname, '../dist');

  const mimeTypes = {
    '.html': 'text/html',
    '.js': 'text/javascript',
    '.css': 'text/css',
    '.json': 'application/json',
    '.png': 'image/png',
    '.jpg': 'image/jpeg',
    '.gif': 'image/gif',
    '.svg': 'image/svg+xml',
    '.ico': 'image/x-icon',
    '.woff': 'font/woff',
    '.woff2': 'font/woff2'
  };

  try {
    webHttpServer = http.createServer((req, res) => {
      let reqUrl = (req.url || '/').split('?')[0];
      if (reqUrl === '/') reqUrl = '/index.html';
      const filePath = path.normalize(path.join(distPath, reqUrl));

      if (!filePath.startsWith(distPath)) {
        res.writeHead(403);
        return res.end('Access Denied');
      }

      fs.stat(filePath, (err, stats) => {
        if (err || !stats.isFile()) {
          const indexPath = path.join(distPath, 'index.html');
          fs.readFile(indexPath, (err2, data) => {
            if (err2) {
              res.writeHead(404);
              return res.end('Not Found');
            }
            res.writeHead(200, { 'Content-Type': 'text/html' });
            res.end(data);
          });
        } else {
          const ext = path.extname(filePath).toLowerCase();
          const contentType = mimeTypes[ext] || 'application/octet-stream';
          fs.readFile(filePath, (err3, data) => {
            if (err3) {
              res.writeHead(500);
              return res.end('Internal Error');
            }
            res.writeHead(200, { 'Content-Type': contentType });
            res.end(data);
          });
        }
      });
    });

    webHttpServer.listen(WEB_PORT, '0.0.0.0', () => {
      console.log(`[Web Server] Servidor Web estático ativo em http://0.0.0.0:${WEB_PORT}`);
    });

    webHttpServer.on('error', (err) => {
      console.warn('[Web Server] Aviso ao iniciar servidor Web estático:', err.message);
    });
  } catch (err) {
    console.error('[Web Server] Erro ao iniciar servidor HTTP:', err);
  }
}

// IPC Listener to publish status back to MQTT (for updating ESP32 LCD display)
ipcMain.on('publish-status', (event, { topic, payload }) => {
  if (aedesInstance) {
    aedesInstance.publish({
      topic: topic || 'magictracked/status/app',
      payload: Buffer.from(typeof payload === 'object' ? JSON.stringify(payload) : String(payload)),
      qos: 0,
      retain: false
    });
  }
});

app.whenReady().then(() => {
  startMqttBroker();
  startStaticWebServer();
  createWindow();

  app.on('activate', () => {
    if (BrowserWindow.getAllWindows().length === 0) {
      createWindow();
    }
  });
});

app.on('window-all-closed', () => {
  if (bonjour) {
    bonjour.unpublishAll();
    bonjour.destroy();
  }
  if (mqttServer) {
    mqttServer.close();
  }
  if (webHttpServer) {
    webHttpServer.close();
  }
  if (process.platform !== 'darwin') {
    app.quit();
  }
});
