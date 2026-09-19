import { contextBridge, ipcRenderer } from 'electron';

contextBridge.exposeInMainWorld('electronAPI', {
  // Listen for remote control commands from ESP32 via MQTT
  onRemoteControl: (callback) => {
    ipcRenderer.on('mqtt-command', (event, data) => callback(data));
  },
  // Send status feedback to ESP32 LCD
  publishStatus: (topic, payload) => {
    ipcRenderer.send('publish-status', { topic, payload });
  }
});
