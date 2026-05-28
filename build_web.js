const fs = require('fs');
const path = require('path');

function build() {
    const rootDir = __dirname;
    const webDir = path.join(rootDir, 'web');
    
    try {
        const html = fs.readFileSync(path.join(webDir, 'index.html'), 'utf8');
        const css = fs.readFileSync(path.join(webDir, 'style.css'), 'utf8');
        const js = fs.readFileSync(path.join(webDir, 'app.js'), 'utf8');

        let out = `// Arquivo gerado automaticamente
// NÃO EDITE MANUALMENTE!

#ifndef WEB_ASSETS_H
#define WEB_ASSETS_H

#include <pgmspace.h>

`;
        out += 'const char WEB_HTML[] PROGMEM = R"=====(\n' + html + ')=====";\n\n';
        out += 'const char WEB_CSS[] PROGMEM = R"=====(\n' + css + ')=====";\n\n';
        out += 'const char WEB_JS[] PROGMEM = R"=====(\n' + js + ')=====";\n\n';
        
        out += "#endif\n";
        
        const outPath = path.join(rootDir, 'taby', 'web_assets.h');
        fs.writeFileSync(outPath, out, 'utf8');
        console.log(`Gerado ${outPath} com sucesso!`);
    } catch (err) {
        console.error("Erro ao gerar assets:", err);
    }
}

build();
