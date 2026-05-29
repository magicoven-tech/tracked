import os

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
    except Exception as e:
        print("Erro ao ler arquivos web:", e)
        return

    out = """// Arquivo gerado automaticamente por build_web.py
// NÃO EDITE MANUALMENTE!

#ifndef WEB_ASSETS_H
#define WEB_ASSETS_H

#include <pgmspace.h>

"""
    
    out += 'const char WEB_HTML[] PROGMEM = R"=====(\n' + html + ')=====";\n\n'
    out += 'const char WEB_CSS[] PROGMEM = R"=====(\n' + css + ')=====";\n\n'
    out += 'const char WEB_JS[] PROGMEM = R"=====(\n' + js + ')=====";\n\n'
    
    out += "#endif\n"
    
    out_path = os.path.join(root_dir, 'trenzin', 'web_assets.h')
    
    with open(out_path, 'w', encoding='utf-8') as f:
        f.write(out)
        
    print(f"Gerado {out_path} com sucesso!")

if __name__ == '__main__':
    build()
