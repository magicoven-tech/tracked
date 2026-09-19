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
let webHttpServer = null;

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

import mqtt from 'mqtt';

// EMQX Cloud Credentials
const EMQX_HOST = 'mqtt://a1d0120f.ala.us-east-1.emqxsl.com:1883';
const EMQX_USER = 'tracked_user';
const EMQX_PASS = 'tufjow-racxof-roRxo9';

let emqxClient = null;

async function startMqttBroker() {
  try {
    // 1. Conecta o App Desktop ao Broker Unificado na Nuvem (EMQX Cloud)
    console.log(`[EMQX Cloud] Conectando Electron ao broker ${EMQX_HOST}...`);
    emqxClient = mqtt.connect(EMQX_HOST, {
      username: EMQX_USER,
      password: EMQX_PASS,
      clientId: 'Electron_Desktop_' + Math.random().toString(16).substring(2, 8)
    });

    emqxClient.on('connect', () => {
      console.log('=======================================================');
      console.log('[EMQX Cloud] Electron Desktop conectado com sucesso!');
      console.log('=======================================================');
      emqxClient.subscribe('magictracked/cmd/#');
      emqxClient.subscribe('magictracked/status/#');
    });

    emqxClient.on('message', (topic, packetPayload) => {
      if (!topic) return;
      const payloadStr = packetPayload ? packetPayload.toString('utf8').trim() : '';
      console.log(`[EMQX RX] Tópico: ${topic} | Payload: ${payloadStr}`);

      // Repassa os comandos para a janela 3D do Electron
      if (mainWindow && !mainWindow.isDestroyed() && topic.startsWith('magictracked/cmd/')) {
        mainWindow.webContents.send('mqtt-command', {
          topic,
          payload: payloadStr
        });
      }
    });

    emqxClient.on('error', (err) => {
      console.warn('[EMQX Cloud] Aviso de conexão:', err.message);
    });

    // 2. Broker Local Fallback (Aedes) mantido para desenvolvimento offline
    aedesInstance = createAedes();
    mqttServer = net.createServer(aedesInstance.handle);

    const PORT = 1883;
    mqttServer.listen(PORT, '0.0.0.0', () => {
      console.log(`[MQTT Broker Local] Aedes ouvindo na porta ${PORT}`);
    });

  } catch (err) {
    console.error('[MQTT Broker] Erro ao iniciar serviços MQTT:', err);
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

// IPC Listener to publish status back to MQTT (for updating ESP32 LCD display / Web App)
ipcMain.on('publish-status', (event, { topic, payload }) => {
  const payloadStr = typeof payload === 'object' ? JSON.stringify(payload) : String(payload);
  
  if (emqxClient && emqxClient.connected) {
    emqxClient.publish(topic || 'magictracked/status/app', payloadStr);
  }
  
  if (aedesInstance) {
    aedesInstance.publish({
      topic: topic || 'magictracked/status/app',
      payload: Buffer.from(payloadStr),
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
