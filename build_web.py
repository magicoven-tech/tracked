import os
import json
import re

def build():
    root_dir = os.path.dirname(os.path.abspath(__file__))
    web_dir = os.path.join(root_dir, 'web')
    
    try:
        with open(os.path.join(web_dir, 'index.html'), 'r', encoding='utf-8') as f:
            html = f.read()
        
        with open(os.path.join(web_dir, 'style.css'), 'r', encoding='utf-8') as f:
            css = f.read()
        
        with open(os.path.join(web_dir, 'app.js'), 'r', encoding='utf-8') as f:
            js = f.read()
            
        with open(os.path.join(web_dir, 'manifest.json'), 'r', encoding='utf-8') as f:
            manifest = f.read()
            
        with open(os.path.join(web_dir, 'sw.js'), 'r', encoding='utf-8') as f:
            sw = f.read()
            
        with open(os.path.join(web_dir, 'icon.svg'), 'r', encoding='utf-8') as f:
            icon = f.read()
    except Exception as e:
        print("Erro ao ler arquivos web:", e)
        return

    # Minificação básica
    manifest_min = json.dumps(json.loads(manifest))
    
    # Remover comentários e agrupar espaços do sw.js
    sw_min = re.sub(r'//.*$', '', sw, flags=re.MULTILINE)
    sw_min = re.sub(r'/\*[\s\S]*?\*/', '', sw_min)
    sw_min = re.sub(r'\s+', ' ', sw_min).strip()
    
    # Remover comentários e agrupar espaços do icon.svg
    icon_min = re.sub(r'<!--[\s\S]*?-->', '', icon)
    icon_min = re.sub(r'\s+', ' ', icon_min).strip()

    out = """// Arquivo gerado automaticamente por build_web.py
// NÃO EDITE MANUALMENTE!

#ifndef WEB_ASSETS_H
#define WEB_ASSETS_H

#include <pgmspace.h>

"""
    
    out += 'const char WEB_HTML[] PROGMEM = R"=====(\n' + html + ')=====";\n\n'
    out += 'const char WEB_CSS[] PROGMEM = R"=====(\n' + css + ')=====";\n\n'
    out += 'const char WEB_JS[] PROGMEM = R"=====(\n' + js + ')=====";\n\n'
    out += 'const char WEB_MANIFEST[] PROGMEM = R"=====(\n' + manifest_min + ')=====";\n\n'
    out += 'const char WEB_SW[] PROGMEM = R"=====(\n' + sw_min + ')=====";\n\n'
    out += 'const char WEB_ICON[] PROGMEM = R"=====(\n' + icon_min + ')=====";\n\n'
    
    out += "#endif\n"
    
    out_path = os.path.join(root_dir, 'trenzin', 'web_assets.h')
    
    with open(out_path, 'w', encoding='utf-8') as f:
        f.write(out)
        
    print(f"Gerado {out_path} com sucesso!")

if __name__ == '__main__':
    build()
