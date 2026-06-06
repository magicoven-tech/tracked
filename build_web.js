const fs = require('fs');
const path = require('path');

function build() {
    const rootDir = __dirname;
    const webDir = path.join(rootDir, 'web');
    
    try {
        const html = fs.readFileSync(path.join(webDir, 'index.html'), 'utf8');
        const css = fs.readFileSync(path.join(webDir, 'style.css'), 'utf8');
        const js = fs.readFileSync(path.join(webDir, 'app.js'), 'utf8');
        const manifest = fs.readFileSync(path.join(webDir, 'manifest.json'), 'utf8');
        const sw = fs.readFileSync(path.join(webDir, 'sw.js'), 'utf8');
        const icon = fs.readFileSync(path.join(webDir, 'icon.svg'), 'utf8');
        
        let iconPngBytes = '';
        let iconPngLen = 0;
        try {
            const iconPng = fs.readFileSync(path.join(webDir, 'icon-192.png'));
            iconPngLen = iconPng.length;
            for (let i = 0; i < iconPngLen; i++) {
                iconPngBytes += '0x' + iconPng[i].toString(16).padStart(2, '0') + ', ';
                if (i % 16 === 15) iconPngBytes += '\n';
            }
        } catch (e) {
            console.warn("icon-192.png não encontrado. Execute o comando sips no terminal.");
        }

        // Minificação básica dos novos arquivos
        const manifestMin = JSON.stringify(JSON.parse(manifest)).replace('icon.svg', 'icon-192.png');
        const swMin = sw
            .replace(/\/\/.*$/gm, '')
            .replace(/\/\*[\s\S]*?\*\//g, '')
            .replace(/\s+/g, ' ')
            .replace('icon.svg', 'icon-192.png')
            .trim();
        const iconMin = icon
            .replace(/<!--[\s\S]*?-->/g, '')
            .replace(/\s+/g, ' ')
            .trim();

        let out = `// Arquivo gerado automaticamente
// NÃO EDITE MANUALMENTE!

#ifndef WEB_ASSETS_H
#define WEB_ASSETS_H

#include <pgmspace.h>

`;
        out += 'const char WEB_HTML[] PROGMEM = R"=====(\n' + html + ')=====";\n\n';
        out += 'const char WEB_CSS[] PROGMEM = R"=====(\n' + css + ')=====";\n\n';
        out += 'const char WEB_JS[] PROGMEM = R"=====(\n' + js + ')=====";\n\n';
        out += 'const char WEB_MANIFEST[] PROGMEM = R"=====(\n' + manifestMin + ')=====";\n\n';
        out += 'const char WEB_SW[] PROGMEM = R"=====(\n' + swMin + ')=====";\n\n';
        out += 'const char WEB_ICON[] PROGMEM = R"=====(\n' + iconMin + ')=====";\n\n';
        out += `const size_t WEB_ICON_PNG_LEN = ${iconPngLen};\n`;
        out += 'const uint8_t WEB_ICON_PNG[] PROGMEM = {\n' + iconPngBytes + '\n};\n\n';
        
        out += "#endif\n";
        
        const outPath = path.join(rootDir, 'trenzin', 'web_assets.h');
        fs.writeFileSync(outPath, out, 'utf8');
        console.log(`Gerado ${outPath} com sucesso!`);
    } catch (err) {
        console.error("Erro ao gerar assets:", err);
    }
}

build();
